#pragma once
#include <cstdint>

// Register-level YM3812 (OPL2) software synthesiser.
//
// Nine two-operator FM channels, four waveforms, ADSR envelopes, key scaling,
// feedback/connection, vibrato/tremolo and the five rhythm voices.  It accepts
// the same register writes as the chip, so the CMF sequencer can drive it
// exactly like the original `_sbfm_*` driver.  The numeric model is tuned by
// ear against the shipped tracks; it is not a cycle-exact emulation.
class Opl2 {
public:
    explicit Opl2(int sample_rate = 49716);

    void reset();
    // Writes one chip register (the pair the original sends via 0x388/0x389).
    void write(uint8_t reg, uint8_t value);

    // Renders `frames` mono samples in about [-1, 1] and *adds* them to out.
    void generate(float* out, int frames);

    int sample_rate() const { return rate_; }

private:
    struct Operator {
        bool am = false;
        bool vib = false;
        bool egt = false;   // 1 = sustaining tone keeps decaying
        bool ksr = false;
        int mult = 1;       // frequency multiple * 2 (0 -> 0.5)
        int ksl = 0;        // key scale level bits (0..3)
        int tl = 0;         // total level (0..63, 0.75 dB steps)
        int ar = 0;         // attack rate (0..15)
        int dr = 0;         // decay rate
        int sl = 0;         // sustain level (0..15)
        int rr = 0;         // release rate
        int wave = 0;       // 0 sine, 1 half, 2 abs, 3 pulse sine
        double phase = 0.0; // 0..1
        double eg_db = 96.0;
        int eg_state = 0;   // 0 off, 1 attack, 2 decay, 3 sustain, 4 release
        bool key = false;
        double prev = 0.0;  // previous output (feedback / modulation)
        bool rhythm = false;  // percussion voice: noise instead of sine
    };

    struct Channel {
        int fnum = 0;
        int block = 0;
        int fb = 0;
        int cnt = 0;
        bool keyon = false;
    };

    double op_sample(int slot, double mod_phase);
    double wave_at(int wave, double phase) const;
    void key_on(Operator& op, bool on);
    void update_envelope(Operator& op, int block);
    static double rate_db_per_sec(int rate4);
    static double rate_attack_db_per_sec(int rate4);

    int rate_;
    unsigned regs_[256] = {};
    Operator ops_[18];
    Channel ch_[9];

    bool rhythm_ = false;
    bool am_depth_ = false;
    bool vib_depth_ = false;

    double lfo_ = 0.0;    // tremolo phase, 3.7 Hz
    double vib_ = 0.0;    // vibrato phase, 6.1 Hz
    uint32_t noise_ = 1;
    double noise_val_ = 0.0;

    static const int kChannelSlot[9][2];
};
