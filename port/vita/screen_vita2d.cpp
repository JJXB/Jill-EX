#include "screen.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <vita2d.h>

#include "platform_paths.h"

namespace {

// The Vita panel is always 960x544.  The game frame is 320x200, so present it
// at the largest integer scale that fits and let the black clear fill the
// letterbox.
constexpr int kVitaWidth = 960;
constexpr int kVitaHeight = 544;

// The Framebuffer produces 0xAARRGGBB.  vita2d's A8B8G8R8 texture stores its
// bytes as R,G,B,A, i.e. 0xAABBGGRR when read as a little-endian uint32_t.
uint32_t argb_to_texture(uint32_t c) {
    return (c & 0xFF00FF00u) | ((c & 0x000000FFu) << 16) |
           ((c & 0x00FF0000u) >> 16);
}

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
        // Keep the app's own frame pacing; high-framerate modes present at
        // 60/90/120/144 Hz and must not be throttled to the panel refresh.
        vita2d_set_vblank_wait(0);
        vita2d_set_clear_color(0xFF000000u);  // RGBA8(0,0,0,255)
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
        if (!ensure_texture(w, h)) {
            return;
        }

        uint32_t* dst =
            static_cast<uint32_t*>(vita2d_texture_get_datap(texture_));
        const int stride =
            static_cast<int>(vita2d_texture_get_stride(texture_) / 4);
        for (int y = 0; y < h; ++y) {
            const uint32_t* src = argb + static_cast<size_t>(y) * w;
            uint32_t* row = dst + static_cast<size_t>(y) * stride;
            for (int x = 0; x < w; ++x) {
                row[x] = argb_to_texture(src[x]);
            }
        }

        const int scale = std::max(
            1, std::min(kVitaWidth / w, kVitaHeight / h));
        const float draw_w = static_cast<float>(w * scale);
        const float draw_h = static_cast<float>(h * scale);
        const float draw_x = (kVitaWidth - draw_w) * 0.5f;
        const float draw_y = (kVitaHeight - draw_h) * 0.5f;

        vita2d_start_drawing();
        vita2d_clear_screen();
        vita2d_draw_texture_scale(texture_, draw_x, draw_y,
                                  static_cast<float>(scale),
                                  static_cast<float>(scale));
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    void set_title(const char* title) override { (void)title; }

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
    bool ensure_texture(int w, int h) {
        if (texture_ && tex_w_ == w && tex_h_ == h) {
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
        vita2d_texture_set_filters(texture_, SCE_GXM_TEXTURE_FILTER_POINT,
                                   SCE_GXM_TEXTURE_FILTER_POINT);
        tex_w_ = w;
        tex_h_ = h;
        return true;
    }

    vita2d_texture* texture_ = nullptr;
    int tex_w_ = 0;
    int tex_h_ = 0;
    bool inited_ = false;
};

}  // namespace

std::unique_ptr<Screen> create_screen_vita2d() {
    return std::unique_ptr<Screen>(new ScreenVita2D());
}
