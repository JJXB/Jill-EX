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

bool Board::parse(const uint8_t* d, size_t dsize, const std::string& label,
                  bool allow_extra, std::string& err) {
    if (dsize < 0x4000 + 2 + kHeaderSize) {
        err = label + " is too small for a board";
        return false;
    }
    for (int i = 0; i < kCols * kRows; ++i) {
        map[i] = rd16(&d[i * 2]);
    }
    size_t p = 0x4000;
    int count = rd16(&d[p]);
    p += 2;
    if (count < 1 || count > kMaxObjects) {
        err = label + ": object count out of range";
        return false;
    }
    size_t need = p + static_cast<size_t>(count) * 31 + kHeaderSize;
    if (need > dsize) {
        err = label + ": truncated object block";
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
        if (sp + 2 > dsize) {
            err = label + ": truncated string";
            return false;
        }
        int len = rd16(&d[sp]);
        sp += 2;
        if (sp + static_cast<size_t>(len) + 1 > dsize) {
            err = label + ": truncated string body";
            return false;
        }
        objects[i].text.assign(reinterpret_cast<const char*>(&d[sp]),
                               static_cast<size_t>(len));
        sp += static_cast<size_t>(len) + 1;
    }
    extra.clear();
    if (sp != dsize && allow_extra) {
        // Port save tail: magic + u32 length + payload.  Only save files carry
        // it; a shipped board with trailing bytes is still an error.
        if (sp + 8 <= dsize && d[sp] == 'J' && d[sp + 1] == 'L' &&
            d[sp + 2] == 'X' && d[sp + 3] == '1') {
            const size_t n = static_cast<size_t>(d[sp + 4]) |
                             (static_cast<size_t>(d[sp + 5]) << 8) |
                             (static_cast<size_t>(d[sp + 6]) << 16) |
                             (static_cast<size_t>(d[sp + 7]) << 24);
            sp += 8;
            if (sp + n != dsize) {
                err = label + ": truncated extra block";
                return false;
            }
            extra.assign(d + sp, d + sp + n);
            sp = dsize;
        }
    }
    if (sp != dsize) {
        err = label + ": trailing bytes after string block";
        return false;
    }
    return true;
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
    std::vector<uint8_t> d(size > 0 ? static_cast<size_t>(size) : 0);
    size_t got = d.empty() ? 0 : std::fread(d.data(), 1, d.size(), f);
    std::fclose(f);
    if (got != d.size()) {
        err = "short read on " + path;
        return false;
    }
    return parse(d.data(), d.size(), path, true, err);
}

void Board::encode(std::vector<uint8_t>& out) const {
    out.clear();
    out.reserve(0x4000 + 2 + objects.size() * 31 + kHeaderSize + 64);
    auto wr16 = [&](uint16_t v) {
        out.push_back(static_cast<uint8_t>(v & 0xFF));
        out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    };
    for (int i = 0; i < kCols * kRows; ++i) {
        wr16(map[static_cast<size_t>(i)]);
    }
    wr16(static_cast<uint16_t>(objects.size()));
    for (const BoardObject& o : objects) {
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
        out.insert(out.end(), r, r + sizeof(r));
    }
    out.insert(out.end(), header.begin(), header.end());
    // Strings follow the header in object order: u16 length, bytes, NUL.
    for (const BoardObject& o : objects) {
        if (o.text.empty()) {
            continue;
        }
        wr16(static_cast<uint16_t>(o.text.size()));
        out.insert(out.end(), o.text.begin(), o.text.end());
        out.push_back(0);
    }
}

bool Board::save(const std::string& path, std::string& err) const {
    std::vector<uint8_t> buf;
    encode(buf);
    if (!extra.empty()) {
        // Port-only tail, used only by save files: the hub board that was
        // stashed when a level was entered.  Board::load accepts it; the
        // game's own loader never sees it.
        const size_t n = extra.size();
        buf.push_back('J');
        buf.push_back('L');
        buf.push_back('X');
        buf.push_back('1');
        buf.push_back(static_cast<uint8_t>(n & 0xFF));
        buf.push_back(static_cast<uint8_t>((n >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>((n >> 16) & 0xFF));
        buf.push_back(static_cast<uint8_t>((n >> 24) & 0xFF));
        buf.insert(buf.end(), extra.begin(), extra.end());
    }
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        err = "cannot write " + path;
        return false;
    }
    const size_t wr = std::fwrite(buf.data(), 1, buf.size(), f);
    const bool closed = std::fclose(f) == 0;
    if (wr != buf.size() || !closed) {
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
