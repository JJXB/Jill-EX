#include "screen.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <psp2/touch.h>
#include <vita2d.h>

#include "platform_paths.h"
#include "scaler.h"

namespace {

// The Vita panel is always 960x544.  The game frame is 320x200, so present it
// with the selected scaler and let the black clear fill the letterbox.
constexpr int kVitaWidth = 960;
constexpr int kVitaHeight = 544;

class ScreenVita2D final : public Screen {
public:
    ~ScreenVita2D() override { shutdown(); }

    bool init(int pixel_w, int pixel_h, int mode, std::string& err) override {
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        if (vita2d_init() < 0) {
            err = "vita2d_init failed";
            return false;
        }
        inited_ = true;
        // Pace on the panel's vblank.  The app's own 60 Hz sleep/spin timer
        // drifts against the 59.94 Hz display and drops/duplicates a frame
        // every so often, which shows up as hitching.  presents_synced() makes
        // the main loop skip its software pacing; rates above 60 Hz cannot be
        // shown by the panel anyway.
        vita2d_set_vblank_wait(1);
        vita2d_set_clear_color(0xFF000000u);  // RGBA8(0,0,0,255)
        // Front touchscreen for the editor toolbar (SDL video stays off, so
        // SDL_FINGER events are not available on Vita).
        sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT,
                                 SCE_TOUCH_SAMPLING_STATE_START);
        SceTouchPanelInfo panel{};
        if (sceTouchGetPanelInfo(SCE_TOUCH_PORT_FRONT, &panel) >= 0) {
            touch_min_x_ = static_cast<float>(panel.minAaX);
            touch_min_y_ = static_cast<float>(panel.minAaY);
            touch_w_ = static_cast<float>(panel.maxAaX - panel.minAaX);
            touch_h_ = static_cast<float>(panel.maxAaY - panel.minAaY);
            if (touch_w_ <= 0.0f) {
                touch_w_ = 1.0f;
            }
            if (touch_h_ <= 0.0f) {
                touch_h_ = 1.0f;
            }
        }
        jill_paths::trace("vita2d: ready (%dx%d)", kVitaWidth, kVitaHeight);
        return true;
    }

    void apply(int pixel_w, int pixel_h, int mode, int display_w,
               int display_h) override {
        // The Vita panel has a single fixed mode and the game always fills it.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
    }

    void present(const uint32_t* argb, int w, int h) override {
        if (!inited_ || !argb || w <= 0 || h <= 0) {
            return;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(w, h, kVitaWidth, kVitaHeight, scale_mode_);
        if (fit.w <= 0 || fit.h <= 0) {
            return;
        }
        const bool vertical = (scale_mode_ == jill_scale::kVertical);
        const int tex_h = vertical ? std::max(1, fit.h) : h;
        const bool linear = (scale_mode_ == jill_scale::kFull);
        if (!ensure_texture(w, tex_h, linear)) {
            return;
        }

        const uint32_t* upload = argb;
        if (vertical) {
            vert_buf_.resize(static_cast<size_t>(w) * tex_h);
            scaler_.setup(w, h, w, tex_h, jill_scale::kVertical);
            scaler_.blit(argb, [&](int x, int y, uint32_t c) {
                vert_buf_[static_cast<size_t>(y) * w + x] = c;
            });
            upload = vert_buf_.data();
        }

        // The palette expansion stamps the frame in the texture's byte order
        // already (see native_frame_order_bgr()), so this is a row copy with
        // no per-pixel channel swap.
        uint32_t* dst =
            static_cast<uint32_t*>(vita2d_texture_get_datap(texture_));
        const int stride =
            static_cast<int>(vita2d_texture_get_stride(texture_) / 4);
        if (stride == w) {
            std::memcpy(dst, upload,
                        static_cast<size_t>(w) * static_cast<size_t>(tex_h) *
                            sizeof(uint32_t));
        } else {
            for (int y = 0; y < tex_h; ++y) {
                std::memcpy(dst + static_cast<size_t>(y) * stride,
                            upload + static_cast<size_t>(y) * w,
                            static_cast<size_t>(w) * sizeof(uint32_t));
            }
        }

        const float draw_w = static_cast<float>(fit.w);
        const float draw_h = static_cast<float>(fit.h);
        const float draw_x = static_cast<float>(fit.x);
        const float draw_y = static_cast<float>(fit.y);
        const float scale_x = draw_w / static_cast<float>(w);
        const float scale_y = draw_h / static_cast<float>(tex_h);

        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_draw_texture_scale(texture_, draw_x, draw_y, scale_x, scale_y);
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    void set_scale_mode(int mode) override {
        if (mode < jill_scale::kInteger || mode > jill_scale::kFull) {
            mode = jill_scale::kInteger;
        }
        scale_mode_ = mode;
    }

    void set_title(const char* title) override { (void)title; }

    bool native_frame_order_bgr() const override { return true; }
    bool presents_synced() const override { return true; }

    bool touch_to_frame(float nx, float ny, int& x, int& y) override {
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, kVitaWidth, kVitaHeight,
                                    scale_mode_);
        if (fit.w <= 0 || fit.h <= 0) {
            return false;
        }
        const float px = nx * kVitaWidth;
        const float py = ny * kVitaHeight;
        x = static_cast<int>((px - fit.x) * 320.0f / fit.w);
        y = static_cast<int>((py - fit.y) * 200.0f / fit.h);
        return x >= 0 && y >= 0 && x < 320 && y < 200;
    }

    bool poll_touch(int& x, int& y, bool& down) override {
        SceTouchData data{};
        const int reports =
            sceTouchPeek(SCE_TOUCH_PORT_FRONT, &data, 1);
        if (reports < 0) {
            return false;
        }
        if (data.reportNum <= 0) {
            down = false;
            return true;
        }
        down = true;
        const float nx = std::max(
            0.0f, std::min(1.0f, (static_cast<float>(data.report[0].x) -
                                  touch_min_x_) /
                                     touch_w_));
        const float ny = std::max(
            0.0f, std::min(1.0f, (static_cast<float>(data.report[0].y) -
                                  touch_min_y_) /
                                     touch_h_));
        return touch_to_frame(nx, ny, x, y);
    }

    void shutdown() override {
        if (texture_) {
            vita2d_free_texture(texture_);
            texture_ = nullptr;
        }
        if (inited_) {
            vita2d_fini();
            inited_ = false;
        }
    }

private:
    bool ensure_texture(int w, int h, bool linear) {
        if (texture_ && tex_w_ == w && tex_h_ == h && tex_linear_ == linear) {
            return true;
        }
        if (texture_) {
            vita2d_free_texture(texture_);
            texture_ = nullptr;
        }
        texture_ = vita2d_create_empty_texture_format(
            static_cast<unsigned>(w), static_cast<unsigned>(h),
            SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
        if (!texture_) {
            return false;
        }
        const SceGxmTextureFilter filter =
            linear ? SCE_GXM_TEXTURE_FILTER_LINEAR : SCE_GXM_TEXTURE_FILTER_POINT;
        vita2d_texture_set_filters(texture_, filter, filter);
        tex_w_ = w;
        tex_h_ = h;
        tex_linear_ = linear;
        return true;
    }

    vita2d_texture* texture_ = nullptr;
    int tex_w_ = 0;
    int tex_h_ = 0;
    bool tex_linear_ = false;
    int scale_mode_ = jill_scale::kInteger;
    jill_scale::Scaler scaler_;
    std::vector<uint32_t> vert_buf_;
    bool inited_ = false;
    float touch_min_x_ = 0.0f;
    float touch_min_y_ = 0.0f;
    float touch_w_ = 1.0f;
    float touch_h_ = 1.0f;
};

}  // namespace

std::unique_ptr<Screen> create_screen_vita2d() {
    return std::unique_ptr<Screen>(new ScreenVita2D());
}
