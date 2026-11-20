#include "fmopl_opl.h"

// fmopl.h selects the YM3812 build from this macro, and fmopl.c defines it
// before including its own header.  The wrapper has to do the same.
#define HAS_YM3812 1
extern "C" {
#include "third_party/fmopl.h"
}

void FmOplDeleter::operator()(fm_opl_f* chip) const {
    if (chip) {
        OPLDestroy(chip);
    }
}

// The YM3812 runs from the PC's 3.579545 MHz master clock; fmopl derives its
// sample increments from clock/72, so the real chip clock keeps the pitch
// identical to the original `_sbfm_*` driver.
namespace {
constexpr int kOplClock = 3579545;
constexpr float kGain = 0.70f / 32768.0f;  // match the Nuked OPL3 level
}  // namespace

FmOpl::FmOpl(int sample_rate) : rate_(sample_rate > 0 ? sample_rate : 48000) {
    chip_.reset(OPLCreate(OPL_TYPE_YM3812, kOplClock, rate_));
    scratch_.resize(512);
}

void FmOpl::reset() {
    if (chip_) {
        OPLResetChip(chip_.get());
    }
}

void FmOpl::write(uint8_t reg, uint8_t value) {
    if (!chip_) {
        return;
    }
    OPLWrite(chip_.get(), 0, reg);
    OPLWrite(chip_.get(), 1, value);
}

void FmOpl::generate(float* out, int frames) {
    if (!chip_ || frames <= 0) {
        return;
    }
    if (static_cast<int>(scratch_.size()) < frames) {
        scratch_.resize(static_cast<size_t>(frames));
    }
    YM3812UpdateOne(chip_.get(), scratch_.data(), frames);
    for (int i = 0; i < frames; ++i) {
        out[i] += static_cast<float>(scratch_[i]) * kGain;
    }
}
