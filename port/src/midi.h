#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "cmf.h"

// Small wrapper around the system MIDI output API.  On Windows this is the
// WinMM MIDI mapper ("Microsoft GS Wavetable Synth" and any installed MIDI
// ports); other platforms currently report no devices.
class MidiOut {
public:
    MidiOut() = default;
    ~MidiOut() { close(); }
    MidiOut(const MidiOut&) = delete;
    MidiOut& operator=(const MidiOut&) = delete;

    static std::vector<std::string> devices();

    bool open(int index, std::string& err);
    void close();
    bool is_open() const { return handle_ != nullptr; }
    int device_index() const { return device_; }

    // status | data1 << 8 | data2 << 16
    void send(uint32_t message) const;
    void all_notes_off() const;

private:
    void* handle_ = nullptr;  // HMIDIOUT
    int device_ = -1;
};

// Plays a CMF/MIDI track through a MIDI output instead of the OPL2 core.
// Channel 11..15 become GM percussion on channel 10 in rhythm mode, matching
// the source format's five drum voices.
class MidiMusic {
public:
    void set_out(const MidiOut* out) { out_ = out; }
    bool out_open() const { return out_ && out_->is_open(); }

    void start(std::shared_ptr<const CmfSong> song);
    void stop();
    bool playing() const { return song_ && playing_; }
    const std::string& title() const;

    // Advances the sequencer by `frames` samples at `rate` Hz and sends due
    // events immediately.
    void advance(int frames, int rate);

    void set_enabled(bool on);
    bool enabled() const { return enabled_; }
    void set_volume(float volume);
    float volume() const { return volume_; }

private:
    void handle_event(const CmfEvent& e);
    void program_change(int chan, int program);
    void controller(int chan, int cc, int value);
    void note_on(int chan, int note, int velocity);
    void note_off(int chan, int note);
    void send_msg(int chan, int status, int data1, int data2);
    void apply_volume(int chan);
    int output_channel(int chan) const;

    const MidiOut* out_ = nullptr;
    std::shared_ptr<const CmfSong> song_;
    bool playing_ = false;
    bool enabled_ = true;
    size_t next_event_ = 0;
    double tick_pos_ = 0.0;
    uint32_t end_tick_ = 0;

    bool rhythm_ = false;  // controller 0x67: five percussion voices
    uint8_t program_[16] = {};
    int base_volume_[16] = {};
    float volume_ = 0.8f;
};
