#include "profile.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "gamedata.inc"

namespace {

// A profile loaded from profiles/<id>.profile.  Owns every byte its
// ProfileDef points at, so an unloaded/reloaded profile cannot dangle.
struct OwnedProfile {
    ProfileDef def{};
    std::deque<std::string> strings;   // stable addresses for kind/board names
    std::vector<KindInfo> kinds;
    HandlerTables tables{};
    std::vector<uint16_t> icons;
    std::deque<std::string> titles;        // level-title text pool
    std::map<int, const char*> title_at;   // level-title index -> text
    std::vector<const char*> title_ptrs;
    std::string sound_bank;                // runtime .vcl name
    std::string save_prefix;               // save file stem
    std::string id;
};

std::vector<std::unique_ptr<OwnedProfile>> g_loaded;
std::map<std::string, std::string> g_files;      // id -> path
std::map<std::string, OwnedProfile*> g_by_id;
std::map<std::string, bool> g_loading;

std::string trim(const std::string& s) {
    size_t a = 0;
    size_t b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) {
        ++a;
    }
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) {
        --b;
    }
    return s.substr(a, b - a);
}

std::string lower(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

std::vector<std::string> split_list(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',' || std::isspace(static_cast<unsigned char>(c))) {
            if (!cur.empty()) {
                out.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        out.push_back(cur);
    }
    return out;
}

int parse_int(const std::string& s, int fallback) {
    if (s.empty()) {
        return fallback;
    }
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 0);
    return end == s.c_str() ? fallback : static_cast<int>(v);
}

// Copy `src`'s owned data into `dst`, leaving non-owned pointers borrowed from
// the compiled table (they live for the whole process).
void clone_profile(const ProfileDef& src, OwnedProfile& dst) {
    dst.def = src;
    dst.id = src.id;
    dst.kinds.clear();
    dst.kinds.reserve(static_cast<size_t>(src.kind_count));
    for (int k = 0; k < src.kind_count; ++k) {
        dst.strings.push_back(src.kinds[k].name);
        KindInfo info = src.kinds[k];
        info.name = dst.strings.back().c_str();
        dst.kinds.push_back(info);
    }
    dst.tables = src.tables ? *src.tables : HandlerTables{};
    if (src.item_icons) {
        dst.icons.assign(src.item_icons, src.item_icons + 16);
    } else {
        dst.icons.assign(16, 0);
    }
    dst.titles.clear();
    dst.title_at.clear();
    for (int i = 0; i < src.title_count; ++i) {
        if (!src.level_titles || !src.level_titles[i]) {
            continue;
        }
        dst.titles.push_back(src.level_titles[i]);
        dst.title_at[i] = dst.titles.back().c_str();
    }
    dst.def.kinds = nullptr;  // set by finalise()
}

void finalise(OwnedProfile& p) {
    p.def.id = p.id.c_str();
    p.def.sound_bank = p.sound_bank.empty()
                           ? (p.def.sound_bank ? p.def.sound_bank : "")
                           : p.sound_bank.c_str();
    p.def.save_prefix = p.save_prefix.empty()
                            ? (p.def.save_prefix ? p.def.save_prefix : "")
                            : p.save_prefix.c_str();
    p.def.kind_count = static_cast<int>(p.kinds.size());
    p.def.kinds = p.kinds.data();
    p.def.tables = &p.tables;
    p.def.item_icons = p.icons.data();
    int max_index = -1;
    for (const auto& kv : p.title_at) {
        max_index = std::max(max_index, kv.first);
    }
    p.title_ptrs.assign(static_cast<size_t>(max_index + 1), "");
    for (const auto& kv : p.title_at) {
        p.title_ptrs[static_cast<size_t>(kv.first)] = kv.second;
    }
    p.def.level_titles = p.title_ptrs.empty() ? nullptr : p.title_ptrs.data();
    p.def.title_count = static_cast<int>(p.title_ptrs.size());
}

void grow_kinds(OwnedProfile& p, int n) {
    while (static_cast<int>(p.kinds.size()) < n) {
        const int k = static_cast<int>(p.kinds.size());
        p.strings.push_back("KIND" + std::to_string(k));
        KindInfo info{};
        info.kind = static_cast<uint8_t>(k);
        info.name = p.strings.back().c_str();
        p.kinds.push_back(info);
    }
}

// Apply one `key = value` to a profile that already contains its base data.
void apply_key(OwnedProfile& p, const std::string& key,
               const std::string& value) {
    if (key == "id") {
        p.id = value;
    } else if (key == "episode") {
        p.def.episode = parse_int(value, p.def.episode);
    } else if (key == "board_ext") {
        p.strings.push_back(value);
        p.def.board_ext = p.strings.back().c_str();
    } else if (key == "pages" || key == "text_pages" || key == "sound_bank") {
        p.sound_bank = value;
        p.def.sound_bank = p.sound_bank.c_str();
    } else if (key == "save_prefix") {
        p.save_prefix = value;
        p.def.save_prefix = p.save_prefix.c_str();
    } else if (key == "save_slots") {
        p.def.save_slots = parse_int(value, p.def.save_slots);
    } else if (key == "kind_count") {
        grow_kinds(p, parse_int(value, static_cast<int>(p.kinds.size())));
    } else if (key == "epic_score") {
        p.def.epic_score = static_cast<uint16_t>(parse_int(value, p.def.epic_score));
    } else if (key == "object_max") {
        p.def.anchors.object_max = static_cast<uint16_t>(parse_int(value, 192));
    } else if (key == "object_stride") {
        p.def.anchors.object_stride = static_cast<uint16_t>(parse_int(value, 31));
    } else if (key == "item_icons") {
        std::vector<std::string> v = split_list(value);
        p.icons.resize(16, 0);
        for (size_t i = 0; i < v.size() && i < 16; ++i) {
            p.icons[i] = static_cast<uint16_t>(parse_int(v[i], 0));
        }
    } else if (key == "handler_offsets" || key == "dgroup_base" ||
               key == "title") {
        // Provenance metadata written by the generator; not used at run time.
    } else if (key == "level_title") {
        const size_t comma = value.find(',');
        if (comma == std::string::npos) {
            std::fprintf(stderr, "profile %s: 'level_title' needs N, text\n",
                         p.id.c_str());
            return;
        }
        const int index = parse_int(trim(value.substr(0, comma)), -1);
        const std::string raw = trim(value.substr(comma + 1));
        std::string text;
        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 1 < raw.size()) {
                if (raw[i + 1] == 'n') {
                    text.push_back('\n');
                    ++i;
                    continue;
                }
                if (raw[i + 1] == '\\') {
                    text.push_back('\\');
                    ++i;
                    continue;
                }
            }
            text.push_back(raw[i]);
        }
        if (index >= 0) {
            p.titles.push_back(text);
            p.title_at[index] = p.titles.back().c_str();
        }
    } else if (key == "kind") {
        std::vector<std::string> v = split_list(value);
        if (v.size() < 7) {
            std::fprintf(stderr, "profile %s: 'kind' needs 7 fields\n",
                         p.id.c_str());
            return;
        }
        const int k = parse_int(v[0], -1);
        if (k < 0) {
            return;
        }
        grow_kinds(p, k + 1);
        p.strings.push_back(v[1]);
        KindInfo& info = p.kinds[static_cast<size_t>(k)];
        info.kind = static_cast<uint8_t>(k);
        info.name = p.strings.back().c_str();
        info.width = static_cast<uint16_t>(parse_int(v[2], 0));
        info.height = static_cast<uint16_t>(parse_int(v[3], 0));
        info.flags = static_cast<uint16_t>(parse_int(v[4], 0));
        info.tileset = static_cast<uint8_t>(parse_int(v[5], 0));
        info.count = static_cast<uint16_t>(parse_int(v[6], 0));
    } else {
        int16_t* dst = nullptr;
        int cap = 0;
        if (key == "fly_frame") {
            dst = p.tables.fly_frame;
            cap = 4;
        } else if (key == "fly_y") {
            dst = p.tables.fly_y;
            cap = 4;
        } else if (key == "phoenix_frame") {
            dst = p.tables.phoenix_frame;
            cap = 8;
        } else if (key == "phoenix_mirror") {
            dst = p.tables.phoenix_mirror;
            cap = 8;
        } else if (key == "demon_frame") {
            dst = p.tables.demon_frame;
            cap = 6;
        } else if (key == "fatso_frame") {
            dst = p.tables.fatso_frame;
            cap = 4;
        } else if (key == "expl2_frame") {
            dst = p.tables.expl2_frame;
            cap = 8;
        } else if (key == "bat_frame") {
            dst = p.tables.bat_frame;
            cap = 4;
        } else if (key == "bat_y") {
            dst = p.tables.bat_y;
            cap = 4;
        }
        if (!dst) {
            std::fprintf(stderr, "profile %s: ignoring unknown key '%s'\n",
                         p.id.c_str(), key.c_str());
            return;
        }
        std::vector<std::string> v = split_list(value);
        for (size_t i = 0; i < v.size() && i < static_cast<size_t>(cap); ++i) {
            dst[i] = static_cast<int16_t>(parse_int(v[i], 0));
        }
    }
}

}  // namespace

const ProfileDef* profile_for(int episode) {
    for (const ProfileDef& p : kProfiles) {
        if (p.episode == episode) {
            return &p;
        }
    }
    return nullptr;
}

const ProfileDef* profile_by_id(const char* id) {
    if (!id) {
        return nullptr;
    }
    auto it = g_by_id.find(id);
    if (it != g_by_id.end()) {
        return &it->second->def;
    }
    for (const ProfileDef& p : kProfiles) {
        if (std::strcmp(p.id, id) == 0) {
            return &p;
        }
    }
    return nullptr;
}

int profile_count() {
    return static_cast<int>(sizeof(kProfiles) / sizeof(kProfiles[0]));
}

const ProfileDef& profile_at(int index) {
    return kProfiles[index];
}

namespace {

const ProfileDef* load_one(const std::string& id, int depth);

// Two-pass: read the file once for `base`, then apply every key on top.
const ProfileDef* load_one(const std::string& id, int depth) {
    if (const ProfileDef* found = profile_by_id(id.c_str())) {
        return found;
    }
    auto file = g_files.find(id);
    if (file == g_files.end()) {
        return nullptr;
    }
    if (depth > 8) {
        std::fprintf(stderr, "profile %s: base chain too deep\n", id.c_str());
        return nullptr;
    }
    if (g_loading[id]) {
        std::fprintf(stderr, "profile %s: cyclic base chain\n", id.c_str());
        return nullptr;
    }
    g_loading[id] = true;

    std::ifstream in(file->second);
    if (!in) {
        std::fprintf(stderr, "cannot open %s\n", file->second.c_str());
        g_loading[id] = false;
        return nullptr;
    }
    std::vector<std::pair<std::string, std::string>> keys;
    std::string base_id;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';' || t[0] == '[') {
            continue;
        }
        const size_t eq = t.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = lower(trim(t.substr(0, eq)));
        const std::string value = trim(t.substr(eq + 1));
        if (key == "base") {
            base_id = value;
        } else {
            keys.emplace_back(key, value);
        }
    }

    std::unique_ptr<OwnedProfile> owned(new OwnedProfile());
    const ProfileDef* base = nullptr;
    if (!base_id.empty()) {
        base = load_one(base_id, depth + 1);
        if (!base) {
            std::fprintf(stderr, "%s: unknown base profile '%s'\n",
                         file->second.c_str(), base_id.c_str());
            g_loading[id] = false;
            return nullptr;
        }
        clone_profile(*base, *owned);
    } else {
        owned->id = id;
        owned->def.episode = 0;
        owned->def.board_ext = ".bin";
        owned->def.shape_file = "";
        owned->def.tile_defs = "";
        owned->def.first_board = "";
        owned->def.intro_board = "";
        owned->def.boards = nullptr;
        owned->def.board_count = 0;
        owned->def.epic_score = 0;
        owned->def.save_prefix = "";
        owned->def.save_slots = 6;
        owned->icons.assign(16, 0);
        grow_kinds(*owned, 64);
    }
    owned->id = id;
    for (const auto& kv : keys) {
        apply_key(*owned, kv.first, kv.second);
    }
    finalise(*owned);

    ProfileDef& def = owned->def;
    def.shape_file = def.shape_file ? def.shape_file : "";
    def.tile_defs = def.tile_defs ? def.tile_defs : "";
    def.board_ext = def.board_ext ? def.board_ext : "";
    def.first_board = def.first_board ? def.first_board : "";
    def.intro_board = def.intro_board ? def.intro_board : "";

    OwnedProfile* raw = owned.get();
    g_loaded.push_back(std::move(owned));
    g_by_id[id] = raw;
    g_loading[id] = false;
    return &raw->def;
}

}  // namespace

int load_profile_dir(const std::string& dir, std::string& err) {
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        return 0;
    }
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        std::error_code file_ec;
        if (!entry.is_regular_file(file_ec)) {
            continue;
        }
        if (lower(entry.path().extension().string()) == ".profile") {
            files.push_back(entry.path());
        }
    }
    if (ec) {
        err = "cannot read " + dir + ": " + ec.message();
        return 0;
    }
    std::sort(files.begin(), files.end(),
              [](const fs::path& a, const fs::path& b) {
                  return a.filename().string() < b.filename().string();
              });
    for (const fs::path& path : files) {
        std::string id = path.stem().string();
        g_files[id] = path.string();
    }
    // Resolve every file so a profile based on a later file also works.
    std::vector<std::string> ids;
    for (const auto& kv : g_files) {
        ids.push_back(kv.first);
    }
    int loaded = 0;
    for (const std::string& id : ids) {
        const ProfileDef* p = load_one(id, 0);
        if (p) {
            ++loaded;
        } else {
            std::fprintf(stderr, "%s: profile '%s' not usable\n",
                         g_files[id].c_str(), id.c_str());
        }
    }
    return loaded;
}

int runtime_profile_count() {
    return static_cast<int>(g_loaded.size());
}

const ProfileDef& runtime_profile_at(int index) {
    return g_loaded[static_cast<size_t>(index)]->def;
}
