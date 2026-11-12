#include "sfx.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace {

uint16_t rd16(const std::vector<uint8_t>& d, size_t p) {
    if (p + 1 >= d.size()) {
        return 0;
    }
    return static_cast<uint16_t>(d[p] | (d[p + 1] << 8));
}

uint32_t rd32(const std::vector<uint8_t>& d, size_t p) {
    if (p + 3 >= d.size()) {
        return 0;
    }
    return static_cast<uint32_t>(d[p]) | (static_cast<uint32_t>(d[p + 1]) << 8) |
           (static_cast<uint32_t>(d[p + 2]) << 16) |
           (static_cast<uint32_t>(d[p + 3]) << 24);
}

}  // namespace

int SfxBank::remap(int logical_id) {
    // Identical in every Jill-engine build (docs/jill23-sounds.md).  0 means
    // "use the id as-is".
    static const uint8_t kRemap[50] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x1C,
        0x00, 0x00, 0x00, 0x18, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x30, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x08, 0x00, 0x29, 0x00,
        0x20, 0x08, 0x18, 0x0A, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (logical_id < 0 || logical_id >= 50) {
        return logical_id;
    }
    const int actual = kRemap[logical_id];
    return actual ? actual : logical_id;
}

bool SfxBank::load(const std::string& path, std::string& err) {
    loaded_ = false;
    samples_.assign(50, std::vector<uint8_t>());
    rates_.assign(50, 6000);

    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::vector<uint8_t> data;
    uint8_t buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) {
        data.insert(data.end(), buf, buf + n);
    }
    std::fclose(f);
    if (data.size() < 0x280) {
        err = path + " is too small to be a .vcl sound bank";
        return false;
    }
    for (int i = 0; i < 50; ++i) {
        const uint32_t off = rd32(data, 0x000 + 4 * i);
        const uint16_t len = rd16(data, 0x0C8 + 2 * i);
        const uint16_t rate = rd16(data, 0x12C + 2 * i);
        if (len == 0 || off >= data.size() ||
            off + len > data.size()) {
            continue;
        }
        samples_[static_cast<size_t>(i)].assign(data.begin() + off,
                                                data.begin() + off + len);
        rates_[static_cast<size_t>(i)] = rate > 0 ? rate : 6000;
    }
    loaded_ = true;
    return true;
}

bool SfxBank::has(int id) const {
    return id >= 0 && id < static_cast<int>(samples_.size()) &&
           !samples_[static_cast<size_t>(id)].empty();
}

const std::vector<uint8_t>& SfxBank::sample(int id) const {
    static const std::vector<uint8_t> kEmpty;
    if (!has(id)) {
        return kEmpty;
    }
    return samples_[static_cast<size_t>(id)];
}

int SfxBank::rate(int id) const {
    if (id < 0 || id >= static_cast<int>(rates_.size())) {
        return 6000;
    }
    return rates_[static_cast<size_t>(id)];
}

void SfxVoice::set_bank(std::shared_ptr<const SfxBank> bank) {
    bank_ = std::move(bank);
    stop();
}

void SfxVoice::set_overrides(std::shared_ptr<const SfxOverrides> overrides) {
    overrides_ = std::move(overrides);
    stop();
}

void SfxVoice::play(int priority, int logical_id) {
    play_sample(priority, SfxBank::remap(logical_id));
}

void SfxVoice::play_sample(int priority, int sample_id) {
    if (overrides_ && sample_id >= 0 &&
        sample_id < static_cast<int>(overrides_->size())) {
        const auto& o = (*overrides_)[static_cast<size_t>(sample_id)];
        if (o && o->valid()) {
            start_external(priority, o);
            return;
        }
    }
    if (!bank_ || !bank_->loaded()) {
        return;
    }
    if (active_ && priority < priority_) {
        return;  // a strictly higher-priority effect is still playing
    }
    if (!bank_->has(sample_id)) {
        return;  // intentionally absent sample (0x11 / 0x25 in the episodes)
    }
    ext_.reset();
    id_ = sample_id;
    priority_ = std::max(1, priority);
    pos_ = 0.0;
    active_ = true;
}

void SfxVoice::start_external(int priority,
                              std::shared_ptr<const DecodedAudio> audio) {
    if (!audio || !audio->valid()) {
        return;
    }
    if (active_ && priority < priority_) {
        return;
    }
    ext_ = std::move(audio);
    ext_pos_ = 0.0;
    id_ = -1;
    priority_ = std::max(1, priority);
    active_ = true;
}

void SfxVoice::stop() {
    active_ = false;
    priority_ = 0;
    id_ = -1;
    pos_ = 0.0;
    ext_.reset();
    ext_pos_ = 0.0;
}

void SfxVoice::mix_external(float* out, int frames, int device_rate) {
    const DecodedAudio& a = *ext_;
    const int ch = a.channels > 0 ? a.channels : 1;
    const size_t total = a.frames();
    const double step = static_cast<double>(a.rate) /
                        (device_rate > 0 ? device_rate : 48000);
    for (int i = 0; i < frames; ++i) {
        const size_t f = static_cast<size_t>(ext_pos_);
        if (f >= total) {
            stop();
            break;
        }
        float v = 0.0f;
        for (int c = 0; c < ch; ++c) {
            v += a.samples[f * static_cast<size_t>(ch) + static_cast<size_t>(c)];
        }
        v /= static_cast<float>(ch);
        if (pc_speaker_) {
            v = (v >= 0.0f ? 1.0f : -1.0f) * 0.5f;
        }
        v *= volume_;
        out[i] += v;
        peak_ = std::max(peak_, static_cast<float>(std::fabs(v)));
        ext_pos_ += step;
    }
}

void SfxVoice::mix(float* out, int frames, int device_rate) {
    if (!active_) {
        return;
    }
    if (ext_) {
        mix_external(out, frames, device_rate);
        return;
    }
    if (!bank_ || !bank_->has(id_)) {
        return;
    }
    const std::vector<uint8_t>& s = bank_->sample(id_);
    if (s.size() < 2) {
        stop();
        return;
    }
    const double step = static_cast<double>(bank_->rate(id_)) /
                        (device_rate > 0 ? device_rate : 48000);
    for (int i = 0; i < frames; ++i) {
        const size_t a = static_cast<size_t>(pos_);
        if (a + 1 >= s.size()) {
            stop();
            break;
        }
        const float frac = static_cast<float>(pos_ - static_cast<double>(a));
        const float s0 = (static_cast<float>(s[a]) - 128.0f) / 128.0f;
        const float s1 = (static_cast<float>(s[a + 1]) - 128.0f) / 128.0f;
        float v = s0 * (1.0f - frac) + s1 * frac;
        if (pc_speaker_) {
            v = (v >= 0.0f ? 1.0f : -1.0f) * 0.5f;
        }
        v *= volume_;
        out[i] += v;
        peak_ = std::max(peak_, static_cast<float>(std::fabs(v)));
        pos_ += step;
    }
}
