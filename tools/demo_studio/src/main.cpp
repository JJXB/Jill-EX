#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#include "studio.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

namespace fs = std::filesystem;

// Find the folder holding jill.ini: the working directory first, then beside
// the executable and up to four parents (so the exe can live in
// tools/demo_studio/build).
std::string config_directory(const char* argv0) {
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
        exe_dir = fs::absolute(argv0, ec).parent_path();
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
        "jill_demo_studio - external .mac playback viewer/editor\n"
        "\n"
        "  --mac FILE       .mac to open (default: the episode's first demo)\n"
        "  --new FILE       create a fresh .mac (one neutral tick-0 record)\n"
        "  --game ID        game manifest id (jill1, jill2, jill3, onesimus)\n"
        "  --episode N      N-th installed game, 1-based\n"
        "  --board NAME     starting board (default: from the .mac demo table)\n"
        "  --level N        spawn CHECKPT level id (-1 = the board's own spawn)\n"
        "  --games DIR      *.game manifests (default games)\n"
        "  --profiles DIR   *.profile engine data (default profiles)\n"
        "  --data DIR       override the game's asset directory\n"
        "  --palette FILE   override the game's 768-byte palette\n"
        "  --external DIR   external pack directory (default external)\n"
        "  --pack ID        mount an external pack at start-up\n"
        "  --respawn N      death reset: 0 checkpoint, 1 level start, 2 none\n"
        "  --no-phoenix-water-death  phoenix passes waterfalls (splash only)\n"
        "  --throw-from-vines        allow throwing while climbing\n"
        "  --interp MODE    smooth | off | original  (default smooth)\n"
        "  --hud            draw the classic HUD panel\n"
        "  --no-portrait    hide the level-title portrait\n"
        "  --window WxH     initial window size (default 1360x780)\n"
        "  --headless       no window: advance --ticks and write --out\n"
        "  --selftest       load --mac, check the parser/serialiser round-trip\n"
        "  --ticks N        headless: advance N world ticks before the dump\n"
        "  --trace FILE     headless: write a per-tick player/state CSV\n"
        "  --dump-map FILE  headless: write the live 128x64 tile map as text\n"
    "  --dump-objects FILE  headless: write the object table at the end tick\n"
        "  --classic-camera use the original edge-triggered scroll during playback\n"
        "  --out FILE       headless frame PPM; with --selftest, the .mac to write\n"
        "  --screenshot-dir DIR  where F9 writes PPM screenshots\n"
        "  --help\n");
}

}  // namespace

int main(int argc, char** argv) {
    const std::string cfg = config_directory(argc > 0 ? argv[0] : nullptr);
    if (!cfg.empty()) {
        std::error_code ec;
        fs::current_path(cfg, ec);
    }

    StudioOptions opt;
    bool selftest = false;
    auto value = [&](int& i, const char* what) -> const char* {
        if (i + 1 >= argc) {
            std::fprintf(stderr, "%s needs a value\n", what);
            std::exit(2);
        }
        return argv[++i];
    };
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--mac") {
            opt.mac = value(i, "--mac");
        } else if (a == "--new") {
            opt.new_macro = value(i, "--new");
        } else if (a == "--game") {
            opt.game = value(i, "--game");
        } else if (a == "--episode") {
            opt.episode = std::atoi(value(i, "--episode"));
        } else if (a == "--board") {
            opt.board = value(i, "--board");
        } else if (a == "--level") {
            opt.level = std::atoi(value(i, "--level"));
        } else if (a == "--games") {
            opt.games_dir = value(i, "--games");
        } else if (a == "--profiles") {
            opt.profiles_dir = value(i, "--profiles");
        } else if (a == "--data") {
            opt.data_dir = value(i, "--data");
        } else if (a == "--palette") {
            opt.palette = value(i, "--palette");
        } else if (a == "--external") {
            opt.external_dir = value(i, "--external");
        } else if (a == "--pack") {
            opt.pack = value(i, "--pack");
        } else if (a == "--respawn") {
            opt.respawn_mode = std::atoi(value(i, "--respawn"));
        } else if (a == "--no-phoenix-water-death") {
            opt.phoenix_water_death = false;
        } else if (a == "--throw-from-vines") {
            opt.throw_from_vines = true;
        } else if (a == "--interp") {
            const std::string mode = value(i, "--interp");
            if (mode == "off" || mode == "1") {
                opt.interpolation = 1;
            } else if (mode == "original" || mode == "2") {
                opt.interpolation = 2;
            } else {
                opt.interpolation = 0;
            }
        } else if (a == "--hud") {
            opt.hud_classic = true;
        } else if (a == "--no-portrait") {
            opt.hud_portrait = false;
        } else if (a == "--window") {
            const char* v = value(i, "--window");
            int w = 0;
            int h = 0;
            if (std::sscanf(v, "%dx%d", &w, &h) == 2 && w >= 960 && h >= 600) {
                opt.window_w = w;
                opt.window_h = h;
            }
        } else if (a == "--headless") {
            opt.headless = true;
        } else if (a == "--selftest") {
            opt.headless = true;
            selftest = true;
        } else if (a == "--ticks") {
            opt.ticks = std::atoi(value(i, "--ticks"));
        } else if (a == "--out") {
            opt.out = value(i, "--out");
        } else if (a == "--trace") {
            opt.trace = value(i, "--trace");
        } else if (a == "--dump-map") {
            opt.dump_map = value(i, "--dump-map");
        } else if (a == "--dump-objects") {
            opt.dump_objects = value(i, "--dump-objects");
        } else if (a == "--classic-camera") {
            opt.classic_camera = true;
        } else if (a == "--screenshot-dir") {
            opt.screenshot_dir = value(i, "--screenshot-dir");
        } else if (a == "--help" || a == "-h") {
            usage();
            return 0;
        } else {
            std::fprintf(stderr, "unknown argument: %s\n", a.c_str());
            usage();
            return 2;
        }
    }

    // The dump/advance switches only mean anything without a window; treating
    // them as an implicit --headless avoids a silent no-op run.
    if (opt.ticks > 0 || !opt.trace.empty() || !opt.dump_map.empty() ||
        !opt.dump_objects.empty()) {
        opt.headless = true;
    }

    std::string err;
    if (opt.headless) {
        Studio studio;
        if (!studio.init(opt, err)) {
            std::fprintf(stderr, "demo studio: %s\n", err.c_str());
            return 2;
        }
        const int rc = selftest ? studio.run_selftest(err)
                                : studio.run_headless(err);
        if (rc != 0) {
            std::fprintf(stderr, "demo studio: %s\n", err.c_str());
        }
        return rc;
    }

    int rc = 0;
    {
        Studio studio;
        if (!studio.init(opt, err)) {
            std::fprintf(stderr, "demo studio: %s\n", err.c_str());
            rc = 2;
        } else {
            rc = studio.run();
        }
    }
    SDL_Quit();
    return rc;
}
