#pragma once
#include <cstdint>
#include <string>

// Per-kind data lifted from the engine's kind-table initialiser by
// re/gen_port_data.py.  Never hardcode these in gameplay code.
struct KindInfo {
    uint8_t kind;
    const char* name;
    uint16_t width;
    uint16_t height;
    uint16_t flags;
    uint8_t tileset;
    uint16_t count;
};

// Animation tables copied out of DGROUP by the original handlers, one set per
// episode (re/gen_port_data.py emits them from docs/profiles.json).
struct HandlerTables {
    int16_t fly_frame[4];
    int16_t fly_y[4];
    int16_t phoenix_frame[8];
    int16_t phoenix_mirror[8];
    int16_t demon_frame[6];
    int16_t fatso_frame[4];
    int16_t expl2_frame[8];
    int16_t bat_frame[4];
    int16_t bat_y[4];
    int16_t snake_head[4];
    int16_t snake_mid[4];
    int16_t snake_tail[4];
    int16_t boll[6];
    int16_t knight_xoff[11];
    int16_t knight_frame[11];
    int16_t beeswarm[32];
    int16_t croc_head[4];
    int16_t croc_tail[4];
    int16_t skull_b[8];
    int16_t skull_a[4];
    int16_t jelly[10];
    int16_t badfish[4];
};

// DGROUP byte offsets of the live engine state.  The port keeps these so the
// documented addresses stay traceable; the actual data lives in Engine.
struct Anchors {    uint16_t tile_map;
    uint16_t board_header;
    uint16_t tile_table;
    uint16_t object_array;
    uint16_t object_count;
    uint16_t visible_count;
    uint16_t active_list;    // 0 = not resolved yet in that episode
    uint16_t camera_ptr;
    uint16_t object_max;
    uint16_t object_stride;
};

struct ProfileDef {
    int episode;
    const char* board_ext;
    const char* shape_file;
    const char* tile_defs;
    const char* first_board;
    const char* intro_board;
    int kind_count;
    const KindInfo* kinds;
    uint16_t epic_score;          // _addscore value for kind 49 (2 / 2 / 25)
    const uint16_t* item_icons;   // item id -> HUD icon frame (DGROUP:0xED4)
    const char* const* boards;    // shipped board names (no extension)
    int board_count;
    const HandlerTables* tables;  // per-episode animation tables
    const char* const* level_titles;  // 36-entry title table (index = level+4)
    int title_count;
    const char* sound_bank;       // .vcl: sampled sounds + text page table
    Anchors anchors;
    const char* id;               // stable name a game manifest can refer to
    const char* save_prefix;      // save file stem, e.g. "jn1save"
    int save_slots;               // number of save slots (6 in every episode)

    const KindInfo& kind(int k) const {
        static const KindInfo dummy{0, "?", 0, 0, 0, 0, 0};
        return (k >= 0 && k < kind_count) ? kinds[k] : dummy;
    }
};

const ProfileDef* profile_for(int episode);
// Look up a compiled engine profile by its stable id ("jill1", "jill2",
// "jill3").  Game manifests name a profile this way so a mod can reuse an
// existing engine dataset while shipping its own art, boards and palette.
const ProfileDef* profile_by_id(const char* id);

// Enumerate the compiled profiles (used to build the built-in game list).
int profile_count();
const ProfileDef& profile_at(int index);

// --- runtime profiles (profiles/<id>.profile) -----------------------------
// Load every *.profile in `dir`.  Files may inherit another profile with
// `base = <id>`, so a total conversion usually needs only a handful of lines.
// Call this before resolving game manifests.  Returns the number registered.
int load_profile_dir(const std::string& dir, std::string& err);
int runtime_profile_count();
const ProfileDef& runtime_profile_at(int index);
