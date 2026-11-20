#pragma once
#include <cstdint>

#include "third_party/opl3.h"

// Wrapper around Nuked OPL3 (LGPL-2.1, see third_party/opl3.h) that exposes a
// small reset/write/generate surface for the CMF sequencer.  Nuked runs in
// OPL2 mode because the CMF driver only writes the low register bank; the
// stereo output is averaged to the port's mono music bus.
class NukedOpl {
public:
    explicit NukedOpl(int sample_rate = 48000);

    void reset();
    void write(uint8_t reg, uint8_t value);
    // Renders `frames` mono samples in about [-1, 1] and *adds* them to out.
    void generate(float* out, int frames);
    int sample_rate() const { return rate_; }

private:
    int rate_;
    opl3_chip chip_{};
};
