#include "screen.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>

#include "platform_paths.h"

#ifdef JILL_HAVE_SDL
#include <SDL.h>
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
#ifdef JILL_PS4
        // The PS4 SDL2 video driver reports the real video mode (1080p on
        // hardware, 720p under shadPS4) and uses the window position as the
        // destination offset when blitting to the video buffer.  Creating the
        // window at any other size makes SDL centre it at a negative position,
        // which the driver clamps into the bottom-right corner -> black screen.
        SDL_DisplayMode dm{};
        if (SDL_GetDesktopDisplayMode(0, &dm) == 0 && dm.w > 0 && dm.h > 0) {
            window_w = dm.w;
            window_h = dm.h;
        }
        jill_paths::trace("sdl screen: video mode %dx%d", window_w, window_h);
        window_ = SDL_CreateWindow("Jill (port skeleton)", 0, 0, window_w,
                                   window_h, 0);
#else
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
        window_ = SDL_CreateWindow("Jill (port skeleton)", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, window_w, window_h,
                                   SDL_WINDOW_RESIZABLE);
#endif
        if (!window_) {
            err = std::string("SDL_CreateWindow: ") + SDL_GetError();
            return false;
        }

#ifdef JILL_PS4
        surface_ = SDL_GetWindowSurface(window_);
        if (!surface_) {
            err = std::string("SDL_GetWindowSurface: ") + SDL_GetError();
            return false;
        }
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
#ifdef JILL_PS4
        // PS4 video output is fixed at 1080p; resizing after the window surface
        // is created can invalidate it.
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
#ifdef JILL_PS4
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
#ifdef JILL_PS4
        surface_ = nullptr;
#endif
    }

private:
#ifndef JILL_PS4
    void present_renderer(const uint32_t* argb, int w, int h) {
        if (!renderer_) {
            return;
        }
        if (!texture_) {
            SDL_RenderSetLogicalSize(renderer_, w, h);
            SDL_RenderSetIntegerScale(renderer_, SDL_TRUE);
            texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                         SDL_TEXTUREACCESS_STREAMING, w, h);
            if (!texture_) {
                return;
            }
        }
        SDL_UpdateTexture(texture_, nullptr, argb,
                          w * static_cast<int>(sizeof(uint32_t)));
        SDL_RenderClear(renderer_);
        if (SDL_RenderCopy(renderer_, texture_, nullptr, nullptr) != 0) {
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

        int scale = std::min(s->w / w, s->h / h);
        if (scale < 1) {
            scale = 1;
        }
        const int out_w = w * scale;
        const int out_h = h * scale;
        const int off_x = (s->w - out_w) / 2;
        const int off_y = (s->h - out_h) / 2;

        if (SDL_MUSTLOCK(s)) {
            SDL_LockSurface(s);
        }
        uint8_t* base = static_cast<uint8_t*>(s->pixels);
        for (int sy = 0; sy < h; ++sy) {
            const uint32_t* src = argb + static_cast<size_t>(sy) * w;
            for (int sx = 0; sx < w; ++sx) {
                const uint32_t c = src[sx];
                const uint32_t r = (c >> 16) & 0xFFu;
                const uint32_t g = (c >> 8) & 0xFFu;
                const uint32_t b = c & 0xFFu;
                uint32_t v = (((r >> f->Rloss) & 0xFFu) << f->Rshift) |
                             (((g >> f->Gloss) & 0xFFu) << f->Gshift) |
                             (((b >> f->Bloss) & 0xFFu) << f->Bshift);
                if (f->Amask != 0) {
                    v |= 0xFFu << f->Ashift;
                }
                for (int dy = 0; dy < scale; ++dy) {
                    uint8_t* row =
                        base + static_cast<size_t>(off_y + sy * scale + dy) *
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
#ifdef JILL_PS4
    SDL_Surface* surface_ = nullptr;
    bool first_present_ = false;
    unsigned present_count_ = 0;
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
