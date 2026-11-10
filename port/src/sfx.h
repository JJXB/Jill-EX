#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "extaudio.h"

// Sampled effects from the `.vcl` bank: 50 entries of raw unsigned 8-bit mono
// PCM (silence = 128) at about 6000 Hz, followed by the text-page table.
class SfxBank {
public:
    bool load(const std::string& path, std::string& err);
    bool loaded() const { return loaded_; }

    bool has(int id) const;
    const std::vector<uint8_t>& sample(int id) const;
    int rate(int id) const;

    // The shared 50-byte logical-id -> sample-index table (0 = identity).
    static int remap(int logical_id);

private:
    bool loaded_ = false;
    std::vector<std::vector<uint8_t>> samples_;
    std::vector<int> rates_;
};

// Pack replacements for the 50 logical effect slots (`sfx/<slot>.ogg`).
using SfxOverrides = std::array<std::shared_ptr<const DecodedAudio>, 50>;

// One effect voice, matching the original's single Sound Blaster DMA channel:
// a new sound replaces the current one unless its priority is lower.
class SfxVoice {
public:
    void set_bank(std::shared_ptr<const SfxBank> bank);
    // External pack overrides.  A non-null entry replaces that raw slot.
    void set_overrides(std::shared_ptr<const SfxOverrides> overrides);
    // 1-bit PC-speaker rendition of the same samples (optional SOUND mode).
    void set_pc_speaker(bool on) { pc_speaker_ = on; }
    bool pc_speaker() const { return pc_speaker_; }
    void play(int priority, int logical_id);
    // Play a raw bank index, bypassing the logical-id remap (sound test).
    void play_sample(int priority, int sample_id);
    void stop();
    void set_volume(float volume) { volume_ = volume; }
    float volume() const { return volume_; }

    bool active() const { return active_; }
    int priority() const { return priority_; }
    float peak() const { return peak_; }
    void reset_peak() { peak_ = 0.0f; }

    // Adds resampled mono samples into `out` (device rate).
    void mix(float* out, int frames, int device_rate);

private:
    void start_external(int priority,
                        std::shared_ptr<const DecodedAudio> audio);
    void mix_external(float* out, int frames, int device_rate);

    std::shared_ptr<const SfxBank> bank_;
    std::shared_ptr<const SfxOverrides> overrides_;
    std::shared_ptr<const DecodedAudio> ext_;
    double ext_pos_ = 0.0;
    int id_ = -1;
    int priority_ = 0;
    double pos_ = 0.0;
    bool active_ = false;
    float volume_ = 0.8f;
    float peak_ = 0.0f;
    bool pc_speaker_ = false;
};
