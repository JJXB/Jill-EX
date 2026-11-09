#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "profile.h"

// A "game" is one playable asset set: which files to load, which boards it
// ships, and which compiled engine profile (kind tables, handler tables,
// DGROUP anchors, item icons) drives it.
//
// Splitting this from ProfileDef is what makes mods cheap: a mod normally
// keeps the original engine dataset and replaces art, boards, palette and
// starting board.  That is one text manifest, no rebuild.
struct GameDef {
    std::string id;            // manifest file name without .game
    std::string title;         // shown in the episode list and window title
    std::string data_dir;      // asset directory (relative to the working dir)
    std::string shape_file;    // .sha shape bank
    std::string tile_defs;     // .dma tile table
    std::string board_ext;     // ".jn1" / ".ONE"
    std::string first_board;   // board started by "START GAME"
    std::string map_board;     // hub/world-map board used by '!' and map mode
    std::string intro_board;   // board used as the menu backdrop
    std::string menu_block;    // optional .dma tile name for the menu frame ring
    std::string palette;       // optional 768-byte raw palette file
    std::string pages_file;    // .vcl sampled-sound bank with the text pages
    int suit_index = -1;       // palette index, -1 = derive from the player art
    int hud_icon_bank = 0x0E;  // shape bank holding item icons and health pips
    std::vector<std::string> boards;  // level-select names, no extension
    std::string profile_id;           // compiled engine profile to reuse
    const ProfileDef* profile = nullptr;

    int board_count() const { return static_cast<int>(boards.size()); }
    const std::string& board(int i) const { return boards[static_cast<size_t>(i)]; }
    // True once profile_id was matched against a compiled profile.
    bool resolved() const { return profile != nullptr; }
};

// Parse one `key = value` manifest.  Unknown keys are reported but do not
// fail the load, so a manifest can carry editor-only metadata.
bool load_game_manifest(const std::string& path, GameDef& out, std::string& err);

// Load every *.game file in `dir` (sorted by file name).  Returns the number
// of usable games; `err` is only set for a directory that exists but cannot be
// read.  Manifests whose profile is unknown are reported and skipped.
int discover_games(const std::string& dir, std::vector<GameDef>& out,
                   std::string& err);

// The three shipped episodes, described by the compiled profiles.  Used when
// no manifest directory is present, so a bare executable still runs.
std::vector<GameDef> builtin_games();

// discover_games() with the built-in list as the fallback.
std::vector<GameDef> load_game_list(const std::string& dir, std::string& err);

const GameDef* game_by_id(const std::vector<GameDef>& games,
                          const std::string& id);
