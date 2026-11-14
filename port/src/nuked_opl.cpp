#include "nuked_opl.h"

NukedOpl::NukedOpl(int sample_rate) : rate_(sample_rate > 0 ? sample_rate : 48000) {
    reset();
}

void NukedOpl::reset() {
    OPL3_Reset(&chip_, static_cast<uint32_t>(rate_));
}

void NukedOpl::write(uint8_t reg, uint8_t value) {
    OPL3_WriteReg(&chip_, reg, value);
}

void NukedOpl::generate(float* out, int frames) {
    int16_t sample[2];
    for (int i = 0; i < frames; ++i) {
        OPL3_GenerateResampled(&chip_, sample);
        out[i] += (static_cast<float>(sample[0]) +
                   static_cast<float>(sample[1])) *
                  (0.65f / 32768.0f);  // 1.3x gain to match the built-in core
    }
}
