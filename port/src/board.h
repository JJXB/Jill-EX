#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

// Object record, 31 bytes in the board file and in DGROUP.  Fields match
// docs/jill1-engine.md #6.1; offsets are listed so the documentation stays
// traceable.
struct BoardObject {
    int kind = 0;        // +0x00
    int x = 0;           // +0x01
    int y = 0;           // +0x03
    int dx = 0;          // +0x05
    int dy = 0;          // +0x07
    int w = 0;           // +0x09
    int h = 0;           // +0x0B
    int state = 0;       // +0x0D
    int p0f = 0;         // +0x0F
    int p11 = 0;         // +0x11
    int p13 = 0;         // +0x13
    int flags = 0;       // +0x15
    int p1b = 0;         // +0x1B
    int ttl = 0;         // +0x1D
    // Player only: frames left in the pull-up ("raise") that finishes a climb.
    // Visual state; the documented state 3 still ends in state 0 as written.
    int raise = 0;
    // High-framerate presentation only: position at the previous logic tick.
    int prev_x = 0;
    int prev_y = 0;
    std::string text;    // attached board string, if any
};

struct Board {
    static constexpr int kCols = 128;
    static constexpr int kRows = 64;
    static constexpr int kHeaderSize = 0x46;

    std::array<uint16_t, kCols * kRows> map{};
    std::vector<BoardObject> objects;
    std::array<uint8_t, kHeaderSize> header{};

    bool load(const std::string& path, std::string& err);
    // Write the live board back in the on-disk board layout.  This is exactly
    // what the original's save routine (FUN_22ab_0c9a) does, so a port save is
    // a normal board file (JN1SAVE.n and friends).
    bool save(const std::string& path, std::string& err) const;

    int level_id() const { return header[0] | (header[1] << 8); }
    int health() const { return header[2] | (header[3] << 8); }
    int item_count_slot() const { return header[4] | (header[5] << 8); }
    int item_id(int i) const { return header[6 + i * 2] | (header[7 + i * 2] << 8); }
    std::string item_ids() const;

    static bool looks_like_board(const std::string& path);
};
