#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

// The original renders into a 320x200 8-bit page (mode 13h).  The port keeps
// that exact framebuffer and only converts at present time, so all coordinates
// and shape blits match the DOS version.
struct Framebuffer {
    static constexpr int W = 320;
    static constexpr int H = 200;

    std::vector<uint8_t> px;

    Framebuffer() : px(static_cast<size_t>(W) * H, 0) {}

    void clear(uint8_t c = 0) { std::memset(px.data(), c, px.size()); }

    void put(int x, int y, uint8_t c) {
        if (static_cast<unsigned>(x) < static_cast<unsigned>(W) &&
            static_cast<unsigned>(y) < static_cast<unsigned>(H)) {
            px[static_cast<size_t>(y) * W + x] = c;
        }
    }

    uint8_t get(int x, int y) const {
        if (static_cast<unsigned>(x) < static_cast<unsigned>(W) &&
            static_cast<unsigned>(y) < static_cast<unsigned>(H)) {
            return px[static_cast<size_t>(y) * W + x];
        }
        return 0;
    }

    void fill_rect(int x, int y, int w, int h, uint8_t c) {
        for (int yy = y; yy < y + h; ++yy) {
            for (int xx = x; xx < x + w; ++xx) {
                put(xx, yy, c);
            }
        }
    }
};

// The shipped games use the standard VGA default palette (index -> 6-bit RGB
// ramp).  A 768-byte .pal override can replace it at run time; the real
// palette load path is not documented yet [open].
void vga_default_palette(uint8_t rgb[768]);
