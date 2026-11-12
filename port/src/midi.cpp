#include "midi.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

namespace {

#ifdef _WIN32
std::string wide_to_utf8(const wchar_t* s) {
    if (!s) {
        return std::string();
    }
    const int n = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr,
                                      nullptr);
    if (n <= 1) {
        return std::string();
    }
    std::string out(static_cast<size_t>(n - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s, -1, out.data(), n, nullptr, nullptr);
    return out;
}
#endif

int gm_drum_note(int chan) {
    switch (chan) {
        case 11: return 36;  // bass drum
        case 12: return 38;  // snare
        case 13: return 45;  // low tom
        case 14: return 49;  // crash cymbal
        default: return 42;  // closed hi-hat
    }
}

// Pick a General MIDI program from an OPL2 instrument and the register the
// part actually plays in.  The CMF program number is only an index into the
// song's AdLib instrument table, so sending it to a GM synth usually selects
// an unrelated instrument.
int choose_gm_program(const uint8_t ins[11], int mean_note, int span) {
    const int ar = ins[5] >> 4;       // carrier attack rate
    const int dr = ins[5] & 0x0F;     // carrier decay rate
    const int sl = ins[7] >> 4;       // carrier sustain level
    const int rr = ins[7] & 0x0F;     // carrier release rate
    const int tl_mod = ins[2] & 0x3F;  // modulator level (lower = brighter)
    const int wave_mod = ins[8] & 3;
    const int wave_car = ins[9] & 3;
    const int feedback = (ins[10] >> 1) & 7;

    const bool bright =
        tl_mod <= 20 || feedback >= 5 || wave_mod != 0 || wave_car != 0;
    const bool very_bright =
        tl_mod <= 8 || wave_mod >= 4 || wave_car >= 4;
    const bool percussive = ar >= 14 && (sl >= 6 || rr >= 9) && dr >= 5;
    const bool plucked = ar >= 12 && (sl >= 3 || rr >= 8);
    const bool slow_attack = ar <= 8;
    const bool long_release = sl <= 3 && rr <= 6;
    const bool sustained = ar >= 10 && long_release;
    const bool organ = ar >= 12 && sl <= 2 && rr <= 5 && tl_mod >= 24 &&
                       wave_mod == 0 && wave_car == 0;

    if (mean_note <= 45) {
        if (very_bright && !plucked) return 38;  // synth bass 1
        if (plucked) return 34;                  // pick bass
        if (slow_attack || long_release) return 35;  // fretless bass
        return 33;                               // finger bass
    }
    if (mean_note <= 52) {
        if (slow_attack) return 42;  // cello
        if (span >= 20) return 4;    // wide part: neutral electric piano
        if (organ) return 19;        // church organ
        if (plucked) return 45;      // pizzicato strings
        if (percussive) return 0;    // piano
        if (bright) return 27;       // electric guitar (clean)
        return 4;                    // electric piano
    }
    if (mean_note <= 60) {
        if (slow_attack) return 48;  // string ensemble 1
        if (span >= 24) return 4;    // wide part: neutral electric piano
        if (organ) return 16;        // drawbar organ
        if (sustained && bright) return 61;  // brass section
        if (plucked) return 25;      // acoustic guitar
        if (percussive) return 4;
        return 4;
    }
    if (mean_note <= 68) {
        if (slow_attack) return 49;  // string ensemble 2
        if (organ) return 17;        // percussive organ
        if (sustained && bright) return 62;  // synth brass 1
        if (plucked) return 27;      // electric guitar
        return 5;                    // electric piano 2
    }
    if (slow_attack) return bright ? 52 : 73;  // choir / flute
    if (organ) return 19;
    if (sustained && bright) return 66;  // tenor sax
    if (plucked) return 11;              // vibraphone
    if (very_bright) return 81;          // lead 2 (sawtooth)
    return 73;                           // flute
}

// One analysis pass over the track: which note range does each CMF program
// index actually play?  Collecting this at note-on time lets the map pick a
// bass for the low parts and a lead/string for the high parts, even though the
// program change itself happens before the first note.
void build_gm_program_map(const CmfSong& song, uint8_t out[128]) {
    struct Stats {
        int count = 0;
        int sum = 0;
        int min_note = 127;
        int max_note = 0;
    };
    Stats stats[128];
    uint8_t current[16] = {};
    bool rhythm = false;

    for (const CmfEvent& e : song.events) {
        if (e.type == 0xC0) {
            if (e.chan < 16) {
                current[e.chan] = e.a & 0x7F;
            }
            continue;
        }
        if (e.type == 0xB0 && e.a == 0x67) {
            rhythm = e.b != 0;
            continue;
        }
        if (e.type != 0x90 || e.b == 0) {
            continue;
        }
        if (rhythm && e.chan >= 11 && e.chan <= 15) {
            continue;  // percussion program
        }
        const int prog = current[e.chan & 0x0F];
        Stats& s = stats[prog];
        s.count += 1;
        s.sum += e.a;
        s.min_note = std::min(s.min_note, static_cast<int>(e.a));
        s.max_note = std::max(s.max_note, static_cast<int>(e.a));
    }

    for (int i = 0; i < 128; ++i) {
        if (i >= song.instrument_count) {
            out[i] = static_cast<uint8_t>(i & 0x7F);
            continue;
        }
        const int mean =
            stats[i].count > 0 ? stats[i].sum / stats[i].count : 60;
        const int span = stats[i].count > 0
                             ? stats[i].max_note - stats[i].min_note
                             : 0;
        out[i] = static_cast<uint8_t>(
            choose_gm_program(song.instruments[i], mean, span));
    }
}

}  // namespace

std::vector<std::string> MidiOut::devices() {
    std::vector<std::string> out;
#ifdef _WIN32
    const UINT n = midiOutGetNumDevs();
    for (UINT i = 0; i < n; ++i) {
        MIDIOUTCAPSW caps;
        if (midiOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            out.push_back(wide_to_utf8(caps.szPname));
        } else {
            out.push_back("MIDI device " + std::to_string(i));
        }
    }
#endif
    return out;
}

bool MidiOut::open(int index, std::string& err) {
    close();
#ifdef _WIN32
    const UINT count = midiOutGetNumDevs();
    if (index < 0 || static_cast<UINT>(index) >= count) {
        err = "MIDI device index out of range";
        return false;
    }
    HMIDIOUT handle = nullptr;
    const MMRESULT rc =
        midiOutOpen(&handle, static_cast<UINT>(index), 0, 0, CALLBACK_NULL);
    if (rc != MMSYSERR_NOERROR) {
        char text[128];
        midiOutGetErrorTextA(rc, text, sizeof(text));
        err = "cannot open MIDI device: " + std::string(text);
        return false;
    }
    handle_ = handle;
    device_ = index;
    const std::vector<std::string> names = devices();
    const std::string name =
        static_cast<size_t>(index) < names.size()
            ? names[static_cast<size_t>(index)]
            : std::string("?");
    std::fprintf(stderr, "midi: opened device %d (%s)\n", index,
                 name.c_str());
    return true;
#else
    (void)index;
    err = "MIDI output is not supported on this platform";
    return false;
#endif
}

void MidiOut::close() {
#ifdef _WIN32
    if (handle_) {
        HMIDIOUT handle = static_cast<HMIDIOUT>(handle_);
        midiOutReset(handle);
        midiOutClose(handle);
    }
#endif
    handle_ = nullptr;
    device_ = -1;
}

void MidiOut::send(uint32_t message) const {
#ifdef _WIN32
    if (handle_) {
        midiOutShortMsg(static_cast<HMIDIOUT>(handle_), message);
    }
#else
    (void)message;
#endif
}

void MidiOut::all_notes_off() const {
    for (int ch = 0; ch < 16; ++ch) {
        send(static_cast<uint32_t>(0xB0 | ch) |
             (static_cast<uint32_t>(123) << 8));
        send(static_cast<uint32_t>(0xB0 | ch) |
             (static_cast<uint32_t>(120) << 8));
    }
}

void MidiMusic::start(std::shared_ptr<const CmfSong> song) {
    stop();
    song_ = std::move(song);
    if (!song_ || !out_open()) {
        return;
    }
    playing_ = true;
    rhythm_ = false;
    next_event_ = 0;
    tick_pos_ = 0.0;
    end_tick_ = std::max<uint32_t>(1, song_->end_tick);
    build_gm_program_map(*song_, gm_program_);
    if (std::getenv("JILL_MIDI_DEBUG") != nullptr) {
        std::fprintf(stderr, "midi map for %s:\n", song_->filename.c_str());
        for (int i = 0; i < song_->instrument_count; ++i) {
            std::fprintf(stderr, "  cmf %3d -> gm %3d\n", i,
                         static_cast<int>(gm_program_[i]));
        }
    }
    for (int i = 0; i < 16; ++i) {
        program_[i] = 0;
        base_volume_[i] = 100;
    }
    out_->all_notes_off();
    for (int ch = 0; ch < 16; ++ch) {
        send_msg(ch, 0xB0, 121, 0);
        send_msg(ch, 0xC0, gm_program_[0], 0);
        apply_volume(ch);
    }
}

void MidiMusic::stop() {
    if (out_open()) {
        out_->all_notes_off();
    }
    playing_ = false;
    song_.reset();
    next_event_ = 0;
    tick_pos_ = 0.0;
    end_tick_ = 0;
    rhythm_ = false;
}

const std::string& MidiMusic::title() const {
    static const std::string kEmpty;
    return song_ ? song_->title : kEmpty;
}

void MidiMusic::set_enabled(bool on) {
    enabled_ = on;
    if (!on && out_open()) {
        out_->all_notes_off();
    }
    if (on) {
        for (int ch = 0; ch < 16; ++ch) {
            if (!(rhythm_ && ch >= 11)) {
                send_msg(output_channel(ch), 0xC0, gm_program_[program_[ch]], 0);
            }
            apply_volume(ch);
        }
    }
}

void MidiMusic::set_volume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
    if (out_open()) {
        for (int ch = 0; ch < 16; ++ch) {
            apply_volume(ch);
        }
    }
}

void MidiMusic::advance(int frames, int rate) {
    if (!playing_ || !song_ || !out_open() || frames <= 0 || rate <= 0) {
        return;
    }
    const double step =
        static_cast<double>(song_->ticks_per_second) / static_cast<double>(rate);
    const double end = tick_pos_ + static_cast<double>(frames) * step;
    const auto& events = song_->events;
    while (next_event_ < events.size() &&
           static_cast<double>(events[next_event_].tick) <= end) {
        handle_event(events[next_event_]);
        ++next_event_;
    }
    tick_pos_ = end;
    if (next_event_ >= events.size()) {
        // Level music loops, as in the original driver.
        tick_pos_ -= static_cast<double>(end_tick_);
        if (tick_pos_ < 0.0) {
            tick_pos_ = 0.0;
        }
        next_event_ = 0;
    }
}

void MidiMusic::handle_event(const CmfEvent& e) {
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

void MidiMusic::program_change(int chan, int program) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    program_[chan] = static_cast<uint8_t>(program & 0x7F);
    if (rhythm_ && chan >= 11) {
        return;  // percussion voices ignore melodic programs
    }
    // Translate the song's OPL instrument index through the per-track GM map
    // built from the patch envelopes and the register each part plays in.
    send_msg(output_channel(chan), 0xC0, gm_program_[program_[chan]], 0);
}

void MidiMusic::controller(int chan, int cc, int value) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    switch (cc) {
        case 0x07:  // channel volume
            base_volume_[chan] = value & 0x7F;
            apply_volume(chan);
            break;
        case 0x0A:  // pan
            send_msg(output_channel(chan), 0xB0, 0x0A, value & 0x7F);
            break;
        case 0x0B:  // expression
            send_msg(output_channel(chan), 0xB0, 0x0B, value & 0x7F);
            break;
        case 0x40:  // sustain pedal
            send_msg(output_channel(chan), 0xB0, 0x40, value & 0x7F);
            break;
        case 0x67:  // OPL melody/rhythm switch
            rhythm_ = value != 0;
            break;
        default:
            // 0x63 (AM/VIB depth), 0x68/0x69 (fine transpose) are OPL-only.
            break;
    }
}

int MidiMusic::output_channel(int chan) const {
    if (rhythm_ && chan >= 11 && chan <= 15) {
        return 9;  // GM percussion
    }
    // Keep melodic channel 9 away from GM's drum channel.
    if (chan == 9) {
        return 10;
    }
    return chan;
}

void MidiMusic::note_on(int chan, int note, int velocity) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    if (rhythm_ && chan >= 11 && chan <= 15) {
        send_msg(9, 0x90, gm_drum_note(chan), velocity & 0x7F);
        return;
    }
    send_msg(output_channel(chan), 0x90, note & 0x7F, velocity & 0x7F);
}

void MidiMusic::note_off(int chan, int note) {
    if (chan < 0 || chan >= 16) {
        return;
    }
    if (rhythm_ && chan >= 11 && chan <= 15) {
        // GM percussion is one-shot; no note-off is required.
        return;
    }
    send_msg(output_channel(chan), 0x80, note & 0x7F, 0);
}

void MidiMusic::send_msg(int chan, int status, int data1, int data2) {
    if (!enabled_ || !out_open()) {
        return;
    }
    const uint32_t message =
        static_cast<uint32_t>(status | (chan & 0x0F)) |
        (static_cast<uint32_t>(data1 & 0x7F) << 8) |
        (static_cast<uint32_t>(data2 & 0x7F) << 16);
    out_->send(message);
}

void MidiMusic::apply_volume(int chan) {
    if (chan < 0 || chan >= 16 || !out_open()) {
        return;
    }
    // The game's music volume scales the track's own channel volume.  When
    // music is muted, send 0 instead of notes so the device goes silent.
    const float gain = enabled_ ? volume_ : 0.0f;
    int v = static_cast<int>(std::lround(base_volume_[chan] * gain));
    v = std::max(0, std::min(127, v));
    send_msg(output_channel(chan), 0xB0, 0x07, v);
}
