#include "board.h"

#include <cstdio>

namespace {

uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

// x/y/dx/dy are the only signed 16-bit fields in the record (docs #6.1).
int16_t rd16s(const uint8_t* p) {
    return static_cast<int16_t>(rd16(p));
}

}  // namespace

std::string Board::item_ids() const {
    std::string out;
    int n = item_count_slot();
    for (int i = 0; i < n && i < 16; ++i) {
        int id = item_id(i);
        out.push_back(static_cast<char>('0' + (id % 10)));
    }
    return out;
}

bool Board::load(const std::string& path, std::string& err) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size < 0x4000 + 2 + kHeaderSize) {
        std::fclose(f);
        err = path + " is too small for a board";
        return false;
    }
    std::vector<uint8_t> d(static_cast<size_t>(size));
    size_t got = std::fread(d.data(), 1, d.size(), f);
    std::fclose(f);
    if (got != d.size()) {
        err = "short read on " + path;
        return false;
    }

    for (int i = 0; i < kCols * kRows; ++i) {
        map[i] = rd16(&d[i * 2]);
    }
    size_t p = 0x4000;
    int count = rd16(&d[p]);
    p += 2;
    if (count < 1 || count > kMaxObjects) {
        err = path + ": object count out of range";
        return false;
    }
    size_t need = p + static_cast<size_t>(count) * 31 + kHeaderSize;
    if (need > d.size()) {
        err = path + ": truncated object block";
        return false;
    }

    objects.clear();
    objects.reserve(static_cast<size_t>(count));
    std::vector<bool> has_string(count, false);
    for (int i = 0; i < count; ++i) {
        const uint8_t* r = &d[p + static_cast<size_t>(i) * 31];
        BoardObject o;
        o.kind = r[0];
        o.x = rd16s(r + 1);
        o.y = rd16s(r + 3);
        o.dx = rd16s(r + 5);
        o.dy = rd16s(r + 7);
        o.w = rd16(r + 9);
        o.h = rd16(r + 0xB);
        o.state = static_cast<int8_t>(r[0xD]);
        o.p0f = rd16(r + 0xF);
        o.p11 = rd16(r + 0x11);
        o.p13 = rd16(r + 0x13);
        o.flags = rd16(r + 0x15);
        o.p1b = rd16(r + 0x1B);
        o.ttl = rd16(r + 0x1D);
        if ((rd16(r + 0x17) | rd16(r + 0x19)) != 0) {
            has_string[static_cast<size_t>(i)] = true;
        }
        objects.push_back(std::move(o));
    }
    p += static_cast<size_t>(count) * 31;
    for (int i = 0; i < kHeaderSize; ++i) {
        header[i] = d[p + static_cast<size_t>(i)];
    }
    p += kHeaderSize;

    // Strings follow the header, one per object that carries the flag word,
    // in object order: u16 length, `length` bytes, NUL.
    size_t sp = p;
    for (int i = 0; i < count; ++i) {
        if (!has_string[static_cast<size_t>(i)]) {
            continue;
        }
        if (sp + 2 > d.size()) {
            err = path + ": truncated string";
            return false;
        }
        int len = rd16(&d[sp]);
        sp += 2;
        if (sp + static_cast<size_t>(len) + 1 > d.size()) {
            err = path + ": truncated string body";
            return false;
        }
        objects[i].text.assign(reinterpret_cast<const char*>(&d[sp]),
                               static_cast<size_t>(len));
        sp += static_cast<size_t>(len) + 1;
    }
    if (sp != d.size()) {
        err = path + ": trailing bytes after string block";
        return false;
    }
    return true;
}

bool Board::save(const std::string& path, std::string& err) const {
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        err = "cannot write " + path;
        return false;
    }
    bool ok = true;
    auto wr8 = [&](uint8_t v) {
        if (ok && std::fputc(v, f) == EOF) {
            ok = false;
        }
    };
    auto wr16 = [&](uint16_t v) {
        wr8(static_cast<uint8_t>(v & 0xFF));
        wr8(static_cast<uint8_t>((v >> 8) & 0xFF));
    };

    for (int i = 0; i < kCols * kRows && ok; ++i) {
        wr16(map[static_cast<size_t>(i)]);
    }
    wr16(static_cast<uint16_t>(objects.size()));
    for (const BoardObject& o : objects) {
        if (!ok) {
            break;
        }
        uint8_t r[31] = {};
        r[0] = static_cast<uint8_t>(o.kind);
        // x/y/dx/dy are signed 16-bit; the rest are unsigned except state.
        uint16_t fields[15] = {
            static_cast<uint16_t>(static_cast<int16_t>(o.x)),
            static_cast<uint16_t>(static_cast<int16_t>(o.y)),
            static_cast<uint16_t>(static_cast<int16_t>(o.dx)),
            static_cast<uint16_t>(static_cast<int16_t>(o.dy)),
            static_cast<uint16_t>(o.w),
            static_cast<uint16_t>(o.h),
            static_cast<uint16_t>(static_cast<uint8_t>(o.state)),
            static_cast<uint16_t>(o.p0f),
            static_cast<uint16_t>(o.p11),
            static_cast<uint16_t>(o.p13),
            static_cast<uint16_t>(o.flags),
            // The loader only tests these two words for non-zero; it allocates
            // fresh string storage and rewrites them on load.
            static_cast<uint16_t>(o.text.empty() ? 0 : 1),
            static_cast<uint16_t>(0),
            static_cast<uint16_t>(o.p1b),
            static_cast<uint16_t>(o.ttl)};
        size_t p = 1;
        for (uint16_t v : fields) {
            r[p++] = static_cast<uint8_t>(v & 0xFF);
            r[p++] = static_cast<uint8_t>((v >> 8) & 0xFF);
        }
        if (std::fwrite(r, 1, sizeof(r), f) != sizeof(r)) {
            ok = false;
        }
    }
    if (ok) {
        if (std::fwrite(header.data(), 1, header.size(), f) != header.size()) {
            ok = false;
        }
    }
    // Strings follow the header in object order: u16 length, bytes, NUL.
    for (const BoardObject& o : objects) {
        if (!ok) {
            break;
        }
        if (o.text.empty()) {
            continue;
        }
        wr16(static_cast<uint16_t>(o.text.size()));
        if (ok && std::fwrite(o.text.data(), 1, o.text.size(), f) !=
                      o.text.size()) {
            ok = false;
        }
        wr8(0);
    }
    if (std::fclose(f) != 0) {
        ok = false;
    }
    if (!ok) {
        err = "short write on " + path;
        return false;
    }
    return true;
}

bool Board::looks_like_board(const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fclose(f);
    if (size < 0x4000 + 2 + kHeaderSize) {
        return false;
    }
    f = std::fopen(path.c_str(), "rb");
    if (!f) {
        return false;
    }
    uint8_t buf[2];
    std::fseek(f, 0x4000, SEEK_SET);
    bool ok = std::fread(buf, 1, 2, f) == 2;
    std::fclose(f);
    if (!ok) {
        return false;
    }
    int count = buf[0] | (buf[1] << 8);
    return count >= 1 && count <= kMaxObjects;
}
