#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "cmf.h"
#include "music.h"
#include "sfx.h"

// Owns the SDL audio device and the CMF player used by the game.  The audio
// callback runs on SDL's thread; track changes and the enable/volume settings
// are posted under the device lock.
class MusicPlayer {
public:
    MusicPlayer();
    ~MusicPlayer();

    bool open(std::string& err);
    void close();
    bool is_open() const { return device_ != 0; }

    // Swaps in a parsed track (nullptr stops).  Safe before open(): the track
    // is queued and starts with the device.
    void play(std::shared_ptr<const CmfSong> song);
    // Loop an externally decoded track (MP3/OGG/M4A/WAV) instead of CMF.
    void play_pcm(std::shared_ptr<const DecodedAudio> audio);
    bool pcm_active() const { return ext_music_ != nullptr; }
    void stop();
    void set_enabled(bool on);  // music only; effects keep playing
    bool enabled() const { return enabled_; }
    void set_pc_speaker(bool on);
    bool pc_speaker() const { return pc_speaker_; }
    void set_volume(float volume);  // 0..1 (music)
    float volume() const { return volume_; }

    // Sampled effects (`.vcl`).  Load once after selecting a game.
    bool load_sfx(const std::string& path, std::string& err);
    // External pack replacements for individual slots (raw indices).
    void set_sfx_overrides(std::shared_ptr<const SfxOverrides> overrides);
    void play_sfx(int priority, int logical_id);
    // Raw bank index for the front-end sound test (no logical-id remap).
    void play_sfx_sample(int priority, int sample_id);
    bool sfx_has_sample(int sample_id) const;
    int sfx_sample_rate(int sample_id) const;
    double sfx_sample_seconds(int sample_id) const;
    int sfx_loaded_samples() const;
    void set_sfx_volume(float volume);  // 0..1
    float sfx_volume() const { return sfx_volume_; }
    float sfx_peak() const { return sfx_.peak(); }
    bool playing() const { return playing_; }
    const std::string& title() const { return title_; }
    // Diagnostics: device format and the peak/frame counters the callback has
    // produced (used by --audio-check).
    int sample_rate() const { return rate_; }
    int channels() const { return channels_; }
    float peak_level() const { return peak_; }
    unsigned long long frames_mixed() const { return frames_mixed_; }

    // Offline render for --dump-music: writes a 16-bit mono WAV.
    static bool render_wav(const CmfSong& song, const std::string& path,
                           double seconds, int sample_rate, float volume,
                           std::string& err);

private:
    static void callback(void* user, uint8_t* stream, int len);
    void mix(uint8_t* stream, int len);

    unsigned device_ = 0;
    int rate_ = 0;
    int channels_ = 0;
    uint16_t format_ = 0;  // SDL_AudioFormat (S16 or F32)
    float peak_ = 0.0f;
    unsigned long long frames_mixed_ = 0;
    Music music_;
    std::shared_ptr<const SfxBank> sfx_bank_;
    SfxVoice sfx_;
    bool enabled_ = true;
    float volume_ = 0.8f;
    float sfx_volume_ = 0.8f;
    bool playing_ = false;
    std::string title_;
    std::vector<float> scratch_;
    std::shared_ptr<const CmfSong> pending_;
    bool has_pending_ = false;
    bool pc_speaker_ = false;
    std::shared_ptr<const DecodedAudio> ext_music_;
    double ext_pos_ = 0.0;
    std::shared_ptr<const SfxOverrides> sfx_overrides_;
};
