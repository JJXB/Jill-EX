#include "audio.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#ifdef JILL_HAVE_SDL
#include <SDL.h>
#endif

MusicPlayer::MusicPlayer() : music_(48000) {}

MusicPlayer::~MusicPlayer() { close(); }

bool MusicPlayer::open(std::string& err) {
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        return true;
    }
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        err = std::string("SDL audio: ") + SDL_GetError();
        return false;
    }
    SDL_AudioSpec want;
    SDL_AudioSpec have;
    SDL_zero(want);
    want.freq = 48000;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = &MusicPlayer::callback;
    want.userdata = this;

    // Many Windows endpoints are 44.1 kHz; let SDL give us the device rate
    // (the sequencer is rebuilt below) and, if necessary, the device format.
    const Uint32 kAllowed =
        SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE |
        SDL_AUDIO_ALLOW_FORMAT_CHANGE;
    device_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have,
                                  SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    if (device_ == 0) {
        want.freq = 44100;
        device_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have,
                                      SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
    }
    if (device_ == 0) {
        device_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, kAllowed);
    }
    if (device_ == 0) {
        err = std::string("SDL_OpenAudioDevice: ") + SDL_GetError();
        return false;
    }
    rate_ = have.freq > 0 ? have.freq : 48000;
    channels_ = have.channels > 0 ? have.channels : 2;
    format_ = have.format;
    // The sequencer runs at the device rate so pitch and envelopes are right
    // even if the system refused 48 kHz.
    music_ = Music(rate_);
    sfx_.set_volume(sfx_volume_);
    std::fprintf(stderr, "audio: %d Hz, %d channel(s), %s\n", rate_, channels_,
                 format_ == AUDIO_F32SYS ? "float" : "s16");
    SDL_PauseAudioDevice(device_, 0);
    return true;
#else
    err = "built without SDL audio support";
    return false;
#endif
}

void MusicPlayer::close() {
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
#endif
    music_.stop();
    playing_ = false;
}

void MusicPlayer::play(std::shared_ptr<const CmfSong> song) {
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_LockAudioDevice(device_);
    }
#endif
    pending_ = std::move(song);
    has_pending_ = true;
    title_ = pending_ ? pending_->title : std::string();
    playing_ = pending_ != nullptr;
    peak_ = 0.0f;
    frames_mixed_ = 0;
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_UnlockAudioDevice(device_);
    }
#endif
}

void MusicPlayer::stop() {
    play(nullptr);
}

void MusicPlayer::set_enabled(bool on) {
    // Keep the device running: effects have their own switch and volume.
    enabled_ = on;
}

void MusicPlayer::set_volume(float volume) {
    volume_ = std::max(0.0f, std::min(1.0f, volume));
}

bool MusicPlayer::load_sfx(const std::string& path, std::string& err) {
    auto bank = std::make_shared<SfxBank>();
    if (!bank->load(path, err)) {
        return false;
    }
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_LockAudioDevice(device_);
    }
#endif
    sfx_bank_ = bank;
    sfx_.set_bank(bank);
    sfx_.set_volume(sfx_volume_);
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_UnlockAudioDevice(device_);
    }
#endif
    return true;
}

void MusicPlayer::play_sfx(int priority, int logical_id) {
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_LockAudioDevice(device_);
    }
#endif
    sfx_.play(priority, logical_id);
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_UnlockAudioDevice(device_);
    }
#endif
}

void MusicPlayer::set_sfx_volume(float volume) {
    sfx_volume_ = std::max(0.0f, std::min(1.0f, volume));
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_LockAudioDevice(device_);
    }
#endif
    sfx_.set_volume(sfx_volume_);
#ifdef JILL_HAVE_SDL
    if (device_ != 0) {
        SDL_UnlockAudioDevice(device_);
    }
#endif
}

#ifdef JILL_HAVE_SDL
void MusicPlayer::callback(void* user, uint8_t* stream, int len) {
    static_cast<MusicPlayer*>(user)->mix(stream, len);
}
#endif

void MusicPlayer::mix(uint8_t* stream, int len) {
    const int bps = format_ == AUDIO_F32SYS ? 4 : 2;
    const int channels = channels_ > 0 ? channels_ : 2;
    const int frames = len / (bps * channels);
    if (frames <= 0) {
        std::memset(stream, 0, static_cast<size_t>(len));
        return;
    }
    if (has_pending_) {
        music_.start(pending_);
        pending_.reset();
        has_pending_ = false;
    }
    if (scratch_.size() < static_cast<size_t>(frames)) {
        scratch_.resize(static_cast<size_t>(frames));
    }
    std::fill(scratch_.begin(), scratch_.begin() + frames, 0.0f);
    music_.render(scratch_.data(), frames);
    // Music toggle/volume only affect the OPL mix; effects have their own.
    const float music_gain = enabled_ ? volume_ : 0.0f;
    for (int i = 0; i < frames; ++i) {
        scratch_[static_cast<size_t>(i)] *= music_gain;
    }
    sfx_.mix(scratch_.data(), frames, rate_);
    float peak = 0.0f;

    if (format_ == AUDIO_F32SYS) {
        auto* out = reinterpret_cast<float*>(stream);
        for (int i = 0; i < frames; ++i) {
            float s = scratch_[static_cast<size_t>(i)];
            s = std::max(-1.0f, std::min(1.0f, s));
            peak = std::max(peak, std::fabs(s));
            for (int c = 0; c < channels; ++c) {
                out[i * channels + c] = s;
            }
        }
    } else if (format_ == AUDIO_S16SYS) {
        auto* out = reinterpret_cast<int16_t*>(stream);
        for (int i = 0; i < frames; ++i) {
            float s = scratch_[static_cast<size_t>(i)];
            s = std::max(-1.0f, std::min(1.0f, s));
            peak = std::max(peak, std::fabs(s));
            const int16_t v = static_cast<int16_t>(s * 32767.0f);
            for (int c = 0; c < channels; ++c) {
                out[i * channels + c] = v;
            }
        }
    } else {
        std::memset(stream, 0, static_cast<size_t>(len));
    }
    peak_ = std::max(peak_, peak);
    frames_mixed_ += static_cast<unsigned long long>(frames);
}

namespace {

void write_le16(std::FILE* f, uint16_t v) {
    const uint8_t b[2] = {static_cast<uint8_t>(v),
                          static_cast<uint8_t>(v >> 8)};
    std::fwrite(b, 1, 2, f);
}

void write_le32(std::FILE* f, uint32_t v) {
    const uint8_t b[4] = {static_cast<uint8_t>(v), static_cast<uint8_t>(v >> 8),
                          static_cast<uint8_t>(v >> 16),
                          static_cast<uint8_t>(v >> 24)};
    std::fwrite(b, 1, 4, f);
}

}  // namespace

bool MusicPlayer::render_wav(const CmfSong& song, const std::string& path,
                             double seconds, int sample_rate, float volume,
                             std::string& err) {
    if (seconds <= 0.0) {
        seconds = 5.0;
    }
    if (sample_rate < 8000) {
        sample_rate = 48000;
    }
    auto shared = std::make_shared<CmfSong>(song);
    Music music(sample_rate);
    music.start(shared);

    const int frames = static_cast<int>(seconds * sample_rate);
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        err = "cannot write " + path;
        return false;
    }
    const uint32_t data_bytes = static_cast<uint32_t>(frames) * 2u;
    std::fwrite("RIFF", 1, 4, f);
    write_le32(f, 36u + data_bytes);
    std::fwrite("WAVEfmt ", 1, 8, f);
    write_le32(f, 16);
    write_le16(f, 1);  // PCM
    write_le16(f, 1);  // mono
    write_le32(f, static_cast<uint32_t>(sample_rate));
    write_le32(f, static_cast<uint32_t>(sample_rate) * 2u);
    write_le16(f, 2);
    write_le16(f, 16);
    std::fwrite("data", 1, 4, f);
    write_le32(f, data_bytes);

    std::vector<float> block(2048);
    for (int done = 0; done < frames; done += static_cast<int>(block.size())) {
        const int n = std::min<int>(static_cast<int>(block.size()), frames - done);
        std::fill(block.begin(), block.begin() + n, 0.0f);
        music.render(block.data(), n);
        for (int i = 0; i < n; ++i) {
            float s = block[static_cast<size_t>(i)] * volume;
            s = std::max(-1.0f, std::min(1.0f, s));
            const int16_t v = static_cast<int16_t>(s * 32767.0f);
            const uint8_t b[2] = {static_cast<uint8_t>(v),
                                  static_cast<uint8_t>(v >> 8)};
            std::fwrite(b, 1, 2, f);
        }
    }
    std::fclose(f);
    return true;
}
