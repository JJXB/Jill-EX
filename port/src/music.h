#pragma once
#include <cstdint>
#include <memory>

#include "cmf.h"
#include "opl2.h"

// CMF sequencer feeding the OPL2 core.  Mirrors the original `_sbfm_*` driver:
// MIDI note on/off and program changes are translated to OPL register writes,
// channels 11..15 become the five rhythm voices, and controller 0x67 switches
// between nine-channel melody mode and six-channel + percussion mode.
class Music {
public:
    explicit Music(int sample_rate = 48000);

    void reset();
    void start(std::shared_ptr<const CmfSong> song);
    void stop();
    bool playing() const { return song_ && playing_; }
    const std::string& title() const;

    // Adds `frames` mono samples in about [-1, 1] to out.
    void render(float* out, int frames);

private:
    void handle_event(const CmfEvent& e);
    void note_on(int chan, int note, int velocity);
    void note_off(int chan, int note);
    void program_change(int chan, int program);
    void controller(int chan, int cc, int value);
    void write_rhythm_mode();
    void key_on(int opl_ch);
    void key_off(int opl_ch);
    void set_frequency(int opl_ch, double semitone_note);
    void load_melodic(int opl_ch, int instrument);
    void load_operator(int op_off, const uint8_t* ins, bool carrier_half);
    void set_carrier_volume(int opl_ch, int midi_chan, int instrument,
                            int velocity);

    Opl2 opl_;
    std::shared_ptr<const CmfSong> song_;
    bool playing_ = false;
    size_t next_event_ = 0;
    double tick_pos_ = 0.0;
    double ticks_per_sample_ = 0.0;
    uint32_t end_tick_ = 0;

    bool rhythm_ = false;
    int depth_ = 3;
    int bd_ = 0;

    uint8_t program_[16] = {};
    int volume_[16] = {};
    double transpose_[16] = {};
    int active_note_[16] = {};
    int active_opl_[16] = {};
    int last_velocity_[16] = {};
    int percussion_notes_[16] = {};
    int fnum_[9] = {};
    int block_[9] = {};
};
