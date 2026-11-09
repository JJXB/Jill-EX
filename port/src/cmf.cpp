#include "cmf.h"

#include <cstdio>
#include <cstring>

namespace {

uint16_t rd16(const std::vector<uint8_t>& d, size_t p) {
    if (p + 1 >= d.size()) {
        return 0;
    }
    return static_cast<uint16_t>(d[p] | (d[p + 1] << 8));
}

bool read_vlq(const std::vector<uint8_t>& d, size_t& p, uint32_t& value) {
    value = 0;
    for (int i = 0; i < 4; ++i) {
        if (p >= d.size()) {
            return false;
        }
        const uint8_t b = d[p++];
        value = (value << 7) | (b & 0x7F);
        if ((b & 0x80) == 0) {
            return true;
        }
    }
    return false;
}

std::string read_string(const std::vector<uint8_t>& d, size_t off) {
    std::string s;
    while (off < d.size() && d[off] != 0 && s.size() < 256) {
        s.push_back(static_cast<char>(d[off++]));
    }
    return s;
}

}  // namespace

bool cmf_load(const std::string& path, CmfSong& song, std::string& err) {
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
    if (!cmf_parse(data, song, err)) {
        return false;
    }
    song.filename = path;
    if (song.title.empty()) {
        const size_t slash = path.find_last_of("/\\");
        song.title = slash == std::string::npos ? path : path.substr(slash + 1);
    }
    return true;
}

bool cmf_parse(const std::vector<uint8_t>& d, CmfSong& song, std::string& err) {
    song = CmfSong();
    if (d.size() < 0x28 || std::memcmp(d.data(), "CTMF", 4) != 0) {
        err = "not a Creative Music File";
        return false;
    }
    const int minor = d[4];
    const int major = d[5];
    if (major != 1) {
        err = "unsupported CMF version";
        return false;
    }
    const size_t inst_off = rd16(d, 0x06);
    const size_t music_off = rd16(d, 0x08);
    song.ticks_per_second = rd16(d, 0x0C);
    if (song.ticks_per_second <= 0) {
        song.ticks_per_second = 96;
    }
    const size_t title_off = rd16(d, 0x0E);
    if (title_off > 0 && title_off < d.size()) {
        song.title = read_string(d, title_off);
    }

    const int count = minor >= 1 ? rd16(d, 0x24) : d[0x24];
    song.instrument_count = count < 128 ? count : 128;
    for (int i = 0; i < song.instrument_count; ++i) {
        const size_t p = inst_off + static_cast<size_t>(i) * 16;
        if (p + 11 > d.size()) {
            break;
        }
        for (int j = 0; j < 11; ++j) {
            song.instruments[i][j] = d[p + j];
        }
    }

    if (music_off == 0 || music_off >= d.size()) {
        err = "CMF has no music data";
        return false;
    }

    size_t p = music_off;
    uint32_t tick = 0;
    uint8_t running = 0;
    uint32_t last_tick = 0;
    while (p < d.size()) {
        uint32_t delta = 0;
        if (!read_vlq(d, p, delta)) {
            break;
        }
        tick += delta;
        if (p >= d.size()) {
            break;
        }
        uint8_t b = d[p];
        if (b == 0xFF) {  // meta event
            ++p;
            if (p >= d.size()) {
                break;
            }
            const uint8_t type = d[p++];
            uint32_t len = 0;
            if (!read_vlq(d, p, len)) {
                break;
            }
            p += len;
            if (type == 0x2F) {  // end of track
                song.end_tick = tick;
                break;
            }
            continue;
        }
        if (b == 0xF0 || b == 0xF7) {  // sysex: skipped like the original
            ++p;
            uint32_t len = 0;
            if (!read_vlq(d, p, len)) {
                break;
            }
            p += len;
            continue;
        }

        uint8_t status = 0;
        if (b & 0x80) {
            status = b;
            ++p;
            if (status < 0xF0) {
                running = status;
            }
        } else {
            status = running;
            if (status == 0) {
                break;
            }
        }
        const int hi = status & 0xF0;
        const uint8_t chan = status & 0x0F;

        if (hi == 0xC0 || hi == 0xD0) {
            if (p >= d.size()) {
                break;
            }
            const uint8_t a = d[p++];
            if (hi == 0xC0) {
                song.events.push_back(CmfEvent{tick, 0xC0, chan, a, 0});
            }
            continue;
        }
        if (hi != 0x80 && hi != 0x90 && hi != 0xA0 && hi != 0xB0 && hi != 0xE0) {
            p += 1;  // unknown message: consume conservatively
            continue;
        }
        if (p + 1 >= d.size()) {
            break;
        }
        const uint8_t a = d[p++];
        const uint8_t bval = d[p++];
        if (hi == 0x90 && bval == 0) {
            song.events.push_back(CmfEvent{tick, 0x80, chan, a, 0});
        } else if (hi == 0x90) {
            song.events.push_back(CmfEvent{tick, 0x90, chan, a, bval});
        } else if (hi == 0x80) {
            song.events.push_back(CmfEvent{tick, 0x80, chan, a, 0});
        } else if (hi == 0xB0) {
            song.events.push_back(CmfEvent{tick, 0xB0, chan, a, bval});
        }
        last_tick = tick;
    }

    if (song.end_tick == 0) {
        song.end_tick = last_tick + 1;
    }
    if (song.events.empty()) {
        err = "CMF has no playable events";
        return false;
    }
    return true;
}
