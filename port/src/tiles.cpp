#include "tiles.h"

#include <cstdio>

bool TileDefs::load(const std::string& path, std::string& err) {
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size < 8) {
        std::fclose(f);
        err = path + " is not a .dma file";
        return false;
    }
    std::string data(static_cast<size_t>(size), '\0');
    size_t got = std::fread(&data[0], 1, data.size(), f);
    std::fclose(f);
    if (got != data.size()) {
        err = "short read on " + path;
        return false;
    }

    // Record 0 is the default entry (id 0, shape 0x4700, attr 0x0201, name "0").
    for (TileInfo& t : tiles_) {
        t.shape = 0x4700;
        t.flags = static_cast<uint16_t>(0x4006 ^ 0x0201);
        t.name.clear();
    }
    base_shapes_.fill(0x4700);

    size_t p = 0;
    int records = 0;
    while (p + 7 <= data.size()) {
        uint16_t id = static_cast<uint8_t>(data[p]) |
                      (static_cast<uint16_t>(static_cast<uint8_t>(data[p + 1])) << 8);
        uint16_t shape = static_cast<uint8_t>(data[p + 2]) |
                         (static_cast<uint16_t>(static_cast<uint8_t>(data[p + 3])) << 8);
        uint16_t attr = static_cast<uint8_t>(data[p + 4]) |
                        (static_cast<uint16_t>(static_cast<uint8_t>(data[p + 5])) << 8);
        int name_len = static_cast<uint8_t>(data[p + 6]);
        p += 7;
        if (p + static_cast<size_t>(name_len) > data.size()) {
            err = "truncated .dma record";
            return false;
        }
        if (id < kCount) {
            TileInfo& t = tiles_[id];
            t.shape = shape;
            t.flags = static_cast<uint16_t>(0x4006 ^ attr);
            t.name.assign(data, p, static_cast<size_t>(name_len));
            base_shapes_[id] = shape;
        }
        p += static_cast<size_t>(name_len);
        ++records;
    }
    if (records == 0) {
        err = "no records in " + path;
        return false;
    }
    return true;
}
