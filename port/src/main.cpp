#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#include "app.h"
#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

// Directory holding jill.ini (and therefore the relative games/, profiles/ and
// asset folders).  Search the working directory first, then the executable's
// directory and up to four parents, so launching port\build\Release\jill.exe
// directly still finds the repository configuration.
std::string config_directory(const char* argv0) {
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path cwd = fs::current_path(ec);
    if (!ec && fs::exists(cwd / "jill.ini", ec)) {
        return cwd.string();
    }

    fs::path exe_dir;
#ifdef _WIN32
    char buf[MAX_PATH];
    const DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        exe_dir = fs::path(std::string(buf, n)).parent_path();
    }
#endif
    if (exe_dir.empty() && argv0 && *argv0) {
        const fs::path p = fs::absolute(argv0, ec);
        if (!ec) {
            exe_dir = p.parent_path();
        }
    }
    if (!exe_dir.empty()) {
        fs::path p = exe_dir;
        for (int i = 0; i < 5; ++i) {
            if (fs::exists(p / "jill.ini", ec)) {
                return p.string();
            }
            if (!p.has_parent_path()) {
                break;
            }
            const fs::path parent = p.parent_path();
            if (parent == p) {
                break;
            }
            p = parent;
        }
    }
    return {};
}

void usage() {
    std::printf(
        "jill - Jill of the Jungle port skeleton\n"
        "\n"
        "  --game ID      game manifest id, e.g. jill1 (see --list-games)\n"
        "  --games DIR    directory of *.game manifests (default games)\n"
        "  --profiles DIR directory of *.profile engine data (default profiles)\n"
        "  --list-games   print the installed games and exit\n"
        "  --external DIR external asset directory (default external)\n"
        "  --pack ID      mount an external pack folder or .zip at start-up\n"
        "  --list-packs   print the external packs and exit\n"
        "  --episode N    N-th installed game, 1-based (default: the first)\n"
        "  --data DIR     override the manifest's asset directory\n"
        "  --board NAME   board name without extension (default: episode start)\n"
        "  --palette FILE optional 768-byte VGA palette\n"
        "  --ticks N      headless: run N updates and print state\n"
        "  --dump FILE    with --ticks: write the final frame as binary PPM\n"
        "  --dump-splash P  write the selected splash pages as P1.ppm, P2.ppm\n"
        "  --dump-objects FILE  with --ticks: write the object table as text\n"
        "  --start X,Y    with --ticks: put the player at X,Y (debug)\n"
        "  --level N      with --ticks: set the board header level id (debug)\n"
        "  --scale N      windowed integer scale, 1..6 (default 3)\n"
        "  --high-fps N   presentation rate: 0 (18.2 Hz), 60, 90, 120 or 144\n"
        "  --no-interpolate  disable high-framerate sprite smoothing\n"
        "  --filter N     display filter: 0 none, 1 smooth, 2 CRT\n"
        "  --fullscreen   start in exclusive fullscreen\n"
        "  --windowed     start windowed (default)\n"
        "  --borderless   start in borderless fullscreen\n"
        "  --resolution WxH  window/display size, or 'desktop' (default)\n"
        "  --hud          draw the original HUD panel art (F2 toggles it)\n"
        "  --no-portrait  hide the character portrait beside level-title dialogs\n"
        "  --invincible   debug: the player takes no damage (F5 toggles)\n"
        "  --super-jump N cheat: treat N feathers (item 9) as carried, 0..15\n"
        "  --menu-theme N front-end theme: 0 port, N = N-th game's intro board\n"
        "  --splash N     publisher screen: 0 off, N = N-th game's intro page\n"
        "  --save N       debug: write save slot N after the run / on exit\n"
        "  --load N       debug: restore save slot N before the run\n"
        "  --fire-crouched  enhancement default: fire while crouched\n"
        "  --retain-items   enhancement default: keep keys/weapons between levels\n"
        "  --fast-dialogs   enhancement default: 4 chars/tick for captions/pages\n"
        "  --no-message-box plain bottom messages, no background frame\n"
        "  --air-jump-buffer keep the original hold-jump-through-a-fall buffer\n"
        "  --minimap      draw a small board overview in the lower-right corner\n"
        "  --center-fish-shot  centre the fish form's shot vertically\n"
        "  --jill-look N   0 = game default, N = installed game N (Jill Select)\n"
        "  --jill-colour N 0 = look's colours, 1..8 = custom suit presets\n"
        "  --items N      debug: start with N knives in the inventory\n"
        "  --blade        debug: spawn one spinning blade in front of the player\n"
        "  --morph N      debug: transform (0 = normal, 4 frog, 5 bird, 6 spider, 7 fish)\n"
        "  --face N       debug: force facing -1 (left) or 1 (right)\n"
        "  --fire         debug: press throw once at the start\n"
        "  --text-page N  debug: open .vcl text page N (1 = How to Play)\n"
        "  --watch-tile N debug: print tile N's live record every tick (headless)\n"
        "  --max-knives N enhancement default: knife cap 3 or 5\n"
        "  --hud-bg N     HUD background palette index, 'suit' or 'vibrant'\n"
        "  --hud-text N   HUD label colour palette index (default 7)\n"
        "  --hud-health N HUD health colour 0-15, or 'original' (default)\n"
        "  --no-music     disable the OPL2/CMF music player\n"
        "  --music-volume N  music volume 0-100 (default 80)\n"
        "  --no-sfx       disable sampled sound effects\n"
        "  --sfx-volume N  effect volume 0-100 (default 80)\n"
        "  --pc-speaker   play effects as 1-bit PC-speaker square waves\n"
        "  --music-file NAME track for --dump-music (default: the game title track)\n"
        "  --music-seconds N seconds to render with --dump-music (default 10)\n"
        "  --dump-music FILE  render the current title track to a WAV and exit\n"
        "  --audio-check  open the audio device, play 1.5 s and report levels\n"
        "  --help\n");
}

}  // namespace

int main(int argc, char** argv) {
    // Make relative `games/`, `profiles/`, asset and save paths work when the
    // executable is launched from Explorer or another working directory.
    const std::string cfg_dir = config_directory(argc > 0 ? argv[0] : nullptr);
    if (!cfg_dir.empty()) {
        std::error_code ec;
        std::filesystem::current_path(cfg_dir, ec);
    }
    Options opt;
    // jill.ini supplies the defaults; command-line arguments override it.
    IniFile ini;
    std::string ini_err;
    if (ini.load("jill.ini", ini_err)) {
        opt.games_dir = ini.get("paths", "games", opt.games_dir);
        opt.profiles_dir = ini.get("paths", "profiles", opt.profiles_dir);
        opt.game = ini.get("game", "id", opt.game);
        opt.external_dir = ini.get("external", "dir", opt.external_dir);
        opt.pack = ini.get("external", "pack", opt.pack);
        opt.window_mode = ini.get_int("video", "window_mode", opt.window_mode);
        opt.scale = ini.get_int("video", "scale", opt.scale);
        opt.resolution = ini.get_int("video", "resolution", opt.resolution);
        opt.high_fps = ini.get_int("video", "high_fps", opt.high_fps);
        opt.interpolate = ini.get_bool("video", "interpolate", opt.interpolate);
        opt.splash = ini.get_int("video", "splash", opt.splash);
        const std::string filter = ini.get("video", "filter");
        if (!filter.empty()) {
            const std::string lf = ini_lower(filter);
            if (lf == "smooth") {
                opt.filter = 1;
            } else if (lf == "crt") {
                opt.filter = 2;
            } else {
                opt.filter = std::atoi(filter.c_str());
            }
            if (opt.filter < 0 || opt.filter > 2) {
                opt.filter = 0;
            }
        }
        opt.hud_classic = ini.get_bool("hud", "classic", opt.hud_classic);
        opt.hud_portrait = ini.get_bool("hud", "portrait", opt.hud_portrait);
        const std::string bg = ini.get("hud", "background");
        if (!bg.empty()) {
            const std::string lb = ini_lower(bg);
            if (lb == "suit") {
                opt.hud_bg = -2;
            } else if (lb == "vibrant") {
                opt.hud_bg = -3;
            } else {
                opt.hud_bg = std::atoi(bg.c_str());
            }
        }
        opt.hud_text = ini.get_int("hud", "text", opt.hud_text) & 0xFF;
        const std::string health = ini.get("hud", "health");
        if (!health.empty()) {
            if (ini_lower(health) == "original") {
                opt.hud_health = -1;
            } else {
                opt.hud_health = std::atoi(health.c_str()) & 0x0F;
            }
        }
        opt.music = ini.get_bool("audio", "music", opt.music);
        opt.music_volume = ini.get_int("audio", "volume", opt.music_volume);
        opt.sfx = ini.get_bool("audio", "sfx", opt.sfx);
        opt.sfx_volume = ini.get_int("audio", "sfx_volume", opt.sfx_volume);
        opt.pc_speaker = ini.get_bool("audio", "pc_speaker", opt.pc_speaker);
        opt.fire_crouched = ini.get_bool("gameplay", "fire_crouched",
                                         opt.fire_crouched);
        opt.max_knives = ini.get_int("gameplay", "max_knives", opt.max_knives);
        opt.retain_items = ini.get_bool("gameplay", "retain_items",
                                        opt.retain_items);
        opt.fast_dialogs = ini.get_bool("gameplay", "fast_dialogs",
                                        opt.fast_dialogs);
        opt.message_box = ini.get_bool("gameplay", "message_box",
                                       opt.message_box);
        opt.air_jump_buffer = ini.get_bool("gameplay", "air_jump_buffer",
                                           opt.air_jump_buffer);
        opt.minimap = ini.get_bool("gameplay", "minimap", opt.minimap);
        opt.center_fish_shot = ini.get_bool("gameplay", "center_fish_shot",
                                            opt.center_fish_shot);
        opt.player_look = ini.get_int("gameplay", "player_look", opt.player_look);
        opt.player_suit = ini.get_int("gameplay", "player_suit", opt.player_suit);
        opt.data_paths = ini.section("paths");
        opt.menu_theme = ini.get_int("gameplay", "menu_theme", opt.menu_theme);
    }
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto value = [&](const char* what) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "%s needs a value\n", what);
                std::exit(2);
            }
            return argv[++i];
        };
        if (a == "--game") {
            opt.game = value("--game");
        } else if (a == "--games") {
            opt.games_dir = value("--games");
        } else if (a == "--profiles") {
            opt.profiles_dir = value("--profiles");
        } else if (a == "--list-games") {
            opt.list_games = true;
        } else if (a == "--external") {
            opt.external_dir = value("--external");
        } else if (a == "--pack") {
            opt.pack = value("--pack");
        } else if (a == "--list-packs") {
            opt.list_packs = true;
        } else if (a == "--episode") {
            opt.episode = std::atoi(value("--episode"));
            opt.game.clear();  // explicit episode overrides jill.ini's game id
        } else if (a == "--data") {
            opt.data_dir = value("--data");
        } else if (a == "--board") {
            opt.board = value("--board");
        } else if (a == "--palette") {
            opt.palette = value("--palette");
        } else if (a == "--ticks") {
            opt.ticks = std::atoi(value("--ticks"));
        } else if (a == "--dump") {
            opt.dump = value("--dump");
        } else if (a == "--dump-splash") {
            opt.dump_splash = value("--dump-splash");
        } else if (a == "--dump-objects") {
            opt.dump_objects = value("--dump-objects");
        } else if (a == "--start") {
            const char* v = value("--start");
            opt.start_x = std::atoi(v);
            const char* comma = std::strchr(v, ',');
            opt.start_y = comma ? std::atoi(comma + 1) : -1;
            if (opt.start_x < 0 || opt.start_y < 0) {
                std::fprintf(stderr, "--start needs X,Y\n");
                return 2;
            }
        } else if (a == "--level") {
            opt.level = std::atoi(value("--level"));
            if (opt.level < 0) {
                opt.level = 0;
            }
        } else if (a == "--scale") {
            opt.scale = std::atoi(value("--scale"));
        } else if (a == "--high-fps") {
            opt.high_fps = std::atoi(value("--high-fps"));
        } else if (a == "--no-interpolate") {
            opt.interpolate = false;
        } else if (a == "--filter") {
            const char* v = value("--filter");
            if (std::strcmp(v, "smooth") == 0) {
                opt.filter = 1;
            } else if (std::strcmp(v, "crt") == 0) {
                opt.filter = 2;
            } else {
                opt.filter = std::atoi(v);
            }
            if (opt.filter < 0 || opt.filter > 2) {
                opt.filter = 0;
            }
        } else if (a == "--fullscreen") {
            opt.fullscreen = true;
            opt.window_mode = 1;
        } else if (a == "--windowed") {
            opt.window_mode = 0;
        } else if (a == "--borderless") {
            opt.window_mode = 2;
        } else if (a == "--resolution") {
            const char* v = value("--resolution");
            if (std::strcmp(v, "desktop") != 0) {
                int w = 0;
                int h = 0;
                if (std::sscanf(v, "%dx%d", &w, &h) != 2 || w <= 0 || h <= 0) {
                    std::fprintf(stderr, "--resolution needs WxH or 'desktop'\n");
                    return 2;
                }
                opt.resolution_w = w;
                opt.resolution_h = h;
            }
        } else if (a == "--hud") {
            opt.hud_classic = true;
        } else if (a == "--no-portrait") {
            opt.hud_portrait = false;
        } else if (a == "--portrait") {
            opt.hud_portrait = true;
        } else if (a == "--invincible") {
            opt.invincible = true;
        } else if (a == "--super-jump") {
            int n = 15;  // bare flag = the strongest allowed setting
            if (i + 1 < argc && argv[i + 1][0] >= '0' && argv[i + 1][0] <= '9') {
                n = std::atoi(argv[++i]);
            }
            opt.super_jump = (n < 0) ? 0 : (n > 15 ? 15 : n);
        } else if (a == "--menu-theme") {
            int n = std::atoi(value("--menu-theme"));
            opt.menu_theme = (n < 0) ? 0 : (n > 3 ? 3 : n);
        } else if (a == "--splash") {
            int n = std::atoi(value("--splash"));
            opt.splash = (n < 0) ? 0 : n;
        } else if (a == "--save") {
            opt.save_slot = std::atoi(value("--save"));
        } else if (a == "--load") {
            opt.load_slot = std::atoi(value("--load"));
        } else if (a == "--fire-crouched") {
            opt.fire_crouched = true;
        } else if (a == "--retain-items") {
            opt.retain_items = true;
        } else if (a == "--fast-dialogs") {
            opt.fast_dialogs = true;
        } else if (a == "--no-message-box") {
            opt.message_box = false;
        } else if (a == "--air-jump-buffer") {
            opt.air_jump_buffer = true;
        } else if (a == "--minimap") {
            opt.minimap = true;
        } else if (a == "--center-fish-shot") {
            opt.center_fish_shot = true;
        } else if (a == "--jill-look") {
            opt.player_look = std::atoi(value("--jill-look"));
        } else if (a == "--jill-colour") {
            opt.player_suit = std::atoi(value("--jill-colour"));
        } else if (a == "--items") {
            opt.items = std::atoi(value("--items"));
        } else if (a == "--blade") {
            opt.blade = true;
        } else if (a == "--morph") {
            opt.morph = std::atoi(value("--morph"));
        } else if (a == "--face") {
            opt.face = std::atoi(value("--face"));
        } else if (a == "--fire") {
            opt.fire = true;
        } else if (a == "--text-page") {
            opt.text_page = std::atoi(value("--text-page"));
        } else if (a == "--watch-tile") {
            opt.watch_tile = std::atoi(value("--watch-tile"));
        } else if (a == "--max-knives") {
            opt.max_knives = std::atoi(value("--max-knives"));
        } else if (a == "--hud-bg") {
            const char* v = value("--hud-bg");
            if (std::strcmp(v, "suit") == 0) {
                opt.hud_bg = -2;
            } else if (std::strcmp(v, "vibrant") == 0) {
                opt.hud_bg = -3;
            } else {
                opt.hud_bg = std::atoi(v);
            }
        } else if (a == "--hud-text") {
            opt.hud_text = std::atoi(value("--hud-text")) & 0xFF;
        } else if (a == "--hud-health") {
            const char* v = value("--hud-health");
            if (std::strcmp(v, "original") == 0) {
                opt.hud_health = -1;
            } else {
                opt.hud_health = std::atoi(v) & 0x0F;
            }
        } else if (a == "--no-music") {
            opt.music = false;
        } else if (a == "--music-volume") {
            opt.music_volume = std::atoi(value("--music-volume"));
        } else if (a == "--no-sfx") {
            opt.sfx = false;
        } else if (a == "--sfx-volume") {
            opt.sfx_volume = std::atoi(value("--sfx-volume"));
        } else if (a == "--pc-speaker") {
            opt.pc_speaker = true;
        } else if (a == "--music-file") {
            opt.music_file = value("--music-file");
        } else if (a == "--music-seconds") {
            opt.music_seconds = std::atof(value("--music-seconds"));
        } else if (a == "--dump-music") {
            opt.dump_music = value("--dump-music");
        } else if (a == "--audio-check") {
            opt.audio_check = true;
        } else if (a == "--help" || a == "-h") {
            usage();
            return 0;
        } else {
            std::fprintf(stderr, "unknown argument: %s\n", a.c_str());
            usage();
            return 2;
        }
    }
    if (opt.scale < 1 || opt.scale > 10) {
        opt.scale = 3;
    }
    return run_app(opt);
}
