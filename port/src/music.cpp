#include "music.h"

#include <algorithm>
#include <cmath>

namespace {

// OPL register offset of the modulator for each channel.  The carrier is
// always three registers above it.
const int kModOff[9] = {0x00, 0x01, 0x02, 0x08, 0x09,
                        0x0A, 0x10, 0x11, 0x12};

}  // namespace

Music::Music(int sample_rate) : opl_(sample_rate) { reset(); }

void Music::reset() {
    opl_.reset();
    song_.reset();
    playing_ = false;
    next_event_ = 0;
    tick_pos_ = 0.0;
    ticks_per_sample_ = 0.0;
    end_tick_ = 0;
    rhythm_ = false;
    depth_ = 3;
    bd_ = 0;
    for (int i = 0; i < 16; ++i) {
        program_[i] = 0;
        volume_[i] = 127;
        transpose_[i] = 0.0;
        active_note_[i] = -1;
        active_opl_[i] = -1;
        last_velocity_[i] = 0;
        percussion_notes_[i] = 0;
    }
    for (int i = 0; i < 9; ++i) {
        fnum_[i] = 0;
        block_[i] = 0;
    }
}

void Music::start(std::shared_ptr<const CmfSong> song) {
    reset();
    song_ = std::move(song);
    if (!song_) {
        return;
    }
    playing_ = true;
    end_tick_ = std::max<uint32_t>(1, song_->end_tick);
    ticks_per_sample_ =
        static_cast<double>(song_->ticks_per_second) / opl_.sample_rate();
}

void Music::stop() {
    playing_ = false;
    opl_.reset();
    bd_ = 0;
    rhythm_ = false;
}

const std::string& Music::title() const {
    static const std::string kEmpty;
    return song_ ? song_->title : kEmpty;
}

void Music::render(float* out, int frames) {
    if (!playing_ || !song_) {
        return;
    }
    const auto& events = song_->events;
    for (int i = 0; i < frames; ++i) {
        tick_pos_ += ticks_per_sample_;
        while (next_event_ < events.size() &&
               events[next_event_].tick <= tick_pos_) {
            handle_event(events[next_event_]);
            ++next_event_;
        }
        if (next_event_ >= events.size()) {
            // The original driver loops level music; wrap while keeping the
            // fractional tick so the rhythm stays steady.
            tick_pos_ -= static_cast<double>(end_tick_);
            if (tick_pos_ < 0.0) {
                tick_pos_ = 0.0;
            }
            next_event_ = 0;
        }
        opl_.generate(out + i, 1);
    }
}

void Music::handle_event(const CmfEvent& e) {
    switch (e.type) {
        case 0x90:
            note_on(e.chan, e.a, e.b);
            break;
        case 0x80:
            note_off(e.chan, e.a);
            break;
        case 0xB0:
            controller(e.chan, e.a, e.b);
            break;
        case 0xC0:
            program_change(e.chan, e.a);
            break;
        default:
            break;
    }
}

void Music::program_change(int chan, int program) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    program_[chan] = static_cast<uint8_t>(program & 0x7F);
    if (rhythm_ && chan >= 11) {
        return;  // loaded by the next percussion note-on
    }
    const int opl = chan < 9 ? chan : chan % 9;
    load_melodic(opl, program_[chan]);
}

void Music::load_melodic(int opl_ch, int instrument) {
    if (!song_ || instrument < 0 || instrument >= song_->instrument_count) {
        return;
    }
    const uint8_t* ins = song_->instruments[instrument];
    const int mo = kModOff[opl_ch];
    const int co = mo + 3;
    opl_.write(static_cast<uint8_t>(0x20 + mo), ins[0]);
    opl_.write(static_cast<uint8_t>(0x20 + co), ins[1]);
    opl_.write(static_cast<uint8_t>(0x40 + mo), ins[2]);
    opl_.write(static_cast<uint8_t>(0x40 + co), ins[3]);
    opl_.write(static_cast<uint8_t>(0x60 + mo), ins[4]);
    opl_.write(static_cast<uint8_t>(0x60 + co), ins[5]);
    opl_.write(static_cast<uint8_t>(0x80 + mo), ins[6]);
    opl_.write(static_cast<uint8_t>(0x80 + co), ins[7]);
    opl_.write(static_cast<uint8_t>(0xE0 + mo), ins[8]);
    opl_.write(static_cast<uint8_t>(0xE0 + co), ins[9]);
    opl_.write(static_cast<uint8_t>(0xC0 + opl_ch), ins[10]);
}

void Music::load_operator(int op_off, const uint8_t* ins, bool carrier_half) {
    const int k = carrier_half ? 1 : 0;
    opl_.write(static_cast<uint8_t>(0x20 + op_off), ins[k ? 1 : 0]);
    opl_.write(static_cast<uint8_t>(0x40 + op_off), ins[k ? 3 : 2]);
    opl_.write(static_cast<uint8_t>(0x60 + op_off), ins[k ? 5 : 4]);
    opl_.write(static_cast<uint8_t>(0x80 + op_off), ins[k ? 7 : 6]);
    opl_.write(static_cast<uint8_t>(0xE0 + op_off), ins[k ? 9 : 8]);
}

void Music::set_carrier_volume(int opl_ch, int midi_chan, int instrument,
                               int velocity) {
    if (!song_ || instrument < 0 || instrument >= song_->instrument_count) {
        return;
    }
    const uint8_t* ins = song_->instruments[instrument];
    int tl = (ins[3] & 0x3F) + (127 - velocity) / 2 +
             (127 - volume_[midi_chan]) / 2;
    tl = std::max(0, std::min(63, tl));
    const int car = kModOff[opl_ch] + 3;
    opl_.write(static_cast<uint8_t>(0x40 + car),
               static_cast<uint8_t>((ins[3] & 0xC0) | tl));
}

// Writes the pitch registers with the key bit *off*; the caller keys the note.
void Music::set_frequency(int opl_ch, double note) {
    const double f = 440.0 * std::pow(2.0, (note - 69.0) / 12.0);
    double x = f * 1048576.0 / opl_.sample_rate();
    int block = 0;
    while (x >= 1024.0 && block < 7) {
        x *= 0.5;
        ++block;
    }
    while (x < 256.0 && block > 0) {
        x *= 2.0;
        --block;
    }
    const int fnum = static_cast<int>(x + 0.5) & 0x3FF;
    fnum_[opl_ch] = fnum;
    block_[opl_ch] = block;
    opl_.write(static_cast<uint8_t>(0xA0 + opl_ch),
               static_cast<uint8_t>(fnum & 0xFF));
    opl_.write(static_cast<uint8_t>(0xB0 + opl_ch),
               static_cast<uint8_t>((block << 2) | ((fnum >> 8) & 3)));
}

void Music::key_on(int opl_ch) {
    opl_.write(static_cast<uint8_t>(0xB0 + opl_ch),
               static_cast<uint8_t>((block_[opl_ch] << 2) |
                                    ((fnum_[opl_ch] >> 8) & 3) | 0x20));
}

void Music::key_off(int opl_ch) {
    opl_.write(static_cast<uint8_t>(0xB0 + opl_ch),
               static_cast<uint8_t>((block_[opl_ch] << 2) |
                                    ((fnum_[opl_ch] >> 8) & 3)));
}

void Music::write_rhythm_mode() {
    int v = (rhythm_ ? 0x20 : 0) | ((depth_ & 1) ? 0x40 : 0) |
            ((depth_ & 2) ? 0x80 : 0);
    if (rhythm_) {
        v |= bd_ & 0x1F;
    }
    opl_.write(0xBD, static_cast<uint8_t>(v));
}

void Music::note_on(int chan, int note, int velocity) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    last_velocity_[chan] = velocity;
    const double pitch = note + transpose_[chan];

    if (rhythm_ && chan >= 11 && chan <= 15) {
        int opl_ch = 6;
        int op_off = -1;
        int bit = 0;
        switch (chan) {
            case 11:
                op_off = 0x00;  // both halves loaded below
                bit = 0x10;
                break;
            case 12:
                opl_ch = 7;
                op_off = 0x14;  // snare: channel 7 carrier
                bit = 0x08;
                break;
            case 13:
                opl_ch = 8;
                op_off = 0x12;  // tom: channel 8 modulator
                bit = 0x04;
                break;
            case 14:
                opl_ch = 8;
                op_off = 0x15;  // top cymbal: channel 8 carrier
                bit = 0x02;
                break;
            default:
                opl_ch = 7;
                op_off = 0x11;  // hi-hat: channel 7 modulator
                bit = 0x01;
                break;
        }
        set_frequency(opl_ch, pitch);
        const int prog = program_[chan];
        if (song_ && prog >= 0 && prog < song_->instrument_count) {
            const uint8_t* ins = song_->instruments[prog];
            if (chan == 11) {
                load_operator(0x10, ins, false);
                load_operator(0x13, ins, true);
            } else {
                load_operator(op_off, ins, false);
            }
            int tl = (chan == 11 ? ins[3] : ins[2]) & 0x3F;
            tl += (127 - velocity) / 2 + (127 - volume_[chan]) / 2;
            tl = std::max(0, std::min(63, tl));
            const int target = chan == 11 ? 0x13 : op_off;
            opl_.write(static_cast<uint8_t>(0x40 + target),
                       static_cast<uint8_t>(
                           ((chan == 11 ? ins[3] : ins[2]) & 0xC0) | tl));
        }
        percussion_notes_[chan] = note;
        bd_ &= ~bit;
        write_rhythm_mode();
        bd_ |= bit;
        write_rhythm_mode();
        return;
    }

    if (rhythm_ && chan >= 6 && chan <= 8) {
        return;  // those OPL channels are percussion while rhythm mode is on
    }
    const int opl = chan < 9 ? chan : chan % 9;
    set_frequency(opl, pitch);
    set_carrier_volume(opl, chan, program_[chan], velocity);
    active_note_[chan] = note;
    active_opl_[chan] = opl;
    key_on(opl);
}

void Music::note_off(int chan, int note) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    if (rhythm_ && chan >= 11 && chan <= 15) {
        int bit = 0;
        switch (chan) {
            case 11: bit = 0x10; break;
            case 12: bit = 0x08; break;
            case 13: bit = 0x04; break;
            case 14: bit = 0x02; break;
            default: bit = 0x01; break;
        }
        bd_ &= ~bit;
        write_rhythm_mode();
        percussion_notes_[chan] = 0;
        return;
    }
    if (active_note_[chan] != note && active_note_[chan] >= 0) {
        return;  // stale note-off for an older note
    }
    const int opl = active_opl_[chan];
    if (opl >= 0) {
        key_off(opl);
    }
    active_note_[chan] = -1;
}

void Music::controller(int chan, int cc, int value) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    switch (cc) {
        case 0x07:  // channel volume
            volume_[chan] = value;
            if (active_note_[chan] >= 0 && active_opl_[chan] >= 0 &&
                !(rhythm_ && chan >= 6)) {
                set_carrier_volume(active_opl_[chan], chan, program_[chan],
                                   last_velocity_[chan]);
            }
            break;
        case 0x63:  // AM/VIB depth extension
            depth_ = value & 3;
            write_rhythm_mode();
            break;
        case 0x67:  // melody <-> rhythm mode
            rhythm_ = value != 0;
            write_rhythm_mode();
            break;
        case 0x68:  // transpose up, 1/128 semitone
            transpose_[chan] = value / 128.0;
            break;
        case 0x69:  // transpose down
            transpose_[chan] = -value / 128.0;
            break;
        default:
            break;
    }
}
