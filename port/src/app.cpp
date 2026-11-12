#include "app.h"

#include "config.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "audio.h"
#include "engine.h"
#include "extaudio.h"
#include "external.h"
#include "framebuffer.h"
#include "game.h"
#include "profile.h"

#ifdef JILL_HAVE_SDL
#include <SDL.h>
#endif

namespace {

std::string upper_ascii(const char* s);
std::string player_look_name(const std::vector<GameDef>& games, int look);
std::string player_suit_name(int suit);
void apply_player_look(Engine& engine, const std::vector<GameDef>& games,
                       int look, int suit);

// Merge two full-resolution eye views into the 320x200 presentation buffer.
// Mode 1 = row-interlaced (even = left, odd = right); mode 2 = half-width
// side-by-side (left in the first 160 columns, right in the last 160).
void compose_stereo(Framebuffer& out, const Framebuffer& left,
                    const Framebuffer& right, int mode) {
    if (mode == 1) {
        for (int y = 0; y < Framebuffer::H; ++y) {
            const Framebuffer& src = (y & 1) ? right : left;
            std::memcpy(out.px.data() + static_cast<size_t>(y) * Framebuffer::W,
                        src.px.data() + static_cast<size_t>(y) * Framebuffer::W,
                        Framebuffer::W);
        }
    } else if (mode == 2) {
        constexpr int kHalf = Framebuffer::W / 2;
        for (int y = 0; y < Framebuffer::H; ++y) {
            const size_t row = static_cast<size_t>(y) * Framebuffer::W;
            for (int x = 0; x < kHalf; ++x) {
                out.px[row + static_cast<size_t>(x)] =
                    left.px[row + static_cast<size_t>(x * 2)];
                out.px[row + static_cast<size_t>(kHalf + x)] =
                    right.px[row + static_cast<size_t>(x * 2)];
            }
        }
    } else {
        out.px = left.px;
    }
}

bool load_palette(const std::string& path, uint8_t rgb[768]) {
    vga_default_palette(rgb);
    if (path.empty()) {
        return true;
    }
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        std::fprintf(stderr, "cannot open palette %s\n", path.c_str());
        return false;
    }
    size_t got = std::fread(rgb, 1, 768, f);
    std::fclose(f);
    if (got != 768) {
        std::fprintf(stderr, "%s is not a 768-byte palette\n", path.c_str());
        return false;
    }
    return true;
}

// Resolve a music name from a manifest / CHECKPT string.  Absolute paths are
// used as-is; everything else lives in the game's asset directory.
std::string track_path(const GameDef& g, const std::string& name) {
    if (name.size() > 1 &&
        (name[1] == ':' || name[0] == '/' || name[0] == '\\')) {
        return name;
    }
    return g.data_dir + "/" + name;
}

std::shared_ptr<CmfSong> load_track(const GameDef& g, const std::string& name,
                                    std::string& err) {
    auto song = std::make_shared<CmfSong>();
    if (!cmf_load(track_path(g, name), *song, err)) {
        return nullptr;
    }
    return song;
}

// --- display post-process -------------------------------------------------
// These work on the 320x200 ARGB frame just before it is uploaded.  They are
// deliberately cheap: no shader pipeline, so they also work with SDL's
// software renderer.

void filter_smooth(std::vector<uint32_t>& argb, int w, int h) {
    static std::vector<uint32_t> tmp;
    tmp.resize(argb.size());
    // Horizontal pass (1,2,1)/4.
    for (int y = 0; y < h; ++y) {
        const size_t row = static_cast<size_t>(y) * w;
        for (int x = 0; x < w; ++x) {
            const uint32_t l = argb[row + (x > 0 ? x - 1 : x)];
            const uint32_t c = argb[row + x];
            const uint32_t r = argb[row + (x + 1 < w ? x + 1 : x)];
            uint32_t out = 0xFF000000u;
            for (int s = 16; s >= 0; s -= 8) {
                out |= ((l >> s & 0xFF) + 2 * ((c >> s) & 0xFF) +
                        ((r >> s) & 0xFF)) / 4
                       << s;
            }
            tmp[row + x] = out;
        }
    }
    // Vertical pass, blended with the original so details stay readable.
    for (int y = 0; y < h; ++y) {
        const size_t row = static_cast<size_t>(y) * w;
        const size_t up = static_cast<size_t>(y > 0 ? y - 1 : y) * w;
        const size_t dn = static_cast<size_t>(y + 1 < h ? y + 1 : y) * w;
        for (int x = 0; x < w; ++x) {
            const uint32_t c = tmp[row + x];
            const uint32_t u = tmp[up + x];
            const uint32_t d = tmp[dn + x];
            uint32_t mixed = 0xFF000000u;
            for (int s = 16; s >= 0; s -= 8) {
                const uint32_t bv =
                    (((u >> s) & 0xFF) + 2 * ((c >> s) & 0xFF) +
                     ((d >> s) & 0xFF)) / 4;
                const uint32_t ov = (argb[row + x] >> s) & 0xFF;
                mixed |= ((bv * 3 + ov) / 4) << s;
            }
            argb[row + x] = mixed;
        }
    }
}

void filter_crt(std::vector<uint32_t>& argb, int w, int h) {
    static std::vector<uint32_t> src;
    src = argb;
    const float cx = (w - 1) * 0.5f;
    const float cy = (h - 1) * 0.5f;
    for (int y = 0; y < h; ++y) {
        const size_t row = static_cast<size_t>(y) * w;
        const bool scan = (y & 1) != 0;
        const float sy = scan ? 0.72f : 1.0f;
        const float dy = (y - cy) / (h * 0.5f);
        for (int x = 0; x < w; ++x) {
            const uint32_t l = src[row + (x > 0 ? x - 1 : x)];
            const uint32_t c = src[row + x];
            const uint32_t r = src[row + (x + 1 < w ? x + 1 : x)];
            const float dx = (x - cx) / (w * 0.5f);
            float v = 1.0f - 0.22f * (dx * dx + dy * dy);
            if (v < 0.62f) {
                v = 0.62f;
            }
            const float scale = sy * v;
            uint32_t out = 0xFF000000u;
            for (int s = 16; s >= 0; s -= 8) {
                const uint32_t blur =
                    (((l >> s) & 0xFF) + 2 * ((c >> s) & 0xFF) +
                     ((r >> s) & 0xFF)) / 4;
                uint32_t ch = static_cast<uint32_t>(blur * scale + 0.5f);
                if (ch > 255) {
                    ch = 255;
                }
                out |= ch << s;
            }
            argb[row + x] = out;
        }
    }
}

int run_headless(Engine& engine, const Options& opt) {
    std::printf("%s\n", engine.status().c_str());
    if (!engine.caption().empty()) {
        std::string cap = engine.caption();
        for (char& c : cap) {
            if (c == '\n') {
                c = '/';
            }
        }
        std::printf("caption: %s\n", cap.c_str());
    }
    if (opt.start_x >= 0 && opt.start_y >= 0) {
        engine.warp_player(opt.start_x, opt.start_y);
    }
    if (opt.level >= 0) {
        engine.set_level_id(opt.level);
    }
    if (opt.text_page > 0) {
        engine.show_text_page(opt.text_page);
    }
    if (opt.blade) {
        engine.debug_spawn_blade();
    }
    if (opt.morph >= 0) {
        engine.debug_morph(opt.morph);
    }
    if (opt.face != 0) {
        engine.debug_face(opt.face);
    }
    int sfx_requests = 0;
    for (int i = 0; i < opt.ticks; ++i) {
        engine.set_input(0, 0, false, opt.fire && i == 0);
        engine.tick();
        const std::string music = engine.take_music_request();
        if (!music.empty()) {
            std::printf("music: %s\n", music.c_str());
        }
        sfx_requests +=
            static_cast<int>(engine.take_sound_requests().size());
        if (opt.watch_tile >= 0) {
            const TileInfo& t = engine.tile(opt.watch_tile);
            std::printf("tick %3d frame %3d tile 0x%03X shape 0x%04X flags 0x%04X\n",
                        i + 1, engine.frame(), opt.watch_tile,
                        static_cast<unsigned>(t.shape),
                        static_cast<unsigned>(t.flags));
        }
        if ((i + 1) % 60 == 0) {
            std::printf("tick %d: %s\n", i + 1, engine.status().c_str());
        }
    }
    std::printf("done: %s\n", engine.status().c_str());
    if (sfx_requests > 0) {
        std::printf("sfx requests: %d\n", sfx_requests);
    }
    if (!engine.caption().empty()) {
        std::string cap = engine.caption();
        for (char& c : cap) {
            if (c == '\n') {
                c = '/';
            }
        }
        std::printf("caption now: %s\n", cap.c_str());
    }

    if (opt.save_slot >= 0) {
        std::string save_err;
        if (engine.save_game(opt.save_slot, save_err)) {
            std::printf("saved slot %d\n", opt.save_slot);
        } else {
            std::fprintf(stderr, "save slot %d: %s\n", opt.save_slot,
                         save_err.c_str());
        }
    }

    if (!opt.dump_objects.empty()) {
        FILE* f = std::fopen(opt.dump_objects.c_str(), "w");
        if (!f) {
            std::fprintf(stderr, "cannot write %s\n", opt.dump_objects.c_str());
            return 1;
        }
        std::fprintf(f, "# idx kind x y dx dy w h state p0f p11 p13 flags ttl\n");
        const auto& objs = engine.objects();
        for (size_t i = 0; i < objs.size(); ++i) {
            const BoardObject& o = objs[i];
            std::fprintf(f, "%3d %2d %4d %4d %3d %3d %2d %2d %3d %3d %3d %3d 0x%04X %d\n",
                         static_cast<int>(i), o.kind, o.x, o.y, o.dx, o.dy,
                         o.w, o.h, o.state, o.p0f, o.p11, o.p13,
                         static_cast<unsigned>(o.flags), o.ttl);
        }
        std::fclose(f);
    }

    if (!opt.dump.empty()) {
        uint8_t palette[768];
        load_palette(opt.palette, palette);
        Framebuffer fb;
        if (opt.stereo == 1) {
            Framebuffer left;
            Framebuffer right;
            const int shift = std::max(1, opt.stereo_depth);
            engine.set_stereo_convergence(opt.stereo_convergence);
            engine.set_stereo_shift(shift);
            engine.render(left);
            engine.set_stereo_shift(-shift);
            engine.render(right);
            engine.set_stereo_shift(0);
            engine.set_stereo_convergence(0);
            compose_stereo(fb, left, right, opt.stereo);
        } else {
            engine.set_stereo_shift(0);
            engine.set_stereo_convergence(0);
            engine.render(fb);
        }
        std::vector<uint32_t> argb(fb.px.size());
        for (size_t i = 0; i < fb.px.size(); ++i) {
            const uint8_t c = fb.px[i];
            const uint32_t r = palette[c * 3 + 0];
            const uint32_t g = palette[c * 3 + 1];
            const uint32_t b = palette[c * 3 + 2];
            argb[i] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
        if (opt.filter == 1) {
            filter_smooth(argb, Framebuffer::W, Framebuffer::H);
        } else if (opt.filter == 2) {
            filter_crt(argb, Framebuffer::W, Framebuffer::H);
        }
        FILE* f = std::fopen(opt.dump.c_str(), "wb");
        if (!f) {
            std::fprintf(stderr, "cannot write %s\n", opt.dump.c_str());
            return 1;
        }
        std::fprintf(f, "P6\n%d %d\n255\n", Framebuffer::W, Framebuffer::H);
        for (uint32_t p : argb) {
            uint8_t rgb[3] = {static_cast<uint8_t>(p >> 16),
                              static_cast<uint8_t>(p >> 8),
                              static_cast<uint8_t>(p)};
            std::fwrite(rgb, 1, 3, f);
        }
        std::fclose(f);
        std::printf("wrote %s\n", opt.dump.c_str());
    }
    return 0;
}

#ifdef JILL_HAVE_SDL

// Keyboard/gamepad bindings.  Every action has two keyboard slots (the second
// may be SDL_SCANCODE_UNKNOWN) and one SDL game-controller button.
enum ActionId {
    ACT_LEFT,
    ACT_RIGHT,
    ACT_UP,
    ACT_DOWN,
    ACT_JUMP,
    ACT_THROW,
    ACT_COUNT
};

struct Bindings {
    SDL_Scancode key1[ACT_COUNT] = {
        SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_UP,
        SDL_SCANCODE_DOWN, SDL_SCANCODE_Z, SDL_SCANCODE_X};
    SDL_Scancode key2[ACT_COUNT] = {
        SDL_SCANCODE_A, SDL_SCANCODE_D, SDL_SCANCODE_W,
        SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_SPACE, SDL_SCANCODE_LALT};
    int pad[ACT_COUNT] = {-1, -1, -1, -1, SDL_CONTROLLER_BUTTON_A,
                          SDL_CONTROLLER_BUTTON_X};
};

void reset_bindings(Bindings& b) { b = Bindings{}; }

const char* const kBindActionKey[ACT_COUNT] = {
    "move_left", "move_right", "move_up", "move_down", "jump", "throw"};

std::string scancode_name(SDL_Scancode sc) {
    if (sc == SDL_SCANCODE_UNKNOWN) {
        return "";
    }
    const char* n = SDL_GetScancodeName(sc);
    return (n && *n) ? std::string(n) : std::string();
}

void load_bindings_ini(const IniFile& ini, Bindings& b,
                       const std::string& section = "controls") {
    for (int a = 0; a < ACT_COUNT; ++a) {
        const std::string value = ini.get(section, kBindActionKey[a]);
        if (!value.empty()) {
            const size_t bar = value.find('|');
            const std::string first = value.substr(0, bar);
            const std::string second =
                bar == std::string::npos ? std::string() : value.substr(bar + 1);
            // Older ports bound move-down to S as well.  S is the original
            // in-game SAVE key, so the default binding gives S back.
            const bool legacy_s_down =
                a == ACT_DOWN && first == "Down" && second == "S";
            if (!first.empty()) {
                b.key1[a] = SDL_GetScancodeFromName(first.c_str());
            }
            b.key2[a] = (second.empty() || legacy_s_down)
                            ? SDL_SCANCODE_UNKNOWN
                            : SDL_GetScancodeFromName(second.c_str());
        }
        const std::string pad = ini.get(
            section, std::string(kBindActionKey[a]) + "_pad");
        b.pad[a] = pad.empty()
                       ? -1
                       : SDL_GameControllerGetButtonFromString(pad.c_str());
    }
}

void save_bindings_ini(IniFile& ini, const Bindings& b,
                       const std::string& section = "controls") {
    for (int a = 0; a < ACT_COUNT; ++a) {
        std::string value = scancode_name(b.key1[a]);
        if (b.key2[a] != SDL_SCANCODE_UNKNOWN) {
            value += "|" + scancode_name(b.key2[a]);
        }
        ini.set(section, kBindActionKey[a], value);
        const char* pad = b.pad[a] >= 0
                              ? SDL_GameControllerGetStringForButton(
                                    static_cast<SDL_GameControllerButton>(b.pad[a]))
                              : "";
        ini.set(section, std::string(kBindActionKey[a]) + "_pad",
                pad ? pad : "");
    }
}

bool bindings_equal(const Bindings& a, const Bindings& b) {
    for (int i = 0; i < ACT_COUNT; ++i) {
        if (a.key1[i] != b.key1[i] || a.key2[i] != b.key2[i] ||
            a.pad[i] != b.pad[i]) {
            return false;
        }
    }
    return true;
}

bool key_down(const uint8_t* keys, const Bindings& b, int action) {
    if (keys[b.key1[action]]) {
        return true;
    }
    return b.key2[action] != SDL_SCANCODE_UNKNOWN && keys[b.key2[action]];
}

bool pad_button(SDL_GameController* gc, const Bindings& b, int action) {
    return gc && b.pad[action] >= 0 &&
           SDL_GameControllerGetButton(
               gc, static_cast<SDL_GameControllerButton>(b.pad[action])) != 0;
}

// XInput pads arrive through SDL_GameController; DirectInput / HID pads that
// SDL does not map as controllers arrive through the raw SDL_Joystick fallback.
void poll_input(const uint8_t* keys, const Bindings& b,
                SDL_GameController* gc, SDL_Joystick* joy, Engine& engine) {
    int h = 0;
    int v = 0;
    if (key_down(keys, b, ACT_LEFT) || pad_button(gc, b, ACT_LEFT)) {
        --h;
    }
    if (key_down(keys, b, ACT_RIGHT) || pad_button(gc, b, ACT_RIGHT)) {
        ++h;
    }
    if (key_down(keys, b, ACT_UP) || pad_button(gc, b, ACT_UP)) {
        --v;
    }
    if (key_down(keys, b, ACT_DOWN) || pad_button(gc, b, ACT_DOWN)) {
        ++v;
    }

    bool jump = key_down(keys, b, ACT_JUMP) || pad_button(gc, b, ACT_JUMP);
    bool throw_btn = key_down(keys, b, ACT_THROW) || pad_button(gc, b, ACT_THROW);

    if (gc) {
        const int dz = 8000;
        int ax = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTX);
        int ay = SDL_GameControllerGetAxis(gc, SDL_CONTROLLER_AXIS_LEFTY);
        if (ax < -dz) {
            --h;
        } else if (ax > dz) {
            ++h;
        }
        if (ay < -dz) {
            --v;
        } else if (ay > dz) {
            ++v;
        }
        if (SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_LEFT)) {
            --h;
        }
        if (SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) {
            ++h;
        }
        if (SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_UP)) {
            --v;
        }
        if (SDL_GameControllerGetButton(gc, SDL_CONTROLLER_BUTTON_DPAD_DOWN)) {
            ++v;
        }
    } else if (joy) {
        const int dz = 8000;
        int ax = SDL_JoystickGetAxis(joy, 0);
        int ay = SDL_JoystickGetAxis(joy, 1);
        if (ax < -dz) {
            --h;
        } else if (ax > dz) {
            ++h;
        }
        if (ay < -dz) {
            --v;
        } else if (ay > dz) {
            ++v;
        }
        Uint8 hat = SDL_JoystickGetHat(joy, 0);
        if (hat & SDL_HAT_LEFT) {
            --h;
        }
        if (hat & SDL_HAT_RIGHT) {
            ++h;
        }
        if (hat & SDL_HAT_UP) {
            --v;
        }
        if (hat & SDL_HAT_DOWN) {
            ++v;
        }
        // Standard DirectInput pad layout: 0 = A, 2 = X.
        jump = jump || SDL_JoystickGetButton(joy, 0) != 0;
        throw_btn = throw_btn || SDL_JoystickGetButton(joy, 2) != 0;
    }

    engine.set_input(h, v, jump, throw_btn);
}

struct LevelSelect {
    bool open = false;
    int game = 0;   // index into the installed game list
    int board = 0;
};

void draw_select(Framebuffer& fb, Engine& engine,
                 const std::vector<GameDef>& games, const LevelSelect& sel) {
    if (sel.game < 0 || sel.game >= static_cast<int>(games.size())) {
        return;
    }
    const GameDef& g = games[static_cast<size_t>(sel.game)];
    int n = g.board_count();
    // A total-conversion board list can exceed the screen (JillE has 21).  Show
    // a scrolling window instead of letting the panel grow past 200 pixels.
    constexpr int kMaxRows = 16;
    const int shown = std::min(n, kMaxRows);
    int first = 0;
    if (n > shown) {
        first = std::max(0, std::min(sel.board - shown / 2, n - shown));
    }
    int boxw = 168;
    int boxh = 46 + shown * 8;
    int x = (Framebuffer::W - boxw) / 2;
    int y = (Framebuffer::H - boxh) / 2;
    if (y < 2) {
        y = 2;
    }
    fb.fill_rect(x, y, boxw, boxh, 0);
    fb.fill_rect(x + 1, y + 1, boxw - 2, boxh - 2, 8);
    char line[64];
    std::snprintf(line, sizeof(line), "%s", upper_ascii(g.title.c_str()).c_str());
    engine.draw_text(fb, x + 6, y + 4, 7, line);
    engine.draw_text(fb, x + 6, y + 12, 7, "UP/DOWN  LEFT/RIGHT  ENTER");
    if (n > shown) {
        std::snprintf(line, sizeof(line), "%d-%d / %d", first + 1,
                      first + shown, n);
        const int w = 6 * static_cast<int>(std::strlen(line));
        engine.draw_text(fb, x + boxw - 6 - w, y + 4, 5, line);
    }
    for (int r = 0; r < shown; ++r) {
        const int i = first + r;
        std::snprintf(line, sizeof(line), "%c %s",
                      i == sel.board ? '>' : ' ', g.board(i).c_str());
        engine.draw_text(fb, x + 8, y + 24 + r * 8,
                         i == sel.board ? 7 : 5, line);
    }
    engine.draw_text(fb, x + 6, y + boxh - 12, 7, "F3/ESC CLOSE");
}

bool apply_hud_frame(Engine& engine, const std::vector<GameDef>& games,
                     int src);
bool apply_theme_frame(Engine& engine, const std::vector<GameDef>& games,
                       int theme);
bool apply_menu_frame(Engine& engine, const std::vector<GameDef>& games,
                      int src, int theme);

// Apply the selected game/board.  A different game needs a fresh Engine (the
// shape bank, tile table and kind tables can all differ).
bool apply_selection(Engine& engine, const std::vector<GameDef>& games,
                     const LevelSelect& sel, int hud_frame,
                     int menu_frame) {
    if (sel.game < 0 || sel.game >= static_cast<int>(games.size())) {
        return false;
    }
    const GameDef& g = games[static_cast<size_t>(sel.game)];
    if (sel.board < 0 || sel.board >= g.board_count()) {
        return false;
    }
    const bool hud = engine.hud_classic();
    const bool hud_portrait = engine.hud_portrait();
    const int hud_bg = engine.hud_bg();
    const int hud_text = engine.hud_text();
    const int hud_health = engine.hud_health();
    const bool fly = engine.debug_fly();
    const bool invincible = engine.invincible();
    const int super_jump = engine.super_jump();
    const int menu_theme = engine.menu_theme();
    const bool fire_crouched = engine.fire_while_crouched();
    const int knives = engine.max_knives();
    const bool retain_items = engine.retain_items();
    const bool fast_dialogs = engine.fast_dialogs();
    const int message_box = engine.message_box();
    const bool air_jump = engine.air_jump_buffer();
    const bool minimap = engine.minimap();
    const bool center_fish = engine.center_fish_shot();
    const int player_look = engine.player_look();
    const int player_suit = engine.player_suit();
    const std::string hud_panel = engine.hud_panel();
    const std::string font_name = engine.font_name();
    const int menu_border_ext = engine.menu_border_ext();
    const bool turtle = engine.turtle();
    std::string err;
    if (engine.game().id != g.id) {
        Engine fresh;
        if (!fresh.init(g, err)) {
            std::fprintf(stderr, "%s: %s\n", g.id.c_str(), err.c_str());
            return false;
        }
        fresh.adopt_external_state(engine);
        engine = std::move(fresh);
    }
    engine.set_hud_panel(hud_panel);
    engine.set_font_name(font_name);
    engine.set_menu_border_ext(menu_border_ext);
    engine.set_turtle(turtle);
    engine.set_fast_dialogs(fast_dialogs);
    engine.set_message_box(message_box);
    engine.set_hud_classic(hud);  // camera centre depends on the HUD style
    if (!engine.load_board(g.board(sel.board), err)) {
        std::fprintf(stderr, "load %s: %s\n", g.board(sel.board).c_str(),
                     err.c_str());
        return false;
    }
    engine.set_hud_classic(hud);
    engine.set_hud_portrait(hud_portrait);
    engine.set_hud_bg(hud_bg);
    engine.set_hud_text(hud_text);
    engine.set_hud_health(hud_health);
    engine.set_debug_fly(fly);
    engine.set_invincible(invincible);
    engine.set_super_jump(super_jump);
    engine.set_menu_theme(menu_theme);
    engine.set_fire_while_crouched(fire_crouched);
    engine.set_max_knives(knives);
    engine.set_retain_items(retain_items);
    engine.set_fast_dialogs(fast_dialogs);
    engine.set_message_box(message_box);
    engine.set_air_jump_buffer(air_jump);
    engine.set_minimap(minimap);
    engine.set_center_fish_shot(center_fish);
    apply_player_look(engine, games, player_look, player_suit);
    apply_hud_frame(engine, games, hud_frame);
    apply_theme_frame(engine, games, menu_theme);
    apply_menu_frame(engine, games, menu_frame, menu_theme);
    return true;
}

// --- per-game high scores -----------------------------------------------
struct HiEntry {
    uint32_t score = 0;
    std::string name;
};

struct HiScores {
    std::string game_id;
    std::vector<HiEntry> entries;
    bool loaded = false;
};

std::string hiscore_safe_id(const std::string& id) {
    std::string out;
    for (char c : id) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                        (c >= '0' && c <= '9') || c == '_' || c == '-';
        out.push_back(ok ? c : '_');
    }
    return out.empty() ? std::string("game") : out;
}

std::string hiscore_path(const std::string& game_id) {
    return "hiscore_" + hiscore_safe_id(game_id) + ".txt";
}

std::string hiscore_default_name(const std::string& game_id) {
    (void)game_id;
    return "JILL";
}

void load_hiscores(HiScores& hi, const std::string& game_id) {
    if (hi.loaded && hi.game_id == game_id) {
        return;
    }
    hi.game_id = game_id;
    hi.entries.clear();
    hi.loaded = true;
    FILE* f = std::fopen(hiscore_path(game_id).c_str(), "rb");
    if (!f) {
        return;
    }
    char line[128];
    while (std::fgets(line, sizeof(line), f)) {
        unsigned long score = 0;
        char name[32] = {0};
        if (std::sscanf(line, "%lu %31[^\r\n]", &score, name) < 1) {
            continue;
        }
        HiEntry e;
        e.score = static_cast<uint32_t>(score);
        e.name = name[0] ? name : hiscore_default_name(game_id);
        hi.entries.push_back(e);
    }
    std::fclose(f);
    std::sort(hi.entries.begin(), hi.entries.end(),
              [](const HiEntry& a, const HiEntry& b) {
                  return a.score > b.score;
              });
    if (hi.entries.size() > 10) {
        hi.entries.resize(10);
    }
}

void save_hiscores(const HiScores& hi) {
    FILE* f = std::fopen(hiscore_path(hi.game_id).c_str(), "wb");
    if (!f) {
        return;
    }
    for (const HiEntry& e : hi.entries) {
        std::fprintf(f, "%lu %s\n", static_cast<unsigned long>(e.score),
                     e.name.c_str());
    }
    std::fclose(f);
}

bool hiscore_qualifies(HiScores& hi, const std::string& game_id,
                       uint32_t score) {
    if (score == 0) {
        return false;
    }
    load_hiscores(hi, game_id);
    return hi.entries.size() < 10 || score > hi.entries.back().score;
}

bool add_hiscore(HiScores& hi, const std::string& game_id, uint32_t score,
                 const std::string& name) {
    if (score == 0) {
        return false;
    }
    load_hiscores(hi, game_id);
    size_t pos = 0;
    while (pos < hi.entries.size() && hi.entries[pos].score >= score) {
        ++pos;
    }
    if (pos >= 10 && hi.entries.size() >= 10) {
        return false;
    }
    HiEntry e;
    e.score = score;
    e.name = name.empty() ? hiscore_default_name(game_id) : name;
    hi.entries.insert(hi.entries.begin() + static_cast<long>(pos), e);
    if (hi.entries.size() > 10) {
        hi.entries.resize(10);
    }
    save_hiscores(hi);
    return true;
}

int game_index_for_id(const std::vector<GameDef>& games,
                      const std::string& id) {
    for (size_t i = 0; i < games.size(); ++i) {
        if (games[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int save_slots_for(const GameDef& g) {
    int slots = g.save_slots;
    if (slots <= 0 && g.profile && g.profile->save_slots > 0) {
        slots = g.profile->save_slots;
    }
    if (slots <= 0) {
        slots = 6;
    }
    return slots > 6 ? 6 : slots;
}

std::string save_prefix_for(const GameDef& g) {
    if (!g.save_prefix.empty()) {
        return g.save_prefix;
    }
    if (g.profile && g.profile->save_prefix) {
        return g.profile->save_prefix;
    }
    return g.id;
}

std::string save_label_for(const GameDef& g, int slot) {
    Board b;
    std::string err;
    const std::string path = save_prefix_for(g) + "." + std::to_string(slot);
    if (!b.load(path, err)) {
        return std::string();
    }
    const uint32_t score = static_cast<uint32_t>(b.header[0x26]) |
                           (static_cast<uint32_t>(b.header[0x27]) << 8) |
                           (static_cast<uint32_t>(b.header[0x28]) << 16) |
                           (static_cast<uint32_t>(b.header[0x29]) << 24);
    char buf[48];
    std::snprintf(buf, sizeof(buf), "LV %d  %lu", b.level_id(),
                  static_cast<unsigned long>(score));
    return buf;
}

// Front end: main menu -> episode select / options -> controls/enhancements.
enum class MenuScreen {
    Closed,
    Main,
    Episodes,
    Options,
    Hud,
    Video,
    Audio,
    Menus,
    Controls,
    Cheats,
    Enhancements,
    SaveGame,
    LoadGame,
    HighScores,
    Noisemaker,
    Assets
};

struct Menu {
    MenuScreen screen = MenuScreen::Main;
    bool classic = false;  // draw menus with the original frame/window art
    bool live_background = false;  // modern menu over the live game screen
    int hud_frame = 0;     // classic HUD art source (0 = current game)
    std::string hud_panel;  // external HUD panel name ("" = default)
    int menu_frame = -1;   // border: -1 theme, 0 current, N = game,
                           // > games = external border
    std::string menu_font;  // external font set ("" = game font)
    int main_sel = 0;
    int ep_sel = 0;
    int opt_sel = 0;
    int hud_sel = 0;
    int video_sel = 0;
    int audio_sel = 0;
    int menus_sel = 0;
    int ctrl_sel = 0;
    int cheat_sel = 0;
    int enh_sel = 0;
    int save_sel = 0;
    int load_game_index = -1;      // episode shown on RESTORE GAME
    std::vector<std::string> load_labels;
    int hi_sel = 0;
    int hi_game = -1;              // episode shown on HIGH SCORES
    HiScores hi_scores;
    int noise_sel = 0;       // front-end sound-test row
    int assets_sel = 0;      // external pack row
    int noise_set = 0;       // installed game whose .vcl is auditioned
    int noise_sound = 1;     // raw sample slot 0..49
    bool noise_loaded = false;  // bank matches noise_set
    int rebind_action = -1;  // >= 0: waiting for the next key/button
    bool music_on = true;    // mirrored from the player for the SOUND screen
    int music_volume = 80;
    bool sfx_on = true;
    int sfx_volume = 80;
    bool pc_speaker = false;  // 1-bit square-wave effect mode
};

void ensure_hiscores(Menu& m, const std::vector<GameDef>& games, int index) {
    if (index < 0 || index >= static_cast<int>(games.size())) {
        return;
    }
    if (m.hi_game != index) {
        m.hi_game = index;
        load_hiscores(m.hi_scores, games[static_cast<size_t>(index)].id);
    }
}

void ensure_load_labels(Menu& m, const std::vector<GameDef>& games,
                        int index) {
    if (index < 0 || index >= static_cast<int>(games.size())) {
        return;
    }
    if (m.load_game_index == index && !m.load_labels.empty()) {
        return;
    }
    m.load_game_index = index;
    const GameDef& g = games[static_cast<size_t>(index)];
    const int slots = save_slots_for(g);
    m.load_labels.assign(static_cast<size_t>(slots), std::string());
    for (int i = 0; i < slots; ++i) {
        m.load_labels[static_cast<size_t>(i)] = save_label_for(g, i);
    }
}

bool menu_open(const Menu& m) { return m.screen != MenuScreen::Closed; }

int& menu_sel(Menu& m) {
    switch (m.screen) {
        case MenuScreen::Episodes:
            return m.ep_sel;
        case MenuScreen::Options:
            return m.opt_sel;
        case MenuScreen::Hud:
            return m.hud_sel;
        case MenuScreen::Video:
            return m.video_sel;
        case MenuScreen::Audio:
            return m.audio_sel;
        case MenuScreen::Menus:
            return m.menus_sel;
        case MenuScreen::Controls:
            return m.ctrl_sel;
        case MenuScreen::Cheats:
            return m.cheat_sel;
        case MenuScreen::Enhancements:
            return m.enh_sel;
        case MenuScreen::SaveGame:
        case MenuScreen::LoadGame:
            return m.save_sel;
        case MenuScreen::HighScores:
            return m.hi_sel;
        case MenuScreen::Noisemaker:
            return m.noise_sel;
        case MenuScreen::Assets:
            return m.assets_sel;
        default:
            return m.main_sel;
    }
}

const char* const kActionLabel[ACT_COUNT] = {"MOVE LEFT", "MOVE RIGHT",
                                             "MOVE UP", "MOVE DOWN", "JUMP",
                                             "THROW"};

std::string upper_ascii(const char* s) {
    std::string out = s ? s : "";
    for (char& c : out) {
        if (c >= 'a' && c <= 'z') {
            c = static_cast<char>(c - 'a' + 'A');
        }
    }
    return out;
}

std::string key_name(SDL_Scancode sc) {
    if (sc == SDL_SCANCODE_UNKNOWN) {
        return "-";
    }
    const char* n = SDL_GetScancodeName(sc);
    return (n && *n) ? upper_ascii(n) : "?";
}

std::string pad_name(int button) {
    if (button < 0) {
        return "-";
    }
    const char* n = SDL_GameControllerGetStringForButton(
        static_cast<SDL_GameControllerButton>(button));
    std::string out = upper_ascii(n);
    for (char& c : out) {
        if (c == '_') {
            c = ' ';
        }
    }
    return out.empty() ? "?" : out;
}

std::string binding_label(const Bindings& b, int action) {
    std::string s = key_name(b.key1[action]);
    if (b.key2[action] != SDL_SCANCODE_UNKNOWN) {
        s += "/" + key_name(b.key2[action]);
    }
    if (b.pad[action] >= 0) {
        s += " [" + pad_name(b.pad[action]) + "]";
    }
    return s;
}

const char* hud_bg_name(int bg) {
    switch (bg) {
        case 0: return "BLACK";
        case 1: return "DARK BLUE";
        case 2: return "DARK GREEN";
        case 4: return "DARK RED";
        case 5: return "DARK PURPLE";
        case 6: return "BROWN";
        case 8: return "GREY";
        case -2: return "SUIT";
        case -3: return "SUIT VIBRANT";
        default: return "CUSTOM";
    }
}

int next_hud_bg(int bg) {
    static const int kOrder[] = {8, 0, 1, 2, 4, 6, 5, -2, -3};
    const int n = static_cast<int>(sizeof(kOrder) / sizeof(kOrder[0]));
    for (int i = 0; i < n; ++i) {
        if (kOrder[i] == bg) {
            return kOrder[(i + 1) % n];
        }
    }
    return 8;
}

// HUD text colours: the 16 EGA palette entries the original text routines use.
const char* hud_text_name(int c) {
    static const char* const kNames[16] = {
        "BLACK", "BLUE", "GREEN", "CYAN", "RED", "PURPLE", "BROWN", "WHITE",
        "GREY", "LT BLUE", "LT GREEN", "LT CYAN", "LT RED", "LT PURPLE",
        "YELLOW", "BRIGHT WHITE"};
    return kNames[c & 0x0F];
}

int next_hud_text(int c) { return (c + 1) & 0x0F; }

// HUD health colour: -1 keeps the original pip art, 0..15 repaints the pips.
const char* hud_health_name(int c) {
    if (c < 0) {
        return "ORIGINAL";
    }
    return hud_text_name(c);
}

int next_hud_health(int c) {
    if (c < 0) {
        return 0;
    }
    return (c >= 15) ? -1 : c + 1;
}

// --- video settings -------------------------------------------------------
struct VideoSettings {
    int mode = 0;        // 0 windowed, 1 fullscreen, 2 borderless
    int scale = 3;       // windowed integer scale, 1..6
    int resolution = 0;  // 0 desktop, else index into kResolutions
    int high_fps = 60;   // presentation rate, 0 = 18.2 Hz
    int splash = -1;     // -1 auto, 0 off, N = games[N-1] splash
    bool interpolate = true;  // high-framerate smoothing (needs high_fps > 0)
    int filter = 0;      // 0 none, 1 smooth (light blur), 2 CRT
    int stereo = 0;      // 0 off, 1 row-interlaced, 2 half-width SBS
    int stereo_depth = 2;  // per-eye horizontal separation in source pixels
    int stereo_convergence = 0;  // common inward pull for both eyes
};

// Presentation rates.  The world always ticks at 18.2065 Hz; these only change
// how often the already-ticked frame is presented.
const int kHighFps[] = {0, 60, 90, 120, 144};
constexpr int kHighFpsCount =
    static_cast<int>(sizeof(kHighFps) / sizeof(kHighFps[0]));

std::string high_fps_name(int hz) {
    if (hz <= 0) {
        return "OFF (18.2 FPS)";
    }
    return std::to_string(hz) + " FPS";
}

const char* filter_name(int filter) {
    switch (filter) {
        case 1: return "SMOOTH";
        case 2: return "CRT";
        default: return "NONE";
    }
}

const char* message_box_name(int style) {
    switch (style) {
        case 1: return "MODERN";
        case 2: return "CLASSIC";
        default: return "OFF";
    }
}

int filter_next(int filter) { return (filter + 1) % 3; }

// SBS output is currently disabled; only OFF and row-interlaced are exposed.
const char* stereo_name(int stereo) {
    return stereo == 1 ? "INTERLACED" : "OFF";
}

int stereo_next(int stereo, int step) {
    const int cur = stereo == 1 ? 1 : 0;
    int v = (cur + step) % 2;
    return (v < 0) ? v + 2 : v;
}

struct ResolutionOption {
    const char* label;
    int w;
    int h;
};

const ResolutionOption kResolutions[] = {
    {"DESKTOP", 0, 0},   {"640x480", 640, 480},   {"800x600", 800, 600},
    {"1024x768", 1024, 768}, {"1280x720", 1280, 720}, {"1600x900", 1600, 900},
    {"1920x1080", 1920, 1080},
};
constexpr int kResolutionCount =
    static_cast<int>(sizeof(kResolutions) / sizeof(kResolutions[0]));

int match_resolution(int w, int h) {
    for (int i = 0; i < kResolutionCount; ++i) {
        if (kResolutions[i].w == w && kResolutions[i].h == h) {
            return i;
        }
    }
    return 0;
}
const char* const kWindowModes[] = {"WINDOWED", "FULLSCREEN", "BORDERLESS"};

int window_width(const VideoSettings& v) {
    return v.resolution ? kResolutions[v.resolution].w
                        : Framebuffer::W * v.scale;
}

int window_height(const VideoSettings& v) {
    return v.resolution ? kResolutions[v.resolution].h
                        : Framebuffer::H * v.scale;
}

std::string super_jump_name(int n) {
    if (n <= 0) {
        return "OFF";
    }
    return std::to_string(n) + " FEATHERS";
}

int external_theme_count(const Engine& engine) {
    const ExternalVisuals* ext = engine.external_visuals();
    return ext ? static_cast<int>(ext->themes().size()) : 0;
}

std::string menu_theme_name(int t, const std::vector<GameDef>& games,
                            const ExternalVisuals* ext) {
    const int game_count = static_cast<int>(games.size());
    const int ext_count =
        ext ? static_cast<int>(ext->theme_names().size()) : 0;
    if (t <= 0 || t > game_count + ext_count) {
        return "PORT";
    }
    if (t > game_count) {
        return upper_ascii(
            ext->theme_names()[static_cast<size_t>(t - game_count - 1)].c_str());
    }
    return upper_ascii(games[static_cast<size_t>(t - 1)].title.c_str());
}

std::string splash_name(int t, const std::vector<GameDef>& games) {
    if (t < 0) {
        return "AUTO";
    }
    if (t == 0) {
        return "OFF";
    }
    if (t > static_cast<int>(games.size())) {
        return "AUTO";
    }
    return upper_ascii(games[static_cast<size_t>(t - 1)].id.c_str());
}

// Theme 0 is the plain port backdrop, 1..N are the installed games' intro
// boards and N+1.. are the mounted pack's `themes/` images, so the cycle
// length follows the game list plus the pack instead of a fixed 3.
int menu_theme_next(int t, int step, const std::vector<GameDef>& games,
                    int ext_themes) {
    const int n = static_cast<int>(games.size()) + 1 + ext_themes;
    int v = (t + step) % n;
    return (v < 0) ? v + n : v;
}

int splash_next(int t, int step, const std::vector<GameDef>& games) {
    const int n = static_cast<int>(games.size()) + 2;
    int v = ((t + 1) + step) % n;
    if (v < 0) {
        v += n;
    }
    return v - 1;
}

// Classic HUD/menu frame sources: 0 = the running game's own shapes, N = the
// N-th installed game's shape bank (shape entry 3 frame, entry 7 window).
std::string frame_source_name(int src, const std::vector<GameDef>& games) {
    if (src <= 0 || src > static_cast<int>(games.size())) {
        return "CURRENT";
    }
    return upper_ascii(games[static_cast<size_t>(src - 1)].title.c_str());
}

int frame_source_next(int src, int step, int game_count) {
    const int n = game_count + 1;
    int v = (src + step) % n;
    return (v < 0) ? v + n : v;
}

int external_border_count(const Engine& engine) {
    const ExternalVisuals* ext = engine.external_visuals();
    return ext ? static_cast<int>(ext->border_names().size()) : 0;
}

// Menu BORDER values: -1 = THEME, 0 = CURRENT, 1..N = installed game,
// N+1..N+B = external `borders/` assets in discovery order.
std::string menu_border_name(int src, const std::vector<GameDef>& games,
                             const ExternalVisuals* ext) {
    if (src < 0) {
        return "THEME";
    }
    const int game_count = static_cast<int>(games.size());
    if (src > game_count && ext) {
        const int i = src - game_count - 1;
        if (i >= 0 && i < static_cast<int>(ext->border_names().size())) {
            return upper_ascii(
                ext->border_names()[static_cast<size_t>(i)].c_str());
        }
    }
    return frame_source_name(src, games);
}

int menu_border_next(int src, int step, int game_count, int ext_borders) {
    const int n = game_count + 2 + ext_borders;
    int v = ((src + 1) + step) % n;
    if (v < 0) {
        v += n;
    }
    return v - 1;
}

std::string hud_panel_name(const std::string& name) {
    return name.empty() ? "ORIGINAL" : upper_ascii(name.c_str());
}

// The HUD list depends on the current style: classic panels are 64x84, modern
// ones are the full-width bar.
std::vector<std::string> hud_panel_names(const ExternalVisuals* ext,
                                         bool classic) {
    std::vector<std::string> out;
    if (!ext) {
        return out;
    }
    for (const ExternalHud& h : ext->huds()) {
        if (h.classic == classic) {
            out.push_back(h.name);
        }
    }
    return out;
}

std::string hud_panel_next(const std::string& current, bool classic,
                           const ExternalVisuals* ext) {
    std::vector<std::string> names = hud_panel_names(ext, classic);
    if (names.empty()) {
        return std::string();
    }
    int idx = -1;
    for (size_t i = 0; i < names.size(); ++i) {
        if (names[i] == current) {
            idx = static_cast<int>(i);
            break;
        }
    }
    idx = (idx + 1) % static_cast<int>(names.size() + 1);
    return idx == 0 ? std::string() : names[static_cast<size_t>(idx - 1)];
}

std::string font_name_next(const std::string& current,
                           const ExternalVisuals* ext) {
    const int count =
        ext ? static_cast<int>(ext->font_names().size()) : 0;
    int idx = -1;
    for (int i = 0; i < count; ++i) {
        if (ext->font_names()[static_cast<size_t>(i)] == current) {
            idx = i;
            break;
        }
    }
    idx = (idx + 1) % (count + 1);
    return idx == 0 ? std::string()
                    : ext->font_names()[static_cast<size_t>(idx - 1)];
}

std::string font_name(const std::string& name) {
    return name.empty() ? "ORIGINAL" : upper_ascii(name.c_str());
}

std::string midi_device_label(const MusicPlayer& audio) {
    const int index = audio.midi_device();
    if (index >= 0) {
        const std::vector<std::string> names = MusicPlayer::midi_devices();
        if (index < static_cast<int>(names.size())) {
            return upper_ascii(names[static_cast<size_t>(index)].c_str());
        }
    }
    const std::vector<std::string> names = MusicPlayer::midi_devices();
    return names.empty() ? "OFF (NO DEVICE)" : "OFF (OPL2)";
}

const GameDef* frame_source_game(int src,
                                 const std::vector<GameDef>& games) {
    if (src <= 0 || src > static_cast<int>(games.size())) {
        return nullptr;
    }
    return &games[static_cast<size_t>(src - 1)];
}

bool apply_hud_frame(Engine& engine, const std::vector<GameDef>& games,
                     int src) {
    std::string err;
    if (!engine.set_hud_frame_game(frame_source_game(src, games), err)) {
        std::fprintf(stderr, "hud frame: %s\n", err.c_str());
        return false;
    }
    return true;
}

bool apply_theme_frame(Engine& engine, const std::vector<GameDef>& games,
                       int theme) {
    const GameDef* g = nullptr;
    if (theme >= 1 && theme <= static_cast<int>(games.size())) {
        g = &games[static_cast<size_t>(theme - 1)];
    }
    std::string err;
    if (!engine.set_classic_frame_game(g, err)) {
        std::fprintf(stderr, "menu theme frame: %s\n", err.c_str());
        return false;
    }
    return true;
}

bool apply_menu_frame(Engine& engine, const std::vector<GameDef>& games,
                      int src, int theme) {
    const int game_count = static_cast<int>(games.size());
    engine.set_menu_border_ext(src > game_count ? src - game_count - 1 : -1);
    const GameDef* g = nullptr;
    if (src < 0) {
        // BORDER = THEME follows the selected menu theme's game (plain/external
        // themes fall back to the running game's shape 7).
        if (theme >= 1 && theme <= game_count) {
            g = &games[static_cast<size_t>(theme - 1)];
        }
    } else {
        g = frame_source_game(src, games);
    }
    std::string err;
    if (!engine.set_menu_frame_game(g, err)) {
        std::fprintf(stderr, "menu frame: %s\n", err.c_str());
        return false;
    }
    return true;
}

std::vector<std::string> menu_items(const Menu& m, bool started,
                                    const VideoSettings& video,
                                    const Engine& engine,
                                    const Bindings& bindings,
                                    const std::vector<GameDef>& games,
                                    const MusicPlayer& audio,
                                    const ExternalAssets& assets) {
    switch (m.screen) {
        case MenuScreen::Main: {
            std::vector<std::string> v;
            if (started) {
                v.push_back("RESUME");
                v.push_back("SAVE GAME");
                v.push_back("RESTORE GAME");
                v.push_back("START GAME");
                v.push_back("HIGH SCORES");
                v.push_back("OPTIONS");
                v.push_back("QUIT TO MAIN MENU");
                v.push_back("QUIT");
            } else {
                v.push_back("START GAME");
                v.push_back("RESTORE GAME");
                v.push_back("HIGH SCORES");
                v.push_back("OPTIONS");
                v.push_back("QUIT");
            }
            return v;
        }
        case MenuScreen::Episodes: {
            std::vector<std::string> v;
            for (const GameDef& g : games) {
                v.push_back(upper_ascii(g.title.c_str()));
            }
            v.push_back("BACK");
            return v;
        }
        case MenuScreen::Options:
            return {"HUD", "VIDEO", "SOUND", "CHEATS", "CONTROLS", "MENUS",
                    "GAMEPLAY", "ASSETS", "BACK"};
        case MenuScreen::Video: {
            char size[64];
            if (video.resolution) {
                std::snprintf(size, sizeof(size), "WINDOW SIZE: AUTO (%dx%d)",
                              window_width(video), window_height(video));
            } else {
                std::snprintf(size, sizeof(size), "WINDOW SIZE: %dx (%dx%d)",
                              video.scale, window_width(video),
                              window_height(video));
            }
            return {std::string("WINDOW TYPE: ") +
                        kWindowModes[video.mode % 3],
                    size,
                    std::string("RESOLUTION: ") +
                        kResolutions[video.resolution % kResolutionCount].label,
                    std::string("HIGH FRAMERATE: ") +
                        high_fps_name(video.high_fps),
                    std::string("INTERPOLATE: ") +
                        (video.high_fps <= 0
                             ? "N/A (HIGH FRAMERATE OFF)"
                             : (video.interpolate ? "ON" : "OFF")),
                    std::string("MESSAGE BOX: ") +
                        message_box_name(engine.message_box()),
                    std::string("FILTER: ") + filter_name(video.filter),
                    std::string("STEREO 3D: ") + stereo_name(video.stereo),
                    "BACK"};
        }
        case MenuScreen::Audio:
            return {std::string("MUSIC: ") + (m.music_on ? "ON" : "OFF"),
                    std::string("MUSIC VOLUME: ") +
                        std::to_string(m.music_volume) + "%",
                    std::string("SFX: ") + (m.sfx_on ? "ON" : "OFF"),
                    std::string("SFX VOLUME: ") +
                        std::to_string(m.sfx_volume) + "%",
                    std::string("PC SPEAKER: ") +
                        (m.pc_speaker ? "ON" : "OFF"),
                    std::string("MIDI OUT: ") + midi_device_label(audio),
                    "NOISEMAKER",
                    "BACK"};
        case MenuScreen::Hud:
            return {std::string("STYLE: ") +
                        (engine.hud_classic() ? "CLASSIC" : "MODERN"),
                    std::string("BACKGROUND: ") +
                        hud_bg_name(engine.hud_bg()),
                    std::string("TEXT: ") + hud_text_name(engine.hud_text()),
                    std::string("HEALTH: ") +
                        hud_health_name(engine.hud_health()),
                    std::string("PORTRAIT: ") +
                        (engine.hud_portrait() ? "ON" : "OFF"),
                    std::string("PANEL: ") +
                        hud_panel_name(m.hud_panel),
                    std::string("FRAME: ") +
                        frame_source_name(m.hud_frame, games),
                    "BACK"};
        case MenuScreen::Menus:
            return {std::string("STYLE: ") +
                        (m.classic ? "CLASSIC" : "MODERN"),
                    std::string("BORDER: ") +
                        menu_border_name(m.menu_frame, games,
                                         engine.external_visuals()),
                    std::string("THEME: ") +
                        menu_theme_name(engine.menu_theme(), games,
                                        engine.external_visuals()),
                    std::string("BACKGROUND: ") +
                        (m.live_background ? "LIVE GAME" : "THEME"),
                    std::string("SPLASH: ") + splash_name(video.splash, games),
                    std::string("FONT: ") + font_name(m.menu_font),
                    "BACK"};
        case MenuScreen::Controls: {
            std::vector<std::string> v;
            for (int i = 0; i < ACT_COUNT; ++i) {
                v.push_back(std::string(kActionLabel[i]) + ": " +
                            binding_label(bindings, i));
            }
            v.push_back("RESET DEFAULTS");
            v.push_back("BACK");
            return v;
        }
        case MenuScreen::Cheats:
            return {std::string("INVINCIBLE: ") +
                        (engine.invincible() ? "ON" : "OFF"),
                    std::string("DEBUG FLY: ") +
                        (engine.debug_fly() ? "ON" : "OFF"),
                    std::string("SUPER JUMP: ") +
                        super_jump_name(engine.super_jump()),
                    "ADD KNIFE", "ADD KEY", "ADD GEM", "ADD BLADE",
                    "REFILL HEALTH",
                    "BECOME JILL", "BECOME FROG", "BECOME BIRD",
                    "BECOME SPIDER", "BECOME FISH", "LEVEL SELECT", "BACK"};
        case MenuScreen::SaveGame: {
            std::vector<std::string> v;
            const int slots = engine.save_slots();
            for (int i = 0; i < slots; ++i) {
                const std::string& label = engine.save_label(i);
                char line[80];
                std::snprintf(line, sizeof(line), "SLOT %d: %s", i + 1,
                              label.empty() ? "EMPTY" : label.c_str());
                v.push_back(line);
            }
            v.push_back("BACK");
            return v;
        }
        case MenuScreen::LoadGame: {
            std::vector<std::string> v;
            int idx = m.load_game_index;
            if (idx < 0 || idx >= static_cast<int>(games.size())) {
                idx = 0;
            }
            v.push_back(std::string("EPISODE: ") +
                        upper_ascii(games[static_cast<size_t>(idx)].title.c_str()));
            const int slots = static_cast<int>(m.load_labels.size());
            for (int i = 0; i < slots; ++i) {
                const std::string& label = m.load_labels[static_cast<size_t>(i)];
                char line[80];
                std::snprintf(line, sizeof(line), "SLOT %d: %s", i + 1,
                              label.empty() ? "EMPTY" : label.c_str());
                v.push_back(line);
            }
            v.push_back("BACK");
            return v;
        }
        case MenuScreen::HighScores: {
            std::vector<std::string> v;
            int idx = m.hi_game;
            if (idx < 0 || idx >= static_cast<int>(games.size())) {
                idx = 0;
            }
            v.push_back(std::string("EPISODE: ") +
                        upper_ascii(games[static_cast<size_t>(idx)].title.c_str()));
            if (m.hi_scores.entries.empty()) {
                v.push_back("NO SCORES YET");
            } else {
                for (size_t i = 0; i < m.hi_scores.entries.size(); ++i) {
                    const HiEntry& e = m.hi_scores.entries[i];
                    std::string name = e.name;
                    if (name.size() > 8) {
                        name.resize(8);
                    }
                    char line[80];
                    std::snprintf(line, sizeof(line), "%2d  %-8s %lu",
                                  static_cast<int>(i + 1), name.c_str(),
                                  static_cast<unsigned long>(e.score));
                    v.push_back(line);
                }
            }
            v.push_back("BACK");
            return v;
        }
        case MenuScreen::Enhancements:
            return {std::string("FIRE WHILE CROUCHED: ") +
                        (engine.fire_while_crouched() ? "ON" : "OFF"),
                    std::string("MAX KNIVES: ") +
                        (engine.max_knives() >= 5 ? "5" : "3"),
                    std::string("RETAIN ITEMS BETWEEN LEVELS: ") +
                        (engine.retain_items() ? "ON" : "OFF"),
                    std::string("FASTER DIALOG BOXES: ") +
                        (engine.fast_dialogs() ? "ON" : "OFF"),
                    std::string("AIR JUMP BUFFER: ") +
                        (engine.air_jump_buffer() ? "ON" : "OFF"),
                    std::string("MINIMAP: ") +
                        (engine.minimap() ? "ON" : "OFF"),
                    std::string("CENTER FISH SHOT: ") +
                        (engine.center_fish_shot() ? "ON" : "OFF"),
                    std::string("JILL LOOK: ") +
                        player_look_name(games, engine.player_look()),
                    std::string("JILL COLOUR: ") +
                        player_suit_name(engine.player_suit()),
                    std::string("TURTLE MODE: ") +
                        (engine.turtle() ? "ON" : "OFF"),
                    "BACK"};
        case MenuScreen::Noisemaker: {
            std::string set = "SET: ";
            if (!games.empty()) {
                const size_t i = static_cast<size_t>(
                    std::max(0, std::min(m.noise_set,
                                         static_cast<int>(games.size()) - 1)));
                set += upper_ascii(games[i].title.c_str());
            } else {
                set += "NO GAMES";
            }
            if (!m.noise_loaded) {
                set += "  (NO BANK)";
            }
            char line[96];
            const int slot = m.noise_sound & 0x3F;
            if (audio.sfx_has_sample(slot)) {
                std::snprintf(line, sizeof(line), "SOUND %02X: %s  %.2fS",
                              slot, sfx_sample_name(slot),
                              audio.sfx_sample_seconds(slot));
            } else {
                std::snprintf(line, sizeof(line), "SOUND %02X: %s  SILENT",
                              slot, sfx_sample_name(slot));
            }
            return {set, line, "PLAY", "BACK"};
        }
        case MenuScreen::Assets: {
            std::vector<std::string> v;
            std::string pack_label = "ORIGINAL ART";
            if (assets.active()) {
                pack_label = upper_ascii(assets.active()->name.c_str());
            } else if (assets.autoload()) {
                pack_label = "AUTOLOAD";
            }
            v.push_back(std::string("PACK: ") + pack_label);
            for (const PackInfo& p : assets.packs()) {
                const bool active =
                    assets.active() && assets.active()->id == p.id;
                v.push_back(std::string(active ? "* " : "  ") +
                            upper_ascii(p.name.c_str()) +
                            (p.is_zip ? " [ZIP]" : ""));
            }
            v.push_back("RESCAN FOLDER");
            v.push_back("BACK");
            return v;
        }
        default:
            return {};
    }
}

// Cached 320x200 backdrop for the menu themes.  The original front end shows
// each episode's intro board behind the text menu; the menu panel itself is
// framed with that board's own tiles (a solid tile for the border ring, its
// most common walkable tile for the interior).
struct ThemeTile {
    bool ok = false;
    int w = 0;
    int h = 0;
    std::vector<uint8_t> px;
};

struct ThemeBackdrop {
    int built = -1;
    std::vector<uint8_t> px;
    ThemeTile frame;
    ThemeTile fill;
};

// Start-up publisher/loading and Onesimus title cards.  The Jill loading card
// uses each episode's own shape-7 window; the Onesimus cards rebuild the ARK
// and title pages from the original strings with the game font.
struct SplashImage {
    int built = -1;
    std::vector<std::vector<uint8_t>> pages;
    uint8_t palette[768] = {};
    bool palette_ready = false;
};

std::string lower_ascii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return s;
}

bool text_has_word(const std::string& text, const std::string& word) {
    if (word.empty()) {
        return false;
    }
    return lower_ascii(text).find(lower_ascii(word)) != std::string::npos;
}

void splash_text_6(Framebuffer& fb, Engine& engine, int y, uint8_t colour,
                   const char* s) {
    const int w = static_cast<int>(std::strlen(s)) * 6;
    engine.draw_text(fb, (Framebuffer::W - w) / 2, y, colour, s);
}

void splash_text_8(Framebuffer& fb, Engine& engine, int y, uint8_t colour,
                   const char* s) {
    const int w = static_cast<int>(std::strlen(s)) * 8;
    engine.draw_text_8(fb, (Framebuffer::W - w) / 2, y, colour, s);
}

void splash_scaled_text(Framebuffer& fb, Engine& engine, int x, int y,
                        uint8_t colour, int scale, const char* s) {
    if (scale <= 1) {
        engine.draw_text(fb, x, y, colour, s);
        return;
    }
    Framebuffer tmp;
    engine.draw_text(tmp, 0, 0, colour, s);
    const int w = static_cast<int>(std::strlen(s)) * 6;
    for (int yy = 0; yy < 8; ++yy) {
        for (int xx = 0; xx < w; ++xx) {
            if (tmp.get(xx, yy) != 0) {
                fb.fill_rect(x + xx * scale, y + yy * scale, scale, scale,
                             colour);
            }
        }
    }
}

void splash_scaled_colour_text(Framebuffer& fb, Engine& engine, int x, int y,
                               int scale, const char* s,
                               const uint8_t* colours, int colour_count) {
    const int adv = 6 * scale;
    int ci = 0;
    for (int i = 0; s[i] != '\0'; ++i) {
        if (s[i] != ' ') {
            char one[2] = {s[i], '\0'};
            splash_scaled_text(fb, engine, x + i * adv, y,
                               colours[ci % colour_count], scale, one);
            ++ci;
        }
    }
}

std::string splash_game_title(const GameDef& g) {
    if (g.id == "jill1") {
        return "Jill of the Jungle";
    }
    if (g.id == "jill2") {
        return "Jill Goes Underground";
    }
    if (g.id == "jill3") {
        return "Jill Saves The Prince";
    }
    return upper_ascii(g.title.c_str());
}

void build_loading_splash(SplashImage& img, const GameDef& g, Engine& art) {
    Framebuffer fb;
    const int bx = 64;
    const int by = 60;
    const int bw = 192;
    const int bh = 72;
    art.draw_menu_window(fb, bx, by, bw, bh);
    splash_text_6(fb, art, by + 8, 15, "A NEW RELEASE FROM");
    splash_text_8(fb, art, by + 18, 15, "Epic MegaGames");
    const std::string title = splash_game_title(g);
    splash_text_8(fb, art, by + 32, 10, title.c_str());
    splash_text_6(fb, art, by + 47, 15, "PRODUCED BY");
    splash_text_6(fb, art, by + 55, 15, "TIM SWEENEY");
    splash_text_6(fb, art, 164, 9, "NOW LOADING, PLEASE WAIT...");
    img.pages.push_back(std::move(fb.px));
}

void build_onesimus_ark_splash(SplashImage& img, Engine& art) {
    Framebuffer fb;
    static const uint8_t ark_colours[3] = {12, 14, 10};
    const int ark_scale = 9;
    const char* ark = "A R K";
    const int ark_w = static_cast<int>(std::strlen(ark)) * 6 * ark_scale;
    splash_scaled_colour_text(fb, art, (Framebuffer::W - ark_w) / 2, 14,
                              ark_scale, ark, ark_colours, 3);
    const char* multi = "Multimedia Publishing";
    const int multi_scale = 2;
    const int multi_w =
        static_cast<int>(std::strlen(multi)) * 6 * multi_scale;
    splash_scaled_text(fb, art, (Framebuffer::W - multi_w) / 2, 98, 15,
                       multi_scale, multi);
    const char* presents = "Presents";
    const int pres_w =
        static_cast<int>(std::strlen(presents)) * 6 * multi_scale;
    splash_scaled_text(fb, art, (Framebuffer::W - pres_w) / 2, 120, 15,
                       multi_scale, presents);
    img.pages.push_back(std::move(fb.px));
}

void build_onesimus_title_splash(SplashImage& img, Engine& art) {
    Framebuffer fb;
    static const uint8_t title_colours[8] = {12, 6, 14, 10, 10, 9, 13, 12};
    const int title_scale = 5;
    const char* title = "ONESIMUS";
    const int title_w =
        static_cast<int>(std::strlen(title)) * 6 * title_scale;
    splash_scaled_colour_text(fb, art, (Framebuffer::W - title_w) / 2, 8,
                              title_scale, title, title_colours, 8);
    const char* quest = "A QUEST FOR";
    const int quest_scale = 3;
    const int quest_w = static_cast<int>(std::strlen(quest)) * 6 * quest_scale;
    splash_scaled_text(fb, art, (Framebuffer::W - quest_w) / 2, 54, 6,
                       quest_scale, quest);
    const char* freedom = "FREEDOM";
    const int free_w =
        static_cast<int>(std::strlen(freedom)) * 6 * quest_scale;
    splash_scaled_text(fb, art, (Framebuffer::W - free_w) / 2, 76, 6,
                       quest_scale, freedom);
    splash_text_8(fb, art, 112, 15, "Bible quotes from The Living Bible (c)");
    splash_text_8(fb, art, 126, 15, "1971 used with permission of Tyndale");
    splash_text_8(fb, art, 140, 15, "House Publishers, Inc., Wheaton IL");
    splash_text_8(fb, art, 154, 15, "60189.  All rights reserved.");
    img.pages.push_back(std::move(fb.px));
}

void ensure_splash_image(SplashImage& img, int index,
                         const std::vector<GameDef>& games) {
    if (img.built == index) {
        return;
    }
    img.built = index;
    img.pages.clear();
    img.palette_ready = false;
    if (index < 1 || index > static_cast<int>(games.size())) {
        return;
    }
    const GameDef& g = games[static_cast<size_t>(index - 1)];
    img.palette_ready = load_palette(g.palette, img.palette);
    Engine art;
    std::string err;
    if (!art.init(g, err)) {
        std::fprintf(stderr, "splash %s: %s\n", g.id.c_str(), err.c_str());
        return;
    }
    if (g.id == "onesimus") {
        build_onesimus_ark_splash(img, art);
        build_onesimus_title_splash(img, art);
    } else {
        build_loading_splash(img, g, art);
    }
}

void dump_splash_pages(const std::vector<GameDef>& games, int index,
                       const std::string& prefix) {
    SplashImage img;
    ensure_splash_image(img, index, games);
    uint8_t palette[768];
    if (img.palette_ready) {
        std::memcpy(palette, img.palette, sizeof(palette));
    } else {
        vga_default_palette(palette);
    }
    for (size_t i = 0; i < img.pages.size(); ++i) {
        const std::string path =
            prefix + std::to_string(i + 1) + std::string(".ppm");
        FILE* f = std::fopen(path.c_str(), "wb");
        if (!f) {
            std::fprintf(stderr, "cannot write %s\n", path.c_str());
            continue;
        }
        std::fprintf(f, "P6\n%d %d\n255\n", Framebuffer::W, Framebuffer::H);
        for (uint8_t c : img.pages[i]) {
            uint8_t rgb[3] = {palette[c * 3 + 0], palette[c * 3 + 1],
                              palette[c * 3 + 2]};
            std::fwrite(rgb, 1, 3, f);
        }
        std::fclose(f);
        std::printf("wrote %s\n", path.c_str());
    }
}

void ensure_theme_backdrop(ThemeBackdrop& tb, int theme,
                           const std::vector<GameDef>& games,
                           const ExternalVisuals* ext) {
    if (tb.built == theme) {
        return;
    }
    tb.built = theme;
    tb.px.clear();
    tb.frame = ThemeTile{};
    tb.fill = ThemeTile{};
    const int game_count = static_cast<int>(games.size());
    if (theme < 1 || theme > game_count) {
        // Pack themes are plain 320x200 pictures; no board frame tiles.
        const int ext_count =
            ext ? static_cast<int>(ext->themes().size()) : 0;
        if (theme > game_count && theme <= game_count + ext_count) {
            tb.px = ext->themes()[static_cast<size_t>(theme - game_count - 1)];
        }
        return;
    }
    const GameDef& g = games[static_cast<size_t>(theme - 1)];
    Engine tmp;
    std::string err;
    if (!tmp.init(g, err)) {
        return;
    }
    if (!tmp.load_board(g.intro_board, err)) {
        return;
    }
    tmp.finish_dialogs();  // the backdrop shows the whole intro page at once
    // Intro-board picture panel: tiles (36,0) to (55,13) inclusive, i.e. a
    // 20x14-cell (320x224 px) rectangle.  The TEXT6/TEXT8 objects inside it are
    // the story/credits copy, so they are hidden; the tiles alone make the
    // menu artwork.  The panel is exactly one screen wide, so only the vertical
    // centre crop is needed.
    constexpr int kPanelCol = 36;
    constexpr int kPanelRow = 0;
    constexpr int kPanelW = 20;
    constexpr int kPanelH = 14;
    const int panel_x = kPanelCol * 16;
    const int panel_y = kPanelRow * 16;
    const int cam_x = panel_x + (kPanelW * 16 - Framebuffer::W) / 2;
    const int cam_y = panel_y + (kPanelH * 16 - Framebuffer::H) / 2;
    tmp.set_hide_text_kinds(true);
    Framebuffer fb;
    tmp.render_at(fb, cam_x, cam_y, false);
    tb.px = std::move(fb.px);

    tb.frame.ok = tmp.tile_pixels(tmp.theme_tile(true), tb.frame.px,
                                  tb.frame.w, tb.frame.h);
    tb.fill.ok = tmp.tile_pixels(tmp.theme_tile(false), tb.fill.px, tb.fill.w,
                                 tb.fill.h);
    if (!tb.fill.ok) {
        tb.fill = tb.frame;
    }
}

void menu_title(Framebuffer& fb, Engine& engine, int y, const char* s) {
    int w = static_cast<int>(std::strlen(s)) * 6;
    engine.draw_text(fb, (Framebuffer::W - w) / 2, y, 7, s);
}

void draw_classic_hiscores(Framebuffer& fb, Engine& engine,
                           const HiScores& hi) {
    const int x = 8;
    const int y = 32;
    const int w = 64;
    const int h = 92;
    fb.fill_rect(x, y, w, h, 4);
    fb.fill_rect(x, y, w, 1, 12);
    fb.fill_rect(x, y + h - 1, w, 1, 12);
    fb.fill_rect(x, y, 1, h, 12);
    fb.fill_rect(x + w - 1, y, 1, h, 12);
    engine.draw_text(fb, x + 4, y + 4, 12, "HI SCORES");
    if (hi.entries.empty()) {
        engine.draw_text(fb, x + 4, y + 18, 14, "NO SCORES");
        return;
    }
    for (size_t i = 0; i < hi.entries.size() && i < 10; ++i) {
        std::string name = hi.entries[i].name;
        if (name.size() > 5) {
            name.resize(5);
        }
        char score[16];
        std::snprintf(score, sizeof(score), "%lu",
                      static_cast<unsigned long>(hi.entries[i].score));
        const int row = y + 17 + static_cast<int>(i) * 7;
        engine.draw_text(fb, x + 3, row, 14, name.c_str());
        const int sw = static_cast<int>(std::strlen(score)) * 6;
        engine.draw_text(fb, x + w - 3 - sw, row, 7, score);
    }
}

void draw_classic_menu(Framebuffer& fb, Engine& engine, const Menu& m,
                       const char* sub,
                       const std::vector<std::string>& items, int sel,
                       bool in_game) {
    if (!in_game) {
        engine.draw_classic_backdrop(fb, engine.game().title, true);
        if (m.screen != MenuScreen::HighScores) {
            draw_classic_hiscores(fb, engine, m.hi_scores);
        }
    }

    int maxlen = 0;
    for (const std::string& item : items) {
        maxlen = std::max(maxlen, static_cast<int>(item.size()));
    }
    const int line_h = items.size() > 12 ? 9 : 10;
    int boxw = 32 + (maxlen + 2) * 6;
    boxw = std::max(168, std::min(boxw, 232));
    boxw = ((boxw + 7) / 8) * 8;
    int boxh = 20 + static_cast<int>(items.size()) * line_h + 8;
    boxh = std::min(boxh, 162);
    boxh = ((boxh + 7) / 8) * 8;

    int bx = std::max(76, std::min(80, 312 - boxw));
    int by = (Framebuffer::H - boxh) / 2;
    by = std::max(28, std::min(by, 190 - boxh));
    engine.draw_menu_window(fb, bx, by, boxw, boxh);

    const char* header =
        (m.screen == MenuScreen::Main) ? "PICK A CHOICE:" : sub;
    engine.draw_text(fb, bx + 12, by + 10, 15, header);

    int y = by + 20;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        char line[96];
        std::snprintf(line, sizeof(line), "%c %s",
                      i == sel ? '>' : ' ',
                      items[static_cast<size_t>(i)].c_str());
        uint8_t colour = 10;  // classic green
        if (i == sel) {
            colour = 15;
        } else if (items[static_cast<size_t>(i)].find("QUIT") !=
                       std::string::npos ||
                   items[static_cast<size_t>(i)].find("BACK") !=
                       std::string::npos) {
            colour = 12;
        }
        engine.draw_text(fb, bx + 10, y, colour, line);
        y += line_h;
    }

    const char* hint = "UP/DOWN MOVE   ENTER SELECT   ESC BACK";
    if (m.rebind_action >= 0) {
        hint = "PRESS A KEY OR PAD BUTTON   ESC CANCEL";
    } else if (m.screen == MenuScreen::Noisemaker) {
        hint = "LEFT/RIGHT CHANGE   ENTER PLAY   ESC BACK";
    } else if (m.screen == MenuScreen::LoadGame ||
               m.screen == MenuScreen::HighScores) {
        hint = "LEFT/RIGHT EPISODE   UP/DOWN MOVE   ESC BACK";
    } else if (m.screen == MenuScreen::Assets) {
        hint = "UP/DOWN MOVE   ENTER MOUNT   ESC BACK";
    }
    const int w = static_cast<int>(std::strlen(hint)) * 6;
    const int footer_y = (by + boxh + 2 <= 188) ? 188 : 192;
    engine.draw_text(fb, (Framebuffer::W - w) / 2, footer_y, 7, hint);
}

void draw_menu(Framebuffer& fb, Engine& engine, Menu& m, bool started,
               const VideoSettings& video, const Bindings& bindings,
               ThemeBackdrop& backdrop, const std::vector<GameDef>& games,
               const MusicPlayer& audio, const ExternalAssets& assets) {
    const int current_game = game_index_for_id(games, engine.game().id);
    if (m.screen != MenuScreen::HighScores && current_game >= 0) {
        ensure_hiscores(m, games, current_game);
    }
    // Keep the live game screen behind an in-game menu when requested (or when
    // using the classic menu, which is an overlay).  A front-end menu still
    // uses its theme backdrop.
    const bool keep_game = started && (m.live_background || m.classic);
    if (!keep_game) {
        ensure_theme_backdrop(backdrop, engine.menu_theme(), games,
                              engine.external_visuals());
        if (backdrop.px.empty()) {
            fb.fill_rect(0, 0, Framebuffer::W, Framebuffer::H, 0);
        } else {
            // 50% checkerboard dim so the menu text stays readable on top of
            // the intro-board artwork.
            for (int y = 0; y < Framebuffer::H; ++y) {
                for (int x = 0; x < Framebuffer::W; ++x) {
                    size_t i = static_cast<size_t>(y) * Framebuffer::W + x;
                    if (((x + y) & 1) != 0) {
                        fb.px[i] = backdrop.px[i];
                    } else {
                        fb.px[i] = 0;
                    }
                }
            }
        }
    }
    const char* sub = "MAIN MENU";
    switch (m.screen) {
        case MenuScreen::Episodes: sub = "SELECT EPISODE"; break;
        case MenuScreen::Options: sub = "OPTIONS"; break;
        case MenuScreen::Hud: sub = "HUD"; break;
        case MenuScreen::Video: sub = "VIDEO"; break;
        case MenuScreen::Audio: sub = "SOUND"; break;
        case MenuScreen::Menus: sub = "MENUS"; break;
        case MenuScreen::Controls: sub = "CONTROLS"; break;
        case MenuScreen::Cheats: sub = "CHEATS"; break;
        case MenuScreen::Enhancements: sub = "GAMEPLAY"; break;
        case MenuScreen::SaveGame: sub = "SAVE GAME"; break;
        case MenuScreen::LoadGame: sub = "RESTORE GAME"; break;
        case MenuScreen::HighScores: sub = "HI SCORES"; break;
        case MenuScreen::Noisemaker: sub = "NOISEMAKER"; break;
        case MenuScreen::Assets: sub = "EXTERNAL ASSETS"; break;
        default: break;
    }

    std::vector<std::string> items =
        menu_items(m, started, video, engine, bindings, games, audio, assets);
    int sel = menu_sel(m);
    if (sel < 0 || sel >= static_cast<int>(items.size())) {
        sel = 0;
    }
    if (m.classic) {
        draw_classic_menu(fb, engine, m, sub, items, sel, started);
        return;
    }
    menu_title(fb, engine, 16,
               upper_ascii(engine.game().title.c_str()).c_str());
    menu_title(fb, engine, 28, sub);
    int maxlen = 0;
    for (const std::string& item : items) {
        maxlen = std::max(maxlen, static_cast<int>(item.size()));
    }
    int boxw = std::max(200, 34 + (maxlen + 2) * 6);
    boxw = std::min(boxw, Framebuffer::W - 12);
    // 10 px rows (2 px gap); the 14-row CHEATS screen uses 9 px so it fits.
    const int line_h = items.size() > 12 ? 9 : 10;
    int boxh = 8 + static_cast<int>(items.size()) * line_h + 6;
    // BORDER = THEME (-1) keeps the existing theme-tile border.  Any explicit
    // game source uses that game's shape-7 nine-slice, independently of THEME.
    const bool shape_border = m.menu_frame >= 0;
    if (shape_border) {
        boxw = ((boxw + 7) / 8) * 8;
        boxh = ((boxh + 7) / 8) * 8;
    }
    int bx = (Framebuffer::W - boxw) / 2;
    int by = 42;

    // Themed menu frame: a ring of the theme board's blocking tile around the
    // panel and its walkable tile inside, both dimmed 50% so the text reads.
    const bool tiled = !shape_border && !keep_game &&
                       engine.menu_theme() > 0 && backdrop.frame.ok;
    const int ring = 8;
    if (shape_border) {
        engine.draw_menu_window(fb, bx, by, boxw, boxh);
    } else if (tiled) {
        auto blit_tile = [&](const ThemeTile& t, int x0, int y0, int x1,
                             int y1, bool dim) {
            if (!t.ok || t.w <= 0 || t.h <= 0) {
                return;
            }
            for (int y = y0; y < y1; ++y) {
                for (int x = x0; x < x1; ++x) {
                    if (dim && ((x + y) & 1) == 0) {
                        continue;
                    }
                    uint8_t c = t.px[static_cast<size_t>((y - y0) % t.h) * t.w +
                                   (x - x0) % t.w];
                    fb.put(x, y, c);
                }
            }
        };
        // Interior first, then the ring around the outside of the panel.
        blit_tile(backdrop.fill, bx, by, bx + boxw, by + boxh, true);
        blit_tile(backdrop.frame, bx - ring, by - ring, bx + boxw + ring,
                  by, false);
        blit_tile(backdrop.frame, bx - ring, by + boxh, bx + boxw + ring,
                  by + boxh + ring, false);
        blit_tile(backdrop.frame, bx - ring, by, bx, by + boxh, false);
        blit_tile(backdrop.frame, bx + boxw, by, bx + boxw + ring, by + boxh,
                  false);
        fb.fill_rect(bx, by, boxw, 1, 0);
        fb.fill_rect(bx, by + boxh - 1, boxw, 1, 0);
    } else {
        fb.fill_rect(bx, by, boxw, boxh, 8);
        fb.fill_rect(bx + 1, by + 1, boxw - 2, boxh - 2, 0);
    }
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        char line[96];
        std::snprintf(line, sizeof(line), "%c %s", i == sel ? '>' : ' ',
                      items[static_cast<size_t>(i)].c_str());
        uint8_t colour = (i == sel && m.rebind_action >= 0) ? 6
                          : (i == sel ? 7 : 5);
        engine.draw_text(fb, bx + 14, by + 5 + i * line_h, colour, line);
    }
    if (m.screen == MenuScreen::Hud) {
        // Live background preview: the bar uses the chosen palette index.
        int py = 150;
        fb.fill_rect(10, py, Framebuffer::W - 20, 12,
                     static_cast<uint8_t>(engine.hud_bg_colour()));
        engine.draw_text(fb, 14, py + 3,
                         static_cast<uint8_t>(engine.hud_text()),
                         "LEVEL 1  [2] 3  HEALTH  SCORE 0");
        // Health colour swatch: the label itself is drawn in that colour, or
        // in the text colour while the original pip art is selected.
        const int hc = engine.hud_health() < 0 ? engine.hud_text()
                                               : engine.hud_health();
        engine.draw_text(fb, 14, py + 16, static_cast<uint8_t>(hc),
                         "HEALTH COLOUR");
    }
    const int footer_y = std::min(Framebuffer::H - 12, by + boxh + 2);
    if (m.rebind_action >= 0) {
        menu_title(fb, engine, footer_y, "PRESS A KEY OR PAD BUTTON");
        menu_title(fb, engine, footer_y + 9, "ESC CANCELS");
    } else if (m.screen == MenuScreen::Noisemaker) {
        menu_title(fb, engine, footer_y,
                   "LEFT/RIGHT CHANGE   ENTER PLAY   ESC BACK");
    } else if (m.screen == MenuScreen::LoadGame ||
               m.screen == MenuScreen::HighScores) {
        menu_title(fb, engine, footer_y,
                   "LEFT/RIGHT EPISODE   UP/DOWN MOVE   ESC BACK");
    } else if (m.screen == MenuScreen::Assets) {
        menu_title(fb, engine, footer_y,
                   "UP/DOWN MOVE   ENTER MOUNT   ESC BACK");
    } else {
        menu_title(fb, engine, footer_y, "UP/DOWN MOVE   ENTER SELECT   ESC BACK");
    }
}

// Jill Select: find the chosen game's player colour table.  Look 0 keeps the
// running episode; 1..N indexes the installed game list.
std::string player_look_name(const std::vector<GameDef>& games, int look) {
    if (look <= 0 || look > static_cast<int>(games.size())) {
        return "GAME DEFAULT";
    }
    return upper_ascii(games[static_cast<size_t>(look - 1)].title.c_str());
}

std::string player_suit_name(int suit) {
    static const char* const kNames[9] = {
        "DEFAULT", "GREEN", "RED", "BLUE", "YELLOW",
        "ORANGE", "CYAN", "PURPLE", "SILVER"};
    if (suit < 0 || suit > 8) {
        return kNames[0];
    }
    return kNames[suit];
}

std::string path_join(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    const char last = dir.back();
    return dir + ((last == '/' || last == '\\') ? "" : "/") + name;
}

void apply_player_look(Engine& engine, const std::vector<GameDef>& games,
                       int look, int suit) {
    std::string path;
    if (look >= 1 && look <= static_cast<int>(games.size())) {
        const GameDef& g = games[static_cast<size_t>(look - 1)];
        path = path_join(g.data_dir, g.shape_file);
    }
    if (!engine.set_player_look(look, path)) {
        engine.set_player_look(0, std::string());
    }
    engine.set_player_suit(suit);
}

// --- ending cards ---------------------------------------------------------
// Ending boards (50.jn1 / 20.jn2 / 14.jn3) keep their text in ordinary
// TEXT6/TEXT8 objects, grouped around CHECKPT objects whose p13 is 200+.  The
// original scrolls the viewport through those groups; the port presents them
// as full-screen cards and then returns to the front end.
struct EndingCard {
    int x = 0;
    int y = 0;
};

struct EndingViewer {
    bool active = false;
    std::vector<EndingCard> cards;
    int index = 0;
    uint32_t started = 0;
};

struct ScoreEntryPrompt {
    bool active = false;
    bool to_menu = true;
    std::string game_id;
    std::string name;
    uint32_t score = 0;
};

bool begin_hiscore_entry(ScoreEntryPrompt& p, HiScores& hi,
                         const std::string& game_id, uint32_t score,
                         bool to_menu) {
    if (!hiscore_qualifies(hi, game_id, score)) {
        return false;
    }
    p.active = true;
    p.to_menu = to_menu;
    p.game_id = game_id;
    p.name.clear();
    p.score = score;
    return true;
}

void commit_hiscore_entry(ScoreEntryPrompt& p, HiScores& hi) {
    if (!p.active) {
        return;
    }
    add_hiscore(hi, p.game_id, p.score, p.name);
    p.active = false;
}

void draw_hiscore_entry(Framebuffer& fb, Engine& engine,
                        const ScoreEntryPrompt& p) {
    if (!p.active) {
        return;
    }
    const int x = 46;
    const int y = 66;
    const int w = 228;
    const int h = 68;
    fb.fill_rect(x, y, w, h, 8);
    fb.fill_rect(x + 1, y + 1, w - 2, h - 2, 0);
    engine.draw_text(fb, x + 10, y + 8, 15, "NEW HIGH SCORE");
    char line[96];
    std::snprintf(line, sizeof(line), "SCORE: %lu",
                  static_cast<unsigned long>(p.score));
    engine.draw_text(fb, x + 10, y + 22, 14, line);
    std::string shown = p.name.empty() ? std::string("_") : (p.name + "_");
    engine.draw_text(fb, x + 10, y + 36, 7, "NAME:");
    engine.draw_text(fb, x + 58, y + 36, 15, shown.c_str());
    engine.draw_text(fb, x + 10, y + 52, 11, "ENTER ACCEPT   ESC = JILL");
}

int clamp_int(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

int ending_text_width(const BoardObject& o) {
    const int cw = o.kind == 21 ? 8 : 6;
    return static_cast<int>(o.text.size()) * cw;
}

int ending_text_height(const BoardObject& o) {
    return o.kind == 21 ? 8 : 7;
}

EndingCard ending_camera(int minx, int miny, int maxx, int maxy) {
    EndingCard c;
    c.x = clamp_int((minx + maxx) / 2 - Framebuffer::W / 2, 0,
                    Board::kCols * 16 - Framebuffer::W);
    c.y = clamp_int((miny + maxy) / 2 - Framebuffer::H / 2, 0,
                    Board::kRows * 16 - Framebuffer::H);
    return c;
}

std::vector<EndingCard> build_ending_cards(const Engine& engine) {
    std::vector<const BoardObject*> texts;
    std::vector<const BoardObject*> anchors;
    for (const BoardObject& o : engine.objects()) {
        if ((o.kind == 20 || o.kind == 21) && !o.text.empty()) {
            texts.push_back(&o);
        } else if (o.kind == 12 && o.p13 >= 200) {
            anchors.push_back(&o);
        }
    }
    std::sort(anchors.begin(), anchors.end(),
              [](const BoardObject* a, const BoardObject* b) {
                  return a->p13 < b->p13;
              });
    std::vector<bool> used(texts.size(), false);
    std::vector<EndingCard> cards;
    // Use the checkpoint anchors as screen positions, assigning the text
    // groups that sit in each anchor's 320x200 window.
    for (const BoardObject* a : anchors) {
        int minx = 0x7FFF, miny = 0x7FFF, maxx = -0x7FFF, maxy = -0x7FFF;
        bool any = false;
        for (size_t i = 0; i < texts.size(); ++i) {
            if (used[i]) {
                continue;
            }
            const BoardObject& o = *texts[i];
            const int tw = ending_text_width(o);
            const int th = ending_text_height(o);
            if (o.x + tw < a->x - 64 || o.x > a->x + 320 ||
                o.y + th < a->y - 64 || o.y > a->y + 220) {
                continue;
            }
            used[i] = true;
            any = true;
            minx = std::min(minx, o.x);
            miny = std::min(miny, o.y);
            maxx = std::max(maxx, o.x + tw);
            maxy = std::max(maxy, o.y + th);
        }
        if (!any) {
            continue;
        }
        EndingCard c;
        c.x = clamp_int(a->x, 0, Board::kCols * 16 - Framebuffer::W);
        c.y = clamp_int(a->y, 0, Board::kRows * 16 - Framebuffer::H);
        if (minx < c.x) {
            c.x = clamp_int(minx, 0, Board::kCols * 16 - Framebuffer::W);
        } else if (maxx > c.x + Framebuffer::W) {
            c.x = clamp_int(maxx - Framebuffer::W, 0,
                            Board::kCols * 16 - Framebuffer::W);
        }
        if (miny < c.y) {
            c.y = clamp_int(miny, 0, Board::kRows * 16 - Framebuffer::H);
        } else if (maxy > c.y + Framebuffer::H) {
            c.y = clamp_int(maxy - Framebuffer::H, 0,
                            Board::kRows * 16 - Framebuffer::H);
        }
        cards.push_back(c);
    }
    // Mods/endings without p13>=200 anchors: cluster the text objects by
    // proximity and centre each cluster instead.
    if (cards.empty()) {
        for (size_t i = 0; i < texts.size(); ++i) {
            if (used[i]) {
                continue;
            }
            used[i] = true;
            const BoardObject& first = *texts[i];
            int minx = first.x;
            int miny = first.y;
            int maxx = first.x + ending_text_width(first);
            int maxy = first.y + ending_text_height(first);
            bool changed = true;
            while (changed) {
                changed = false;
                for (size_t j = 0; j < texts.size(); ++j) {
                    if (used[j]) {
                        continue;
                    }
                    const BoardObject& o = *texts[j];
                    const int tw = ending_text_width(o);
                    const int th = ending_text_height(o);
                    if (o.x + tw < minx - 48 || o.x > maxx + 48 ||
                        o.y + th < miny - 24 || o.y > maxy + 24) {
                        continue;
                    }
                    used[j] = true;
                    changed = true;
                    minx = std::min(minx, o.x);
                    miny = std::min(miny, o.y);
                    maxx = std::max(maxx, o.x + tw);
                    maxy = std::max(maxy, o.y + th);
                }
            }
            cards.push_back(ending_camera(minx, miny, maxx, maxy));
        }
    }
    return cards;
}

bool start_ending_viewer(Engine& engine, EndingViewer& viewer) {
    viewer.cards = build_ending_cards(engine);
    viewer.index = 0;
    viewer.started = SDL_GetTicks();
    viewer.active = !viewer.cards.empty();
    if (viewer.active) {
        engine.finish_dialogs();  // show every line of the card at once
    }
    return viewer.active;
}

// Returns true when the last card has been passed.
bool advance_ending_viewer(EndingViewer& viewer) {
    if (!viewer.active) {
        return true;
    }
    ++viewer.index;
    viewer.started = SDL_GetTicks();
    if (viewer.index >= static_cast<int>(viewer.cards.size())) {
        viewer.active = false;
        viewer.index = 0;
        return true;
    }
    return false;
}

void draw_ending_viewer(Framebuffer& fb, Engine& engine,
                        const EndingViewer& viewer) {
    if (!viewer.active || viewer.index < 0 ||
        viewer.index >= static_cast<int>(viewer.cards.size())) {
        return;
    }
    const EndingCard& c = viewer.cards[static_cast<size_t>(viewer.index)];
    engine.render_at_fullscreen(fb, c.x, c.y);
    fb.fill_rect(0, 188, Framebuffer::W, 12, 0);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d/%d  ENTER NEXT  ESC SKIP",
                  viewer.index + 1, static_cast<int>(viewer.cards.size()));
    engine.draw_text(fb, 4, 190, 7, buf);
}

// Start a fresh game on a game's first board, keeping the current
// display/engine options.
bool start_game(Engine& engine, const std::vector<GameDef>& games, int index,
                int hud_frame, int menu_frame) {
    if (index < 0 || index >= static_cast<int>(games.size())) {
        return false;
    }
    const GameDef& g = games[static_cast<size_t>(index)];
    const bool hud = engine.hud_classic();
    const bool hud_portrait = engine.hud_portrait();
    const int hud_bg = engine.hud_bg();
    const int hud_text = engine.hud_text();
    const int hud_health = engine.hud_health();
    const bool fly = engine.debug_fly();
    const bool inv = engine.invincible();
    const int super_jump = engine.super_jump();
    const int menu_theme = engine.menu_theme();
    const bool crouch_fire = engine.fire_while_crouched();
    const int knives = engine.max_knives();
    const bool retain = engine.retain_items();
    const bool fast_dialogs = engine.fast_dialogs();
    const int message_box = engine.message_box();
    const bool air_jump = engine.air_jump_buffer();
    const bool minimap = engine.minimap();
    const bool center_fish = engine.center_fish_shot();
    const int player_look = engine.player_look();
    const int player_suit = engine.player_suit();
    const std::string hud_panel = engine.hud_panel();
    const std::string font_name = engine.font_name();
    const int menu_border_ext = engine.menu_border_ext();
    const bool turtle = engine.turtle();
    std::string err;
    if (engine.game().id != g.id) {
        Engine fresh;
        if (!fresh.init(g, err)) {
            std::fprintf(stderr, "%s: %s\n", g.id.c_str(), err.c_str());
            return false;
        }
        fresh.adopt_external_state(engine);
        engine = std::move(fresh);
    }
    engine.set_hud_panel(hud_panel);
    engine.set_font_name(font_name);
    engine.set_menu_border_ext(menu_border_ext);
    engine.set_turtle(turtle);
    engine.set_fast_dialogs(fast_dialogs);
    engine.set_message_box(message_box);
    engine.set_hud_classic(hud);  // camera centre depends on the HUD style
    if (!engine.load_board(g.first_board, err)) {
        std::fprintf(stderr, "load %s: %s\n", g.first_board.c_str(),
                     err.c_str());
        return false;
    }
    engine.set_hud_classic(hud);
    engine.set_hud_portrait(hud_portrait);
    engine.set_hud_bg(hud_bg);
    engine.set_hud_text(hud_text);
    engine.set_hud_health(hud_health);
    engine.set_debug_fly(fly);
    engine.set_invincible(inv);
    engine.set_super_jump(super_jump);
    engine.set_menu_theme(menu_theme);
    engine.set_fire_while_crouched(crouch_fire);
    engine.set_max_knives(knives);
    engine.set_retain_items(retain);
    engine.set_fast_dialogs(fast_dialogs);
    engine.set_message_box(message_box);
    engine.set_air_jump_buffer(air_jump);
    engine.set_minimap(minimap);
    engine.set_center_fish_shot(center_fish);
    apply_player_look(engine, games, player_look, player_suit);
    apply_hud_frame(engine, games, hud_frame);
    apply_theme_frame(engine, games, menu_theme);
    apply_menu_frame(engine, games, menu_frame, menu_theme);
    return true;
}

int run_sdl(const Options& opt, Engine& engine,
            const std::vector<GameDef>& games) {
    // Let SDL drive Xbox pads through XInput and legacy pads through the
    // DirectInput path (both hints default to on; set them explicitly).
    SDL_SetHint(SDL_HINT_XINPUT_ENABLED, "1");
    SDL_SetHint(SDL_HINT_DIRECTINPUT_ENABLED, "1");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER |
                 SDL_INIT_JOYSTICK) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    std::atexit(SDL_Quit);

    VideoSettings video;
    video.mode = opt.fullscreen ? 1 : opt.window_mode;
    video.scale = opt.scale;
    video.resolution = opt.resolution;
    video.high_fps = opt.high_fps;
    video.splash = opt.splash;
    video.interpolate = opt.interpolate;
    video.filter = opt.filter;
    video.stereo = opt.stereo == 1 ? 1 : 0;
    video.stereo_depth = opt.stereo_depth;
    video.stereo_convergence = opt.stereo_convergence;

    SDL_Window* window = SDL_CreateWindow(
        "Jill (port skeleton)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        window_width(video), window_height(video), SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(renderer, Framebuffer::W, Framebuffer::H);
    SDL_RenderSetIntegerScale(renderer, SDL_TRUE);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             Framebuffer::W, Framebuffer::H);
    if (!texture) {
        std::fprintf(stderr, "SDL_CreateTexture: %s\n", SDL_GetError());
        return 1;
    }

    uint8_t palette[768];
    if (!load_palette(opt.palette.empty() ? engine.game().palette : opt.palette,
                      palette)) {
        return 1;
    }
    std::vector<uint32_t> argb(static_cast<size_t>(Framebuffer::W) * Framebuffer::H);

    // External pack root (`<base>/external` by default).  Missing folders are
    // created so pack authors have somewhere to drop files.
    ExternalAssets ext_assets;
    {
        std::string ext_err;
        if (!ext_assets.scan(opt.external_dir, ext_err)) {
            std::fprintf(stderr, "%s\n", ext_err.c_str());
        }
    }

    Framebuffer fb;
    Framebuffer stereo_left;
    Framebuffer stereo_right;
    ThemeBackdrop theme_backdrop;
    SplashImage splash_image;
    LevelSelect select;
    EndingViewer ending;
    ScoreEntryPrompt score_entry;
    Menu menu;
    menu.classic = opt.classic_menu;
    menu.live_background = opt.live_menu_bg;
    menu.hud_frame = opt.hud_frame;
    menu.hud_panel = opt.hud_panel;
    menu.menu_frame = opt.menu_frame;
    menu.menu_font = opt.menu_font;
    engine.set_hud_panel(menu.hud_panel);
    engine.set_font_name(menu.menu_font);
    engine.set_turtle(opt.turtle);
    menu.music_on = opt.music;
    menu.music_volume = std::max(0, std::min(100, opt.music_volume));
    menu.sfx_on = opt.sfx;
    menu.sfx_volume = std::max(0, std::min(100, opt.sfx_volume));
    menu.pc_speaker = opt.pc_speaker;
    MusicPlayer music_player;
    music_player.set_enabled(menu.music_on);
    music_player.set_volume(menu.music_volume / 100.0f);
    music_player.set_sfx_volume(menu.sfx_on ? menu.sfx_volume / 100.0f : 0.0f);
    music_player.set_pc_speaker(menu.pc_speaker);
    {
        std::string audio_err;
        if (!music_player.open(audio_err)) {
            std::fprintf(stderr, "%s\n", audio_err.c_str());
        }
    }
    if (opt.midi_device >= 0) {
        std::string midi_err;
        if (!music_player.set_midi_device(opt.midi_device, midi_err)) {
            std::fprintf(stderr, "midi: %s\n", midi_err.c_str());
        }
    }
    if (!engine.game().pages_file.empty()) {
        std::string sfx_err;
        if (!music_player.load_sfx(
                track_path(engine.game(), engine.game().pages_file),
                sfx_err)) {
            std::fprintf(stderr, "%s\n", sfx_err.c_str());
        }
    }
    std::string music_game = engine.game().id;
    // External pack audio: a `music/<name>.<ext>` file replaces the matching
    // CMF track, and `sfx/<slot>.<ext>` replaces an individual effect.
    auto decode_pack_audio = [&](const std::string& rel,
                                 std::shared_ptr<const DecodedAudio>& out) {
        std::vector<uint8_t> data;
        if (!ext_assets.read(rel, data)) {
            return false;
        }
        const size_t dot = rel.find_last_of('.');
        const std::string ext =
            dot == std::string::npos ? std::string() : rel.substr(dot + 1);
        auto audio = std::make_shared<DecodedAudio>();
        std::string derr;
        if (!decode_audio_memory(data.data(), data.size(), ext, *audio, derr)) {
            std::fprintf(stderr, "external: %s: %s\n", rel.c_str(), derr.c_str());
            return false;
        }
        out = std::move(audio);
        return true;
    };
    auto play_music = [&](const std::string& name) {
        if (!name.empty()) {
            std::string rel;
            std::shared_ptr<const DecodedAudio> audio;
            if (ext_assets.mounted() && ext_assets.find_music(name, rel) &&
                decode_pack_audio(rel, audio)) {
                music_player.play_pcm(audio);
                return;
            }
        }
        if (name.empty()) {
            music_player.stop();
            return;
        }
        std::string merr;
        auto song = load_track(engine.game(), name, merr);
        if (song) {
            music_player.play(song);
        } else {
            std::fprintf(stderr, "%s\n", merr.c_str());
        }
    };
    if (!engine.game().title_music.empty()) {
        play_music(engine.game().title_music);
    }
    if (!opt.board.empty()) {
        menu.screen = MenuScreen::Closed;  // explicit --board run: skip the menu
    }
    bool game_started = !opt.board.empty();
    auto finish_ending = [&]() {
        ending.active = false;
        ending.index = 0;
        if (game_started &&
            begin_hiscore_entry(score_entry, menu.hi_scores,
                                engine.game().id,
                                static_cast<uint32_t>(engine.score()), true)) {
            menu.screen = MenuScreen::Closed;
            return;
        }
        menu.screen = MenuScreen::Main;
        menu.main_sel = 0;
        game_started = false;
    };
    bool running = true;
    int page_axis_x = 0;  // text-page stick edge latches
    int page_axis_y = 0;
    // -1 = auto: show the splash for the game being started.  0 disables it.
    int splash_index = video.splash;
    if (splash_index < 0) {
        splash_index = game_index_for_id(games, engine.game().id) + 1;
    }
    splash_index =
        std::max(0, std::min(splash_index, static_cast<int>(games.size())));
    bool splash_active = false;
    size_t splash_page = 0;
    Uint32 splash_page_start = 0;
    if (splash_index > 0) {
        ensure_splash_image(splash_image, splash_index, games);
        if (!splash_image.pages.empty()) {
            splash_active = true;
            splash_page = 0;
            splash_page_start = SDL_GetTicks();
        }
    }
    if (opt.text_page > 0) {
        engine.show_text_page(opt.text_page);
    }

    // Apply the current WINDOW TYPE / WINDOW SIZE / RESOLUTION triple.
    auto apply_video = [&]() {
        if (video.mode == 2) {  // borderless fullscreen
            SDL_SetWindowBordered(window, SDL_FALSE);
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            return;
        }
        SDL_SetWindowBordered(window, SDL_TRUE);
        if (video.mode == 1) {  // exclusive fullscreen at the chosen mode
            if (video.resolution) {
                SDL_DisplayMode want{};
                want.w = kResolutions[video.resolution].w;
                want.h = kResolutions[video.resolution].h;
                want.format = SDL_PIXELFORMAT_UNKNOWN;
                want.refresh_rate = 0;
                SDL_DisplayMode closest{};
                if (SDL_GetClosestDisplayMode(0, &want, &closest)) {
                    SDL_SetWindowDisplayMode(window, &closest);
                }
            }
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
            return;
        }
        // Windowed: leave fullscreen, then size to the resolution or scale.
        SDL_SetWindowFullscreen(window, 0);
        SDL_SetWindowSize(window, window_width(video), window_height(video));
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED);
    };
    apply_video();
    const double step = 1.0 / 18.2065;  // one BIOS tick per update (18.2 fps)
    double accumulator = 0.0;
    uint64_t last = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());
    uint32_t title_timer = SDL_GetTicks();

    // Bindings: `[controls]` is the base; `[controls.<game id>]` overrides it
    // for one installed game.  Editing in the Controls menu changes the active
    // game's set and is written back to its section.
    Bindings base_bindings;
    std::map<std::string, Bindings> game_bindings;
    {
        IniFile ini;
        std::string ini_err;
        if (ini.load("jill.ini", ini_err)) {
            load_bindings_ini(ini, base_bindings);
            for (const std::string& sec : ini.sections()) {
                static const std::string kPrefix = "controls.";
                if (sec.compare(0, kPrefix.size(), kPrefix) == 0 &&
                    sec.size() > kPrefix.size()) {
                    Bindings b = base_bindings;
                    load_bindings_ini(ini, b, sec);
                    game_bindings[sec.substr(kPrefix.size())] = b;
                }
            }
        }
    }
    std::string bindings_game = engine.game().id;
    Bindings bindings = base_bindings;
    {
        const auto it = game_bindings.find(bindings_game);
        if (it != game_bindings.end()) {
            bindings = it->second;
        }
    }
    SDL_GameController* controller = nullptr;
    SDL_Joystick* joystick = nullptr;
    auto close_pads = [&]() {
        if (controller) {
            SDL_GameControllerClose(controller);
            controller = nullptr;
        }
        if (joystick) {
            SDL_JoystickClose(joystick);
            joystick = nullptr;
        }
    };
    auto open_pads = [&]() {
        if (controller || joystick) {
            return;
        }
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller) {
                    return;
                }
            }
        }
        if (SDL_NumJoysticks() > 0) {
            joystick = SDL_JoystickOpen(0);
        }
    };
    open_pads();

    // Front-end sound test: switch the sampled bank to any installed game so
    // the same screen can audition every episode/pack.  The game's own bank is
    // restored when the screen closes (and whenever the episode changes).
    auto load_game_sfx = [&](const GameDef& g) -> bool {
        if (g.pages_file.empty()) {
            std::fprintf(stderr, "%s has no sound bank\n", g.id.c_str());
            return false;
        }
        std::string sfx_err;
        if (!music_player.load_sfx(track_path(g, g.pages_file), sfx_err)) {
            std::fprintf(stderr, "%s\n", sfx_err.c_str());
            return false;
        }
        music_player.set_sfx_volume(menu.sfx_on ? menu.sfx_volume / 100.0f
                                                : 0.0f);
        return true;
    };
    auto select_noise_set = [&](int index) {
        if (games.empty()) {
            return;
        }
        const int n = static_cast<int>(games.size());
        menu.noise_set = ((index % n) + n) % n;
        menu.noise_loaded =
            load_game_sfx(games[static_cast<size_t>(menu.noise_set)]);
        if (menu.noise_loaded) {
            // A sound test should be audible even when gameplay SFX are off.
            music_player.set_sfx_volume(menu.sfx_volume / 100.0f);
        }
        if (menu.noise_loaded &&
            !music_player.sfx_has_sample(menu.noise_sound)) {
            for (int i = 1; i < 50; ++i) {
                if (music_player.sfx_has_sample(i)) {
                    menu.noise_sound = i;
                    break;
                }
            }
        }
    };
    auto restore_game_sfx = [&]() {
        if (load_game_sfx(engine.game())) {
            menu.noise_loaded = false;
        }
    };

    // Build the pack's effect overrides (raw 0..49 slots).  Called whenever the
    // pack changes; the bank itself stays the game's own `.vcl`.
    auto build_sfx_overrides = [&]() {
        auto overrides = std::make_shared<SfxOverrides>();
        if (ext_assets.mounted()) {
            int used = 0;
            for (int slot = 0; slot < 50; ++slot) {
                std::string rel;
                if (!ext_assets.find_sfx(slot, rel)) {
                    continue;
                }
                std::shared_ptr<const DecodedAudio> audio;
                if (decode_pack_audio(rel, audio)) {
                    (*overrides)[static_cast<size_t>(slot)] = std::move(audio);
                    ++used;
                }
            }
            if (used > 0) {
                std::printf("external: %d effect override(s) from %s\n", used,
                            ext_assets.active() ? ext_assets.active()->name.c_str()
                                                : "autoload");
            }
        }
        music_player.set_sfx_overrides(overrides);
    };
    auto refresh_external_visuals = [&]() {
        if (ext_assets.mounted()) {
            if (!engine.apply_external(ext_assets, palette)) {
                std::fprintf(stderr, "external: %s has no usable art\n",
                             ext_assets.active() ? ext_assets.active()->name.c_str()
                                                 : "autoload");
            }
        } else {
            engine.apply_external(ext_assets, palette);  // clears overrides
        }
    };
    auto apply_pack = [&](int index) {
        if (index < 0 || index >= static_cast<int>(ext_assets.packs().size())) {
            ext_assets.unmount();
            refresh_external_visuals();
            build_sfx_overrides();
            if (!engine.game().title_music.empty()) {
                play_music(engine.game().title_music);
            } else {
                music_player.stop();
            }
            return;
        }
        std::string mount_err;
        if (!ext_assets.mount(index, mount_err)) {
            std::fprintf(stderr, "external: %s\n", mount_err.c_str());
            return;
        }
        std::printf("external: mounted %s\n", ext_assets.active()->name.c_str());
        refresh_external_visuals();
        build_sfx_overrides();
        if (!engine.game().title_music.empty()) {
            play_music(engine.game().title_music);
        } else {
            music_player.stop();
        }
    };
    // Mount the initial pack from --pack / [external] pack.
    if (!opt.pack.empty()) {
        const int index = ext_assets.find(opt.pack);
        if (index < 0) {
            std::fprintf(stderr, "external: pack '%s' not found in %s\n",
                         opt.pack.c_str(), opt.external_dir.c_str());
        } else {
            apply_pack(index);
        }
    } else {
        refresh_external_visuals();
        build_sfx_overrides();
    }
    std::string ext_game = engine.game().id;
    if (opt.audio_check) {
        SDL_Delay(1500);
        std::printf("audio-check: device %s, %d Hz, %d ch, music '%s', "
                    "frames %llu, peak %.3f\n",
                    music_player.is_open() ? "open" : "FAILED",
                    music_player.sample_rate(), music_player.channels(),
                    music_player.title().c_str(),
                    music_player.frames_mixed(), music_player.peak_level());
        music_player.play_sfx(6, 0x19);  // APPLE pickup sound
        SDL_Delay(400);
        std::printf("audio-check: sfx peak %.3f\n", music_player.sfx_peak());
        return music_player.is_open() ? 0 : 1;
    }

    for (size_t i = 0; i < games.size(); ++i) {
        if (games[i].id == engine.game().id) {
            menu.noise_set = static_cast<int>(i);
            break;
        }
    }

    // Shared menu/level-select navigation.  Codes: 1 up, 2 down, 3 left,
    // 4 right, 5 confirm, 6 cancel/back.
    auto menu_nav = [&](int nav) {
        if (select.open) {
            const GameDef* sg =
                (select.game >= 0 && select.game < static_cast<int>(games.size()))
                    ? &games[static_cast<size_t>(select.game)]
                    : nullptr;
            int n = sg ? sg->board_count() : 1;
            switch (nav) {
                case 6:
                    select.open = false;
                    break;
                case 1:
                    select.board = (select.board + n - 1) % n;
                    break;
                case 2:
                    select.board = (select.board + 1) % n;
                    break;
                case 3:
                case 4: {
                    const int count = static_cast<int>(games.size());
                    select.game = (select.game + (nav == 3 ? count - 1 : 1)) % count;
                    select.board = 0;
                    break;
                }
                case 5:
                    if (apply_selection(engine, games, select, menu.hud_frame,
                                        menu.menu_frame)) {
                        select.open = false;
                        game_started = true;
                    }
                    break;
                default:
                    break;
            }
            return;
        }
        if (!menu_open(menu) || menu.rebind_action >= 0) {
            return;
        }
        std::vector<std::string> items =
            menu_items(menu, game_started, video, engine, bindings, games,
                       music_player, ext_assets);
        int& sel = menu_sel(menu);
        const int n = static_cast<int>(items.size());
        if (sel < 0 || sel >= n) {
            sel = 0;
        }
        auto toggle_option = [&]() {
            if (menu.screen == MenuScreen::Hud && sel == 0) {
                engine.set_hud_classic(!engine.hud_classic());
            } else if (menu.screen == MenuScreen::Hud && sel == 1) {
                engine.set_hud_bg(next_hud_bg(engine.hud_bg()));
            } else if (menu.screen == MenuScreen::Hud && sel == 2) {
                engine.set_hud_text(next_hud_text(engine.hud_text()));
            } else if (menu.screen == MenuScreen::Hud && sel == 3) {
                engine.set_hud_health(next_hud_health(engine.hud_health()));
            } else if (menu.screen == MenuScreen::Hud && sel == 4) {
                engine.set_hud_portrait(!engine.hud_portrait());
            } else if (menu.screen == MenuScreen::Hud && sel == 5) {
                menu.hud_panel = hud_panel_next(
                    engine.hud_panel(), engine.hud_classic(),
                    engine.external_visuals());
                engine.set_hud_panel(menu.hud_panel);
            } else if (menu.screen == MenuScreen::Hud && sel == 6) {
                menu.hud_frame = frame_source_next(
                    menu.hud_frame, 1, static_cast<int>(games.size()));
                apply_hud_frame(engine, games, menu.hud_frame);
            } else if (menu.screen == MenuScreen::Menus && sel == 0) {
                menu.classic = !menu.classic;
            } else if (menu.screen == MenuScreen::Menus && sel == 1) {
                menu.menu_frame = menu_border_next(
                    menu.menu_frame, 1, static_cast<int>(games.size()),
                    external_border_count(engine));
                apply_menu_frame(engine, games, menu.menu_frame,
                                 engine.menu_theme());
            } else if (menu.screen == MenuScreen::Menus && sel == 2) {
                engine.set_menu_theme(menu_theme_next(
                    engine.menu_theme(), 1, games,
                    external_theme_count(engine)));
                apply_theme_frame(engine, games, engine.menu_theme());
                if (menu.menu_frame < 0) {
                    apply_menu_frame(engine, games, menu.menu_frame,
                                     engine.menu_theme());
                }
            } else if (menu.screen == MenuScreen::Menus && sel == 3) {
                menu.live_background = !menu.live_background;
            } else if (menu.screen == MenuScreen::Menus && sel == 4) {
                video.splash = splash_next(video.splash, 1, games);
            } else if (menu.screen == MenuScreen::Menus && sel == 5) {
                menu.menu_font = font_name_next(
                    engine.font_name(), engine.external_visuals());
                engine.set_font_name(menu.menu_font);
            } else if (menu.screen == MenuScreen::Enhancements && sel == 0) {
                engine.set_fire_while_crouched(!engine.fire_while_crouched());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 1) {
                engine.set_max_knives(engine.max_knives() >= 5 ? 3 : 5);
            } else if (menu.screen == MenuScreen::Enhancements && sel == 2) {
                engine.set_retain_items(!engine.retain_items());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 3) {
                engine.set_fast_dialogs(!engine.fast_dialogs());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 4) {
                engine.set_air_jump_buffer(!engine.air_jump_buffer());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 5) {
                engine.set_minimap(!engine.minimap());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 6) {
                engine.set_center_fish_shot(!engine.center_fish_shot());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 7) {
                int look = engine.player_look() + 1;
                if (look > static_cast<int>(games.size())) {
                    look = 0;
                }
                apply_player_look(engine, games, look, engine.player_suit());
            } else if (menu.screen == MenuScreen::Enhancements && sel == 8) {
                int suit = engine.player_suit() + 1;
                if (suit > 8) {
                    suit = 0;
                }
                engine.set_player_suit(suit);
            } else if (menu.screen == MenuScreen::Enhancements && sel == 9) {
                engine.set_turtle(!engine.turtle());
            }
        };
        auto video_step = [&](int delta) {
            if (sel == 0) {
                video.mode = (video.mode + 3 + delta) % 3;
            } else if (sel == 1) {
                video.scale = ((video.scale - 1 + 6 + delta) % 6) + 1;
            } else if (sel == 2) {
                video.resolution =
                    (video.resolution + kResolutionCount + delta) % kResolutionCount;
            } else if (sel == 3) {
                int i = 0;
                for (int k = 0; k < kHighFpsCount; ++k) {
                    if (kHighFps[k] == video.high_fps) {
                        i = k;
                        break;
                    }
                }
                i = (i + kHighFpsCount + delta) % kHighFpsCount;
                video.high_fps = kHighFps[i];
                return;  // presentation rate only; no window change
            } else {
                return;  // FILTERS is a placeholder
            }
            apply_video();
        };

        if (nav == 6) {  // cancel / back
            if (menu.screen == MenuScreen::Main) {
                if (game_started) {
                    menu.screen = MenuScreen::Closed;
                } else {
                    running = false;
                }
            } else if (menu.screen == MenuScreen::SaveGame ||
                       menu.screen == MenuScreen::LoadGame) {
                menu.screen = MenuScreen::Main;
            } else if (menu.screen == MenuScreen::Enhancements ||
                       menu.screen == MenuScreen::Controls ||
                       menu.screen == MenuScreen::Hud ||
                       menu.screen == MenuScreen::Video ||
                       menu.screen == MenuScreen::Audio ||
                       menu.screen == MenuScreen::Menus ||
                       menu.screen == MenuScreen::Assets ||
                       menu.screen == MenuScreen::Cheats) {
                menu.screen = MenuScreen::Options;
            } else if (menu.screen == MenuScreen::Noisemaker) {
                restore_game_sfx();
                music_player.set_enabled(menu.music_on);
                menu.screen = MenuScreen::Audio;
                menu.audio_sel = 4;
            } else {
                menu.screen = MenuScreen::Main;
            }
            return;
        }
        if (nav == 1) {
            sel = (sel + n - 1) % n;
            return;
        }
        if (nav == 2) {
            sel = (sel + 1) % n;
            return;
        }

        switch (menu.screen) {
            case MenuScreen::Main: {
                if (nav != 5) {
                    break;
                }
                if (game_started) {
                    switch (sel) {
                        case 0: menu.screen = MenuScreen::Closed; break;
                        case 1: menu.screen = MenuScreen::SaveGame; menu.save_sel = 0; break;
                        case 2: {
                            int idx = game_index_for_id(games, engine.game().id);
                            if (idx < 0) idx = 0;
                            menu.screen = MenuScreen::LoadGame;
                            menu.save_sel = 0;
                            menu.load_labels.clear();
                            ensure_load_labels(menu, games, idx);
                            break;
                        }
                        case 3:
                            menu.screen = MenuScreen::Episodes;
                            menu.ep_sel = 0;
                            for (size_t i = 0; i < games.size(); ++i) {
                                if (games[i].id == engine.game().id) {
                                    menu.ep_sel = static_cast<int>(i);
                                    break;
                                }
                            }
                            break;
                        case 4: {
                            int idx = game_index_for_id(games, engine.game().id);
                            if (idx < 0) idx = 0;
                            ensure_hiscores(menu, games, idx);
                            menu.hi_sel = 0;
                            menu.screen = MenuScreen::HighScores;
                            break;
                        }
                        case 5: menu.screen = MenuScreen::Options; break;
                        case 6:
                            if (!begin_hiscore_entry(
                                    score_entry, menu.hi_scores,
                                    engine.game().id,
                                    static_cast<uint32_t>(engine.score()),
                                    true)) {
                                menu.main_sel = 0;
                                menu.screen = MenuScreen::Main;
                                game_started = false;
                            } else {
                                menu.screen = MenuScreen::Closed;
                            }
                            break;
                        default: running = false; break;
                    }
                } else {
                    switch (sel) {
                        case 0:
                            menu.screen = MenuScreen::Episodes;
                            menu.ep_sel = 0;
                            for (size_t i = 0; i < games.size(); ++i) {
                                if (games[i].id == engine.game().id) {
                                    menu.ep_sel = static_cast<int>(i);
                                    break;
                                }
                            }
                            break;
                        case 1: {
                            int idx = game_index_for_id(games, engine.game().id);
                            if (idx < 0) idx = 0;
                            menu.screen = MenuScreen::LoadGame;
                            menu.save_sel = 0;
                            menu.load_labels.clear();
                            ensure_load_labels(menu, games, idx);
                            break;
                        }
                        case 2: {
                            int idx = game_index_for_id(games, engine.game().id);
                            if (idx < 0) idx = 0;
                            ensure_hiscores(menu, games, idx);
                            menu.hi_sel = 0;
                            menu.screen = MenuScreen::HighScores;
                            break;
                        }
                        case 3: menu.screen = MenuScreen::Options; break;
                        default: running = false; break;
                    }
                }
                break;
            }
            case MenuScreen::Episodes: {
                const int count = static_cast<int>(games.size());
                if (nav == 3 || nav == 4) {
                    menu.ep_sel =
                        (menu.ep_sel + (nav == 3 ? count - 1 : 1)) % count;
                } else if (sel < count) {
                    if (start_game(engine, games, sel, menu.hud_frame,
                                   menu.menu_frame)) {
                        game_started = true;
                        menu.screen = MenuScreen::Closed;
                        if (engine.message_box_classic()) {
                            engine.show_message_box(
                                "Reminder",
                                "YOU CAN PRESS F1 AT ANY TIME TO GET HELP.");
                        }
                    }
                } else {
                    menu.screen = MenuScreen::Main;
                }
                break;
            }
            case MenuScreen::Options:
                if (nav == 5 && sel == 0) {
                    menu.screen = MenuScreen::Hud;
                } else if (nav == 5 && sel == 1) {
                    menu.screen = MenuScreen::Video;
                } else if (nav == 5 && sel == 2) {
                    menu.screen = MenuScreen::Audio;
                    menu.audio_sel = 0;
                } else if (nav == 5 && sel == 3) {
                    menu.screen = MenuScreen::Cheats;
                } else if (nav == 5 && sel == 4) {
                    menu.screen = MenuScreen::Controls;
                } else if (nav == 5 && sel == 5) {
                    menu.screen = MenuScreen::Menus;
                } else if (nav == 5 && sel == 6) {
                    menu.screen = MenuScreen::Enhancements;
                } else if (nav == 5 && sel == 7) {
                    menu.assets_sel = 0;
                    menu.screen = MenuScreen::Assets;
                } else if (nav == 5 && sel == 8) {
                    menu.screen = MenuScreen::Main;
                } else {
                    toggle_option();
                }
                break;
            case MenuScreen::Audio:
                if (nav == 5 && sel == 6) {
                    select_noise_set(menu.noise_set);
                    menu.noise_sel = 0;
                    music_player.set_enabled(false);
                    menu.screen = MenuScreen::Noisemaker;
                } else if (nav == 5 && sel == 7) {
                    menu.screen = MenuScreen::Options;
                } else if (sel == 5 && (nav == 3 || nav == 4 || nav == 5)) {
                    const int count =
                        static_cast<int>(MusicPlayer::midi_devices().size());
                    if (count > 0) {
                        int next = music_player.midi_device();
                        if (nav == 3) {
                            --next;
                            if (next < -1) {
                                next = count - 1;
                            }
                        } else {
                            ++next;
                            if (next >= count) {
                                next = -1;
                            }
                        }
                        if (next != music_player.midi_device()) {
                            std::string midi_err;
                            if (!music_player.set_midi_device(next, midi_err)) {
                                std::fprintf(stderr, "midi: %s\n",
                                             midi_err.c_str());
                            }
                        }
                    }
                } else if (sel == 0 && (nav == 5 || nav == 3 || nav == 4)) {
                    menu.music_on = !menu.music_on;
                    music_player.set_enabled(menu.music_on);
                } else if (sel == 1 && (nav == 3 || nav == 4)) {
                    menu.music_volume =
                        std::max(0, std::min(100, menu.music_volume +
                                                      (nav == 3 ? -10 : 10)));
                    music_player.set_volume(menu.music_volume / 100.0f);
                } else if (sel == 2 && (nav == 5 || nav == 3 || nav == 4)) {
                    menu.sfx_on = !menu.sfx_on;
                    music_player.set_sfx_volume(
                        menu.sfx_on ? menu.sfx_volume / 100.0f : 0.0f);
                } else if (sel == 3 && (nav == 3 || nav == 4)) {
                    menu.sfx_volume =
                        std::max(0, std::min(100, menu.sfx_volume +
                                                      (nav == 3 ? -10 : 10)));
                    music_player.set_sfx_volume(
                        menu.sfx_on ? menu.sfx_volume / 100.0f : 0.0f);
                } else if (sel == 4 && (nav == 5 || nav == 3 || nav == 4)) {
                    menu.pc_speaker = !menu.pc_speaker;
                    music_player.set_pc_speaker(menu.pc_speaker);
                }
                break;
            case MenuScreen::Noisemaker:
                if (nav == 5 && sel == 3) {
                    restore_game_sfx();
                    music_player.set_enabled(menu.music_on);
                    menu.screen = MenuScreen::Audio;
                    menu.audio_sel = 6;
                } else if (sel == 0 && (nav == 3 || nav == 4)) {
                    select_noise_set(menu.noise_set + (nav == 3 ? -1 : 1));
                } else if (sel == 1 && (nav == 3 || nav == 4)) {
                    menu.noise_sound =
                        (menu.noise_sound + (nav == 3 ? 49 : 1)) % 50;
                } else if (nav == 5 && (sel == 1 || sel == 2)) {
                    music_player.play_sfx_sample(6, menu.noise_sound);
                }
                break;
            case MenuScreen::Assets: {
                if (nav != 5) {
                    break;
                }
                const int packs = static_cast<int>(ext_assets.packs().size());
                if (sel == 0) {
                    apply_pack(-1);
                } else if (sel >= 1 && sel <= packs) {
                    apply_pack(sel - 1);
                } else if (sel == packs + 1) {
                    std::string e;
                    if (!ext_assets.scan(opt.external_dir, e)) {
                        std::fprintf(stderr, "%s\n", e.c_str());
                    }
                    menu.assets_sel = 0;
                } else {
                    menu.screen = MenuScreen::Options;
                }
                break;
            }
            case MenuScreen::Video:
                if (nav == 5 && sel == 8) {
                    menu.screen = MenuScreen::Options;
                } else if (sel == 4) {
                    if (nav == 5 || nav == 3 || nav == 4) {
                        video.interpolate = !video.interpolate;
                    }
                } else if (sel == 5) {
                    if (nav == 5 || nav == 3 || nav == 4) {
                        const int step = (nav == 3) ? 2 : 1;
                        engine.set_message_box((engine.message_box() + step) % 3);
                    }
                } else if (sel == 6) {
                    if (nav == 5 || nav == 3 || nav == 4) {
                        video.filter = filter_next(video.filter);
                    }
                } else if (sel == 7) {
                    if (nav == 5 || nav == 3 || nav == 4) {
                        video.stereo =
                            stereo_next(video.stereo, nav == 3 ? -1 : 1);
                    }
                } else if (nav == 3) {
                    video_step(-1);
                } else if (nav == 4 || nav == 5) {
                    video_step(1);
                }
                break;
            case MenuScreen::Hud:
                if (nav == 5 && sel == 7) {
                    menu.screen = MenuScreen::Options;
                } else if (sel >= 0 && sel <= 6) {
                    toggle_option();
                }
                break;
            case MenuScreen::Menus:
                if (nav == 5 && sel == 6) {
                    menu.screen = MenuScreen::Options;
                } else if (sel == 0 && (nav == 5 || nav == 3 || nav == 4)) {
                    menu.classic = !menu.classic;
                } else if (sel == 1 && (nav == 3 || nav == 4)) {
                    menu.menu_frame = menu_border_next(
                        menu.menu_frame, nav == 3 ? -1 : 1,
                        static_cast<int>(games.size()),
                        external_border_count(engine));
                    apply_menu_frame(engine, games, menu.menu_frame,
                                     engine.menu_theme());
                } else if (sel == 2 && (nav == 3 || nav == 4)) {
                    engine.set_menu_theme(
                        menu_theme_next(engine.menu_theme(),
                                        nav == 3 ? -1 : 1, games,
                                        external_theme_count(engine)));
                    apply_theme_frame(engine, games, engine.menu_theme());
                    if (menu.menu_frame < 0) {
                        apply_menu_frame(engine, games, menu.menu_frame,
                                         engine.menu_theme());
                    }
                } else if (sel == 3 && (nav == 5 || nav == 3 || nav == 4)) {
                    menu.live_background = !menu.live_background;
                } else if (sel == 4 && (nav == 5 || nav == 3 || nav == 4)) {
                    video.splash = splash_next(video.splash,
                                               nav == 3 ? -1 : 1, games);
                } else if (sel == 5 && (nav == 3 || nav == 4)) {
                    menu.menu_font = font_name_next(
                        engine.font_name(), engine.external_visuals());
                    engine.set_font_name(menu.menu_font);
                }
                break;
            case MenuScreen::Controls:
                if (nav != 5) {
                    break;
                }
                if (sel < ACT_COUNT) {
                    menu.rebind_action = sel;
                } else if (sel == ACT_COUNT) {
                    reset_bindings(bindings);
                } else {
                    menu.screen = MenuScreen::Options;
                }
                break;
            case MenuScreen::Cheats:
                if (nav == 3 || nav == 4) {
                    if (sel == 0) {
                        engine.set_invincible(!engine.invincible());
                    } else if (sel == 1) {
                        engine.set_debug_fly(!engine.debug_fly());
                    } else if (sel == 2) {
                        engine.set_super_jump((engine.super_jump() + 1) % 16);
                    }
                    break;
                }
                if (nav != 5) {
                    break;
                }
                if (sel == 0) {
                    engine.set_invincible(!engine.invincible());
                } else if (sel == 1) {
                    engine.set_debug_fly(!engine.debug_fly());
                } else if (sel == 2) {
                    engine.set_super_jump((engine.super_jump() + 1) % 16);
                } else if (sel == 3) {
                    engine.add_item(2);  // knife
                } else if (sel == 4) {
                    engine.add_item(1);  // key
                } else if (sel == 5) {
                    engine.add_item(3);  // gem
                } else if (sel == 6) {
                    engine.add_item(8);  // spinning blade
                } else if (sel == 7) {
                    engine.debug_heal();
                } else if (sel >= 8 && sel <= 12) {
                    // 8 Jill, 9 frog, 10 bird, 11 spider, 12 fish.
                    static const int kItem[5] = {0, 4, 5, 6, 7};
                    engine.debug_morph(kItem[sel - 8]);
                } else if (sel == 13) {
                    // Same panel as F3, opened from the cheats list.
                    select.open = true;
                    select.game = 0;
                    for (size_t i = 0; i < games.size(); ++i) {
                        if (games[i].id == engine.game().id) {
                            select.game = static_cast<int>(i);
                            break;
                        }
                    }
                    select.board = 0;
                    const GameDef& cur =
                        games[static_cast<size_t>(select.game)];
                    for (int i = 0; i < cur.board_count(); ++i) {
                        if (engine.board_name() == cur.board(i)) {
                            select.board = i;
                            break;
                        }
                    }
                    menu.screen = MenuScreen::Closed;
                } else {
                    menu.screen = MenuScreen::Options;
                }
                break;
            case MenuScreen::SaveGame: {
                if (nav != 5) {
                    break;
                }
                if (sel >= engine.save_slots()) {
                    menu.screen = MenuScreen::Main;
                    break;
                }
                std::string err;
                if (!engine.save_game(sel, err)) {
                    std::fprintf(stderr, "save: %s\n", err.c_str());
                }
                break;
            }
            case MenuScreen::LoadGame: {
                const int count = static_cast<int>(games.size());
                if (nav == 3 || nav == 4) {
                    int idx = menu.load_game_index;
                    if (idx < 0 || idx >= count) idx = 0;
                    idx = (idx + (nav == 3 ? count - 1 : 1)) % count;
                    ensure_load_labels(menu, games, idx);
                    menu.save_sel = 0;
                    break;
                }
                if (nav != 5) {
                    break;
                }
                const int slots = static_cast<int>(menu.load_labels.size());
                if (sel == 0) {
                    break;  // episode row: use Left/Right to change it
                }
                if (sel > slots) {
                    menu.screen = MenuScreen::Main;
                    break;
                }
                const int idx = menu.load_game_index;
                if (idx < 0 || idx >= count) {
                    break;
                }
                const int slot = sel - 1;
                if (engine.game().id != games[static_cast<size_t>(idx)].id) {
                    if (!start_game(engine, games, idx, menu.hud_frame,
                                    menu.menu_frame)) {
                        break;
                    }
                }
                std::string err;
                if (!engine.load_game(slot, err)) {
                    std::fprintf(stderr, "restore: %s\n", err.c_str());
                } else {
                    game_started = true;
                    menu.screen = MenuScreen::Closed;
                }
                break;
            }
            case MenuScreen::HighScores: {
                const int count = static_cast<int>(games.size());
                if (nav == 3 || nav == 4) {
                    int idx = menu.hi_game;
                    if (idx < 0 || idx >= count) idx = 0;
                    idx = (idx + (nav == 3 ? count - 1 : 1)) % count;
                    ensure_hiscores(menu, games, idx);
                    menu.hi_sel = 0;
                    break;
                }
                if (nav == 5 &&
                    sel >= static_cast<int>(items.size()) - 1) {
                    menu.screen = MenuScreen::Main;
                }
                break;
            }
            case MenuScreen::Enhancements:
                if (nav == 5 && sel == 10) {
                    menu.screen = MenuScreen::Options;
                } else {
                    toggle_option();
                }
                break;
            default:
                break;
        }
    };

    while (running) {
        const uint64_t frame_start = SDL_GetPerformanceCounter();
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                running = false;
            } else if (score_entry.active &&
                       (ev.type == SDL_KEYDOWN ||
                        ev.type == SDL_CONTROLLERBUTTONDOWN)) {
                bool commit = false;
                bool use_default = false;
                if (ev.type == SDL_KEYDOWN && !ev.key.repeat) {
                    const SDL_Scancode sc = ev.key.keysym.scancode;
                    if (sc == SDL_SCANCODE_RETURN ||
                        sc == SDL_SCANCODE_KP_ENTER) {
                        commit = true;
                    } else if (sc == SDL_SCANCODE_ESCAPE) {
                        use_default = true;
                    } else if (sc == SDL_SCANCODE_BACKSPACE) {
                        if (!score_entry.name.empty()) {
                            score_entry.name.pop_back();
                        }
                    } else {
                        const SDL_Keycode k = ev.key.keysym.sym;
                        if (k >= 32 && k < 127 && score_entry.name.size() < 9) {
                            char c = static_cast<char>(k);
                            if (c >= 'a' && c <= 'z') {
                                c = static_cast<char>(c - 'a' + 'A');
                            }
                            score_entry.name.push_back(c);
                        }
                    }
                } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
                    const int b = ev.cbutton.button;
                    if (b == SDL_CONTROLLER_BUTTON_A ||
                        b == SDL_CONTROLLER_BUTTON_START) {
                        commit = true;
                    } else if (b == SDL_CONTROLLER_BUTTON_B ||
                               b == SDL_CONTROLLER_BUTTON_BACK) {
                        use_default = true;
                    }
                }
                if (commit || use_default) {
                    if (use_default) {
                        score_entry.name.clear();
                    }
                    const bool to_menu = score_entry.to_menu;
                    commit_hiscore_entry(score_entry, menu.hi_scores);
                    if (to_menu) {
                        menu.screen = MenuScreen::Main;
                        menu.main_sel = 0;
                        game_started = false;
                    }
                }
                continue;
            } else if (ending.active &&
                       (ev.type == SDL_KEYDOWN ||
                        ev.type == SDL_MOUSEBUTTONDOWN ||
                        ev.type == SDL_CONTROLLERBUTTONDOWN)) {
                bool skip = false;
                if (ev.type == SDL_KEYDOWN) {
                    skip = ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE;
                } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
                    const int b = ev.cbutton.button;
                    skip = b == SDL_CONTROLLER_BUTTON_B ||
                           b == SDL_CONTROLLER_BUTTON_BACK;
                }
                const bool done = skip || advance_ending_viewer(ending);
                if (done) {
                    finish_ending();
                }
                continue;
            } else if (splash_active &&
                       (ev.type == SDL_KEYDOWN ||
                        ev.type == SDL_MOUSEBUTTONDOWN ||
                        ev.type == SDL_CONTROLLERBUTTONDOWN)) {
                splash_active = false;  // any input skips the publisher screen
            } else if (ev.type == SDL_CONTROLLERDEVICEADDED ||
                       ev.type == SDL_CONTROLLERDEVICEREMOVED ||
                       ev.type == SDL_JOYDEVICEADDED ||
                       ev.type == SDL_JOYDEVICEREMOVED) {
                // Re-scan so an XInput pad takes precedence over a raw pad and
                // unplugged devices are released.
                close_pads();
                open_pads();
            } else if (ev.type == SDL_CONTROLLERAXISMOTION &&
                       engine.text_page_open()) {
                const int dir = ev.caxis.value > 16000
                                    ? 1
                                    : (ev.caxis.value < -16000 ? -1 : 0);
                if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if (dir != page_axis_y) {
                        page_axis_y = dir;
                        if (dir != 0) {
                            engine.scroll_text_page(dir * 2);
                        }
                    }
                } else if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if (dir != page_axis_x) {
                        page_axis_x = dir;
                        if (dir != 0) {
                            engine.cycle_text_page(dir);
                        }
                    }
                }
            } else if (ev.type == SDL_JOYHATMOTION &&
                       engine.text_page_open()) {
                const Uint8 h = ev.jhat.value;
                if (h & SDL_HAT_UP) {
                    engine.scroll_text_page(-1);
                } else if (h & SDL_HAT_DOWN) {
                    engine.scroll_text_page(1);
                }
                if (h & SDL_HAT_LEFT) {
                    engine.cycle_text_page(-1);
                } else if (h & SDL_HAT_RIGHT) {
                    engine.cycle_text_page(1);
                }
            } else if (ev.type == SDL_CONTROLLERBUTTONDOWN) {
                const int b = ev.cbutton.button;
                if (engine.text_page_open()) {
                    switch (b) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            engine.scroll_text_page(-1);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            engine.scroll_text_page(1);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            engine.cycle_text_page(-1);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            engine.cycle_text_page(1);
                            break;
                        case SDL_CONTROLLER_BUTTON_A:
                        case SDL_CONTROLLER_BUTTON_B:
                        case SDL_CONTROLLER_BUTTON_START:
                        case SDL_CONTROLLER_BUTTON_BACK:
                            engine.close_text_page();
                            break;
                        default:
                            break;
                    }
                    continue;
                }
                if (menu_open(menu) && menu.rebind_action >= 0) {
                    bindings.pad[menu.rebind_action] = b;
                    menu.rebind_action = -1;
                } else {
                    int nav = 0;
                    switch (b) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP: nav = 1; break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: nav = 2; break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: nav = 3; break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: nav = 4; break;
                        case SDL_CONTROLLER_BUTTON_A: nav = 5; break;
                        case SDL_CONTROLLER_BUTTON_B: nav = 6; break;
                        case SDL_CONTROLLER_BUTTON_START:
                            if (select.open) {
                                nav = 6;
                            } else if (!menu_open(menu)) {
                                // Start pauses into the main menu.
                                menu.screen = MenuScreen::Main;
                                menu.main_sel = 0;
                            } else if (game_started) {
                                nav = 6;  // resume
                            }
                            break;
                        default:
                            break;
                    }
                    if (nav != 0) {
                        menu_nav(nav);
                    }
                }
            } else if (ev.type == SDL_KEYDOWN && !ev.key.repeat) {
                const SDL_Scancode sc = ev.key.keysym.scancode;
                if (engine.text_page_open()) {
                    switch (sc) {
                        case SDL_SCANCODE_UP: engine.scroll_text_page(-1); break;
                        case SDL_SCANCODE_DOWN: engine.scroll_text_page(1); break;
                        case SDL_SCANCODE_PAGEUP:
                            engine.scroll_text_page(-8);
                            break;
                        case SDL_SCANCODE_PAGEDOWN:
                            engine.scroll_text_page(8);
                            break;
                        case SDL_SCANCODE_LEFT: engine.cycle_text_page(-1); break;
                        case SDL_SCANCODE_RIGHT: engine.cycle_text_page(1); break;
                        case SDL_SCANCODE_ESCAPE:
                        case SDL_SCANCODE_RETURN:
                        case SDL_SCANCODE_KP_ENTER:
                        case SDL_SCANCODE_F1:
                            engine.close_text_page();
                            break;
                        default: break;
                    }
                    continue;
                }
                if (menu_open(menu) && menu.rebind_action >= 0) {
                    if (sc == SDL_SCANCODE_ESCAPE) {
                        menu.rebind_action = -1;
                    } else {
                        bindings.key1[menu.rebind_action] = sc;
                        bindings.key2[menu.rebind_action] =
                            SDL_SCANCODE_UNKNOWN;
                        menu.rebind_action = -1;
                    }
                    continue;
                }
                int nav = 0;
                switch (sc) {
                    case SDL_SCANCODE_UP: nav = 1; break;
                    case SDL_SCANCODE_DOWN: nav = 2; break;
                    case SDL_SCANCODE_LEFT: nav = 3; break;
                    case SDL_SCANCODE_RIGHT: nav = 4; break;
                    case SDL_SCANCODE_RETURN:
                    case SDL_SCANCODE_KP_ENTER: nav = 5; break;
                    case SDL_SCANCODE_ESCAPE: nav = 6; break;
                    default: break;
                }
                if (nav != 0) {
                    if (nav == 6 && !menu_open(menu) && !select.open) {
                        menu.screen = MenuScreen::Main;  // Esc pauses
                        menu.main_sel = 0;
                    } else {
                        menu_nav(nav);
                    }
                    continue;
                }
                if (menu_open(menu)) {
                    continue;  // other keys do nothing while a menu is up
                }
                if (select.open) {
                    if (sc == SDL_SCANCODE_F3) {
                        select.open = false;
                    }
                    continue;
                }
                switch (sc) {
                    case SDL_SCANCODE_F11:
                        video.mode = (video.mode == 0) ? 1 : 0;
                        apply_video();
                        break;
                    case SDL_SCANCODE_R:
                        // Original in-game RESTORE key (22AB jump table: R).
                        if ((ev.key.keysym.mod & KMOD_CTRL) != 0) {
                            std::string err;
                            if (!engine.load_board(engine.board_name(), err)) {
                                std::fprintf(stderr, "reload: %s\n", err.c_str());
                            }
                        } else if (game_started) {
                            menu.screen = MenuScreen::LoadGame;
                            menu.save_sel = 0;
                        }
                        break;
                    case SDL_SCANCODE_S:
                        // Original in-game SAVE key (22AB jump table: S).
                        if (game_started) {
                            menu.screen = MenuScreen::SaveGame;
                            menu.save_sel = 0;
                        }
                        break;
                    case SDL_SCANCODE_F1:
                        // F1 help: the sampled-sound bank's page 1 is "How the
                        // Heck to Play" (the original's PRESS F1 FOR HELP).
                        if (engine.text_pages_available()) {
                            engine.show_text_page(1);
                        } else if (engine.message_box_classic()) {
                            engine.show_message_box(
                                "Reminder",
                                "YOU CAN PRESS F1 AT ANY TIME TO GET HELP.");
                        } else {
                            std::printf("%s\n", engine.status().c_str());
                        }
                        break;
                    case SDL_SCANCODE_F2:
                        engine.set_hud_classic(!engine.hud_classic());
                        std::printf("HUD: %s\n",
                                    engine.hud_classic() ? "classic" : "minimal");
                        break;
                    case SDL_SCANCODE_F3: {
                        if (games.empty()) {
                            break;
                        }
                        select.open = true;
                        select.game = 0;
                        select.board = 0;
                        for (size_t i = 0; i < games.size(); ++i) {
                            if (games[i].id == engine.game().id) {
                                select.game = static_cast<int>(i);
                                break;
                            }
                        }
                        const GameDef& cur = games[static_cast<size_t>(select.game)];
                        for (int i = 0; i < cur.board_count(); ++i) {
                            if (engine.board_name() == cur.board(i)) {
                                select.board = i;
                                break;
                            }
                        }
                        break;
                    }
                    case SDL_SCANCODE_F4:
                        engine.set_debug_fly(!engine.debug_fly());
                        std::printf("debug fly: %s\n",
                                    engine.debug_fly() ? "on" : "off");
                        break;
                    case SDL_SCANCODE_F5:
                        engine.set_invincible(!engine.invincible());
                        std::printf("invincible: %s\n",
                                    engine.invincible() ? "on" : "off");
                        break;
                    case SDL_SCANCODE_F8:
                        std::printf("%s\n", engine.status().c_str());
                        break;
                    default:
                        break;
                }
            }
        }

        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        if (!engine.text_page_open()) {
            page_axis_x = 0;
            page_axis_y = 0;
        }
        const bool paused = splash_active || select.open || menu_open(menu) ||
                            engine.text_page_open() || ending.active ||
                            score_entry.active;
        if (paused) {
            engine.set_input(0, 0, false, false);
        } else {
            poll_input(keys, bindings, controller, joystick, engine);
        }

        uint64_t now = SDL_GetPerformanceCounter();
        double dt = static_cast<double>(now - last) / freq;
        last = now;
        if (dt > 0.25) {
            dt = 0.25;
        }
        accumulator += dt;
        if (paused) {
            accumulator = 0.0;  // pause the world while a menu is up
        }
        while (accumulator >= step) {
            engine.tick();
            accumulator -= step;
        }
        if (ending.active) {
            // Cards advance automatically; Enter/click/A skips ahead and Esc/B
            // leaves the ending immediately.
            if (SDL_GetTicks() - ending.started >= 7000 &&
                advance_ending_viewer(ending)) {
                finish_ending();
            }
        }
        if (!ending.active && engine.take_ending()) {
            // The state-3 CHECKPT is the in-map ending event.  Show the
            // board's text cards, then return to the front end.
            if (!start_ending_viewer(engine, ending)) {
                menu.screen = MenuScreen::Main;
                menu.main_sel = 0;
                game_started = false;
            }
        }

        // Per-game bindings: stash edits when the episode changes, then load
        // the new game's override set.
        if (engine.game().id != bindings_game) {
            game_bindings[bindings_game] = bindings;
            bindings_game = engine.game().id;
            const auto it = game_bindings.find(bindings_game);
            bindings = (it != game_bindings.end()) ? it->second : base_bindings;
        }

        // Music: a `*name.ddt` CHECKPT starts that track; switching episode
        // in the menu falls back to the new game's title theme.
        if (engine.game().id != music_game) {
            music_game = engine.game().id;
            music_player.stop();
            // The sampled bank belongs to the episode, not to the executable.
            load_game_sfx(engine.game());
            menu.noise_loaded = false;
            for (size_t i = 0; i < games.size(); ++i) {
                if (games[i].id == engine.game().id) {
                    menu.noise_set = static_cast<int>(i);
                    break;
                }
            }
            if (!engine.game().title_music.empty()) {
                play_music(engine.game().title_music);
            }
        }
        if (engine.game().id != ext_game) {
            ext_game = engine.game().id;
            // A fresh Engine was built for the new episode, so rebuild the
            // pack's visual overrides against its shape bank and tile table.
            refresh_external_visuals();
        }
        const std::string music_request = engine.take_music_request();
        if (!music_request.empty()) {
            play_music(music_request);
        }
        for (uint16_t s : engine.take_sound_requests()) {
            music_player.play_sfx(s >> 8, s & 0xFF);
        }

        if (splash_active) {
            const std::vector<uint8_t>& page = splash_image.pages[splash_page];
            if (page.size() == fb.px.size()) {
                fb.px = page;
            } else {
                fb.fill_rect(0, 0, Framebuffer::W, Framebuffer::H, 0);
            }
            if (SDL_GetTicks() - splash_page_start >= 2500) {
                ++splash_page;
                if (splash_page >= splash_image.pages.size()) {
                    splash_active = false;
                } else {
                    splash_page_start = SDL_GetTicks();
                }
            }
        } else if (ending.active) {
            draw_ending_viewer(fb, engine, ending);
        } else {
            const float alpha =
                (!paused && video.interpolate && video.high_fps > 0)
                    ? static_cast<float>(accumulator / step)
                    : 1.0f;
            // Menus, the level-select panel and the save-name prompt are 2D
            // overlays; render them on the normal frame instead of splitting
            // them across the eyes.
            const bool stereo_view =
                video.stereo != 0 && !menu_open(menu) && !select.open &&
                !score_entry.active;
            if (stereo_view) {
                const int shift = std::max(1, video.stereo_depth);
                engine.set_stereo_convergence(video.stereo_convergence);
                engine.set_stereo_shift(shift);
                engine.render(stereo_left, true, alpha);
                engine.set_stereo_shift(-shift);
                engine.render(stereo_right, true, alpha);
                engine.set_stereo_shift(0);
                engine.set_stereo_convergence(0);
                compose_stereo(fb, stereo_left, stereo_right, video.stereo);
            } else {
                engine.set_stereo_shift(0);
                engine.set_stereo_convergence(0);
                engine.render(fb, true, alpha);
            }
            // Temporary debug indicators sit just below the HUD: under the
            // modern top bar, or under the classic side panel.
            const int overlay_y = engine.hud_classic() ? 86 : 14;
            if (engine.debug_fly() && !paused) {
                engine.draw_text(fb, Framebuffer::W - 26, overlay_y, 7, "FLY");
            }
            if (engine.invincible() && !paused) {
                engine.draw_text(fb, Framebuffer::W - 46, overlay_y, 7, "INV");
            }
            if (select.open) {
                draw_select(fb, engine, games, select);
            }
            if (menu_open(menu)) {
                draw_menu(fb, engine, menu, game_started, video, bindings,
                          theme_backdrop, games, music_player, ext_assets);
            }
        }
        if (score_entry.active) {
            draw_hiscore_entry(fb, engine, score_entry);
        }
        const uint8_t* out_pal =
            (splash_active && splash_image.palette_ready) ? splash_image.palette
                                                          : palette;
        for (size_t i = 0; i < fb.px.size(); ++i) {
            const uint8_t c = fb.px[i];
            const uint32_t r = out_pal[c * 3 + 0];
            const uint32_t g = out_pal[c * 3 + 1];
            const uint32_t b = out_pal[c * 3 + 2];
            argb[i] = 0xFF000000u | (r << 16) | (g << 8) | b;
        }
        if (video.filter == 1) {
            filter_smooth(argb, Framebuffer::W, Framebuffer::H);
        } else if (video.filter == 2) {
            filter_crt(argb, Framebuffer::W, Framebuffer::H);
        }
        SDL_UpdateTexture(texture, nullptr, argb.data(),
                          Framebuffer::W * static_cast<int>(sizeof(uint32_t)));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        if (SDL_GetTicks() - title_timer >= 500) {
            title_timer = SDL_GetTicks();
            char title[240];
            std::snprintf(title, sizeof(title), "Jill  %s  %s  hp=%d  %.1f fps",
                          engine.game().id.c_str(), engine.board_name().c_str(),
                          engine.health(), 1.0 / step);
            SDL_SetWindowTitle(window, title);
        }
        // Presentation pacing.  The world still advances at `step` (18.2065 Hz);
        // HIGH FRAMERATE presents the current frame at up to 144 Hz.
        const double present_hz =
            video.high_fps > 0 ? static_cast<double>(video.high_fps) : 1.0 / step;
        const double present_time = 1.0 / present_hz;
        const double spent =
            static_cast<double>(SDL_GetPerformanceCounter() - frame_start) / freq;
        const double remain = present_time - spent;
        if (remain >= 0.001) {
            // Coarse sleep, then a short spin so SDL_Delay's millisecond
            // quantisation does not drag the rate below the chosen value.
            const double remain_ms = remain * 1000.0;
            if (remain_ms > 1.0) {
                SDL_Delay(static_cast<Uint32>(remain_ms - 0.5));
            }
            while (static_cast<double>(
                       SDL_GetPerformanceCounter() - frame_start) /
                       freq < present_time) {
            }
        }
    }

    if (opt.save_slot >= 0) {
        std::string save_err;
        if (!engine.save_game(opt.save_slot, save_err)) {
            std::fprintf(stderr, "save slot %d: %s\n", opt.save_slot,
                         save_err.c_str());
        }
    }
    if (game_started) {
        add_hiscore(menu.hi_scores, engine.game().id,
                    static_cast<uint32_t>(engine.score()), "JILL");
    }

    {
        IniFile ini;
        ini.set("paths", "games",
                opt.games_dir.empty() ? "games" : opt.games_dir);
        ini.set("paths", "profiles",
                opt.profiles_dir.empty() ? "profiles" : opt.profiles_dir);
        for (const GameDef& g : games) {
            ini.set("paths", g.id,
                    g.id == engine.game().id ? engine.game().data_dir
                                             : g.data_dir);
        }
        ini.set("game", "id", engine.game().id);
        ini.set_int("video", "window_mode", video.mode);
        ini.set_int("video", "scale", video.scale);
        ini.set_int("video", "resolution", video.resolution);
        ini.set_int("video", "high_fps", video.high_fps);
        ini.set_int("video", "splash", video.splash);
        ini.set_bool("video", "interpolate", video.interpolate);
        ini.set_int("video", "filter", video.filter);
        ini.set_int("video", "stereo", video.stereo);
        ini.set_int("video", "stereo_depth", video.stereo_depth);
        ini.set_int("video", "stereo_convergence",
                    video.stereo_convergence);
        ini.set_bool("audio", "music", menu.music_on);
        ini.set_int("audio", "volume", menu.music_volume);
        ini.set_bool("audio", "sfx", menu.sfx_on);
        ini.set_int("audio", "sfx_volume", menu.sfx_volume);
        ini.set_bool("audio", "pc_speaker", menu.pc_speaker);
        ini.set_int("audio", "midi_device", music_player.midi_device());
        ini.set_bool("hud", "classic", engine.hud_classic());
        ini.set_bool("hud", "portrait", engine.hud_portrait());
        ini.set_int("hud", "frame", menu.hud_frame);
        ini.set("hud", "panel", engine.hud_panel());
        std::string bg_name;
        if (engine.hud_bg() == -2) {
            bg_name = "suit";
        } else if (engine.hud_bg() == -3) {
            bg_name = "vibrant";
        } else {
            bg_name = std::to_string(engine.hud_bg());
        }
        ini.set("hud", "background", bg_name);
        ini.set_int("hud", "text", engine.hud_text());
        if (engine.hud_health() < 0) {
            ini.set("hud", "health", "original");
        } else {
            ini.set_int("hud", "health", engine.hud_health());
        }
        ini.set_bool("gameplay", "fire_crouched",
                     engine.fire_while_crouched());
        ini.set_int("gameplay", "max_knives", engine.max_knives());
        ini.set_bool("gameplay", "retain_items", engine.retain_items());
        ini.set_bool("gameplay", "fast_dialogs", engine.fast_dialogs());
        ini.set_int("gameplay", "message_box", engine.message_box());
        ini.set_bool("gameplay", "air_jump_buffer", engine.air_jump_buffer());
        ini.set_bool("gameplay", "minimap", engine.minimap());
        ini.set_bool("gameplay", "center_fish_shot", engine.center_fish_shot());
        ini.set_bool("gameplay", "turtle", engine.turtle());
        ini.set_int("gameplay", "player_look", engine.player_look());
        ini.set_int("gameplay", "player_suit", engine.player_suit());
        ini.set_int("gameplay", "menu_theme", engine.menu_theme());
        ini.set_bool("menus", "classic", menu.classic);
        ini.set_bool("menus", "live_background", menu.live_background);
        ini.set_int("menus", "frame", menu.menu_frame);
        ini.set("menus", "font", engine.font_name());
        game_bindings[bindings_game] = bindings;
        save_bindings_ini(ini, base_bindings);
        for (const auto& kv : game_bindings) {
            const std::string sec = "controls." + kv.first;
            if (bindings_equal(kv.second, base_bindings)) {
                ini.remove_section(sec);
            } else {
                save_bindings_ini(ini, kv.second, sec);
            }
        }
        ini.save("jill.ini");
    }

    close_pads();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}

#endif  // JILL_HAVE_SDL

}  // namespace

int run_app(const Options& opt) {
    std::string err;
    const std::string profiles_dir =
        opt.profiles_dir.empty() ? std::string("profiles") : opt.profiles_dir;
    const int profile_files = load_profile_dir(profiles_dir, err);
    if (!err.empty()) {
        std::fprintf(stderr, "%s\n", err.c_str());
        err.clear();
    }
    if (profile_files > 0) {
        std::printf("loaded %d runtime profile(s) from %s\n", profile_files,
                    profiles_dir.c_str());
    }
    std::vector<GameDef> games = load_game_list(opt.games_dir, err);
    if (!err.empty()) {
        std::fprintf(stderr, "%s\n", err.c_str());
    }
    if (opt.list_midi) {
        const std::vector<std::string> devices = MusicPlayer::midi_devices();
        if (devices.empty()) {
            std::printf("no MIDI output devices\n");
        } else {
            for (size_t i = 0; i < devices.size(); ++i) {
                std::printf("%2d  %s\n", static_cast<int>(i),
                            devices[i].c_str());
            }
        }
        return 0;
    }
    if (games.empty()) {
        std::fprintf(stderr, "no games found\n");
        return 2;
    }
    // jill.ini [paths] overrides the manifest's local default directory.  The
    // keys are game ids (jill1, jill2, jill3, onesimus), so deleting the local
    // copies only needs one line per game in jill.ini.
    for (GameDef& g : games) {
        const auto it = opt.data_paths.find(g.id);
        if (it != opt.data_paths.end() && !it->second.empty() &&
            it->first != "games" && it->first != "profiles") {
            g.data_dir = it->second;
        }
    }

    if (opt.list_games) {
        for (const GameDef& g : games) {
            std::printf("%-12s %-28s profile=%s data=%s boards=%d\n",
                        g.id.c_str(), g.title.c_str(), g.profile_id.c_str(),
                        g.data_dir.c_str(), g.board_count());
        }
        return 0;
    }

    if (opt.list_packs) {
        ExternalAssets assets;
        std::string ext_err;
        if (!assets.scan(opt.external_dir, ext_err)) {
            std::fprintf(stderr, "%s\n", ext_err.c_str());
            return 2;
        }
        std::printf("%s: %d pack(s)\n", opt.external_dir.c_str(),
                    static_cast<int>(assets.packs().size()));
        if (assets.autoload()) {
            std::printf("%-24s %-8s %s\n", "autoload", "folder",
                        assets.autoload_root().c_str());
        }
        for (const PackInfo& p : assets.packs()) {
            std::printf("%-24s %-8s %s\n", p.id.c_str(),
                        p.is_zip ? "zip" : "folder", p.path.c_str());
        }
        return 0;
    }

    if (!opt.dump_splash.empty()) {
#ifdef JILL_HAVE_SDL
        int dump_index = opt.splash;
        if (dump_index < 0) {
            if (!opt.game.empty()) {
                dump_index = game_index_for_id(games, opt.game) + 1;
            } else if (opt.episode > 0) {
                dump_index = opt.episode;
            }
        }
        if (dump_index <= 0) {
            dump_index = 1;
        }
        dump_splash_pages(games, dump_index, opt.dump_splash);
        return 0;
#else
        std::fprintf(stderr, "--dump-splash needs an SDL build\n");
        return 2;
#endif
    }

    const GameDef* chosen = nullptr;
    if (!opt.game.empty()) {
        chosen = game_by_id(games, opt.game);
        if (!chosen) {
            std::fprintf(stderr, "unknown game '%s' (--list-games shows the ids)\n",
                         opt.game.c_str());
            return 2;
        }
    } else if (opt.episode >= 1 && opt.episode <= static_cast<int>(games.size())) {
        chosen = &games[static_cast<size_t>(opt.episode - 1)];
    } else if (opt.episode > 0) {
        std::fprintf(stderr, "unknown episode %d (%d installed); use --game ID\n",
                     opt.episode, static_cast<int>(games.size()));
        return 2;
    } else {
        chosen = &games.front();
    }

    GameDef game = *chosen;
    Options run_opt = opt;
    if (opt.data_dir.empty()) {
        // jill.ini can relocate each installed game individually; a bare
        // [paths] data key is the fallback for manifests without one.
        IniFile ini;
        std::string ini_err;
        if (ini.load("jill.ini", ini_err)) {
            std::string data = ini.get("paths", game.id);
            if (data.empty()) {
                data = ini.get("paths", "data");
            }
            if (!data.empty()) {
                game.data_dir = data;
            }
        }
    }
    if (!opt.data_dir.empty()) {
        game.data_dir = opt.data_dir;
    }
    if (!opt.palette.empty()) {
        game.palette = opt.palette;
    }
    run_opt.data_dir = game.data_dir;
    run_opt.palette = game.palette;
    run_opt.menu_theme = std::min(opt.menu_theme, static_cast<int>(games.size()));
    run_opt.hud_frame =
        std::min(opt.hud_frame, static_cast<int>(games.size()));
    // External borders are addressed after the installed games, so the upper
    // bound is deliberately generous; apply_menu_frame ignores stale indices.
    run_opt.menu_frame =
        std::max(-1, std::min(opt.menu_frame, static_cast<int>(games.size()) + 63));
    if (opt.resolution_w > 0) {
        run_opt.resolution = match_resolution(opt.resolution_w, opt.resolution_h);
    }

    Engine engine;
    if (!engine.init(game, err)) {
        std::fprintf(stderr, "%s\n", err.c_str());
        return 2;
    }
    // Display/dialog options first: load_board derives the caption timing from
    // the dialog speed.
    engine.set_hud_classic(opt.hud_classic);
    engine.set_hud_portrait(opt.hud_portrait);
    engine.set_invincible(opt.invincible);
    engine.set_super_jump(opt.super_jump);
    engine.set_menu_theme(run_opt.menu_theme);
    engine.set_fire_while_crouched(opt.fire_crouched);
    engine.set_max_knives(opt.max_knives);
    engine.set_retain_items(opt.retain_items);
    engine.set_fast_dialogs(opt.fast_dialogs);
    engine.set_message_box(opt.message_box);
    engine.set_air_jump_buffer(opt.air_jump_buffer);
    engine.set_minimap(opt.minimap);
    engine.set_center_fish_shot(opt.center_fish_shot);
    apply_player_look(engine, games, opt.player_look, opt.player_suit);
    engine.set_hud_bg(opt.hud_bg);
    engine.set_hud_text(opt.hud_text);
    engine.set_hud_health(opt.hud_health);
    apply_hud_frame(engine, games, run_opt.hud_frame);
    apply_theme_frame(engine, games, run_opt.menu_theme);
    engine.set_hud_panel(run_opt.hud_panel);
    engine.set_font_name(run_opt.menu_font);
    engine.set_turtle(run_opt.turtle);
    // External art (pack + autoload) must be mounted before the menu border
    // selection is resolved; headless --dump then uses the same overrides.
    ExternalAssets headless_assets;
    {
        std::string ext_err;
        if (headless_assets.scan(opt.external_dir, ext_err)) {
            if (!opt.pack.empty()) {
                const int index = headless_assets.find(opt.pack);
                if (index < 0) {
                    std::fprintf(stderr, "external: pack '%s' not found\n",
                                 opt.pack.c_str());
                } else if (!headless_assets.mount(index, ext_err)) {
                    std::fprintf(stderr, "external: %s\n", ext_err.c_str());
                }
            }
            if (headless_assets.mounted()) {
                uint8_t ext_palette[768];
                load_palette(game.palette, ext_palette);
                if (engine.apply_external(headless_assets, ext_palette)) {
                    std::printf("external: mounted %s\n",
                                headless_assets.active()
                                    ? headless_assets.active()->name.c_str()
                                    : "autoload");
                }
            }
        } else {
            std::fprintf(stderr, "%s\n", ext_err.c_str());
        }
    }
    apply_menu_frame(engine, games, run_opt.menu_frame, run_opt.menu_theme);
    std::string board = opt.board.empty() ? game.first_board : opt.board;
    if (!engine.load_board(board, err)) {
        std::fprintf(stderr, "%s\n", err.c_str());
        std::fprintf(stderr, "data dir: %s (use --data to point at the game files)\n",
                     game.data_dir.c_str());
        return 2;
    }
    for (int i = 0; i < opt.items; ++i) {
        engine.add_item(2);  // debug: knives
    }
    if (opt.blade && opt.ticks == 0) {
        engine.debug_spawn_blade();
    }
    if (opt.morph >= 0 && opt.ticks == 0) {
        engine.debug_morph(opt.morph);
    }
    if (opt.face != 0 && opt.ticks == 0) {
        engine.debug_face(opt.face);
    }
    if (opt.load_slot >= 0) {
        std::string load_err;
        if (!engine.load_game(opt.load_slot, load_err)) {
            std::fprintf(stderr, "restore slot %d: %s\n", opt.load_slot,
                         load_err.c_str());
        }
    }

    if (opt.ticks > 0) {
        return run_headless(engine, run_opt);
    }

    if (!opt.dump_music.empty()) {
        std::string track =
            opt.music_file.empty() ? game.title_music : opt.music_file;
        if (track.empty()) {
            std::fprintf(stderr,
                         "%s has no title_music; use --music-file NAME\n",
                         game.id.c_str());
            return 2;
        }
        CmfSong song;
        std::string merr;
        if (!cmf_load(track_path(game, track), song, merr)) {
            std::fprintf(stderr, "%s\n", merr.c_str());
            return 2;
        }
        const float vol =
            std::max(0, std::min(100, opt.music_volume)) / 100.0f;
        if (!MusicPlayer::render_wav(song, opt.dump_music, opt.music_seconds,
                                     48000, vol, merr)) {
            std::fprintf(stderr, "%s\n", merr.c_str());
            return 2;
        }
        std::printf("wrote %s (%s, %.1fs)\n", opt.dump_music.c_str(),
                    track.c_str(), opt.music_seconds);
        return 0;
    }

#ifdef JILL_HAVE_SDL
    return run_sdl(run_opt, engine, games);
#else
    std::fprintf(stderr,
                 "built without SDL2; rebuild with SDL2 available, or use --ticks N\n");
    return 3;
#endif
}
