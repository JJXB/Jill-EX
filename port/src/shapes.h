#pragma once
#include <cstdint>
#include <string>
#include <vector>

// One decoded shape frame, already translated to palette indices (video mode 4,
// 8 bpp).  The decoder follows docs/jill1-engine.md #9.
struct ShapeFrame {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> px;
};

struct ShapeEntry {
    bool loaded = false;
    int bpp = 0;
    int flags = 0;
    std::vector<ShapeFrame> frames;
};

class ShapeBank {
public:
    bool load(const std::string& path, std::string& err);

    // A shape id is `tileset << 8 | frame`.  The tileset byte may carry bit
    // 0x40 (0x4000 in the id): it is part of the id, not a mirror flag - the
    // runtime XORs it out and selects .sha entry (tileset & 0x3F).
    const ShapeFrame* frame(int entry, int index);
    const ShapeEntry* entry(int index);

private:
    bool decode(int index, std::string& err);

    std::vector<uint8_t> file_;
    uint32_t offsets_[128] = {};
    uint16_t sizes_[128] = {};
    std::vector<ShapeEntry> entries_;
};

// Read just one entry's colour-translation table (the 16-entry table stored
// after the entry header for bpp 4/1 shapes).  The Jill episodes share the
// player art bytes but use different colour tables, which is what changes
// Jill's outfit between episodes.  Returns false for raw/8-bpp entries.
bool read_entry_xlate(const std::string& path, int entry, uint8_t out[16]);
