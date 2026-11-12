#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "board.h"
#include "external.h"
#include "framebuffer.h"
#include "game.h"
#include "profile.h"
#include "shapes.h"
#include "textpages.h"
#include "tiles.h"

struct InputState {
    int h = 0;       // -1 / 0 / +1, like DGROUP input+0
    int v = 0;       // -1 / 0 / +1
    bool btn1 = false;  // edge, after latch
    bool btn2 = false;
};

// The game state and the per-frame update pass.  Data layout and order follow
// docs/jill1-player-physics.md; every episode value comes from ProfileDef.
class Engine {
public:
    // `game` supplies the asset set; its `profile` supplies the compiled
    // engine data (kinds, handler tables, anchors, item icons).
    bool init(const GameDef& game, std::string& err);
    bool load_board(const std::string& name, std::string& err);
    // Save/restore (original hotkeys S/R and the front-end menu).  A save is
    // the live board written in the on-disk board layout, exactly like the
    // original JN1SAVE.n files.
    bool save_game(int slot, std::string& err);
    bool load_game(int slot, std::string& err);
    // Ending marker: a state-3 CHECKPT ends the sequence and asks the front end
    // to return to the main menu.
    bool take_ending() {
        const bool v = ending_done_;
        ending_done_ = false;
        return v;
    }
    // Replay a recorded `*.mac` input script (the `&end*.mac` ending macros).
    void start_macro(const std::string& name);
    int save_slots() const {
        const int slots = game_.save_slots > 0
                              ? game_.save_slots
                              : (profile_ && profile_->save_slots > 0
                                     ? profile_->save_slots
                                     : 6);
        return slots > 6 ? 6 : slots;
    }
    // Empty when the slot has no file yet.
    const std::string& save_label(int slot) const;

    void set_input(int h, int v, bool btn1_down, bool btn2_down);
    void tick();    // with_hud = false draws only the board (used for the menu theme
    // backdrops, which must not include a HUD bar).  `alpha` blends the
    // previous logic tick into the current one for high-framerate modes.
    void render(Framebuffer& fb, bool with_hud = true, float alpha = 1.0f);
    // Stereoscopic eye separation.  Only the board layers move: the backdrop
    // stays fixed and tiles move half as far as objects, so the two views have
    // a simple depth order.  HUD/UI are unaffected.
    void set_stereo_shift(int pixels) { stereo_shift_ = pixels; }
    int stereo_shift() const { return stereo_shift_; }
    // Build the draw list for an arbitrary camera and render once.  Used by the
    // menu/splash backdrops, which are not tied to the player's position.
    void render_at(Framebuffer& fb, int x, int y, bool with_hud = false);
    // Ending slides always use the full 320x200 board view, even when the
    // in-game HUD is CLASSIC (which otherwise offsets/clips the viewport).
    void render_at_fullscreen(Framebuffer& fb, int x, int y) {
        const bool saved = hud_classic_;
        hud_classic_ = false;
        render_at(fb, x, y, false);
        hud_classic_ = saved;
    }
    // Menu backdrops can use a board as artwork only: hide the TEXT6/TEXT8
    // objects so their captions do not end up baked into the backdrop.
    void set_hide_text_kinds(bool on) { hide_text_kinds_ = on; }
    // External pack overrides (sprite frames, tile art, board backgrounds,
    // extra menu themes).  Null/shared-empty disables them.
    void set_external_visuals(std::shared_ptr<const ExternalVisuals> visuals) {
        ext_visuals_ = std::move(visuals);
    }
    const ExternalVisuals* external_visuals() const {
        return ext_visuals_.get();
    }
    // Carry packed art/palette over when swapping to a fresh Engine for a
    // different episode.
    void adopt_external_state(const Engine& other) {
        ext_visuals_ = other.ext_visuals_;
        for (int i = 0; i < 768; ++i) {
            ext_palette_[i] = other.ext_palette_[i];
        }
    }
    // Palette used to quantise external art (0..255 per channel, 768 bytes).
    void set_external_palette(const uint8_t rgb[768]);
    // Rebuild `ext_visuals_` for a pack.  Returns false when nothing applied.
    bool apply_external(const ExternalAssets& assets, const uint8_t rgb[768]);
    // Classic presentation can borrow another installed game's shape bank, so
    // Jill 1's HUD frame or another episode's shape-7 menu window can be used
    // over the running game's art.  Null (or the current game) restores the
    // game's own bank.
    bool set_hud_frame_game(const GameDef* game, std::string& err);
    // Classic menu backdrop (shape 3) and menu window (shape 7) are separate:
    // THEME selects the backdrop's source independently of BORDER.
    bool set_classic_frame_game(const GameDef* game, std::string& err);
    bool set_menu_frame_game(const GameDef* game, std::string& err);
    // Draw the original screen frame/title strip.  Reused by the CLASSIC HUD
    // and the CLASSIC menu style.  `menu` selects the menu frame source.
    void draw_classic_backdrop(Framebuffer& fb, const std::string& title,
                               bool menu = false);
    // Draw the original window border from shape entry 7 (8x8 nine-slice).
    void draw_menu_window(Framebuffer& fb, int x, int y, int w, int h);
    // The original HUD (pips, HEALTH/SCORE/LEVEL text, key icons) is optional:
    // the port keeps its minimal pip strip unless this is enabled.
    void set_hud_classic(bool on) { hud_classic_ = on; }
    bool hud_classic() const { return hud_classic_; }
    // Selected external HUD panel (empty = the pack's classic/modern default).
    void set_hud_panel(const std::string& name) { hud_panel_ = name; }
    const std::string& hud_panel() const { return hud_panel_; }
    // Selected external menu border (index into ExternalVisuals::borders()).
    void set_menu_border_ext(int index) { menu_border_ext_ = index; }
    int menu_border_ext() const { return menu_border_ext_; }
    // Selected external font set (empty = the game's own font).
    void set_font_name(const std::string& name) { font_name_ = name; }
    const std::string& font_name() const { return font_name_; }
    // Original Jill 3 TURTLE option: run the game logic at half speed.  The
    // first tick after enabling runs a normal update so interpolation starts
    // on a fresh prev/current pair.
    void set_turtle(bool on) {
        if (on && !turtle_) {
            turtle_skip_ = true;
            slow_phase_ = 0;
        }
        turtle_ = on;
    }
    bool turtle() const { return turtle_; }
    // Draw the character portrait beside level-title dialogs (HUD option).
    void set_hud_portrait(bool on) { hud_portrait_ = on; }
    bool hud_portrait() const { return hud_portrait_; }
    // Debug fly: arrow keys reposition the player directly (no collision or
    // gravity) so level elements can be tested quickly.
    void set_debug_fly(bool on) { debug_fly_ = on; }
    bool debug_fly() const { return debug_fly_; }
    // Debug invincibility: player_kill becomes a no-op (F5 toggles, --invincible
    // starts with it on).  Nothing else in the update path changes.
    void set_invincible(bool on) { invincible_ = on; }
    bool invincible() const { return invincible_; }
    // Optional gameplay enhancements (main menu > Options > Enhancements).
    // The original cannot fire while crouched because crouching clears the
    // facing field; this keeps the last facing direction instead.
    void set_fire_while_crouched(bool on) { fire_while_crouched_ = on; }
    bool fire_while_crouched() const { return fire_while_crouched_; }
    // Original behaviour is to lose keys/weapons when a level is exited; this
    // enhancement keeps the inventory across board warps instead.
    void set_retain_items(bool on) { retain_items_ = on; }
    bool retain_items() const { return retain_items_; }
    // Enhancement: the original caps knife pickups at 3; allowing 5 is optional.
    void set_max_knives(int n) { max_knives_ = (n >= 5) ? 5 : 3; }
    int max_knives() const { return max_knives_; }
    // Enhancement: default off means a jump pressed while airborne is consumed
    // (holding jump does not auto-jump on landing, i.e. no air buffer).  On
    // keeps the original hold-to-bounce behaviour.
    void set_air_jump_buffer(bool on) { air_jump_buffer_ = on; }
    bool air_jump_buffer() const { return air_jump_buffer_; }
    // Enhancement: small 1-px-per-tile board overview in the lower-right corner.
    void set_minimap(bool on) { minimap_ = on; }
    bool minimap() const { return minimap_; }
    // Enhancement: centre the fish form's shot vertically on the player.
    void set_center_fish_shot(bool on) { center_fish_shot_ = on; }
    bool center_fish_shot() const { return center_fish_shot_; }
    // Jill Select: draw normal Jill with another episode's player colour table.
    // look 0 = the running game, otherwise 1..N = games[N-1].  `suit` 0 keeps
    // the look's own colours; 1..8 are the custom presets.
    bool set_player_look(int look, const std::string& shape_path);
    void set_player_suit(int suit);
    int player_look() const { return player_look_; }
    int player_suit() const { return player_suit_; }
    // HUD background: a palette index, -1 = black, -2 = Jill's suit colour,
    // -3 = the vibrant/highlight shade of the suit.
    void set_hud_bg(int bg) { hud_bg_ = bg; }
    int hud_bg() const { return hud_bg_; }
    // HUD label/text colour (palette index, default 7 = white).
    void set_hud_text(int c) { hud_text_ = c & 0xFF; }
    int hud_text() const { return hud_text_; }
    // HUD health colour: -1 keeps the original pip art, 0..15 repaints the
    // health pips/flags in that palette index.
    void set_hud_health(int c) { hud_health_ = c; }
    int hud_health() const { return hud_health_; }
    // Resolve the configured background to a palette index (may derive the
    // per-episode suit shade from the player sprite on first use).
    int hud_bg_colour();
    int suit_vibrant_colour();
    // Cheat: treat the player as carrying at least N feathers (item 9, the
    // jump power-up), 0 = off, capped at 15.  The normal impulse formula is
    // used, so found feathers above N still increase the jump.
    void set_super_jump(int n) {
        super_jump_ = (n < 0) ? 0 : (n > 15 ? 15 : n);
    }
    int super_jump() const { return super_jump_; }
    // Cheat menu: refill the health bar to the apple maximum.
    void debug_heal() {
        header_[2] = 8;
        header_[3] = 0;
    }
    // Bottom-of-screen message line (the original's _putbotmsg, 22AB:05b4).
    // Default life is 0x50 ticks with the text shown immediately; the "faster
    // dialogs" enhancement halves that and reveals the text 4 chars per tick.
    void put_bottom_msg(const std::string& text, int colour = 7);
    const std::string& bottom_msg() const { return msg_text_; }
    int bottom_msg_ttl() const { return msg_ttl_; }
    // Level-title caption currently queued/drawn (empty when none).
    const std::string& caption() const { return caption_; }
    // "Faster dialog boxes": 4 characters per tick instead of 1 for the level
    // caption and board text pages (the bottom message keeps its original
    // 0x50-tick lifetime).
    void set_fast_dialogs(bool on) {
        fast_dialogs_ = on;
        dialogs_per_tick_ = on ? 4 : 1;
    }
    bool fast_dialogs() const { return fast_dialogs_; }
    // Message-box style (VIDEO menu option): 0 = off, 1 = modern frame,
    // 2 = classic shape-7 windows (portrait caption + titled card).
    void set_message_box(int style) {
        message_box_style_ = (style < 0) ? 0 : (style > 2 ? 2 : style);
    }
    int message_box() const { return message_box_style_; }
    bool message_box_classic() const { return message_box_style_ == 2; }
    // Classic "Reminder" card: a titled shape-7 window centred over the
    // playfield.  Used by the help prompt and the start-of-game reminder.
    void show_message_box(const std::string& title, const std::string& text,
                          int ticks = 0x78);
    void close_message_box();
    bool message_box_open() const { return box_ttl_ > 0; }
    // Skip any pending caption/reveal (used for the static menu backdrop).
    void finish_dialogs() {
        caption_ttl_ = 0;
        msg_ttl_ = 0;
        box_ttl_ = 0;
        text_reveal_ = 0x400;
    }
    // --- text page viewer --------------------------------------------------
    // The `.vcl` text section holds help/hint/news pages (Jill) and the
    // Onesimus verses.  Page apples (kind 1 with state > 0) and F1 open it;
    // the world is frozen while it is up.
    bool text_pages_available() const { return pages_.has_pages(); }
    bool text_page_open() const { return page_open_; }
    int text_page_number() const { return page_number_; }
    // 0..39 (the bank's page-table index); a missing/empty page is ignored.
    void show_text_page(int number);
    void close_text_page();
    void scroll_text_page(int delta);
    void cycle_text_page(int dir);   // previous/next non-empty page
    // Headless testing aid: put the player at a pixel position and make that
    // the respawn point (used by --start).
    void warp_player(int x, int y);
    // Debug aid: spawn one SPINBLAD in front of the player (--blade).
    void debug_spawn_blade();
    // Debug aid: run a transformation token immediately (--morph).
    void debug_morph(int item) { morph_player(item); }
    // Debug aid: force the player's facing/direction (--face -1/1).
    void debug_face(int dir) {
        if (!has_player_) {
            return;
        }
        objects_[0].p0f = dir;
        objects_[0].p1b = dir;
        objects_[0].dx = dir;
    }
    // Fixed camera, used by the menu theme backdrops (and headless probes).
    void set_camera(int x, int y) {
        cam_x_ = x;
        cam_y_ = y;
        prev_cam_x_ = x;
        prev_cam_y_ = y;
    }
    // Front-end theme: 0 = plain port style, N = the N-th game's intro board
    // as the menu backdrop (the app clamps N to the installed game count).
    void set_menu_theme(int t) { menu_theme_ = (t < 0) ? 0 : t; }
    int menu_theme() const { return menu_theme_; }
    // Theme helpers: the board's most common solid / walkable tile id, and that
    // tile's decoded pixels, used to build the tiled menu frame.
    int theme_tile(bool solid) const;
    bool tile_pixels(int tile_id, std::vector<uint8_t>& px, int& w, int& h);
    // Headless testing aid: override the board header level id (--level) so
    // exits (`!`) can be exercised without walking the whole hub graph.
    void set_level_id(int id) {
        header_[0] = static_cast<uint8_t>(id & 0xFF);
        header_[1] = static_cast<uint8_t>((id >> 8) & 0xFF);
    }
    const uint16_t* item_icons() const { return profile_->item_icons; }
    // Shape id for an item icon/pip frame in the game's HUD icon bank.
    int icon_shape(int frame) const {
        return (game_.hud_icon_bank << 8) | (frame & 0xFF);
    }
    void draw_text(Framebuffer& fb, int x, int y, uint8_t colour, const char* s);
    // 8-pixel font (bank 1), used by the start-up splash titles.
    void draw_text_8(Framebuffer& fb, int x, int y, uint8_t colour, const char* s);

    const GameDef& game() const { return game_; }
    const ProfileDef& profile() const { return *profile_; }
    const Board& board() const { return board_; }
    const std::vector<BoardObject>& objects() const { return objects_; }
    const BoardObject& player() const { return objects_.front(); }
    // Some shipped boards (Jill 3's `map.jn3` world map) have no player object.
    bool has_player() const { return has_player_; }
    const std::vector<int>& draw_order() const { return order_; }
    const std::string& board_name() const { return board_name_; }

    int cam_x() const { return cam_x_; }
    int cam_y() const { return cam_y_; }
    int health() const { return header_[2] | (header_[3] << 8); }
    int level_id() const { return header_[0] | (header_[1] << 8); }
    uint32_t score() const;
    int frame() const { return frame_; }
    std::string status() const;
    // Debug aid for tile-script work: the live record of one tile id.
    const TileInfo& tile(int id) const { return tiles_.at(id); }
    // Map cell (tile id only, without the 0xC000 redraw bits).
    int map_tile(int col, int row) const;

    // --- engine services (used by the player handler) ---------------------
    // The header's item list caps at 15 entries (see add_item), so that is also
    // the largest number of feathers (item 9) a jump can be scaled by.
    // The original array holds 0x100 records; total-conversion boards can
    // start with more (JillE uses up to 258), so the port's runtime vector
    // allows the same generous ceiling as Board::kMaxObjects.
    static constexpr int kObjectLimit = Board::kMaxObjects;
    // Visible/active list.  The original caps it at 0xC0; mods with dense
    // boards can need more, so allow up to the object-pool ceiling.
    static constexpr int kActiveLimit = 0x180;
    static constexpr int kMaxInventory = 15;
    int add_obj(int kind, int x, int y);
    uint16_t cando(int i, int x, int y, uint16_t mask) const;
    void set_origin(int i, int x, int y);
    bool try_move(int i, int x, int y);   // 26F4:1E70
    bool just_move(int i, int x, int y);  // 26F4:1D73
    int move_obj(int i, int x, int y);    // 26F4:1C89 _trymove (1/2/4/0)
    int crawl(int i, int dx, int dy);     // 26F4:1F35 _crawl
    bool fishdo(int i, int x, int y);     // 26F4:2605 _fishdo
    int inv_count(int id) const;
    bool del_item(int id);
    void add_item(int id);
    void player_kill(int amount, int dir);
    void respawn();
    // `_sound(priority, logical_id)`: the app drains the queue after a tick and
    // hands it to the sampled-effect voice.
    void sound(int priority, int id);
    std::vector<uint16_t> take_sound_requests();
    // Music requested by a CHECKPT string (`*name.ddt`).  The app polls this
    // after each update and hands the file to the CMF player.
    std::string take_music_request() {
        std::string s;
        s.swap(music_request_);
        return s;
    }
    void kill_obj(int i);                          // 26F4:1863 _killobj
    void add_score(int n, int x, int y);           // 26F4:1fb2 _addscore
    void explode1(int x, int y, int n);            // 1A41:09a8 (n x EXPL1)
    void explode2(int i);                          // 1A41:0930
    void hit_player(int i);                        // 26F4:25B8
    void killaward(int i);                         // 26F4:0091
    void stamp_footprint(int i);                   // 26F4:012D
    void set_obj_size(int i);                      // 26F4:020C
    int touch_bkgnd(int i, int x, int y);          // 26F4:27DD
    void send_trig(int trig, int code, int self);  // 26F4:20EF
    bool onscreen(int i) const;                    // 26F4:1DA8
    void pointvect(int i, int& dx, int& dy) const; // 26F4:06B4
    void vectdist(int a, int b, int& dx, int& dy, int speed) const;  // 26F4:263A
    int dist(int a, int b) const;                  // 26F4:2772
    int obj_w(int i) const { return objects_[static_cast<size_t>(i)].w; }
    int obj_h(int i) const { return objects_[static_cast<size_t>(i)].h; }

private:
    void tile_prepass();          // FUN_26f4_11d7
    void tile_touch(int i);       // FUN_26f4_23e1 (hazard tiles under an object)
    void tile_hook(int col, int row, int mode);  // FUN_1f28_0006
    void update_pass();           // FUN_26f4_12b9
    bool update_object(int i);
    bool trig_object(int i, int code, int self);  // action 3/4/5
    void player_update(int i);
    void tiny_update(int i);      // kind 23 TINY (map Jill)
    void form_update(int i);      // kinds 54-57 morphed Jill
    void morph_player(int item);  // 1FAE:2E61 transformation tokens
    void process_pending_warp();
    void place_at_checkpoint(int level);
    void apply_macro_input();
    void compact_objects();
    void draw_minimap(Framebuffer& fb);
    void update_caption_for_level(int level);
    std::string save_path(int slot) const;
    void refresh_save_labels();
    bool on_hit(int a, int b);
    void player_draw(int i, Framebuffer& fb);
    void form_draw(int i, Framebuffer& fb);
    void draw_object(int i, Framebuffer& fb);
    void draw_player_shape(Framebuffer& fb, int shape, int x, int y,
                           bool transparent);
    void build_player_remap();
    // Shape lookup: external pack override first, then the game's shape bank.
    const ShapeFrame* shape_frame(int entry, int index);
    // Classic frame lookup: selected game bank when set, else the normal path.
    const ShapeFrame* classic_hud_shape(int entry, int index);
    const ShapeFrame* classic_frame_shape(int entry, int index);
    const ShapeFrame* classic_menu_shape(int entry, int index);
    void draw_shape(Framebuffer& fb, int shape, int x, int y, bool transparent);
    // Draws the shape's set pixels in a single palette colour (health pips).
    void draw_shape_tint(Framebuffer& fb, int shape, int x, int y,
                         uint8_t colour);
    // Health pip: original art, or the selected HUD health colour.
    void draw_health_pip(Framebuffer& fb, int frame, int x, int y);
    // Horizontal mirror, used only when an episode lacks real right-facing
    // frames for a kind (Jill 2/3 ROMAN art is a 7-frame left-facing sheet).
    void draw_shape_mirrored(Framebuffer& fb, int shape, int x, int y,
                             bool transparent);
    // Masked blit used by the text/font shapes: any non-zero pixel becomes
    // `colour`, level 3 (anti-alias/serif) is skipped.
    void draw_shape_text(Framebuffer& fb, int shape, int x, int y,
                         uint8_t colour);
    // Text with a selectable font: bank 2 = 6 px cells (advance 6), bank 1 =
    // 8 px cells (advance 8).  Used by TEXT6 (20) / TEXT8 (21).
    void draw_text_font(Framebuffer& fb, int x, int y, uint8_t colour, int bank,
                        int advance, const char* s);
    void draw_bottom_msg(Framebuffer& fb);
    void draw_caption(Framebuffer& fb);           // level-title dialog box
    void draw_caption_classic(Framebuffer& fb,
                              const std::vector<std::string>& lines);
    void draw_portrait(Framebuffer& fb, int x, int y);
    void draw_message_box(Framebuffer& fb);       // classic Reminder card
    void draw_text_page(Framebuffer& fb);         // VCL page viewer
    void draw_frame(Framebuffer& fb, int x, int y, int w, int h);
    void render_hud_classic(Framebuffer& fb);  // original 64x84 panel
    void render_hud_modern(Framebuffer& fb);   // single top-of-screen bar
    int suit_colour();                         // cached per-episode suit shade
    void follow_camera();
    int view_x() const;
    int view_y() const;
    int view_w() const;
    int view_h() const;
    int view_center_x() const;
    int view_center_y() const;
    void clear_frame_marks();
    int rnd(int mod);

    const ProfileDef* profile_ = nullptr;
    GameDef game_;
    std::string data_dir_;
    std::string board_name_;
    TileDefs tiles_;
    ShapeBank shapes_;
    // Optional shape banks used only for the classic screen/menu frames.
    ShapeBank hud_frame_shapes_;
    ShapeBank classic_frame_shapes_;
    ShapeBank menu_frame_shapes_;
    bool hud_frame_custom_ = false;
    bool classic_frame_custom_ = false;
    bool menu_frame_custom_ = false;
    Board board_;
    std::array<uint16_t, Board::kCols * Board::kRows> map_{};
    std::array<uint8_t, Board::kHeaderSize> header_{};
    std::vector<BoardObject> objects_;
    std::vector<int> active_;
    std::vector<int> order_;
    int cam_x_ = 0;
    int cam_y_ = 0;
    int prev_cam_x_ = 0;  // presentation interpolation (previous tick)
    int prev_cam_y_ = 0;
    int frame_ = 0;
    int water_sound_ = -0x100;  // DGROUP:0x439E, last water-splash frame
    uint32_t rng_ = 0x2A3B1C4D;  // stands in for FUN_2ca6_0015
    bool hud_classic_ = false;
    bool hud_portrait_ = true;
    bool hide_text_kinds_ = false;
    bool debug_fly_ = false;
    bool invincible_ = false;
    bool fire_while_crouched_ = false;
    bool retain_items_ = false;
    int max_knives_ = 3;
    int hud_bg_ = -2;  // -2 = suit colour (default), -3 = vibrant suit
    int hud_text_ = 7; // HUD label colour (palette index)
    int hud_health_ = -1;  // -1 = original pip art, else palette index
    int suit_colour_ = -1;
    int suit_vibrant_ = -1;
    int super_jump_ = 0;
    int menu_theme_ = 0;
    bool air_jump_buffer_ = false;  // enhancement: hold jump through a fall
    bool minimap_ = false;          // enhancement: board overview overlay
    bool center_fish_shot_ = false; // enhancement: centre the fish shot
    // Jill Select: source and target player colour tables (16 x palette index).
    std::array<uint8_t, 16> player_base_xlate_{};
    std::array<uint8_t, 16> player_raw_xlate_{};
    std::array<uint8_t, 16> player_look_xlate_{};
    uint8_t player_remap_[256] = {};
    bool player_remap_active_ = false;
    int player_look_ = 0;
    int player_suit_ = 0;
    bool map_mode_ = false;      // hub/world-map board (MAPDEMO visibility)
    std::array<std::string, 6> save_labels_{};
    // Hub board kept in memory while a level is entered, so returning with `!`
    // restores the map screen instead of reloading it (original `temp` board).
    Board hub_backup_;
    bool hub_valid_ = false;
    int hub_level_ = 0;
    std::string hub_name_;
    int last_facing_ = 0;        // last non-zero player facing (enhancement aid)
    std::string pending_warp_;   // CHECKPT string, handled on the next tick
    std::string music_request_;  // `*name.ddt` from a CHECKPT, polled by the app
    std::string last_music_;     // last requested track (do not restart it)
    std::vector<uint16_t> sound_queue_;  // packed priority<<8 | logical id
    // Recorded ending macro (`&end1.mac`): one record per input change.  Time
    // is an absolute tick count from playback start (jill1-engine.md 13.1).
    struct MacroRec {
        int time = 0;      // tick when this record takes effect
        int mask = 0;      // which fields below are valid
        int h = 0;
        int v = 0;
        bool btn1 = false;
        bool btn2 = false;
    };
    std::vector<MacroRec> macro_;
    size_t macro_pos_ = 0;
    int macro_tick_ = 0;
    int macro_last_ = 0;
    bool macro_playing_ = false;
    bool macro_held1_ = false;
    bool macro_held2_ = false;
    bool ending_done_ = false;
    std::string msg_text_;       // _putbotmsg buffer (DGROUP:0xB662)
    int msg_ttl_ = 0;            // DGROUP:0xB6B4, 0x50 by default
    uint8_t msg_colour_ = 7;     // DGROUP:0xB6B2
    size_t msg_shown_ = 0;       // characters revealed so far
    bool fast_dialogs_ = false;  // enhancement (captions / text pages)
    int dialogs_per_tick_ = 1;
    int message_box_style_ = 1;  // 0 off, 1 modern frame, 2 classic windows
    std::string box_title_;      // classic titled message card
    std::string box_text_;
    int box_ttl_ = 0;
    int box_hold_ = 0;           // input cannot dismiss the card yet
    std::string caption_;        // level-title caption text
    int caption_ttl_ = 0;
    int caption_hold_ = 0;       // input cannot dismiss the caption yet
    size_t caption_shown_ = 0;
    size_t text_reveal_ = 0;     // characters revealed on TEXT6/TEXT8 pages
    TextPages pages_;            // .vcl text pages (page number = apple state)
    bool page_open_ = false;
    int page_number_ = 0;
    int page_scroll_ = 0;
    bool apple_hint_ = false;    // one message per level
    bool switch_hint_ = false;   // one message per level
    bool knight_hint_ = false;   // "feeble attempt" taunt once per level
    bool elev_hint_ = false;     // "Press UP/DOWN" once per level
    bool item_hint_[16] = {};    // TOKEN hints, once per item per level
    int spawn_x_ = 0;
    int spawn_y_ = 0;
    InputState in_;
    bool latch1_ = false;
    bool latch2_ = false;
    bool has_player_ = false;
    std::shared_ptr<const ExternalVisuals> ext_visuals_;
    uint8_t ext_palette_[768] = {};
    std::string hud_panel_;
    std::string font_name_;
    int menu_border_ext_ = -1;
    bool turtle_ = false;
    bool turtle_skip_ = false;
    int slow_phase_ = 0;  // 0 = first half of a 2-tick move, 1 = second half
    int stereo_shift_ = 0;
};
