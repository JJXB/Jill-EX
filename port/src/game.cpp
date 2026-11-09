#include "game.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>

namespace {

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

int parse_int(const std::string& s, int fallback) {
    if (s.empty()) {
        return fallback;
    }
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 0);
    return end == s.c_str() ? fallback : static_cast<int>(v);
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

// Manifest keys are case-insensitive.  Anything unrecognised is collected in
// `warnings` so a typo is visible without making the file unusable.
bool apply_key(GameDef& g, const std::string& raw_key, const std::string& value,
               std::string& warning) {
    const std::string key = lower(raw_key);
    if (key == "id") {
        g.id = value;
    } else if (key == "title") {
        g.title = value;
    } else if (key == "data" || key == "data_dir") {
        g.data_dir = value;
    } else if (key == "shapes" || key == "shape_file") {
        g.shape_file = value;
    } else if (key == "tiles" || key == "tile_defs") {
        g.tile_defs = value;
    } else if (key == "board_ext" || key == "extension") {
        g.board_ext = value;
    } else if (key == "first_board" || key == "start_board") {
        g.first_board = value;
    } else if (key == "map_board" || key == "hub_board") {
        g.map_board = value;
    } else if (key == "intro_board") {
        g.intro_board = value;
    } else if (key == "menu_block" || key == "menu_frame") {
        g.menu_block = value;
    } else if (key == "palette") {
        g.palette = value;
    } else if (key == "pages" || key == "text_pages" || key == "vcl") {
        g.pages_file = value;
    } else if (key == "title_music" || key == "music") {
        g.title_music = value;
    } else if (key == "save_prefix") {
        g.save_prefix = value;
    } else if (key == "save_slots") {
        g.save_slots = parse_int(value, 0);
    } else if (key == "suit" || key == "suit_index") {
        g.suit_index = parse_int(value, -1);
    } else if (key == "hud_icon_bank") {
        g.hud_icon_bank = parse_int(value, 0x0E);
    } else if (key == "boards" || key == "board_list") {
        g.boards = split_list(value);
    } else if (key == "profile" || key == "engine") {
        g.profile_id = value;
    } else {
        warning = raw_key;
        return false;
    }
    return true;
}

// Fill anything the manifest left out from its engine profile, so a minimal
// manifest ("profile = jill1, data = mymod") is still playable.
void apply_profile_defaults(GameDef& g) {
    const ProfileDef& p = *g.profile;
    if (g.title.empty()) {
        g.title = p.id;
    }
    if (g.data_dir.empty()) {
        g.data_dir = "jillt";
    }
    if (g.shape_file.empty()) {
        g.shape_file = p.shape_file;
    }
    if (g.tile_defs.empty()) {
        g.tile_defs = p.tile_defs;
    }
    if (g.board_ext.empty()) {
        g.board_ext = p.board_ext;
    }
    if (g.first_board.empty()) {
        g.first_board = p.first_board;
    }
    if (g.map_board.empty()) {
        // Most episodes start on their hub.  Onesimus is the exception: it
        // starts on 1.ONE and reaches MAP.ONE later, so its manifest sets
        // map_board explicitly.
        g.map_board = g.first_board;
    }
    if (g.intro_board.empty()) {
        g.intro_board = p.intro_board;
    }
    if (g.pages_file.empty() && p.sound_bank) {
        g.pages_file = p.sound_bank;
    }
    if (g.title_music.empty() && p.title_music) {
        g.title_music = p.title_music;
    }
    if (g.save_prefix.empty() && p.save_prefix) {
        g.save_prefix = p.save_prefix;
    }
    if (g.save_slots <= 0) {
        g.save_slots = p.save_slots;
    }
    if (g.boards.empty() && p.boards) {
        for (int i = 0; i < p.board_count; ++i) {
            g.boards.push_back(p.boards[i]);
        }
    }
}

}  // namespace

bool load_game_manifest(const std::string& path, GameDef& out, std::string& err) {
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }
    GameDef g;
    std::string line;
    std::vector<std::string> unknown;
    int lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';' || t[0] == '[') {
            continue;
        }
        const size_t eq = t.find('=');
        if (eq == std::string::npos) {
            std::fprintf(stderr, "%s:%d: expected key = value\n", path.c_str(),
                         lineno);
            continue;
        }
        std::string warning;
        if (!apply_key(g, trim(t.substr(0, eq)), trim(t.substr(eq + 1)),
                       warning)) {
            unknown.push_back(warning);
        }
    }
    if (g.profile_id.empty()) {
        err = path + ": missing 'profile'";
        return false;
    }
    g.profile = profile_by_id(g.profile_id.c_str());
    if (!g.profile) {
        err = path + ": unknown engine profile '" + g.profile_id + "'";
        return false;
    }
    apply_profile_defaults(g);
    if (g.id.empty()) {
        size_t slash = path.find_last_of("/\\");
        std::string base = (slash == std::string::npos) ? path : path.substr(slash + 1);
        size_t dot = base.find_last_of('.');
        g.id = (dot == std::string::npos) ? base : base.substr(0, dot);
    }
    if (g.boards.empty()) {
        err = path + ": no boards listed and the profile has none";
        return false;
    }
    if (!unknown.empty()) {
        std::fprintf(stderr, "%s: ignoring unknown key '%s'\n", path.c_str(),
                     unknown.front().c_str());
    }
    out = std::move(g);
    return true;
}

int discover_games(const std::string& dir, std::vector<GameDef>& out,
                   std::string& err) {
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
        if (lower(entry.path().extension().string()) == ".game") {
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
        GameDef g;
        std::string why;
        if (!load_game_manifest(path.string(), g, why)) {
            std::fprintf(stderr, "%s\n", why.c_str());
            continue;
        }
        out.push_back(std::move(g));
    }
    return static_cast<int>(out.size());
}

std::vector<GameDef> builtin_games() {
    std::vector<GameDef> out;
    for (int i = 0; i < profile_count(); ++i) {
        const ProfileDef& p = profile_at(i);
        GameDef g;
        g.id = p.id;
        g.title = p.id;
        g.profile_id = p.id;
        g.profile = &p;
        apply_profile_defaults(g);
        out.push_back(std::move(g));
    }
    return out;
}

std::vector<GameDef> load_game_list(const std::string& dir, std::string& err) {
    std::vector<GameDef> games;
    const std::string root = dir.empty() ? std::string("games") : dir;
    if (discover_games(root, games, err) == 0) {
        // No manifests installed: the compiled episodes remain playable.
        games = builtin_games();
    }
    return games;
}

const GameDef* game_by_id(const std::vector<GameDef>& games,
                          const std::string& id) {
    for (const GameDef& g : games) {
        if (g.id == id) {
            return &g;
        }
    }
    return nullptr;
}
