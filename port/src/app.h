#pragma once
#include <map>
#include <string>

struct Options {
    int episode = 0;            // 0 = first installed game, N = games[N-1]
    std::string game;           // --game ID: manifest id, wins over episode
    std::string games_dir;      // --games DIR: manifest directory (default games)
    std::string profiles_dir;   // --profiles DIR: runtime profiles (default profiles)
    bool list_games = false;    // --list-games
    std::string data_dir;       // --data DIR: overrides the manifest's directory
    // [paths] in jill.ini: per-game id -> asset directory.  Missing entries
    // keep the manifest's local default (jillt / ONESIM).
    std::map<std::string, std::string> data_paths;
    std::string board;          // empty = the game's first board
    std::string palette;        // optional raw 768-byte VGA palette
    int ticks = 0;              // > 0 = headless, run that many updates
    std::string dump;           // optional PPM of the final headless frame
    std::string dump_splash;    // optional PPM prefix for the splash pages
    std::string dump_objects;   // optional text table of the object list
    int start_x = -1;           // --start X,Y: headless player warp
    int start_y = -1;
    int level = -1;             // --level N: headless header level id override
    int scale = 3;              // windowed integer scale (1..6)
    int high_fps = 60;          // presentation rate: 0 = 18.2 Hz, else 60..144
    bool interpolate = true;    // --no-interpolate: no high-fps smoothing
    int window_mode = 0;        // 0 windowed, 1 fullscreen, 2 borderless
    int resolution = 0;         // 0 desktop, else index into kResolutions
    int resolution_w = 0;       // --resolution WxH, resolved at start-up
    int resolution_h = 0;
    int items = 0;              // --items N: debug, start with N knives
    bool blade = false;         // --blade: debug, spawn one SPINBLAD
    int morph = -1;             // --morph N: debug, transform (0, 4..7)
    int face = 0;               // --face N: debug, force facing (-1/1)
    bool fire = false;          // --fire: debug, press throw on the first tick
    int text_page = 0;          // --text-page N: open a .vcl page at start
    int watch_tile = -1;        // --watch-tile N: headless, dump one tile record
    int max_knives = 3;         // --max-knives N: enhancement default (3/5)
    int hud_bg = -2;            // --hud-bg: palette index, -2 suit, -3 vibrant
    int hud_text = 7;           // --hud-text: HUD label colour palette index
    bool fullscreen = false;    // legacy alias for window_mode = 1
    bool hud_classic = false;   // --hud: use the original HUD panel art
    bool invincible = false;    // --invincible: debug no-damage mode
    int super_jump = 0;         // --super-jump N: cheat, minimum feather count (0..15)
    int menu_theme = 0;         // --menu-theme N: 0 plain, N = games[N-1] intro board
    int splash = 0;             // --splash N: 0 off, N = games[N-1] publisher screen
    int save_slot = -1;         // --save N: headless debug, write save slot N
    int load_slot = -1;         // --load N: load save slot N before running
    bool fire_crouched = false; // --fire-crouched: enhancement default
    bool retain_items = false;  // --retain-items: enhancement default
    bool fast_dialogs = false;  // --fast-dialogs: shorter, revealing messages
    bool message_box = true;    // --no-message-box: plain text, no frame
    bool air_jump_buffer = false; // --air-jump-buffer: hold jump through a fall
    bool minimap = false;       // --minimap: board overview overlay
    bool center_fish_shot = false; // --center-fish-shot: vertical fish shot
    int player_look = 0;       // --jill-look N: 0 game default, N = games[N-1]
    int player_suit = 0;       // --jill-colour N: 0 default, 1..8 presets
};

int run_app(const Options& opt);
