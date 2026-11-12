#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "profile.h"
#include "shapes.h"
#include "tiles.h"

// An external image decoded to 8-bit RGBA.  Animated GIF/APNG files decode to
// their default (first) frame; sprite animation is driven by the game's own
// shape ids, so a pack supplies a strip of frames instead of an animated file.
struct ExternalImage {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> rgba;
};

// One selectable pack: a folder or a .zip under `<base>/external`.
struct PackInfo {
    std::string id;       // folder/zip stem, used in jill.ini
    std::string path;     // full path on disk
    std::string name;     // display name ([pack] name, or the stem)
    std::string game;     // optional game id restriction ("" = any)
    bool is_zip = false;
};

// Mounts one pack at a time and exposes its files by relative path
// (`sprites/kind_4.png`, `music/funky.ogg`, ...).  Folder packs are read on
// demand; zip packs are mounted in memory through miniz.
class ExternalAssets {
public:
    // `base` is the `external` directory next to jill.ini.  Missing folders
    // (including `autoload`) are created empty so pack authors have somewhere
    // to drop files.  Everything inside `autoload` is always active, under the
    // mounted pack; a pack file of the same name wins.
    bool scan(const std::string& base, std::string& err);
    const std::vector<PackInfo>& packs() const { return packs_; }
    int find(const std::string& id) const;
    int active_index() const { return active_; }
    const PackInfo* active() const;
    bool autoload() const { return autoload_; }
    const std::string& autoload_root() const { return autoload_root_; }

    // Mount/unmount a pack.  Mounting a zip parses its central directory.
    // With no pack mounted the autoload folder still supplies files.
    bool mount(int index, std::string& err);
    void unmount();
    bool mounted() const { return active_ >= 0 || autoload_; }
    const std::string& root() const { return root_; }

    bool has(const std::string& rel) const;
    bool read(const std::string& rel, std::vector<uint8_t>& out) const;
    // All files under `prefix` (lower-case relative paths, sorted).
    std::vector<std::string> files(const std::string& prefix) const;

    // Pack manifest helpers.  Both look at `[sfx]` / `[music]` first and then
    // at the conventional auto-discovered names.
    bool find_sfx(int slot, std::string& rel) const;
    bool find_music(const std::string& base, std::string& rel) const;
    // Returns the first existing relative path from a list of candidates.
    bool first_existing(const std::vector<std::string>& candidates,
                        std::string& rel) const;

    const IniFile& manifest() const { return manifest_; }

    // Load and decode an image (PNG/JPG/APNG/GIF) from the mounted pack.
    bool load_image(const std::string& rel, ExternalImage& out,
                    std::string& err) const;

    // Extensions the image loader understands, in probe order.
    static const std::vector<std::string>& image_exts();
    static const std::vector<std::string>& audio_exts();

private:
    bool read_folder(const std::string& rel, std::vector<uint8_t>& out) const;
    bool read_autoload(const std::string& rel, std::vector<uint8_t>& out) const;

    std::string root_;
    std::string autoload_root_;
    bool autoload_ = false;
    std::vector<PackInfo> packs_;
    int active_ = -1;
    IniFile manifest_;
    IniFile autoload_manifest_;

    // Zip state (valid while a zip pack is mounted).
    bool zip_open_ = false;
    std::vector<uint8_t> zip_data_;
    std::unordered_map<std::string, std::string> zip_names_;  // lower -> real
    void* zip_ = nullptr;  // mz_zip_archive*, hidden from the header
};

// A palette-indexed overlay picture (HUD panel or text-page art).  `mask` is
// 1 where the source pixel is opaque; 0 leaves the framebuffer untouched.
struct ExternalPicture {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> px;
    std::vector<uint8_t> mask;
};

// One selectable HUD panel.  `classic` is the 64x84 top-left panel, otherwise
// it is the 320x12 modern bar.  The name comes from `hud/<name>.png` (or
// `hud/classic_<name>.png` / `hud/modern_<name>.png`).
struct ExternalHud {
    std::string name;
    bool classic = true;
    ExternalPicture picture;
};

// A complete font set built from `fonts/<name>.png` (6 px bank 2) and/or
// `fonts/<name>_8.png` (8 px bank 1).
struct ExternalFont {
    std::string name;
    std::unordered_map<int, ShapeFrame> frames;  // (entry << 8) | code
};

// One selectable menu border: shape entry 7's nine-slice frames, keyed 1..9.
struct ExternalBorder {
    std::string name;
    std::unordered_map<int, ShapeFrame> frames;
};

// Palette-indexed overrides ready for the engine's 8-bit framebuffer.
class ExternalVisuals {
public:
    void clear();
    // Build every override for the mounted pack.  Silently skips assets that
    // fail to decode (errors are printed once per asset).
    bool build(const ExternalAssets& ext, ShapeBank& shapes,
               const TileDefs& tiles, const ProfileDef& profile,
               const uint8_t palette[768], int board_w, int board_h);
    bool empty() const {
        return shapes_.empty() && boards_.empty() && !has_hud_classic_ &&
               !has_hud_modern_ && huds_.empty() && borders_.empty() &&
               fonts_.empty() && pages_.empty() && !has_portrait_;
    }

    const ShapeFrame* shape(int id) const;
    const std::vector<uint8_t>* board(const std::string& name) const;
    const std::vector<std::vector<uint8_t>>& themes() const { return themes_; }
    const std::vector<std::string>& theme_names() const { return theme_names_; }
    const ExternalPicture* hud_classic() const {
        return has_hud_classic_ ? &hud_classic_ : nullptr;
    }
    const ExternalPicture* hud_modern() const {
        return has_hud_modern_ ? &hud_modern_ : nullptr;
    }
    // Named HUD panels, in discovery order.  `classic` filters to the style.
    const std::vector<ExternalHud>& huds() const { return huds_; }
    const ExternalPicture* hud(const std::string& name, bool classic) const;
    const std::vector<std::string>& border_names() const {
        return border_names_;
    }
    const ShapeFrame* border_shape(int index, int frame) const;
    const std::vector<std::string>& font_names() const { return font_names_; }
    const ShapeFrame* font_shape(const std::string& name, int entry,
                                 int code) const;
    const ExternalPicture* text_page(int number) const {
        auto it = pages_.find(number);
        return it == pages_.end() ? nullptr : &it->second;
    }
    const ExternalPicture* portrait() const {
        return has_portrait_ ? &portrait_ : nullptr;
    }

private:
    std::unordered_map<int, ShapeFrame> shapes_;
    std::unordered_map<std::string, std::vector<uint8_t>> boards_;
    std::vector<std::vector<uint8_t>> themes_;
    std::vector<std::string> theme_names_;
    ExternalPicture hud_classic_;
    ExternalPicture hud_modern_;
    bool has_hud_classic_ = false;
    bool has_hud_modern_ = false;
    std::vector<ExternalHud> huds_;
    std::vector<ExternalBorder> borders_;
    std::vector<std::string> border_names_;
    std::vector<ExternalFont> fonts_;
    std::vector<std::string> font_names_;
    ExternalPicture portrait_;
    bool has_portrait_ = false;
    std::unordered_map<int, ExternalPicture> pages_;
};
