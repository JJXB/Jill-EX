#include "engine.h"

#include "palette.inc"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace {

// Classic (original) HUD frame.  The board still fills the 320x200 buffer; the
// patterned border, title strip and 64-px side panel cover it, matching the
// original presentation.
constexpr int kClassicBorder = 8;
constexpr int kClassicTop = 20;
constexpr int kClassicPanelX = kClassicBorder;
constexpr int kClassicPanelY = kClassicTop;
constexpr int kClassicPanelW = 64;
constexpr int kClassicPanelH =
    Framebuffer::H - kClassicTop - kClassicBorder;  // to the bottom border
constexpr int kClassicViewX = kClassicPanelX + kClassicPanelW + 2;
constexpr int kClassicViewY = kClassicTop;
constexpr int kClassicViewW = Framebuffer::W - kClassicViewX - kClassicBorder;
constexpr int kClassicViewH = Framebuffer::H - kClassicTop - 14;

std::string join(const std::string& dir, const std::string& name) {
    if (dir.empty()) {
        return name;
    }
    char last = dir.back();
    if (last == '/' || last == '\\') {
        return dir + name;
    }
    return dir + "/" + name;
}

bool has_extension(const std::string& name) {
    size_t dot = name.find_last_of('.');
    size_t slash = name.find_last_of("/\\");
    return dot != std::string::npos &&
           (slash == std::string::npos || dot > slash);
}

std::string board_stem(const std::string& name) {
    size_t slash = name.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? name : name.substr(slash + 1);
    size_t dot = base.find_last_of('.');
    if (dot != std::string::npos) {
        base.resize(dot);
    }
    return base;
}

// Numeric board names carry the level id (1.jn2 = level 1); most Jill 2/3
// headers store 0, so the file name is the reliable source.
bool numeric_stem(const std::string& name, int& value) {
    const std::string stem = board_stem(name);
    if (stem.empty() || stem.size() > 5) {
        return false;
    }
    int v = 0;
    for (char c : stem) {
        if (c < '0' || c > '9') {
            return false;
        }
        v = v * 10 + (c - '0');
    }
    value = v;
    return true;
}

bool same_board_name(const std::string& a, const std::string& b) {
    std::string x = board_stem(a);
    std::string y = board_stem(b);
    if (x.size() != y.size()) {
        return false;
    }
    for (size_t i = 0; i < x.size(); ++i) {
        char c = x[i];
        char d = y[i];
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
        if (d >= 'A' && d <= 'Z') {
            d = static_cast<char>(d - 'A' + 'a');
        }
        if (c != d) {
            return false;
        }
    }
    return true;
}

// ---- .vcl text pages ------------------------------------------------------

constexpr int kPageVisibleLines = 16;

uint8_t page_colour(uint8_t code) {
    // The original's line renderer accepts 0-7 and defaults to 7.  Colour 0
    // would be black and colour 1 is dim, so lift those two for readability.
    if (code == 1) {
        return 9;
    }
    if (code == 0) {
        return 8;
    }
    return code;
}

// The original window/HUD frame uses shape entry 3: frames 0-15 are the
// noise-pattern edges/corners and fill, already in the episode's palette.

void classic_box(Framebuffer& fb, int x, int y, int w, int h, uint8_t accent) {
    fb.fill_rect(x, y, w, h, 0);
    fb.fill_rect(x, y, w, 1, 8);
    fb.fill_rect(x, y + h - 1, w, 1, 8);
    fb.fill_rect(x, y, 1, h, 8);
    fb.fill_rect(x + w - 1, y, 1, h, 8);
    if (accent != 0) {
        fb.fill_rect(x + 1, y + 1, w - 2, 1, accent);
    }
}

// Body lines (everything after the title), word-wrapped at 48 characters so
// long prose stays inside the frame.  The shipped pages already break at ~38.
// A board's start music is the first `*name.ddt` / `*name.ark` CHECKPT string.
// Later music CHECKPTs are section changes and are still handled when touched.
bool is_music_name(const std::string& name) {
    if (name.size() < 5) {
        return false;
    }
    std::string ext;
    for (size_t i = name.size() - 4; i < name.size(); ++i) {
        ext.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(name[i]))));
    }
    return ext == ".ddt" || ext == ".ark";
}

std::string first_board_music(const std::vector<BoardObject>& objects) {
    for (const BoardObject& o : objects) {
        if (o.kind != 12 || o.text.empty() || o.text[0] != '*') {
            continue;
        }
        std::string name = o.text.substr(1);
        while (!name.empty() &&
               (name.back() == ' ' || name.back() == '\r')) {
            name.pop_back();
        }
        if (is_music_name(name)) {
            return name;
        }
    }
    return std::string();
}

std::vector<TextPage::Line> page_body(const TextPage& page) {
    std::vector<TextPage::Line> out;
    const size_t first = page.lines.empty() ? 0 : 1;
    for (size_t i = first; i < page.lines.size(); ++i) {
        TextPage::Line line = page.lines[i];
        while (line.text.size() > 48) {
            size_t cut = line.text.rfind(' ', 48);
            if (cut == std::string::npos || cut < 24) {
                cut = 48;
            }
            out.push_back(TextPage::Line{line.colour,
                                         line.text.substr(0, cut)});
            size_t next = cut;
            if (next < line.text.size() && line.text[next] == ' ') {
                ++next;
            }
            line.text = line.text.substr(next);
        }
        out.push_back(std::move(line));
    }
    return out;
}

}  // namespace

bool Engine::init(const GameDef& game, std::string& err) {
    if (!game.resolved()) {
        err = "game '" + game.id + "' has no engine profile";
        return false;
    }
    game_ = game;
    profile_ = game.profile;
    data_dir_ = game.data_dir;
    if (!tiles_.load(join(data_dir_, game_.tile_defs), err)) {
        return false;
    }
    if (!shapes_.load(join(data_dir_, game_.shape_file), err)) {
        return false;
    }
    // Player colour table (entry 8).  Each episode ships the same art with a
    // different 16-entry translation table, which is the only difference
    // between Jill 1/2/3's outfits (Jill Select reads other games' tables).
    for (int i = 0; i < 16; ++i) {
        player_base_xlate_[static_cast<size_t>(i)] = static_cast<uint8_t>(i);
    }
    read_entry_xlate(join(data_dir_, game_.shape_file), 8,
                     player_base_xlate_.data());
    player_raw_xlate_ = player_base_xlate_;
    player_look_xlate_ = player_base_xlate_;
    // Optional: the sampled-sound bank also carries the text pages.  A mod
    // without one simply has no help/news screens.
    pages_ = TextPages{};
    page_open_ = false;
    page_number_ = 0;
    page_scroll_ = 0;
    if (!game_.pages_file.empty()) {
        std::string perr;
        if (!pages_.load(join(data_dir_, game_.pages_file), perr)) {
            std::fprintf(stderr, "text pages: %s\n", perr.c_str());
        }
    }
    refresh_save_labels();
    return true;
}

bool Engine::load_board(const std::string& name, std::string& err) {
    std::string file = name;
    if (!has_extension(file)) {
        file += game_.board_ext;
    }
    if (!board_.load(join(data_dir_, file), err)) {
        return false;
    }
    if (board_.objects.empty()) {
        err = file + ": board has no objects";
        return false;
    }
    has_player_ = board_.objects[0].kind == 0 || board_.objects[0].kind == 23;
    pending_warp_.clear();
    music_request_.clear();
    last_music_.clear();
    sound_queue_.clear();
    last_facing_ = 0;
    msg_text_.clear();
    msg_ttl_ = 0;
    msg_shown_ = 0;
    page_open_ = false;
    page_number_ = 0;
    page_scroll_ = 0;
    apple_hint_ = false;
    switch_hint_ = false;
    knight_hint_ = false;
    elev_hint_ = false;
    for (bool& hint : item_hint_) {
        hint = false;
    }

    board_name_ = file;
    // The shipped boards mark nearly every cell 0xC000: those bits are the
    // original's "tile needs redrawing" flags (cleared in FUN_26f4_0d52), not
    // collision data.  The port redraws every frame, so strip them on load and
    // reserve the bits for the per-frame object footprints only.
    map_ = board_.map;
    for (uint16_t& cell : map_) {
        cell &= 0x3FFF;
    }
    header_ = board_.header;
    objects_ = board_.objects;
    // Start the level on the track its first music CHECKPT names.  The
    // original does not wait for the player to walk onto that trigger: board 1
    // begins on zeppelin.ddt, the hub on funky.ddt, and so on.
    music_request_ = first_board_music(objects_);
    last_music_ = music_request_;

    // Level id: derive it from a numeric board name first (Jill 2/3 store 0 in
    // most headers), otherwise trust the header (map/intro boards).
    int level = header_[0] | (header_[1] << 8);
    int numbered = 0;
    if (numeric_stem(file, numbered)) {
        level = numbered;
        header_[0] = static_cast<uint8_t>(level & 0xFF);
        header_[1] = static_cast<uint8_t>((level >> 8) & 0xFF);
    }
    // MAPDEMO (kind 67) is the animated map preview; it only runs on the hub /
    // world-map board (original guard: DGROUP:0x34EB != 0).
    map_mode_ = same_board_name(file, game_.map_board);
    if (map_mode_) {
        hub_valid_ = false;  // a fresh hub load discards any stashed copy
        hub_level_ = level;
        hub_name_ = file;
    }

    // Level-title caption (36-entry table, index = level id + 4).  Shown as a
    // dialog box over the board; the reveal speed is the dialog speed.
    caption_.clear();
    caption_ttl_ = 0;
    caption_hold_ = 0;
    caption_shown_ = 0;
    text_reveal_ = 0;
    update_caption_for_level(level);
    // Keep references stable: handlers may append objects (score popups,
    // particles) while they hold a reference to an existing record.
    for (BoardObject& o : objects_) {
        o.prev_x = o.x;
        o.prev_y = o.y;
    }
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    objects_.reserve(std::max<size_t>(
        static_cast<size_t>(kObjectLimit), objects_.size()));
    active_.clear();
    order_.clear();
    frame_ = 0;

    spawn_x_ = has_player_ ? objects_[0].x : 0;
    spawn_y_ = has_player_ ? objects_[0].y : 0;
    cam_x_ = std::max(0, std::min(spawn_x_ + 8 - view_center_x(),
                                  Board::kCols * 16 - view_w()));
    cam_y_ = std::max(0, std::min(spawn_y_ + 16 - view_center_y(),
                                  Board::kRows * 16 - view_h()));
    if (!has_player_) {
        cam_x_ = 0;
        cam_y_ = 0;
    }
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    return true;
}

void Engine::update_caption_for_level(int level) {
    caption_.clear();
    caption_ttl_ = 0;
    caption_hold_ = 0;
    caption_shown_ = 0;
    const int title_index = level + 4;
    if (profile_->level_titles && title_index >= 0 &&
        title_index < profile_->title_count &&
        profile_->level_titles[title_index] &&
        profile_->level_titles[title_index][0]) {
        caption_ = profile_->level_titles[title_index];
        caption_ttl_ = fast_dialogs_ ? 0x50 : 0x78;
        caption_hold_ = 0x28;  // a held direction must not eat the popup
    }
}

void Engine::set_input(int h, int v, bool btn1_down, bool btn2_down) {
    // While a recorded macro is playing the original skips the normal keyboard
    // poll entirely (1415:093B) and only the script drives the inputs.
    if (macro_playing_) {
        return;
    }
    in_.h = h < -1 ? -1 : (h > 1 ? 1 : h);
    in_.v = v < -1 ? -1 : (v > 1 ? 1 : v);

    // Any input dismisses the level-title caption early (the original's title
    // screen waits for a key/timer).  A short hold keeps the popup visible when
    // the player is still holding a direction from the previous level.
    if ((h != 0 || v != 0 || btn1_down || btn2_down) && caption_hold_ == 0) {
        caption_ttl_ = 0;
    }
    // Same for the classic Reminder card.
    if ((h != 0 || v != 0 || btn1_down || btn2_down) && box_hold_ == 0) {
        box_ttl_ = 0;
    }

    // The original latches are cleared when the physical button is released;
    // the player sets them when it consumes a jump/throw (26F4 tail).
    if (!btn1_down) {
        latch1_ = false;
    }
    if (!btn2_down) {
        latch2_ = false;
    }
    in_.btn1 = btn1_down && !latch1_;
    in_.btn2 = btn2_down && !latch2_;
}

void Engine::tick() {
    if (page_open_) {
        return;  // the original blocks in the text viewer
    }
    if (turtle_) {
        // Jill 3's TURTLE option slows the whole simulation: one full update
        // every other 18.2 Hz tick (about 9.1 Hz).  `slow_phase_` tells
        // render() to interpolate the previous/current pair over two ticks,
        // so movement stays continuous instead of move-hold-move-hold.
        turtle_skip_ = !turtle_skip_;
        if (turtle_skip_) {
            slow_phase_ = 1;
            return;
        }
    }
    slow_phase_ = 0;
    compact_objects();
    // Snapshot for high-framerate interpolation (previous logic tick).
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    for (BoardObject& o : objects_) {
        o.prev_x = o.x;
        o.prev_y = o.y;
    }
    ++frame_;
    process_pending_warp();
    if (macro_playing_) {
        apply_macro_input();
    }
    tile_prepass();
    update_pass();
    follow_camera();

    // _putbotmsg timer (26F4:259B): decrement once per tick, clear at 0.
    if (msg_ttl_ > 0 && --msg_ttl_ == 0) {
        msg_text_.clear();
    }
    // Level-title caption and TEXT6/TEXT8 page reveal (faster dialogs = 4/tick).
    if (caption_hold_ > 0) {
        --caption_hold_;
    }
    if (caption_ttl_ > 0) {
        if (caption_shown_ < caption_.size()) {
            caption_shown_ = std::min(caption_.size(),
                                      caption_shown_ + static_cast<size_t>(dialogs_per_tick_));
        }
        --caption_ttl_;
    }
    if (text_reveal_ < 0x400) {
        text_reveal_ += static_cast<size_t>(dialogs_per_tick_);
    }
    if (caption_ttl_ > 0 && box_ttl_ > 0) {
        box_hold_ = 8;  // re-arm when the caption finishes and the card appears
    } else if (box_hold_ > 0) {
        --box_hold_;
    }
    if (box_ttl_ > 0 && caption_ttl_ <= 0) {
        --box_ttl_;
    }
}

void Engine::tile_prepass() {
    // FUN_26f4_11d7: tiles with attribute bit 0x20 run the 1F28:0006 mode-2
    // hook.  The only mode-2 effect is the 5-frame lava cycle, advanced once
    // every 4th frame (the hook tests `frame & 3 == 0`).
    int c0 = cam_x_ >> 4;
    int c1 = std::min(Board::kCols - 1, (cam_x_ + view_w()) >> 4);
    int r0 = cam_y_ >> 4;
    int r1 = std::min(Board::kRows - 1, (cam_y_ + view_h()) >> 4);
    bool advance = (frame_ & 3) == 0;
    for (int c = c0; c <= c1; ++c) {
        for (int r = r0; r <= r1; ++r) {
            uint16_t& cell = map_[c * Board::kRows + r];
            uint16_t id = cell & 0x3FFF;
            if (!(tiles_.at(id).flags & 0x0020)) {
                continue;
            }
            if (advance && id >= 0x186 && id <= 0x18A) {  // LAVA1..LAVA5
                id = (id >= 0x18A) ? 0x186 : static_cast<uint16_t>(id + 1);
                cell = static_cast<uint16_t>(id | 0xC000);
            }
            // 1F28:01E0: BLINK (0x81) is the blue fading block.  Every 8th
            // frame (phase = frame>>3 & 0xF) the original rewrites tile 0x81's
            // own record: the shape becomes bank-41 frame blink[phase] and
            // flags bit 1 is set only for phases 8..12.  Bit 1 is the
            // "walk-through" bit of the mask-3 test used by the airborne
            // mover, so the block is solid while it is visible and falls away
            // while it is invisible ("fade in = stand on it").  Frames 8..12
            // are the blue block art, frame 38 is blank.  Verified table
            // (identical in all three episodes): img 0x1E178 / 0x1E544 /
            // 0x1EA72.
            if (id == 0x81 && (frame_ & 7) == 2) {
                static const uint8_t kBlink[16] = {8, 8, 8, 8, 9, 10, 11, 12,
                                                   38, 38, 38, 38, 38, 11, 10, 9};
                const int phase = (frame_ >> 3) & 0x0F;
                TileInfo& rec = tiles_.at(0x81);
                rec.shape = static_cast<uint16_t>((rec.shape & 0xFF00) |
                                                  kBlink[phase]);
                rec.flags |= 2;
                if (phase < 8 || phase > 12) {
                    rec.flags ^= 2;
                }
                cell = static_cast<uint16_t>(cell | 0x4000);
            }
            // Mode-0 cell repaint: SPIKE (0xA6) uses base_shape + ((frame>>2)&3),
            // a four-frame pop-up cycle (1F28:08BF).  The mode-2 pass only marks
            // the cell for repaint once every fourth frame; the port redraws
            // every frame, so animating the shared tile record is enough.
            if (id == 0xA6) {
                TileInfo& rec = tiles_.at(0xA6);
                const int phase = (frame_ >> 2) & 3;
                rec.shape = static_cast<uint16_t>(tiles_.base_shape(0xA6) + phase);
                cell = static_cast<uint16_t>(cell | 0x4000);
            }
            // 1f28:0006 mode 2 flame emitters.  FFLOOR/FROOF run a 32-frame
            // phase keyed on the column and spit a FIRE object on the single
            // frame where the phase equals 0x10; WOODTORCH fires when the
            // frame and column low bits line up.
            if (id == 0x15F || id == 0x160) {
                const int phase = (id == 0x160) ? ((frame_ - c) & 0x1F)
                                                : ((frame_ + c) & 0x1F);
                if (phase == 0x10) {
                    const int fy = (id == 0x15F) ? (r * 16 - 0x20)
                                                 : (r * 16 + 0x10);
                    int n = add_obj(0x1F, c * 16, fy);
                    if (n >= 0) {
                        objects_[static_cast<size_t>(n)].dy =
                            (id == 0x15F) ? -1 : 1;
                    }
                    sound(3, 0x1A);
                }
            } else if (id == 0x21 && (frame_ & 0x3F) == (c & 0x3F)) {
                int n = add_obj(0x1F, c * 16, r * 16 - 0x18);
                if (n >= 0) {
                    objects_[static_cast<size_t>(n)].dy = -1;
                }
                sound(3, 0x1A);
            }
        }
    }
}

// 26F4:23E1 _tilehit: run the per-tile contact hook for every cell under the
// object's AABB whose tile has flag bit 3 (`flags & 8`).  The original only
// calls this for the player, from the end of the player handler.
void Engine::tile_touch(int i) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    // state_flags[state] & 2 means invulnerable / already dying.
    static const uint16_t kStateFlags[6] = {1, 0, 1, 0, 2, 2};
    if (o.kind == 0 && o.state >= 0 && o.state < 6 &&
        (kStateFlags[o.state] & 2)) {
        return;
    }
    int r0 = o.y >> 4;
    int r1 = (o.y + o.h + 0x0F) >> 4;
    int c0 = o.x >> 4;
    int c1 = (o.x + o.w + 0x0F) >> 4;
    for (int r = r0; r < r1; ++r) {
        for (int c = c0; c < c1; ++c) {
            if (c < 0 || c >= Board::kCols || r < 0 || r >= Board::kRows) {
                continue;
            }
            uint16_t id = map_[c * Board::kRows + r] & 0x3FFF;
            if (tiles_.at(id).flags & 0x0008) {
                tile_hook(c, r, 1);
            }
        }
    }
}

// 1F28:0006 _tilehook(col,row,mode): mode 1 is the contact test (damage),
// mode 0 repaints a cell and mode 2 is the decorative pre-pass.  The port
// implements the contact cases; mode 0/2 only touch the 0x4000 redraw flags
// this port does not need.
void Engine::tile_hook(int col, int row, int mode) {
    if (mode != 1 || !has_player_) {
        return;
    }
    int idx = col * Board::kRows + row;
    if (idx < 0 || idx >= static_cast<int>(map_.size())) {
        return;
    }
    uint16_t id = map_[idx] & 0x3FFF;
    if (id >= 0x186 && id <= 0x18B) {  // LAVA1..LAVAB
        player_kill(0x10, 1);
        explode2(0);
        return;
    }
    if (id == 0x141 || id == 0x142 || id == 0x155 || id == 0x156 ||
        id == 0xA6) {  // stalactite/stalagmite, thorns, SPIKE
        player_kill(0x10, 2);
        return;
    }
    if (id >= 0x190 && id <= 0x197) {  // water surface
        int k = objects_[0].kind;
        if (k != 0x36 && k != 0x39) {  // fish / frog forms can swim
            player_kill(0x10, 1);
        }
        return;
    }
    if (id >= 0x198 && id <= 0x1A3) {  // water body: splash sound only
        if (frame_ - water_sound_ > 0x0A) {
            sound(2, 4);
            water_sound_ = frame_;
        }
        return;
    }
}

void Engine::update_pass() {
    active_.clear();
    if (objects_.empty()) {
        return;
    }
    int first = 0;
    if (has_player_) {
        active_.push_back(0);  // the player is always first
        objects_[0].flags &= ~0xC000;
        first = 1;
    }

    for (int i = first; i < static_cast<int>(objects_.size()); ++i) {
        BoardObject& o = objects_[static_cast<size_t>(i)];
        bool visible =
            o.x + o.w >= cam_x_ - 0x60 && o.x <= cam_x_ + view_w() + 0x60 &&
            o.y + o.h >= cam_y_ - 0x30 && o.y <= cam_y_ + view_h() + 0x30;
        if (visible && active_.size() < static_cast<size_t>(kActiveLimit)) {
            active_.push_back(i);
            o.flags &= ~0xC000;
        } else {
            o.flags |= 0xC000;
        }
    }

    for (int i : active_) {
        BoardObject& o = objects_[static_cast<size_t>(i)];
        if (o.ttl > 0) {
            --o.ttl;
        }
        if (o.kind == 0 || o.kind == 23 ||
            (o.kind >= 54 && o.kind <= 57)) {
            if (debug_fly_) {
                // Free positioning for testing: move the player directly and
                // skip the normal state machine (no collision, no gravity).
                BoardObject& p = objects_[static_cast<size_t>(i)];
                p.x = static_cast<int16_t>(std::max(
                    0, std::min(Board::kCols * 16 - 16, p.x + in_.h * 4)));
                p.y = static_cast<int16_t>(std::max(
                    0, std::min(Board::kRows * 16 - 32, p.y + in_.v * 4)));
                p.flags |= 0xC000;
            } else if (o.kind == 23) {
                tiny_update(i);
            } else if (o.kind == 0) {
                player_update(i);
            } else {
                form_update(i);
            }
        } else if (update_object(i)) {
            objects_[static_cast<size_t>(i)].flags |= 0xC000;
        }
        // The hit pass is symmetric: a kind with 0x0008 tests itself against
        // every other listed object and both sides receive action 1
        // (FUN_26f4_12b9 0x28448).  Pairs where both sides carry 0x0008 are
        // handled once.
        const BoardObject& a0 = objects_[static_cast<size_t>(i)];
        if (a0.kind < profile_->kind_limit() &&
            (profile_->kind(a0.kind).flags & 0x0008)) {
            for (int j : active_) {
                if (j == i) {
                    continue;
                }
                const BoardObject& b0 = objects_[static_cast<size_t>(j)];
                if (b0.kind < profile_->kind_limit() &&
                    (profile_->kind(b0.kind).flags & 0x0008) && j < i) {
                    continue;
                }
                const BoardObject& aa = objects_[static_cast<size_t>(i)];
                const BoardObject& bb = objects_[static_cast<size_t>(j)];
                // Recorded ending macros were made in the original with a
                // slightly different sprite alignment; give token pickups a
                // few pixels of slack so the recorded path still collects
                // them (e.g. the ending's JILL icon).
                const int slack =
                    (macro_playing_ &&
                     ((aa.kind == 28 && aa.p13 == 0) ||
                      (bb.kind == 28 && bb.p13 == 0)))
                        ? 10
                        : 0;
                if (aa.x - slack < bb.x + bb.w && bb.x < aa.x + aa.w + slack &&
                    aa.y - slack < bb.y + bb.h && bb.y < aa.y + aa.h + slack) {
                    bool ra = on_hit(i, j);
                    bool rb = on_hit(j, i);
                    if (ra) {
                        objects_[static_cast<size_t>(i)].flags |= 0xC000;
                    }
                    if (rb) {
                        objects_[static_cast<size_t>(j)].flags |= 0xC000;
                    }
                }
            }
        }
    }

    // Blocking objects become temporary map walls for the rest of the frame.
    for (int i : active_) {
        const BoardObject& o = objects_[static_cast<size_t>(i)];
        if (!(o.flags & 0xC000)) {
            continue;
        }
        int r0 = o.y >> 4;
        int r1 = (o.y + o.h + 0x0F) >> 4;
        int c0 = o.x >> 4;
        int c1 = (o.x + o.w + 0x0F) >> 4;
        for (int r = r0; r < r1; ++r) {
            for (int c = c0; c < c1; ++c) {
                if (c >= 0 && c < Board::kCols && r >= 0 && r < Board::kRows) {
                    map_[c * Board::kRows + r] |= 0xC000;
                }
            }
        }
    }

    // Final sweep: an object sitting entirely on blocked cells is blocked.
    for (int i : active_) {
        BoardObject& o = objects_[static_cast<size_t>(i)];
        bool blocked = false;
        int r0 = o.y >> 4;
        int r1 = (o.y + o.h + 0x0F) >> 4;
        int c0 = o.x >> 4;
        int c1 = (o.x + o.w + 0x0F) >> 4;
        for (int r = r0; r < r1; ++r) {
            for (int c = c0; c < c1; ++c) {
                if (c >= 0 && c < Board::kCols && r >= 0 && r < Board::kRows &&
                    (map_[c * Board::kRows + r] & 0xC000)) {
                    blocked = true;
                }
            }
        }
        if (blocked) {
            o.flags |= 0xC000;
        }
    }

    order_ = active_;
    // Sort by y for normal depth, but keep wall switches (kind 32) behind
    // everything: the original draws the player in front of the switch panel.
    auto depth = [this](int i) {
        const BoardObject& o = objects_[static_cast<size_t>(i)];
        return o.kind == 32 ? -0x10000 : static_cast<int>(o.y);
    };
    std::stable_sort(order_.begin(), order_.end(), [&](int a, int b) {
        return depth(a) < depth(b);
    });
}

// Action 2 ("update yourself") for the ported kinds.  Returns true when the
// handler reports "blocked/done" and the pass should OR 0xC000 into flags.
bool Engine::update_object(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    switch (o.kind) {
        case 1:  // APPLE: bob the animation, return 1 on even p13
            o.p13 = (o.p13 + ((o.dx > 0) ? 1 : -1)) & 7;
            return (o.p13 & 1) == 0;

        case 14:  // KEY: 8-frame blink
        case 33:  // GEM: sparkle
            o.p13 = (o.p13 + 1) & 7;
            return (o.p13 & 1) == 0;

        case 2: {  // KNIFE (1644:0fff)
            int p11 = o.p11;
            if (p11 < 1) {
                if (p11 == -1) {
                    // Fall one pixel until it lands; then it stays put and can
                    // be picked up again (p11 == 0).
                    if (move_obj(i, o.x, o.y + 1) == 0) {
                        objects_[static_cast<size_t>(i)].p11 = 0;
                    }
                }
                return true;
            }
            ++o.p11;
            if (o.p11 > 0x0E) {
                // Home on the player: add the sign of the offset, then clamp.
                const BoardObject& p = objects_[0];
                int sx = (o.x < p.x) - (p.x < o.x);
                int sy = (o.y < p.y) - (p.y < o.y);
                o.dx += sx;
                if (o.dx < 9) {
                    if (o.dx < -7) {
                        o.dx = -8;
                    }
                } else {
                    o.dx = 8;
                }
                o.dy += sy;
                if (o.dy < 5) {
                    if (o.dy < -3) {
                        o.dy = -4;
                    }
                } else {
                    o.dy = 4;
                }
            }
            int moved = move_obj(i, o.x + o.dx, o.y + o.dy);
            if (moved == 0 || o.p11 > 0x40) {
                objects_[static_cast<size_t>(i)].p11 = -1;
            }
            return true;
        }

        case 27: {  // SCORE popup (1644:3482)
            --o.p13;
            if (o.p13 >= 0) {
                bool onscreen =
                    o.x + o.w >= cam_x_ && o.x <= cam_x_ + view_w() &&
                    o.y + o.h >= cam_y_ && o.y <= cam_y_ + view_h();
                if (onscreen) {
                    set_origin(i, o.x + o.dx, o.y + o.dy);
                    objects_[static_cast<size_t>(i)].dy -= 1;
                    return true;
                }
            }
            kill_obj(i);
            return true;
        }

        case 49:  // EPIC: never moves, never expires (1a41:29fa)
            return true;

        case 36: {  // EXPL1 (1a41:0a3b): ballistic debris, no collision
            ++o.p13;
            bool onscreen =
                o.x + o.w >= cam_x_ && o.x <= cam_x_ + view_w() &&
                o.y + o.h >= cam_y_ && o.y <= cam_y_ + view_h();
            if (o.p13 < 0x28 && onscreen) {
                if (o.dy < 0x0C) {
                    ++o.dy;
                }
                set_origin(i, o.x + o.dx, o.y + o.dy);
                return true;
            }
            kill_obj(i);
            return true;
        }

        case 0x32: {  // SPINBLAD (1a41:2b1a): homing blade, 0x40 ticks
            o.p13 = ((o.dx > 0) - (o.dx < 0) + o.p13) & 3;
            ++o.p0f;
            if (o.p0f >= 0x40 || !onscreen(i)) {
                kill_obj(i);
                return true;
            }
            // Nearest object flagged 0x400 (killable/valid target) within 0x60.
            int target = -1;
            int best = 0x7FFF;
            for (int j : active_) {
                if (j == 0 || j == i) {
                    continue;
                }
                const BoardObject& t = objects_[static_cast<size_t>(j)];
                if (!(profile_->kind(t.kind).flags & 0x0400)) {
                    continue;
                }
                const int d = dist(i, j);
                if (d < best && d < 0x60) {
                    best = d;
                    target = j;
                }
            }
            int vx = 0;
            int vy = 0;
            if (target >= 0) {
                vectdist(target, i, vx, vy, 1);
            }
            o.dx += vx;
            o.dy += 1;  // the blade always falls
            o.dx = std::max(-0x0C, std::min(o.dx, 0x0C));
            o.dy = std::max(-0x0C, std::min(o.dy, 0x0C));
            int dx = o.dx;
            int dy = o.dy;
            const int x = o.x;
            const int y = o.y;
            int p0f = o.p0f;
            // touch_bkgnd can spawn an EXPL1, so re-fetch the record after it.
            p0f += touch_bkgnd(i, x + dx, y + dy) * 10;
            BoardObject& b = objects_[static_cast<size_t>(i)];
            b.p0f = p0f;
            if (!just_move(i, x + dx, y)) {
                dx = -dx;
            }
            // 1a41:2b1a snaps to `(y+dy) & ~0x0F + 0x10 - DAT_2cff_b7a4`;
            // b7a4 is 0x10, so the extra +0x10 cancels out.  The vertical
            // probes use the object's x *after* the horizontal just_move.
            const int row_bottom = (y + dy) & ~0x0F;
            if (!just_move(i, b.x, y + dy)) {
                sound(1, 0x23);
                if (y == row_bottom || !just_move(i, b.x, row_bottom)) {
                    dy = -dy;
                }
            }
            b.dx = dx;
            b.dy = dy;
            if (!just_move(i, b.x, b.y)) {
                kill_obj(i);
            }
            return true;
        }

        case 0x3E: {  // FIREBULLET (1a41:436d): baked arc, inert to Jill
            if (!onscreen(i)) {
                kill_obj(i);
                return true;
            }
            static const int kArc[20] = {3,  2,  1,  1,  0,  -1, -1, -2, -2, -3,
                                         -3, -2, -2, -1, -1, 0,  1,  1,  2,  3};
            BoardObject& b = objects_[static_cast<size_t>(i)];
            b.p13 = (b.p13 + 1) & 3;
            b.state = (b.state + 1) % 0x14;
            touch_bkgnd(i, b.x, b.y);
            b = objects_[static_cast<size_t>(i)];
            if (!just_move(i, b.x + b.dx,
                           b.y + b.dy + kArc[b.state % 20])) {
                kill_obj(i);
                return true;
            }
            ++b.p0f;
            if (b.p0f > 0x4F) {
                kill_obj(i);
            }
            return true;
        }

        case 0x3F:  // FISHBULLET (1a41:45ed): flat, gravity-free shot
            if (!onscreen(i)) {
                kill_obj(i);
                return true;
            }
            if (!just_move(i, o.x + o.dx, o.y)) {
                kill_obj(i);
            }
            return true;

        case 4:  // BIGANT (1644:13cf)
            if (o.state == 0) {
                o.p13 = (o.p13 + 1) % 5;
                if (crawl(i, o.dx, 0) == 0) {
                    o.state = 5;
                    o.dx = -o.dx;
                }
            } else {
                --o.state;
            }
            return true;

        case 5: {  // FLY (1644:1789): drift, bounce off walls
            const HandlerTables& t = *profile_->tables;
            int idx = o.p13 & 3;
            if (!just_move(i, o.x + o.dx, o.y + t.fly_y[idx])) {
                o.dx = -o.dx;
                sound(1, 0x0F);
            }
            o.p13 = (o.p13 + 1) & 7;
            return true;
        }

        case 7: {  // DEMON (1644:2974): walks and spits FIREBALLs
            o.p13 = (o.p13 + 1) % 12;
            bool moved = just_move(i, o.x + o.dx, o.y + o.dy);
            if (!moved || rnd(0x24) == 0) {
                int vx = 0;
                int vy = 0;
                vectdist(0, i, vx, vy, 4);
                o.dx = vx;
                o.dy = vy;
                int idx = add_obj(0x12, o.x, o.y);
                if (idx >= 0) {
                    objects_[static_cast<size_t>(idx)].dx = o.dx;
                    objects_[static_cast<size_t>(idx)].dy = o.dy;
                }
                if (rnd(2) == 0) {
                    o.dx = rnd(5) - 2;
                    o.dy = rnd(5) - 2;
                } else {
                    o.dx /= 2;
                    o.dy /= 2;
                }
            }
            if (o.p11 > 0) {
                --o.p11;
            }
            return true;
        }

        case 8:    // BUNNY
        case 22: {  // FROG (shared handler 1644:36ee)
            if (o.state == 0) {
                ++o.p13;
                if (o.p13 > 0x10) {
                    o.state = 1;
                    int sx = 0;
                    int sy = 0;
                    pointvect(i, sx, sy);
                    o.dx = sx << 2;
                    o.dy = -10;
                }
                return o.p13 > 0x10;
            }
            ++o.dy;
            if (o.dy > 10) {
                o.dy = 10;
            }
            int nx = o.x + o.dx;
            int ny = o.y + o.dy;
            if ((move_obj(i, nx, ny) & 3) == 0) {
                if (o.dy < 0) {
                    o.dy = 0;
                } else {
                    move_obj(i, nx, ((ny & ~0x0F) + 0x10) - o.h);
                    int sx = 0;
                    int sy = 0;
                    pointvect(i, sx, sy);
                    o.dx = sx;
                    o.state = 0;
                    o.p13 = 0;
                    sound(1, 0x12);
                }
            }
            return true;
        }

        case 9:  // INCHWORM (1644:1c33): 4-tick stride
            o.p13 = (o.p13 + 1) & 3;
            if (o.p13 != 0) {
                return false;
            }
            o.p11 ^= 1;
            if (crawl(i, o.dx, 0) == 0) {
                o.dx = -o.dx;
            }
            return true;

        case 10:  // ZAPPER (1644:1d91): static 5-frame hazard
            o.p13 = (o.p13 + 1) % 5;
            return true;

        case 11:  // BOBSLUG (1644:1e50)
            if (o.state == 0) {
                if (crawl(i, o.dx, 0) == 0) {
                    o.state = 1;
                }
                return true;
            }
            o.p11 ^= 1;
            if (o.p11 & 1) {
                return false;
            }
            ++o.state;
            if (o.state > 5) {
                o.state = 0;
                o.dx = -o.dx;
            }
            return true;

        case 17: {  // FATSO (1644:2d43): pounce at long intervals
            o.p13 = (o.p13 + 1) & 0x0F;
            if (o.p13 & 1) {
                return false;
            }
            if (rnd(0x1E) == 0) {
                int sx = 0;
                int sy = 0;
                pointvect(i, sx, sy);
                o.dy = 0;
                o.dx = sx << 2;
            }
            if (crawl(i, o.dx, 0) == 0) {
                o.dx = -o.dx;
            } else {
                sound(1, 0x11);
            }
            return true;
        }

        case 18:  // FIREBALL (1644:3105): flies until off screen or blocked
            if (!onscreen(i)) {
                kill_obj(i);
                return true;
            }
            o.p13 = (o.p13 + 1) & 7;
            if (!just_move(i, o.x + o.dx, o.y + o.dy)) {
                kill_obj(i);
            }
            return true;

        case 19:  // CLOUD (1644:324e): slow horizontal drift
            o.p13 = (o.p13 + 1) & 0x0F;
            if (!(o.p13 & 1)) {
                return false;
            }
            if (!just_move(i, o.x + o.dx, o.y)) {
                o.dx = -o.dx;
            }
            return true;

        case 24:  // DOOR (1644:3a00): runs the opening animation
            if (o.state == 0) {
                return false;
            }
            ++o.state;
            if (o.state > 0x10) {
                kill_obj(i);
            }
            return true;

        case 25: {  // FALLDOOR (1644:3da3): drops and becomes a tile
            if (o.state == 0) {
                return false;
            }
            int col = o.x >> 4;
            int row = o.y >> 4;
            if (!just_move(i, o.x, o.y + 4)) {
                sound(3, 0x0E);
                if (col >= 0 && col < Board::kCols && row >= 0 && row < Board::kRows) {
                    map_[col * Board::kRows + row] =
                        static_cast<uint16_t>((o.p13 & 0x3FFF) | 0xC000);
                }
                kill_obj(i);
            } else if ((o.y & 0x0F) == 0) {
                if (col >= 0 && col < Board::kCols && row >= 1 && row < Board::kRows) {
                    map_[col * Board::kRows + row - 1] =
                        static_cast<uint16_t>((o.p13 & 0x3FFF) | 0xC000);
                }
            }
            return true;
        }

        case 29:  // ANT (1644:15a8)
            if (o.state == 0) {
                o.p13 = (o.p13 + 1) % 4;
                if (crawl(i, o.dx, 0) == 0) {
                    o.state = 2;
                    o.dx = -o.dx;
                }
            } else {
                --o.state;
            }
            return true;

        case 30:  // PHOENIX (1644:198a): patrols horizontally
            if (cando(i, o.x, o.y, 1) == 0) {
                return false;
            }
            ++o.p13;
            if (o.p13 == 6 || o.p13 > 7) {
                o.p13 = 0;
            }
            if (!just_move(i, o.x + o.dx, o.y)) {
                o.dx = -o.dx;
                sound(1, 0x0F);
            } else if (cando(i, o.x + o.dx, o.y, 1) == 0) {
                o.p13 = 6;
            }
            return true;

        case 35: {  // BOULDER (1a41:05f6): rolls and falls
            if (cando(i, o.x, o.y + 1, 3) == 3) {
                o.dy += 2;
                if (o.dy > 0x0C) {
                    o.dy = 0x0C;
                }
                int ny = o.y + o.dy;
                if (move_obj(i, o.x + o.dx, ny) != 1) {
                    move_obj(i, o.x, ((ny - 1) & ~0x0F) + 0x10 - o.h);
                    o.dx = 0;
                }
            } else {
                if (o.dy != 0) {
                    sound(2, 0x2D);
                }
                o.dy = 0;
                if (o.dx == 0) {
                    int sx = 0;
                    int sy = 0;
                    pointvect(i, sx, sy);
                    o.dx = sx << 2;
                }
                o.p13 = (o.p13 + (o.dx < 1 ? -1 : 1)) & 3;
                o.dy = 0;
                if (move_obj(i, o.x + o.dx, o.y) != 1) {
                    o.dx = -o.dx;
                }
            }
            return true;
        }

        case 37:  // EXPL2 (1a41:0b91): stationary burst
            ++o.p13;
            if (o.p13 > 8 || !onscreen(i)) {
                kill_obj(i);
            }
            return true;

        case 38:  // STALAG (1a41:0c54): falls once a trigger sets dy
            if (o.dy == 0) {
                return false;
            }
            if (!just_move(i, o.x, o.y + o.dy)) {
                sound(2, 0x1B);
                kill_obj(i);
            }
            o.dy += 2;
            if (o.dy > 0x10) {
                o.dy = 0x10;
            }
            return true;

        case 42:  // MEGA (1a41:165a): bouncing block, started by trigger
            if (o.state == 0) {
                return false;
            }
            o.dy += 2;
            if (o.dy > 0x10) {
                o.dy = 0x10;
            }
            if (!just_move(i, o.x, o.y + o.dy)) {
                o.dy = 2 - o.dy;
                if (o.dy == 0) {
                    return false;
                }
                sound(2, 0x1C);
            }
            return true;

        case 43:  // BAT (1a41:17ee): hangs, then flutters
            o.p13 = (o.p13 + 1) & 7;
            if (o.state == 0) {
                if (rnd(0x28) != 0) {
                    return false;
                }
                o.state = 1;
                o.dx = (rnd(2) * 8) - 4;
                o.dy = 2;
            } else {
                int moved = move_obj(i, o.x + o.dx, o.y + o.dy);
                if (moved == 1) {
                    if (rnd(0x23) == 0) {
                        o.dx = -o.dx;
                    }
                    if (rnd(0x14) == 0) {
                        o.dy = -o.dy;
                    }
                } else if (moved == 2) {
                    o.dx = -o.dx;
                } else {
                    o.dx = -o.dx;
                    if (rnd(3) == 0 && o.dy < 0) {
                        o.state = 0;
                        move_obj(i, o.x, o.y & ~0x0F);
                    } else {
                        o.dy = -o.dy;
                    }
                }
            }
            return true;

        case 47:  // CRAB (1a41:22f3): walks, sometimes drops a ledge row
            o.p13 = (o.p13 + 1) & 3;
            if (o.state == 0) {
                if (o.dx == 0) {
                    o.dx = (rnd(2) * 8) - 4;
                }
                if ((o.x & 0x0F) == 0 && cando(i, o.x, o.y, 4) == 0 &&
                    rnd(2) == 0) {
                    o.state = 1;
                }
                if (o.state == 0 && crawl(i, o.dx, 0) == 0) {
                    o.dx = -o.dx;
                }
            }
            if (o.state == 1) {
                if (o.dy == 0) {
                    o.dy = (rnd(2) * 4) - 2;
                }
                if (!just_move(i, o.x, o.y + o.dy)) {
                    o.dy = -o.dy;
                    just_move(i, o.x, o.y + o.dy);
                }
                if ((o.y & 0x0F) == 0) {
                    if (crawl(i, o.dx, 0) == 0) {
                        if (crawl(i, -o.dx, 0) != 0) {
                            o.state = 0;
                            o.dx = -o.dx;
                        }
                    } else {
                        o.state = 0;
                    }
                }
            }
            return true;

        case 52:  // BUTTON (1a41:3261): debounce timer
            if (o.p0f > 0) {
                --o.p0f;
            }
            return false;

        case 16:  // WISEMAN (1644:234b): pace, one step every other tick
            if (o.p13 == 0) {
                o.state = (o.state + 1) & 3;
                o.p13 = 1;
                if (crawl(i, o.dx, 0) == 0) {
                    o.dx = -o.dx;
                }
            } else {
                --o.p13;
            }
            return true;

        case 20:  // TEXT6 (1644:3371): static board text, p13 = lifetime
        case 21:  // TEXT8 (1644:35df)
            if (o.p13 > 0) {
                --o.p13;
                if (o.p13 == 0) {
                    kill_obj(i);
                    return true;
                }
            }
            return false;

        case 34:  // TXTMSG (1a41:4c8f): message cooldown
            if (o.state > 0) {
                --o.state;
                return true;
            }
            return false;

        case 64: {  // EYE (1a53:483a): pupil follows the player
            int dxv = 0;
            int dyv = 0;
            vectdist(0, i, dxv, dyv, 3);
            const int ny = (dyv >= 1) ? 1 : -1;
            const int nx = std::max(dxv, -2);
            if (nx != o.dx || ny != o.dy) {
                o.dx = nx;
                o.dy = ny;
                return true;
            }
            return false;
        }

        case 65:  // VINECLIMB (1a53:49b1): patrol the climbable span
            o.p13 = (o.p13 + 1) & 7;
            // The handler only seeds dy when it is zero; it does NOT restart a
            // downward stroke every 8 ticks (the earlier port read p13 here,
            // which made the sparks jitter instead of travelling the vine).
            if (o.dy == 0) {
                o.dy = 2;
            }
            if (cando(i, o.x, o.y + o.dy, 4) || o.y + o.dy < 0 ||
                o.y + o.dy > Board::kRows * 16) {
                o.dy = -o.dy;
            } else {
                set_origin(i, o.x, o.y + o.dy);
            }
            return true;

        case 66:  // FLAG (1a53:4baa): 6-tick animation, no movement
            o.p13 = (o.p13 + 1) % 6;
            return (o.p13 & 1) == 0;

        case 67:  // MAPDEMO (1a53:4df2): pinned to the viewport corner
            if (!map_mode_) {
                return false;  // only runs on the hub / world-map board
            }
            o.x = cam_x_ + 0x10;
            o.y = cam_y_ + 4;
            return true;

        case 68:  // ROMAN (1650:2f6b): ground walker, re-aims occasionally
            o.p13 = (o.p13 + 1) & 7;
            if (rnd(30) == 0) {
                int sx = 0;
                int sy = 0;
                pointvect(i, sx, sy);
                o.dx = sx;
            }
            o.dy = 0;
            if (crawl(i, o.dx, 0) != 0) {
                sound(1, 0x11);
            } else {
                o.dx = -o.dx;
            }
            return true;

        case 31:  // FIRE (1a41:0160): six-frame flame, expires after 0x0B
            ++o.p13;
            if (o.p13 > 0x0B || o.p13 < 0) {
                kill_obj(i);
            }
            return (o.p13 & 1) == 0;

        case 39:  // SNAKE (1a41:0dcb): three-part crawler
            if (o.state > 0) {
                --o.state;
            }
            o.p13 = (o.p13 + 1) & 3;
            if (crawl(i, o.dx, 0) == 0) {
                o.dx = -o.dx;
            }
            return true;

        case 40:  // SEAROCK (1a41:46d5): bubble emitter
            if (rnd(12) == 0) {
                add_obj(0x3A, o.x + 2, o.y + 4);
                return true;
            }
            return false;

        case 41: {  // BOLL (1a41:1261): seeks the next boll, rolls downhill
            const HandlerTables& t = *profile_->tables;
            int ti = o.p13;
            if (ti < 0 || ti > 5) {
                ti = 0;
            }
            o.w = t.boll[ti];
            o.h = t.boll[ti];
            int target = -1;
            int best = 0x60;
            for (int j : active_) {
                if (j == i || objects_[static_cast<size_t>(j)].kind != 41) {
                    continue;
                }
                if (objects_[static_cast<size_t>(j)].p13 <= o.p13) {
                    continue;
                }
                const int d = dist(i, j);
                if (d < best) {
                    best = d;
                    target = j;
                }
            }
            int vx = o.dx;
            int vy = 1;
            if (target >= 0) {
                vectdist(target, i, vx, vy, 1);
            }
            o.dx += vx;
            o.dy += vy;
            o.dx = std::max(-0x0C, std::min(o.dx, 0x0C));
            o.dy = std::max(-0x0C, std::min(o.dy, 0x0C));
            if (!just_move(i, o.x + o.dx, o.y)) {
                o.dx = -o.dx;
            }
            if (!just_move(i, o.x, o.y + o.dy)) {
                int ny = ((o.y + o.dy) & ~0x0F) + 0x10 - o.h;
                if (!just_move(i, o.x, ny)) {
                    o.dy = -o.dy;
                    sound(1, 9);
                }
            }
            return true;
        }

        case 44:  // KNIGHT (1a41:1bb4): lever-driven swing, never moves
            o.state = (o.p11 == 6) ? 1 : 0;
            if ((frame_ & 1) == 0) {
                return false;
            }
            if (o.p11 == o.dx) {
                return false;
            }
            ++o.p11;
            if (o.p11 >= 0x0A) {
                o.p11 = 0;
                o.dx = 0;
                return true;
            }
            return false;

        case 45: {  // BEENEST (1a41:1e13): aims, then spits a BEESWARM
            if ((frame_ & 3) != 2) {
                return true;
            }
            if (o.p13 == 0) {
                if (rnd(0x20) == 0) {
                    int sx = 0;
                    int sy = 0;
                    pointvect(i, sx, sy);
                    o.dx = sx;
                    o.dy = sy;
                    o.p13 = 1;
                }
            } else {
                ++o.p13;
                if (o.p13 > 2) {
                    o.p13 = 0;
                    add_obj(0x2E, o.x + o.dx * 8, o.y);
                }
            }
            return true;
        }

        case 46: {  // BEESWARM (1a41:2069): jittered homing, ignores walls
            const HandlerTables& t = *profile_->tables;
            ++o.state;
            if (o.state >= 0xA1) {
                kill_obj(i);
                return true;
            }
            o.p13 = (o.p13 + 1) % 3;
            int sx = 0;
            int sy = 0;
            pointvect(i, sx, sy);
            o.dx = sx;
            o.dy = sy;
            o.dx = (o.dx * 2 + rnd(2)) * t.beeswarm[o.state & 31] / 2;
            o.dy = (o.dy * 2 + rnd(2)) * t.beeswarm[(o.state + 16) & 31] / 2;
            just_move(i, o.x + o.dx, o.y + o.dy);
            return true;
        }

        case 48:  // CROC (1a41:269f): two-part crawler, only moves while awake
            if (o.state > 0) {
                --o.state;
            }
            o.p13 = (o.p13 + 1) & 3;
            if (crawl(i, o.dx, 0) == 0) {
                o.dx = -o.dx;
            }
            return true;

        case 51:  // SKULL (1a41:3029): frozen until triggered
            if (o.state == 0) {
                return false;
            }
            o.p11 = (o.p11 + 1) & 7;
            return true;

        case 53: {  // PAC (1a41:3402): corridor follower on tile centres
            if ((o.x & 0x0F) != 0 || (o.y & 0x0F) != 0) {
                move_obj(i, o.x + o.dx, o.y + o.dy);
                return true;
            }
            int col = o.x >> 4;
            int row = o.y >> 4;
            auto cell = [&](int c, int r) -> int {
                if (c < 0 || c >= Board::kCols || r < 0 || r >= Board::kRows) {
                    return -1;
                }
                return map_[c * Board::kRows + r] & 0x3FFF;
            };
            const int cur = cell(col, row);
            int sx = (o.dx > 0) - (o.dx < 0);
            int sy = (o.dy > 0) - (o.dy < 0);
            if (sx == 0 && sy == 0) {
                if (rnd(2) == 0) {
                    sx = 1;
                } else {
                    sy = 1;
                }
            }
            // 1a41:3402: the forward test rejects a different tile, then the
            // turn tests ACCEPT a candidate whose tile equals the current one
            // (the corridor is a run of identical tile ids).
            if (cell(col + sx, row + sy) != cur) {
                int sign = rnd(2) * 2 - 1;
                int tx = sy * sign;
                int ty = sx * sign;
                if (cell(col + tx, row + ty) != cur) {
                    if (cell(col - tx, row - ty) != cur) {
                        if (cell(col - sx, row - sy) != cur) {
                            sx = 0;
                            sy = 0;
                        } else {
                            sx = -sx;
                            sy = -sy;
                        }
                    } else {
                        sx = -tx;
                        sy = -ty;
                    }
                } else {
                    sx = tx;
                    sy = ty;
                }
            }
            o.dx = o.p13 * sx;
            o.dy = o.p13 * sy;
            move_obj(i, o.x + o.dx, o.y + o.dy);
            return true;
        }

        case 58:  // BUBBLE (1a41:3834): grows, wobbles, pops out of water
            if (rnd(15) == 0) {
                ++o.p13;
            }
            if (o.p13 < 3 && onscreen(i)) {
                if (!fishdo(i, o.x + (rnd(3) - 1), o.y)) {
                    kill_obj(i);
                }
            } else {
                kill_obj(i);
            }
            return true;

        case 59:  // JELLYFISH (1a41:3957): drifting kick, bounces off walls
            ++o.p13;
            if (o.p13 > 9) {
                o.p13 = 0;
                o.dx += (rnd(3) - 1) * ((std::abs(o.dx) < 3) ? 2 : 1);
                o.dx = std::max(-6, std::min(o.dx, 6));
                o.dy += (rnd(3) - 1) * (((o.dy < 2) ? 1 : 0) + 1);
                o.dy = std::max(-6, std::min(o.dy, 6));
                if (!fishdo(i, o.x + o.dx, o.y)) {
                    o.dx = -o.dx;
                }
                if (!fishdo(i, o.x, o.y + o.dy)) {
                    o.dy = -o.dy;
                }
            }
            return true;

        case 60:  // BADFISH (1a41:3d03): swims, blows bubbles
            ++o.p13;
            if (rnd(20) == 0) {
                o.dx = rnd(3) * 4 - 4;
                o.dy = (o.dx == 0) ? (rnd(2) * 4 - 2) : (rnd(3) * 2 - 2);
            }
            if (!fishdo(i, o.x + o.dx, o.y)) {
                o.dx = -o.dx;
            }
            if (!fishdo(i, o.x, o.y + o.dy)) {
                o.dy = -o.dy;
            }
            if (rnd(4) == 0) {
                add_obj(0x3A, o.x + 6, o.y - 2);
            }
            return true;

        case 61: {  // ELEV (1a41:3fe8): carries the 0x8A floor tile
            if (o.state > 0) {
                --o.state;
            }
            int col = o.x >> 4;
            int row = o.y >> 4;
            if (o.state == 0 && o.p13 != 0xFFFF && col >= 0 &&
                col < Board::kCols && row + 1 < Board::kRows &&
                (map_[col * Board::kRows + row + 1] & 0x3FFF) == 0x8A) {
                map_[col * Board::kRows + row + 1] =
                    static_cast<uint16_t>((map_[col * Board::kRows + row] & 0x3FFF) | 0xC000);
                set_origin(i, o.x, o.y + 0x10);
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

// Actions 3/4/5: the three _sendtrig codes.  Returns true when the handler
// reported "blocked/done" (the pass then marks 0xC000).
bool Engine::trig_object(int i, int code, int self) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    switch (o.kind) {
        case 24: {  // DOOR (1644:3a00): key or gem
            if (code != 3 || o.state != 0) {
                return false;
            }
            int col = o.x >> 4;
            int row = o.y >> 4;
            if (col < 0 || col >= Board::kCols || row < 0 || row >= Board::kRows) {
                return false;
            }
            uint16_t& cell = map_[col * Board::kRows + row];
            if ((cell & 0x3FFF) == 0x0BE) {
                if (!del_item(3)) {
                    return false;
                }
                sound(3, 0x24);
                int sc = col + o.dx;
                int sr = row + o.dy;
                uint16_t v = 0;
                if (sc >= 0 && sc < Board::kCols && sr >= 0 && sr < Board::kRows) {
                    v = static_cast<uint16_t>(map_[sc * Board::kRows + sr] & 0x3FFF);
                }
                cell = static_cast<uint16_t>(v | 0xC000);
                kill_obj(i);
                return true;
            }
            if (!del_item(1)) {
                return false;
            }
            sound(3, 0x0C);
            o.state = 1;
            for (int k = 0; k < 2; ++k) {
                int r = row + k;
                if (col >= 1 && r >= 0 && r < Board::kRows) {
                    uint16_t v = static_cast<uint16_t>(
                        map_[(col - 1) * Board::kRows + r] & 0x3FFF);
                    map_[col * Board::kRows + r] =
                        static_cast<uint16_t>(v | 0xC000);
                }
            }
            return true;
        }

        case 25: {  // FALLDOOR (1644:3da3)
            if (code != 3) {
                return false;
            }
            o.state = 1;
            int col = o.x >> 4;
            int row = o.y >> 4;
            if (col >= 0 && col < Board::kCols && row >= 1 && row < Board::kRows) {
                o.p13 = map_[col * Board::kRows + row - 1] & 0x3FFF;
            }
            return true;
        }

        case 26: {  // BRIDGER (1644:2503): build / erase a run of tiles
            int col = o.x >> 4;
            int row = o.y >> 4;
            int stepc = (o.dx > 0) - (o.dx < 0);
            int stepr = (o.dy > 0) - (o.dy < 0);
            int newtile = (o.dx == 0) ? 0x8C : 0x1B0;
            if (stepc == 0 && stepr == 0) {
                return false;
            }
            bool neutral = (code == 3);
            if (neutral) {
                bool has = col >= 0 && col < Board::kCols && row >= 0 &&
                           row < Board::kRows &&
                           (map_[col * Board::kRows + row] & 0x3FFF) == newtile;
                code = has ? 4 : 5;
            }
            if (code == 5) {
                uint16_t startid = 0;
                if (col >= 0 && col < Board::kCols && row >= 0 && row < Board::kRows) {
                    startid = static_cast<uint16_t>(
                        map_[col * Board::kRows + row] & 0x3FFF);
                }
                for (int c = col, r = row;
                     c >= 0 && c < Board::kCols && r >= 0 && r < Board::kRows;
                     c += stepc, r += stepr) {
                    uint16_t& cell = map_[c * Board::kRows + r];
                    if ((cell & 0x3FFF) != startid) {
                        break;
                    }
                    cell = static_cast<uint16_t>(newtile | 0xC000);
                }
            } else if (code == 4) {
                uint16_t restore = 0;
                int nc = col + stepr;
                int nr = row + stepc;
                if (nc >= 0 && nc < Board::kCols && nr >= 0 && nr < Board::kRows) {
                    uint16_t id = map_[nc * Board::kRows + nr] & 0x3FFF;
                    if ((tiles_.at(id).flags & 3) == 3) {
                        restore = id;
                    }
                }
                if (restore == 0) {
                    nc = col - stepr;
                    nr = row - stepc;
                    if (nc >= 0 && nc < Board::kCols && nr >= 0 && nr < Board::kRows) {
                        uint16_t id = map_[nc * Board::kRows + nr] & 0x3FFF;
                        if ((tiles_.at(id).flags & 3) == 3) {
                            restore = id;
                        }
                    }
                }
                for (int c = col, r = row;
                     c >= 0 && c < Board::kCols && r >= 0 && r < Board::kRows;
                     c += stepc, r += stepr) {
                    uint16_t& cell = map_[c * Board::kRows + r];
                    if ((cell & 0x3FFF) != newtile) {
                        break;
                    }
                    cell = static_cast<uint16_t>(restore | 0xC000);
                }
            }
            if (neutral && self >= 0 && self < static_cast<int>(objects_.size()) &&
                objects_[static_cast<size_t>(self)].kind == 15) {
                kill_obj(self);
            }
            return true;
        }

        case 38:  // STALAG (1a41:0c54): actions 3/5 release the stalactite
            if (code != 3 && code != 5) {
                return false;  // action 4 returns immediately
            }
            if (o.dy == 0) {
                o.dy = 2;
                sound(3, 0x2F);
            }
            if (self >= 0 && self < static_cast<int>(objects_.size())) {
                kill_obj(self);
            }
            return true;

        case 42:  // MEGA (1a41:165a): trigger starts the fall
            if (code == 3) {
                o.state = 1;
                return true;
            }
            return false;

        case 44:  // KNIGHT (1a41:1bb4): lever selects the swing hold frame
            if (code == 3) {
                o.dx = 0x0A;
                return true;
            }
            if (code == 4) {
                o.dx = 0;
                return true;
            }
            if (code == 5) {
                o.dx = 6;
                return true;
            }
            return false;

        case 51:  // SKULL (1a41:3029): action 3 starts the animation
            if (code == 3 && o.state == 0) {
                o.state = 1;
                o.p11 = 0;
                sound(4, 0x21);
                return true;
            }
            return false;

        default:
            return false;
    }
}

// Action 1 ("was hit by other") for one side of an overlap.  Returns true when
// the original handler's return value is non-zero (the pass then marks 0xC000).
bool Engine::on_hit(int a, int b) {
    if (a < 0 || b < 0 || a == b) {
        return false;
    }
    BoardObject& o = objects_[static_cast<size_t>(a)];
    BoardObject& other = objects_[static_cast<size_t>(b)];
    const int other_kind = other.kind;

    switch (o.kind) {
        case 0:  // PLAYER: action 1 just reports the other object's kind
            return other_kind != 0;

        case 1:  // APPLE (1644:0e14)
            if (o.state < 1) {
                if (b == 0) {
                    if (!apple_hint_) {
                        put_bottom_msg("APPLES GIVE YOU HEALTH");
                        apple_hint_ = true;
                    }
                    int hp = health();
                    if (hp < 8) {
                        header_[2] = static_cast<uint8_t>(hp + 1);
                        header_[3] = 0;
                    }
                    add_score(profile_->kind(1).count, o.x, o.y);
                    objects_[static_cast<size_t>(a)].flags |= 0xC000;
                    sound(2, 0x0B);
                    kill_obj(a);
                    return true;
                }
                return false;
            }
            if (o.ttl == 0) {
                sound(6, 0x19);
                // Page apple: state is a .vcl text page number (the "News
                // bulletin" and the Onesimus verses are placed this way).
                show_text_page(o.state);
            }
            o.ttl = 4;
            kill_obj(a);
            return true;

        case 2:  // KNIFE (1644:0fff)
            if (b == 0) {
                if ((o.p11 < 1 || o.p11 > 10) && inv_count(2) < max_knives_) {
                    add_item(2);
                    sound(2, 7);
                    kill_obj(a);
                    return true;
                }
                return true;
            }
            if ((profile_->kind(other_kind).flags & 0x0080) && o.p11 > 0) {
                o.dx = 0;
                o.dy = 0;
                o.p11 = 0x0F;
                add_score(profile_->kind(other_kind).count, other.x, other.y);
                kill_obj(b);
                sound(3, 10);
            }
            return true;

        case 0x32:  // SPINBLAD (1a41:2b1a)
            if (b == 0) {
                // It never hurts Jill; after a few ticks it just expires on
                // contact.
                if (o.p0f > 7) {
                    kill_obj(a);
                }
                return true;
            }
            if (profile_->kind(other_kind).flags & 0x0400) {
                kill_obj(a);
                killaward(b);
                sound(3, 10);
            }
            return true;

        case 0x3E:  // FIREBULLET
            if (b != 0 && (profile_->kind(other_kind).flags & 0x0080)) {
                kill_obj(a);
                killaward(b);
                sound(3, 10);
            }
            return true;

        case 0x3F:  // FISHBULLET: no action-1 branch
            return true;

        case 12: {  // CHECKPT (1644:1fe2): entrance / level-exit trigger
            if (b != 0) {
                return false;
            }
            if (o.state == 3) {
                // Ending marker (50.jn1 / 14.jn3): the original calls
                // FUN_1415_06eb and sets the quit flag (DGROUP:0xBCAA = 2),
                // which fades back to the front end.  The app turns this into
                // a return to the main menu.
                ending_done_ = true;
                macro_playing_ = false;
                return true;
            }
            // p13 == 0 is the map music trigger, not a level entrance; it
            // always fires even when the hub's stored level id is also 0.
            // Music triggers (`*track.ddt`) must also fire when their p13
            // matches the current level: board entrances place Jill on the
            // checkpoint that carries the level's start track.
            const bool music_cmd = !o.text.empty() && o.text[0] == '*';
            if (music_cmd) {
                // A music checkpoint only changes the track.  Do NOT move or
                // re-enter the player: the spawn checkpoint is touched every
                // tick, and resetting state 4 there left Jill stuck in the
                // entrance rise (and hidden under the floor) forever.
                pending_warp_ = o.text;
                return true;
            }
            if (o.p13 != 0 && o.p13 == level_id()) {
                return false;
            }
            // Snapshot the fields before touching the player record.
            const int level = o.p13;
            const int cx = o.x;
            const int cy = o.y;
            const int sx = o.dx;
            const int sy = o.dy;
            const std::string text = o.text;
            sound(4, level == 0 ? 5 : level + 0x32);
            if (level != 0) {
                header_[0] = static_cast<uint8_t>(level & 0xFF);
                header_[1] = static_cast<uint8_t>((level >> 8) & 0xFF);
            }
            BoardObject& p = objects_[0];
            if (p.kind == 0x17) {
                // Map Jill: checkpoint dx/dy is a respawn cache.  The shipped
                // map boards leave it (0,0) until it is filled, so only honour
                // a populated value; otherwise the avatar was being moved to
                // (0,0), which is not map-walkable and left her stuck there
                // after returning from a level.
                if (sx != 0 || sy != 0) {
                    set_origin(0, sx, sy);
                }
            } else {
                p.x = cx;
                p.y = cy - 0x10;
                p.state = 4;
                p.p11 = 0;
                p.dx = 0;
                p.dy = 0;
            }
            if (!text.empty()) {
                pending_warp_ = text;
            }
            return true;
        }

        case 14:  // KEY (1644:2790)
            if (b == 0) {
                add_item(1);
                sound(3, 6);
                kill_obj(a);
                return true;
            }
            return false;

        case 33:  // GEM (1a41:04b2)
            if (b == 0) {
                add_item(3);
                sound(3, 0x10);
                add_score(profile_->kind(33).count, o.x, o.y);
                kill_obj(a);
                return true;
            }
            return false;

        case 28: {  // TOKEN (1a41:000f): inventory pickup, p13 = item id
            // p13 1/2/3/8/9/10 add a key/knife/gem/spinblade/feather/shield
            // (the first pickup of each type also shows a hint in the original;
            // the port has no message line).  p13 0 and 4..7 additionally run
            // 1FAE:2E61, which morphs Jill back / into the fish, spider, bird
            // and frog forms - those forms are not ported yet, so the inventory
            // entry is still granted.
            if (b != 0) {
                return false;
            }
            // First pickup of an item type shows the original hint line
            // (DGROUP:0x0F00 far-pointer table; values documented in
            // docs/jill1-kind-handlers.md #28).
            static const char* const kHints[16] = {
                "FOOF!",
                "USE KEYS TO OPEN DOORS",
                "YOU FOUND A KNIFE",
                "YOU FOUND A GEM",
                nullptr, nullptr, nullptr, nullptr,
                "YOU FOUND A SPINNING BLADE",
                "EXTRA JUMPING POWER",
                "SHIELD OF INVINCIBILITY",
                nullptr, nullptr, nullptr, nullptr, nullptr,
            };
            if (o.p13 >= 0 && o.p13 < 16 && !item_hint_[o.p13] &&
                kHints[o.p13]) {
                put_bottom_msg(kHints[o.p13]);
                item_hint_[o.p13] = true;
            }
            if (o.p13 == 0 || (o.p13 >= 4 && o.p13 <= 7)) {
                morph_player(o.p13);  // 1FAE:2E61
            }
            add_item(o.p13);
            sound(3, 0x19);
            kill_obj(a);
            return true;
        }

        case 49:  // EPIC (1a41:29fa): eleven player touches, then it explodes
            if (b == 0) {
                add_score(profile_->epic_score, o.x, o.y);
                o.state = static_cast<uint16_t>(o.state + 1);
                if (o.state < 0x0B) {
                    sound(2, 0x20);
                } else {
                    explode1(o.x, o.y, 10);
                    kill_obj(a);
                    sound(3, 0x30);
                }
                return true;
            }
            return false;

        case 4:   // BIGANT
        case 8:   // BUNNY
        case 9:   // INCHWORM
        case 11:  // BOBSLUG
        case 17:  // FATSO
        case 22:  // FROG
        case 29:  // ANT
        case 43:  // BAT
        case 47:  // CRAB
            if (b == 0) {
                hit_player(a);
                return true;
            }
            return false;

        case 5:  // FLY (1644:1789)
            if (b == 0) {
                explode1(o.x, o.y, 4);
                player_kill(1, 0);
                kill_obj(a);
                return true;
            }
            return false;

        case 7:  // DEMON (1644:2974)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if (profile_->kind(other_kind).flags & 0x4000) {
                if (o.state == 0) {
                    ++o.p11;
                    if (o.p11 < 6) {
                        explode1(o.x, o.y, 4);
                    } else {
                        explode2(a);
                        kill_obj(a);
                        return true;
                    }
                }
                o.state = 4;
                return true;
            }
            return false;

        case 10:  // ZAPPER (1644:1d91): lethal, no cooldown check
            if (b == 0) {
                player_kill(0x10, 2);
                return true;
            }
            return false;

        case 31:  // FIRE (1a41:0160): one-shot burn
            if (b == 0 && o.state != 1) {
                player_kill(2, 0);
                explode2(0);
                objects_[static_cast<size_t>(a)].state = 1;
                return true;
            }
            return false;

        case 39:  // SNAKE (1a41:0dcb)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if ((profile_->kind(other_kind).flags & 0x4000) && o.state == 0) {
                stamp_footprint(a);
                o.state = 0x10;
                o.h -= 8;
                if (o.h < 0x18) {
                    killaward(a);
                } else {
                    sound(3, 0x1F);
                }
                return true;
            }
            return false;

        case 41:  // BOLL (1a41:1261)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            return false;

        case 44:  // KNIGHT (1a41:1bb4): hazard, immune to weapons
            if (b == 0) {
                if (o.state == 1) {
                    put_bottom_msg("The knight slices Jill in half", 7);
                    player_kill(0x10, 2);
                }
                return true;
            }
            if (profile_->kind(other_kind).flags & 0x4000) {
                if (!knight_hint_) {
                    put_bottom_msg("YOUR FEEBLE ATTEMPT FAILS.", 2);
                    knight_hint_ = true;
                }
                return false;
            }
            return false;

        case 45:  // BEENEST (1a41:1e13)
            if (profile_->kind(other_kind).flags & 0x4000) {
                add_score(profile_->kind(45).count, o.x, o.y);
                explode1(o.x, o.y, 10);
                kill_obj(a);
                sound(3, 0x20);
                return true;
            }
            return false;

        case 46:  // BEESWARM (1a41:2069)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            return false;

        case 48:  // CROC (1a41:269f)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if ((profile_->kind(other_kind).flags & 0x4000) && o.state == 0) {
                stamp_footprint(a);
                o.state = 0x10;
                int sx = 0;
                int sy = 0;
                pointvect(a, sx, sy);
                if (o.dx * sx < 1) {
                    killaward(a);
                } else {
                    o.dx = -o.dx;
                }
                return true;
            }
            return false;

        case 51:  // SKULL (1a41:3029): no hit branch
            return false;

        case 53:  // PAC (1a41:3402)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if (other_kind == 0x3E) {  // FIREBULLET
                kill_obj(a);
                return true;
            }
            return false;

        case 58:  // BUBBLE (1a41:3834): no hit branch
            return true;

        case 59:  // JELLYFISH (1a41:3957)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if (profile_->kind(other_kind).flags & 0x4000) {
                explode1(o.x, o.y, 0x14);
                kill_obj(a);
                return true;
            }
            return false;

        case 60:  // BADFISH (1a41:3d03)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            if (profile_->kind(other_kind).flags & 0x4000) {
                killaward(a);
                return true;
            }
            return false;

        case 61: {  // ELEV (1a41:3fe8): ride up/down on contact
            if (b != 0) {
                return false;
            }
            const int col = o.x >> 4;
            const int row = o.y >> 4;
            o.state = 6;
            if (!elev_hint_) {
                put_bottom_msg("Press UP/DOWN to use elevator", 2);
                elev_hint_ = true;
            }
            BoardObject& p = objects_[0];
            const int px = p.x;
            if (in_.v < 0) {
                p.dy = 0;
                if (o.p0f != in_.v) {
                    sound(2, 0x1D);
                }
                if (just_move(0, px, (row - 2) << 4)) {
                    set_origin(a, o.x, o.y - 0x10);
                    if (col >= 0 && col < Board::kCols && row >= 0 &&
                        row < Board::kRows) {
                        map_[col * Board::kRows + row] = 0xC08A;
                    }
                }
            } else if (in_.v > 0) {
                p.dy = 0;
                if (o.p0f != in_.v) {
                    sound(2, 0x1E);
                }
                if (col >= 0 && col < Board::kCols && row + 1 < Board::kRows &&
                    (map_[col * Board::kRows + row + 1] & 0x3FFF) == 0x8A) {
                    map_[col * Board::kRows + row + 1] = static_cast<uint16_t>(
                        (map_[col * Board::kRows + row] & 0x3FFF) | 0xC000);
                    set_origin(a, o.x, o.y + 0x10);
                    just_move(0, px, row << 4);
                }
            }
            o.p0f = in_.v;
            return true;
        }

        case 6:  // MACROTRIG (1a41:4c75): blocks the player
            return true;

        case 15:  // PAD (1644:28a4): tells its trigger channel it was touched
            if (b == 0) {
                int code = (o.state == -1) ? 4 : (o.state == 1 ? 5 : 3);
                send_trig(o.p13, code, a);
                return true;
            }
            return false;

        case 18:  // FIREBALL (1644:3105)
            if (b == 0) {
                hit_player(a);
                explode2(0);  // original: _explode2(0) = at the player
                return true;
            }
            return false;

        case 30:  // PHOENIX (1644:198a)
            if (b == 0) {
                explode1(o.x, o.y, 4);
                player_kill(1, 0);
                explode2(0);  // original: _explode2(0) = at the player
                kill_obj(a);
                return true;
            }
            return false;

        case 32:  // SWITCH (1a41:02ab): up/down toggles the channel
            if (b != 0) {
                return false;
            }
            if (in_.v != 0) {
                objects_[0].dy = 0;
                if (!switch_hint_) {
                    put_bottom_msg("PRESS UP/DOWN TO TOGGLE SWITCH");
                    switch_hint_ = true;
                }
            }
            if (in_.v < 0 && o.state == 1) {
                o.state = 0;
                sound(2, 0x17);
                send_trig(o.p13, o.dx == 1 ? 3 : 5, a);
            } else if (in_.v > 0 && o.state == 0) {
                o.state = 1;
                sound(2, 0x18);
                send_trig(o.p13, o.dx == 1 ? 3 : 4, a);
            }
            return true;

        case 35:  // BOULDER (1a41:05f6)
            if (b == 0) {
                hit_player(a);
                return true;
            }
            return false;

        case 38:  // STALAG (1a41:0c54): 2 damage
            if (b == 0) {
                player_kill(2, 2);
                return true;
            }
            return false;

        case 52:  // BUTTON (1a41:3261)
            if (o.p0f == 0) {
                o.state = 1 - o.state;
                if (o.state == 1) {
                    send_trig(o.p13, 4, a);
                    sound(3, 0x2C);
                } else {
                    send_trig(o.p13, 5, a);
                    sound(3, 0x2C);
                }
            }
            o.p0f = 3;
            return true;

        case 23:  // TINY (map Jill): handler ignores the hit action
            return true;

        case 16:  // WISEMAN (1644:234b): talking NPC
            if (b == 0 && o.ttl == 0) {
                put_bottom_msg(o.text, 7);
                o.ttl = 3;
                return true;
            }
            return false;

        case 34:  // TXTMSG (1a41:4c8f): per-board message, gated by state
            if (b == 0 && o.state == 0) {
                put_bottom_msg(o.text, 7);
                sound(3, 0x14);
                o.state = 8;
                return true;
            }
            return false;

        case 65: {  // VINECLIMB (1a53:49b1): push the player off, then hurt
            if (b != 0) {
                return false;
            }
            BoardObject& p = objects_[0];
            const bool invulnerable = p.state == 4 || p.state == 5;
            just_move(0, p.x + (invulnerable ? 8 : -8), p.y);
            p.state = 0;
            p.p13 += 0x0F;
            hit_player(a);
            return true;
        }

        case 66:  // FLAG (1a53:4baa): one-shot score pickup
            if (b == 0 && o.state == 0) {
                o.state = 1;
                add_score(6, o.x, o.y);
                sound(2, 0x20);
                explode1(o.x, o.y, 5);
                return true;
            }
            return false;

        case 68:  // ROMAN (1650:2f6b): contact damage, guarded by p1b
            if (b == 0) {
                if (o.p1b == 0) {
                    hit_player(a);
                }
                return true;
            }
            return false;

        case 64:  // EYE: scenery, no hit response
        case 67:  // MAPDEMO: overlay, no hit response
            return false;

        default:
            return false;
    }
}

void Engine::kill_obj(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    o.flags |= 0xC000;
    o.kind = 3;  // KILLME
}

// FUN_26f4_2506: at the end of the frame the original compacts the object
// array, dropping every kind-3 record.  Without this the port keeps corpses
// forever, so add_obj eventually hits the 0x100 cap and weapons/particles stop
// appearing.  Called at the start of tick(), when the previous frame's render
// has finished and no stale index is in use.
void Engine::compact_objects() {
    size_t w = 0;
    for (size_t r = 0; r < objects_.size(); ++r) {
        if (objects_[r].kind == 3) {
            continue;
        }
        if (w != r) {
            objects_[w] = std::move(objects_[r]);
        }
        ++w;
    }
    if (w != objects_.size()) {
        objects_.resize(w);
    }
}

void Engine::add_score(int n, int x, int y) {
    uint32_t s = score() + static_cast<uint32_t>(n);
    header_[0x26] = static_cast<uint8_t>(s & 0xFF);
    header_[0x27] = static_cast<uint8_t>((s >> 8) & 0xFF);
    header_[0x28] = static_cast<uint8_t>((s >> 16) & 0xFF);
    header_[0x29] = static_cast<uint8_t>((s >> 24) & 0xFF);

    int idx = add_obj(27, x, y);
    if (idx < 0) {
        return;
    }
    BoardObject& p = objects_[static_cast<size_t>(idx)];
    p.state = n;
    p.p13 = 0x10;
    int px = has_player_ ? objects_[0].x : 0;
    p.dy = ((px < x) ? 1 : 0) - ((x < px) ? 1 : 0);
    p.dy *= 2;
    // _setobjsize: SCORE width follows the digit count.
    int digits = 1;
    for (int v = n; v >= 10; v /= 10) {
        ++digits;
    }
    p.w = static_cast<uint16_t>(digits * 4);
}

// 1A41:09A8: spawn `n` EXPL1 objects (kind 36) with a random spread.
void Engine::explode1(int x, int y, int n) {
    for (int i = 0; i < n; ++i) {
        int idx = add_obj(36, x, y);
        if (idx < 0) {
            return;
        }
        BoardObject& e = objects_[static_cast<size_t>(idx)];
        e.dx = rnd(7) - 3;
        e.dy = rnd(0x0B) - 8;
        e.state = static_cast<uint16_t>(rnd(3));
    }
}

int Engine::rnd(int mod) {
    // The original FUN_2ca6_0015 is a table-driven generator; a small xorshift
    // gives the same spread without pulling in its private state.
    rng_ ^= rng_ << 13;
    rng_ ^= rng_ >> 17;
    rng_ ^= rng_ << 5;
    if (mod <= 0) {
        return 0;
    }
    return static_cast<int>(rng_ % static_cast<uint32_t>(mod));
}

int Engine::view_x() const {
    return hud_classic_ ? kClassicViewX : 0;
}

int Engine::view_y() const {
    return hud_classic_ ? kClassicViewY : 0;
}

int Engine::view_w() const {
    return hud_classic_ ? kClassicViewW : Framebuffer::W;
}

int Engine::view_h() const {
    return hud_classic_ ? kClassicViewH : Framebuffer::H;
}

int Engine::view_center_x() const {
    return view_w() / 2;
}

int Engine::view_center_y() const {
    return view_h() / 2;
}

void Engine::follow_camera() {
    if (!has_player_) {
        return;
    }
    const BoardObject& p = objects_[0];
    int tx = p.x + 8 - view_center_x();
    int ty = p.y + 16 - view_center_y();
    tx = std::max(0, std::min(tx, Board::kCols * 16 - view_w()));
    ty = std::max(0, std::min(ty, Board::kRows * 16 - view_h()));
    // Follow fast movement without the old +/-8 px/tick lag.  Horizontal
    // walking is 8 px/tick, but a fall reaches 16 px/tick and the speed cheat
    // can move faster, so the vertical step is larger.  A large jump (respawn,
    // warp) snaps immediately instead of scrolling across the board.
    auto follow = [](int target, int cam, int max_step, int snap) {
        const int d = target - cam;
        if (d > snap || d < -snap) {
            return d;
        }
        return std::max(-max_step, std::min(max_step, d));
    };
    cam_x_ += follow(tx, cam_x_, 16, 96);
    cam_y_ += follow(ty, cam_y_, 24, 96);
}

void Engine::render_at(Framebuffer& fb, int x, int y, bool with_hud) {
    cam_x_ = x;
    cam_y_ = y;
    prev_cam_x_ = x;
    prev_cam_y_ = y;
    order_.clear();
    order_.reserve(objects_.size());
    for (int i = 0; i < static_cast<int>(objects_.size()); ++i) {
        order_.push_back(i);
    }
    std::stable_sort(order_.begin(), order_.end(), [this](int a, int b) {
        const BoardObject& oa = objects_[static_cast<size_t>(a)];
        const BoardObject& ob = objects_[static_cast<size_t>(b)];
        const int da = oa.kind == 32 ? -0x10000 : oa.y;
        const int db = ob.kind == 32 ? -0x10000 : ob.y;
        return da < db;
    });
    render(fb, with_hud, 1.0f);
}

void Engine::render(Framebuffer& fb, bool with_hud, float alpha) {
    float a = std::max(0.0f, std::min(1.0f, alpha));
    if (turtle_) {
        // A turtle update covers two 18.2 Hz periods: the first presentation
        // tick blends 0..0.5, the skipped one 0.5..1.
        a = (slow_phase_ ? 0.5f : 0.0f) + a * 0.5f;
    }
    const bool blend = a < 0.999f;
    std::vector<std::pair<int, int>> saved;
    const int saved_cam_x = cam_x_;
    const int saved_cam_y = cam_y_;
    if (blend) {
        saved.reserve(objects_.size());
        auto mix = [a](int prev, int cur) {
            return prev + static_cast<int>(
                              (static_cast<float>(cur - prev) * a) +
                              (cur >= prev ? 0.5f : -0.5f));
        };
        for (BoardObject& o : objects_) {
            saved.emplace_back(o.x, o.y);
            o.x = mix(o.prev_x, o.x);
            o.y = mix(o.prev_y, o.y);
        }
        cam_x_ = mix(prev_cam_x_, cam_x_);
        cam_y_ = mix(prev_cam_y_, cam_y_);
    }

    fb.clear(0);
    // An external pack can supply a whole-board backdrop.  Tiles are then
    // drawn with colour 0 treated as transparent so the picture shows through
    // the open areas of the art.
    const std::vector<uint8_t>* bg = nullptr;
    if (ext_visuals_) {
        bg = ext_visuals_->board(board_name_);
    }
    const bool bg_ok = bg && bg->size() == fb.px.size();
    if (bg_ok) {
        fb.px = *bg;
    }

    int c0 = cam_x_ >> 4;
    int c1 = std::min(Board::kCols - 1, (cam_x_ + view_w()) >> 4);
    int r0 = cam_y_ >> 4;
    int r1 = std::min(Board::kRows - 1, (cam_y_ + view_h()) >> 4);
    for (int c = c0; c <= c1; ++c) {
        for (int r = r0; r <= r1; ++r) {
            uint16_t id = map_[c * Board::kRows + r] & 0x3FFF;
            uint16_t shape = tiles_.at(id).shape;
            if (ext_visuals_) {
                // Tile animation rewrites the live record's shape, so fall
                // back to the pack override of the tile's original shape.
                const int entry = (shape >> 8) & 0x3F;
                const int index = shape & 0xFF;
                if (!ext_visuals_->shape((entry << 8) | index)) {
                    const uint16_t base = tiles_.base_shape(id);
                    const int bentry = (base >> 8) & 0x3F;
                    const int bindex = base & 0xFF;
                    if (ext_visuals_->shape((bentry << 8) | bindex)) {
                        shape = base;
                    }
                }
            }
            draw_shape(fb, shape, c * 16 - cam_x_ + view_x(), r * 16 - cam_y_ + view_y(), bg_ok);
        }
    }

    for (int i : order_) {
        const BoardObject& o = objects_[static_cast<size_t>(i)];
        if (hide_text_kinds_ && (o.kind == 20 || o.kind == 21)) {
            continue;
        }
        if (o.kind == 0) {
            player_draw(i, fb);
        } else if (o.kind >= 54 && o.kind <= 57) {
            form_draw(i, fb);
        } else if (o.kind == 23) {
            // TINY: 4x10 sheet 0x10, drawn 2 px below the origin.
            int entry = profile_->kind(23).tileset;  // 0x10
            int frame;
            if (o.dx == 0) {
                frame = (o.dy < 0 ? 6 : 4) + (o.p11 & 1);
            } else {
                frame = (o.dx < 0 ? 8 : 0) + (o.p11 & 3);
            }
            draw_shape(fb, (entry << 8) | frame, o.x - cam_x_ + view_x(),
                       o.y - cam_y_ + view_y() + 2, true);
        } else {
            draw_object(i, fb);
        }
    }

    if (with_hud) {
        if (hud_classic_) {
            render_hud_classic(fb);
        } else {
            render_hud_modern(fb);
        }
        if (minimap_) {
            draw_minimap(fb);
        }
    }
    // The bottom message is game UI, not HUD art, so it is drawn in both HUD
    // styles (and skipped only for the menu theme backdrops, which render with
    // with_hud=false and have no message anyway).
    draw_bottom_msg(fb);
    draw_caption(fb);
    draw_message_box(fb);
    draw_text_page(fb);

    clear_frame_marks();

    if (blend) {
        for (size_t i = 0; i < objects_.size() && i < saved.size(); ++i) {
            objects_[i].x = saved[i].first;
            objects_[i].y = saved[i].second;
        }
        cam_x_ = saved_cam_x;
        cam_y_ = saved_cam_y;
    }
}

// Palette-indexed external overlays (HUD panels, text-page art).  Transparent
// pixels in the source leave the framebuffer untouched.
static void blit_external_picture(Framebuffer& fb, int x, int y,
                                  const ExternalPicture& pic) {
    for (int sy = 0; sy < pic.h; ++sy) {
        for (int sx = 0; sx < pic.w; ++sx) {
            const size_t i = static_cast<size_t>(sy) * pic.w + sx;
            if (i < pic.mask.size() && pic.mask[i]) {
                fb.put(x + sx, y + sy, pic.px[i]);
            }
        }
    }
}

void Engine::draw_classic_backdrop(Framebuffer& fb, const std::string& title,
                                   bool menu) {
    const uint8_t label = static_cast<uint8_t>(hud_text_);
    // Frame 0x0A is the 16x16 noise fill; 0x00/0x03 are the 8x16 vertical
    // edges, 0x06/0x08 the 16x12 horizontal edges and 0x04 the corner.
    auto frame = [&](int entry, int index) {
        return menu ? classic_frame_shape(entry, index)
                    : classic_hud_shape(entry, index);
    };
    const ShapeFrame* fill = frame(3, 0x0A);
    const ShapeFrame* corner = frame(3, 0x04);
    const ShapeFrame* edge_top = frame(3, 0x06);
    const ShapeFrame* edge_bottom = frame(3, 0x08);
    const ShapeFrame* edge_left = frame(3, 0x00);
    const ShapeFrame* edge_right = frame(3, 0x03);

    for (int y = 0; y < Framebuffer::H; ++y) {
        for (int x = 0; x < Framebuffer::W; ++x) {
            const bool ring = x < kClassicBorder ||
                              x >= Framebuffer::W - kClassicBorder ||
                              y < kClassicBorder ||
                              y >= Framebuffer::H - kClassicBorder;
            const bool left = x < kClassicViewX;
            const bool top = y < kClassicTop;
            const bool bottom = y >= Framebuffer::H - 14;
            if (!(ring || left || top || bottom)) {
                continue;
            }
            const ShapeFrame* f = fill;
            int sx = x;
            int sy = y;
            if (x < kClassicBorder && y < kClassicBorder) {
                f = corner;
            } else if (x >= Framebuffer::W - kClassicBorder &&
                       y < kClassicBorder) {
                f = corner;
                sx = x - (Framebuffer::W - kClassicBorder);
            } else if (x < kClassicBorder &&
                       y >= Framebuffer::H - kClassicBorder) {
                f = corner;
                sy = y - (Framebuffer::H - kClassicBorder);
            } else if (x >= Framebuffer::W - kClassicBorder &&
                       y >= Framebuffer::H - kClassicBorder) {
                f = corner;
                sx = x - (Framebuffer::W - kClassicBorder);
                sy = y - (Framebuffer::H - kClassicBorder);
            } else if (y < kClassicBorder) {
                f = edge_top;
                sy = y;
            } else if (y >= Framebuffer::H - kClassicBorder) {
                f = edge_bottom;
                sy = 4 + (y - (Framebuffer::H - kClassicBorder));
            } else if (x < kClassicBorder) {
                f = edge_left;
                sy = y - kClassicBorder;
            } else if (x >= Framebuffer::W - kClassicBorder) {
                f = edge_right;
                sx = x - (Framebuffer::W - kClassicBorder);
                sy = y - kClassicBorder;
            }
            uint8_t colour;
            if (f && f->w > 0 && f->h > 0) {
                colour = f->px[(sy % f->h) * f->w + (sx % f->w)];
            } else {
                colour = (((x >> 1) + (y >> 1)) & 1) ? 7 : 8;
            }
            fb.put(x, y, colour);
        }
    }
    // Dark edge around the playfield.
    fb.fill_rect(kClassicViewX - 1, kClassicViewY, 1, kClassicViewH, 0);
    fb.fill_rect(kClassicViewX, kClassicViewY - 1, kClassicViewW, 1, 0);
    fb.fill_rect(kClassicViewX, kClassicViewY + kClassicViewH,
                 kClassicViewW, 1, 0);
    fb.fill_rect(kClassicViewX + kClassicViewW, kClassicViewY, 1,
                 kClassicViewH, 0);

    int adv = 8;
    int bank = 1;
    if (static_cast<int>(title.size()) * adv > kClassicViewW - 10) {
        adv = 6;
        bank = 2;
    }
    const int tw = static_cast<int>(title.size()) * adv;
    const int tx = kClassicViewX + (kClassicViewW - tw) / 2;
    const int clean_x = std::max(kClassicViewX + 2, tx);
    draw_text_font(fb, clean_x + 1, 7, 0, bank, adv, title.c_str());
    draw_text_font(fb, clean_x, 6, label, bank, adv, title.c_str());
}

void Engine::draw_menu_window(Framebuffer& fb, int x, int y, int w, int h) {
    if (w < 16 || h < 16) {
        return;
    }
    auto part = [&](int index, int px, int py) {
        const ShapeFrame* f = classic_menu_shape(7, index);
        if (!f) {
            return;
        }
        for (int sy = 0; sy < f->h; ++sy) {
            for (int sx = 0; sx < f->w; ++sx) {
                fb.put(px + sx, py + sy,
                       f->px[static_cast<size_t>(sy) * f->w + sx]);
            }
        }
    };
    // Interior: the checkerboard fill (frame 9), tiled in 8x8 cells.
    const ShapeFrame* fill = classic_menu_shape(7, 9);
    for (int yy = y + 8; yy < y + h - 8; yy += 8) {
        for (int xx = x + 8; xx < x + w - 8; xx += 8) {
            if (fill && fill->w > 0 && fill->h > 0) {
                for (int sy = 0; sy < 8 && yy + sy < y + h - 8; ++sy) {
                    for (int sx = 0; sx < 8 && xx + sx < x + w - 8; ++sx) {
                        fb.put(xx + sx, yy + sy,
                               fill->px[(sy % fill->h) * fill->w +
                                        (sx % fill->w)]);
                    }
                }
            } else {
                fb.fill_rect(xx, yy, 8, 8, 0);
            }
        }
    }
    // Frames 1..8 are a scan-order nine-slice: 1/2/3 top row, 4/5 middle
    // edges, 6/7/8 bottom row.  (The old mapping used 5/7 as corners and
    // therefore put the right edge and bottom row in the wrong places.)
    for (int xx = x + 8; xx < x + w - 8; xx += 8) {
        part(2, xx, y);
        part(7, xx, y + h - 8);
    }
    for (int yy = y + 8; yy < y + h - 8; yy += 8) {
        part(4, x, yy);
        part(5, x + w - 8, yy);
    }
    part(1, x, y);
    part(3, x + w - 8, y);
    part(8, x + w - 8, y + h - 8);
    part(6, x, y + h - 8);
}

// Original HUD (22ab:05dd), the optional CLASSIC style.  The board still fills
// the 320x200 buffer; this draws the original screen presentation over it: a
// dithered border/title strip, a 64-px status column (health, level/score,
// inventory) and the INVENTORY caption.  The MODERN style is separate.
void Engine::render_hud_classic(Framebuffer& fb) {
    const uint8_t label = static_cast<uint8_t>(hud_text_);
    const int px = kClassicPanelX;
    const int py = kClassicPanelY;
    const int hp = std::max(0, std::min(8, health()));

    if (ext_visuals_) {
        const ExternalPicture* panel = ext_visuals_->hud(hud_panel_, true);
        if (!panel && !hud_panel_.empty()) {
            panel = ext_visuals_->hud(std::string(), true);
        }
        if (panel) {
            // A pack supplies the original 64x84 panel.  Keep the legacy
            // placement so existing HUD packs still line up.
            blit_external_picture(fb, 0, 0, *panel);
            draw_text(fb, 2, 2, label, "HEALTH");
            if (hp > 0) {
                for (int i = 0; i < hp - 1; ++i) {
                    draw_health_pip(fb, 0x2A, i * 3 + 0x2A, 2);
                }
                draw_health_pip(fb, 0x2B, (hp - 1) * 3 + 0x28, 2);
            }
            char buf[16];
            int len = 0;
            uint32_t s = score();
            do {
                buf[len++] = static_cast<char>('0' + (s % 10));
                s /= 10;
            } while (s != 0);
            draw_text(fb, 0x21, 10, label, "SCORE");
            int sx = 0x40 - (len * 6 + 1);
            for (int i = 0; i < len; ++i) {
                char ch[2] = {buf[len - 1 - i], 0};
                draw_text(fb, sx + i * 6, 0x10, label, ch);
            }
            draw_text(fb, 1, 10, label, "LEVEL");
            int level = level_id();
            if (level != 0x7F) {
                std::snprintf(buf, sizeof(buf), "%d", level);
                draw_text(fb, 1, 0x10, label, buf);
            }
            const int n = header_[4] | (header_[5] << 8);
            for (int i = 0; i < n && i < 15; ++i) {
                const int id = header_[6 + i * 2] | (header_[7 + i * 2] << 8);
                if (id < 0 || id >= 16) {
                    continue;
                }
                const int frame = profile_->item_icons[id];
                if (!frame) {
                    continue;
                }
                draw_shape(fb, icon_shape(frame), (i / 3) * 0x0E + 1,
                           (i % 3) * 0x0E + 0x1A, true);
            }
            return;
        }
    }

    draw_classic_backdrop(fb, game_.title);

    // Status column.
    classic_box(fb, px, py, kClassicPanelW, 30, 12);
    classic_box(fb, px, py + 30, kClassicPanelW, 22, 8);
    classic_box(fb, px, py + 52, kClassicPanelW, kClassicPanelH - 52 - 14, 8);

    // HEALTH + bar.
    draw_text(fb, px + 2, py + 5, label, "HEALTH");
    const int barX = px + 40;
    const int barY = py + 5;
    constexpr int kBarW = 20;
    constexpr int kBarH = 7;
    fb.fill_rect(barX, barY, kBarW, kBarH, 0);
    fb.fill_rect(barX, barY, kBarW, 1, 8);
    fb.fill_rect(barX, barY + kBarH - 1, kBarW, 1, 8);
    fb.fill_rect(barX, barY, 1, kBarH, 8);
    fb.fill_rect(barX + kBarW - 1, barY, 1, kBarH, 8);
    if (hp > 0) {
        const int fillW = std::max(1, (kBarW - 2) * hp / 8);
        const uint8_t barColour =
            static_cast<uint8_t>(hud_health_ >= 0 ? hud_health_ : 4);
        fb.fill_rect(barX + 1, barY + 1, fillW, kBarH - 2, barColour);
    }

    // LEVEL / SCORE.
    const int ry = py + 35;
    draw_text(fb, px + 1, ry, label, "LEVEL");
    draw_text(fb, px + 33, ry, label, "SCORE");
    char buf[24];
    const int level = level_id();
    if (level == 0) {
        std::snprintf(buf, sizeof(buf), "MAP");
    } else if (level == 0x7F) {
        buf[0] = '\0';
    } else {
        std::snprintf(buf, sizeof(buf), "%d", level);
    }
    draw_text(fb, px + 2, ry + 8, label, buf);
    std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(score()));
    const int slen = static_cast<int>(std::strlen(buf));
    draw_text(fb, px + kClassicPanelW - 2 - slen * 6, ry + 8, label, buf);

    // Inventory icons (3-column grid, as in the original).
    const int invY = py + 58;
    const int n = header_[4] | (header_[5] << 8);
    for (int i = 0; i < n && i < 15; ++i) {
        const int id = header_[6 + i * 2] | (header_[7 + i * 2] << 8);
        if (id < 0 || id >= 16) {
            continue;
        }
        const int frame = profile_->item_icons[id];
        if (!frame) {
            continue;
        }
        draw_shape(fb, icon_shape(frame), px + 1 + (i / 3) * 16,
                   invY + (i % 3) * 16, true);
    }

    // INVENTORY caption in the bottom strip of the panel.
    draw_text_font(fb, px + 3, py + kClassicPanelH - 9, 0, 2, 6,
                   "INVENTORY");
    draw_text_font(fb, px + 2, py + kClassicPanelH - 10, label, 2, 6,
                   "INVENTORY");
}

// MODERN/MINIMAL HUD (the default): one horizontal strip across the top.
//   LEVEL n   <item icon><count> ...   HEALTH <pips>   SCORE n
void Engine::render_hud_modern(Framebuffer& fb) {
    const uint8_t kLabel = static_cast<uint8_t>(hud_text_);
    constexpr int kBarH = 12;

    if (ext_visuals_) {
        const ExternalPicture* panel = ext_visuals_->hud(hud_panel_, false);
        if (!panel && !hud_panel_.empty()) {
            panel = ext_visuals_->hud(std::string(), false);
        }
        if (panel) {
            blit_external_picture(fb, 0, 0, *panel);
        } else {
            fb.fill_rect(0, 0, Framebuffer::W, kBarH,
                         static_cast<uint8_t>(hud_bg_colour()));
        }
    } else {
        fb.fill_rect(0, 0, Framebuffer::W, kBarH,
                     static_cast<uint8_t>(hud_bg_colour()));
    }

    char buf[32];

    // Level number is the first field (blank at 0x7F, like the original).
    int level = level_id();
    if (level == 0x7F) {
        std::snprintf(buf, sizeof(buf), "LEVEL");
    } else {
        std::snprintf(buf, sizeof(buf), "LEVEL %d", level);
    }
    draw_text(fb, 2, 3, kLabel, buf);
    int x = 2 + 6 * static_cast<int>(std::strlen(buf)) + 10;

    // Inventory: one icon per distinct item id, then the picked-up count.
    for (int id = 0; id < 16; ++id) {
        int count = inv_count(id);
        if (count <= 0) {
            continue;
        }
        int frame = profile_->item_icons[id];
        if (!frame) {
            continue;
        }
        draw_shape(fb, icon_shape(frame), x, 0, true);
        std::snprintf(buf, sizeof(buf), "%d", count);
        draw_text(fb, x + 13, 3, kLabel, buf);
        x += 13 + 6 * static_cast<int>(std::strlen(buf)) + 9;
    }

    // Health pips live in a fixed slot so taking damage or gaining health does
    // not move the bar.  The score is right-aligned at the very end.
    constexpr int kHpX = 203;  // left edge of the 8-pip slot
    int hp = std::max(0, std::min(8, health()));
    if (x <= kHpX - 36) {
        draw_text(fb, kHpX - 40, 3, kLabel, "HEALTH");
    }
    if (hp > 0) {
        for (int i = 0; i < hp - 1; ++i) {
            draw_health_pip(fb, 0x2A, kHpX + 2 + i * 3, 3);
        }
        draw_health_pip(fb, 0x2B, kHpX, 3);
    }

    std::snprintf(buf, sizeof(buf), "SCORE %u",
                  static_cast<unsigned>(score()));
    int score_w = 6 * static_cast<int>(std::strlen(buf));
    draw_text(fb, Framebuffer::W - 2 - score_w, 3, kLabel, buf);
}

// Enhancement: one pixel per 16x16 tile, lower-right corner.  Solid cells are
// light grey, open cells dark blue, water cyan, lava/spikes red; checkpoints
// green and the player bright white.  Drawn in both HUD styles, beneath the
// bottom message line.
void Engine::draw_minimap(Framebuffer& fb) {
    constexpr int kMapW = Board::kCols;   // 128
    constexpr int kMapH = Board::kRows;   // 64
    const int x0 = Framebuffer::W - kMapW - 4;
    const int y0 = Framebuffer::H - kMapH - 4;
    fb.fill_rect(x0 - 2, y0 - 2, kMapW + 4, kMapH + 4, 0);
    fb.fill_rect(x0 - 1, y0 - 1, kMapW + 2, kMapH + 2, 8);
    for (int c = 0; c < Board::kCols; ++c) {
        for (int r = 0; r < Board::kRows; ++r) {
            const uint16_t id = map_[c * Board::kRows + r] & 0x3FFF;
            uint8_t col = (tiles_.at(id).flags & 1) ? 1 : 7;
            if (id >= 0x186 && id <= 0x18B) {
                col = 12;  // lava
            } else if (id >= 0x190 && id <= 0x1A3) {
                col = 9;   // water
            } else if (id == 0xA6 || id == 0x141 || id == 0x142 ||
                       id == 0x155 || id == 0x156) {
                col = 13;  // spikes / thorns / stalactites
            }
            fb.put(x0 + c, y0 + r, col);
        }
    }
    for (const BoardObject& o : objects_) {
        if (o.kind == 12) {
            fb.fill_rect(x0 + (o.x >> 4) - 1, y0 + (o.y >> 4) - 1, 3, 3, 10);
        }
    }
    if (has_player_) {
        const BoardObject& p = objects_[0];
        fb.fill_rect(x0 + (p.x >> 4) - 1, y0 + (p.y >> 4) - 1, 3, 3, 15);
    }
}

int Engine::hud_bg_colour() {
    if (hud_bg_ == -2) {
        return suit_colour();
    }
    if (hud_bg_ == -3) {
        return suit_vibrant_colour();
    }
    if (hud_bg_ < 0) {
        return 0;  // black
    }
    return hud_bg_ & 0xFF;
}

void Engine::set_external_palette(const uint8_t rgb[768]) {
    std::memcpy(ext_palette_, rgb, 768);
}

bool Engine::apply_external(const ExternalAssets& assets, const uint8_t rgb[768]) {
    set_external_palette(rgb);
    if (!assets.mounted()) {
        ext_visuals_.reset();
        return false;
    }
    auto visuals = std::make_shared<ExternalVisuals>();
    visuals->build(assets, shapes_, tiles_, *profile_, ext_palette_,
                   Framebuffer::W, Framebuffer::H);
    if (visuals->empty() && visuals->themes().empty()) {
        ext_visuals_.reset();
        return false;
    }
    ext_visuals_ = std::move(visuals);
    return true;
}

// The front end frames its menu with the theme board's own tiles: the most
// common blocking tile becomes the frame and the most common walkable tile the
// panel background.  Counting the loaded map keeps it data-driven (no per-game
// tile ids), and a board with no tiles falls back to the plain panel.
int Engine::theme_tile(bool solid) const {
    int counts[TileDefs::kCount] = {};
    for (uint16_t cell : board_.map) {
        const int id = cell & 0x3FFF;
        if (id < TileDefs::kCount) {
            ++counts[id];
        }
    }
    // A manifest can name the frame tile explicitly (Jill 3 uses the red brick
    // instead of the grey castle block that dominates its intro board).  The
    // name is matched case-insensitively against the .dma table.
    if (solid && !game_.menu_block.empty()) {
        for (int id = 0; id < TileDefs::kCount; ++id) {
            const std::string& n = tiles_.at(id).name;
            if (n.size() != game_.menu_block.size()) {
                continue;
            }
            bool same = true;
            for (size_t k = 0; k < n.size(); ++k) {
                char a = n[k];
                char b = game_.menu_block[k];
                if (a >= 'a' && a <= 'z') a = static_cast<char>(a - 'a' + 'A');
                if (b >= 'a' && b <= 'z') b = static_cast<char>(b - 'a' + 'A');
                if (a != b) {
                    same = false;
                    break;
                }
            }
            if (same && counts[id] > 0) {
                return id;
            }
        }
    }
    // Tile class is `flags & 7` (docs/jill1-player-physics.md #5): 6 = solid,
    // 7 = open/walkable.  Tile 0 is the board filler, so prefer a real tile.
    for (int pass = 0; pass < 2; ++pass) {
        int best = -1;
        int best_count = 0;
        for (int id = 0; id < TileDefs::kCount; ++id) {
            if (counts[id] == 0 || (pass == 0 && id == 0)) {
                continue;
            }
            const bool is_solid = (tiles_.at(id).flags & 0x0007) == 0x0006;
            if (is_solid != solid) {
                continue;
            }
            if (counts[id] > best_count) {
                best_count = counts[id];
                best = id;
            }
        }
        if (best >= 0) {
            return best;
        }
    }
    return solid ? theme_tile(false) : -1;
}

bool Engine::tile_pixels(int tile_id, std::vector<uint8_t>& px, int& w,
                         int& h) {
    if (tile_id < 0) {
        return false;
    }
    const int shape = tiles_.at(tile_id).shape;
    const ShapeFrame* f = shape_frame((shape >> 8) & 0x3F, shape & 0xFF);
    if (!f || f->w <= 0 || f->h <= 0) {
        return false;
    }
    w = f->w;
    h = f->h;
    px = f->px;
    return true;
}

// The player art (tileset 0x08, idle frame 0x10) uses three shades for Jill's
// leotard.  The torso box rows 11..18 / cols 4..11 is dominated by the darkest
// of them; the palette indices differ per episode (Jill 1 green, Jill 2 red,
// Jill 3 blue), so derive it from the loaded shape bank instead of hardcoding.
// Jill Select.  The player art is the same 4-bpp sheet in every episode; only
// the embedded 16-entry colour table differs (Jill 1 green, Jill 2 red,
// Jill 3 blue, Onesimus grey).  We read another episode's table and remap the
// current palette indices at draw time, so no second shape bank is needed.
bool Engine::set_player_look(int look, const std::string& shape_path) {
    player_look_ = look < 0 ? 0 : look;
    player_raw_xlate_ = player_base_xlate_;
    if (player_look_ > 0 && shape_path.empty()) {
        player_look_ = 0;
    }
    if (player_look_ > 0) {
        uint8_t table[16];
        if (!read_entry_xlate(shape_path, 8, table)) {
            player_look_ = 0;
            return false;
        }
        for (int i = 0; i < 16; ++i) {
            player_raw_xlate_[static_cast<size_t>(i)] = table[i];
        }
    }
    build_player_remap();
    return true;
}

void Engine::set_player_suit(int suit) {
    player_suit_ = suit < 0 ? 0 : (suit > 8 ? 8 : suit);
    build_player_remap();
}

void Engine::build_player_remap() {
    static const uint8_t kSuit[9][3] = {
        {0, 0, 0},        // 0: keep the selected look's own colours
        {28, 2, 10},      // 1: green (Jill 1)
        {58, 60, 61},     // 2: red (Jill 2)
        {176, 216, 228},  // 3: blue (Jill 3)
        {30, 55, 14},     // 4: yellow
        {24, 39, 45},     // 5: orange
        {3, 11, 15},      // 6: cyan
        {57, 180, 221},   // 7: purple
        {17, 22, 23}      // 8: silver
    };
    player_look_xlate_ = player_raw_xlate_;
    if (player_suit_ > 0 && player_suit_ <= 8) {
        const uint8_t* s = kSuit[player_suit_];
        player_look_xlate_[1] = s[0];   // dark leotard shade
        player_look_xlate_[2] = s[1];   // main leotard colour
        player_look_xlate_[10] = s[2];  // highlight shade
    }
    for (int i = 0; i < 256; ++i) {
        player_remap_[i] = static_cast<uint8_t>(i);
    }
    bool active = false;
    for (int slot = 0; slot < 16; ++slot) {
        const int src = player_base_xlate_[static_cast<size_t>(slot)];
        const int dst = player_look_xlate_[static_cast<size_t>(slot)];
        if (src != dst) {
            player_remap_[src] = static_cast<uint8_t>(dst);
            active = true;
        }
    }
    player_remap_active_ = active;
    suit_colour_ = -1;
    suit_vibrant_ = -1;
}

int Engine::suit_colour() {
    auto out = [this](int c) {
        return player_remap_active_ ? player_remap_[c & 0xFF] : c;
    };
    if (suit_colour_ >= 0) {
        return out(suit_colour_);
    }
    if (game_.suit_index >= 0) {
        suit_colour_ = game_.suit_index & 0xFF;
        return out(suit_colour_);
    }
    suit_colour_ = 8;
    const ShapeFrame* f = shapes_.frame(0x08, 0x10);
    if (!f) {
        return out(suit_colour_);
    }
    int counts[256] = {};
    for (int y = 11; y <= 18; ++y) {
        for (int x = 4; x <= 11; ++x) {
            if (x < f->w && y < f->h) {
                ++counts[f->px[static_cast<size_t>(y) * f->w + x]];
            }
        }
    }
    counts[0] = 0;
    int best = 0;
    for (int i = 1; i < 256; ++i) {
        if (counts[i] > best) {
            best = counts[i];
            suit_colour_ = i;
        }
    }
    return out(suit_colour_);
}

// The vibrant HUD background is the brightest leotard shade in the same hue
// family as the base suit colour (Jill 1's muted green becomes the bright
// green highlight, Jill 2's dark red the bright red, etc.).  Skin/face tones
// share the torso box but are rejected by hue distance.
int Engine::suit_vibrant_colour() {
    if (suit_vibrant_ >= 0) {
        return suit_vibrant_;
    }
    const int base = suit_colour();
    suit_vibrant_ = base;
    const ShapeFrame* f = shapes_.frame(0x08, 0x10);
    if (!f) {
        return suit_vibrant_;
    }
    int counts[256] = {};
    for (int y = 11; y <= 18; ++y) {
        for (int x = 4; x <= 11; ++x) {
            if (x < f->w && y < f->h) {
                ++counts[f->px[static_cast<size_t>(y) * f->w + x]];
            }
        }
    }
    counts[0] = 0;

    auto rgb = [](int i, int c) {
        return static_cast<int>(kDefaultPalette[i * 3 + c]);
    };
    auto lum = [&](int i) { return rgb(i, 0) + rgb(i, 1) + rgb(i, 2); };
    auto hue = [&](int i, int c) {
        const int sum = lum(i);
        return sum > 0 ? rgb(i, c) * 100 / sum : 0;
    };
    auto hue_dist = [&](int a, int b) {
        return std::abs(hue(a, 0) - hue(b, 0)) +
               std::abs(hue(a, 1) - hue(b, 1)) +
               std::abs(hue(a, 2) - hue(b, 2));
    };

    const int base_lum = lum(base);
    int best_score = -10000;
    for (int i = 1; i < 256; ++i) {
        if (i == base || counts[i] == 0) {
            continue;
        }
        const int l = lum(i);
        if (l <= base_lum) {
            continue;
        }
        const int dist = hue_dist(i, base);
        if (dist > 40) {
            continue;
        }
        const int score = l * 4 - dist;
        if (score > best_score) {
            best_score = score;
            suit_vibrant_ = i;
        }
    }
    return suit_vibrant_;
}

void Engine::draw_shape_text(Framebuffer& fb, int shape, int x, int y,
                             uint8_t colour) {
    // Bit 0x4000 is not a mirror flag: it is part of the tileset byte
    // (tilesets 0x40..0x7F map onto shape entries 0x00..0x3F).  The original
    // blitter also draws every shape unmirrored - left/right facing uses
    // separate frames - so the entry alone selects the art.
    int entry = (shape >> 8) & 0x3F;
    int index = shape & 0xFF;
    const ShapeFrame* f = shape_frame(entry, index);
    if (!f) {
        return;
    }
    for (int sy = 0; sy < f->h; ++sy) {
        for (int sx = 0; sx < f->w; ++sx) {
            uint8_t c = f->px[static_cast<size_t>(sy) * f->w + sx];
            if (c == 0 || c == 3) {
                continue;
            }
            fb.put(x + sx, y + sy, colour);
        }
    }
}

void Engine::draw_text_font(Framebuffer& fb, int x, int y, uint8_t colour,
                            int bank, int advance, const char* s) {
    for (int i = 0; s[i]; ++i) {
        draw_shape_text(fb, (bank << 8) | (static_cast<uint8_t>(s[i]) & 0x7F),
                        x + i * advance, y, colour);
    }
}

void Engine::draw_text(Framebuffer& fb, int x, int y, uint8_t colour,
                       const char* s) {
    draw_text_font(fb, x, y, colour, 2, 6, s);
}

void Engine::draw_text_8(Framebuffer& fb, int x, int y, uint8_t colour,
                         const char* s) {
    draw_text_font(fb, x, y, colour, 1, 8, s);
}

void Engine::put_bottom_msg(const std::string& text, int colour) {
    if (text.empty()) {
        return;
    }
    msg_text_ = text;
    msg_colour_ = static_cast<uint8_t>((colour > 0 && colour < 16) ? colour : 7);
    // Original timing: shown in full for 0x50 ticks (DGROUP:0xB6B4 = 0x50).
    msg_ttl_ = 0x50;
    msg_shown_ = msg_text_.size();
}

void Engine::draw_frame(Framebuffer& fb, int x, int y, int w, int h) {
    fb.fill_rect(x, y, w, h, 8);
    fb.fill_rect(x + 1, y + 1, w - 2, h - 2, 0);
}

void Engine::draw_bottom_msg(Framebuffer& fb) {
    if (msg_text_.empty() || msg_ttl_ <= 0) {
        return;
    }
    size_t len = std::min(msg_shown_, msg_text_.size());
    if (len == 0) {
        return;
    }
    const std::string shown = msg_text_.substr(0, len);
    int w = static_cast<int>(shown.size()) * 6;
    int x = (Framebuffer::W - w) / 2;
    int y = Framebuffer::H - 14;
    if (message_box_style_ == 1) {
        draw_frame(fb, x - 5, y - 3, w + 10, 13);
    } else {
        // Plain line (the original look) with a 1 px shadow so the text stays
        // readable over bright tiles.  Classic mode also uses this branch.
        draw_text(fb, x + 1, y + 1, 0, shown.c_str());
    }
    draw_text(fb, x, y, msg_colour_, shown.c_str());
}

// Level-title dialog box.  Text lines come from the 36-entry title table and
// are revealed at the dialog speed, so FASTER DIALOG BOXES also speeds this up.
void Engine::draw_portrait(Framebuffer& fb, int x, int y) {
    // A pack's portrait.<ext> is a hand-authored replacement and wins.
    const ExternalPicture* ext_portrait =
        ext_visuals_ ? ext_visuals_->portrait() : nullptr;
    if (ext_portrait) {
        // Older reference packs exported a 64x84 portrait; clip it to the
        // original 64x70 card so it cannot spill over the window border.
        const int w = std::min(ext_portrait->w, 64);
        const int h = std::min(ext_portrait->h, 70);
        for (int sy = 0; sy < h; ++sy) {
            for (int sx = 0; sx < w; ++sx) {
                const size_t i = static_cast<size_t>(sy) * ext_portrait->w + sx;
                if (i < ext_portrait->mask.size() && ext_portrait->mask[i]) {
                    fb.put(x + sx, y + sy, ext_portrait->px[i]);
                }
            }
        }
        return;
    }
    // The original title/menu portrait is shape entry 24: sixteen 16x16 (the
    // bottom row is 16x22/16x20) tiles arranged as a 4-wide grid.  Jill 1/2/3
    // all ship it; Onesimus has no equivalent.
    bool drew = false;
    for (int i = 0; i < 16; ++i) {
        const ShapeFrame* f = shape_frame(24, i);
        if (!f || f->w <= 0 || f->h <= 0) {
            continue;
        }
        drew = true;
        const int ox = x + (i % 4) * 16;
        const int oy = y + (i / 4) * 16;
        for (int sy = 0; sy < f->h; ++sy) {
            for (int sx = 0; sx < f->w; ++sx) {
                fb.put(ox + sx, oy + sy,
                       f->px[static_cast<size_t>(sy) * f->w + sx]);
            }
        }
    }
    if (drew) {
        return;
    }
    // Fallback for a game without entry 24: the player's idle sprite, 2x.
    draw_frame(fb, x, y, 64, 70);
    const ShapeFrame* pf = shape_frame(8, 0x10);
    if (pf) {
        const int ox = x + (64 - pf->w * 2) / 2;
        const int oy = y + (70 - pf->h * 2) / 2;
        for (int sy = 0; sy < pf->h; ++sy) {
            for (int sx = 0; sx < pf->w; ++sx) {
                const uint8_t c = pf->px[static_cast<size_t>(sy) * pf->w + sx];
                if (c == 0) {
                    continue;
                }
                fb.fill_rect(ox + sx * 2, oy + sy * 2, 2, 2, c);
            }
        }
    }
}

void Engine::draw_caption(Framebuffer& fb) {
    if (caption_.empty() || caption_ttl_ <= 0) {
        return;
    }
    std::vector<std::string> lines;
    std::string cur;
    for (char c : caption_) {
        if (c == '\n') {
            if (!cur.empty()) {
                lines.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        lines.push_back(cur);
    }
    if (lines.empty()) {
        return;
    }
    if (message_box_style_ == 2) {
        draw_caption_classic(fb, lines);
        return;
    }
    int maxlen = 0;
    for (const std::string& line : lines) {
        maxlen = std::max(maxlen, static_cast<int>(line.size()));
    }
    maxlen = std::min(maxlen, 46);
    const int boxw = maxlen * 6 + 16;
    const int boxh = static_cast<int>(lines.size()) * 10 + 12;
    int bx = (Framebuffer::W - boxw) / 2;
    const int by = (Framebuffer::H - boxh) / 2;

    // Dialog portrait: the original title screen shows the character where the
    // in-game inventory panel sits.  Draw the same portrait beside the caption
    // box; pack `portrait.<ext>` overrides it.
    constexpr int kPortraitW = 64;
    constexpr int kPortraitH = 70;
    constexpr int kPortraitGap = 6;
    int px = -1;
    if (hud_portrait_ && boxw + kPortraitGap + kPortraitW <= Framebuffer::W) {
        const int total = boxw + kPortraitGap + kPortraitW;
        const int start = (Framebuffer::W - total) / 2;
        px = start;
        bx = start + kPortraitW + kPortraitGap;
    }
    if (px >= 0) {
        const int py = std::max(
            0, std::min(by + (boxh - kPortraitH) / 2,
                        Framebuffer::H - kPortraitH));
        draw_frame(fb, px - 3, py - 3, kPortraitW + 6, kPortraitH + 6);
        draw_portrait(fb, px, py);
    }
    draw_frame(fb, bx, by, boxw, boxh);

    size_t budget = caption_shown_;
    int ty = by + 6;
    for (const std::string& line : lines) {
        const size_t n = std::min(budget, line.size());
        if (n > 0) {
            const std::string part = line.substr(0, n);
            draw_text(fb, bx + 8, ty, 7, part.c_str());
            budget -= n;
        }
        ty += 10;
    }
}

// Original level-title card: a shape-7 window with the 64x70 Jill portrait on
// the left and a blue text plate on the right (the "JILL BOUNDS THROUGH THE
// BOULDERS" box).  Selected by VIDEO > MESSAGE BOX: CLASSIC.
void Engine::draw_caption_classic(Framebuffer& fb,
                                  const std::vector<std::string>& lines) {
    constexpr int kBoxW = 200;
    constexpr int kBoxH = 88;
    const int centre =
        hud_classic_ ? view_x() + view_w() / 2 : Framebuffer::W / 2;
    int bx = centre - kBoxW / 2;
    bx = std::max(0, std::min(bx, Framebuffer::W - kBoxW));
    const int by = (Framebuffer::H - kBoxH) / 2;

    draw_menu_window(fb, bx, by, kBoxW, kBoxH);

    const int panel_x = hud_portrait_ ? bx + 80 : bx + 10;
    const int panel_y = by + 9;
    const int panel_w = hud_portrait_ ? kBoxW - 88 : kBoxW - 20;
    const int panel_h = 70;
    if (hud_portrait_) {
        draw_portrait(fb, bx + 8, by + 9);
    }
    fb.fill_rect(panel_x, panel_y, panel_w, panel_h, 0);
    fb.fill_rect(panel_x + 1, panel_y + 1, panel_w - 2, panel_h - 2, 1);

    const int line_h = 9;
    int ty = panel_y + (panel_h - static_cast<int>(lines.size()) * line_h) / 2;
    size_t budget = caption_shown_;
    for (const std::string& line : lines) {
        const size_t n = std::min(budget, line.size());
        if (n > 0) {
            const std::string part = line.substr(0, n);
            const int tx =
                panel_x + (panel_w - static_cast<int>(part.size()) * 6) / 2;
            draw_text(fb, tx + 1, ty + 1, 4, part.c_str());
            draw_text(fb, tx, ty, 15, part.c_str());
            budget -= n;
        }
        ty += line_h;
    }
}

// Classic titled card (the original "Reminder" window).
void Engine::show_message_box(const std::string& title, const std::string& text,
                              int ticks) {
    box_title_ = title;
    box_text_ = text;
    box_ttl_ = ticks > 0 ? ticks : 0x78;
    box_hold_ = 8;
}

void Engine::close_message_box() {
    box_ttl_ = 0;
    box_hold_ = 0;
}

void Engine::draw_message_box(Framebuffer& fb) {
    if (box_ttl_ <= 0 || box_text_.empty()) {
        return;
    }
    if (caption_ttl_ > 0) {
        return;  // let the level-title card finish first
    }
    std::vector<std::string> lines;
    std::string cur;
    auto flush = [&]() {
        while (!cur.empty() && cur.front() == ' ') {
            cur.erase(cur.begin());
        }
        lines.push_back(cur);
        cur.clear();
    };
    // Word wrap at 24 characters (6 px font), matching the original card.
    for (size_t i = 0; i < box_text_.size(); ++i) {
        const char c = box_text_[i];
        if (c == '\n') {
            flush();
            continue;
        }
        cur.push_back(c);
        if (cur.size() >= 24) {
            size_t sp = cur.find_last_of(' ');
            if (sp != std::string::npos) {
                const std::string rest = cur.substr(sp + 1);
                cur.resize(sp);
                flush();
                cur = rest;
            } else {
                flush();
            }
        }
    }
    if (!cur.empty()) {
        flush();
    }
    if (lines.empty()) {
        return;
    }

    constexpr int kBoxW = 200;
    const int line_h = 9;
    const int content_h = static_cast<int>(lines.size()) * line_h;
    const int kBoxH = std::max(96, content_h + 52);
    const int centre =
        hud_classic_ ? view_x() + view_w() / 2 : Framebuffer::W / 2;
    int bx = centre - kBoxW / 2;
    bx = std::max(0, std::min(bx, Framebuffer::W - kBoxW));
    const int by = (Framebuffer::H - kBoxH) / 2;

    draw_menu_window(fb, bx, by, kBoxW, kBoxH);

    if (!box_title_.empty()) {
        const int tw = static_cast<int>(box_title_.size()) * 8;
        const int tx = bx + (kBoxW - tw) / 2;
        draw_text_8(fb, tx + 1, by + 9, 1, box_title_.c_str());
        draw_text_8(fb, tx, by + 8, 13, box_title_.c_str());
    }

    const int panel_x = bx + 10;
    const int panel_y = by + 22;
    const int panel_w = kBoxW - 20;
    const int panel_h = kBoxH - 32;
    fb.fill_rect(panel_x, panel_y, panel_w, panel_h, 0);
    fb.fill_rect(panel_x + 1, panel_y + 1, panel_w - 2, panel_h - 2, 1);

    int ty = panel_y + (panel_h - content_h) / 2;
    for (const std::string& line : lines) {
        const int tx =
            panel_x + (panel_w - static_cast<int>(line.size()) * 6) / 2;
        draw_text(fb, tx + 1, ty + 1, 4, line.c_str());
        draw_text(fb, tx, ty, 15, line.c_str());
        ty += line_h;
    }
}


void Engine::draw_text_page(Framebuffer& fb) {
    if (!page_open_) {
        return;
    }
    const TextPage* page = pages_.page(page_number_);
    if (!page) {
        page_open_ = false;
        return;
    }
    const std::vector<TextPage::Line> body = page_body(*page);

    fb.fill_rect(0, 0, Framebuffer::W, Framebuffer::H, 0);
    if (ext_visuals_) {
        if (const ExternalPicture* art = ext_visuals_->text_page(page_number_)) {
            blit_external_picture(fb, 0, 0, *art);
        } else {
            draw_frame(fb, 6, 4, Framebuffer::W - 12, Framebuffer::H - 8);
        }
    } else {
        draw_frame(fb, 6, 4, Framebuffer::W - 12, Framebuffer::H - 8);
    }

    // Title (the page's first line); its colour digit follows the same 0-7
    // rule as the body but colour 1 is remapped so it stays readable.
    std::string title = page->lines.empty() ? std::string() : page->lines[0].text;
    if (title.empty()) {
        title = "PAGE " + std::to_string(page_number_);
    }
    const uint8_t title_colour =
        page->lines.empty() ? 7 : page_colour(page->lines[0].colour);
    const int title_x =
        (Framebuffer::W - static_cast<int>(title.size()) * 6) / 2;
    draw_text_font(fb, std::max(12, title_x), 11, title_colour, 2, 6,
                   title.c_str());
    fb.fill_rect(14, 21, Framebuffer::W - 28, 1, 8);

    const int top = 27;
    const int line_h = 9;
    const int visible = kPageVisibleLines;
    const int max_scroll = std::max(0, static_cast<int>(body.size()) - visible);
    page_scroll_ = std::max(0, std::min(page_scroll_, max_scroll));
    int y = top;
    for (int i = page_scroll_;
         i < static_cast<int>(body.size()) && i < page_scroll_ + visible; ++i) {
        const TextPage::Line& line = body[static_cast<size_t>(i)];
        if (!line.text.empty()) {
            const int x =
                (Framebuffer::W - static_cast<int>(line.text.size()) * 6) / 2;
            draw_text_font(fb, std::max(10, x), y, page_colour(line.colour), 2,
                           6, line.text.c_str());
        }
        y += line_h;
    }

    char info[48];
    std::snprintf(info, sizeof(info), "PAGE %d", page_number_);
    draw_text_font(fb, 14, Framebuffer::H - 13, 8, 2, 6, info);
    if (max_scroll > 0) {
        const int boxes = (static_cast<int>(body.size()) + visible - 1) / visible;
        std::snprintf(info, sizeof(info), "SCREEN %d/%d",
                      page_scroll_ / visible + 1, boxes);
        draw_text_font(fb, (Framebuffer::W - static_cast<int>(std::strlen(info)) * 6) / 2,
                       Framebuffer::H - 13, 8, 2, 6, info);
    }
    draw_text_font(fb, Framebuffer::W - 14 - 9 * 6, Framebuffer::H - 13, 8, 2,
                   6, "ESC CLOSE");
}

void Engine::show_text_page(int number) {
    if (!pages_.page(number)) {
        return;
    }
    page_open_ = true;
    page_number_ = number;
    page_scroll_ = 0;
    caption_ttl_ = 0;
    msg_ttl_ = 0;
}

void Engine::close_text_page() {
    page_open_ = false;
    page_scroll_ = 0;
}

void Engine::scroll_text_page(int delta) {
    if (!page_open_ || delta == 0) {
        return;
    }
    const TextPage* page = pages_.page(page_number_);
    if (!page) {
        return;
    }
    const int max_scroll = std::max(
        0, static_cast<int>(page_body(*page).size()) - kPageVisibleLines);
    page_scroll_ = std::max(0, std::min(page_scroll_ + delta, max_scroll));
}

void Engine::cycle_text_page(int dir) {
    if (!page_open_ || dir == 0) {
        return;
    }
    const TextPage* page = pages_.page(page_number_);
    if (page) {
        const int lines = static_cast<int>(page_body(*page).size());
        const int visible = kPageVisibleLines;
        if (dir > 0 && page_scroll_ + visible < lines) {
            page_scroll_ += visible;  // next screenful ("box") of this page
            return;
        }
        if (dir < 0 && page_scroll_ > 0) {
            page_scroll_ = std::max(0, page_scroll_ - visible);
            return;
        }
    }
    int n = page_number_;
    for (int i = 0; i < 40; ++i) {
        n += (dir > 0) ? 1 : -1;
        if (n < 0) {
            n = 39;
        } else if (n > 39) {
            n = 0;
        }
        if (pages_.page(n)) {
            show_text_page(n);
            if (dir < 0) {
                // Entering a page backwards: land on its last screenful.
                const TextPage* p = pages_.page(n);
                const int lines = static_cast<int>(page_body(*p).size());
                page_scroll_ = std::max(0, ((lines - 1) / kPageVisibleLines) *
                                               kPageVisibleLines);
            }
            return;
        }
    }
}

const ShapeFrame* Engine::shape_frame(int entry, int index) {
    if (ext_visuals_) {
        // A selected font set replaces only the two text banks (1 and 2), so
        // sprites/tiles keep using the normal pack overrides.
        if (!font_name_.empty() && (entry == 1 || entry == 2)) {
            if (const ShapeFrame* f =
                    ext_visuals_->font_shape(font_name_, entry, index)) {
                return f;
            }
        }
        if (const ShapeFrame* f = ext_visuals_->shape((entry << 8) | index)) {
            return f;
        }
    }
    return shapes_.frame(entry, index);
}

bool Engine::set_hud_frame_game(const GameDef* game, std::string& err) {
    hud_frame_custom_ = false;
    hud_frame_shapes_ = ShapeBank{};
    if (!game || game->id == game_.id) {
        return true;  // current game: use the normal shape lookup
    }
    if (!hud_frame_shapes_.load(join(game->data_dir, game->shape_file), err)) {
        return false;
    }
    hud_frame_custom_ = true;
    return true;
}

bool Engine::set_classic_frame_game(const GameDef* game, std::string& err) {
    classic_frame_custom_ = false;
    classic_frame_shapes_ = ShapeBank{};
    if (!game || game->id == game_.id) {
        return true;
    }
    if (!classic_frame_shapes_.load(join(game->data_dir, game->shape_file),
                                    err)) {
        return false;
    }
    classic_frame_custom_ = true;
    return true;
}

bool Engine::set_menu_frame_game(const GameDef* game, std::string& err) {
    menu_frame_custom_ = false;
    menu_frame_shapes_ = ShapeBank{};
    if (!game || game->id == game_.id) {
        return true;
    }
    if (!menu_frame_shapes_.load(join(game->data_dir, game->shape_file), err)) {
        return false;
    }
    menu_frame_custom_ = true;
    return true;
}

const ShapeFrame* Engine::classic_hud_shape(int entry, int index) {
    if (hud_frame_custom_) {
        if (const ShapeFrame* f = hud_frame_shapes_.frame(entry, index)) {
            return f;
        }
    }
    return shape_frame(entry, index);
}

const ShapeFrame* Engine::classic_frame_shape(int entry, int index) {
    if (classic_frame_custom_) {
        if (const ShapeFrame* f = classic_frame_shapes_.frame(entry, index)) {
            return f;
        }
    }
    return shape_frame(entry, index);
}

const ShapeFrame* Engine::classic_menu_shape(int entry, int index) {
    if (entry == 7 && menu_border_ext_ >= 0 && ext_visuals_) {
        if (const ShapeFrame* f =
                ext_visuals_->border_shape(menu_border_ext_, index)) {
            return f;
        }
    }
    if (menu_frame_custom_) {
        if (const ShapeFrame* f = menu_frame_shapes_.frame(entry, index)) {
            return f;
        }
    }
    return shape_frame(entry, index);
}

void Engine::draw_player_shape(Framebuffer& fb, int shape, int x, int y,
                               bool transparent) {
    const int entry = (shape >> 8) & 0x3F;
    const int index = shape & 0xFF;
    const ShapeFrame* f = shape_frame(entry, index);
    if (!f) {
        return;
    }
    const bool external = ext_visuals_ && ext_visuals_->shape((entry << 8) | index);
    for (int sy = 0; sy < f->h; ++sy) {
        for (int sx = 0; sx < f->w; ++sx) {
            uint8_t c = f->px[static_cast<size_t>(sy) * f->w + sx];
            if (transparent && c == 0) {
                continue;
            }
            if (!external && player_remap_active_ && c != 0) {
                c = player_remap_[c];
            }
            fb.put(x + sx, y + sy, c);
        }
    }
}

void Engine::draw_shape(Framebuffer& fb, int shape, int x, int y,
                        bool transparent) {
    // See draw_shape_text: 0x4000 selects the shape entry, it is not a flip.
    int entry = (shape >> 8) & 0x3F;
    int index = shape & 0xFF;
    const ShapeFrame* f = shape_frame(entry, index);
    if (!f) {
        return;
    }
    for (int sy = 0; sy < f->h; ++sy) {
        for (int sx = 0; sx < f->w; ++sx) {
            uint8_t c = f->px[static_cast<size_t>(sy) * f->w + sx];
            if (transparent && c == 0) {
                continue;
            }
            fb.put(x + sx, y + sy, c);
        }
    }
}

void Engine::draw_shape_tint(Framebuffer& fb, int shape, int x, int y,
                             uint8_t colour) {
    int entry = (shape >> 8) & 0x3F;
    int index = shape & 0xFF;
    const ShapeFrame* f = shape_frame(entry, index);
    if (!f) {
        return;
    }
    for (int sy = 0; sy < f->h; ++sy) {
        for (int sx = 0; sx < f->w; ++sx) {
            uint8_t c = f->px[static_cast<size_t>(sy) * f->w + sx];
            if (c == 0) {
                continue;
            }
            fb.put(x + sx, y + sy, colour);
        }
    }
}

void Engine::draw_health_pip(Framebuffer& fb, int frame, int x, int y) {
    const int shape = icon_shape(frame);
    if (hud_health_ < 0) {
        draw_shape(fb, shape, x, y, true);
        return;
    }
    draw_shape_tint(fb, shape, x, y, static_cast<uint8_t>(hud_health_ & 0xFF));
}

void Engine::draw_shape_mirrored(Framebuffer& fb, int shape, int x, int y,
                                 bool transparent) {
    int entry = (shape >> 8) & 0x3F;
    int index = shape & 0xFF;
    const ShapeFrame* f = shape_frame(entry, index);
    if (!f) {
        return;
    }
    for (int sy = 0; sy < f->h; ++sy) {
        for (int sx = 0; sx < f->w; ++sx) {
            uint8_t c = f->px[static_cast<size_t>(sy) * f->w + sx];
            if (transparent && c == 0) {
                continue;
            }
            fb.put(x + (f->w - 1 - sx), y + sy, c);
        }
    }
}

void Engine::draw_object(int i, Framebuffer& fb) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int x = o.x - cam_x_ + view_x();
    int y = o.y - cam_y_ + view_y();

    switch (o.kind) {
        case 1:  // APPLE: tileset 0x09, frames 0..3
            draw_shape(fb, 0x0900 | ((o.p13 / 2) & 3), x, y, true);
            return;
        case 2:  // KNIFE: fixed frame
            draw_shape(fb, 0x0D00, x, y, true);
            return;
        case 14:  // KEY: absolute shape 0x0E06 + p13/2
            draw_shape(fb, icon_shape(0x06) + ((o.p13 / 2) & 3), x, y, true);
            return;
        case 33: {  // GEM: tileset 0x09, frames 4..7
            int ts = profile_->kind(33).tileset;
            draw_shape(fb, (ts << 8) | (((o.p13 / 2) & 3) + 4), x, y, true);
            return;
        }
        case 28: {  // TOKEN: shape bank 0x0E, icon frame from item_icons[p13]
            int id = o.p13 & 0x0F;
            int frame = profile_->item_icons[id];
            if (frame) {
                draw_shape(fb, icon_shape(frame & 0xFF), x, y, true);
            }
            return;
        }
        case 49: {  // EPIC: 8-frame spin driven by the global frame counter
            int ts = profile_->kind(49).tileset;
            draw_shape(fb, (ts << 8) | (frame_ & 7), x, y, true);
            return;
        }
        case 0x32: {  // SPINBLAD: 4-frame spin following travel direction
            int ts = profile_->kind(0x32).tileset;
            draw_shape(fb, (ts << 8) | (o.p13 & 3), x, y, true);
            return;
        }
        case 0x3E:  // FIREBULLET: 4-frame flame
            draw_shape(fb, 0x1A00 | (o.p13 & 3), x, y, true);
            return;
        case 0x3F:  // FISHBULLET: frames 0x15/0x16 by direction
            draw_shape(fb, 0x3300 | (0x15 + (o.dx > 0 ? 1 : 0)), x, y, true);
            return;
        case 36: {  // EXPL1: shrinking explosion sheet
            int frame = o.state + 0x0C - (o.p13 / 8) * 3;
            // The runtime globals DGROUP:0xBB86/0xBB88 are initialised from the
            // kind-table tilesets (0x2E / 0x0E), not 0x5A / 0x5C.
            int ts = profile_->kind(36).tileset;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }
        case 27: {  // SCORE: one 4x5 digit glyph per character (font bank 4)
            int v = o.state;
            char buf[12];
            int n = 0;
            if (v < 0) {
                v = 0;
            }
            do {
                buf[n++] = static_cast<char>('0' + (v % 10));
                v /= 10;
            } while (v != 0 && n < 11);
            for (int d = 0; d < n; ++d) {
                int digit = buf[n - 1 - d] - '0';
                draw_shape(fb, 0x0400 + digit, x + d * 4, y, true);
            }
            return;
        }

        case 4: {  // BIGANT (1644:13cf)
            int ts = profile_->kind(4).tileset;
            int frame;
            if (o.state == 0) {
                frame = (o.dx > 0 ? 10 : 0) + o.p13;
            } else {
                frame = (o.dx < 0) ? (o.state + 4) : (10 - o.state);
            }
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 5: {  // FLY (1644:1789)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(5).tileset;
            int frame = t.fly_frame[o.p13 & 3] + (o.dx > 0 ? 4 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 7: {  // DEMON (1644:2974)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(7).tileset;
            int frame = t.demon_frame[(o.p13 / 2) % 6] + (o.dx < 0 ? 4 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 8:    // BUNNY
        case 22: {  // FROG (1644:36ee)
            int ts = profile_->kind(o.kind).tileset;
            int frame = (o.dx < 0 ? 3 : 0);
            if (o.state == 1) {
                frame += (o.dy > 0 ? 2 : 0) + (o.dy < 1 ? 1 : 0);
            }
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 9: {  // INCHWORM (1644:1c33)
            int ts = profile_->kind(9).tileset;
            int frame = (o.dx < 0 ? 3 : 0) + o.p11;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 10: {  // ZAPPER (1644:1d91)
            int ts = profile_->kind(10).tileset;
            draw_shape(fb, (ts << 8) | (o.p13 & 0xFF), x, y, true);
            return;
        }

        case 11: {  // BOBSLUG (1644:1e50)
            int ts = profile_->kind(11).tileset;
            int frame = (o.dx < 1) ? (6 - o.state) : o.state;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 17: {  // FATSO (1644:2d43)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(17).tileset;
            int frame = t.fatso_frame[(o.p13 / 4) % 4] + (o.dx > 0 ? 3 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 18: {  // FIREBALL (1644:3105)
            int ts = profile_->kind(18).tileset;
            draw_shape(fb, (ts << 8) | ((o.p13 / 2) & 0xFF), x, y, true);
            return;
        }

        case 19:  // CLOUD (1644:324e)
            draw_shape(fb, 0x0E0A, x, y, true);
            return;

        case 24:  // DOOR (1644:3a00)
            // The 0xE05 marker is always drawn; the original's two sliding
            // panel shapes live in DGROUP 0x88F4/0x88FC, whose writer is not
            // recovered, so the port leaves the map cells themselves in place
            // (see docs/jill1-kind-handlers.md, kind 24 [open]).
            draw_shape(fb, 0x0E05, x + 4, y + 0x0C, true);
            return;

        case 25:  // FALLDOOR (1644:3da3)
            draw_shape(fb, 0x0E10, x, y, true);
            return;

        case 29: {  // ANT (1644:15a8)
            int ts = profile_->kind(29).tileset;
            int frame;
            if (o.state == 0) {
                frame = (o.dx < 0 ? 5 : 0) + o.p13;
            } else {
                frame = (o.dx < 0) ? (0x0E - 5 * o.state) : (5 * o.state - 1);
            }
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 30: {  // PHOENIX (1644:198a)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(30).tileset;
            int idx = o.p13 & 7;
            int frame = t.phoenix_frame[idx] +
                        (o.dx < 0 ? t.phoenix_mirror[idx] : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 32: {  // SWITCH (1a41:02ab)
            int ts = profile_->kind(32).tileset;
            draw_shape(fb, (ts << 8) | (o.state & 0xFF), x, y, true);
            return;
        }

        case 35:  // BOULDER (1a41:05f6)
            draw_shape(fb, 0x0E18 + (o.p13 & 3), x, y, true);
            return;

        case 37: {  // EXPL2 (1a41:0b91)
            const HandlerTables& t = *profile_->tables;
            int idx = o.p13 < 8 ? o.p13 : 7;
            int ts = profile_->kind(37).tileset;
            draw_shape(fb, (ts << 8) | (t.expl2_frame[idx] & 0xFF), x, y, true);
            return;
        }

        case 38:  // STALAG (1a41:0c54)
            draw_shape(fb, 0x0E21, x, y, true);
            return;

        case 42: {  // MEGA (1a41:165a): dx is the frame index
            int ts = profile_->kind(42).tileset;
            draw_shape(fb, (ts << 8) | (o.dx & 0xFF), x, y, true);
            return;
        }

        case 43: {  // BAT (1a41:17ee)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(43).tileset;
            if (o.state == 0) {
                draw_shape(fb, (ts << 8) | 6, x, y + 2, true);
                return;
            }
            int idx = (o.p13 / 2) & 3;
            int frame = t.bat_y[idx] + (o.dx < 0 ? 3 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x,
                       y + t.bat_frame[idx], true);
            return;
        }

        case 47: {  // CRAB (1a41:22f3)
            int ts = profile_->kind(47).tileset;
            draw_shape(fb, (ts << 8) | (o.p13 & 0xFF), x, y, true);
            return;
        }

        case 52: {  // BUTTON (1a41:3261)
            int ts = profile_->kind(52).tileset;
            int frame = 1 - o.state + (o.dx > 0 ? 2 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 16: {  // WISEMAN (1644:234b)
            int ts = profile_->kind(16).tileset;
            int frame = (o.dx > 0 ? 5 : 0) + o.state;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 20:  // TEXT6 (1644:3371): 6-pixel font, one pixel down
        case 21: {  // TEXT8 (1644:35df): 8-pixel font
            // Board text pages type out at the dialog speed (1 char/tick, or 4
            // with FASTER DIALOG BOXES); with no caption system they would
            // appear all at once.
            const bool eight = o.kind == 21;
            const size_t shown =
                std::min(o.text.size(), text_reveal_);
            const std::string part = o.text.substr(0, shown);
            draw_text_font(fb, x, y + (eight ? 0 : 1),
                           static_cast<uint8_t>(o.dx > 0 && o.dx < 16 ? o.dx : 7),
                           eight ? 1 : 2, eight ? 8 : 6, part.c_str());
            return;
        }

        case 64: {  // EYE (1a53:483a): body, then the pupil on top
            int ts = profile_->kind(64).tileset;
            draw_shape(fb, (ts << 8) | 0, x, y, true);
            draw_shape(fb, (ts << 8) | 1, x + o.dx + 5, y + o.dy + 4, true);
            return;
        }

        case 65: {  // VINECLIMB (1a53:49b1)
            int ts = profile_->kind(65).tileset;
            draw_shape(fb, (ts << 8) | ((o.p13 / 2) & 3), x, y, true);
            return;
        }

        case 66: {  // FLAG (1a53:4baa)
            int ts = profile_->kind(66).tileset;
            int frame = o.p13 / 2 + 1;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x + 2, y, true);
            return;
        }

        case 67: {  // MAPDEMO (1a53:4df2): the "MAP" label
            if (!map_mode_) {
                return;  // hidden outside the hub / world-map board
            }
            int ts = profile_->kind(67).tileset;
            // Frames 0x14..0x16 are M/A/P (0x10..0x13 spell DEMO and used to
            // overlap here, leaving a stray trailing O).  Draw at a fixed
            // viewport position so the label cannot drift with the camera.
            static const int kBase = 0x14;
            static const int kLen = 3;
            const int sx = view_x() + 0x10;
            // The modern HUD occupies the top 12 px; keep the label below it.
            const int sy = view_y() + (hud_classic_ ? 4 : 16);
            for (int k = 0; k < kLen; ++k) {
                draw_shape(fb, (ts << 8) | ((kBase + k) & 0xFF),
                           sx + k * 16, sy, true);
            }
            return;
        }

        case 68: {  // ROMAN (1650:2f6b)
            int ts = profile_->kind(68).tileset;
            const ShapeEntry* art = shapes_.entry(ts & 0x3F);
            const int frames = art ? static_cast<int>(art->frames.size()) : 0;
            int frame = o.p13 & 7;
            bool mirror = false;
            if (o.dx > 0) {
                // Onesimus ships real right-facing frames 8..15; Jill 2/3's
                // 7-frame sheet has none, so mirror the left-facing art.
                if (frames > 8) {
                    frame += 8;
                } else {
                    mirror = true;
                }
            }
            if (frames > 0) {
                frame %= frames;
            }
            if (mirror) {
                draw_shape_mirrored(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            } else {
                draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            }
            return;
        }

        case 13: {  // PAUL (1644:22d0): one pose per state byte
            int ts = profile_->kind(13).tileset;
            draw_shape(fb, (ts << 8) | (o.state & 0xFF), x, y, true);
            return;
        }

        case 31: {  // FIRE (1a41:0160): 6 frames, second bank when falling
            int ts = profile_->kind(31).tileset;
            int frame = o.p13 / 2 + ((o.dy > 0) ? 6 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 39: {  // SNAKE (1a41:0dcb): head / body run / tail
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(39).tileset;
            int p13 = o.p13 & 3;
            if (o.dx < 1) {
                draw_shape(fb, (ts << 8) | (t.snake_head[p13] + 9), x, y, true);
                for (int i = 1; i <= (o.w - 0x18) / 8; ++i) {
                    draw_shape(fb, (ts << 8) | t.snake_mid[p13],
                               x + (i + 1) * 8, y + 0x0C, true);
                }
                draw_shape(fb, (ts << 8) | (t.snake_tail[p13] + 5),
                           x + o.w - 8, y + 0x0C, true);
            } else {
                draw_shape(fb, (ts << 8) | t.snake_head[p13],
                           x + o.w - 0x10, y, true);
                for (int i = 1; i <= (o.w - 0x18) / 8; ++i) {
                    draw_shape(fb, (ts << 8) | t.snake_mid[p13], x + i * 8,
                               y + 0x0C, true);
                }
                draw_shape(fb, (ts << 8) | t.snake_tail[p13], x, y + 0x0C, true);
            }
            return;
        }

        case 40: {  // SEAROCK (1a41:46d5)
            int ts = profile_->kind(40).tileset;
            draw_shape(fb, (ts << 8) | 0x22, x, y, true);
            return;
        }

        case 41: {  // BOLL (1a41:1261)
            int ts = profile_->kind(41).tileset;
            draw_shape(fb, (ts << 8) | (o.p13 & 0xFF), x, y, true);
            return;
        }

        case 44: {  // KNIGHT (1a41:1bb4): 11-step swing loop
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(44).tileset;
            int p11 = o.p11;
            if (p11 < 0 || p11 > 10) {
                p11 = 0;
            }
            draw_shape(fb, (ts << 8) | (t.knight_frame[p11] & 0xFF),
                       x + t.knight_xoff[p11], y, true);
            return;
        }

        case 45: {  // BEENEST (1a41:1e13): three-frame opening nest
            int ts = profile_->kind(45).tileset;
            int frame = 0;
            if (o.p13 == 1) {
                frame = 1;
            } else if (o.p13 == 2) {
                frame = (o.dx < 1) ? 2 : 3;
            }
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 46: {  // BEESWARM (1a41:2069): frames 4..9
            int ts = profile_->kind(46).tileset;
            int frame = o.p13 + 4 + ((o.dx >= 1) ? 3 : 0);
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 48: {  // CROC (1a41:269f): head and tail halves
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(48).tileset;
            int p13 = o.p13 & 3;
            int flip = (o.dx < 1) ? 8 : 0;
            int hx = (o.dx < 1) ? x : x + 0x20;
            int tx = (o.dx < 1) ? x + 0x20 : x;
            draw_shape(fb, (ts << 8) | ((t.croc_head[p13] + flip) & 0xFF),
                       hx, y, true);
            draw_shape(fb, (ts << 8) | ((t.croc_tail[p13] + flip) & 0xFF),
                       tx, y, true);
            return;
        }

        case 51: {  // SKULL (1a41:3029): body plus two small sprites
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(51).tileset;
            int p11 = o.p11 & 7;
            draw_shape(fb, (ts << 8) | (t.skull_a[(p11 / 2) & 3] & 0xFF),
                       x, y, true);
            if (o.state != 0) {
                draw_shape(fb, (ts << 8) | (t.skull_b[p11] & 0xFF),
                           x, y + 5, true);
                draw_shape(fb, (ts << 8) | (t.skull_b[(p11 + 4) & 7] & 0xFF),
                           x + 10, y + 6, true);
            }
            return;
        }

        case 53: {  // PAC (1a41:3402)
            int ts = profile_->kind(53).tileset;
            int frame = 0;
            if (o.dx < 0) {
                frame = 1;
            } else if (o.dy < 1) {
                if (o.dy < 0) {
                    frame = 2;
                }
            } else {
                frame = 3;
            }
            draw_shape(fb, (ts << 8) | frame, x, y, true);
            return;
        }

        case 58: {  // BUBBLE (1a41:3834): frames 6..8
            int ts = profile_->kind(58).tileset;
            draw_shape(fb, (ts << 8) | ((o.p13 + 6) & 0xFF), x, y, true);
            return;
        }

        case 59: {  // JELLYFISH (1a41:3957)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(59).tileset;
            int idx = o.p13 % 10;
            if (idx < 0) {
                idx += 10;
            }
            draw_shape(fb, (ts << 8) | (t.jelly[idx] & 0xFF), x, y, true);
            return;
        }

        case 60: {  // BADFISH (1a41:3d03)
            const HandlerTables& t = *profile_->tables;
            int ts = profile_->kind(60).tileset;
            int frame = t.badfish[o.p13 & 3] + ((o.dx > 0) ? 3 : 0) + 0x0F;
            draw_shape(fb, (ts << 8) | (frame & 0xFF), x, y, true);
            return;
        }

        case 61:  // ELEV (1a41:3fe8): plain 0xE2C platform
            draw_shape(fb, icon_shape(0x2C), x, y, true);
            return;

        default:
            break;
    }

    int tileset = profile_->kind(o.kind).tileset;
    if (!tileset) {
        return;
    }
    int shape = (tileset << 8) | ((o.p13 / 2) & 0xFF);
    draw_shape(fb, shape, x, y, true);
}

void Engine::clear_frame_marks() {
    for (uint16_t& cell : map_) {
        cell &= 0x3FFF;
    }
    for (BoardObject& o : objects_) {
        o.flags &= 0x3FFF;
    }
}

// --- engine services ------------------------------------------------------

int Engine::add_obj(int kind, int x, int y) {
    // The original stores up to 0x100 records; boards themselves may contain
    // at most 192.  Dead records are compacted at the start of every tick
    // (FUN_26f4_2506), so this is not a per-frame spawn budget.
    const int max = kObjectLimit;
    if (static_cast<int>(objects_.size()) >= max) {
        return -1;
    }
    const KindInfo& k = profile_->kind(kind);
    BoardObject o;
    o.kind = kind;
    o.x = x;
    o.y = y;
    o.w = k.width;
    o.h = k.height;
    o.prev_x = x;
    o.prev_y = y;
    objects_.push_back(o);
    return static_cast<int>(objects_.size()) - 1;
}

int Engine::map_tile(int col, int row) const {
    if (col < 0 || col >= Board::kCols || row < 0 || row >= Board::kRows) {
        return -1;
    }
    return map_[col * Board::kRows + row] & 0x3FFF;
}

uint16_t Engine::cando(int i, int x, int y, uint16_t mask) const {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int kind_h = profile_->kind(o.kind).height;
    uint16_t extra = 2;
    uint16_t result = 0xFFFF;

    int r0 = y >> 4;
    int r1 = (y + o.h + 0x0F) >> 4;
    int c0 = x >> 4;
    int c1 = (x + o.w + 0x0F) >> 4;
    // The bottom-edge test uses the object's *current* y (26F4:2227 reads
    // DGROUP:0x96AF), not the candidate position.
    int bottom = (o.y + kind_h + 0x0F) >> 4;

    for (int r = r0; r < r1; ++r) {
        if (bottom <= r) {
            extra = 0;
        }
        for (int c = c0; c < c1; ++c) {
            int cc = std::max(0, std::min(c, Board::kCols - 1));
            int rr = std::max(0, std::min(r, Board::kRows - 1));
            uint16_t id = map_[cc * Board::kRows + rr] & 0x3FFF;
            result &= static_cast<uint16_t>((tiles_.at(id).flags | extra) & mask);
        }
    }
    return result;
}

void Engine::set_origin(int i, int x, int y) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    if (y < 0) {
        y = 0;
    } else if (y > 0x3F0 - o.h) {
        y = 0x3F0 - o.h;
    }
    if (x < 0) {
        x = 0;
    } else if (x > 0x7F0 - o.w) {
        x = 0x7F0 - o.w;
    }
    o.x = x;
    o.y = y;
}

bool Engine::try_move(int i, int x, int y) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    if (cando(i, x, y, 3) == 3) {
        set_origin(i, x, y);
        o.dx = in_.h;
        return true;
    }
    if (cando(i, o.x, y, 3) == 3) {
        set_origin(i, o.x, y);
        o.dx = 0;
        return true;
    }
    o.dx = 0;
    return false;
}

bool Engine::just_move(int i, int x, int y) {
    if (cando(i, x, y, 1) != 0) {
        set_origin(i, x, y);
        return true;
    }
    return false;
}

// 26F4:1C89 _trymove: full move, then x-only, then y-only.  The mask is 1 for
// horizontal/upward moves and 3 when moving down (mask 3 also requires the
// "blocked from above" bit).  Returns 1/2/4 for the axis that worked, 0 when
// every candidate is blocked.
int Engine::move_obj(int i, int x, int y) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int mask = 1;
    if (o.y < y) {
        mask |= 2;
    }
    if (cando(i, x, y, static_cast<uint16_t>(mask)) == mask) {
        set_origin(i, x, y);
        return 1;
    }
    if (cando(i, o.x, y, static_cast<uint16_t>(mask)) == mask) {
        set_origin(i, o.x, y);
        return 2;
    }
    // Do not report a horizontal-only move when x did not change: the frog
    // form relies on a 0 result to snap onto the floor when a fall is blocked.
    if (x != o.x && cando(i, x, o.y, static_cast<uint16_t>(mask)) == mask) {
        set_origin(i, x, o.y);
        return 4;
    }
    return 0;
}

// 26F4:1F35 _crawl: only move when the object's bottom edge sits on a tile
// boundary and the row underneath every destination column is solid.  This is
// what keeps walking enemies on the floor instead of walking off ledges.
int Engine::crawl(int i, int dx, int dy) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int nx = o.x + dx;
    int ny = o.y + dy;
    if (((o.h + ny) & 0x0F) != 0) {
        return 0;
    }
    int c0 = nx >> 4;
    int c1 = (o.w + nx + 0x0F) >> 4;
    int r = (o.h + ny) >> 4;
    for (int c = c0; c < c1; ++c) {
        int cc = std::max(0, std::min(c, Board::kCols - 1));
        int rr = std::max(0, std::min(r, Board::kRows - 1));
        uint16_t id = map_[cc * Board::kRows + rr] & 0x3FFF;
        // Spikes/thorns also have the "passable + stand" bit pair, so the
        // original treats them as holes and walkers pace forever at the pit
        // edge.  Let crawlers cross them instead (enemies do not run the tile
        // contact hook, so this is purely a movement fix).
        const bool spike = id == 0xA6 || id == 0x141 || id == 0x142 ||
                           id == 0x155 || id == 0x156;
        if ((tiles_.at(id).flags & 3) == 3 && !spike) {
            return 0;
        }
    }
    if (cando(i, nx, ny, 1) != 1) {
        return 0;
    }
    set_origin(i, nx, ny);
    return 1;
}

// 26F4:2605 _fishdo: move test that treats water (0x2000) as walkable.
bool Engine::fishdo(int i, int x, int y) {
    if (cando(i, x, y, 0x2001) == 0x2001) {
        set_origin(i, x, y);
        return true;
    }
    return false;
}

// 26F4:1DA8 _onscreen: object AABB against the camera rect.
bool Engine::onscreen(int i) const {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    return !(o.x + o.w < cam_x_ || o.y + o.h < cam_y_ ||
             cam_x_ + view_w() < o.x ||
             cam_y_ + view_h() < o.y);
}

// 26F4:06B4 _pointvect: sign of the direction from object i to the player.
void Engine::pointvect(int i, int& dx, int& dy) const {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    const BoardObject& p = objects_[0];
    dx = (o.x < p.x) - (p.x < o.x);
    dy = (o.y < p.y) - (p.y < o.y);
}

// 26F4:263A _vectdist: velocity of magnitude `speed` from b towards a.
void Engine::vectdist(int a, int b, int& dx, int& dy, int speed) const {
    const BoardObject& oa = objects_[static_cast<size_t>(a)];
    const BoardObject& ob = objects_[static_cast<size_t>(b)];
    int ox = oa.x - ob.x;
    int oy = oa.y - ob.y;
    int ax = ox < 0 ? -ox : ox;
    int ay = oy < 0 ? -oy : oy;
    if (ox == 0) {
        dy = oy == 0 ? 0 : ((oy > 0 ? 1 : -1) * speed);
        dx = 0;
        return;
    }
    if (oy == 0) {
        dx = (ox > 0 ? 1 : -1) * speed;
        dy = 0;
        return;
    }
    if (ay < ax) {
        dy = (oy * speed) / ax;
        dx = (ox > 0 ? 1 : -1) * speed;
    } else {
        dx = (ox * speed) / ay;
        dy = (oy > 0 ? 1 : -1) * speed;
    }
}

// 26F4:2772 _dist: Manhattan distance.
int Engine::dist(int a, int b) const {
    const BoardObject& oa = objects_[static_cast<size_t>(a)];
    const BoardObject& ob = objects_[static_cast<size_t>(b)];
    return std::abs(oa.x - ob.x) + std::abs(oa.y - ob.y);
}

// 26F4:25B8 _hitplayer: damage the player unless it is briefly invulnerable.
void Engine::hit_player(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    static const uint16_t kStateFlags[6] = {1, 0, 1, 0, 2, 2};
    bool invulnerable = false;
    if (has_player_) {
        const BoardObject& p = objects_[0];
        invulnerable = p.kind == 0 && p.state >= 0 && p.state < 6 &&
                       (kStateFlags[p.state] & 2) != 0;
    }
    if (o.ttl == 0 && !invulnerable) {
        player_kill(1, 0);
    }
    o.ttl = 3;
}

// 26F4:0091 _killaward: score, footprint, then the poof state.
void Engine::killaward(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    add_score(profile_->kind(o.kind).count, o.x, o.y);
    stamp_footprint(i);
    kill_obj(i);
}

// 26F4:012D _stampfootprint: mark every covered cell as blocked.
void Engine::stamp_footprint(int i) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int r0 = o.y >> 4;
    int r1 = (o.y + o.h + 0x0F) >> 4;
    int c0 = o.x >> 4;
    int c1 = (o.x + o.w + 0x0F) >> 4;
    for (int r = r0; r < r1; ++r) {
        for (int c = c0; c < c1; ++c) {
            if (c >= 0 && c < Board::kCols && r >= 0 && r < Board::kRows) {
                map_[c * Board::kRows + r] |= 0xC000;
            }
        }
    }
}

// 26F4:020C _setobjsize: reset the AABB from the kind table.
void Engine::set_obj_size(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    const KindInfo& k = profile_->kind(o.kind);
    o.w = k.width;
    o.h = k.height;
}

// 26F4:27DD _touchbkgnd: eat 0xA7 tiles under the given corner.
int Engine::touch_bkgnd(int i, int x, int y) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    int n = 0;
    int c1 = (x + o.w) >> 4;
    int r1 = (y + o.h) >> 4;
    for (int c = x >> 4; c <= c1; ++c) {
        for (int r = y >> 4; r <= r1; ++r) {
            if (c < 0 || c >= Board::kCols || r < 0 || r >= Board::kRows) {
                continue;
            }
            uint16_t& cell = map_[c * Board::kRows + r];
            if ((cell & 0x3FFF) == 0xA7) {
                cell = 0xC000;
                if (n == 0) {
                    explode1(c << 4, r << 4, 5);
                    sound(2, 0x31);
                }
                ++n;
            }
        }
    }
    return n;
}

// 26F4:20EF _sendtrig: call every object on the matching trigger channel.
void Engine::send_trig(int trig, int code, int self) {
    int count = static_cast<int>(objects_.size());
    for (int i = 0; i < count; ++i) {
        const BoardObject& o = objects_[static_cast<size_t>(i)];
        if (o.kind < profile_->kind_limit() &&
            (profile_->kind(o.kind).flags & 0x0100) && o.p13 == trig) {
            trig_object(i, code, self);
        }
    }
}

// 1A41:0930 _explode2: single EXPL2 particle centred on the object.  The
// original centres a fixed 16x32 sprite (the EXPL2 kind size is 16x32).
void Engine::explode2(int i) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    const KindInfo& e = profile_->kind(37);
    add_obj(37, o.x + (o.w - e.width) / 2, o.y + o.h - e.height);
}

int Engine::inv_count(int id) const {
    int n = header_[4] | (header_[5] << 8);
    int count = 0;
    for (int i = 0; i < n && i < 16; ++i) {
        if ((header_[6 + i * 2] | (header_[7 + i * 2] << 8)) == id) {
            ++count;
        }
    }
    return count;
}

void Engine::add_item(int id) {
    int n = header_[4] | (header_[5] << 8);
    if (n >= kMaxInventory) {
        return;
    }
    header_[4] = static_cast<uint8_t>(n + 1);
    header_[5] = 0;
    header_[6 + n * 2] = static_cast<uint8_t>(id);
    header_[7 + n * 2] = 0;
}

bool Engine::del_item(int id) {
    int n = header_[4] | (header_[5] << 8);
    for (int i = 0; i < n && i < 16; ++i) {
        if ((header_[6 + i * 2] | (header_[7 + i * 2] << 8)) != id) {
            continue;
        }
        for (int j = i; j + 1 < n; ++j) {
            header_[6 + j * 2] = header_[6 + (j + 1) * 2];
            header_[7 + j * 2] = header_[7 + (j + 1) * 2];
        }
        header_[4] = static_cast<uint8_t>(n - 1);
        return true;
    }
    return false;
}

// 1FAE:2E61: transformation tokens.  4-7 become frog/bird/spider/fish; token
// 0 ("FOOF!") reverts to normal Jill.  The object stays in slot 0.
void Engine::morph_player(int item) {
    if (!has_player_) {
        return;
    }
    int kind = 0;
    switch (item) {
        case 4: kind = 57; break;  // JILLFROG
        case 5: kind = 56; break;  // JILLBIRD
        case 6: kind = 55; break;  // JILLSPIDER
        case 7: kind = 54; break;  // JILLFISH
        default: kind = 0; break;  // FOOF! / anything else
    }
    BoardObject& p = objects_[0];
    const int old_h = p.h;
    p.kind = kind;
    if (kind == 0) {
        p.w = 0x10;
        p.h = 0x20;
    } else {
        set_obj_size(0);
    }
    // 1FAE:2E61 keeps the feet planted: the new top is old_bottom - new_h.
    // Without this the 10-px frog or 16-px fish appears floating after a morph.
    p.y += old_h - p.h;
    p.x &= ~7;
    p.state = 0;
    p.dx = 0;
    p.dy = 0;
    p.p0f = 0;
    p.p11 = 0;
    p.p13 = 0;
    p.p1b = 0;
    p.ttl = 0;
    p.raise = 0;
    p.prev_x = p.x;
    p.prev_y = p.y;
}

void Engine::player_kill(int amount, int dir) {
    if (!has_player_) {
        return;
    }
    if (invincible_) {  // debug: ignore all damage sources
        return;
    }
    BoardObject& p = objects_[0];
    if (p.kind == 0x17) {
        return;
    }
    // state_flags = {1,0,1,0,2,2}; bit 1 = invulnerable.
    static const uint16_t kStateFlags[6] = {1, 0, 1, 0, 2, 2};
    if (p.kind == 0 && p.state >= 0 && p.state < 6 &&
        (kStateFlags[p.state] & 2)) {
        return;
    }
    int damage = amount - inv_count(10);
    if (damage <= 0) {
        return;
    }
    int hp = health() - damage;
    header_[2] = static_cast<uint8_t>(hp < 0 ? 0 : (hp & 0xFF));
    header_[3] = 0;
    p.flags |= 0xC000;
    if (hp >= 1) {
        sound(4, 0x13);
        return;
    }

    header_[2] = 0;
    p.kind = 0;
    p.w = 0x10;
    p.h = 0x20;
    p.state = 5;
    p.p11 = 0;
    p.p0f = dir;
    if (dir == 1) {
        p.y = (p.y - 1) & ~0x0F;
    }
    p.dy = -0x0C;
    sound(4, dir + 0x27);
    explode1(p.x, p.y, 10);
}

// Headless testing aid (--start): teleport the player and move the respawn
// point with it so death tests can be repeated.
void Engine::warp_player(int x, int y) {
    if (!has_player_) {
        return;
    }
    objects_[0].x = x;
    objects_[0].y = y;
    spawn_x_ = x;
    spawn_y_ = y;
    cam_x_ = std::max(0, std::min(x + 8 - view_center_x(),
                                  Board::kCols * 16 - view_w()));
    cam_y_ = std::max(0, std::min(y + 16 - view_center_y(),
                                  Board::kRows * 16 - view_h()));
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    objects_[0].prev_x = x;
    objects_[0].prev_y = y;
}

void Engine::debug_spawn_blade() {
    if (!has_player_) {
        return;
    }
    BoardObject& p = objects_[0];
    const int facing = p.p1b ? p.p1b : 1;
    int n = add_obj(0x32, p.x + facing * 0x0C + 4, p.y + 2);
    if (n >= 0) {
        objects_[static_cast<size_t>(n)].dx = facing * 6;
    }
}

void Engine::respawn() {
    BoardObject& p = objects_[0];
    p.kind = 0;
    p.x = spawn_x_;
    p.y = spawn_y_;
    p.dx = 0;
    p.dy = 0;
    p.w = 0x10;
    p.h = 0x20;
    p.state = 4;  // recover state; state 4 ticks down p11 and returns control
    p.p0f = 0;
    p.p11 = 0;
    p.p13 = 0;
    p.p1b = 0;
    p.raise = 0;
    p.flags = 0;
    p.prev_x = p.x;
    p.prev_y = p.y;
    cam_x_ = std::max(0, std::min(p.x + 8 - view_center_x(),
                                  Board::kCols * 16 - view_w()));
    cam_y_ = std::max(0, std::min(p.y + 16 - view_center_y(),
                                  Board::kRows * 16 - view_h()));
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    header_[2] = 6;
    header_[3] = 0;
    // Clear the temporary collision bits the death frame left on the map.
    for (uint16_t& cell : map_) {
        cell &= 0x3FFF;
    }
}

// 1415:05FD / 0415:0A8B: replay a recorded input macro.  The file has no
// header; each record is an optional base-128 tick delta, a mask, then one
// signed byte per set mask bit (jill1-engine.md 13.1).
void Engine::start_macro(const std::string& name) {
    macro_.clear();
    macro_pos_ = 0;
    macro_tick_ = 0;
    macro_last_ = 0;
    macro_playing_ = false;
    macro_held1_ = false;
    macro_held2_ = false;
    if (name.empty()) {
        return;
    }
    std::string clean = name;
    while (!clean.empty() && clean.front() == '&') {
        clean.erase(clean.begin());
    }
    FILE* f = std::fopen(join(data_dir_, clean).c_str(), "rb");
    if (!f) {
        std::fprintf(stderr, "macro %s: cannot open\n", clean.c_str());
        return;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(f);
        return;
    }
    std::vector<uint8_t> d(static_cast<size_t>(size));
    const size_t got = std::fread(d.data(), 1, d.size(), f);
    std::fclose(f);
    d.resize(got);

    size_t p = 0;
    // Record layout (from the recorder at 1415:0A8B and the replayer at
    // 1415:093B): rec[0] is mask + values; every later record starts with an
    // absolute tick (base-128 varint) and then the mask + values.  The
    // replayer reads the tick *after* each record, so a record's stored tick
    // is the deadline at which the next record takes effect.
    int time = 0;
    bool first = true;
    while (p < d.size()) {
        if (first) {
            first = false;
        } else {
            if (p >= d.size()) {
                break;
            }
            int v = d[p++];
            if (v & 0x80) {
                if (p >= d.size()) {
                    break;
                }
                time = (v & 0x7F) | (d[p++] << 7);
            } else {
                time = v;
            }
        }
        if (p >= d.size()) {
            break;
        }
        const int mask = d[p++];
        int vals[5] = {0, 0, 0, 0, 0};
        for (int bit = 0; bit < 5; ++bit) {
            if (mask & (1 << bit)) {
                if (p >= d.size()) {
                    break;
                }
                vals[bit] = static_cast<int8_t>(d[p++]);
            }
        }
        MacroRec r;
        r.time = time;
        r.mask = mask;
        r.h = vals[0] < 0 ? -1 : (vals[0] > 0 ? 1 : 0);
        r.v = vals[1] < 0 ? -1 : (vals[1] > 0 ? 1 : 0);
        r.btn1 = vals[2] != 0;
        r.btn2 = vals[3] != 0;
        macro_.push_back(r);
        macro_last_ = time;
    }
    if (!macro_.empty()) {
        std::fprintf(stderr, "macro %s: %zu records, %d ticks\n",
                     clean.c_str(), macro_.size(), macro_last_);
    }
    macro_playing_ = !macro_.empty();
}

void Engine::apply_macro_input() {
    while (macro_pos_ < macro_.size() &&
           macro_[macro_pos_].time <= macro_tick_) {
        const MacroRec& r = macro_[macro_pos_++];
        // A record only updates the fields named by its mask; the original
        // leaves the rest of the input state alone between records.
        if (r.mask & 0x01) {
            in_.h = r.h;
        }
        if (r.mask & 0x02) {
            in_.v = r.v;
        }
        if (r.mask & 0x04) {
            macro_held1_ = r.btn1;
        }
        if (r.mask & 0x08) {
            macro_held2_ = r.btn2;
        }
    }
    // The input poller's edge latches behave as if these were real buttons.
    if (!macro_held1_) {
        latch1_ = false;
    }
    if (!macro_held2_) {
        latch2_ = false;
    }
    in_.btn1 = macro_held1_ && !latch1_;
    in_.btn2 = macro_held2_ && !latch2_;
    ++macro_tick_;
    if (macro_pos_ >= macro_.size() && macro_tick_ > macro_last_) {
        macro_playing_ = false;
        in_ = InputState{};
    }
}

// 22AB:1531 pending-string handler.  A CHECKPT copies its string into the
// pending buffer; the main loop consumes it at the top of the next iteration.
//   '*' music / '#' conditional load / '&' macro  -> audio/macro, not ported
//   '!'                                            -> end of level, back to the hub
//   bare filename                                  -> load that board
// The original restores the score (and level id) after the load; the port
// keeps the whole live header, which also preserves health.  Keys/weapons are
// dropped on exit unless the "retain items" enhancement is enabled.
void Engine::process_pending_warp() {
    if (pending_warp_.empty()) {
        return;
    }
    std::string cmd;
    cmd.swap(pending_warp_);
    const char c = cmd[0];
    if (c == '*' || c == '#' || c == '&') {
        if (c == '&') {
            // Ending / demo input script: play it back instead of changing
            // boards.  The ending boards use `&end1.mac` / `&end2.mac`.
            start_macro(cmd.substr(1));
        } else if (c == '*') {
            // Level / hub music: the app resolves the name against the data
            // directory and starts the CMF track.
            std::string name = cmd.substr(1);
            while (!name.empty() &&
                   (name.back() == ' ' || name.back() == '\r')) {
                name.pop_back();
            }
            // The hub's music checkpoint can be touched every tick (the
            // player is placed on it), so only report a change of track.
            if (!name.empty() && name != last_music_) {
                music_request_ = name;
                last_music_ = name;
            }
        }
        return;  // music / conditional / macro commands: no board change
    }

    const std::array<uint8_t, Board::kHeaderSize> saved = header_;
    const int level = level_id();
    std::string err;
    int loaded_level = 0;
    bool restored_hub = false;
    if (c == '!') {
        // FUN_22ab_1531 restores the board stashed in `temp` when a level is
        // entered.  The port keeps the hub map/objects in memory, so completed
        // entrances and every other change on the map screen survive.
        if (hub_valid_) {
            board_ = hub_backup_;
            map_ = hub_backup_.map;
            for (uint16_t& cell : map_) {
                cell &= 0x3FFF;
            }
            objects_ = hub_backup_.objects;
            has_player_ = !objects_.empty() &&
                          (objects_[0].kind == 0 || objects_[0].kind == 23);
            for (BoardObject& o : objects_) {
                o.prev_x = o.x;
                o.prev_y = o.y;
            }
            active_.clear();
            order_.clear();
            board_name_ = hub_name_;
            map_mode_ = true;
            loaded_level = hub_level_;
            restored_hub = true;
        } else if (!load_board(game_.map_board, err)) {
            std::fprintf(stderr, "warp %s: %s\n", game_.map_board.c_str(),
                         err.c_str());
            return;
        } else {
            loaded_level = level_id();
        }
    } else {
        // Leaving the hub: stash it exactly as the original writes the current
        // board to its `temp` save before loading the destination.
        if (map_mode_) {
            hub_backup_.map = map_;
            hub_backup_.header = header_;
            hub_backup_.objects = objects_;
            hub_valid_ = true;
        }
        if (!load_board(cmd, err)) {
            std::fprintf(stderr, "warp %s: %s\n", cmd.c_str(), err.c_str());
            return;
        }
        loaded_level = level_id();
    }
    header_ = saved;
    if (!retain_items_) {
        // Default: keys and weapons are left behind when a level is exited,
        // but unused gems (inventory item 3, the keygem) carry over so they
        // can open a door in a later level.
        const int n = header_[4] | (header_[5] << 8);
        int out = 0;
        for (int i = 0; i < n && i < Engine::kMaxInventory; ++i) {
            const int id = header_[6 + i * 2] | (header_[7 + i * 2] << 8);
            if (id != 3) {
                continue;
            }
            header_[6 + out * 2] = static_cast<uint8_t>(id);
            header_[7 + out * 2] = 0;
            ++out;
        }
        header_[4] = static_cast<uint8_t>(out);
        header_[5] = 0;
        std::fill(header_.begin() + 6 + out * 2, header_.begin() + 6 + 32, 0);
    }
    int active_level = level;
    if (c == '!') {
        active_level = restored_hub ? hub_level_ : loaded_level;
    }
    header_[0] = static_cast<uint8_t>(active_level & 0xFF);
    header_[1] = static_cast<uint8_t>((active_level >> 8) & 0xFF);
    update_caption_for_level(active_level);
    if (!map_mode_) {
        place_at_checkpoint(active_level);
    } else if (has_player_) {
        // Returning to the hub keeps the player where the stashed board left
        // them (at the entrance just used).  The map's p13=0 CHECKPT is the
        // music trigger, not an entrance, so running place_at_checkpoint(0)
        // here teleported Jill to (0,0) - the "glitchy map spawn".
        const BoardObject& p = objects_[0];
        spawn_x_ = p.x;
        spawn_y_ = p.y;
        cam_x_ = std::max(0, std::min(p.x + 8 - view_center_x(),
                                     Board::kCols * 16 - view_w()));
        cam_y_ = std::max(0, std::min(p.y + 16 - view_center_y(),
                                     Board::kRows * 16 - view_h()));
        prev_cam_x_ = cam_x_;
        prev_cam_y_ = cam_y_;
    }

    if (c == '!') {
        // The entrance that led into the finished level is consumed, which is
        // also how the original marks it as completed on the hub board.
        for (int i = 1; i < static_cast<int>(objects_.size()); ++i) {
            const BoardObject& o = objects_[static_cast<size_t>(i)];
            if (o.kind == 12 && o.p13 == level) {
                kill_obj(i);
                break;
            }
        }
    }
}

// 26F4:04C7: after a board load, stand the player on the CHECKPT whose p13
// matches the active level id.  Normal Jill is placed 16 px above the flag and
// snapped to an 8-px column; TINY ignores the -16 adjustment.
void Engine::place_at_checkpoint(int level) {
    if (!has_player_) {
        return;
    }
    int idx = -1;
    for (int i = 1; i < static_cast<int>(objects_.size()); ++i) {
        const BoardObject& o = objects_[static_cast<size_t>(i)];
        if (o.kind == 12 && o.p13 == level) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        return;
    }
    const BoardObject& cp = objects_[static_cast<size_t>(idx)];
    BoardObject& p = objects_[0];
    // Ending / demo boards (50.jn1, 20.jn2) put the macro trigger next to the
    // board's own start position.  The recording begins when Jill first
    // overlaps the trigger, so start her just left of it instead of snapping
    // to the flag and starting the script one tile late.
    const bool entrance_macro = !cp.text.empty() && cp.text[0] == '&';
    int x = p.x;
    int y = p.y;
    if (entrance_macro) {
        // The recording starts when the player first overlaps the trigger,
        // i.e. with her right edge at cp.x.  Start 16 px left of the flag.
        x = (cp.x & ~7) - 16;
        y = cp.y - 16;
        p.x = x;
        p.y = y;
        p.dx = 0;
        p.dy = 0;
        p.prev_x = x;
        p.prev_y = y;
    } else {
        x = cp.x & ~7;
        y = cp.y;
        if (p.kind != 0x17) {
            y -= 0x10;
        }
        p.x = x;
        p.y = y;
        p.dx = 0;
        p.dy = 0;
        p.state = 4;
        p.p11 = 0;
        p.raise = 0;
        p.flags = 0;
        p.prev_x = p.x;
        p.prev_y = p.y;
    }
    spawn_x_ = x;
    spawn_y_ = y;
    cam_x_ = std::max(0, std::min(x + 8 - view_center_x(),
                                  Board::kCols * 16 - view_w()));
    cam_y_ = std::max(0, std::min(y + 16 - view_center_y(),
                                  Board::kRows * 16 - view_h()));
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    // A level can start on its macro checkpoint (50.jn1 / 20.jn2).  The
    // per-tick CHECKPT handler skips text when p13 == the active level, so
    // queue the entrance macro here instead; the next tick starts playback.
    if (!cp.text.empty() && cp.text[0] == '&') {
        pending_warp_ = cp.text;
    }
}

std::string Engine::save_path(int slot) const {
    // A mod can override the profile's save stem so its slots do not collide
    // with the base game's (JillE uses `jillesave` instead of `jn1save`).
    const std::string prefix =
        !game_.save_prefix.empty()
            ? game_.save_prefix
            : (profile_ && profile_->save_prefix ? profile_->save_prefix : "");
    if (prefix.empty()) {
        return std::string();
    }
    return prefix + "." + std::to_string(slot);
}

void Engine::refresh_save_labels() {
    for (int i = 0; i < 6; ++i) {
        save_labels_[static_cast<size_t>(i)].clear();
        if (i >= save_slots()) {
            continue;
        }
        const std::string path = save_path(i);
        if (path.empty()) {
            continue;
        }
        Board b;
        std::string err;
        if (!b.load(path, err)) {
            continue;
        }
        const uint32_t score = static_cast<uint32_t>(b.header[0x26]) |
                               (static_cast<uint32_t>(b.header[0x27]) << 8) |
                               (static_cast<uint32_t>(b.header[0x28]) << 16) |
                               (static_cast<uint32_t>(b.header[0x29]) << 24);
        char buf[48];
        std::snprintf(buf, sizeof(buf), "LV %d  %lu", b.level_id(),
                      static_cast<unsigned long>(score));
        save_labels_[static_cast<size_t>(i)] = buf;
    }
}

const std::string& Engine::save_label(int slot) const {
    static const std::string empty;
    if (slot < 0 || slot >= 6) {
        return empty;
    }
    return save_labels_[static_cast<size_t>(slot)];
}

bool Engine::save_game(int slot, std::string& err) {
    const std::string path = save_path(slot);
    if (path.empty()) {
        err = "this game profile has no save prefix";
        return false;
    }
    if (slot < 0 || slot >= save_slots()) {
        err = "save slot out of range";
        return false;
    }
    Board b;
    b.map = map_;
    b.header = header_;
    b.objects = objects_;
    if (!b.save(path, err)) {
        return false;
    }
    refresh_save_labels();
    put_bottom_msg("GAME SAVED");
    return true;
}

bool Engine::load_game(int slot, std::string& err) {
    const std::string path = save_path(slot);
    if (path.empty()) {
        err = "this game profile has no save prefix";
        return false;
    }
    Board b;
    if (!b.load(path, err)) {
        return false;
    }
    if (b.objects.empty()) {
        err = path + ": save has no objects";
        return false;
    }
    board_ = b;
    map_ = b.map;
    for (uint16_t& cell : map_) {
        cell &= 0x3FFF;
    }
    header_ = b.header;
    objects_ = b.objects;
    // A save does not record the board name; the hub is the only board whose
    // level id is 0 in the episodes that place MAPDEMO (Onesimus).
    map_mode_ = (header_[0] | (header_[1] << 8)) == 0;
    hub_valid_ = false;
    hub_level_ = map_mode_ ? 0 : level_id();
    if (map_mode_) {
        hub_name_ = game_.map_board;
    }
    has_player_ = objects_[0].kind == 0 || objects_[0].kind == 23;
    for (BoardObject& o : objects_) {
        o.prev_x = o.x;
        o.prev_y = o.y;
    }
    objects_.reserve(std::max<size_t>(
        static_cast<size_t>(kObjectLimit), objects_.size()));
    active_.clear();
    order_.clear();
    frame_ = 0;
    pending_warp_.clear();
    caption_.clear();
    caption_ttl_ = 0;
    caption_hold_ = 0;
    caption_shown_ = 0;
    msg_text_.clear();
    msg_ttl_ = 0;
    page_open_ = false;
    spawn_x_ = has_player_ ? objects_[0].x : 0;
    spawn_y_ = has_player_ ? objects_[0].y : 0;
    cam_x_ = std::max(0, std::min(spawn_x_ + 8 - view_center_x(),
                                  Board::kCols * 16 - view_w()));
    cam_y_ = std::max(0, std::min(spawn_y_ + 16 - view_center_y(),
                                  Board::kRows * 16 - view_h()));
    if (!has_player_) {
        cam_x_ = 0;
        cam_y_ = 0;
    }
    prev_cam_x_ = cam_x_;
    prev_cam_y_ = cam_y_;
    put_bottom_msg("GAME RESTORED");
    return true;
}

uint32_t Engine::score() const {
    return static_cast<uint32_t>(header_[0x26]) |
           (static_cast<uint32_t>(header_[0x27]) << 8) |
           (static_cast<uint32_t>(header_[0x28]) << 16) |
           (static_cast<uint32_t>(header_[0x29]) << 24);
}

std::string Engine::status() const {
    char buf[200];
    std::snprintf(buf, sizeof(buf), "%s %s  lvl=%d  hp=%d  inv=%d  objs=%d  cam=%d,%d%s",
                  game_.id.empty() ? profile_->id : game_.id.c_str(),
                  board_name_.c_str(), level_id(), health(),
                  header_[4] | (header_[5] << 8),
                  static_cast<int>(objects_.size()), cam_x_, cam_y_,
                  invincible_ ? "  INV" : "");
    return buf;
}

void Engine::sound(int priority, int id) {
    // Bounded: a busy collision tick must not grow the queue without limit.
    if (sound_queue_.size() >= 64) {
        return;
    }
    sound_queue_.push_back(static_cast<uint16_t>(
        ((priority & 0xFF) << 8) | (id & 0xFF)));
}

std::vector<uint16_t> Engine::take_sound_requests() {
    std::vector<uint16_t> out;
    out.swap(sound_queue_);
    return out;
}
