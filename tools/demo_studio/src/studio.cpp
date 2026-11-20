#include "studio.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <system_error>

#include "external.h"
#include "platform_paths.h"
#include "profile.h"
#include "timing.h"
#include "ui.h"

namespace fs = std::filesystem;

namespace {

constexpr double kTickStep = jill_time::kLogicStep;  // one original BIOS tick
constexpr int kPad = 16;
constexpr int kHeaderH = 40;
constexpr int kFooterH = 36;

struct DemoPreset {
    const char* macro;
    const char* game;
    const char* board;
    int level;
};

// The same board/level pairs the game's front-end DEMO menu uses
// (docs/jill23-frontend-flow.md section 3).
constexpr DemoPreset kPresets[] = {
    {"jn1demo.mac", "jill1", "intro", 100},
    {"jn2dem1.mac", "jill2", "3.jn2", 3},
    {"jn2dem2.mac", "jill2", "9.jn2", 9},
    {"jn2dem3.mac", "jill2", "17.jn2", 17},
    {"jn3dem1.mac", "jill3", "1.jn3", 1},
    {"jn3dem2.mac", "jill3", "5.jn3", 5},
    {"jn3dem3.mac", "jill3", "12.jn3", 12},
};

const char* kFieldNames[] = {"TICK", "H", "V", "B1", "B2", "KEY", "MASK"};

std::string lower(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return s;
}

std::string file_name_of(const std::string& path) {
    return fs::path(path).filename().string();
}

std::string dir_of(const std::string& path) {
    return fs::path(path).parent_path().string();
}

std::string mask_letters(int mask) {
    std::string s;
    s += (mask & kMacroMaskH) ? 'H' : '.';
    s += (mask & kMacroMaskV) ? 'V' : '.';
    s += (mask & kMacroMaskBtn1) ? '1' : '.';
    s += (mask & kMacroMaskBtn2) ? '2' : '.';
    s += (mask & kMacroMaskKey) ? 'K' : '.';
    return s;
}

std::string dir_symbol(int v, char neg, char pos) {
    if (v < 0) {
        return std::string(1, neg);
    }
    if (v > 0) {
        return std::string(1, pos);
    }
    return "-";
}

std::string key_label(int key) {
    if (key <= 0) {
        return "-";
    }
    if (key >= 32 && key < 127) {
        return std::string("'") + static_cast<char>(key) + "'";
    }
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02X", key & 0xFF);
    return buf;
}

std::string tick_label(int t) { return std::to_string(t); }

bool load_palette_file(const std::string& path, uint8_t rgb[768]) {
    vga_default_palette(rgb);
    if (path.empty()) {
        return true;
    }
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        return false;
    }
    const size_t got = std::fread(rgb, 1, 768, f);
    std::fclose(f);
    return got == 768;
}

double seconds_now() {
    static const uint64_t freq = SDL_GetPerformanceFrequency();
    return static_cast<double>(SDL_GetPerformanceCounter()) /
           static_cast<double>(freq);
}

int clamp_int(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

uint32_t rgb_of(const uint8_t pal[768], uint8_t index) {
    return (static_cast<uint32_t>(pal[index * 3 + 0]) << 16) |
           (static_cast<uint32_t>(pal[index * 3 + 1]) << 8) |
           static_cast<uint32_t>(pal[index * 3 + 2]);
}

}  // namespace

// Layout of the whole window, recomputed from the current drawable size.
struct Studio::Layout {
    int w = 0;
    int h = 0;
    demo_ui::Rect header;
    demo_ui::Rect footer;
    demo_ui::Rect game;
    demo_ui::Rect info;
    demo_ui::Rect panel;
    demo_ui::Rect track;
    demo_ui::Rect rows;
    demo_ui::Rect editor;
    int game_scale = 2;
    int panel_pad = 12;
    int row_h = 22;
    int visible_rows = 1;
    int col[9] = {};  // idx, tick, delta, H, V, B1, B2, key, mask
};

Studio::~Studio() {
    // The UI owns a texture created by this renderer, so release it first.
    if (ui_) {
        ui_->shutdown();
    }
    if (frame_tex_) {
        SDL_DestroyTexture(frame_tex_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
}

// --------------------------------------------------------------------------
// startup
// --------------------------------------------------------------------------

bool Studio::init(const StudioOptions& opt, std::string& err) {
    opt_ = opt;
    interp_mode_ = clamp_int(opt.interpolation, 0, 2);
    headless_ = opt.headless;

    // jill.ini supplies asset paths exactly like the game, so the tool finds
    // the same install without extra arguments.
    IniFile ini;
    std::string ini_err;
    if (ini.load(jill_paths::resolve("jill.ini"), ini_err)) {
        opt_.games_dir = ini.get("paths", "games", opt_.games_dir);
        opt_.profiles_dir = ini.get("paths", "profiles", opt_.profiles_dir);
        opt_.external_dir = ini.get("external", "dir", opt_.external_dir);
        if (opt_.pack.empty()) {
            opt_.pack = ini.get("external", "pack", opt_.pack);
        }
        opt_.hud_classic = ini.get_bool("hud", "classic", opt_.hud_classic);
        opt_.hud_portrait = ini.get_bool("hud", "portrait", opt_.hud_portrait);
        opt_.hud_panel = ini.get("hud", "panel", opt_.hud_panel);
        opt_.hud_layout = ini.get("hud", "layout", opt_.hud_layout);
        opt_.hud_text = ini.get_int("hud", "text", opt_.hud_text) & 0xFF;
        opt_.hud_font_scale =
            ini.get_int("hud", "font_scale", opt_.hud_font_scale);
        opt_.hud_frame = ini.get_int("hud", "frame", opt_.hud_frame);
        opt_.menu_font = ini.get("menus", "font", opt_.menu_font);
        const std::string bg = ini.get("hud", "background");
        if (bg == "suit") {
            opt_.hud_bg = -2;
        } else if (bg == "vibrant") {
            opt_.hud_bg = -3;
        } else if (!bg.empty()) {
            opt_.hud_bg = std::atoi(bg.c_str());
        }
        const std::string health = ini.get("hud", "health");
        if (health == "original") {
            opt_.hud_health = -1;
        } else if (!health.empty()) {
            opt_.hud_health = std::atoi(health.c_str()) & 0x0F;
        }
        opt_.fast_dialogs =
            ini.get_bool("gameplay", "fast_dialogs", opt_.fast_dialogs);
        opt_.animated_water =
            ini.get_bool("gameplay", "animated_water", opt_.animated_water);
        opt_.retain_items =
            ini.get_bool("gameplay", "retain_items", opt_.retain_items);
        opt_.air_jump_buffer =
            ini.get_bool("gameplay", "air_jump_buffer", opt_.air_jump_buffer);
        opt_.fire_crouched =
            ini.get_bool("gameplay", "fire_crouched", opt_.fire_crouched);
        opt_.max_knives =
            ini.get_int("gameplay", "max_knives", opt_.max_knives);
        opt_.center_fish_shot =
            ini.get_bool("gameplay", "center_fish_shot", opt_.center_fish_shot);
        opt_.minimap = ini.get_bool("gameplay", "minimap", opt_.minimap);
        opt_.turtle = ini.get_bool("gameplay", "turtle", opt_.turtle);
    }
    load_settings();

    if (!load_games(err)) {
        return false;
    }
    // Pick the starting episode.
    int index = 0;
    if (!opt_.game.empty()) {
        for (size_t i = 0; i < games_.size(); ++i) {
            if (games_[i].id == opt_.game) {
                index = static_cast<int>(i);
                break;
            }
        }
    } else if (opt_.episode >= 1 &&
               opt_.episode <= static_cast<int>(games_.size())) {
        index = opt_.episode - 1;
    }
    if (!select_game(index, err)) {
        return false;
    }

    // Open the requested macro, create a fresh one, or fall back to the
    // selected episode's first demo.
    std::string macro = opt_.mac;
    if (!opt_.new_macro.empty()) {
        save_name_ = opt_.new_macro;
        std::string new_err;
        if (!create_new_macro(new_err)) {
            err = "new .mac: " + new_err;
            return false;
        }
    } else if (macro.empty()) {
        const std::string& id = games_[static_cast<size_t>(game_index_)].id;
        const char* candidate = id == "jill2"   ? "jn2dem1.mac"
                                : id == "jill3" ? "jn3dem1.mac"
                                                : "jn1demo.mac";
        const std::string path =
            jill_paths::resolve_file(game_data_dir() + "/" + candidate);
        if (fs::exists(path)) {
            macro = path;
        }
    }
    if (!macro.empty()) {
        std::string open_err;
        if (!open_macro(macro, true, open_err)) {
            err = open_err;
            return false;
        }
    } else {
        board_ = opt_.board.empty() ? games_[size_t(game_index_)].first_board
                                    : opt_.board;
        level_ = opt_.level;
    }
    if (!opt_.board.empty()) {
        board_ = opt_.board;
    }
    if (opt_.level >= 0) {
        level_ = opt_.level;
    }

    std::string run_err;
    if (!start_run(0, run_err)) {
        err = run_err;
        return false;
    }
    playing_ = !records_.empty() && !finished_;  // start on the first frame
    if (headless_) {
        return true;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        err = std::string("SDL_Init: ") + SDL_GetError();
        return false;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window_ = SDL_CreateWindow(
        "Jill-EX Demo Studio", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, opt_.window_w, opt_.window_h,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        err = std::string("SDL_CreateWindow: ") + SDL_GetError();
        return false;
    }
    SDL_SetWindowMinimumSize(window_, 960, 600);
    renderer_ = SDL_CreateRenderer(
        window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    }
    if (!renderer_) {
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer_) {
        err = std::string("SDL_CreateRenderer: ") + SDL_GetError();
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
    frame_tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING, Framebuffer::W,
                                   Framebuffer::H);
    if (!frame_tex_) {
        err = std::string("SDL_CreateTexture(frame): ") + SDL_GetError();
        return false;
    }
    SDL_SetTextureScaleMode(frame_tex_, SDL_ScaleModeNearest);
    ui_.reset(new demo_ui::Ui());
    if (!ui_->init(renderer_, *engine_, err)) {
        return false;
    }
    if (opt_.screenshot_dir.empty()) {
        opt_.screenshot_dir =
            jill_paths::resolve("tools/demo_studio/shots");
    }
    refresh_file_list();
    ensure_window_fits();
    say("SPACE plays, F1 lists all keys", demo_ui::kTextDim, 6.0);
    return true;
}

bool Studio::load_games(std::string& err) {
    std::string profile_err;
    load_profile_dir(opt_.profiles_dir, profile_err);
    if (!profile_err.empty()) {
        std::fprintf(stderr, "profiles: %s\n", profile_err.c_str());
    }
    std::string game_err;
    games_ = load_game_list(opt_.games_dir, game_err);
    if (games_.empty()) {
        err = game_err.empty() ? "no game manifests found in " + opt_.games_dir
                               : game_err;
        return false;
    }
    IniFile ini;
    std::string ini_err;
    const bool have_ini = ini.load(jill_paths::resolve("jill.ini"), ini_err);
    for (GameDef& g : games_) {
        g.data_dir = jill_paths::resolve(g.data_dir);
        g.palette = jill_paths::resolve(g.palette);
        if (have_ini) {
            std::string data = ini.get("paths", g.id);
            if (!data.empty()) {
                g.data_dir = jill_paths::resolve(data);
            }
        }
        if (!opt_.data_dir.empty()) {
            g.data_dir = jill_paths::resolve(opt_.data_dir);
        }
        if (!opt_.palette.empty()) {
            g.palette = jill_paths::resolve(opt_.palette);
        }
    }
    return true;
}

bool Studio::select_game(int index, std::string& err) {
    if (index < 0 || index >= static_cast<int>(games_.size())) {
        err = "unknown episode";
        return false;
    }
    const GameDef& g = games_[static_cast<size_t>(index)];
    std::unique_ptr<Engine> fresh(new Engine());
    if (!fresh->init(g, err)) {
        err = g.id + ": " + err;
        return false;
    }
    fresh->set_hud_classic(opt_.hud_classic);
    fresh->set_classic_camera(opt_.classic_camera);
    fresh->set_hud_portrait(opt_.hud_portrait);
    fresh->set_hud_bg(opt_.hud_bg);
    fresh->set_hud_text(opt_.hud_text);
    fresh->set_hud_health(opt_.hud_health);
    fresh->set_hud_font_scale(opt_.hud_font_scale);
    fresh->set_hud_panel(opt_.hud_panel);
    fresh->set_hud_layout(opt_.hud_layout);
    fresh->set_font_name(opt_.menu_font);
    {
        std::string frame_err;
        const GameDef* frame_game = nullptr;
        if (opt_.hud_frame >= 1 &&
            opt_.hud_frame <= static_cast<int>(games_.size())) {
            frame_game = &games_[static_cast<size_t>(opt_.hud_frame - 1)];
        }
        if (!fresh->set_hud_frame_game(frame_game, frame_err) &&
            !frame_err.empty()) {
            std::fprintf(stderr, "hud frame: %s\n", frame_err.c_str());
        }
    }
    fresh->set_fast_dialogs(opt_.fast_dialogs);
    fresh->set_animated_water(opt_.animated_water);
    fresh->set_retain_items(opt_.retain_items);
    fresh->set_air_jump_buffer(opt_.air_jump_buffer);
    fresh->set_fire_while_crouched(opt_.fire_crouched);
    fresh->set_max_knives(opt_.max_knives);
    fresh->set_center_fish_shot(opt_.center_fish_shot);
    fresh->set_minimap(opt_.minimap);
    fresh->set_turtle(opt_.turtle);
    load_palette_file(g.palette, palette_);

    // Mount the optional external pack/autoload exactly like the game does, so
    // the preview shows the same assets.
    if (!opt_.external_dir.empty()) {
        ExternalAssets assets;
        std::string ext_err;
        if (assets.scan(opt_.external_dir, ext_err)) {
            if (!opt_.pack.empty()) {
                const int p = assets.find(opt_.pack);
                if (p >= 0) {
                    assets.mount(p, ext_err);
                }
            }
            if (assets.mounted()) {
                fresh->apply_external(assets, palette_);
            }
        }
    }
    engine_ = std::move(fresh);
    game_index_ = index;
    board_ = g.first_board;
    level_ = -1;

    // Board picker: the manifest's level-select list first, then every board
    // file actually present (demo boards such as 0.dem are not in the list).
    board_choices_ = g.boards;
    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(g.data_dir, ec)) {
        if (ec) {
            break;
        }
        std::error_code fec;
        if (!entry.is_regular_file(fec)) {
            continue;
        }
        const std::string ext = lower(entry.path().extension().string());
        // Demo boards are `.dem` files rather than manifest boards; include
        // them so the picker can jump straight into the original scenes.
        if (ext != lower(g.board_ext) && ext != ".dem") {
            continue;
        }
        const std::string stem = entry.path().stem().string();
        // `.dem` demo boards need the extension kept: the engine appends the
        // manifest's board extension when the name has none, so a bare "0"
        // would look for 0.jn1 instead of 0.dem.
        const std::string choice = (ext == ".dem") ? entry.path().filename().string()
                                                   : stem;
        if (std::find(board_choices_.begin(), board_choices_.end(), choice) ==
            board_choices_.end()) {
            board_choices_.push_back(choice);
        }
    }
    std::sort(board_choices_.begin() + std::min<size_t>(g.boards.size(),
                                                        board_choices_.size()),
              board_choices_.end());
    if (ui_) {
        std::string ui_err;
        ui_->init(renderer_, *engine_, ui_err);
    }
    return true;
}

std::string Studio::game_data_dir() const {
    if (game_index_ < 0 || game_index_ >= static_cast<int>(games_.size())) {
        return {};
    }
    return games_[static_cast<size_t>(game_index_)].data_dir;
}

bool Studio::open_macro(const std::string& path, bool apply_preset,
                        std::string& err) {
    // Accept a bare file name: look beside the game's assets as well.
    std::string resolved = path;
    std::error_code ec;
    if (!path.empty() && !fs::exists(path, ec)) {
        const std::string alt = game_data_dir() + "/" + path;
        if (fs::exists(alt, ec)) {
            resolved = alt;
        }
    }
    macro_path_ = resolved;
    records_.clear();
    undo_.clear();
    pending_open_.clear();
    quit_armed_ = false;
    selected_ = 0;
    field_ = 0;
    scroll_ = 0;
    editing_ = false;
    edit_buf_.clear();
    dirty_ = false;
    run_dirty_ = false;
    if (!resolved.empty()) {
        std::string diag;
        if (!macro_load(resolved, records_, diag)) {
            err = file_name_of(resolved) + ": " + diag;
            say(err, demo_ui::kBad, 6.0);
        } else if (!diag.empty()) {
            say("MACRO: " + diag, demo_ui::kWarn, 6.0);
        } else {
            char buf[160];
            std::snprintf(buf, sizeof(buf), "OPEN %s  (%d records, %d ticks)",
                          file_name_of(resolved).c_str(),
                          static_cast<int>(records_.size()),
                          macro_last_tick(records_));
            say(buf, demo_ui::kGood, 4.0);
        }
    }
    if (apply_preset) {
        apply_preset_for(resolved);
    }
    if (window_) {
        std::string title = "Jill-EX Demo Studio";
        if (!resolved.empty()) {
            title += " - " + file_name_of(resolved);
        }
        SDL_SetWindowTitle(window_, title.c_str());
    }
    refresh_file_list();
    return true;
}

void Studio::apply_preset_for(const std::string& path) {
    const std::string name = lower(file_name_of(path));
    for (const DemoPreset& p : kPresets) {
        if (name != p.macro) {
            continue;
        }
        // Explicit --game / --board / --level only pin that one field.
        if (opt_.game.empty() && games_[size_t(game_index_)].id != p.game) {
            for (size_t i = 0; i < games_.size(); ++i) {
                if (games_[i].id == p.game) {
                    std::string err;
                    select_game(static_cast<int>(i), err);
                    break;
                }
            }
        }
        if (games_[size_t(game_index_)].id != p.game) {
            return;  // a different episode was requested explicitly
        }
        if (opt_.board.empty()) {
            board_ = p.board;
        }
        if (opt_.level < 0) {
            level_ = p.level;
        }
        return;
    }
}

// --------------------------------------------------------------------------
// playback
// --------------------------------------------------------------------------

bool Studio::start_run(int target_tick, std::string& err) {
    if (!engine_) {
        err = "no engine";
        return false;
    }
    if (board_.empty()) {
        board_ = games_[static_cast<size_t>(game_index_)].first_board;
    }
    std::string load_err;
    if (!engine_->load_board(board_, load_err, level_)) {
        err = "load " + board_ + ": " + load_err;
        return false;
    }
    // Playback must start before the entry spawn so the camera/culling use the
    // original 232x160 simulation view (same order as the game's start_game).
    if (!records_.empty()) {
        engine_->set_macro_records(records_);
    } else {
        engine_->stop_macro();
    }
    if (level_ >= 0) {
        engine_->spawn_at_checkpoint(level_);
    }
    accumulator_ = 0.0;
    finished_ = false;
    run_dirty_ = false;
    const int target = std::max(0, target_tick);
    for (int i = 0; i < target; ++i) {
        if (engine_->macro_records().empty()) {
            break;
        }
        engine_->tick();
        if (!engine_->macro_active()) {
            finished_ = true;
            break;
        }
    }
    if (records_.empty()) {
        finished_ = true;
    }
    return true;
}

bool Studio::rebuild_to(int target_tick, std::string& err) {
    const int was_playing = playing_;
    std::string run_err;
    if (!start_run(target_tick, run_err)) {
        err = run_err;
        say(run_err, demo_ui::kBad, 6.0);
        return false;
    }
    playing_ = was_playing && !finished_;
    return true;
}

int Studio::playhead() const {
    return engine_ ? engine_->macro_tick() : 0;
}

void Studio::tick_once() {
    if (!engine_) {
        return;
    }
    const bool had_macro = !engine_->macro_records().empty();
    engine_->tick();
    if (had_macro && !engine_->macro_active()) {
        finished_ = true;
        playing_ = false;
        accumulator_ = 0.0;
        say("MACRO FINISHED", demo_ui::kWarn, 4.0);
    }
}

void Studio::toggle_play() {
    if (!engine_ || finished_) {
        seek(0);
        playing_ = !finished_;
        return;
    }
    playing_ = !playing_;
    accumulator_ = 0.0;
}

void Studio::step_ticks(int delta) {
    if (!engine_ || delta == 0) {
        return;
    }
    playing_ = false;
    accumulator_ = 0.0;
    if (delta > 0) {
        for (int i = 0; i < delta; ++i) {
            tick_once();
        }
        return;
    }
    std::string err;
    rebuild_to(std::max(0, playhead() + delta), err);
}

void Studio::seek(int target_tick) {
    if (!engine_) {
        return;
    }
    const int max_tick =
        std::max(macro_last_tick(records_), engine_->macro_duration());
    const int target = clamp_int(target_tick, 0, max_tick);
    const bool was_playing = playing_;
    std::string err;
    if (!start_run(target, err)) {
        say(err, demo_ui::kBad, 5.0);
        playing_ = false;
        return;
    }
    playing_ = was_playing && !finished_;
    accumulator_ = 0.0;
}

void Studio::set_interpolation_mode(int mode) {
    interp_mode_ = clamp_int(mode, 0, 2);
    accumulator_ = 0.0;
    say(std::string("INTERPOLATION: ") + interpolation_label(),
        demo_ui::kAccent, 3.0);
}

const char* Studio::interpolation_label() const {
    switch (interp_mode_) {
        case 0:
            return "SMOOTH (60 HZ, ALPHA BLEND)";
        case 1:
            return "OFF (60 HZ, HOLD LAST TICK)";
        default:
            return "ORIGINAL SPEED (18.2 HZ)";
    }
}

// --------------------------------------------------------------------------
// editing
// --------------------------------------------------------------------------

void Studio::push_undo() {
    Snapshot s;
    s.records = records_;
    s.selected = selected_;
    undo_.push_back(std::move(s));
    if (undo_.size() > 64) {
        undo_.erase(undo_.begin());
    }
}

void Studio::undo() {
    if (undo_.empty()) {
        say("NOTHING TO UNDO", demo_ui::kTextDim, 2.0);
        return;
    }
    records_ = std::move(undo_.back().records);
    selected_ = undo_.back().selected;
    undo_.pop_back();
    selected_ = clamp_int(selected_, 0, std::max(0, static_cast<int>(records_.size()) - 1));
    dirty_ = true;
    run_dirty_ = true;
    editing_ = false;
    ensure_visible();
    say("UNDO", demo_ui::kTextDim, 2.0);
}

namespace {

bool same_record(const MacroRecord& a, const MacroRecord& b) {
    return a.time == b.time && a.mask == b.mask && a.h == b.h && a.v == b.v &&
           a.btn1 == b.btn1 && a.btn2 == b.btn2 && a.key == b.key;
}

}  // namespace

void Studio::sort_records(int keep_index) {
    if (records_.empty()) {
        selected_ = 0;
        return;
    }
    keep_index = clamp_int(keep_index, 0, static_cast<int>(records_.size()) - 1);
    const MacroRecord key = records_[static_cast<size_t>(keep_index)];
    std::stable_sort(records_.begin(), records_.end(),
                     [](const MacroRecord& a, const MacroRecord& b) {
                         return a.time < b.time;
                     });
    selected_ = 0;
    for (size_t i = 0; i < records_.size(); ++i) {
        if (same_record(records_[i], key)) {
            selected_ = static_cast<int>(i);
            break;
        }
    }
}

void Studio::insert_record_at_playhead() {
    push_undo();
    const int tick = playhead();
    const MacroInputState st = macro_state_at(records_, tick);
    MacroRecord r;
    r.time = tick;
    r.mask = kMacroMaskKnown;
    r.h = st.h;
    r.v = st.v;
    r.btn1 = st.btn1;
    r.btn2 = st.btn2;
    r.key = st.key;
    // Insert after any record already at this tick so the new one wins.
    auto it = std::upper_bound(
        records_.begin(), records_.end(), tick,
        [](int t, const MacroRecord& rec) { return t < rec.time; });
    const int pos = static_cast<int>(it - records_.begin());
    records_.insert(it, r);
    selected_ = pos;
    dirty_ = true;
    run_dirty_ = true;
    editing_ = false;
    ensure_visible();
    say("INSERTED RECORD AT TICK " + std::to_string(tick), demo_ui::kGood, 3.0);
}

void Studio::delete_selected() {
    if (records_.empty()) {
        return;
    }
    push_undo();
    records_.erase(records_.begin() + selected_);
    selected_ = clamp_int(selected_, 0,
                          std::max(0, static_cast<int>(records_.size()) - 1));
    dirty_ = true;
    run_dirty_ = true;
    editing_ = false;
    ensure_visible();
}

void Studio::begin_edit() {
    if (records_.empty()) {
        return;
    }
    const MacroRecord& r = records_[static_cast<size_t>(selected_)];
    switch (static_cast<Field>(field_)) {
        case Field::Tick:
            edit_buf_ = std::to_string(r.time);
            break;
        case Field::H:
            edit_buf_ = std::to_string(r.h);
            break;
        case Field::V:
            edit_buf_ = std::to_string(r.v);
            break;
        case Field::Btn1:
            edit_buf_ = r.btn1 ? "1" : "0";
            break;
        case Field::Btn2:
            edit_buf_ = r.btn2 ? "1" : "0";
            break;
        case Field::Key:
            edit_buf_ = std::to_string(r.key);
            break;
        case Field::Mask:
            edit_buf_ = std::to_string(r.mask & kMacroMaskKnown);
            break;
        default:
            return;
    }
    editing_ = true;
}

void Studio::cancel_edit() {
    editing_ = false;
    edit_buf_.clear();
}

void Studio::commit_edit() {
    if (!editing_ || records_.empty()) {
        cancel_edit();
        return;
    }
    if (edit_buf_.empty() || edit_buf_ == "-") {
        cancel_edit();
        return;
    }
    const int value = std::atoi(edit_buf_.c_str());
    push_undo();
    MacroRecord& r = records_[static_cast<size_t>(selected_)];
    switch (static_cast<Field>(field_)) {
        case Field::Tick:
            r.time = std::max(0, value);
            sort_records(selected_);
            break;
        case Field::H:
            r.h = value < 0 ? -1 : (value > 0 ? 1 : 0);
            r.mask |= kMacroMaskH;
            break;
        case Field::V:
            r.v = value < 0 ? -1 : (value > 0 ? 1 : 0);
            r.mask |= kMacroMaskV;
            break;
        case Field::Btn1:
            r.btn1 = value != 0;
            r.mask |= kMacroMaskBtn1;
            break;
        case Field::Btn2:
            r.btn2 = value != 0;
            r.mask |= kMacroMaskBtn2;
            break;
        case Field::Key:
            r.key = clamp_int(value, 0, 255);
            r.mask |= kMacroMaskKey;
            break;
        case Field::Mask:
            r.mask = clamp_int(value, 0, kMacroMaskKnown);
            break;
        default:
            break;
    }
    cancel_edit();
    dirty_ = true;
    run_dirty_ = true;
    ensure_visible();
}

void Studio::nudge_field(int dir) {
    if (records_.empty() || dir == 0) {
        return;
    }
    push_undo();
    MacroRecord& r = records_[static_cast<size_t>(selected_)];
    switch (static_cast<Field>(field_)) {
        case Field::Tick:
            r.time = std::max(0, r.time + dir);
            sort_records(selected_);
            break;
        case Field::H:
            r.h = clamp_int(r.h + dir, -1, 1);
            r.mask |= kMacroMaskH;
            break;
        case Field::V:
            r.v = clamp_int(r.v + dir, -1, 1);
            r.mask |= kMacroMaskV;
            break;
        case Field::Btn1:
            r.btn1 = dir > 0;
            r.mask |= kMacroMaskBtn1;
            break;
        case Field::Btn2:
            r.btn2 = dir > 0;
            r.mask |= kMacroMaskBtn2;
            break;
        case Field::Key:
            r.key = clamp_int(r.key + dir, 0, 255);
            r.mask |= kMacroMaskKey;
            break;
        case Field::Mask:
            r.mask = dir > 0 ? kMacroMaskKnown : 0;
            break;
        default:
            break;
    }
    dirty_ = true;
    run_dirty_ = true;
    ensure_visible();
}

void Studio::toggle_mask_bit(int bit) {
    if (records_.empty() || bit < 0 || bit > 4) {
        return;
    }
    push_undo();
    MacroRecord& r = records_[static_cast<size_t>(selected_)];
    const int mask_bit = 1 << bit;
    if (r.mask & mask_bit) {
        r.mask &= ~mask_bit;
    } else {
        r.mask |= mask_bit;
    }
    dirty_ = true;
    run_dirty_ = true;
    say("MASK " + mask_letters(r.mask), demo_ui::kTextDim, 2.0);
}

void Studio::select_row(int index) {
    if (records_.empty()) {
        selected_ = 0;
        return;
    }
    selected_ = clamp_int(index, 0, static_cast<int>(records_.size()) - 1);
    ensure_visible();
}

void Studio::ensure_visible() {
    const Layout l = layout();
    const int visible = std::max(1, l.visible_rows);
    while (selected_ < scroll_) {
        scroll_ = selected_;
    }
    while (selected_ >= scroll_ + visible) {
        scroll_ = selected_ - visible + 1;
    }
    scroll_ = std::max(0, scroll_);
}

bool Studio::save_macro(bool save_as, std::string& err) {
    if (macro_path_.empty() || save_as) {
        if (records_.empty()) {
            err = "nothing to save";
            return false;
        }
        std::string dir = macro_path_.empty() ? game_data_dir() : dir_of(macro_path_);
        if (dir.empty()) {
            dir = ".";
        }
        const std::string stem =
            macro_path_.empty() ? std::string("newdemo")
                                : fs::path(macro_path_).stem().string();
        save_name_ = stem + ".edited.mac";
        save_name_editing_ = true;
        overlay_ = Overlay::SaveAs;
        SDL_StartTextInput();
        (void)dir;
        return false;
    }
    std::string diag;
    if (!macro_save(macro_path_, records_, diag)) {
        err = diag;
        say("SAVE FAILED: " + diag, demo_ui::kBad, 6.0);
        return false;
    }
    dirty_ = false;
    quit_armed_ = false;
    char buf[160];
    std::snprintf(buf, sizeof(buf), "SAVED %d RECORDS TO %s",
                  static_cast<int>(records_.size()),
                  file_name_of(macro_path_).c_str());
    say(buf, demo_ui::kGood, 4.0);
    return true;
}

// A freshly created recording: one valid tick-0 record holding neutral input.
// The format has no header, so an empty file is not usable; this gives the
// editor an anchor record that later INS/edit operations extend.
MacroRecord Studio::blank_record() {
    MacroRecord r;
    r.time = 0;
    r.mask = kMacroMaskKnown;
    r.h = 0;
    r.v = 0;
    r.btn1 = false;
    r.btn2 = false;
    r.key = 0;
    return r;
}

void Studio::begin_new_macro() {
    cancel_edit();
    const std::string stem =
        macro_path_.empty() ? std::string("newdemo")
                            : fs::path(macro_path_).stem().string() + "_new";
    save_name_ = stem + ".mac";
    save_name_editing_ = true;
    overlay_ = Overlay::NewMac;
    SDL_StartTextInput();
}

bool Studio::create_new_macro(std::string& err) {
    if (save_name_.empty()) {
        err = "enter a file name";
        return false;
    }
    std::string name = save_name_;
    if (fs::path(name).extension().empty()) {
        name += ".mac";
    }
    std::string dir =
        macro_path_.empty() ? game_data_dir() : dir_of(macro_path_);
    if (dir.empty()) {
        dir = ".";
    }
    fs::path target(name);
    if (!target.is_absolute() && !target.has_parent_path()) {
        target = fs::path(dir) / target;
    }
    const std::string path = target.string();
    std::error_code ec;
    if (fs::exists(path, ec)) {
        err = file_name_of(path) + " already exists";
        return false;
    }
    std::vector<MacroRecord> fresh{blank_record()};
    std::string diag;
    if (!macro_save(path, fresh, diag)) {
        err = diag;
        return false;
    }
    macro_path_ = path;
    records_ = std::move(fresh);
    undo_.clear();
    selected_ = 0;
    field_ = static_cast<int>(Field::Tick);
    scroll_ = 0;
    dirty_ = false;
    run_dirty_ = false;
    editing_ = false;
    edit_buf_.clear();
    pending_open_.clear();
    quit_armed_ = false;
    save_name_editing_ = false;
    overlay_ = Overlay::None;
    if (!headless_) {
        SDL_StopTextInput();
    }
    refresh_file_list();
    if (window_) {
        const std::string title =
            "Jill-EX Demo Studio - " + file_name_of(macro_path_);
        SDL_SetWindowTitle(window_, title.c_str());
    }
    // Show the empty board at tick 0, paused, ready for INS.
    playing_ = false;
    std::string run_err;
    if (!start_run(0, run_err)) {
        say(run_err, demo_ui::kBad, 6.0);
    }
    say("CREATED " + file_name_of(path) +
            "  -  INS ADDS RECORDS AT THE PLAYHEAD",
        demo_ui::kGood, 6.0);
    return true;
}

void Studio::refresh_file_list() {    file_list_.clear();
    auto add_dir = [this](const fs::path& dir) {
        std::error_code ec;
        if (!fs::is_directory(dir, ec)) {
            return;
        }
        for (const auto& entry : fs::directory_iterator(dir, ec)) {
            if (ec) {
                break;
            }
            std::error_code fec;
            if (!entry.is_regular_file(fec)) {
                continue;
            }
            const std::string ext = lower(entry.path().extension().string());
            if (ext != ".mac") {
                continue;
            }
            file_list_.push_back(entry.path().string());
        }
    };
    add_dir(game_data_dir());
    add_dir(fs::current_path());
    if (!macro_path_.empty()) {
        add_dir(dir_of(macro_path_));
    }
    std::sort(file_list_.begin(), file_list_.end());
    file_list_.erase(std::unique(file_list_.begin(), file_list_.end()),
                     file_list_.end());
    overlay_sel_ = clamp_int(overlay_sel_, 0,
                             std::max(0, static_cast<int>(file_list_.size()) - 1));
}

// --------------------------------------------------------------------------
// status
// --------------------------------------------------------------------------

void Studio::say(const std::string& text, uint32_t rgb, double seconds) {
    status_ = text;
    status_rgb_ = rgb;
    status_until_ = seconds_now() + seconds;
}

void Studio::toggle_help() {
    overlay_ = overlay_ == Overlay::Help ? Overlay::None : Overlay::Help;
}

// --------------------------------------------------------------------------
// options / settings
// --------------------------------------------------------------------------

std::string Studio::settings_path() const {
    return jill_paths::resolve("demo_studio.ini");
}

void Studio::load_settings() {
    IniFile ini;
    std::string err;
    if (!ini.load(settings_path(), err)) {
        return;
    }
    text_scale_ = clamp_int(ini.get_int("ui", "text_scale", text_scale_), 1, 3);
    game_scale_ = clamp_int(ini.get_int("ui", "game_scale", game_scale_), 0, 3);
}

void Studio::save_settings() const {
    IniFile ini;
    ini.set_int("ui", "text_scale", text_scale_);
    ini.set_int("ui", "game_scale", game_scale_);
    std::string err;
    ini.save(settings_path());
}

void Studio::ensure_window_fits() {
    if (!window_) {
        return;
    }
    const int gs = game_scale_ >= 1 ? game_scale_ : 2;
    const int unit = 6 * text_scale_;
    const int panel_need = 40 * unit + 2 * 12 + 24;
    const int need = kPad + Framebuffer::W * gs + kPad + panel_need + kPad;
    int w = 0;
    int h = 0;
    SDL_GetWindowSize(window_, &w, &h);
    if (w < need) {
        SDL_SetWindowSize(window_, need, h);
    }
    SDL_SetWindowMinimumSize(window_, std::min(need, 960), 600);
}

void Studio::set_text_scale(int scale) {
    if (scale < 1) {
        scale = 3;
    } else if (scale > 3) {
        scale = 1;
    }
    text_scale_ = scale;
    save_settings();
    ensure_window_fits();
    ensure_visible();
    say("UI TEXT " + std::to_string(text_scale_) + "X", demo_ui::kAccent, 2.0);
}

void Studio::set_game_scale(int scale) {
    if (scale < 0) {
        scale = 3;
    } else if (scale > 3) {
        scale = 0;
    }
    game_scale_ = scale;
    save_settings();
    ensure_window_fits();
    say(game_scale_ == 0 ? std::string("GAME VIEW AUTO")
                         : "GAME VIEW " + std::to_string(game_scale_) + "X",
        demo_ui::kAccent, 2.0);
}

// --------------------------------------------------------------------------
// layout / drawing
// --------------------------------------------------------------------------

Studio::Layout Studio::layout() const {
    Layout l;
    if (!renderer_) {
        // Headless has no window; keep the layout harmless.
        l.w = opt_.window_w;
        l.h = opt_.window_h;
        l.game_scale = 2;
        return l;
    }
    int w = 0;
    int h = 0;
    SDL_GetRendererOutputSize(renderer_, &w, &h);
    l.w = w;
    l.h = h;
    const int ts = body_scale();
    const int hs = hint_scale();
    const int unit = 6 * ts;
    const int header_h = std::max(kHeaderH, 12 + 8 * ts + 2 + 8 * hs + 6);
    const int footer_h =
        std::max(34, (hs > 1 ? 4 : 2) * (8 * hs + 3) + 12);
    l.header = {0, 0, w, header_h};
    l.footer = {0, h - footer_h, w, footer_h};
    const int content_y = header_h + 10;
    const int content_h = h - content_y - footer_h - 10;

    // Forced game scale from OPTIONS, otherwise 2x when the panel still fits.
    const int panel_min = 40 * unit + 2 * 12 + 24;
    int gs = game_scale_;
    if (gs < 1) {
        gs = 2;
        while (gs > 1 && w - 320 * gs - panel_min - 3 * kPad < 0) {
            --gs;
        }
    }
    l.game_scale = gs;
    const int gw = Framebuffer::W * gs;
    const int gh = Framebuffer::H * gs;
    l.game = {kPad, content_y, gw, gh};
    l.info = {kPad, l.game.y + gh + 10, gw,
              std::max(8 * hs + 20, content_h - gh - 10)};
    const int px = kPad + gw + kPad;
    l.panel = {px, content_y, w - px - kPad, content_h};
    if (l.panel.w < 400) {
        l.panel.w = 400;
    }
    l.panel_pad = 12;
    const int ix = l.panel.x + l.panel_pad;
    const int iw = l.panel.w - 2 * l.panel_pad;
    const int track_y = l.panel.y + 10 + 8 * ts + 8 + 2 * (8 * hs + 3) + 4;
    l.track = {ix, track_y, iw, 26 + 8 * ts};
    l.rows = {ix, l.track.y + l.track.h + 8 * hs + 20, iw, 0};
    const int editor_h = 8 * ts + 8 + 7 * (8 * hs + 3) + 6;
    l.editor = {ix, l.panel.y + l.panel.h - editor_h - 8, iw, editor_h};
    l.rows.h = std::max(8 * ts + 4, l.editor.y - 10 - l.rows.y);
    l.row_h = 8 * ts + 6;
    l.visible_rows = std::max(1, l.rows.h / l.row_h);

    // Columns sized in character cells so every text scale stays aligned.
    static const int widths[9] = {4, 6, 5, 3, 3, 4, 4, 5, 6};
    int cursor = 4;
    for (int i = 0; i < 9; ++i) {
        l.col[i] = l.rows.x + cursor;
        cursor += widths[i] * unit;
    }
    return l;
}

void Studio::draw() {
    const Layout l = layout();
    SDL_SetRenderDrawColor(renderer_, 0x14, 0x17, 0x1E, 0xFF);
    SDL_RenderClear(renderer_);
    draw_header(l);
    draw_game(l);
    draw_panel(l);
    draw_footer(l);
    // Status line sits under the header title.
    if (!status_.empty() && now_ < status_until_) {
        ui_->text(l.header.x + kPad,
                  l.header.y + 10 + 8 * body_scale() + 2, status_, status_rgb_,
                  hint_scale());
    }
    switch (overlay_) {
        case Overlay::Setup:
            draw_setup(l);
            break;
        case Overlay::Files:
            draw_files(l);
            break;
        case Overlay::Help:
            draw_help(l);
            break;
        case Overlay::SaveAs:
            draw_save_as(l);
            break;
        case Overlay::Options:
            draw_options(l);
            break;
        case Overlay::NewMac:
            draw_new_macro(l);
            break;
        default:
            break;
    }
}

void Studio::draw_header(const Layout& l) {
    ui_->fill(l.header, demo_ui::kPanel);
    ui_->hline(0, l.header.y + l.header.h - 1, l.w, demo_ui::kBorder);
    const int ts = body_scale();
    const int hs = hint_scale();
    ui_->text_shadow(kPad, l.header.y + 10, "JILL-EX DEMO STUDIO",
                     demo_ui::kText, ts);
    ui_->text(kPad + 20 * 6 * ts + 12, l.header.y + 12,
              "EXTERNAL .MAC VIEWER / EDITOR", demo_ui::kTextDim, hs);

    std::string right;
    if (macro_path_.empty()) {
        right = "NO MACRO - F3 OPEN";
    } else {
        right = file_name_of(macro_path_);
        if (dirty_) {
            right += " *";
        }
    }
    const int rw = ui_->text_w(right, hs);
    ui_->text(l.w - kPad - rw, l.header.y + 10, right,
              dirty_ ? demo_ui::kWarn : demo_ui::kText, hs);
    char buf[128];
    std::snprintf(buf, sizeof(buf), "GAME %s   BOARD %s   LEVEL %d",
                  games_[static_cast<size_t>(game_index_)].id.c_str(),
                  board_.c_str(), level_);
    const int gw = ui_->text_w(buf, hs);
    ui_->text(l.w - kPad - gw, l.header.y + 10 + 8 * hs + 3, buf,
              demo_ui::kTextDim, hs);
}

void Studio::draw_game(const Layout& l) {
    ui_->frame({l.game.x - 2, l.game.y - 2, l.game.w + 4, l.game.h + 4},
               demo_ui::kBorder);
    present_game();
    draw_info(l);
}

void Studio::draw_info(const Layout& l) {
    using namespace demo_ui;
    ui_->fill(l.info, kPanel);
    ui_->frame(l.info, kBorder);
    int y = l.info.y + 8;
    ui_->text(l.info.x + 10, y, "LIVE STATE", kText, body_scale());
    // Playback state on the right.
    const std::string state =
        finished_ ? "ENDED" : (playing_ ? "PLAYING" : "PAUSED");
    const int sw = ui_->text_w(state, hint_scale());
    ui_->text(l.info.x + l.info.w - 10 - sw, y + 5, state,
              finished_ ? kWarn : (playing_ ? kGood : kTextDim),
              hint_scale());
    y += 8 * body_scale() + 8;

    auto line = [&](const std::string& s, uint32_t c = kTextDim) {
        const int hs = hint_scale();
        const int line_h = 8 * hs + 4;
        for (const std::string& part :
             ui_->wrap(s, l.info.w - 20, hs)) {
            if (y + 8 * hs > l.info.y + l.info.h - 2) {
                return;
            }
            ui_->text(l.info.x + 10, y, part, c, hs);
            y += line_h;
        }
    };
    if (engine_) {
        char buf[220];
        const int level = engine_->level_id();
        std::snprintf(buf, sizeof(buf), "BOARD %-10s LEVEL %d   SCORE %u   HEALTH %d",
                      engine_->board_name().c_str(), level,
                      static_cast<unsigned>(engine_->score()),
                      engine_->health());
        line(buf);
        if (engine_->has_player()) {
            const BoardObject& p = engine_->player();
            std::snprintf(buf, sizeof(buf),
                          "PLAYER %d,%d   PREV %d,%d   OBJECTS %d   FRAME %d",
                          p.x, p.y, p.prev_x, p.prev_y,
                          static_cast<int>(engine_->objects().size()),
                          engine_->frame());
        } else {
            std::snprintf(buf, sizeof(buf),
                          "NO PLAYER OBJECT   CAMERA %d,%d   OBJECTS %d   FRAME %d",
                          engine_->cam_x(), engine_->cam_y(),
                          static_cast<int>(engine_->objects().size()),
                          engine_->frame());
        }
        line(buf);
        const double frac =
            (interp_mode_ == 0 && playing_) ? accumulator_ / kTickStep : 1.0;
        if (engine_->has_player()) {
            const BoardObject& p = engine_->player();
            const double dx = p.prev_x + (p.x - p.prev_x) * frac;
            const double dy = p.prev_y + (p.y - p.prev_y) * frac;
            std::snprintf(buf, sizeof(buf),
                          "INTERPOLATED DRAW POSITION  %.1f, %.1f   ALPHA %.2f",
                          dx, dy, frac);
            line(buf, interp_mode_ == 0 ? kGood : kWarn);
        } else {
            std::snprintf(buf, sizeof(buf),
                          "CAMERA %d,%d   ALPHA %.2f", engine_->cam_x(),
                          engine_->cam_y(), frac);
            line(buf, interp_mode_ == 0 ? kGood : kWarn);
        }
        const MacroInputState st = macro_state_at(records_, playhead());
        std::snprintf(buf, sizeof(buf),
                      "INPUT   H %s   V %s   JUMP %d   THROW %d   KEY %s",
                      dir_symbol(st.h, 'L', 'R').c_str(),
                      dir_symbol(st.v, 'U', 'D').c_str(), st.btn1 ? 1 : 0,
                      st.btn2 ? 1 : 0, key_label(st.key).c_str());
        line(buf, kText);
        const int duration = std::max(
            macro_last_tick(records_), engine_->macro_duration());
        std::snprintf(buf, sizeof(buf), "MACRO   TICK %d / %d   %d RECORDS",
                      playhead(), duration,
                      static_cast<int>(records_.size()));
        line(buf);
        std::snprintf(buf, sizeof(buf), "MODE    %s", interpolation_label());
        line(buf, interp_mode_ == 0 ? kGood : kWarn);
    }
    line("F1 HELP   F2 SETUP   F3 OPEN   F5 INTERPOLATION   F6 HUD",
         kTextDim);
    line("SPACE PLAY   R RESTART   LEFT/RIGHT STEP   CLICK TIMELINE TO SEEK",
         kTextDim);
}

void Studio::present_game() {
    if (!renderer_ || !frame_tex_ || !engine_) {
        return;
    }
    const double frac =
        (interp_mode_ == 0 && playing_) ? accumulator_ / kTickStep : 1.0;
    engine_->render(fb_, true, static_cast<float>(frac));
    static std::vector<uint32_t> argb(
        static_cast<size_t>(Framebuffer::W) * Framebuffer::H);
    for (size_t i = 0; i < fb_.px.size(); ++i) {
        argb[i] = 0xFF000000u | rgb_of(palette_, fb_.px[i]);
    }
    SDL_UpdateTexture(frame_tex_, nullptr, argb.data(), Framebuffer::W * 4);
    const Layout l = layout();
    SDL_Rect dst{l.game.x, l.game.y, l.game.w, l.game.h};
    SDL_RenderCopy(renderer_, frame_tex_, nullptr, &dst);
}

void Studio::draw_panel(const Layout& l) {
    ui_->fill(l.panel, demo_ui::kPanel);
    ui_->frame(l.panel, demo_ui::kBorder);

    const int x = l.panel.x + l.panel_pad;
    const int duration =
        std::max(macro_last_tick(records_), engine_ ? engine_->macro_duration() : 0);
    char buf[200];
    std::snprintf(buf, sizeof(buf), "TIMELINE  %d RECORDS  %d TICKS",
                  static_cast<int>(records_.size()), duration);
    const int title_y = l.panel.y + 10;
    ui_->text(x, title_y, buf, demo_ui::kText, body_scale());
    const std::string state =
        finished_ ? "ENDED" : (playing_ ? "PLAYING" : "PAUSED");
    const int sw = ui_->text_w(state, hint_scale());
    ui_->text(l.panel.x + l.panel.w - l.panel_pad - sw,
              title_y + (8 * body_scale() - 8 * hint_scale()) / 2, state,
              finished_ ? demo_ui::kWarn
                        : (playing_ ? demo_ui::kGood : demo_ui::kTextDim),
              hint_scale());
    const int interp_y = title_y + 8 * body_scale() + 4;
    ui_->text(x, interp_y,
              std::string("INTERPOLATION  ") + interpolation_label(),
              interp_mode_ == 0 ? demo_ui::kGood : demo_ui::kWarn,
              hint_scale());
    const int warn_y = interp_y + 8 * hint_scale() + 3;
    if (run_dirty_) {
        ui_->text(x, warn_y,
                  "EDITED - PRESS R TO RELOAD AND REPLAY THE CHANGES",
                  demo_ui::kWarn, hint_scale());
    } else if (!records_.empty() && engine_ &&
               engine_->macro_records().size() != records_.size()) {
        ui_->text(x, warn_y,
                  "THE BOARD STARTED ITS OWN SCRIPT (ENTRANCE MACRO)",
                  demo_ui::kBad, hint_scale());
    }

    draw_track(l);
    draw_rows(l);
    draw_editor(l);
}

void Studio::draw_track(const Layout& l) {
    const int duration =
        std::max(1, std::max(macro_last_tick(records_),
                             engine_ ? engine_->macro_duration() : 0));
    ui_->fill(l.track, demo_ui::kPanelAlt);
    ui_->frame(l.track, demo_ui::kBorder);

    // Tick grid: keep at most ~8 time labels so long recordings stay legible.
    int label_every = 18 * 5;
    while (duration / label_every > 8) {
        label_every *= 2;
    }
    int minor_every = 18;
    while (duration / minor_every > 60) {
        minor_every *= 2;
    }
    for (int t = 0; t <= duration; t += minor_every) {
        const int px = l.track.x + t * (l.track.w - 2) / duration;
        const bool major = (t % label_every) == 0;
        const int grid_h = major ? 8 * hint_scale() + 4 : 5;
        ui_->vline(px, l.track.y + l.track.h - grid_h, grid_h,
                   major ? 0x3F4A5C : 0x2C3442);
        if (major) {
            char lbl[24];
            std::snprintf(lbl, sizeof(lbl), "%ds", t / 18);
            ui_->text(px + 2, l.track.y + 2, lbl, demo_ui::kTextDim,
                      hint_scale());
        }
    }

    // One marker per record, coloured by what it sets.
    for (size_t i = 0; i < records_.size(); ++i) {
        const MacroRecord& r = records_[i];
        const int px = l.track.x + r.time * (l.track.w - 2) / duration;
        uint32_t c = demo_ui::kTextDim;
        if (r.mask & kMacroMaskBtn1) {
            c = r.btn1 ? demo_ui::kWarn : 0x6B5634;
        } else if (r.mask & kMacroMaskBtn2) {
            c = r.btn2 ? demo_ui::kBad : 0x6B3A37;
        } else if (r.mask & (kMacroMaskH | kMacroMaskV)) {
            c = demo_ui::kAccent;
        }
        const int top = l.track.y + 8 * hint_scale() + 6;
        const int bottom = l.track.y + l.track.h - 2;
        const int h = (static_cast<int>(i) == selected_) ? bottom - top
                                                         : (bottom - top) / 2;
        ui_->fill(px, top, 1, std::max(2, h), c);
    }

    // Playhead.
    const int head = clamp_int(playhead(), 0, duration);
    const int px = l.track.x + head * (l.track.w - 2) / duration;
    ui_->fill(px, l.track.y + 1, 1, l.track.h - 2, demo_ui::kGood);
    char lbl[32];
    std::snprintf(lbl, sizeof(lbl), "%d", head);
    const int lw = ui_->text_w(lbl, hint_scale());
    const int label_h = 8 * hint_scale() + 3;
    const int lx = clamp_int(px - lw / 2, l.track.x, l.track.x + l.track.w - lw);
    ui_->fill(lx - 2, l.track.y + l.track.h - label_h, lw + 4, label_h,
              0x000000);
    ui_->text(lx, l.track.y + l.track.h - label_h + 1, lbl, demo_ui::kGood,
              hint_scale());
}

void Studio::draw_rows(const Layout& l) {
    // Column header.
    static const char* heads[9] = {"#",  "TICK", "D",   "H",   "V",
                                   "B1", "B2",   "KEY", "MASK"};
    for (int i = 0; i < 9; ++i) {
        ui_->text(l.col[i], l.rows.y - 8 * hint_scale() - 4, heads[i],
                  demo_ui::kTextDim, hint_scale());
    }
    ui_->hline(l.rows.x, l.rows.y - 2, l.rows.w, demo_ui::kBorder);

    scroll_ = clamp_int(scroll_, 0,
                        std::max(0, static_cast<int>(records_.size()) -
                                        l.visible_rows));
    const int play = playhead();
    for (int row = 0; row < l.visible_rows; ++row) {
        const int index = scroll_ + row;
        if (index >= static_cast<int>(records_.size())) {
            break;
        }
        const MacroRecord& r = records_[static_cast<size_t>(index)];
        const int y = l.rows.y + row * l.row_h;
        const bool sel = index == selected_;
        if (sel) {
            ui_->fill(l.rows.x, y, l.rows.w, l.row_h - 1, demo_ui::kSelection);
        } else if (index % 2) {
            ui_->fill(l.rows.x, y, l.rows.w, l.row_h - 1, 0x212632);
        }
        // Records applied at the playhead get a green edge.
        const int next_time =
            index + 1 < static_cast<int>(records_.size()) ? records_[index + 1].time
                                                          : 0x7FFFFFFF;
        const bool active = r.time <= play && play < next_time;
        if (active) {
            ui_->fill(l.rows.x, y, 3, l.row_h - 1, demo_ui::kGood);
        }

        const int ty = y + (l.row_h - 8 * body_scale()) / 2;
        ui_->text(l.col[0], ty, std::to_string(index), demo_ui::kTextDim,
                  body_scale());
        ui_->text(l.col[1], ty, tick_label(r.time), demo_ui::kText, body_scale());
        const int delta =
            index > 0 ? r.time - records_[static_cast<size_t>(index) - 1].time : 0;
        ui_->text(l.col[2], ty,
                  index > 0 ? "+" + std::to_string(delta) : std::string("-"),
                  demo_ui::kTextDim, body_scale());
        ui_->text(l.col[3], ty, dir_symbol(r.h, 'L', 'R'),
                  (r.mask & kMacroMaskH) ? demo_ui::kText : demo_ui::kTextDim,
                  body_scale());
        ui_->text(l.col[4], ty, dir_symbol(r.v, 'U', 'D'),
                  (r.mask & kMacroMaskV) ? demo_ui::kText : demo_ui::kTextDim,
                  body_scale());
        ui_->text(l.col[5], ty, r.btn1 ? "X" : ".",
                  (r.mask & kMacroMaskBtn1) ? demo_ui::kWarn
                                            : demo_ui::kTextDim,
                  body_scale());
        ui_->text(l.col[6], ty, r.btn2 ? "X" : ".",
                  (r.mask & kMacroMaskBtn2) ? demo_ui::kBad : demo_ui::kTextDim,
                  body_scale());
        ui_->text(l.col[7], ty, key_label(r.key),
                  (r.mask & kMacroMaskKey) ? demo_ui::kText : demo_ui::kTextDim,
                  body_scale());
        if (sel && editing_ && field_ == static_cast<int>(Field::Mask)) {
            ui_->fill(l.col[8] - 2, y + 3, ui_->text_w("HHH11", body_scale()) + 6,
                      l.row_h - 7, 0x000000);
            ui_->text(l.col[8], ty, edit_buf_ + "_", demo_ui::kAccent,
                      body_scale());
        } else {
            ui_->text(l.col[8], ty, mask_letters(r.mask), demo_ui::kTextDim,
                      body_scale());
        }
    }

    if (records_.empty()) {
        ui_->text(l.rows.x, l.rows.y + 8,
                  "NO RECORDS.  F3 OPENS A .MAC FILE, INS ADDS ONE HERE.",
                  demo_ui::kTextDim, hint_scale());
    }

    // Scrollbar.
    if (static_cast<int>(records_.size()) > l.visible_rows) {
        const int sx = l.rows.x + l.rows.w - 4;
        ui_->fill(sx, l.rows.y, 4, l.rows.h, 0x161A22);
        const int total = static_cast<int>(records_.size());
        int thumb_h = std::max(16, l.rows.h * l.visible_rows / total);
        int thumb_y = l.rows.y +
                      (l.rows.h - thumb_h) * scroll_ /
                          std::max(1, total - l.visible_rows);
        ui_->fill(sx, thumb_y, 4, thumb_h, demo_ui::kBorder);
    }
}

void Studio::draw_editor(const Layout& l) {
    ui_->hline(l.editor.x, l.editor.y - 6, l.editor.w, demo_ui::kBorder);
    if (records_.empty()) {
        ui_->text(l.editor.x, l.editor.y + 4,
                  "CTRL+N NEW  |  F3 OPEN  |  F2 SETUP  |  F4 OPTIONS  |  SPACE PLAY",
                  demo_ui::kTextDim, hint_scale());
        return;
    }
    const MacroRecord& r = records_[static_cast<size_t>(selected_)];
    const int ts = body_scale();
    const int hs = hint_scale();
    char buf[220];
    std::snprintf(buf, sizeof(buf), "RECORD %d/%d   TICK %d   FIELD %s",
                  selected_, static_cast<int>(records_.size()), r.time,
                  kFieldNames[clamp_int(field_, 0, 6)]);
    int y = l.editor.y + 2;
    ui_->text(l.editor.x, y, buf, demo_ui::kText, ts);
    y += 8 * ts + 4;

    std::snprintf(buf, sizeof(buf),
                  "MASK %s   H %s   V %s   JUMP %d   THROW %d   KEY %s",
                  mask_letters(r.mask).c_str(),
                  dir_symbol(r.h, 'L', 'R').c_str(),
                  dir_symbol(r.v, 'U', 'D').c_str(), r.btn1 ? 1 : 0,
                  r.btn2 ? 1 : 0, key_label(r.key).c_str());
    ui_->text(l.editor.x, y, ui_->ellipsize(buf, l.editor.w, hs),
              editing_ ? demo_ui::kAccent : demo_ui::kTextDim, hs);
    y += 8 * hs + 3;
    if (editing_) {
        std::snprintf(buf, sizeof(buf), "EDIT %s: %s_",
                      kFieldNames[clamp_int(field_, 0, 6)], edit_buf_.c_str());
        ui_->text(l.editor.x, y, buf, demo_ui::kAccent, hs);
    } else {
        const MacroInputState st = macro_state_at(records_, playhead());
        std::snprintf(buf, sizeof(buf),
                      "PLAYHEAD INPUT  H %s  V %s  JUMP %d  THROW %d  KEY %s",
                      dir_symbol(st.h, 'L', 'R').c_str(),
                      dir_symbol(st.v, 'U', 'D').c_str(), st.btn1 ? 1 : 0,
                      st.btn2 ? 1 : 0, key_label(st.key).c_str());
        ui_->text(l.editor.x, y, ui_->ellipsize(buf, l.editor.w, hs),
                  (engine_ && engine_->macro_active()) ? demo_ui::kGood
                                                       : demo_ui::kTextDim,
                  hs);
    }
    y += 8 * hs + 3;

    auto hint_line = [&](const std::string& s) {
        for (const std::string& part : ui_->wrap(s, l.editor.w, hs)) {
            ui_->text(l.editor.x, y, part, demo_ui::kTextDim, hs);
            y += 8 * hs + 3;
        }
    };
    hint_line("TAB FIELD   TYPE VALUE   +/- STEP   INS ADD   DEL REMOVE   "
              "^Z UNDO   ^S SAVE");
    hint_line("MASK FIELD: 1-5 TOGGLE H V JUMP THROW KEY   "
              "CTRL+SHIFT+S SAVE AS");
    if (!macro_path_.empty()) {
        std::string hint = "FILE ";
        hint += macro_path_;
        ui_->text(l.editor.x, y, ui_->ellipsize(hint, l.editor.w, hs),
                  demo_ui::kTextDim, hs);
    }
}

void Studio::draw_footer(const Layout& l) {
    ui_->fill(l.footer, demo_ui::kPanel);
    ui_->hline(0, l.footer.y, l.w, demo_ui::kBorder);
    const int hs = hint_scale();
    const int line_h = 8 * hs + 3;
    const int max_w = l.w - 2 * kPad;
    const std::string hints =
        "SPACE PLAY/PAUSE|R RESTART|LEFT/RIGHT STEP|SHIFT STEP 18|HOME/END "
        "SEEK|F5 INTERPOLATION|F6 HUD|F9 SCREENSHOT|F2 SETUP|F3 OPEN .MAC|F4 "
        "OPTIONS|CTRL+N NEW .MAC|INS ADD|DEL REMOVE|MASK FIELD 1-5 TOGGLE|"
        "CTRL+S SAVE|CTRL+SHIFT+S SAVE AS|F1 HELP|F10 QUIT";
    std::string line;
    int y = l.footer.y + 6;
    const int limit = l.footer.y + l.footer.h - 4;
    size_t start = 0;
    while (start <= hints.size()) {
        const size_t bar = hints.find('|', start);
        const std::string token =
            hints.substr(start, bar == std::string::npos ? std::string::npos
                                                         : bar - start);
        const std::string candidate =
            line.empty() ? token : line + "   " + token;
        if (!line.empty() && ui_->text_w(candidate, hs) > max_w) {
            ui_->text(kPad, y, line, demo_ui::kTextDim, hs);
            y += line_h;
            line = token;
            if (y + line_h > limit) {
                break;
            }
        } else {
            line = candidate;
        }
        if (bar == std::string::npos) {
            break;
        }
        start = bar + 1;
    }
    if (!line.empty() && y + line_h <= limit) {
        ui_->text(kPad, y, line, demo_ui::kTextDim, hs);
    }
}

void Studio::draw_modal_frame(const Layout& l, int x, int y, int w, int h,
                              const std::string& title) {
    const int ts = body_scale();
    const int bar_h = 8 * ts + 10;
    ui_->fill(x + 3, y + 3, w, h, 0x000000);
    ui_->fill(x, y, w, h, demo_ui::kPanel);
    ui_->frame({x, y, w, h}, demo_ui::kAccent);
    ui_->fill(x + 1, y + 1, w - 2, bar_h, demo_ui::kAccentDim);
    ui_->text(x + 8, y + 5, title, 0xFFFFFF, ts);
    (void)l;
}

void Studio::draw_setup(const Layout& l) {
    const int ts = body_scale();
    const int hs = hint_scale();
    const int w = std::min(680 + std::max(0, ts - 2) * 160, l.w - 80);
    const int row_h = 8 * ts + 12;
    const int top = 8 * ts + 20;
    const int hint_h = 8 * hs + 4;
    const char* raw_hints[] = {
        "UP/DOWN SELECT   LEFT/RIGHT CHANGE   ENTER APPLY+REPLAY",
        "MACRO ROW: ENTER OPENS THE FILE LIST   ESC CLOSE",
        "JILL 1 DEMO: intro / level 100.  JILL 2: 3.jn2 / 9.jn2 / 17.jn2.",
        "JILL 3: 1.jn3 / 5.jn3 / 12.jn3.  Filled in automatically for shipped demos.",
        "NOTE: THE MACRO MUST MATCH THE BOARD IT WAS RECORDED ON.",
    };
    std::vector<std::string> hint_lines;
    std::vector<bool> hint_warn;
    for (const char* raw : raw_hints) {
        const bool warn = std::string(raw).rfind("NOTE:", 0) == 0;
        for (const std::string& line : ui_->wrap(raw, w - 36, hs)) {
            hint_lines.push_back(line);
            hint_warn.push_back(warn);
        }
    }
    const int h = top + 4 * row_h +
                  static_cast<int>(hint_lines.size()) * hint_h + 14;
    const int x = (l.w - w) / 2;
    const int y = (l.h - h) / 2;
    draw_modal_frame(l, x, y, w, h, "SETUP - START STATE FOR THE RECORDING");
    const GameDef& g = games_[static_cast<size_t>(game_index_)];
    const int value_x = x + 18 + 10 * 6 * ts;
    auto row = [&](int i, const std::string& label, const std::string& value,
                   bool selected) {
        const int ry = y + top + i * row_h;
        if (selected) {
            ui_->fill(x + 10, ry - 4, w - 20, row_h - 2, demo_ui::kSelection);
        }
        ui_->text(x + 18, ry, label, demo_ui::kTextDim, ts);
        ui_->text(value_x, ry,
                  ui_->ellipsize(value, x + w - 18 - value_x, ts),
                  selected ? demo_ui::kAccent : demo_ui::kText, ts);
    };
    char lvl[32];
    std::snprintf(lvl, sizeof(lvl), "%d%s", level_,
                  level_ < 0 ? "  (board's own spawn)" : "");
    row(0, "GAME", g.id + "  -  " + g.title, overlay_sel_ == 0);
    row(1, "BOARD", board_ + "   (" + std::to_string(board_choices_.size()) +
                      " available)",
        overlay_sel_ == 1);
    row(2, "LEVEL", lvl, overlay_sel_ == 2);
    row(3, "MACRO", macro_path_.empty() ? "none" : file_name_of(macro_path_),
        overlay_sel_ == 3);
    int hy = y + top + 4 * row_h + 8;
    for (size_t i = 0; i < hint_lines.size(); ++i) {
        ui_->text(x + 18, hy, hint_lines[i],
                  hint_warn[i] ? demo_ui::kWarn : demo_ui::kTextDim, hs);
        hy += hint_h;
    }
    (void)l;
}

Studio::FileGeometry Studio::file_geometry(const Layout& l) const {
    FileGeometry g;
    const int hs = hint_scale();
    g.w = std::min(760, l.w - 80);
    g.h = std::min(540, l.h - 80);
    g.x = (l.w - g.w) / 2;
    g.y = (l.h - g.h) / 2;
    g.title_h = 8 * body_scale() + 10;
    g.row_h = 8 * hs + 6;
    g.list_y = g.y + g.title_h + 6;
    const int footer_h = (hs > 1 ? 3 : 2) * (8 * hs + 4) + 8;
    g.visible =
        std::max(1, (g.h - g.title_h - footer_h - 10) / g.row_h);
    return g;
}

void Studio::draw_files(const Layout& l) {
    const FileGeometry g = file_geometry(l);
    const int hs = hint_scale();
    draw_modal_frame(l, g.x, g.y, g.w, g.h, "OPEN .MAC FILE");
    if (file_list_.empty()) {
        ui_->text(g.x + 18, g.list_y,
                  "NO .MAC FILES FOUND IN THE GAME FOLDER OR",
                  demo_ui::kTextDim, hs);
        ui_->text(g.x + 18, g.list_y + 8 * hs + 4, "THE CURRENT DIRECTORY.",
                  demo_ui::kTextDim, hs);
    }
    int first = 0;
    if (overlay_sel_ >= g.visible) {
        first = overlay_sel_ - g.visible + 1;
    }
    for (int i = 0; i < g.visible; ++i) {
        const int index = first + i;
        if (index >= static_cast<int>(file_list_.size())) {
            break;
        }
        const int ry = g.list_y + i * g.row_h;
        const bool sel = index == overlay_sel_;
        if (sel) {
            ui_->fill(g.x + 10, ry - 2, g.w - 20, g.row_h - 1,
                      demo_ui::kSelection);
        }
        const std::string path = file_list_[static_cast<size_t>(index)];
        const std::string name = file_name_of(path);
        ui_->text(g.x + 18, ry, name,
                  sel ? demo_ui::kAccent : demo_ui::kText, hs);
        const int name_w = ui_->text_w(name, hs);
        ui_->text(g.x + 30 + name_w, ry,
                  ui_->ellipsize(dir_of(path), g.w - 40 - name_w, hs),
                  demo_ui::kTextDim, hs);
    }
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%d FILE(S)   FOUND IN THE GAME FOLDER, "
                                    "CURRENT DIR AND THE OPEN FILE'S FOLDER",
                  static_cast<int>(file_list_.size()));
    std::vector<std::string> footer_lines;
    for (const std::string& line : ui_->wrap(buf, g.w - 36, hs)) {
        footer_lines.push_back(line);
    }
    for (const std::string& line :
         ui_->wrap("UP/DOWN SELECT   ENTER OPEN   ESC CANCEL   F3 RESCAN   N NEW",
                   g.w - 36, hs)) {
        footer_lines.push_back(line);
    }
    const int line_h = 8 * hs + 4;
    int fy = g.y + g.h - 8 -
             static_cast<int>(footer_lines.size()) * line_h;
    for (const std::string& line : footer_lines) {
        ui_->text(g.x + 18, fy, line, demo_ui::kTextDim, hs);
        fy += line_h;
    }
}

void Studio::draw_help(const Layout& l) {
    const int ts = body_scale();
    // The help page stays at the native 6x8 cell: it is a dense reference and
    // must fit on screen at every UI scale.
    const int hs = 1;
    const char* lines[] = {
        "PLAYBACK",
        "  SPACE            play / pause the recording",
        "  R                reload the board and replay from tick 0",
        "  LEFT / RIGHT     step one tick (rebuilds when stepping back)",
        "  SHIFT + LEFT/RIGHT  step 18 ticks (one second)",
        "  HOME / END       seek to the first / last tick",
        "  click the timeline bar   seek to that tick",
        "  F5               interpolation: smooth 60 Hz -> off -> original 18.2 Hz",
        "  F6               toggle the classic HUD so its viewport matches the game",
        "",
        "TIMELINE EDITING (all changes stay in memory until CTRL+S)",
        "  UP / DOWN        select a record",
        "  TAB / SHIFT+TAB  next / previous field (tick, H, V, jump, throw, key)",
        "  TYPE or +/-      edit the selected field; ENTER commits, ESC cancels",
        "  with MASK field: 1..5 toggle the H V jump throw key bits",
        "  INS              insert a record at the playhead with its current input",
        "  DEL              delete the selected record",
        "  CTRL+Z           undo (64 steps)",
        "  CTRL+S           save; with no file open, opens SAVE AS",
        "  CTRL+SHIFT+S     save as a new .mac next to the current file",
        "",
        "FILES AND VIEW",
        "  CTRL+N           new .mac from scratch (one neutral tick-0 record)",
        "  F2               setup: episode, board, level and macro",
        "  F3               open a .mac (game folder, current dir)",
        "  F4               options: UI text scale, game view, interpolation",
        "  F9               write a PPM screenshot into the shots folder",
        "  F10              quit (asks again when the file has unsaved edits)",
        "",
        "The world tick is always 18.2065 Hz.  F5 only changes how the 60 Hz",
        "presentation smooths between ticks, so gameplay timing is identical",
        "and only the interpolation effect changes.",
    };
    const int count = static_cast<int>(sizeof(lines) / sizeof(lines[0]));
    const int line_h = 8 * hs + 4;
    const int w = std::min(780, l.w - 60);
    const int h = std::min(l.h - 60, 8 * ts + 26 + count * line_h + 14);
    const int x = (l.w - w) / 2;
    const int y = (l.h - h) / 2;
    draw_modal_frame(l, x, y, w, h, "DEMO STUDIO HELP");
    int ty = y + 8 * ts + 20;
    for (const char* line : lines) {
        const uint32_t c = (line[0] != ' ' && line[0] != '\0')
                               ? demo_ui::kAccent
                               : demo_ui::kTextDim;
        ui_->text(x + 16, ty, line, c, hs);
        ty += line_h;
    }
}

void Studio::draw_save_as(const Layout& l) {
    const int ts = body_scale();
    const int hs = hint_scale();
    const int w = std::min(640, l.w - 80);
    const int h = 8 * ts + 18 + 3 * (8 * hs + 6) + 20;
    const int x = (l.w - w) / 2;
    const int y = (l.h - h) / 2;
    draw_modal_frame(l, x, y, w, h, "SAVE .MAC AS");
    int ty = y + 8 * ts + 16;
    ui_->text(x + 18, ty, "FILE NAME (WRITTEN NEXT TO THE OPEN FILE):",
              demo_ui::kTextDim, hs);
    ty += 8 * hs + 6;
    const int field_h = 8 * hs + 10;
    ui_->fill(x + 16, ty, w - 32, field_h, 0x0F1218);
    ui_->frame({x + 16, ty, w - 32, field_h}, demo_ui::kAccent);
    ui_->text(x + 22, ty + 4,
              ui_->ellipsize(save_name_ + "_", w - 44, hs), demo_ui::kText,
              hs);
    ty += field_h + 12;
    ui_->text(x + 18, ty, "ENTER SAVE   ESC CANCEL   BACKSPACE DELETE",
              demo_ui::kTextDim, hs);
}

void Studio::draw_new_macro(const Layout& l) {
    const int ts = body_scale();
    const int hs = hint_scale();
    const int w = std::min(700, l.w - 80);
    const int h = 8 * ts + 18 + 7 * (8 * hs) + 52;
    const int x = (l.w - w) / 2;
    const int y = (l.h - h) / 2;
    draw_modal_frame(l, x, y, w, h, "NEW .MAC FILE");
    int ty = y + 8 * ts + 16;
    ui_->text(x + 18, ty, "FILE NAME (CREATED IN THE FOLDER BELOW):",
              demo_ui::kTextDim, hs);
    ty += 8 * hs + 6;
    const int field_h = 8 * hs + 10;
    ui_->fill(x + 16, ty, w - 32, field_h, 0x0F1218);
    ui_->frame({x + 16, ty, w - 32, field_h}, demo_ui::kAccent);
    ui_->text(x + 22, ty + 4,
              ui_->ellipsize(save_name_ + "_", w - 44, hs), demo_ui::kText,
              hs);
    ty += field_h + 8;
    std::string dir = macro_path_.empty() ? game_data_dir() : dir_of(macro_path_);
    if (dir.empty()) {
        dir = ".";
    }
    ui_->text(x + 18, ty, ui_->ellipsize("FOLDER " + dir, w - 36, hs),
              demo_ui::kTextDim, hs);
    ty += 8 * hs + 6;
    ui_->text(x + 18, ty,
              "A NEW FILE STARTS WITH ONE NEUTRAL TICK-0 RECORD.",
              demo_ui::kTextDim, hs);
    ty += 8 * hs + 6;
    ui_->text(x + 18, ty,
              "USE INS IN THE TIMELINE TO ADD RECORDS AT THE PLAYHEAD.",
              demo_ui::kTextDim, hs);
    ty += 8 * hs + 10;
    ui_->text(x + 18, ty, "ENTER CREATE   ESC CANCEL   BACKSPACE DELETE",
              demo_ui::kTextDim, hs);
}


void Studio::draw_options(const Layout& l) {
    const int ts = body_scale();
    const int hs = hint_scale();
    const int w = std::min(700, l.w - 80);
    const int row_h = 8 * ts + 12;
    const int top = 8 * ts + 20;
    const int hint_h = 8 * hs + 4;
    const char* hints[] = {
        "UP/DOWN SELECT   LEFT/RIGHT CHANGE   ESC or F4 CLOSE",
        "UI TEXT SCALE RESIZES THE WINDOW WHEN THE TIMELINE NEEDS MORE ROOM.",
        "F5 ALSO CYCLES INTERPOLATION; F6 TOGGLES THE CLASSIC HUD.",
    };
    std::vector<std::string> hint_lines;
    for (const char* h : hints) {
        for (const std::string& line : ui_->wrap(h, w - 36, hs)) {
            hint_lines.push_back(line);
        }
    }
    const int h = top + 4 * row_h +
                  static_cast<int>(hint_lines.size()) * hint_h + 18;
    const int x = (l.w - w) / 2;
    const int y = (l.h - h) / 2;
    draw_modal_frame(l, x, y, w, h, "OPTIONS");
    const int value_x = x + 18 + 14 * 6 * ts;
    auto row = [&](int i, const std::string& label, const std::string& value,
                   bool selected) {
        const int ry = y + top + i * row_h;
        if (selected) {
            ui_->fill(x + 10, ry - 4, w - 20, row_h - 2, demo_ui::kSelection);
        }
        ui_->text(x + 18, ry, label, demo_ui::kTextDim, ts);
        ui_->text(value_x, ry,
                  ui_->ellipsize(value, x + w - 18 - value_x, ts),
                  selected ? demo_ui::kAccent : demo_ui::kText, ts);
    };
    const char* interp_short = interp_mode_ == 0
                                   ? "SMOOTH 60HZ"
                                   : (interp_mode_ == 1 ? "HELD 60HZ"
                                                        : "ORIGINAL 18.2HZ");
    row(0, "TEXT SCALE",
        std::to_string(text_scale_) + "X" +
            (text_scale_ == 1 ? "  (COMPACT)"
                              : (text_scale_ == 2 ? "  (DEFAULT)" : "  (LARGE)")),
        options_sel_ == 0);
    row(1, "GAME VIEW",
        game_scale_ == 0 ? std::string("AUTO") :
                           (std::to_string(game_scale_) + "X"),
        options_sel_ == 1);
    row(2, "INTERPOLATION", interp_short, options_sel_ == 2);
    row(3, "CLASSIC HUD",
        (engine_ && engine_->hud_classic()) ? "ON" : "OFF",
        options_sel_ == 3);
    int hy = y + top + 4 * row_h + 8;
    for (const std::string& line : hint_lines) {
        ui_->text(x + 18, hy, line, demo_ui::kTextDim, hs);
        hy += hint_h;
    }
}


void Studio::handle_event(const SDL_Event& ev, const Layout& l) {
    switch (ev.type) {
        case SDL_QUIT:
            if (dirty_) {
                if (!quit_armed_) {
                    quit_armed_ = true;
                    say("UNSAVED CHANGES - CTRL+S TO SAVE, F10 AGAIN TO QUIT",
                        demo_ui::kWarn, 10.0);
                } else {
                    running_ = false;
                }
            } else {
                running_ = false;
            }
            break;
        case SDL_KEYDOWN:
            handle_key(ev.key.keysym.sym,
                       (ev.key.keysym.mod & KMOD_CTRL) != 0,
                       (ev.key.keysym.mod & KMOD_SHIFT) != 0, l);
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (ev.button.button == SDL_BUTTON_LEFT) {
                handle_click(ev.button.x, ev.button.y, l);
            }
            break;
        case SDL_MOUSEWHEEL:
            handle_wheel(ev.wheel.y, l);
            break;
        case SDL_TEXTINPUT:
            if ((overlay_ == Overlay::SaveAs || overlay_ == Overlay::NewMac) &&
                save_name_editing_) {
                save_name_ += ev.text.text;
            }
            break;
        case SDL_WINDOWEVENT:
            if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                ensure_visible();
            }
            break;
        default:
            break;
    }
}

bool Studio::handle_key(int key, bool ctrl, bool shift, const Layout& l) {
    // --- overlays first -----------------------------------------------------
    if (overlay_ == Overlay::NewMac) {
        if (key == SDLK_ESCAPE) {
            overlay_ = Overlay::None;
            save_name_editing_ = false;
            SDL_StopTextInput();
            return true;
        }
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            std::string err;
            if (!create_new_macro(err)) {
                say("NEW: " + err, demo_ui::kBad, 6.0);
            }
            return true;
        }
        if (key == SDLK_BACKSPACE) {
            if (!save_name_.empty()) {
                save_name_.pop_back();
            }
            return true;
        }
        return true;
    }
    if (overlay_ == Overlay::SaveAs) {
        if (key == SDLK_ESCAPE) {
            overlay_ = Overlay::None;
            save_name_editing_ = false;
            SDL_StopTextInput();
            return true;
        }
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            std::string dir =
                macro_path_.empty() ? game_data_dir() : dir_of(macro_path_);
            if (dir.empty()) {
                dir = ".";
            }
            const std::string path = (fs::path(dir) / save_name_).string();
            std::string diag;
            if (!macro_save(path, records_, diag)) {
                say("SAVE FAILED: " + diag, demo_ui::kBad, 6.0);
                return true;
            }
            macro_path_ = path;
            dirty_ = false;
            quit_armed_ = false;
            overlay_ = Overlay::None;
            save_name_editing_ = false;
            SDL_StopTextInput();
            refresh_file_list();
            say("SAVED " + file_name_of(path), demo_ui::kGood, 4.0);
            if (window_) {
                const std::string title =
                    "Jill-EX Demo Studio - " + file_name_of(macro_path_);
                SDL_SetWindowTitle(window_, title.c_str());
            }
            return true;
        }
        if (key == SDLK_BACKSPACE) {
            if (!save_name_.empty()) {
                save_name_.pop_back();
            }
            return true;
        }
        return true;
    }
    if (overlay_ == Overlay::Help) {
        if (key == SDLK_ESCAPE || key == SDLK_F1 || key == SDLK_RETURN) {
            overlay_ = Overlay::None;
            return true;
        }
        return true;
    }
    if (overlay_ == Overlay::Options) {
        if (key == SDLK_ESCAPE || key == SDLK_F4) {
            overlay_ = Overlay::None;
            return true;
        }
        if (key == SDLK_UP) {
            options_sel_ = (options_sel_ + 3) % 4;
            return true;
        }
        if (key == SDLK_DOWN || key == SDLK_TAB) {
            options_sel_ = (options_sel_ + 1) % 4;
            return true;
        }
        if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            const int dir = key == SDLK_RIGHT ? 1 : -1;
            switch (options_sel_) {
                case 0:
                    set_text_scale(text_scale_ + dir);
                    break;
                case 1:
                    set_game_scale(game_scale_ == 0
                                       ? (dir > 0 ? 1 : 3)
                                       : game_scale_ + dir);
                    break;
                case 2:
                    set_interpolation_mode((interp_mode_ + 3 + dir) % 3);
                    break;
                case 3:
                    if (engine_) {
                        engine_->set_hud_classic(!engine_->hud_classic());
                    }
                    break;
                default:
                    break;
            }
            return true;
        }
        return true;
    }
    if (overlay_ == Overlay::Files) {
        const int count = static_cast<int>(file_list_.size());
        if (key == SDLK_ESCAPE) {
            overlay_ = Overlay::None;
            return true;
        }
        if (key == SDLK_F3) {
            refresh_file_list();
            return true;
        }
        if (key == SDLK_n) {
            begin_new_macro();
            return true;
        }
        if (key == SDLK_UP) {
            overlay_sel_ = std::max(0, overlay_sel_ - 1);
            return true;
        }
        if (key == SDLK_DOWN) {
            overlay_sel_ = count > 0 ? std::min(count - 1, overlay_sel_ + 1) : 0;
            return true;
        }
        if (key == SDLK_PAGEUP) {
            overlay_sel_ = std::max(0, overlay_sel_ - 10);
            return true;
        }
        if (key == SDLK_PAGEDOWN) {
            overlay_sel_ = count > 0 ? std::min(count - 1, overlay_sel_ + 10) : 0;
            return true;
        }
        if ((key == SDLK_RETURN || key == SDLK_KP_ENTER) && count > 0) {            const std::string path = file_list_[static_cast<size_t>(overlay_sel_)];
            if (dirty_ && pending_open_ != path) {
                pending_open_ = path;
                say("UNSAVED CHANGES - CTRL+S TO SAVE, ENTER AGAIN TO DISCARD",
                    demo_ui::kWarn, 8.0);
                return true;
            }
            pending_open_.clear();
            std::string err;
            open_macro(path, true, err);
            overlay_ = Overlay::None;
            std::string run_err;
            if (!start_run(0, run_err)) {
                say(run_err, demo_ui::kBad, 6.0);
            } else {
                playing_ = !records_.empty();
            }
            return true;
        }
        return true;
    }
    if (overlay_ == Overlay::Setup) {
        const GameDef& g = games_[static_cast<size_t>(game_index_)];
        const int board_count = static_cast<int>(board_choices_.size());
        if (key == SDLK_ESCAPE) {
            overlay_ = Overlay::None;
            return true;
        }
        if (key == SDLK_UP) {
            overlay_sel_ = (overlay_sel_ + 3) % 4;
            return true;
        }
        if (key == SDLK_DOWN || key == SDLK_TAB) {
            overlay_sel_ = (overlay_sel_ + 1) % 4;
            return true;
        }
        if (key == SDLK_LEFT || key == SDLK_RIGHT) {
            const int dir = key == SDLK_RIGHT ? 1 : -1;
            if (overlay_sel_ == 0) {
                const int n = static_cast<int>(games_.size());
                const int next = (game_index_ + dir + n) % n;
                std::string err;
                if (!select_game(next, err)) {
                    say(err, demo_ui::kBad, 5.0);
                } else {
                    // Keep the preview valid: a fresh engine has no board yet.
                    std::string run_err;
                    if (!start_run(0, run_err)) {
                        say(run_err, demo_ui::kBad, 5.0);
                    } else {
                        playing_ = false;
                    }
                }
            } else if (overlay_sel_ == 1 && board_count > 0) {
                int idx = 0;
                for (int i = 0; i < board_count; ++i) {
                    if (board_choices_[static_cast<size_t>(i)] == board_) {
                        idx = i;
                        break;
                    }
                }
                idx = (idx + dir + board_count) % board_count;
                board_ = board_choices_[static_cast<size_t>(idx)];
            } else if (overlay_sel_ == 2) {
                level_ = clamp_int(level_ + dir, -1, 999);
            }
            return true;
        }
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            if (overlay_sel_ == 3) {
                refresh_file_list();
                overlay_sel_ = 0;
                overlay_ = Overlay::Files;
                return true;
            }
            std::string err;
            if (!start_run(0, err)) {
                say(err, demo_ui::kBad, 6.0);
            } else {
                playing_ = !records_.empty() && !finished_;
                overlay_ = Overlay::None;
                say("STARTED " + board_ + " / LEVEL " + std::to_string(level_),
                    demo_ui::kGood, 3.0);
            }
            return true;
        }
        return true;
    }

    // --- global keys --------------------------------------------------------
    if (key == SDLK_F10) {
        if (dirty_ && !quit_armed_) {
            quit_armed_ = true;
            say("UNSAVED CHANGES - CTRL+S TO SAVE, F10 AGAIN TO QUIT",
                demo_ui::kWarn, 10.0);
            return true;
        }
        running_ = false;
        return true;
    }
    if (key == SDLK_F1) {
        toggle_help();
        return true;
    }
    if (key == SDLK_F2) {
        overlay_ = Overlay::Setup;
        overlay_sel_ = 0;
        return true;
    }
    if (key == SDLK_F4) {
        overlay_ = Overlay::Options;
        options_sel_ = 0;
        return true;
    }
    if (key == SDLK_F3) {
        refresh_file_list();
        overlay_sel_ = 0;
        overlay_ = Overlay::Files;
        return true;
    }
    if (key == SDLK_F5) {
        set_interpolation_mode((interp_mode_ + 1) % 3);
        return true;
    }
    if (key == SDLK_F6) {
        if (engine_) {
            engine_->set_hud_classic(!engine_->hud_classic());
            say(engine_->hud_classic() ? "CLASSIC HUD ON" : "CLASSIC HUD OFF",
                demo_ui::kTextDim, 2.0);
        }
        return true;
    }
    if (key == SDLK_F9) {
        std::string err;
        if (save_screenshot(err)) {
            say("SCREENSHOT: " + err, demo_ui::kGood, 4.0);
        } else {
            say("SCREENSHOT FAILED: " + err, demo_ui::kBad, 5.0);
        }
        return true;
    }
    if (ctrl && (key == SDLK_s)) {
        std::string err;
        save_macro(shift, err);
        if (!err.empty()) {
            say("SAVE FAILED: " + err, demo_ui::kBad, 5.0);
        }
        return true;
    }
    if (ctrl && key == SDLK_z) {
        undo();
        return true;
    }
    if (ctrl && key == SDLK_n) {
        begin_new_macro();
        return true;
    }
    if (key == SDLK_SPACE) {
        toggle_play();
        return true;
    }
    if (key == SDLK_r) {
        seek(0);
        playing_ = !records_.empty();
        return true;
    }
    if (key == SDLK_LEFT) {
        step_ticks(shift ? -18 : -1);
        return true;
    }
    if (key == SDLK_RIGHT) {
        step_ticks(shift ? 18 : 1);
        return true;
    }
    if (key == SDLK_HOME) {
        seek(0);
        return true;
    }
    if (key == SDLK_END) {
        seek(std::max(macro_last_tick(records_),
                      engine_ ? engine_->macro_duration() : 0));
        return true;
    }
    if (key == SDLK_INSERT) {
        insert_record_at_playhead();
        return true;
    }
    if (key == SDLK_DELETE) {
        delete_selected();
        return true;
    }
    if (key == SDLK_UP) {
        select_row(selected_ - 1);
        editing_ = false;
        return true;
    }
    if (key == SDLK_DOWN) {
        select_row(selected_ + 1);
        editing_ = false;
        return true;
    }
    if (key == SDLK_PAGEUP) {
        select_row(selected_ - l.visible_rows);
        editing_ = false;
        return true;
    }
    if (key == SDLK_PAGEDOWN) {
        select_row(selected_ + l.visible_rows);
        editing_ = false;
        return true;
    }
    if (key == SDLK_TAB) {
        if (!records_.empty()) {
            if (editing_) {
                commit_edit();
            }
            const int n = static_cast<int>(Field::Count);
            field_ = (field_ + (shift ? n - 1 : 1)) % n;
        }
        return true;
    }
    if (key >= SDLK_1 && key <= SDLK_5 && !editing_ &&
        field_ == static_cast<int>(Field::Mask)) {
        toggle_mask_bit(key - SDLK_1);
        return true;
    }
    // Digits on any other field start (or extend) an inline value edit.
    if (key >= SDLK_0 && key <= SDLK_9) {
        if (records_.empty()) {
            return true;
        }
        if (!editing_) {
            begin_edit();
            edit_buf_.clear();
        }
        if (edit_buf_.size() < 8) {
            edit_buf_.push_back(static_cast<char>('0' + (key - SDLK_0)));
        }
        return true;
    }
    if (key == SDLK_PLUS || key == SDLK_EQUALS || key == SDLK_KP_PLUS) {
        nudge_field(1);
        return true;
    }
    if (key == SDLK_MINUS || key == SDLK_KP_MINUS) {
        nudge_field(-1);
        return true;
    }
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        if (editing_) {
            commit_edit();
        } else {
            begin_edit();
        }
        return true;
    }
    if (key == SDLK_ESCAPE) {
        if (editing_) {
            cancel_edit();
        } else {
            playing_ = false;
        }
        return true;
    }
    if (key == SDLK_BACKSPACE && editing_) {
        if (!edit_buf_.empty()) {
            edit_buf_.pop_back();
        }
        return true;
    }
    return false;
}

void Studio::handle_click(int x, int y, const Layout& l) {
    if (overlay_ != Overlay::None) {
        if (overlay_ == Overlay::Files) {
            const FileGeometry g = file_geometry(l);
            int first = 0;
            if (overlay_sel_ >= g.visible) {
                first = overlay_sel_ - g.visible + 1;
            }
            for (int i = 0; i < g.visible; ++i) {
                const int index = first + i;
                if (index >= static_cast<int>(file_list_.size())) {
                    break;
                }
                const int ry = g.list_y + i * g.row_h;
                if (x >= g.x + 10 && x <= g.x + g.w - 10 && y >= ry - 2 &&
                    y < ry + g.row_h - 1) {
                    overlay_sel_ = index;
                    return;
                }
            }
        }
        return;
    }

    if (l.track.contains(x, y)) {
        const int duration = std::max(
            1, std::max(macro_last_tick(records_),
                        engine_ ? engine_->macro_duration() : 0));
        const int rel = clamp_int(x - l.track.x, 0, l.track.w - 2);
        seek(rel * duration / std::max(1, l.track.w - 2));
        return;
    }
    if (l.rows.contains(x, y)) {
        const int row = (y - l.rows.y) / l.row_h;
        const int index = scroll_ + row;
        if (index >= 0 && index < static_cast<int>(records_.size())) {
            select_row(index);
            editing_ = false;
            // Pick the field from the clicked column.
            int best = 0;
            for (int i = 0; i < 9; ++i) {
                if (x >= l.col[i]) {
                    best = i;
                }
            }
            static const int field_for_col[9] = {
                static_cast<int>(Field::Tick), static_cast<int>(Field::Tick),
                static_cast<int>(Field::Tick), static_cast<int>(Field::H),
                static_cast<int>(Field::V),    static_cast<int>(Field::Btn1),
                static_cast<int>(Field::Btn2), static_cast<int>(Field::Key),
                static_cast<int>(Field::Mask)};
            field_ = field_for_col[best];
        }
        return;
    }
    if (l.game.contains(x, y)) {
        toggle_play();
    }
}

void Studio::handle_wheel(int dy, const Layout& l) {
    if (dy == 0) {
        return;
    }
    scroll_ = clamp_int(scroll_ - dy * 3, 0,
                        std::max(0, static_cast<int>(records_.size()) -
                                        l.visible_rows));
}

// --------------------------------------------------------------------------
// frame loop / headless
// --------------------------------------------------------------------------

int Studio::run() {
    double last = seconds_now();
    while (running_) {
        now_ = seconds_now();
        bool had_event = false;
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            had_event = true;
            handle_event(ev, layout());
        }
        double dt = now_ - last;
        last = now_;
        if (dt > 0.25) {
            dt = 0.25;
        }
        bool ticked = false;
        if (playing_ && overlay_ == Overlay::None && !editing_ && !finished_) {
            if (interp_mode_ == 2) {
                // Original cadence: one logic tick and one presented frame per
                // 18.2 Hz step.  Nothing is drawn between ticks, so the tool
                // shows exactly what the original speed looks like.
                accumulator_ += dt;
                if (accumulator_ >= kTickStep) {
                    accumulator_ -= kTickStep;
                    tick_once();
                    ticked = true;
                }
            } else {
                accumulator_ += dt;
                int guard = 0;
                while (accumulator_ >= kTickStep && guard++ < 8) {
                    accumulator_ -= kTickStep;
                    tick_once();
                    ticked = true;
                    if (!playing_) {
                        accumulator_ = 0.0;
                        break;
                    }
                }
            }
        } else {
            accumulator_ = 0.0;
        }
        if (interp_mode_ == 2 && playing_ && !ticked && !had_event) {
            SDL_Delay(1);
            continue;
        }
        draw();
        SDL_RenderPresent(renderer_);
    }
    return 0;
}

bool Studio::save_screenshot(std::string& err) {
    if (!engine_) {
        err = "no engine";
        return false;
    }
    std::error_code ec;
    fs::create_directories(opt_.screenshot_dir, ec);
    char name[64];
    std::snprintf(name, sizeof(name), "demo_%03d.ppm", shot_counter_++);
    const std::string path = (fs::path(opt_.screenshot_dir) / name).string();
    const double frac = 1.0;
    engine_->render(fb_, true, static_cast<float>(frac));
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        err = "cannot write " + path;
        return false;
    }
    std::fprintf(f, "P6\n%d %d\n255\n", Framebuffer::W, Framebuffer::H);
    for (uint8_t c : fb_.px) {
        std::fputc(palette_[c * 3 + 0], f);
        std::fputc(palette_[c * 3 + 1], f);
        std::fputc(palette_[c * 3 + 2], f);
    }
    std::fclose(f);
    err = path;
    return true;
}

int Studio::run_selftest(std::string& err) {
    if (macro_path_.empty()) {
        err = "no .mac loaded (use --mac FILE)";
        return 2;
    }
    if (records_.empty()) {
        err = "no records parsed";
        return 2;
    }
    // Exercise the editor's serialiser: change one field, encode, re-parse and
    // compare every record.  Nothing is written to disk.
    // Re-encoding the untouched parse should reproduce the file byte for byte
    // when the recording only uses the five documented mask bits.
    std::vector<uint8_t> clean;
    std::vector<uint8_t> raw;
    {
        std::string clean_diag;
        macro_encode(records_, clean, clean_diag);
        if (FILE* f = std::fopen(macro_path_.c_str(), "rb")) {
            std::fseek(f, 0, SEEK_END);
            const long n = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            if (n > 0) {
                raw.resize(static_cast<size_t>(n));
                const size_t got = std::fread(raw.data(), 1, raw.size(), f);
                raw.resize(got);
            }
            std::fclose(f);
        }
    }
    const bool byte_identical = !raw.empty() && raw == clean;

    std::vector<MacroRecord> edited = records_;
    edited.front().mask |= kMacroMaskH;
    edited.front().h = edited.front().h > 0 ? 0 : 1;
    std::vector<uint8_t> bytes;
    std::string diag;
    if (!macro_encode(edited, bytes, diag)) {
        err = "encode: " + diag;
        return 2;
    }
    std::vector<MacroRecord> back;
    std::string parse_diag;
    if (!macro_parse(bytes.data(), bytes.size(), back, parse_diag) ||
        !parse_diag.empty()) {
        err = "re-parse: " + parse_diag;
        return 2;
    }
    if (back.size() != edited.size()) {
        // macro_encode() inserts a leading tick-0 marker when the first record
        // starts later, so allow exactly that one extra mask-0 record.
        const bool marker = back.size() == edited.size() + 1 &&
                            back.front().time == 0 && back.front().mask == 0;
        if (!marker) {
            err = "record count changed: " + std::to_string(edited.size()) +
                  " -> " + std::to_string(back.size());
            return 2;
        }
    }
    const size_t offset = back.size() - edited.size();
    for (size_t i = 0; i < edited.size(); ++i) {
        if (!same_record(back[i + offset], edited[i])) {
            err = "record " + std::to_string(i) + " differs after round-trip";
            return 2;
        }
    }
    std::printf(
        "selftest: %s  records=%d ticks=%d edited-bytes=%d roundtrip=ok  "
        "reencode=%s\n",
        file_name_of(macro_path_).c_str(), static_cast<int>(records_.size()),
        macro_last_tick(records_), static_cast<int>(bytes.size()),
        byte_identical ? "identical" : "differs");

    // Keyboard routing regression: F2 is the episode setup and F4 is options.
    {
        const Layout l = layout();
        overlay_ = Overlay::None;
        handle_key(SDLK_F2, false, false, l);
        const bool f2_ok = overlay_ == Overlay::Setup;
        overlay_ = Overlay::None;
        handle_key(SDLK_F4, false, false, l);
        const bool f4_ok = overlay_ == Overlay::Options;
        overlay_ = Overlay::None;
        if (!f2_ok || !f4_ok) {
            err = "key routing: F2 must open SETUP and F4 must open OPTIONS";
            return 2;
        }
        std::printf("selftest: F2=setup F4=options\n");

    // A brand-new file seeds one neutral tick-0 record; it must round-trip.
    {
        const std::vector<MacroRecord> fresh{blank_record()};
        std::vector<uint8_t> fbytes;
        std::string fdiag;
        std::vector<MacroRecord> fback;
        std::string fpdiag;
        const bool ok =
            macro_encode(fresh, fbytes, fdiag) &&
            macro_parse(fbytes.data(), fbytes.size(), fback, fpdiag) &&
            fpdiag.empty() && fback.size() == 1 &&
            same_record(fback.front(), fresh.front());
        if (!ok) {
            err = "new .mac seed record does not round-trip";
            return 2;
        }
        std::printf("selftest: new .mac seed=ok (%d bytes)\n",
                    static_cast<int>(fbytes.size()));
    }
    }

    if (!opt_.out.empty()) {
        std::string save_diag;
        if (!macro_save(opt_.out, edited, save_diag)) {
            err = "save: " + save_diag;
            return 2;
        }
        std::vector<MacroRecord> reloaded;
        std::string load_diag;
        if (!macro_load(opt_.out, reloaded, load_diag) || !load_diag.empty()) {
            err = "reload " + opt_.out + ": " + load_diag;
            return 2;
        }
        if (reloaded.size() != back.size()) {
            err = "file round-trip changed the record count";
            return 2;
        }
        for (size_t i = 0; i < reloaded.size(); ++i) {
            if (!same_record(reloaded[i], back[i])) {
                err = "file round-trip changed record " + std::to_string(i);
                return 2;
            }
        }
        std::printf("selftest: wrote and reloaded %s (%d records)\n",
                    opt_.out.c_str(), static_cast<int>(reloaded.size()));
    }
    return 0;
}

int Studio::run_headless(std::string& err) {
    if (!engine_) {
        err = "no engine";
        return 2;
    }
    FILE* trace = nullptr;
    if (!opt_.trace.empty()) {
        trace = std::fopen(opt_.trace.c_str(), "wb");
        if (!trace) {
            err = "cannot write " + opt_.trace;
            return 2;
        }
        std::fprintf(trace,
                     "tick,board,level,px,py,state,dx,dy,p0f,p11,p13,raise,"
                     "kind,health,feathers,score,camx,camy,in_h,in_v,in_b1,"
                     "in_b2\n");
    }
    if (opt_.ticks > 0) {
        for (int i = 0; i < opt_.ticks; ++i) {
            tick_once();
            if (trace && engine_->has_player()) {
                const BoardObject& p = engine_->player();
                const InputState& in = engine_->input();
                std::fprintf(trace,
                             "%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%u,"
                             "%d,%d,%d,%d,%d,%d\n",
                             i, engine_->board_name().c_str(),
                             engine_->level_id(), p.x, p.y, p.state, p.dx, p.dy,
                             p.p0f, p.p11, p.p13, p.raise, p.kind,
                             engine_->health(), engine_->inv_count(9),
                             engine_->score(), engine_->cam_x(), engine_->cam_y(),
                             in.h, in.v, in.btn1 ? 1 : 0, in.btn2 ? 1 : 0);
            }
        }
    }
    if (trace) {
        std::fclose(trace);
    }
    if (!opt_.dump_map.empty()) {
        FILE* mf = std::fopen(opt_.dump_map.c_str(), "wb");
        if (!mf) {
            err = "cannot write " + opt_.dump_map;
            return 2;
        }
        for (int r = 0; r < Board::kRows; ++r) {
            for (int c = 0; c < Board::kCols; ++c) {
                std::fprintf(mf, "%04X%c", engine_->map_tile(c, r),
                             (c + 1 == Board::kCols) ? '\n' : ' ');
            }
        }
        std::fclose(mf);
    }
    if (!opt_.dump_objects.empty()) {
        FILE* of = std::fopen(opt_.dump_objects.c_str(), "wb");
        if (!of) {
            err = "cannot write " + opt_.dump_objects;
            return 2;
        }
        std::fprintf(of,
                     "# idx kind x y dx dy w h state p0f p11 p13 flags ttl\n");
        const auto& objs = engine_->objects();
        for (size_t i = 0; i < objs.size(); ++i) {
            const BoardObject& o = objs[i];
            std::fprintf(of,
                         "%3d %2d %4d %4d %3d %3d %2d %2d %3d %3d %3d %3d "
                         "0x%04X %d\n",
                         static_cast<int>(i), o.kind, o.x, o.y, o.dx, o.dy,
                         o.w, o.h, o.state, o.p0f, o.p11, o.p13,
                         static_cast<unsigned>(o.flags), o.ttl);
        }
        std::fclose(of);
    }
    engine_->render(fb_, true, 1.0f);
    const std::string out =
        opt_.out.empty() ? std::string("demo_studio.ppm") : opt_.out;
    FILE* f = std::fopen(out.c_str(), "wb");
    if (!f) {
        err = "cannot write " + out;
        return 2;
    }
    std::fprintf(f, "P6\n%d %d\n255\n", Framebuffer::W, Framebuffer::H);
    for (uint8_t c : fb_.px) {
        std::fputc(palette_[c * 3 + 0], f);
        std::fputc(palette_[c * 3 + 1], f);
        std::fputc(palette_[c * 3 + 2], f);
    }
    std::fclose(f);
    std::printf(
        "game=%s board=%s level=%d macro=%s records=%d tick=%d/%d out=%s\n",
        games_[static_cast<size_t>(game_index_)].id.c_str(), board_.c_str(),
        level_, macro_path_.empty() ? "(none)" : file_name_of(macro_path_).c_str(),
        static_cast<int>(records_.size()), playhead(),
        std::max(macro_last_tick(records_),
                 engine_ ? engine_->macro_duration() : 0),
        out.c_str());
    return 0;
}
