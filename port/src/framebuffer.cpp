#include "framebuffer.h"

#include "palette.inc"

void vga_default_palette(uint8_t rgb[768]) {
    // The shipped games load this palette from the executable image into
    // DGROUP:0x194 and program the DAC from it (FUN_1123_0791).  It is a
    // 6x6x6-ish colour cube plus the EGA colours and a grey ramp, not the
    // standard VGA ramp.
    for (int i = 0; i < 768; ++i) {
        rgb[i] = static_cast<uint8_t>(kDefaultPalette[i] * 255 / 63);
    }
}
