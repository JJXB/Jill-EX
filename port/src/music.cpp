#include "music.h"

#include <algorithm>
#include <cmath>

namespace {

// OPL register offset of the modulator for each channel.  The carrier is
// always three registers above it.
const int kModOff[9] = {0x00, 0x01, 0x02, 0x08, 0x09,
                        0x0A, 0x10, 0x11, 0x12};

// The SBFMDRV driver programmes every melodic channel with this patch at
// start-up, before a song has loaded its own instruments.
const uint8_t kInitInstrument[11] = {0x01, 0x11, 0x4F, 0x00, 0xF1,
                                     0xF2, 0x53, 0x74, 0x00, 0x00,
                                     0x08};

// Native OPL2 output rate (3.579545 MHz / 72).  Nuked and fmopl both generate
// pitch from this rate and resample to the device rate, so F-numbers must be
// derived from it rather than from the output rate.
constexpr double kOpl2Rate = 3579545.0 / 72.0;

// SBFMDRV folds notes below C0 up an octave and caps the top octave (notes
// 108..119 repeat 96..107, 120..127 sit on B7).
int fold_note(int note) {
    if (note < 12) {
        return note + 12;
    }
    if (note >= 120) {
        return 119;
    }
    if (note >= 108) {
        return note - 12;
    }
    return note;
}

// SBFMDRV's velocity scaling: velocity multiplies the patch's base output
// level, and the KSL bits from the instrument byte are preserved.
uint8_t carrier_level(uint8_t scaling, int velocity) {
    const int base = 63 - (scaling & 0x3F);
    const int level = 63 - ((((velocity & 0x7F) | 0x80) * base) >> 8);
    return static_cast<uint8_t>((scaling & 0xC0) | (level & 0x3F));
}

}  // namespace

Music::Music(int sample_rate)
    : nuked_(sample_rate), fmopl_(sample_rate) {
    reset();
}

const char* Music::opl_core_name(OplCore core) {
    switch (core) {
        case OplCore::Fmopl:
            return "FMOPL";
        default:
            return "NUKED OPL3";
    }
}

void Music::set_opl_core(OplCore core) {
    if (core_ == core) {
        return;
    }
    core_ = core;
    reset_cores();
}

void Music::reset_cores() {
    nuked_.reset();
    fmopl_.reset();
}

void Music::write_reg(uint8_t reg, uint8_t value) {
    switch (core_) {
        case OplCore::Fmopl:
            fmopl_.write(reg, value);
            break;
        default:
            nuked_.write(reg, value);
            break;
    }
}

int Music::core_rate() const {
    switch (core_) {
        case OplCore::Fmopl:
            return fmopl_.sample_rate();
        default:
            return nuked_.sample_rate();
    }
}

double Music::note_rate() const {
    // Both cores run at the chip's native rate internally and resample to the
    // output rate.
    return kOpl2Rate;
}

void Music::core_generate(float* out, int frames) {
    switch (core_) {
        case OplCore::Fmopl:
            fmopl_.generate(out, frames);
            break;
        default:
            nuked_.generate(out, frames);
            break;
    }
}

void Music::reset() {
    reset_cores();
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
        transpose_[i] = 0.0;
        percussion_notes_[i] = 0;
        note_playing_[i] = -1;
        note_fix_[i] = false;
        note_fix_vel_[i] = 0;
    }
    for (int i = 0; i < 9; ++i) {
        voice_[i] = Voice();
        fnum_[i] = 0;
        block_[i] = 0;
    }
    voice_age_ = 0;
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
        static_cast<double>(song_->ticks_per_second) / core_rate();
    // Match the SBFMDRV reset sequence: waveform select on, CSM/SEL off, the
    // default patch on every channel, and AM+VIB depth enabled.
    driver_reset();
}

void Music::stop() {
    playing_ = false;
    reset_cores();
    bd_ = 0;
    rhythm_ = false;
    for (int i = 0; i < 9; ++i) {
        voice_[i] = Voice();
    }
}

void Music::driver_reset() {
    write_reg(0x01, 0x20);  // enable waveform select (OPL2)
    write_reg(0x08, 0x00);  // CSM / note-select off
    for (int i = 0; i < 9; ++i) {
        write_instrument(i, kInitInstrument);
        voice_[i] = Voice();
        fnum_[i] = 0;
        block_[i] = 0;
    }
    for (int i = 0; i < 16; ++i) {
        program_[i] = 0;
        percussion_notes_[i] = 0;
        percussion_loaded_[i] = -1;
        note_playing_[i] = -1;
        note_fix_[i] = false;
    }
    voice_age_ = 0;
    bd_ = 0;
    depth_ = 3;
    write_rhythm_mode();
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
        if (next_event_ >= events.size() && tick_pos_ >= end_tick_) {
            // The original driver restarts the track from the top; cut any
            // sounding voices and clear per-channel transpose before replaying
            // the tick-0 controllers/program changes.
            tick_pos_ -= static_cast<double>(end_tick_);
            if (tick_pos_ < 0.0) {
                tick_pos_ = 0.0;
            }
            for (int v = 0; v < 9; ++v) {
                if (voice_[v].active) {
                    key_off(v);
                }
                voice_[v] = Voice();
            }
            for (int ch = 0; ch < 16; ++ch) {
                transpose_[ch] = 0.0;
                note_playing_[ch] = -1;
                note_fix_[ch] = false;
                percussion_notes_[ch] = 0;
            }
            bd_ = 0;
            write_rhythm_mode();
            next_event_ = 0;
        }
        core_generate(out + i, 1);
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
    if (chan < 0 || chan >= 16 || !song_ || song_->instrument_count <= 0) {
        return;
    }
    // SBFMDRV reduces the patch number modulo the file's instrument count.
    program = program % song_->instrument_count;
    if (program < 0) {
        program = 0;
    }
    program_[chan] = program;
    if (rhythm_ && chan >= 11) {
        // SBFMDRV loads a percussion patch on the program change itself, not
        // per note; reloading the ADSR/TL registers on every hit alters the
        // sounding envelope.
        load_percussion(chan);
        return;
    }
    const int limit = rhythm_ ? 6 : 9;
    for (int i = 0; i < limit; ++i) {
        if (voice_[i].owner != chan) {
            continue;
        }
        if (voice_[i].active) {
            load_melodic(i, program);
            voice_[i].patch = program;
        } else {
            voice_[i].owner = -1;  // parked voice: reload on next use
        }
    }
}

void Music::write_instrument(int opl_ch, const uint8_t* ins) {
    const int mo = kModOff[opl_ch];
    const int co = mo + 3;
    write_reg(static_cast<uint8_t>(0x20 + mo), ins[0]);
    write_reg(static_cast<uint8_t>(0x20 + co), ins[1]);
    write_reg(static_cast<uint8_t>(0x40 + mo), ins[2]);
    write_reg(static_cast<uint8_t>(0x40 + co), ins[3]);
    write_reg(static_cast<uint8_t>(0x60 + mo), ins[4]);
    write_reg(static_cast<uint8_t>(0x60 + co), ins[5]);
    write_reg(static_cast<uint8_t>(0x80 + mo), ins[6]);
    write_reg(static_cast<uint8_t>(0x80 + co), ins[7]);
    write_reg(static_cast<uint8_t>(0xE0 + mo), ins[8]);
    write_reg(static_cast<uint8_t>(0xE0 + co), ins[9]);
    write_reg(static_cast<uint8_t>(0xC0 + opl_ch), ins[10]);
}

void Music::load_melodic(int opl_ch, int instrument) {
    if (!song_ || instrument < 0 || instrument >= song_->instrument_count) {
        return;
    }
    write_instrument(opl_ch, song_->instruments[instrument]);
}

void Music::load_operator(int op_off, const uint8_t* ins, bool carrier_half) {
    const int k = carrier_half ? 1 : 0;
    write_reg(static_cast<uint8_t>(0x20 + op_off), ins[k ? 1 : 0]);
    write_reg(static_cast<uint8_t>(0x40 + op_off), ins[k ? 3 : 2]);
    write_reg(static_cast<uint8_t>(0x60 + op_off), ins[k ? 5 : 4]);
    write_reg(static_cast<uint8_t>(0x80 + op_off), ins[k ? 7 : 6]);
    write_reg(static_cast<uint8_t>(0xE0 + op_off), ins[k ? 9 : 8]);
}

void Music::load_percussion(int chan) {
    const int prog = program_[chan];
    if (!song_ || prog < 0 || prog >= song_->instrument_count) {
        return;
    }
    const uint8_t* ins = song_->instruments[prog];
    int c0ch = 7;
    switch (chan) {
        case 11:  // bass drum: both halves of OPL channel 6
            load_operator(0x10, ins, false);
            load_operator(0x13, ins, true);
            c0ch = 6;
            break;
        case 12:  // snare: channel 7 carrier
            load_operator(0x14, ins, false);
            c0ch = 7;
            break;
        case 13:  // tom: channel 8 modulator
            load_operator(0x12, ins, false);
            c0ch = 8;
            break;
        case 14:  // top cymbal: channel 8 carrier
            load_operator(0x15, ins, false);
            c0ch = 8;
            break;
        default:  // hi-hat: channel 7 modulator
            load_operator(0x11, ins, false);
            c0ch = 7;
            break;
    }
    // The bass drum keeps the file's feedback/connection byte; the four
    // single-operator drums are written with additive connection forced on.
    write_reg(static_cast<uint8_t>(0xC0 + c0ch),
              static_cast<uint8_t>(chan == 11 ? ins[10] : (ins[10] | 1)));
    percussion_loaded_[chan] = prog;
}

void Music::set_carrier_volume(int opl_ch, int instrument, int velocity) {
    if (!song_ || instrument < 0 || instrument >= song_->instrument_count) {
        return;
    }
    const uint8_t* ins = song_->instruments[instrument];
    const int car = kModOff[opl_ch] + 3;
    write_reg(static_cast<uint8_t>(0x40 + car),
              carrier_level(ins[3], velocity));
}

// Writes the pitch registers.  `key_on` sets the B0 key bit for a sounding
// note; percussion and key-off writes leave it clear.
void Music::set_frequency(int opl_ch, double note, bool key_on) {
    if (note < 0.0) {
        note = 0.0;
    } else if (note > 127.0) {
        note = 127.0;
    }
    const double f = 440.0 * std::pow(2.0, (note - 69.0) / 12.0);
    double x = f * 1048576.0 / note_rate();
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
    write_reg(static_cast<uint8_t>(0xA0 + opl_ch),
              static_cast<uint8_t>(fnum & 0xFF));
    write_reg(static_cast<uint8_t>(0xB0 + opl_ch),
              static_cast<uint8_t>((block << 2) | ((fnum >> 8) & 3) |
                                   (key_on ? 0x20 : 0)));
}

void Music::key_on(int opl_ch) {
    write_reg(static_cast<uint8_t>(0xB0 + opl_ch),
              static_cast<uint8_t>((block_[opl_ch] << 2) |
                                   ((fnum_[opl_ch] >> 8) & 3) | 0x20));
}

void Music::key_off(int opl_ch) {
    write_reg(static_cast<uint8_t>(0xB0 + opl_ch),
              static_cast<uint8_t>((block_[opl_ch] << 2) |
                                   ((fnum_[opl_ch] >> 8) & 3)));
}

void Music::write_rhythm_mode() {
    int v = (rhythm_ ? 0x20 : 0) | ((depth_ & 1) ? 0x40 : 0) |
            ((depth_ & 2) ? 0x80 : 0);
    if (rhythm_) {
        v |= bd_ & 0x1F;
    }
    write_reg(0xBD, static_cast<uint8_t>(v));
}

int Music::allocate_voice(int chan) {
    const int limit = rhythm_ ? 6 : 9;
    // 1) a voice this channel last released.
    for (int i = 0; i < limit; ++i) {
        if (!voice_[i].active && voice_[i].owner == chan) {
            return i;
        }
    }
    // 2) a voice that has never been used.
    for (int i = 0; i < limit; ++i) {
        if (voice_[i].owner == -1) {
            return i;
        }
    }
    // 3) any released voice.
    for (int i = 0; i < limit; ++i) {
        if (!voice_[i].active) {
            return i;
        }
    }
    // 4) steal the oldest sounding voice.
    int best = 0;
    for (int i = 1; i < limit; ++i) {
        if (voice_[i].age < voice_[best].age) {
            best = i;
        }
    }
    key_off(best);
    voice_[best].active = false;
    return best;
}

void Music::note_on(int chan, int note, int velocity) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    if (rhythm_ && chan >= 11 && chan <= 15) {
        // SBFMDRV's percussion voices are 6..10 in its own table.  The pitch
        // write goes to 0xA0+voice/0xB0+voice, so only voices 6..8 (bass drum,
        // snare, tom) reach a real OPL channel; top cymbal (voice 9) and
        // hi-hat (voice 10) land on 0xA9/0xAA, which the chip ignores.  Those
        // two therefore keep whatever pitch channel 7/8 already had.
        int opl_ch = 6;
        int bit = 0;
        switch (chan) {
            case 11: opl_ch = 6; bit = 0x10; break;
            case 12: opl_ch = 7; bit = 0x08; break;
            case 13: opl_ch = 8; bit = 0x04; break;
            case 14: opl_ch = -1; bit = 0x02; break;
            default: opl_ch = -1; bit = 0x01; break;
        }
        if (opl_ch >= 0) {
            set_frequency(opl_ch, fold_note(note) + transpose_[chan], false);
        }
        if (percussion_loaded_[chan] != program_[chan]) {
            load_percussion(chan);
        }
        if (song_) {
            const int prog = program_[chan];
            if (prog >= 0 && prog < song_->instrument_count) {
                const uint8_t* ins = song_->instruments[prog];
                const int target = chan == 11 ? 0x13
                                   : (chan == 12) ? 0x14
                                   : (chan == 13) ? 0x12
                                   : (chan == 14) ? 0x15
                                                  : 0x11;
                uint8_t level = carrier_level(ins[3], velocity);
                if (chan == 15) {
                    // The hi-hat sits well under the mix with the chip's TL
                    // scale; lift it 6 dB (8 TL steps) so the taps read
                    // clearly without touching the other drums.
                    const int tl = std::max(0, (level & 0x3F) - 8);
                    level = static_cast<uint8_t>((level & 0xC0) | tl);
                }
                write_reg(static_cast<uint8_t>(0x40 + target), level);
            }
        }
        percussion_notes_[chan] = note;
        // The driver only sets the key bit; the preceding note-off clears it.
        bd_ |= bit;
        write_rhythm_mode();
        return;
    }

    if (rhythm_ && chan >= 6 && chan <= 8) {
        return;  // those OPL channels are percussion while rhythm mode is on
    }

    if (note_playing_[chan] == note) {
        // Repeated note-on without an intervening note-off.  Release the
        // sounding note now and retrigger it at the following note-off; the
        // original library and AdPlug use this rule for same-tick reties.
        release_note(chan, note);
        note_playing_[chan] = -1;
        note_fix_[chan] = true;
        note_fix_vel_[chan] = velocity;
        return;
    }
    note_fix_[chan] = false;
    note_playing_[chan] = note;
    start_note(chan, note, velocity);
}

void Music::start_note(int chan, int note, int velocity) {
    const double pitch = fold_note(note) + transpose_[chan];
    const int opl = allocate_voice(chan);
    Voice& v = voice_[opl];
    const int patch = program_[chan];
    if (v.owner != chan || v.patch != patch) {
        load_melodic(opl, patch);
        v.patch = patch;
    }
    set_frequency(opl, pitch, false);
    set_carrier_volume(opl, patch, velocity);
    key_on(opl);
    v.owner = chan;
    v.note = note;
    v.active = true;
    v.age = ++voice_age_;
}

void Music::release_note(int chan, int note) {
    const int limit = rhythm_ ? 6 : 9;
    for (int i = 0; i < limit; ++i) {
        if (voice_[i].owner == chan && voice_[i].note == note &&
            voice_[i].active) {
            key_off(i);
            voice_[i].active = false;
        }
    }
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
    if (rhythm_ && chan >= 6 && chan <= 8) {
        return;
    }
    if (note_fix_[chan]) {
        // The retrigger deferred by a duplicate note-on (see note_on).
        note_fix_[chan] = false;
        note_playing_[chan] = note;
        start_note(chan, note, note_fix_vel_[chan]);
        return;
    }
    release_note(chan, note);
    if (note_playing_[chan] == note) {
        note_playing_[chan] = -1;
    }
}

void Music::update_transpose(int chan) {
    const int limit = rhythm_ ? 6 : 9;
    for (int i = 0; i < limit; ++i) {
        if (voice_[i].active && voice_[i].owner == chan) {
            set_frequency(i, fold_note(voice_[i].note) + transpose_[chan], true);
        }
    }
    // SBFMDRV's transpose only walks its six melodic voices; percussion is
    // left alone.
}

void Music::controller(int chan, int cc, int value) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    switch (cc) {
        case 0x63:  // AM/VIB depth extension
            depth_ = value & 3;
            write_rhythm_mode();
            break;
        case 0x67: {  // melody <-> rhythm mode
            const bool on = value != 0;
            if (on != rhythm_) {
                rhythm_ = on;
                driver_reset();  // SBFMDRV performs a full reset here
            }
            break;
        }
        case 0x68:  // transpose up, 1/256 semitone per step
            transpose_[chan] = value / 256.0;
            update_transpose(chan);
            break;
        case 0x69:  // transpose down
            transpose_[chan] = -value / 256.0;
            update_transpose(chan);
            break;
        default:
            break;
    }
}

