#include "midi.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

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
    for (int i = 0; i < 16; ++i) {
        program_[i] = 0;
        base_volume_[i] = 100;
    }
    out_->all_notes_off();
    for (int ch = 0; ch < 16; ++ch) {
        send_msg(ch, 0xB0, 121, 0);
        send_msg(ch, 0xC0, 0, 0);
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
                send_msg(output_channel(ch), 0xC0, program_[ch], 0);
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
    // The CMF program is an index into the song's OPL instrument table.  Send
    // the same number to the MIDI device as a GM program; a bank/port that
    // understands GM will at least give each part a distinct instrument.
    send_msg(output_channel(chan), 0xC0, program_[chan], 0);
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
