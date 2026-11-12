#include "external.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <unordered_set>

#define STB_IMAGE_IMPLEMENTATION
#include "third_party/stb_image.h"

#include "third_party/miniz.h"

namespace fs = std::filesystem;

namespace {

std::string lower(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string norm_rel(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return lower(s);
}

std::string stem_of(const std::string& rel) {
    const size_t slash = rel.find_last_of('/');
    std::string name = slash == std::string::npos ? rel : rel.substr(slash + 1);
    const size_t dot = name.find_last_of('.');
    if (dot != std::string::npos) {
        name.erase(dot);
    }
    return lower(name);
}

std::string ext_of(const std::string& rel) {
    const size_t dot = rel.find_last_of('.');
    return dot == std::string::npos ? std::string() : lower(rel.substr(dot + 1));
}

bool parse_int(const std::string& s, long& out) {
    if (s.empty()) {
        return false;
    }
    char* end = nullptr;
    out = std::strtol(s.c_str(), &end, 0);
    return end && *end == '\0';
}

// Nearest-colour palette mapping with a small cache (external art is RGB while
// the engine framebuffer is 8-bit indexed).
struct PaletteMap {
    const uint8_t* pal = nullptr;
    std::unordered_map<uint32_t, uint8_t> cache;

    uint8_t nearest(int r, int g, int b) {
        const uint32_t key = (static_cast<uint32_t>(r) << 16) |
                             (static_cast<uint32_t>(g) << 8) |
                             static_cast<uint32_t>(b);
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
        int best = 0;
        long best_d = 1L << 30;
        for (int i = 0; i < 256; ++i) {
            const int dr = r - pal[i * 3 + 0];
            const int dg = g - pal[i * 3 + 1];
            const int db = b - pal[i * 3 + 2];
            const long d = static_cast<long>(dr) * dr +
                           static_cast<long>(dg) * dg +
                           static_cast<long>(db) * db;
            if (d < best_d) {
                best_d = d;
                best = i;
                if (d == 0) {
                    break;
                }
            }
        }
        cache[key] = static_cast<uint8_t>(best);
        return static_cast<uint8_t>(best);
    }
};

// Box-filter resize so high-resolution pack art downsamples cleanly.
void resize_rgba(const uint8_t* src, int sw, int sh, std::vector<uint8_t>& out,
                 int tw, int th) {
    out.assign(static_cast<size_t>(tw) * th * 4, 0);
    if (sw <= 0 || sh <= 0 || tw <= 0 || th <= 0) {
        return;
    }
    for (int ty = 0; ty < th; ++ty) {
        const int y0 = ty * sh / th;
        const int y1 = std::max(y0 + 1, (ty + 1) * sh / th);
        for (int tx = 0; tx < tw; ++tx) {
            const int x0 = tx * sw / tw;
            const int x1 = std::max(x0 + 1, (tx + 1) * sw / tw);
            unsigned sum[4] = {0, 0, 0, 0};
            unsigned n = 0;
            for (int y = y0; y < y1 && y < sh; ++y) {
                for (int x = x0; x < x1 && x < sw; ++x) {
                    const uint8_t* p = src + (static_cast<size_t>(y) * sw + x) * 4;
                    sum[0] += p[0];
                    sum[1] += p[1];
                    sum[2] += p[2];
                    sum[3] += p[3];
                    ++n;
                }
            }
            uint8_t* d = out.data() + (static_cast<size_t>(ty) * tw + tx) * 4;
            if (n == 0) {
                continue;
            }
            for (int c = 0; c < 4; ++c) {
                d[c] = static_cast<uint8_t>(sum[c] / n);
            }
        }
    }
}

bool image_has_alpha(const std::vector<uint8_t>& rgba) {
    for (size_t i = 3; i < rgba.size(); i += 4) {
        if (rgba[i] < 255) {
            return true;
        }
    }
    return false;
}

// Convert (a cropped/resized region of) an RGBA image to palette indices.
// Transparent pixels become index 0 (the engine's sprite key colour).
void to_frame(const std::vector<uint8_t>& rgba, int w, int h,
              const uint8_t palette[768], PaletteMap& pm, ShapeFrame& out,
              bool has_alpha, bool colour_key = true) {
    out.w = w;
    out.h = h;
    out.px.assign(static_cast<size_t>(w) * h, 0);
    if (w <= 0 || h <= 0) {
        return;
    }
    int key_r = rgba[0], key_g = rgba[1], key_b = rgba[2];
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const uint8_t* p = rgba.data() + (static_cast<size_t>(y) * w + x) * 4;
            bool transparent;
            if (has_alpha) {
                transparent = p[3] < 128;
            } else {
                // Board/theme art is a full-screen picture: without an alpha
                // channel it is opaque, not keyed on its top-left colour.
                transparent = colour_key && p[0] == key_r && p[1] == key_g &&
                              p[2] == key_b;
            }
            if (transparent) {
                out.px[static_cast<size_t>(y) * w + x] = 0;
            } else {
                out.px[static_cast<size_t>(y) * w + x] =
                    pm.nearest(p[0], p[1], p[2]);
            }
        }
    }
}

bool make_frame(const ExternalImage& img, int tw, int th,
                const uint8_t palette[768], PaletteMap& pm, ShapeFrame& out) {
    if (tw <= 0 || th <= 0 || img.w <= 0 || img.h <= 0) {
        return false;
    }
    std::vector<uint8_t> scaled;
    resize_rgba(img.rgba.data(), img.w, img.h, scaled, tw, th);
    to_frame(scaled, tw, th, palette, pm, out, image_has_alpha(img.rgba));
    return true;
}

std::string zip_error(mz_zip_archive* z) {
    return mz_zip_get_error_string(mz_zip_get_last_error(z));
}

// Add every entry of `src` that `dst` does not already define.  Autoload is
// the base layer; the mounted pack overrides it key by key.
void merge_manifest(const IniFile& src, IniFile& dst) {
    for (const std::string& section : src.sections()) {
        for (const auto& kv : src.section(section)) {
            if (dst.get(section, kv.first).empty()) {
                dst.set(section, kv.first, kv.second);
            }
        }
    }
}

}  // namespace

const std::vector<std::string>& ExternalAssets::image_exts() {
    static const std::vector<std::string> kExts = {"png", "apng", "gif", "jpg",
                                                   "jpeg"};
    return kExts;
}

const std::vector<std::string>& ExternalAssets::audio_exts() {
    static const std::vector<std::string> kExts = {"ogg", "mp3", "m4a",
                                                   "wav"};
    return kExts;
}

bool ExternalAssets::scan(const std::string& base, std::string& err) {
    unmount();
    packs_.clear();
    root_ = base;
    std::error_code ec;
    fs::create_directories(base, ec);
    if (ec) {
        err = "cannot create " + base + ": " + ec.message();
        return false;
    }
    // `autoload` is never a selectable pack: its files are always mounted
    // underneath whichever pack is active.
    autoload_root_ = (fs::path(base) / "autoload").string();
    fs::create_directories(autoload_root_, ec);
    autoload_ = false;
    if (fs::is_directory(autoload_root_, ec)) {
        for (const auto& e : fs::recursive_directory_iterator(autoload_root_,
                                                              ec)) {
            if (e.is_regular_file()) {
                autoload_ = true;
                break;
            }
        }
    }
    autoload_manifest_ = IniFile{};
    manifest_ = IniFile{};
    if (autoload_) {
        std::vector<uint8_t> data;
        std::string merr;
        if (read_autoload("pack.ini", data)) {
            autoload_manifest_.parse(std::string(data.begin(), data.end()),
                                      autoload_root_ + ":pack.ini", merr);
        }
        manifest_ = autoload_manifest_;
    }
    std::vector<fs::directory_entry> entries;
    for (const auto& e : fs::directory_iterator(base, ec)) {
        entries.push_back(e);
    }
    if (ec) {
        err = "cannot read " + base + ": " + ec.message();
        return false;
    }
    std::sort(entries.begin(), entries.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  return lower(a.path().filename().string()) <
                         lower(b.path().filename().string());
              });
    for (const auto& e : entries) {
        std::error_code entry_ec;
        PackInfo p;
        p.path = e.path().string();
        if (lower(e.path().filename().string()) == "autoload") {
            continue;
        }
        if (e.is_directory(entry_ec)) {
            p.id = e.path().filename().string();
            p.name = p.id;
            p.is_zip = false;
            packs_.push_back(p);
        } else if (e.is_regular_file(entry_ec) &&
                   ext_of(p.path) == "zip") {
            p.id = e.path().stem().string();
            p.name = p.id;
            p.is_zip = true;
            packs_.push_back(p);
        }
    }
    return true;
}

int ExternalAssets::find(const std::string& id) const {
    const std::string want = lower(id);
    for (size_t i = 0; i < packs_.size(); ++i) {
        if (lower(packs_[i].id) == want) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const PackInfo* ExternalAssets::active() const {
    return (active_ >= 0 && active_ < static_cast<int>(packs_.size()))
               ? &packs_[static_cast<size_t>(active_)]
               : nullptr;
}

bool ExternalAssets::mount(int index, std::string& err) {
    unmount();
    if (index < 0 || index >= static_cast<int>(packs_.size())) {
        err = "pack index out of range";
        return false;
    }
    active_ = index;
    const PackInfo& p = packs_[static_cast<size_t>(index)];
    if (p.is_zip) {
        std::ifstream in(p.path, std::ios::binary);
        if (!in) {
            err = "cannot open " + p.path;
            active_ = -1;
            return false;
        }
        zip_data_.assign(std::istreambuf_iterator<char>(in),
                         std::istreambuf_iterator<char>());
        auto* z = new mz_zip_archive{};
        if (!mz_zip_reader_init_mem(z, zip_data_.data(), zip_data_.size(), 0)) {
            err = p.path + ": " + zip_error(z);
            delete z;
            active_ = -1;
            return false;
        }
        zip_ = z;
        zip_open_ = true;
        const mz_uint n = mz_zip_reader_get_num_files(z);
        for (mz_uint i = 0; i < n; ++i) {
            mz_zip_archive_file_stat st;
            if (!mz_zip_reader_file_stat(z, i, &st) || st.m_is_directory) {
                continue;
            }
            zip_names_[norm_rel(st.m_filename)] = st.m_filename;
        }
    }
    manifest_ = autoload_manifest_;
    std::string merr;
    std::vector<uint8_t> data;
    bool pack_has_manifest = false;
    if (p.is_zip) {
        pack_has_manifest = zip_names_.find("pack.ini") != zip_names_.end();
    } else {
        std::error_code ec;
        pack_has_manifest =
            fs::is_regular_file(fs::path(p.path) / "pack.ini", ec);
    }
    if (pack_has_manifest && read("pack.ini", data)) {
        IniFile pack_manifest;
        pack_manifest.parse(std::string(data.begin(), data.end()),
                            p.path + ":pack.ini", merr);
        merge_manifest(pack_manifest, manifest_);
        const std::string name = pack_manifest.get("pack", "name");
        if (!name.empty()) {
            packs_[static_cast<size_t>(index)].name = name;
        }
        packs_[static_cast<size_t>(index)].game =
            pack_manifest.get("pack", "game");
    }
    return true;
}

void ExternalAssets::unmount() {
    if (zip_open_ && zip_) {
        mz_zip_reader_end(static_cast<mz_zip_archive*>(zip_));
    }
    delete static_cast<mz_zip_archive*>(zip_);
    zip_ = nullptr;
    zip_open_ = false;
    zip_data_.clear();
    zip_names_.clear();
    manifest_ = autoload_manifest_;
    active_ = -1;
}

bool ExternalAssets::has(const std::string& rel) const {
    const std::string key = norm_rel(rel);
    if (active_ >= 0) {
        if (zip_open_) {
            if (zip_names_.find(key) != zip_names_.end()) {
                return true;
            }
        } else if (active() != nullptr) {
            std::error_code ec;
            if (fs::is_regular_file(fs::path(active()->path) / rel, ec)) {
                return true;
            }
        }
    }
    if (!autoload_) {
        return false;
    }
    std::error_code ec;
    return fs::is_regular_file(fs::path(autoload_root_) / rel, ec);
}

bool ExternalAssets::read(const std::string& rel,
                          std::vector<uint8_t>& out) const {
    out.clear();
    if (active_ >= 0) {
        if (zip_open_) {
            const std::string key = norm_rel(rel);
            auto it = zip_names_.find(key);
            if (it != zip_names_.end()) {
                auto* z = static_cast<mz_zip_archive*>(zip_);
                const int idx =
                    mz_zip_reader_locate_file(z, it->second.c_str(), nullptr, 0);
                if (idx >= 0) {
                    mz_zip_archive_file_stat st;
                    if (mz_zip_reader_file_stat(z, static_cast<mz_uint>(idx),
                                                &st)) {
                        out.resize(static_cast<size_t>(st.m_uncomp_size));
                        if (out.empty() ||
                            mz_zip_reader_extract_to_mem(
                                z, static_cast<mz_uint>(idx), out.data(),
                                out.size(), 0) != 0) {
                            return true;
                        }
                    }
                }
            }
        } else if (read_folder(rel, out)) {
            return true;
        }
    }
    return read_autoload(rel, out);
}

bool ExternalAssets::read_folder(const std::string& rel,
                                 std::vector<uint8_t>& out) const {
    out.clear();
    const PackInfo* p = active();
    if (!p) {
        return false;
    }
    std::ifstream in(fs::path(p->path) / rel, std::ios::binary);
    if (!in) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(in),
               std::istreambuf_iterator<char>());
    return true;
}

bool ExternalAssets::read_autoload(const std::string& rel,
                                   std::vector<uint8_t>& out) const {
    out.clear();
    if (!autoload_) {
        return false;
    }
    std::ifstream in(fs::path(autoload_root_) / rel, std::ios::binary);
    if (!in) {
        return false;
    }
    out.assign(std::istreambuf_iterator<char>(in),
               std::istreambuf_iterator<char>());
    return true;
}

std::vector<std::string> ExternalAssets::files(const std::string& prefix) const {
    std::vector<std::string> out;
    const std::string want = norm_rel(prefix);
    auto add_folder = [&](const std::string& root) {
        std::error_code ec;
        const fs::path dir = fs::path(root) / prefix;
        if (!fs::is_directory(dir, ec)) {
            return;
        }
        for (const auto& e : fs::recursive_directory_iterator(dir, ec)) {
            if (!e.is_regular_file()) {
                continue;
            }
            std::string rel = fs::relative(e.path(), root).string();
            std::replace(rel.begin(), rel.end(), '\\', '/');
            out.push_back(lower(rel));
        }
    };
    if (active_ >= 0) {
        if (zip_open_) {
            for (const auto& kv : zip_names_) {
                if (kv.first.compare(0, want.size(), want) == 0) {
                    out.push_back(kv.first);
                }
            }
        } else if (const PackInfo* p = active()) {
            add_folder(p->path);
        }
    }
    if (autoload_) {
        add_folder(autoload_root_);
    }
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

bool ExternalAssets::first_existing(
    const std::vector<std::string>& candidates, std::string& rel) const {
    for (const std::string& c : candidates) {
        if (has(c)) {
            rel = c;
            return true;
        }
    }
    return false;
}

bool ExternalAssets::find_sfx(int slot, std::string& rel) const {
    if (!mounted()) {
        return false;
    }
    const std::string key = std::to_string(slot);
    const std::string explicit_rel = manifest_.get("sfx", key);
    if (!explicit_rel.empty() && has(explicit_rel)) {
        rel = explicit_rel;
        return true;
    }
    std::vector<std::string> c;
    for (const std::string& e : audio_exts()) {
        c.push_back("sfx/" + key + "." + e);
    }
    // Also accept `<slot>_<label>` names so a pack can document itself.
    for (const std::string& f : files("sfx/")) {
        const std::string stem = stem_of(f);
        const size_t us = stem.find('_');
        if (us == std::string::npos) {
            continue;
        }
        long n = 0;
        if (parse_int(stem.substr(0, us), n) && n == slot) {
            rel = f;
            return true;
        }
    }
    return first_existing(c, rel);
}

bool ExternalAssets::find_music(const std::string& base, std::string& rel) const {
    if (!mounted()) {
        return false;
    }
    const std::string explicit_rel =
        manifest_.get("music", lower(stem_of(base)));
    if (!explicit_rel.empty() && has(explicit_rel)) {
        rel = explicit_rel;
        return true;
    }
    std::vector<std::string> c;
    for (const std::string& e : audio_exts()) {
        c.push_back("music/" + stem_of(base) + "." + e);
    }
    return first_existing(c, rel);
}

bool ExternalAssets::load_image(const std::string& rel, ExternalImage& out,
                                std::string& err) const {
    out = ExternalImage{};
    std::vector<uint8_t> data;
    if (!read(rel, data)) {
        err = "cannot read " + rel;
        return false;
    }
    int w = 0, h = 0, comp = 0;
    unsigned char* px = stbi_load_from_memory(data.data(),
                                              static_cast<int>(data.size()), &w, &h,
                                              &comp, 4);
    if (!px) {
        err = rel + ": " + (stbi_failure_reason() ? stbi_failure_reason() : "?");
        return false;
    }
    out.w = w;
    out.h = h;
    out.rgba.assign(px, px + static_cast<size_t>(w) * h * 4);
    stbi_image_free(px);
    return true;
}

// --- ExternalVisuals -------------------------------------------------------

void ExternalVisuals::clear() {
    shapes_.clear();
    boards_.clear();
    themes_.clear();
    theme_names_.clear();
    hud_classic_ = ExternalPicture{};
    hud_modern_ = ExternalPicture{};
    has_hud_classic_ = false;
    has_hud_modern_ = false;
    huds_.clear();
    borders_.clear();
    border_names_.clear();
    fonts_.clear();
    font_names_.clear();
    portrait_ = ExternalPicture{};
    has_portrait_ = false;
    pages_.clear();
}

const ShapeFrame* ExternalVisuals::shape(int id) const {
    auto it = shapes_.find(id);
    return it == shapes_.end() ? nullptr : &it->second;
}

const std::vector<uint8_t>* ExternalVisuals::board(
    const std::string& name) const {
    auto it = boards_.find(lower(name));
    return it == boards_.end() ? nullptr : &it->second;
}

const ExternalPicture* ExternalVisuals::hud(const std::string& name,
                                            bool classic) const {
    if (name.empty()) {
        return classic ? hud_classic() : hud_modern();
    }
    const std::string want = lower(name);
    for (const ExternalHud& h : huds_) {
        if (h.classic == classic && h.name == want) {
            return &h.picture;
        }
    }
    return nullptr;
}

const ShapeFrame* ExternalVisuals::border_shape(int index, int frame) const {
    if (index < 0 || index >= static_cast<int>(borders_.size())) {
        return nullptr;
    }
    const auto& m = borders_[static_cast<size_t>(index)].frames;
    auto it = m.find(frame);
    return it == m.end() ? nullptr : &it->second;
}

const ShapeFrame* ExternalVisuals::font_shape(const std::string& name, int entry,
                                              int code) const {
    if (name.empty()) {
        return nullptr;
    }
    const std::string want = lower(name);
    for (const ExternalFont& f : fonts_) {
        if (f.name != want) {
            continue;
        }
        auto it = f.frames.find((entry << 8) | (code & 0xFF));
        return it == f.frames.end() ? nullptr : &it->second;
    }
    return nullptr;
}

namespace {

void add_kind_sheet(ExternalAssets& ext, ShapeBank& shapes,
                    const ProfileDef& profile, const uint8_t palette[768],
                    PaletteMap& pm, int kind, const std::string& rel,
                    std::unordered_map<int, ShapeFrame>& out) {
    const int tileset = profile.kind(kind).tileset;
    const ShapeEntry* e = shapes.entry(tileset);
    if (!e || e->frames.empty()) {
        return;
    }
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    const int n = static_cast<int>(e->frames.size());
    if (n == 1) {
        ShapeFrame f;
        if (make_frame(img, e->frames[0].w, e->frames[0].h, palette, pm, f)) {
            out[(tileset << 8) | 0] = std::move(f);
        }
        return;
    }
    // Horizontal strip: frame i occupies columns [i*w/n, (i+1)*w/n).
    const int cw = img.w / n;
    if (cw <= 0) {
        return;
    }
    for (int i = 0; i < n; ++i) {
        ExternalImage sub;
        sub.w = cw;
        sub.h = img.h;
        sub.rgba.resize(static_cast<size_t>(cw) * img.h * 4);
        for (int y = 0; y < img.h; ++y) {
            std::memcpy(sub.rgba.data() + static_cast<size_t>(y) * cw * 4,
                        img.rgba.data() +
                            (static_cast<size_t>(y) * img.w + i * cw) * 4,
                        static_cast<size_t>(cw) * 4);
        }
        ShapeFrame f;
        if (make_frame(sub, e->frames[static_cast<size_t>(i)].w,
                       e->frames[static_cast<size_t>(i)].h, palette, pm, f)) {
            out[(tileset << 8) | (i & 0xFF)] = std::move(f);
        }
    }
}

void add_shape(ExternalAssets& ext, ShapeBank& shapes,
               const uint8_t palette[768], PaletteMap& pm, int entry, int frame,
               const std::string& rel,
               std::unordered_map<int, ShapeFrame>& out) {
    const ShapeEntry* e = shapes.entry(entry);
    if (!e || frame < 0 || frame >= static_cast<int>(e->frames.size())) {
        return;
    }
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    ShapeFrame f;
    const ShapeFrame& src = e->frames[static_cast<size_t>(frame)];
    if (make_frame(img, src.w, src.h, palette, pm, f)) {
        out[(entry << 8) | (frame & 0xFF)] = std::move(f);
    }
}

void add_tile(ExternalAssets& ext, ShapeBank& shapes,
              const TileDefs& tiles, const uint8_t palette[768], PaletteMap& pm,
              int tile_id, const std::string& rel,
              std::unordered_map<int, ShapeFrame>& out) {
    const uint16_t shape = tiles.base_shape(tile_id);
    const int entry = (shape >> 8) & 0x3F;
    const int index = shape & 0xFF;
    const ShapeEntry* e = shapes.entry(entry);
    if (!e || index >= static_cast<int>(e->frames.size())) {
        return;
    }
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    ShapeFrame f;
    const ShapeFrame& src = e->frames[static_cast<size_t>(index)];
    if (make_frame(img, src.w, src.h, palette, pm, f)) {
        out[(entry << 8) | index] = std::move(f);
    }
}

bool add_picture(ExternalAssets& ext, const uint8_t palette[768], PaletteMap& pm,
                 const std::string& rel, int w, int h,
                 std::vector<uint8_t>& out) {
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return false;
    }
    std::vector<uint8_t> scaled;
    resize_rgba(img.rgba.data(), img.w, img.h, scaled, w, h);
    ShapeFrame f;
    to_frame(scaled, w, h, palette, pm, f, image_has_alpha(img.rgba), false);
    out = std::move(f.px);
    return true;
}

bool add_overlay(ExternalAssets& ext, const uint8_t palette[768],
                 PaletteMap& pm, const std::string& rel, int w, int h,
                 ExternalPicture& out, int* src_w = nullptr,
                 int* src_h = nullptr) {
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return false;
    }
    if (src_w) {
        *src_w = img.w;
    }
    if (src_h) {
        *src_h = img.h;
    }
    std::vector<uint8_t> scaled;
    resize_rgba(img.rgba.data(), img.w, img.h, scaled, w, h);
    out = ExternalPicture{};
    out.w = w;
    out.h = h;
    if (w <= 0 || h <= 0) {
        return false;
    }
    out.px.assign(static_cast<size_t>(w) * h, 0);
    out.mask.assign(static_cast<size_t>(w) * h, 1);
    // Only real alpha defines transparency here: an opaque HUD/page picture
    // is drawn as-is, without colour-keying its corner pixel.
    const bool alpha = image_has_alpha(img.rgba);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const size_t i = static_cast<size_t>(y) * w + x;
            const uint8_t* p = scaled.data() + i * 4;
            if (alpha && p[3] < 128) {
                out.mask[i] = 0;
            } else {
                out.px[i] = pm.nearest(p[0], p[1], p[2]);
            }
        }
    }
    return true;
}

void add_font_glyph(ExternalAssets& ext, ShapeBank& shapes,
                    const uint8_t palette[768], PaletteMap& pm, int code,
                    const std::string& rel,
                    std::unordered_map<int, ShapeFrame>& out) {
    add_shape(ext, shapes, palette, pm, 2, code, rel, out);
}

void add_font_sheet(ExternalAssets& ext, ShapeBank& shapes,
                    const uint8_t palette[768], PaletteMap& pm, int entry,
                    const std::string& rel,
                    std::unordered_map<int, ShapeFrame>& out) {
    const ShapeEntry* e = shapes.entry(entry);
    if (!e || e->frames.empty()) {
        return;
    }
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    const int n = std::min(128, static_cast<int>(e->frames.size()));
    if (n == 1) {
        ShapeFrame f;
        if (make_frame(img, e->frames[0].w, e->frames[0].h, palette, pm, f)) {
            out[(entry << 8) | 0] = std::move(f);
        }
        return;
    }
    const int cw = img.w / n;
    if (cw <= 0) {
        return;
    }
    for (int i = 0; i < n; ++i) {
        ExternalImage sub;
        sub.w = cw;
        sub.h = img.h;
        sub.rgba.resize(static_cast<size_t>(cw) * img.h * 4);
        for (int y = 0; y < img.h; ++y) {
            std::memcpy(sub.rgba.data() + static_cast<size_t>(y) * cw * 4,
                        img.rgba.data() +
                            (static_cast<size_t>(y) * img.w + i * cw) * 4,
                        static_cast<size_t>(cw) * 4);
        }
        ShapeFrame f;
        if (make_frame(sub, e->frames[static_cast<size_t>(i)].w,
                       e->frames[static_cast<size_t>(i)].h, palette, pm, f)) {
            out[(entry << 8) | i] = std::move(f);
        }
    }
}

void set_hud(std::vector<ExternalHud>& huds, const std::string& name,
             bool classic, ExternalPicture pic) {
    const std::string key = lower(name);
    for (ExternalHud& h : huds) {
        if (h.classic == classic && h.name == key) {
            h.picture = std::move(pic);
            return;
        }
    }
    huds.push_back(ExternalHud{key, classic, std::move(pic)});
}

ExternalImage crop_image(const ExternalImage& src, int x, int y, int w, int h) {
    ExternalImage out;
    out.w = w;
    out.h = h;
    if (w <= 0 || h <= 0) {
        return out;
    }
    out.rgba.assign(static_cast<size_t>(w) * h * 4, 0);
    for (int sy = 0; sy < h; ++sy) {
        const int src_y = y + sy;
        if (src_y < 0 || src_y >= src.h) {
            continue;
        }
        for (int sx = 0; sx < w; ++sx) {
            const int src_x = x + sx;
            if (src_x < 0 || src_x >= src.w) {
                continue;
            }
            std::memcpy(out.rgba.data() + (static_cast<size_t>(sy) * w + sx) * 4,
                        src.rgba.data() +
                            (static_cast<size_t>(src_y) * src.w + src_x) * 4,
                        4);
        }
    }
    return out;
}

void set_border(std::vector<ExternalBorder>& borders,
                std::vector<std::string>& names, const std::string& name,
                ExternalBorder border) {
    const std::string key = lower(name);
    border.name = key;
    for (ExternalBorder& b : borders) {
        if (b.name == key) {
            b = std::move(border);
            return;
        }
    }
    borders.push_back(std::move(border));
    names.push_back(key);
}

// A border sheet is a 3x3 grid of cells.  Cell (0,0)..(2,2) map onto shape
// entry 7's frames in the original scan order, with the centre as the fill.
void add_border_sheet(ExternalAssets& ext, const uint8_t palette[768],
                      PaletteMap& pm, const std::string& rel,
                      const std::string& name,
                      std::vector<ExternalBorder>& borders,
                      std::vector<std::string>& names) {
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    if (img.w < 3 || img.h < 3) {
        return;
    }
    static const int kMap[3][3] = {{1, 2, 3}, {4, 9, 5}, {6, 7, 8}};
    ExternalBorder border;
    const int cw = img.w / 3;
    const int ch = img.h / 3;
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            ExternalImage cell =
                crop_image(img, col * cw, row * ch, cw, ch);
            ShapeFrame f;
            if (!make_frame(cell, 8, 8, palette, pm, f)) {
                continue;
            }
            border.frames[kMap[row][col]] = std::move(f);
        }
    }
    if (!border.frames.empty()) {
        set_border(borders, names, name, std::move(border));
    }
}

void add_border_frame(ExternalAssets& ext, const uint8_t palette[768],
                      PaletteMap& pm, const std::string& rel,
                      const std::string& name, int frame,
                      std::vector<ExternalBorder>& borders,
                      std::vector<std::string>& names) {
    ExternalImage img;
    std::string err;
    if (!ext.load_image(rel, img, err)) {
        std::fprintf(stderr, "external: %s\n", err.c_str());
        return;
    }
    ShapeFrame f;
    if (!make_frame(img, 8, 8, palette, pm, f)) {
        return;
    }
    const std::string key = lower(name);
    for (ExternalBorder& b : borders) {
        if (b.name == key) {
            b.frames[frame] = std::move(f);
            return;
        }
    }
    ExternalBorder border;
    border.name = key;
    border.frames[frame] = std::move(f);
    borders.push_back(std::move(border));
    names.push_back(key);
}

}  // namespace

bool ExternalVisuals::build(const ExternalAssets& ext, ShapeBank& shapes,
                            const TileDefs& tiles, const ProfileDef& profile,
                            const uint8_t palette[768], int board_w,
                            int board_h) {
    clear();
    if (!ext.mounted()) {
        return false;
    }
    ExternalAssets& mutable_ext = const_cast<ExternalAssets&>(ext);
    PaletteMap pm;
    pm.pal = palette;

    // Auto-discovered kind sheets and single shape overrides.
    for (const std::string& rel : ext.files("sprites/")) {
        const std::string e = ext_of(rel);
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      e) == ExternalAssets::image_exts().end()) {
            continue;
        }
        const std::string stem = stem_of(rel);
        long a = 0, b = 0;
        if (stem.rfind("kind", 0) == 0) {
            const std::string num = stem.substr(4);
            const size_t us = num.find('_');
            if (parse_int(us == std::string::npos ? num : num.substr(0, us), a)) {
                add_kind_sheet(mutable_ext, shapes, profile, palette, pm,
                               static_cast<int>(a), rel, shapes_);
            }
        } else if (stem.rfind("shape", 0) == 0) {
            const size_t first = stem.find('_', 5);
            const size_t second =
                first == std::string::npos ? std::string::npos
                                           : stem.find('_', first + 1);
            if (first != std::string::npos && second != std::string::npos &&
                parse_int(stem.substr(first + 1, second - first - 1), a) &&
                parse_int(stem.substr(second + 1), b)) {
                add_shape(mutable_ext, shapes, palette, pm, static_cast<int>(a),
                          static_cast<int>(b), rel, shapes_);
            }
        }
    }

    // Explicit [sprites] overrides win over auto-discovery.
    for (const auto& kv : ext.manifest().section("sprites")) {
        const std::string& key = kv.first;
        const std::string& rel = kv.second;
        long a = 0, b = 0;
        if (key.rfind("shape", 0) == 0) {
            const size_t first = key.find('_', 5);
            const size_t second =
                first == std::string::npos ? std::string::npos
                                           : key.find('_', first + 1);
            if (first != std::string::npos && second != std::string::npos &&
                parse_int(key.substr(first + 1, second - first - 1), a) &&
                parse_int(key.substr(second + 1), b)) {
                add_shape(mutable_ext, shapes, palette, pm, static_cast<int>(a),
                          static_cast<int>(b), rel, shapes_);
            }
        } else {
            std::string num = key;
            if (num.rfind("kind", 0) == 0) {
                num = num.substr(4);
            }
            const size_t us = num.find('_');
            if (parse_int(us == std::string::npos ? num : num.substr(0, us), a)) {
                add_kind_sheet(mutable_ext, shapes, profile, palette, pm,
                               static_cast<int>(a), rel, shapes_);
            }
        }
    }

    // Tiles: `tiles/<id>.<ext>` or `[tiles] <id> = file`.
    for (const std::string& rel : ext.files("tiles/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        long id = 0;
        if (parse_int(stem_of(rel), id)) {
            add_tile(mutable_ext, shapes, tiles, palette, pm,
                     static_cast<int>(id), rel, shapes_);
        }
    }
    for (const auto& kv : ext.manifest().section("tiles")) {
        long id = 0;
        if (parse_int(kv.first, id)) {
            add_tile(mutable_ext, shapes, tiles, palette, pm,
                     static_cast<int>(id), kv.second, shapes_);
        }
    }

    // Level backgrounds: `boards/<name>.<ext>` or `[boards] <name> = file`.
    for (const std::string& rel : ext.files("boards/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        std::vector<uint8_t> px;
        if (add_picture(mutable_ext, palette, pm, rel, board_w, board_h, px)) {
            boards_[stem_of(rel)] = std::move(px);
        }
    }
    for (const auto& kv : ext.manifest().section("boards")) {
        std::vector<uint8_t> px;
        if (add_picture(mutable_ext, palette, pm, kv.second, board_w, board_h,
                        px)) {
            boards_[lower(kv.first)] = std::move(px);
        }
    }

    // Extra menu themes: every image under `themes/`, sorted by name.
    for (const std::string& rel : ext.files("themes/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        std::vector<uint8_t> px;
        if (add_picture(mutable_ext, palette, pm, rel, 320, 200, px)) {
            themes_.push_back(std::move(px));
            theme_names_.push_back(stem_of(rel));
        }
    }
    // Fonts: `fonts/main.<ext>` replaces the whole 6x? text glyph bank (shape
    // entry 2), split horizontally like a kind sheet.  `fonts/<name>.<ext>`
    // adds a selectable 6 px set, `fonts/<name>_8.<ext>` its 8 px counterpart.
    // `fonts/glyph_<code>` replaces one character globally.
    auto font_set = [&](const std::string& name) -> ExternalFont& {
        const std::string key = lower(name);
        for (ExternalFont& f : fonts_) {
            if (f.name == key) {
                return f;
            }
        }
        fonts_.push_back(ExternalFont{key, {}});
        if (key != "main" && key != "font") {
            font_names_.push_back(key);
        }
        return fonts_.back();
    };
    auto add_font_auto = [&](const std::string& rel) {
        const std::string stem = stem_of(rel);
        long code = 0;
        if (stem.rfind("glyph_", 0) == 0 &&
            parse_int(stem.substr(6), code)) {
            add_font_glyph(mutable_ext, shapes, palette, pm,
                           static_cast<int>(code), rel, shapes_);
            return;
        }
        int entry = 2;
        std::string name = stem;
        if (name.size() > 2 && name.compare(name.size() - 2, 2, "_8") == 0) {
            entry = 1;
            name.resize(name.size() - 2);
        }
        if (name == "main" || name == "font") {
            add_font_sheet(mutable_ext, shapes, palette, pm, entry, rel, shapes_);
            return;
        }
        if (name.empty()) {
            return;
        }
        add_font_sheet(mutable_ext, shapes, palette, pm, entry, rel,
                       font_set(name).frames);
    };
    for (const std::string& rel : ext.files("fonts/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        add_font_auto(rel);
    }
    for (const auto& kv : ext.manifest().section("fonts")) {
        long code = 0;
        if (parse_int(kv.first, code)) {
            add_font_glyph(mutable_ext, shapes, palette, pm,
                           static_cast<int>(code), kv.second, shapes_);
            continue;
        }
        int entry = 2;
        std::string name = lower(kv.first);
        if (name.size() > 2 && name.compare(name.size() - 2, 2, "_8") == 0) {
            entry = 1;
            name.resize(name.size() - 2);
        }
        if (name == "main" || name == "font") {
            add_font_sheet(mutable_ext, shapes, palette, pm, entry, kv.second,
                           shapes_);
        } else if (!name.empty()) {
            add_font_sheet(mutable_ext, shapes, palette, pm, entry, kv.second,
                           font_set(name).frames);
        }
    }
    std::sort(font_names_.begin(), font_names_.end());
    font_names_.erase(std::unique(font_names_.begin(), font_names_.end()),
                      font_names_.end());

    // HUD panels: `hud/classic.<ext>` (64x84) and `hud/modern.<ext>`
    // (320x12) are the defaults.  `hud/classic_<name>` and
    // `hud/modern_<name>` add more selectable panels; an unprefixed image is
    // classified by its source aspect.  Labels/icons/pips are still drawn on
    // top.
    auto set_hud_pic = [&](const std::string& rel, const std::string& raw_name,
                           bool forced_classic, bool forced_style) {
        const std::string name = lower(raw_name);
        bool classic = forced_classic;
        if (!forced_style) {
            ExternalImage probe;
            std::string probe_err;
            if (!mutable_ext.load_image(rel, probe, probe_err)) {
                std::fprintf(stderr, "external: %s\n", probe_err.c_str());
                return;
            }
            classic = !(probe.w > 96 && probe.h > 0 && probe.h <= 20);
        }
        ExternalPicture pic;
        if (!add_overlay(mutable_ext, palette, pm, rel, classic ? 64 : 320,
                         classic ? 84 : 12, pic)) {
            return;
        }
        if (classic) {
            if (name == "classic") {
                hud_classic_ = pic;
                has_hud_classic_ = true;
            }
        } else if (name == "modern") {
            hud_modern_ = pic;
            has_hud_modern_ = true;
        }
        set_hud(huds_, name, classic, std::move(pic));
    };
    for (const std::string& rel : ext.files("hud/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        const std::string stem = stem_of(rel);
        if (stem == "classic" || stem == "modern") {
            set_hud_pic(rel, stem, stem == "classic", true);
        } else if (stem.rfind("classic_", 0) == 0) {
            set_hud_pic(rel, stem.substr(8), true, true);
        } else if (stem.rfind("modern_", 0) == 0) {
            set_hud_pic(rel, stem.substr(7), false, true);
        } else {
            set_hud_pic(rel, stem, false, false);
        }
    }
    for (const auto& kv : ext.manifest().section("hud")) {
        const std::string key = lower(kv.first);
        if (key == "classic" || key == "modern") {
            set_hud_pic(kv.second, key, key == "classic", true);
        } else if (key.rfind("classic_", 0) == 0) {
            set_hud_pic(kv.second, key.substr(8), true, true);
        } else if (key.rfind("modern_", 0) == 0) {
            set_hud_pic(kv.second, key.substr(7), false, true);
        } else {
            set_hud_pic(kv.second, key, false, false);
        }
    }

    // Menu borders: a `borders/<name>.<ext>` 3x3 sheet, or individual
    // `borders/<name>/<1..9>.<ext>` frames.  They replace shape entry 7's
    // nine-slice, so they work with the classic and modern menu styles.
    for (const std::string& rel : ext.files("borders/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        std::string sub = rel;
        const std::string prefix = "borders/";
        if (sub.compare(0, prefix.size(), prefix) == 0) {
            sub = sub.substr(prefix.size());
        }
        const size_t slash = sub.find('/');
        if (slash == std::string::npos) {
            add_border_sheet(mutable_ext, palette, pm, rel, stem_of(rel),
                             borders_, border_names_);
        } else {
            long frame = 0;
            if (parse_int(stem_of(sub), frame) && frame >= 1 && frame <= 9) {
                add_border_frame(mutable_ext, palette, pm, rel,
                                 sub.substr(0, slash), static_cast<int>(frame),
                                 borders_, border_names_);
            }
        }
    }
    for (const auto& kv : ext.manifest().section("borders")) {
        add_border_sheet(mutable_ext, palette, pm, kv.second, kv.first,
                         borders_, border_names_);
    }

    // Text-page art: `pages/page_<n>.<ext>` or `pages/<n>.<ext>` (320x200).
    for (const std::string& rel : ext.files("pages/")) {
        if (std::find(ExternalAssets::image_exts().begin(),
                      ExternalAssets::image_exts().end(),
                      ext_of(rel)) == ExternalAssets::image_exts().end()) {
            continue;
        }
        const std::string stem = stem_of(rel);
        long n = 0;
        const std::string num =
            stem.rfind("page_", 0) == 0 ? stem.substr(5) : stem;
        if (parse_int(num, n) && n >= 0 && n < 40) {
            ExternalPicture pic;
            if (add_overlay(mutable_ext, palette, pm, rel, 320, 200, pic)) {
                pages_[static_cast<int>(n)] = std::move(pic);
            }
        }
    }
    for (const auto& kv : ext.manifest().section("pages")) {
        long n = 0;
        if (parse_int(kv.first, n) && n >= 0 && n < 40) {
            ExternalPicture pic;
            if (add_overlay(mutable_ext, palette, pm, kv.second, 320, 200,
                            pic)) {
                pages_[static_cast<int>(n)] = std::move(pic);
            }
        }
    }

    // Dialog portrait: `portrait.<ext>` (or `portraits/portrait.<ext>`),
    // 64x84, shown beside level-title/dialog boxes.  A manifest `[portrait]`
    // entry wins.
    {
        std::vector<std::string> candidates;
        for (const std::string& e : ExternalAssets::image_exts()) {
            candidates.push_back("portrait." + e);
            candidates.push_back("portraits/portrait." + e);
        }
        std::string rel;
        if (ext.first_existing(candidates, rel)) {
            ExternalPicture pic;
            if (add_overlay(mutable_ext, palette, pm, rel, 64, 84, pic)) {
                portrait_ = std::move(pic);
                has_portrait_ = true;
            }
        }
    }
    for (const auto& kv : ext.manifest().section("portrait")) {
        if (kv.first == "file" || kv.first == "image" ||
            kv.first == "portrait") {
            ExternalPicture pic;
            if (add_overlay(mutable_ext, palette, pm, kv.second, 64, 84,
                            pic)) {
                portrait_ = std::move(pic);
                has_portrait_ = true;
            }
        }
    }

    return !empty() || !themes_.empty();
}
