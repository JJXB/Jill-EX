#pragma once
#include <cstdint>
#include <memory>

#include "cmf.h"
#include "fmopl_opl.h"
#include "nuked_opl.h"
#include "opl2.h"

// CMF sequencer feeding an OPL2-compatible core.  Mirrors the original
// `_sbfm_*` (SBFMDRV) driver: MIDI note on/off and program changes are
// translated to OPL register writes, voices are allocated from the nine
// melodic channels (six in rhythm mode), channels 11..15 become the five
// rhythm voices, and controller 0x67 switches between melody and rhythm mode.
class Music {
public:
    enum class OplCore { Builtin, Nuked, Fmopl };

    explicit Music(int sample_rate = 48000);

    // Selects the synthesis core.  Changing it resets the register state; the
    // caller restarts the current song.
    void set_opl_core(OplCore core);
    OplCore opl_core() const { return core_; }
    static const char* opl_core_name(OplCore core);

    void reset();
    void start(std::shared_ptr<const CmfSong> song);
    void stop();
    bool playing() const { return song_ && playing_; }
    const std::string& title() const;

    // Adds `frames` mono samples in about [-1, 1] to out.
    void render(float* out, int frames);

private:
    // One melodic OPL voice.  `owner` is the MIDI channel that most recently
    // used the voice (-1 = never used) and is kept after note-off so the next
    // note from that channel can reuse it without reloading the patch.
    struct Voice {
        int owner = -1;
        int note = -1;
        int patch = 0;
        bool active = false;
        uint32_t age = 0;
    };

    void handle_event(const CmfEvent& e);
    void note_on(int chan, int note, int velocity);
    void note_off(int chan, int note);
    void program_change(int chan, int program);
    void controller(int chan, int cc, int value);
    void write_rhythm_mode();
    void key_on(int opl_ch);
    void key_off(int opl_ch);
    void set_frequency(int opl_ch, double semitone_note, bool key_on);
    void load_melodic(int opl_ch, int instrument);
    void write_instrument(int opl_ch, const uint8_t* ins);
    void load_operator(int op_off, const uint8_t* ins, bool carrier_half);
    void set_carrier_volume(int opl_ch, int instrument, int velocity);
    void driver_reset();
    void release_note(int chan, int note);
    void update_transpose(int chan);
    int allocate_voice(int chan);
    void start_note(int chan, int note, int velocity);

    Opl2 opl_;
    NukedOpl nuked_;
    FmOpl fmopl_;
    OplCore core_ = OplCore::Builtin;

    void reset_cores();
    void write_reg(uint8_t reg, uint8_t value);
    int core_rate() const;
    double note_rate() const;
    void core_generate(float* out, int frames);
    std::shared_ptr<const CmfSong> song_;
    bool playing_ = false;
    size_t next_event_ = 0;
    double tick_pos_ = 0.0;
    double ticks_per_sample_ = 0.0;
    uint32_t end_tick_ = 0;

    bool rhythm_ = false;
    int depth_ = 3;
    int bd_ = 0;

    int program_[16] = {};
    double transpose_[16] = {};
    int percussion_notes_[16] = {};
    int note_playing_[16] = {};
    bool note_fix_[16] = {};
    int note_fix_vel_[16] = {};
    Voice voice_[9];
    uint32_t voice_age_ = 0;
    int fnum_[9] = {};
    int block_[9] = {};
};
