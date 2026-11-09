#include "shapes.h"

#include <cstdio>
#include <cstring>

namespace {

uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

}  // namespace

bool ShapeBank::load(const std::string& path, std::string& err) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size < 0x300) {
        std::fclose(f);
        err = path + " is too small to be a .sha bank";
        return false;
    }
    file_.resize(static_cast<size_t>(size));
    if (std::fread(file_.data(), 1, file_.size(), f) != file_.size()) {
        std::fclose(f);
        err = "short read on " + path;
        return false;
    }
    std::fclose(f);

    for (int i = 0; i < 128; ++i) {
        offsets_[i] = rd32(&file_[i * 4]);
        sizes_[i] = rd16(&file_[512 + i * 2]);
        if (sizes_[i] && (offsets_[i] > file_.size() ||
                          sizes_[i] > file_.size() - offsets_[i])) {
            err = path + ": shape directory entry out of range";
            return false;
        }
    }
    entries_.assign(128, ShapeEntry{});
    return true;
}

bool ShapeBank::decode(int index, std::string& err) {
    ShapeEntry& e = entries_[index];
    if (e.loaded) {
        return true;
    }
    e.loaded = true;
    if (!sizes_[index]) {
        return true;
    }
    const uint8_t* d = file_.data() + offsets_[index];
    const size_t n = sizes_[index];
    if (n < 12) {
        err = "short shape entry";
        return false;
    }

    int count = d[0];
    e.bpp = d[9];
    e.flags = rd16(d + 10);
    size_t pos = 12;

    uint8_t xlate[256];
    for (int i = 0; i < 256; ++i) {
        xlate[i] = static_cast<uint8_t>(i);
    }
    if (!(e.flags & 1) && e.bpp < 8) {
        int colors = 1 << e.bpp;
        if (pos + static_cast<size_t>(colors) * 4 > n) {
            err = "short colour table";
            return false;
        }
        for (int i = 0; i < colors; ++i) {
            // Methods 2/3 use shift 8/12, mode 4 uses bits 16..23.
            xlate[i] = static_cast<uint8_t>((rd32(d + pos) >> 16) & 0xFF);
            pos += 4;
        }
    }

    for (int fi = 0; fi < count; ++fi) {
        if (pos + 3 > n) {
            err = "short frame header";
            return false;
        }
        int w = d[pos + 0];
        int h = d[pos + 1];
        int op = d[pos + 2];
        pos += 3;

        ShapeFrame fr;
        fr.w = w;
        fr.h = h;
        fr.px.assign(static_cast<size_t>(w) * h, 0);

        if (op == 0) {
            if (pos + static_cast<size_t>(w) * h > n) {
                err = "short raw frame";
                return false;
            }
            for (size_t i = 0; i < fr.px.size(); ++i) {
                fr.px[i] = xlate[d[pos + i]];
            }
            pos += fr.px.size();
        } else if (op == 1) {
            size_t bits = static_cast<size_t>(w) * h * e.bpp;
            size_t bytes = (bits + 7) / 8;
            if (pos + bytes > n) {
                err = "short packed frame";
                return false;
            }
            size_t bitpos = 0;
            for (size_t i = 0; i < fr.px.size(); ++i) {
                int v = 0;
                for (int b = 0; b < e.bpp; ++b, ++bitpos) {
                    v |= ((d[pos + bitpos / 8] >> (bitpos % 8)) & 1) << b;
                }
                fr.px[i] = xlate[v];
            }
            pos += bytes;
        }
        e.frames.push_back(std::move(fr));
    }
    return true;
}

const ShapeEntry* ShapeBank::entry(int index) {
    if (index < 0 || index >= 128 || !sizes_[index]) {
        return nullptr;
    }
    std::string err;
    if (!decode(index, err)) {
        std::fprintf(stderr, "shape %d: %s\n", index, err.c_str());
        return nullptr;
    }
    return &entries_[index];
}

const ShapeFrame* ShapeBank::frame(int entry_index, int index) {
    const ShapeEntry* e = entry(entry_index);
    if (!e || index < 0 || static_cast<size_t>(index) >= e->frames.size()) {
        return nullptr;
    }
    return &e->frames[index];
}

bool read_entry_xlate(const std::string& path, int entry, uint8_t out[16]) {
    for (int i = 0; i < 16; ++i) {
        out[i] = static_cast<uint8_t>(i);
    }
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        return false;
    }
    uint8_t dir[512];
    if (std::fread(dir, 1, sizeof(dir), f) != sizeof(dir)) {
        std::fclose(f);
        return false;
    }
    const uint32_t off = rd32(&dir[entry * 4]);
    uint8_t szbuf[2];
    std::fseek(f, 512 + entry * 2, SEEK_SET);
    if (std::fread(szbuf, 1, 2, f) != 2) {
        std::fclose(f);
        return false;
    }
    const uint16_t size = rd16(szbuf);
    if (!size || size < 12) {
        std::fclose(f);
        return false;
    }
    uint8_t hdr[12];
    std::fseek(f, static_cast<long>(off), SEEK_SET);
    if (std::fread(hdr, 1, sizeof(hdr), f) != sizeof(hdr)) {
        std::fclose(f);
        return false;
    }
    const int bpp = hdr[9];
    const int flags = rd16(hdr + 10);
    if ((flags & 1) || bpp >= 8 || bpp < 1) {
        std::fclose(f);
        return false;
    }
    const int colors = 1 << bpp;
    if (colors > 16) {
        std::fclose(f);
        return false;
    }
    uint8_t table[16 * 4];
    if (std::fread(table, 1, static_cast<size_t>(colors) * 4, f) !=
        static_cast<size_t>(colors) * 4) {
        std::fclose(f);
        return false;
    }
    std::fclose(f);
    for (int i = 0; i < colors; ++i) {
        out[i] = static_cast<uint8_t>((rd32(&table[i * 4]) >> 16) & 0xFF);
    }
    return true;
}
