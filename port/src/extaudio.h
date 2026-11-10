#pragma once
#include <cstdint>
#include <string>
#include <vector>

// A decoded external audio file, kept as interleaved float samples in the
// source's own rate.  Music is mixed looped and effects play once, both
// resampled to the device rate by audio.cpp/sfx.cpp.
struct DecodedAudio {
    int rate = 0;
    int channels = 0;              // 1 or 2
    std::vector<float> samples;    // interleaved, -1..1

    bool valid() const { return rate > 0 && channels > 0 && !samples.empty(); }
    size_t frames() const {
        return channels > 0 ? samples.size() / static_cast<size_t>(channels) : 0;
    }
    double seconds() const {
        return rate > 0 ? static_cast<double>(frames()) / rate : 0.0;
    }
};

// Decode an audio file from memory.  `ext` is a lower-case extension without
// the dot ("ogg", "mp3", "m4a", "wav", ...).  Supported: OGG/Vorbis, MP3,
// M4A/AAC (Windows Media Foundation), WAV.
bool decode_audio_memory(const uint8_t* data, size_t size, const std::string& ext,
                         DecodedAudio& out, std::string& err);

// Read a file and decode it.  `path` may live inside a zip; callers normally
// read the bytes and use decode_audio_memory().
bool decode_audio_file(const std::string& path, DecodedAudio& out,
                       std::string& err);
