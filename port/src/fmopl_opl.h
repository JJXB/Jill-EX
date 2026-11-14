#pragma once
#include <cstdint>
#include <memory>
#include <vector>

struct fm_opl_f;

// Deletes an fmopl chip instance.  Defined in fmopl_opl.cpp, where the C
// header is visible.
struct FmOplDeleter {
    void operator()(fm_opl_f* chip) const;
};

// Wrapper around fmopl 0.37a, the AdPlug fork of Tatsuyuki Satoh's YM3812
// emulator relicensed under the LGPL-2.1 (see third_party/fmopl.c).  It
// exposes the same reset/write/generate surface as the built-in Opl2 core so
// the CMF sequencer can select it at runtime.
class FmOpl {
public:
    explicit FmOpl(int sample_rate = 48000);
    FmOpl(FmOpl&&) noexcept = default;
    FmOpl& operator=(FmOpl&&) noexcept = default;

    void reset();
    void write(uint8_t reg, uint8_t value);
    // Renders `frames` mono samples in about [-1, 1] and *adds* them to out.
    void generate(float* out, int frames);
    int sample_rate() const { return rate_; }

private:
    int rate_;
    std::unique_ptr<fm_opl_f, FmOplDeleter> chip_;
    std::vector<int16_t> scratch_;
};
