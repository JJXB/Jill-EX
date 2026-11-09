#pragma once
#include <array>
#include <cstdint>
#include <string>

// One row of jill.dma.  `shape` is the virtual shape id (tileset << 8 | frame);
// `flags` is the collision word the engine keeps at DGROUP:0x83EE, i.e.
// 0x4006 ^ attr (docs/jill1-engine.md #8).
struct TileInfo {
    uint16_t shape = 0x4700;
    uint16_t flags = 0x4207;
    std::string name;
};

class TileDefs {
public:
    static constexpr int kCount = 600;

    bool load(const std::string& path, std::string& err);
    const TileInfo& at(int id) const {
        return (id >= 0 && id < kCount) ? tiles_[id] : tiles_[0];
    }
    // Tile-script hooks (1F28:0006) rewrite the live tile record, exactly like
    // the original's table at DGROUP:0x83EC + id*8.
    TileInfo& at(int id) { return (id >= 0 && id < kCount) ? tiles_[id] : tiles_[0]; }
    // Shape recorded by jill.dma, before any per-frame animation rewrite.
    uint16_t base_shape(int id) const {
        return (id >= 0 && id < kCount) ? base_shapes_[id] : base_shapes_[0];
    }

private:
    std::array<TileInfo, kCount> tiles_;
    std::array<uint16_t, kCount> base_shapes_{};
};
