#include "opl2.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

// Register offset (0x00..0x15) -> internal operator slot (0..17).  The OPL
// operator layout groups the first three channels, then the next three, then
// the last three; the carrier sits three registers above its modulator.
int reg_to_slot(int off) {
    if (off >= 0x00 && off <= 0x05) {
        return off;
    }
    if (off >= 0x08 && off <= 0x0D) {
        return off - 0x08 + 6;
    }
    if (off >= 0x10 && off <= 0x15) {
        return off - 0x10 + 12;
    }
    return -1;
}

double mult_factor(int m) { return m == 0 ? 0.5 : static_cast<double>(m); }

}  // namespace

// channel -> {modulator slot, carrier slot}
const int Opl2::kChannelSlot[9][2] = {
    {0, 3},   {1, 4},   {2, 5},
    {6, 9},   {7, 10},  {8, 11},
    {12, 15}, {13, 16}, {14, 17},
};

Opl2::Opl2(int sample_rate) : rate_(sample_rate > 0 ? sample_rate : 49716) {
    reset();
}

void Opl2::reset() {
    for (unsigned& r : regs_) {
        r = 0;
    }
    for (Operator& o : ops_) {
        o = Operator();
    }
    for (Channel& c : ch_) {
        c = Channel();
    }
    // Rhythm voices: hi-hat, snare and top cymbal are noise; the others are
    // pitched.  Slot 14 (tom) stays a normal sine.
    ops_[13].rhythm = true;
    ops_[16].rhythm = true;
    ops_[17].rhythm = true;
    // Rhythm-mode envelope: none of the five percussion voices sustain.  The
    // chip decays them to silence even though the instrument's SL field is not
    // 15; without this the hi-hat/cymbal sat at ~3 dB and hissed forever.
    for (int slot = 12; slot <= 17; ++slot) {
        ops_[slot].perc = true;
    }
    rhythm_ = am_depth_ = vib_depth_ = false;
    lfo_ = vib_ = 0.0;
    noise_ = 1;
    noise_val_ = 0.0;
}

void Opl2::write(uint8_t reg, uint8_t value) {
    if (reg == 0xBD) {
        const uint8_t old = regs_[0xBD];
        regs_[0xBD] = value;
        rhythm_ = (value & 0x20) != 0;
        am_depth_ = (value & 0x80) != 0;
        vib_depth_ = (value & 0x40) != 0;
        if (!rhythm_) {
            return;
        }
        // Bits 4..0 key the five rhythm voices: BD, SD, TOM, TC, HH.
        struct Voice {
            int mask;
            int slot;
        };
        static const Voice kVoices[] = {
            {0x10, 12}, {0x10, 15},  // bass drum: both halves
            {0x08, 16},              // snare: channel 7 carrier
            {0x04, 14},              // tom: channel 8 modulator
            {0x02, 17},              // top cymbal: channel 8 carrier
            {0x01, 13},              // hi-hat: channel 7 modulator
        };
        for (const Voice& v : kVoices) {
            const bool now = (value & v.mask) != 0;
            const bool was = (old & v.mask) != 0;
            if (now != was) {
                key_on(ops_[v.slot], now);
            }
        }
        return;
    }
    regs_[reg] = value;

    if (reg >= 0x20 && reg <= 0x35) {
        const int slot = reg_to_slot(reg - 0x20);
        if (slot < 0) {
            return;
        }
        Operator& o = ops_[slot];
        o.am = (value & 0x80) != 0;
        o.vib = (value & 0x40) != 0;
        o.egt = (value & 0x20) != 0;
        o.ksr = (value & 0x10) != 0;
        o.mult = value & 0x0F;
        return;
    }
    if (reg >= 0x40 && reg <= 0x55) {
        const int slot = reg_to_slot(reg - 0x40);
        if (slot < 0) {
            return;
        }
        ops_[slot].ksl = (value >> 6) & 3;
        ops_[slot].tl = value & 0x3F;
        return;
    }
    if (reg >= 0x60 && reg <= 0x75) {
        const int slot = reg_to_slot(reg - 0x60);
        if (slot < 0) {
            return;
        }
        ops_[slot].ar = (value >> 4) & 0x0F;
        ops_[slot].dr = value & 0x0F;
        return;
    }
    if (reg >= 0x80 && reg <= 0x95) {
        const int slot = reg_to_slot(reg - 0x80);
        if (slot < 0) {
            return;
        }
        ops_[slot].sl = (value >> 4) & 0x0F;
        ops_[slot].rr = value & 0x0F;
        return;
    }
    if (reg >= 0xE0 && reg <= 0xF5) {
        const int slot = reg_to_slot(reg - 0xE0);
        if (slot >= 0) {
            ops_[slot].wave = value & 3;
        }
        return;
    }
    if (reg >= 0xA0 && reg <= 0xA8) {
        Channel& c = ch_[reg - 0xA0];
        c.fnum = (c.fnum & 0x300) | value;
        return;
    }
    if (reg >= 0xB0 && reg <= 0xB8) {
        const int idx = reg - 0xB0;
        Channel& c = ch_[idx];
        c.fnum = (c.fnum & 0xFF) | ((value & 3) << 8);
        c.block = (value >> 2) & 7;
        const bool on = (value & 0x20) != 0;
        // In rhythm mode the last three channels are keyed from 0xBD.
        if (!rhythm_ || idx < 6) {
            if (on != c.keyon) {
                c.keyon = on;
                key_on(ops_[kChannelSlot[idx][0]], on);
                key_on(ops_[kChannelSlot[idx][1]], on);
            }
        }
        return;
    }
    if (reg >= 0xC0 && reg <= 0xC8) {
        Channel& c = ch_[reg - 0xC0];
        c.fb = (value >> 1) & 7;
        c.cnt = value & 1;
    }
}

void Opl2::key_on(Operator& op, bool on) {
    if (on) {
        op.key = true;
        if (op.eg_state == 0) {
            op.eg_db = 96.0;  // start from silence
        }
        op.eg_state = 1;  // attack
    } else {
        op.key = false;
        op.eg_state = 4;  // release
    }
}

// Approximate OPL decay/release speed: attenuation is linear in dB, and each
// rate step halves the time taken to cross 96 dB.  Rate 0 is effectively a
// very slow fade (the chip treats it as "never").
double Opl2::rate_db_per_sec(int rate4) {
    // Seconds to cross 96 dB.  Rate 0 is effectively "hold" on the chip, so
    // it is made very slow rather than truly infinite.
    static const double kTime96[16] = {
        60.0,    12.0,    6.0,     3.0,     1.5,     0.75,    0.375,
        0.1875,  0.0938,  0.0469,  0.0234,  0.0117,  0.00586, 0.00293,
        0.00146, 0.00073};
    return 96.0 / kTime96[rate4 & 0x0F];
}

double Opl2::rate_attack_db_per_sec(int rate4) {
    return 2.0 * rate_db_per_sec(rate4);
}

void Opl2::update_envelope(Operator& op, int block) {
    if (op.eg_state == 0) {
        return;
    }
    const int shift = op.ksr ? (block >> 1) : 0;
    switch (op.eg_state) {
        case 1: {  // attack (attenuation falls to 0)
            const int r = std::min(15, op.ar + shift);
            op.eg_db -= rate_attack_db_per_sec(r) / rate_;
            if (op.eg_db <= 0.0) {
                op.eg_db = 0.0;
                op.eg_state = 2;
            }
            break;
        }
        case 2: {  // decay to the sustain level
            const int r = std::min(15, op.dr + shift);
            op.eg_db += rate_db_per_sec(r) / rate_;
            const bool perc = rhythm_ && op.perc;
            const double sustain =
                perc ? 96.0 : (op.sl == 15 ? 93.0
                                           : static_cast<double>(op.sl) * 3.0);
            if (op.eg_db >= sustain) {
                op.eg_db = sustain;
                // Percussion voices have no sustain phase: they decay to
                // silence, then switch off.
                op.eg_state = perc ? 0 : (op.egt ? 5 : 3);
            }
            break;
        }
        case 5: {  // sustaining tone: keep decaying at the decay rate
            const int r = std::min(15, op.dr + shift);
            op.eg_db += rate_db_per_sec(r) / rate_;
            if (op.eg_db >= 96.0) {
                op.eg_db = 96.0;
                op.eg_state = 0;
            }
            break;
        }
        case 4: {  // release
            const int r = std::min(15, op.rr + shift);
            op.eg_db += rate_db_per_sec(r) / rate_;
            if (op.eg_db >= 96.0) {
                op.eg_db = 96.0;
                op.eg_state = 0;
            }
            break;
        }
        default:
            break;
    }
}

double Opl2::wave_at(int wave, double phase) const {
    phase -= std::floor(phase);
    const double s = std::sin(kTwoPi * phase);
    switch (wave & 3) {
        case 1:
            return s > 0.0 ? s : 0.0;  // half sine
        case 2:
            return std::fabs(s);  // absolute sine
        case 3: {                 // pulse sine (quarter-wave gate)
            const double q = phase - std::floor(phase * 2.0) * 0.5;
            return q < 0.25 ? std::fabs(s) : -std::fabs(s);
        }
        default:
            return s;
    }
}

double Opl2::op_sample(int slot, double mod_phase) {
    Operator& o = ops_[slot];
    double p = o.phase + mod_phase;
    // Noise is only for the rhythm voices while rhythm mode is enabled.  With
    // rhythm mode off, slots 13/16/17 are ordinary melodic operators; using
    // the LFSR there produced a constant hiss on channels 7-9.
    double s = (rhythm_ && o.rhythm) ? noise_val_ : wave_at(o.wave, p);

    double att = o.eg_db + static_cast<double>(o.tl) * 0.75;
    static const double kKsl[4] = {0.0, 3.0, 1.5, 6.0};
    att += kKsl[o.ksl & 3];
    if (o.am) {
        att += std::sin(kTwoPi * lfo_) *
               (am_depth_ ? 4.8 : 1.0);
    }
    if (att <= 0.0) {
        return s * 1.0;
    }
    if (att >= 96.0) {
        return 0.0;
    }
    return s * std::pow(10.0, -att / 20.0);
}

void Opl2::generate(float* out, int frames) {
    for (int i = 0; i < frames; ++i) {
        lfo_ += 3.7 / rate_;
        if (lfo_ >= 1.0) {
            lfo_ -= 1.0;
        }
        vib_ += 6.1 / rate_;
        if (vib_ >= 1.0) {
            vib_ -= 1.0;
        }
        noise_ = noise_ * 1103515245u + 12345u;
        noise_val_ = (noise_ & 0x80000000u) ? 1.0 : -1.0;

        double mix = 0.0;
        const double vib_scale =
            std::sin(kTwoPi * vib_) * (vib_depth_ ? 14.0 : 7.0) / 1200.0;

        for (int ch = 0; ch < 9; ++ch) {
            Channel& c = ch_[ch];
            const int sm = kChannelSlot[ch][0];
            const int sc = kChannelSlot[ch][1];
            Operator& m = ops_[sm];
            Operator& car = ops_[sc];

            update_envelope(m, c.block);
            update_envelope(car, c.block);

            const double base = c.fnum * (1 << c.block) / 1048576.0;
            double m_inc = base * mult_factor(m.mult);
            double c_inc = base * mult_factor(car.mult);
            if (m.vib && vib_scale != 0.0) {
                m_inc *= 1.0 + vib_scale;
            }
            if (car.vib && vib_scale != 0.0) {
                c_inc *= 1.0 + vib_scale;
            }
            m.phase += m_inc;
            car.phase += c_inc;
            m.phase -= std::floor(m.phase);
            car.phase -= std::floor(car.phase);

            if (rhythm_ && ch >= 7) {
                // Hi-hat/snare (channel 7) and tom/cymbal (channel 8) are
                // independent voices, not an FM pair.
                mix += op_sample(sm, 0.0) + op_sample(sc, 0.0);
                continue;
            }

            const double fb =
                c.fb ? static_cast<double>(1 << (c.fb - 1)) / 16.0 : 0.0;
            const double m_out = op_sample(sm, m.prev * fb);
            m.prev = m_out;

            double c_out;
            if (c.cnt != 0) {
                c_out = m_out + op_sample(sc, 0.0);  // additive
            } else {
                c_out = op_sample(sc, m_out);  // frequency modulation
            }
            mix += c_out;
        }

        // Headroom for 9 channels; tanh keeps the odd loud chord from clipping.
        mix *= 0.30;
        mix = std::tanh(mix);
        out[i] += static_cast<float>(mix);
    }
}
