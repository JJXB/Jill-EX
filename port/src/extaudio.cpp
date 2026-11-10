#include "extaudio.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <type_traits>

#define DR_MP3_IMPLEMENTATION
#include "third_party/dr_mp3.h"

// The implementation is compiled separately as C (third_party/stb_vorbis.c);
// this only pulls in the declarations.
#define STB_VORBIS_HEADER_ONLY
#include "third_party/stb_vorbis.c"
#undef STB_VORBIS_HEADER_ONLY

#if defined(_WIN32)
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <objbase.h>
#endif

namespace {

std::string lower_ext(const std::string& in) {
    std::string s = in;
    if (!s.empty() && s[0] == '.') {
        s.erase(s.begin());
    }
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

bool decode_ogg(const uint8_t* data, size_t size, DecodedAudio& out,
                std::string& err) {
    int error = 0;
    stb_vorbis* v = stb_vorbis_open_memory(data, static_cast<int>(size), &error,
                                           nullptr);
    if (!v) {
        err = "OGG: cannot open Vorbis stream";
        return false;
    }
    const stb_vorbis_info info = stb_vorbis_get_info(v);
    out.rate = static_cast<int>(info.sample_rate);
    out.channels = static_cast<int>(info.channels);
    out.samples.clear();
    const int total = stb_vorbis_stream_length_in_samples(v);
    if (total > 0) {
        out.samples.reserve(static_cast<size_t>(total) * out.channels);
    }
    float** frame = nullptr;
    int n = 0;
    while ((n = stb_vorbis_get_frame_float(v, nullptr, &frame)) > 0) {
        for (int i = 0; i < n; ++i) {
            if (out.channels == 1) {
                out.samples.push_back(frame[0][i]);
            } else {
                for (int c = 0; c < out.channels; ++c) {
                    out.samples.push_back(frame[c][i]);
                }
            }
        }
    }
    stb_vorbis_close(v);
    if (out.samples.empty()) {
        err = "OGG: no audio frames";
        return false;
    }
    return true;
}

bool decode_mp3(const uint8_t* data, size_t size, DecodedAudio& out,
                std::string& err) {
    drmp3 mp3;
    if (!drmp3_init_memory(&mp3, data, size, nullptr)) {
        err = "MP3: cannot open stream";
        return false;
    }
    out.rate = static_cast<int>(mp3.sampleRate);
    out.channels = static_cast<int>(mp3.channels);
    const drmp3_uint64 frames = drmp3_get_pcm_frame_count(&mp3);
    out.samples.clear();
    if (frames > 0) {
        out.samples.resize(static_cast<size_t>(frames) * mp3.channels);
        drmp3_read_pcm_frames_f32(&mp3, frames, out.samples.data());
    }
    drmp3_uninit(&mp3);
    if (out.samples.empty()) {
        err = "MP3: no audio frames";
        return false;
    }
    return true;
}

uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}
uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

bool decode_wav(const uint8_t* d, size_t size, DecodedAudio& out,
                std::string& err) {
    if (size < 12 || std::memcmp(d, "RIFF", 4) != 0 ||
        std::memcmp(d + 8, "WAVE", 4) != 0) {
        err = "WAV: not a RIFF/WAVE file";
        return false;
    }
    int format = 0;
    int channels = 0;
    int bits = 0;
    int rate = 0;
    const uint8_t* pcm = nullptr;
    size_t pcm_bytes = 0;
    size_t p = 12;
    while (p + 8 <= size) {
        const uint32_t len = rd32(d + p + 4);
        const uint8_t* chunk = d + p + 8;
        const size_t avail = size - (p + 8);
        const size_t n = std::min<size_t>(len, avail);
        if (std::memcmp(d + p, "fmt ", 4) == 0 && n >= 16) {
            format = rd16(chunk);
            channels = rd16(chunk + 2);
            rate = static_cast<int>(rd32(chunk + 4));
            bits = rd16(chunk + 14);
            if (format == 0xFFFE && n >= 26) {
                format = rd16(chunk + 24);  // WAVE_FORMAT_EXTENSIBLE subformat
            }
        } else if (std::memcmp(d + p, "data", 4) == 0) {
            pcm = chunk;
            pcm_bytes = n;
        }
        p += 8 + len + (len & 1);
    }
    if (!pcm || channels <= 0 || rate <= 0 || bits <= 0) {
        err = "WAV: missing fmt/data chunk";
        return false;
    }
    out.rate = rate;
    out.channels = channels > 2 ? 2 : channels;
    out.samples.clear();
    const size_t bytes_per = static_cast<size_t>(bits / 8);
    if (bytes_per == 0) {
        err = "WAV: bad sample size";
        return false;
    }
    const size_t frames = pcm_bytes / (bytes_per * channels);
    out.samples.reserve(frames * out.channels);
    for (size_t i = 0; i < frames; ++i) {
        for (int c = 0; c < channels; ++c) {
            if (c >= out.channels) {
                continue;
            }
            const uint8_t* s = pcm + (i * channels + c) * bytes_per;
            float v = 0.0f;
            if (format == 3 && bits == 32) {
                float f = 0.0f;
                std::memcpy(&f, s, 4);
                v = f;
            } else if (bits == 8) {
                v = (static_cast<int>(s[0]) - 128) / 128.0f;
            } else if (bits == 16) {
                v = static_cast<int16_t>(rd16(s)) / 32768.0f;
            } else if (bits == 24) {
                int32_t x = static_cast<int32_t>(s[0] | (s[1] << 8) | (s[2] << 16));
                if (x & 0x800000) {
                    x |= ~0xFFFFFF;
                }
                v = static_cast<float>(x) / 8388608.0f;
            } else if (bits == 32) {
                v = static_cast<float>(static_cast<int32_t>(rd32(s))) /
                    2147483648.0f;
            }
            out.samples.push_back(v);
        }
    }
    if (out.samples.empty()) {
        err = "WAV: no audio frames";
        return false;
    }
    return true;
}

#if defined(_WIN32)
template <typename T>
void release(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

bool mf_started() {
    static bool started = false;
    if (!started) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET))) {
            return false;
        }
        started = true;
    }
    return true;
}

template <typename T>
bool read_attr(IMFMediaType* type, const GUID& key, T& out) {
    PROPVARIANT v;
    PropVariantInit(&v);
    const HRESULT hr = type->GetItem(key, &v);
    if (SUCCEEDED(hr)) {
        if constexpr (std::is_same<T, UINT32>::value) {
            out = v.uintVal;
        } else {
            out = *reinterpret_cast<T*>(&v);
        }
        PropVariantClear(&v);
        return true;
    }
    PropVariantClear(&v);
    return false;
}

bool decode_mf(const uint8_t* data, size_t size, DecodedAudio& out,
               std::string& err) {
    if (!mf_started()) {
        err = "M4A: Media Foundation unavailable";
        return false;
    }
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!mem) {
        err = "M4A: out of memory";
        return false;
    }
    void* dst = GlobalLock(mem);
    std::memcpy(dst, data, size);
    GlobalUnlock(mem);
    IStream* stream = nullptr;
    if (FAILED(CreateStreamOnHGlobal(mem, TRUE, &stream))) {
        GlobalFree(mem);
        err = "M4A: cannot create stream";
        return false;
    }
    IMFByteStream* bytes = nullptr;
    IMFSourceReader* reader = nullptr;
    IMFMediaType* want = nullptr;
    IMFMediaType* have = nullptr;
    bool ok = false;
    do {
        if (FAILED(MFCreateMFByteStreamOnStream(stream, &bytes))) {
            err = "M4A: cannot wrap stream";
            break;
        }
        if (FAILED(MFCreateSourceReaderFromByteStream(bytes, nullptr, &reader))) {
            err = "M4A: unsupported container";
            break;
        }
        reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
        reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
        if (FAILED(MFCreateMediaType(&want))) {
            err = "M4A: media type";
            break;
        }
        want->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        want->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        want->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
        want->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 44100);
        want->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
        if (FAILED(reader->SetCurrentMediaType(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, want))) {
            err = "M4A: no PCM decoder";
            break;
        }
        if (FAILED(reader->GetCurrentMediaType(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM, &have))) {
            err = "M4A: cannot read format";
            break;
        }
        UINT32 rate = 0;
        UINT32 channels = 0;
        UINT32 bits = 16;
        read_attr(have, MF_MT_AUDIO_SAMPLES_PER_SECOND, rate);
        read_attr(have, MF_MT_AUDIO_NUM_CHANNELS, channels);
        read_attr(have, MF_MT_AUDIO_BITS_PER_SAMPLE, bits);
        if (rate == 0 || channels == 0) {
            err = "M4A: bad output format";
            break;
        }
        out.rate = static_cast<int>(rate);
        out.channels = channels > 2 ? 2 : static_cast<int>(channels);
        out.samples.clear();
        for (;;) {
            DWORD flags = 0;
            IMFSample* sample = nullptr;
            const HRESULT hr = reader->ReadSample(
                MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags,
                nullptr, &sample);
            if (FAILED(hr)) {
                break;
            }
            if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
                release(sample);
                break;
            }
            if (!sample) {
                continue;
            }
            IMFMediaBuffer* buf = nullptr;
            if (SUCCEEDED(sample->ConvertToContiguousBuffer(&buf)) && buf) {
                BYTE* ptr = nullptr;
                DWORD len = 0;
                if (SUCCEEDED(buf->Lock(&ptr, nullptr, &len)) && ptr) {
                    const int bps = static_cast<int>(bits / 8);
                    if (bps > 0) {
                        const size_t frames = len / (bps * channels);
                        for (size_t i = 0; i < frames; ++i) {
                            for (UINT32 c = 0; c < channels; ++c) {
                                if (static_cast<int>(c) >= out.channels) {
                                    continue;
                                }
                                const BYTE* s =
                                    ptr + (i * channels + c) * bps;
                                float v = 0.0f;
                                if (bits == 16) {
                                    v = static_cast<int16_t>(rd16(s)) / 32768.0f;
                                } else if (bits == 32) {
                                    v = static_cast<float>(
                                            static_cast<int32_t>(rd32(s))) /
                                        2147483648.0f;
                                } else if (bits == 8) {
                                    v = (static_cast<int>(s[0]) - 128) / 128.0f;
                                }
                                out.samples.push_back(v);
                            }
                        }
                    }
                    buf->Unlock();
                }
                release(buf);
            }
            release(sample);
        }
        ok = !out.samples.empty();
        if (!ok) {
            err = "M4A: no audio frames";
        }
    } while (false);
    release(have);
    release(want);
    release(reader);
    release(bytes);
    release(stream);
    return ok;
}
#endif

}  // namespace

bool decode_audio_memory(const uint8_t* data, size_t size, const std::string& ext,
                         DecodedAudio& out, std::string& err) {
    out = DecodedAudio{};
    if (!data || size == 0) {
        err = "empty audio file";
        return false;
    }
    const std::string e = lower_ext(ext);
    if (e == "ogg" || e == "oga") {
        return decode_ogg(data, size, out, err);
    }
    if (e == "mp3" || e == "mp2") {
        return decode_mp3(data, size, out, err);
    }
    if (e == "wav") {
        return decode_wav(data, size, out, err);
    }
#if defined(_WIN32)
    if (e == "m4a" || e == "mp4" || e == "aac" || e == "m4b") {
        return decode_mf(data, size, out, err);
    }
#endif
    // Unknown extension: sniff the common signatures, then fall back to MF.
    if (size >= 4 && std::memcmp(data, "OggS", 4) == 0) {
        return decode_ogg(data, size, out, err);
    }
    if (size >= 3 && std::memcmp(data, "ID3", 3) == 0) {
        return decode_mp3(data, size, out, err);
    }
    if (size >= 4 && std::memcmp(data, "RIFF", 4) == 0) {
        return decode_wav(data, size, out, err);
    }
#if defined(_WIN32)
    {
        std::string mf_err;
        if (decode_mf(data, size, out, mf_err)) {
            return true;
        }
        err = mf_err;
    }
#else
    err = "unsupported audio format: " + e;
#endif
    return false;
}

bool decode_audio_file(const std::string& path, DecodedAudio& out,
                       std::string& err) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(f);
        err = "empty file " + path;
        return false;
    }
    std::vector<uint8_t> data(static_cast<size_t>(size));
    const size_t got = std::fread(data.data(), 1, data.size(), f);
    std::fclose(f);
    if (got != data.size()) {
        err = "short read " + path;
        return false;
    }
    const size_t dot = path.find_last_of('.');
    const std::string ext =
        dot == std::string::npos ? std::string() : path.substr(dot + 1);
    return decode_audio_memory(data.data(), data.size(), ext, out, err);
}
