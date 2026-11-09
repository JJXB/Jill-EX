#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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

// One effect voice, matching the original's single Sound Blaster DMA channel:
// a new sound replaces the current one unless its priority is lower.
class SfxVoice {
public:
    void set_bank(std::shared_ptr<const SfxBank> bank);
    void play(int priority, int logical_id);
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
    std::shared_ptr<const SfxBank> bank_;
    int id_ = -1;
    int priority_ = 0;
    double pos_ = 0.0;
    bool active_ = false;
    float volume_ = 0.8f;
    float peak_ = 0.0f;
};
