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

    // Upload one 0xAARRGGBB frame and present it.
    virtual void present(const uint32_t* argb, int w, int h) = 0;

    virtual void set_title(const char* title) = 0;
    virtual void shutdown() = 0;

    // Map a normalized touch position (0..1 of the window) to the 320x200 game
    // frame, honouring letterboxing and integer scaling.  Returns false when
    // the touch is outside the frame or the backend has no window mapping.
    // Used by the optional on-screen controls (Android); other targets keep the
    // default no-op.
    virtual bool touch_to_frame(float nx, float ny, int& x, int& y) {
        (void)nx;
        (void)ny;
        (void)x;
        (void)y;
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
