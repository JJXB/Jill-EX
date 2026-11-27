#include "screen.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

#include "platform_paths.h"
#include "scaler.h"

#ifdef JILL_HAVE_SDL
#include <SDL.h>
#endif

#ifdef JILL_XBOX
#include <hal/video.h>
#endif

// Console targets that present by scaling the ARGB frame into the window
// surface themselves: the PS4 driver needs it, and the Xbox (NXDK) SDL2 2.0.9
// port only provides a software window framebuffer.
#if defined(JILL_PS4) || defined(JILL_XBOX)
#define JILL_DIRECT_PRESENT 1
#endif

namespace {

#ifdef JILL_HAVE_SDL

// Desktop display: SDL2 window + accelerated (or software) renderer.
// PS4 display: SDL2 PS4 video driver, but with direct writes into the window
// surface.  On PS4 the SDL software renderer's conversion to the BGR888 window
// surface produced a black image, so the pixels are written and scaled by hand.
class ScreenSDL final : public Screen {
public:
    ~ScreenSDL() override { shutdown(); }

    bool init(int window_w, int window_h, int mode, std::string& err) override {
        jill_paths::trace("sdl screen: init %dx%d mode=%d", window_w, window_h,
                          mode);
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
            err = std::string("SDL video: ") + SDL_GetError();
            return false;
        }
#ifdef JILL_DIRECT_PRESENT
        // The PS4 SDL2 video driver reports the real video mode (1080p on
        // hardware, 720p under shadPS4) and uses the window position as the
        // destination offset when blitting to the video buffer.  Creating the
        // window at any other size makes SDL centre it at a negative position,
        // which the driver clamps into the bottom-right corner -> black screen.
        // The Xbox port has a single 640x480 output; SDL reports it as the
        // desktop mode and the window covers the whole screen.
        SDL_DisplayMode dm{};
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
            window_w = dm.w;
            window_h = dm.h;
        }
#ifdef JILL_XBOX
        // The window surface must fit the GPU framebuffer (SDL asserts on it),
        // so take the size straight from NXDK's current video mode.
        {
            const VIDEO_MODE vm = XVideoGetMode();
            if (vm.width > 0 && vm.height > 0) {
                window_w = vm.width;
                window_h = vm.height;
            }
            jill_paths::trace("sdl screen: xbox mode %dx%d %dbpp",
                              window_w, window_h, vm.bpp);
        }
#endif
        jill_paths::trace("sdl screen: video mode %dx%d", window_w, window_h);
        window_ = SDL_CreateWindow("Jill (port skeleton)", 0, 0, window_w,
                                   window_h, 0);
#else
#ifdef JILL_VITA
        // The Vita panel is fixed at 960x544.  Always create the window at the
        // real display size; the desktop-oriented scale/resolution settings
        // must never produce a larger drawable than the panel.
        SDL_DisplayMode dm{};
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
            window_w = dm.w;
            window_h = dm.h;
        } else {
            window_w = 960;
            window_h = 544;
        }
        mode = 2;  // always borderless fullscreen on the handheld panel
        jill_paths::trace("sdl screen: vita mode %dx%d", window_w, window_h);
#elif defined(JILL_ANDROID)
        // The window is the device display and always fullscreen.  Landscape
        // is enforced by the manifest; the hint keeps SDL from rotating it
        // into a portrait surface.
        SDL_DisplayMode dm{};
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
            window_w = dm.w;
            window_h = dm.h;
        }
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        jill_paths::trace("sdl screen: android mode %dx%d", window_w, window_h);
#endif
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        window_ = SDL_CreateWindow("Jill (port skeleton)", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, window_w, window_h,
                                   SDL_WINDOW_RESIZABLE);
#endif
        if (!window_) {
            err = std::string("SDL_CreateWindow: ") + SDL_GetError();
            return false;
        }

#ifdef JILL_DIRECT_PRESENT
        surface_ = SDL_GetWindowSurface(window_);
        if (!surface_) {
            err = std::string("SDL_GetWindowSurface: ") + SDL_GetError();
            return false;
        }
#ifdef JILL_XBOX
        mode_w_ = surface_->w;
        mode_h_ = surface_->h;
        // One-off bandwidth check: the presenter writes ~1 MB into the video
        // buffer every frame, so knowing the achievable write rate tells us
        // whether a slow frame is memory-bound.  Logged as whole MB/s because
        // pdclib's printf has no %f.
        {
            const VIDEO_MODE vm = XVideoGetMode();
            uint8_t* fb = XVideoGetFB();
            const size_t bytes = static_cast<size_t>(vm.width) * vm.height * 4;
            const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
            if (fb != nullptr && bytes > 0 && freq > 0.0) {
                std::vector<uint8_t> cached(bytes);
                uint64_t t0 = SDL_GetPerformanceCounter();
                for (int i = 0; i < 5; ++i) {
                    std::memset(cached.data(), i, bytes);
                }
                uint64_t t1 = SDL_GetPerformanceCounter();
                for (int i = 0; i < 5; ++i) {
                    std::memset(fb, i, bytes);
                }
                XVideoFlushFB();
                uint64_t t2 = SDL_GetPerformanceCounter();
                const double cached_s = (t1 - t0) / freq;
                const double fb_s = (t2 - t1) / freq;
                jill_paths::trace(
                    "xbox fb bench: cached=%u MB/s framebuffer=%u MB/s "
                    "(%u bytes)",
                    cached_s > 0.0 ? static_cast<unsigned>(5.0 * bytes / cached_s / 1e6) : 0u,
                    fb_s > 0.0 ? static_cast<unsigned>(5.0 * bytes / fb_s / 1e6) : 0u,
                    static_cast<unsigned>(bytes));
            }
        }
#endif
        int wx = 0, wy = 0;
        SDL_GetWindowPosition(window_, &wx, &wy);
        jill_paths::trace("sdl screen: window at %d,%d surface %dx%d %s bpp=%d",
                          wx, wy, surface_->w, surface_->h,
                          SDL_GetPixelFormatName(surface_->format->format),
                          surface_->format->BytesPerPixel);
#else
        renderer_ =
            SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer_) {
            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
        }
        if (!renderer_) {
            err = std::string("SDL_CreateRenderer: ") + SDL_GetError();
            return false;
        }
        apply(window_w, window_h, mode, 0, 0);
#endif
        jill_paths::trace("sdl screen: ready (driver=%s)",
                          SDL_GetCurrentVideoDriver()
                              ? SDL_GetCurrentVideoDriver()
                              : "?");
        return true;
    }

    void apply(int pixel_w, int pixel_h, int mode, int display_w,
               int display_h) override {
#ifdef JILL_VITA
        // The panel is the only output; ignore windowed/resolution requests.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
        if (window_) {
            SDL_SetWindowBordered(window_, SDL_FALSE);
            SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
#elif defined(JILL_XBOX)
        // Xbox: VIDEO > RESOLUTION picks the output mode.  SDL's Xbox video
        // driver cannot switch modes (its SetDisplayMode is a no-op), so the
        // mode is set through NXDK and the window surface is recreated to match
        // the new framebuffer size.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        const int want_w = display_w > 0 ? display_w : 640;
        const int want_h = display_h > 0 ? display_h : 480;
        set_xbox_mode(want_w, want_h);
#elif defined(JILL_DIRECT_PRESENT)
        // PS4 video output is fixed at 1080p; resizing after the window surface
        // is created can invalidate it.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
#elif defined(JILL_ANDROID)
        // Android owns the window size and orientation.  The presenter scales
        // the 320x200 frame to the surface with the selected scaler.
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
#else
        if (!window_) {
            return;
        }
        if (mode == 2) {  // borderless fullscreen
            SDL_SetWindowBordered(window_, SDL_FALSE);
            SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN_DESKTOP);
            return;
        }
        SDL_SetWindowBordered(window_, SDL_TRUE);
        if (mode == 1) {  // exclusive fullscreen at the chosen mode
            if (display_w > 0 && display_h > 0) {
                SDL_DisplayMode want{};
                want.w = display_w;
                want.h = display_h;
                want.format = SDL_PIXELFORMAT_UNKNOWN;
                want.refresh_rate = 0;
                SDL_DisplayMode closest{};
                if (SDL_GetClosestDisplayMode(0, &want, &closest)) {
                    SDL_SetWindowDisplayMode(window_, &closest);
                }
            }
            SDL_SetWindowFullscreen(window_, SDL_WINDOW_FULLSCREEN);
            return;
        }
        SDL_SetWindowFullscreen(window_, 0);
        SDL_SetWindowSize(window_, pixel_w, pixel_h);
        SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
#endif
    }

    void present(const uint32_t* argb, int w, int h) override {
#if defined(JILL_XBOX)
        present_xbox(argb, w, h);
#elif defined(JILL_DIRECT_PRESENT)
        present_direct(argb, w, h);
#else
        present_renderer(argb, w, h);
#endif
    }

    void set_title(const char* title) override {
        if (window_) {
            SDL_SetWindowTitle(window_, title);
        }
    }

    void set_scale_mode(int mode) override {
        if (mode < jill_scale::kInteger || mode > jill_scale::kFull) {
            mode = jill_scale::kInteger;
        }
        scale_mode_ = mode;
    }

    void set_mouse_grab(bool on) override {
        if (!window_) {
            return;
        }
        mouse_grabbed_ = on;
        SDL_SetWindowGrab(window_, on ? SDL_TRUE : SDL_FALSE);
        if (on) {
            refresh_mouse_rect();
        } else {
#if SDL_VERSION_ATLEAST(2, 0, 18)
            SDL_SetWindowMouseRect(window_, nullptr);
#endif
            mouse_rect_ = SDL_Rect{};
        }
    }

    void refresh_mouse_rect() override {
#if SDL_VERSION_ATLEAST(2, 0, 18)
        if (!window_ || !mouse_grabbed_) {
            return;
        }
        SDL_Rect rect{};
        if (!frame_window_rect(rect)) {
            return;
        }
        if (std::memcmp(&rect, &mouse_rect_, sizeof(rect)) == 0) {
            return;
        }
        mouse_rect_ = rect;
        SDL_SetWindowMouseRect(window_, &rect);
#else
        (void)0;
#endif
    }

    bool window_to_frame(int wx, int wy, int& x, int& y) override {
        if (!window_) {
            return false;
        }
#ifndef JILL_DIRECT_PRESENT
        if (!renderer_) {
            return false;
        }
        // Map the window pixel through the same fit the presenter uses.
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        if (ww <= 0 || wh <= 0) {
            return false;
        }
        int ow = 0;
        int oh = 0;
        if (SDL_GetRendererOutputSize(renderer_, &ow, &oh) != 0 || ow <= 0 ||
            oh <= 0) {
            return false;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, ow, oh, scale_mode_);
        if (fit.w <= 0 || fit.h <= 0) {
            return false;
        }
        const float px = static_cast<float>(wx) * ow / ww;
        const float py = static_cast<float>(wy) * oh / wh;
        x = static_cast<int>((px - fit.x) * 320.0f / fit.w);
        y = static_cast<int>((py - fit.y) * 200.0f / fit.h);
        // Clicking in the letterbox saturates at the nearest frame edge.
        if (x < 0) {
            x = 0;
        } else if (x >= 320) {
            x = 319;
        }
        if (y < 0) {
            y = 0;
        } else if (y >= 200) {
            y = 199;
        }
        return true;
#else
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        if (ww <= 0 || wh <= 0) {
            return false;
        }
        int pw = ww;
        int ph = wh;
        if (surface_) {
            pw = surface_->w;
            ph = surface_->h;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, pw, ph, scale_mode_);
        const float scale_x =
            static_cast<float>(fit.w) / static_cast<float>(320);
        const float scale_y =
            static_cast<float>(fit.h) / static_cast<float>(200);
        const int px = wx * pw / ww;
        const int py = wy * ph / wh;
        x = static_cast<int>((px - fit.x) / (scale_x > 0.0f ? scale_x : 1.0f));
        y = static_cast<int>((py - fit.y) / (scale_y > 0.0f ? scale_y : 1.0f));
        if (x < 0) {
            x = 0;
        } else if (x >= 320) {
            x = 319;
        }
        if (y < 0) {
            y = 0;
        } else if (y >= 200) {
            y = 199;
        }
        return true;
#endif
    }

    bool touch_to_frame(float nx, float ny, int& x, int& y) override {
#if defined(JILL_DIRECT_PRESENT)
        // Direct window-surface presentation has no SDL renderer/logical size.
        (void)nx;
        (void)ny;
        (void)x;
        (void)y;
        return false;
#else
        if (!window_ || !renderer_) {
            return false;
        }
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        if (ww <= 0 || wh <= 0) {
            return false;
        }
        int ow = 0;
        int oh = 0;
        if (SDL_GetRendererOutputSize(renderer_, &ow, &oh) != 0 || ow <= 0 ||
            oh <= 0) {
            return false;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, ow, oh, scale_mode_);
        if (fit.w <= 0 || fit.h <= 0) {
            return false;
        }
        const float px = nx * static_cast<float>(ow);
        const float py = ny * static_cast<float>(oh);
        x = static_cast<int>((px - fit.x) * 320.0f / fit.w);
        y = static_cast<int>((py - fit.y) * 200.0f / fit.h);
        if (x < 0) {
            x = 0;
        } else if (x >= 320) {
            x = 319;
        }
        if (y < 0) {
            y = 0;
        } else if (y >= 200) {
            y = 199;
        }
        return true;
#endif
    }

    void shutdown() override {
        if (texture_) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
            renderer_ = nullptr;
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = nullptr;
        }
#ifdef JILL_DIRECT_PRESENT
        surface_ = nullptr;
#endif
    }

private:
    bool frame_window_rect(SDL_Rect& rect) {
        if (!window_) {
            return false;
        }
        int ww = 0;
        int wh = 0;
        SDL_GetWindowSize(window_, &ww, &wh);
        if (ww <= 0 || wh <= 0) {
            return false;
        }
#ifndef JILL_DIRECT_PRESENT
        if (!renderer_) {
            return false;
        }
        int ow = 0;
        int oh = 0;
        if (SDL_GetRendererOutputSize(renderer_, &ow, &oh) != 0 || ow <= 0 ||
            oh <= 0) {
            return false;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, ow, oh, scale_mode_);
        rect.x = fit.x * ww / ow;
        rect.y = fit.y * wh / oh;
        rect.w = fit.w * ww / ow;
        rect.h = fit.h * wh / oh;
        // Keep a whole-pixel rect so SDL's pointer confine does not jitter.
        if (rect.w < 1) {
            rect.w = 1;
        }
        if (rect.h < 1) {
            rect.h = 1;
        }
#else
        const jill_scale::Fit fit =
            jill_scale::compute_fit(320, 200, ww, wh, scale_mode_);
        rect.w = fit.w;
        rect.h = fit.h;
        rect.x = fit.x;
        rect.y = fit.y;
#endif
        return rect.w > 0 && rect.h > 0;
    }

#ifdef JILL_XBOX
    // Scale the 320x200 ARGB frame straight into the GPU framebuffer.
    // SDL's own window-surface route would copy the frame twice: once into the
    // window surface (this loop) and again through SDL_ConvertPixels into the
    // video buffer, which is slow write-combined memory on the Xbox.
    void present_xbox(const uint32_t* argb, int w, int h) {
        const VIDEO_MODE vm = XVideoGetMode();
        uint32_t* dst = reinterpret_cast<uint32_t*>(XVideoGetFB());
        if (dst == nullptr || vm.width <= 0 || vm.height <= 0 || vm.bpp != 32) {
            present_direct(argb, w, h);  // unusual mode: use the SDL surface
            return;
        }
        const jill_scale::Fit fit =
            jill_scale::compute_fit(w, h, vm.width, vm.height, scale_mode_);
        const int out_w = fit.w;
        const int out_h = fit.h;
        const int off_x = fit.x;
        const int off_y = fit.y;

        // Black out the letterbox bars only; the frame itself overwrites every
        // pixel it covers.
        for (int y = 0; y < vm.height; ++y) {
            uint32_t* row = dst + static_cast<size_t>(y) * vm.width;
            if (y < off_y || y >= off_y + out_h) {
                std::memset(row, 0, static_cast<size_t>(vm.width) * 4);
                continue;
            }
            if (off_x > 0) {
                std::memset(row, 0, static_cast<size_t>(off_x) * 4);
            }
            if (off_x + out_w < vm.width) {
                std::memset(row + off_x + out_w, 0,
                            static_cast<size_t>(vm.width - off_x - out_w) * 4);
            }
        }

        if (scale_mode_ == jill_scale::kInteger) {
            // Build one scaled row, then copy it into the `scale` destination
            // rows.  memcpy into the write-combined framebuffer is much faster
            // than per-pixel stores.
            const int scale = out_w / w;
            if (row_buf_.size() < static_cast<size_t>(out_w)) {
                row_buf_.resize(static_cast<size_t>(out_w));
            }
            for (int sy = 0; sy < h; ++sy) {
                const uint32_t* src = argb + static_cast<size_t>(sy) * w;
                uint32_t* expanded = row_buf_.data();
                for (int sx = 0; sx < w; ++sx) {
                    const uint32_t c = src[sx] | 0xFF000000u;
                    for (int dx = 0; dx < scale; ++dx) {
                        *expanded++ = c;
                    }
                }
                for (int dy = 0; dy < scale; ++dy) {
                    uint32_t* row =
                        dst + static_cast<size_t>(off_y + sy * scale + dy) *
                                  vm.width +
                        off_x;
                    std::memcpy(row, row_buf_.data(),
                                static_cast<size_t>(out_w) * sizeof(uint32_t));
                }
            }
        } else {
            scaler_.setup(w, h, out_w, out_h, scale_mode_);
            scaler_.blit(argb, [&](int x, int y, uint32_t c) {
                dst[static_cast<size_t>(off_y + y) * vm.width + off_x + x] = c;
            });
        }
        XVideoFlushFB();
        if (present_count_ < 3 || present_count_ % 120 == 0) {
            jill_paths::trace("sdl screen: presented #%u (%dx%d -> %dx%d)",
                              present_count_, w, h, out_w, out_h);
        }
        ++present_count_;
    }
#endif

#ifdef JILL_XBOX
    // Set the Xbox output mode and rebuild the window surface to match it.
    // XVideoSetMode() fails for modes the AV pack / dashboard does not allow,
    // in which case the current mode stays.
    void set_xbox_mode(int want_w, int want_h) {
        if (want_w == mode_w_ && want_h == mode_h_) {
            return;
        }
        if (!XVideoSetMode(want_w, want_h, 32, REFRESH_DEFAULT)) {
            jill_paths::trace("sdl screen: XVideoSetMode %dx%d unavailable",
                              want_w, want_h);
            return;
        }
        const VIDEO_MODE vm = XVideoGetMode();
        jill_paths::trace("sdl screen: mode %dx%d (%dbpp, %d Hz)", vm.width,
                          vm.height, vm.bpp, vm.refresh);
        // No window recreation: the frame is presented straight into the GPU
        // framebuffer (present_xbox), and SDL's Xbox driver refuses a second
        // window.  The window surface is only a fallback for odd modes.
        mode_w_ = vm.width;
        mode_h_ = vm.height;
    }
#endif

#ifndef JILL_DIRECT_PRESENT
    void present_renderer(const uint32_t* argb, int w, int h) {
        if (!renderer_) {
            return;
        }
        int out_pw = 0;
        int out_ph = 0;
        if (SDL_GetRendererOutputSize(renderer_, &out_pw, &out_ph) != 0 ||
            out_pw <= 0 || out_ph <= 0) {
            out_pw = w;
            out_ph = h;
        }
        // Logical scaling is disabled (set once below), so every destination
        // rect is in renderer output pixels and the fit can be fractional.
        const bool vertical = (scale_mode_ == jill_scale::kVertical);
        const bool linear = (scale_mode_ == jill_scale::kFull);
        const jill_scale::Fit fit =
            jill_scale::compute_fit(w, h, out_pw, out_ph, scale_mode_);
        int tex_h = h;
        if (vertical) {
            tex_h = std::max(1, fit.h);
        }
        if (!texture_ || tex_w_ != w || tex_h_ != tex_h || tex_linear_ != linear) {
            if (texture_) {
                SDL_DestroyTexture(texture_);
                texture_ = nullptr;
            }
            SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
                        linear ? "linear" : "nearest");
            texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, w, tex_h);
            if (!texture_) {
                return;
            }
            tex_w_ = w;
            tex_h_ = tex_h;
            tex_linear_ = linear;
            SDL_RenderSetLogicalSize(renderer_, 0, 0);
        }
        if (vertical) {
            // Bilinear on Y only: pre-scale the rows, then let the nearest
            // sampler handle the fractional horizontal stretch.
            vert_buf_.resize(static_cast<size_t>(w) * tex_h);
            scaler_.setup(w, h, w, tex_h, jill_scale::kVertical);
            scaler_.blit(argb, [&](int x, int y, uint32_t c) {
                vert_buf_[static_cast<size_t>(y) * w + x] = c;
            });
            SDL_UpdateTexture(texture_, nullptr, vert_buf_.data(),
                              w * static_cast<int>(sizeof(uint32_t)));
        } else {
            SDL_UpdateTexture(texture_, nullptr, argb,
                              w * static_cast<int>(sizeof(uint32_t)));
        }
        SDL_RenderClear(renderer_);
        SDL_Rect dst{};
        dst.x = fit.x;
        dst.y = fit.y;
        dst.w = fit.w;
        dst.h = fit.h;
        if (SDL_RenderCopy(renderer_, texture_, nullptr, &dst) != 0) {
            jill_paths::trace("sdl screen: copy failed: %s", SDL_GetError());
        }
        SDL_RenderPresent(renderer_);
    }
#else
    void present_direct(const uint32_t* argb, int w, int h) {
        SDL_Surface* s = surface_;
        if (!s || !s->pixels || w <= 0 || h <= 0) {
            return;
        }
        const SDL_PixelFormat* f = s->format;
        const int bpp = f->BytesPerPixel;
        if (bpp != 2 && bpp != 3 && bpp != 4) {
            jill_paths::trace("sdl screen: unsupported surface bpp=%d", bpp);
            return;
        }

        SDL_FillRect(s, nullptr, 0);  // black letterbox

        const jill_scale::Fit fit =
            jill_scale::compute_fit(w, h, s->w, s->h, scale_mode_);
        const int out_w = fit.w;
        const int out_h = fit.h;
        const int off_x = fit.x;
        const int off_y = fit.y;

        if (SDL_MUSTLOCK(s)) {
            SDL_LockSurface(s);
        }
        uint8_t* base = static_cast<uint8_t*>(s->pixels);

        // Convert one 0xAARRGGBB pixel to the window surface's format.
        const auto pack = [&](uint32_t c) -> uint32_t {
            const uint32_t r = (c >> 16) & 0xFFu;
            const uint32_t g = (c >> 8) & 0xFFu;
            const uint32_t b = c & 0xFFu;
            uint32_t v = (((r >> f->Rloss) & 0xFFu) << f->Rshift) |
                         (((g >> f->Gloss) & 0xFFu) << f->Gshift) |
                         (((b >> f->Bloss) & 0xFFu) << f->Bshift);
            if (f->Amask != 0) {
                v |= 0xFFu << f->Ashift;
            }
            return v;
        };

        if (scale_mode_ == jill_scale::kInteger) {
            const int scale = out_w / w;
            for (int sy = 0; sy < h; ++sy) {
                const uint32_t* src = argb + static_cast<size_t>(sy) * w;
                for (int sx = 0; sx < w; ++sx) {
                    const uint32_t v = pack(src[sx]);
                    for (int dy = 0; dy < scale; ++dy) {
                        uint8_t* row =
                            base +
                            static_cast<size_t>(off_y + sy * scale + dy) *
                                s->pitch;
                        uint8_t* dst =
                            row + static_cast<size_t>(off_x + sx * scale) * bpp;
                        for (int dx = 0; dx < scale; ++dx) {
                            if (bpp == 4) {
                                *reinterpret_cast<uint32_t*>(dst) = v;
                            } else if (bpp == 3) {
                                dst[0] = static_cast<uint8_t>(v & 0xFF);
                                dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
                                dst[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
                            } else {
                                *reinterpret_cast<uint16_t*>(dst) =
                                    static_cast<uint16_t>(v);
                            }
                            dst += bpp;
                        }
                    }
                }
            }
        } else {
            scaler_.setup(w, h, out_w, out_h, scale_mode_);
            scaler_.blit(argb, [&](int x, int y, uint32_t c) {
                const uint32_t v = pack(c);
                uint8_t* dst = base +
                               static_cast<size_t>(off_y + y) * s->pitch +
                               static_cast<size_t>(off_x + x) * bpp;
                if (bpp == 4) {
                    *reinterpret_cast<uint32_t*>(dst) = v;
                } else if (bpp == 3) {
                    dst[0] = static_cast<uint8_t>(v & 0xFF);
                    dst[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
                    dst[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
                } else {
                    *reinterpret_cast<uint16_t*>(dst) =
                        static_cast<uint16_t>(v);
                }
            });
        }
        if (SDL_MUSTLOCK(s)) {
            SDL_UnlockSurface(s);
        }

        if (!first_present_) {
            first_present_ = true;
            jill_paths::trace("sdl screen: first present %dx%d -> %dx%d @%d,%d",
                              w, h, out_w, out_h, off_x, off_y);
        }
        if (SDL_UpdateWindowSurface(window_) != 0) {
            jill_paths::trace("sdl screen: update failed: %s", SDL_GetError());
        }
        // Proves the SDL flip returns; a missing line here means the driver
        // blocked inside SubmitFlip/WaitOnFlip.
        if (present_count_ < 3 || present_count_ % 120 == 0) {
            jill_paths::trace("sdl screen: presented #%u", present_count_);
        }
        ++present_count_;
    }
#endif

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    bool mouse_grabbed_ = false;
    SDL_Rect mouse_rect_{};
    int scale_mode_ = jill_scale::kInteger;
    jill_scale::Scaler scaler_;
#ifndef JILL_DIRECT_PRESENT
    std::vector<uint32_t> vert_buf_;
    int tex_w_ = 0;
    int tex_h_ = 0;
    bool tex_linear_ = false;
#endif
#ifdef JILL_DIRECT_PRESENT
    SDL_Surface* surface_ = nullptr;
    bool first_present_ = false;
    unsigned present_count_ = 0;
#endif
#ifdef JILL_XBOX
    int mode_w_ = 0;
    int mode_h_ = 0;
    std::vector<uint32_t> row_buf_;
#endif
};

#endif  // JILL_HAVE_SDL

}  // namespace

std::unique_ptr<Screen> create_screen_sdl() {
#ifdef JILL_HAVE_SDL
    return std::unique_ptr<Screen>(new ScreenSDL());
#else
    return nullptr;
#endif
}
