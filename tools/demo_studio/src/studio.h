#pragma once
#include <SDL.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "engine.h"
#include "framebuffer.h"
#include "game.h"
#include "macro.h"
#include "ui.h"

// Command-line / startup configuration for the demo studio.
struct StudioOptions {
    std::string game;                 // --game ID
    int episode = 0;                  // --episode N
    std::string games_dir = "games";
    std::string profiles_dir = "profiles";
    std::string external_dir = "external";
    std::string pack;                 // --pack ID
    std::string data_dir;             // --data DIR
    std::string palette;              // --palette FILE
    std::string mac;                   // --mac FILE
    std::string new_macro;             // --new NAME: create a fresh .mac
    std::string board;                // --board NAME
    int level = -1;                   // --level N
    bool hud_classic = false;         // --hud
    bool hud_portrait = true;
    // The rest mirrors jill.ini so the preview matches the player's game.
    int hud_bg = -2;                  // -2 suit, -3 vibrant, else palette index
    int hud_text = 7;
    int hud_health = -1;
    int hud_font_scale = 100;
    int hud_frame = 0;                // 0 = the running game, N = games[N-1]
    std::string hud_panel;
    std::string hud_layout;
    std::string menu_font;
    bool fast_dialogs = false;
    bool animated_water = true;
    bool retain_items = false;
    bool air_jump_buffer = false;
    bool fire_crouched = false;
    int max_knives = 3;
    bool center_fish_shot = false;
    bool minimap = false;
    bool turtle = false;
    int respawn_mode = 0;             // [gameplay] 0 checkpoint, 1 level start, 2 none
    bool phoenix_water_death = true;  // [gameplay] phoenix dies on waterfalls
    bool throw_from_vines = false;    // [gameplay] throw while climbing
    int interpolation = 0;            // 0 smooth, 1 no smoothing, 2 original 18.2 Hz
    int window_w = 1360;
    int window_h = 780;
    bool headless = false;            // --headless: no window, for smoke tests
    int ticks = 0;                    // --ticks N: headless advance before the dump
    std::string out;                  // --out: frame PPM / selftest .mac
    std::string trace;                // --trace FILE: headless per-tick CSV
    std::string dump_map;             // --dump-map FILE: headless live tile map
    std::string dump_objects;         // --dump-objects FILE: headless object table
    bool classic_camera = false;      // --classic-camera: original dead-zone scroll
    std::string screenshot_dir;       // --screenshot-dir DIR (F9 writes here)
};

// External playback viewer/editor for `*.mac` recordings.  The tool owns an
// Engine, renders the real game view, and shows the recording as an editable
// timeline.  Nothing here is linked into the shipped game.
class Studio {
public:
    ~Studio();
    bool init(const StudioOptions& opt, std::string& err);
    // GUI loop.  Returns the process exit code.
    int run();
    // Headless smoke test: advance `opt.ticks` and write the frame as PPM.
    int run_headless(std::string& err);
    // Headless parser/serialiser round-trip on the loaded file.
    int run_selftest(std::string& err);

private:
    struct Layout;

    enum class Field { Tick, H, V, Btn1, Btn2, Key, Mask, Count };
    enum class Overlay { None, Setup, Files, Help, SaveAs, Options, NewMac };

    struct Snapshot {
        std::vector<MacroRecord> records;
        int selected = 0;
    };

    struct FileGeometry {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        int title_h = 0;
        int row_h = 0;
        int list_y = 0;
        int visible = 1;
    };
    FileGeometry file_geometry(const Layout& l) const;

    // --- setup ---
    bool load_games(std::string& err);
    bool select_game(int index, std::string& err);
    bool open_macro(const std::string& path, bool apply_preset, std::string& err);
    bool start_run(int target_tick, std::string& err);
    bool rebuild_to(int target_tick, std::string& err);
    void apply_preset_for(const std::string& path);
    std::string game_data_dir() const;
    int playhead() const;

    // --- playback ---
    void tick_once();
    void toggle_play();
    void step_ticks(int delta);
    void seek(int target_tick);
    void set_interpolation_mode(int mode);
    const char* interpolation_label() const;

    // --- editing ---
    void push_undo();
    void undo();
    void sort_records(int keep_index);
    void insert_record_at_playhead();
    void delete_selected();
    void begin_edit();
    void commit_edit();
    void cancel_edit();
    void nudge_field(int dir);
    void toggle_mask_bit(int bit);
    void clear_mask_bit(int bit);
    void clear_mask_bits();
    // Mouse editing of one table cell: TICK opens the inline number editor,
    // H/V/B1/B2/KEY toggle or cycle in place (`shift` cycles backwards).
    void click_field(int index, int field, bool shift);
    int next_key_value(int from, int dir) const;
    void select_row(int index);
    void ensure_visible();
    bool save_macro(bool save_as, std::string& err);
    void refresh_file_list();
    void begin_new_macro();
    bool create_new_macro(std::string& err);
    static MacroRecord blank_record();

    // --- status ---
    void say(const std::string& text, uint32_t rgb = 0xFFFFFF, double seconds = 3.0);
    void toggle_help();

    // --- options ------------------------------------------------------------
    void set_text_scale(int scale);
    void set_game_scale(int scale);
    void ensure_window_fits();
    int body_scale() const { return text_scale_; }
    int hint_scale() const { return text_scale_ > 1 ? text_scale_ - 1 : 1; }
    void load_settings();
    void save_settings() const;
    std::string settings_path() const;

    // --- drawing ---
    struct Layout;
    Layout layout() const;
    void draw();
    void draw_header(const Layout& l);
    void draw_game(const Layout& l);
    void draw_info(const Layout& l);
    void draw_panel(const Layout& l);
    void draw_track(const Layout& l);
    void draw_rows(const Layout& l);
    void draw_editor(const Layout& l);
    void draw_footer(const Layout& l);
    void draw_setup(const Layout& l);
    void draw_files(const Layout& l);
    void draw_help(const Layout& l);
    void draw_save_as(const Layout& l);
    void draw_new_macro(const Layout& l);
    void draw_options(const Layout& l);
    void draw_modal_frame(const Layout& l, int x, int y, int w, int h,
                          const std::string& title);
    void present_game();
    bool save_screenshot(std::string& err);

    // --- input ---
    void handle_event(const SDL_Event& ev, const Layout& l);
    bool handle_key(int key, bool ctrl, bool shift, const Layout& l);
    void handle_click(int x, int y, bool shift, const Layout& l);
    void handle_right_click(int x, int y, bool shift, const Layout& l);
    void handle_wheel(int dy, const Layout& l);

    StudioOptions opt_;
    std::vector<GameDef> games_;
    int game_index_ = 0;
    std::vector<std::string> board_choices_;  // manifest boards + files on disk
    std::unique_ptr<Engine> engine_;
    uint8_t palette_[768] = {};

    std::vector<MacroRecord> records_;
    std::string macro_path_;
    std::string board_;
    int level_ = -1;

    int selected_ = 0;
    int field_ = 0;
    int scroll_ = 0;
    bool dirty_ = false;
    bool run_dirty_ = false;  // records changed after the current run started
    bool playing_ = false;
    bool finished_ = false;
    double accumulator_ = 0.0;
    int interp_mode_ = 0;     // 0 smooth, 1 no smoothing, 2 original cadence
    int text_scale_ = 2;      // UI/timeline text scale (1..3)
    int game_scale_ = 0;      // 0 = auto, else forced integer game-view scale
    int options_sel_ = 0;

    bool editing_ = false;
    std::string edit_buf_;
    std::vector<Snapshot> undo_;

    Overlay overlay_ = Overlay::None;
    int overlay_sel_ = 0;
    std::vector<std::string> file_list_;
    std::string save_name_;
    bool save_name_editing_ = false;
    bool quit_armed_ = false;
    std::string pending_open_;  // second ENTER discards unsaved edits

    std::string status_;
    uint32_t status_rgb_ = 0xFFFFFF;
    double status_until_ = 0.0;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* frame_tex_ = nullptr;
    std::unique_ptr<demo_ui::Ui> ui_;
    Framebuffer fb_;
    bool running_ = true;
    bool headless_ = false;
    double now_ = 0.0;
    int shot_counter_ = 0;
};
