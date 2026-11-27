#pragma once

#include <cstdint>
#include <memory>
#include <string>

// Platform display seam.
//
// The game still renders into a 320x200 indexed Framebuffer on every target.
// `Screen` owns the window/display and the GPU-side upload, so the SDL
// renderer (desktop) and the OpenGL ES 2.0 / Piglet renderer (PS4) can be
// swapped without touching game code.
class Screen {
public:
    virtual ~Screen() = default;

    // Create the display.  `mode`: 0 windowed, 1 fullscreen, 2 borderless.
    virtual bool init(int pixel_w, int pixel_h, int mode, std::string& err) = 0;

    // Re-apply the window/display mode after the VIDEO menu changed it.
    // `display_w`/`display_h` are 0 when "DESKTOP" is selected.
    virtual void apply(int pixel_w, int pixel_h, int mode, int display_w,
                       int display_h) = 0;

    // Upload one 0xAARRGGBB frame and present it.  Backends that report
    // `native_frame_order_bgr()` instead receive 0xAABBGGRR with matching
    // pixel order, so they can blit the buffer without a channel swap.
    virtual void present(const uint32_t* argb, int w, int h) = 0;

    // True when the backend's present() takes the frame with R/B swapped
    // (Vita's GXM texture format).  The palette expansion then writes the
    // swap directly, saving a full-frame pass on weak CPUs.
    virtual bool native_frame_order_bgr() const { return false; }

    // True when present() blocks until the display's vblank.  The main loop
    // then skips its own sleep/spin pacing so the two timers cannot beat.
    virtual bool presents_synced() const { return false; }

    // Presentation scaler: 0 integer (largest whole multiple), 1 fractional
    // nearest, 2 vertical bilinear, 3 full bilinear.  Backends that scale
    // through the GPU apply the same fit, so mouse/touch mapping stays right.
    virtual void set_scale_mode(int mode) { (void)mode; }

    virtual void set_title(const char* title) = 0;
    virtual void shutdown() = 0;

    // Map a normalized touch position to the 320x200 game frame, honouring
    // letterboxing and integer scaling.  SDL backends receive positions that
    // SDL's renderer event filter has already normalized to the logical
    // viewport; native backends (Vita) pass panel-normalized positions.
    // Returns false when the backend has no mapping.
    virtual bool touch_to_frame(float nx, float ny, int& x, int& y) {
        (void)nx;
        (void)ny;
        (void)x;
        (void)y;
        return false;
    }

    // Map mouse coordinates to the 320x200 game frame.  With an SDL renderer
    // and a logical size the event coordinates are already frame pixels;
    // other backends keep the no-op.
    virtual bool window_to_frame(int wx, int wy, int& x, int& y) {
        (void)wx;
        (void)wy;
        (void)x;
        (void)y;
        return false;
    }

    // Confine the pointer to the window while the editor is open.  Backends
    // without a desktop pointer ignore this.
    virtual void set_mouse_grab(bool on) { (void)on; }
    // Recompute the pointer-confine rectangle after a window/resolution change.
    virtual void refresh_mouse_rect() {}

    // Poll a native single-finger touch (Vita).  Returns true when the backend
    // implements touch polling, with the current frame position and press
    // state.  SDL_FINGER* targets use touch_to_frame instead.
    virtual bool poll_touch(int& x, int& y, bool& down) {
        (void)x;
        (void)y;
        (void)down;
        return false;
    }
};

// Implemented by screen_sdl.cpp (desktop / emulator fallback) and
// port/ps4/screen_gl.cpp (real PS4, OpenGL ES via Piglet).
std::unique_ptr<Screen> create_screen_sdl();
std::unique_ptr<Screen> create_screen_gl();
// PS4 native sceVideoOut presentation (triple buffered).
std::unique_ptr<Screen> create_screen_videoout();
// Vita libvita2d presentation (hardware GXM texture blit).
std::unique_ptr<Screen> create_screen_vita2d();
