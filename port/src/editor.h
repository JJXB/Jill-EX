#pragma once
#include <array>
#include <string>
#include <vector>

#include "board.h"
#include "textkey.h"

class Engine;
struct Framebuffer;

// In-engine level editor ("JILL MAKER").  It edits a Board snapshot, pushes it
// into the live Engine for preview/playtest and writes it back in the normal
// on-disk board format.
class LevelEditor {
public:
    enum class Mode { Tile, Object, Select, Region };

    enum class Key {
        None,
        Escape,
        Up,
        Down,
        Left,
        Right,
        ModeTile,
        ModeObject,
        ModeSelect,
        ModeRegion,
        TilePalette,
        ObjectPalette,
        Help,
        Grid,
        Save,
        Playtest,
        LoadSaved,
        Info,
        PageUp,
        PageDown,
        Confirm,
        Erase,
        Pick,
        ModeNext,
        ModePrev,
        Palette,
        SaveAs,
        LoadPick,
        PaintMode,
        Undo,
        Redo,
        Copy,
        Paste,
        ObjectProps,
        Presets,
        Menu,
        Minimap
    };

    // Load board data from the already-initialised engine.  Returns false when
    // the board cannot be edited (for example an empty object list).
    bool open(Engine& engine, const std::string& board_name, std::string& err);
    // Open the editor on a board file from an explicit path (a JILL MAKER
    // save under editor/<game>/).  The engine must already be on that game.
    bool open_path(Engine& engine, const std::string& path,
                   const std::string& board_name, std::string& err);
    // Replace the open board with a fresh blank level (floor, player, header)
    // and pick a new unique save name under editor/<game>/.
    bool new_level(Engine& engine, std::string& err);
    void close();

    bool active() const { return open_; }
    bool playtest() const { return playtest_; }
    // Start a playtest.  `start_playtest()` uses the board's player object
    // position; `start_playtest_at_cursor()` temporarily moves the player to
    // the cursor cell (the edit itself is restored on return).
    void start_playtest(Engine& engine);
    void start_playtest_at_cursor(Engine& engine);
    void stop_playtest(Engine& engine);

    // Returns true when the editor closed itself (Escape in edit mode) so the
    // app can return to the front-end menu.
    bool handle_key(Key key, Engine& engine);
    // Mouse position in 320x200 frame coordinates.
    void set_mouse(int x, int y, Engine& engine);
    void mouse_button(int button, bool down, int x, int y, bool shift,
                      bool ctrl, Engine& engine);
    // Drop any button/drag state (window lost focus or the pointer was released
    // outside the frame).
    void release_mouse();

    // Touch UI for Vita/Android: an on-screen toolbar supplies mode, palette,
    // pick, erase, save, load and playtest without L2/L3/R2/R3.
    void set_touch_controls(bool on) { touch_controls_ = on; }
    bool touch_controls() const { return touch_controls_; }
    bool touch_down(int x, int y, Engine& engine);
    void touch_move(int x, int y, Engine& engine);
    void touch_up(Engine& engine);

    // Native text prompt (object strings).  The app enables the platform text
    // input while text_input_active() is true.
    bool text_input_active() const { return text_input_active_; }
    void text_input_text(const char* utf8);
    void text_input_backspace();
    void text_input_commit(Engine& engine);
    void text_input_cancel();

    void wheel(int dy, bool shift);
    void pan(int dx, int dy);
    // Keyboard/gamepad navigation.  Moves the editor cursor, or the active
    // palette/header selection when a panel is open.
    void navigate(int dx, int dy, Engine& engine);
    bool overlay_open() const {
        return show_tiles_ || show_objects_ || show_help_ || show_info_ ||
               show_files_ || show_menu_ || show_props_ || show_presets_ ||
               show_game_confirm_ || text_input_active_;
    }

    // `alpha` is only used by the playtest view (board interpolation).
    void render(Framebuffer& fb, Engine& engine, float alpha);

private:
    static constexpr int kBoardCols = Board::kCols;
    static constexpr int kBoardRows = Board::kRows;
    static constexpr int kTileCount = 600;

    void apply(Engine& engine);
    void set_status(std::string text);
    std::string save_path() const;

    int tile_at(int col, int row) const;
    int object_at(int wx, int wy) const;
    void move_cursor(int dx, int dy);
    // Clamp the cursor cell into the camera rect after a camera pan, so the
    // highlight never scrolls out of view (fast pan / stick scroll).
    void keep_cursor_in_view();
    void move_selected(int dx, int dy, Engine& engine);
    void toggle_paint_mode();
    void primary_tile_action(Engine& engine);
    void paint_line(Engine& engine, int c0, int r0, int c1, int r1,
                    uint16_t id);
    void paint_rect(Engine& engine, int c0, int r0, int c1, int r1,
                    uint16_t id);
    void flood_fill(Engine& engine, int c0, int r0, uint16_t id);
    void push_undo();
    void push_undo_coalesced(uint64_t token);
    void undo(Engine& engine);
    void redo(Engine& engine);
    void clear_history();
    void copy_object(Engine& engine);
    void paste_object(Engine& engine);
    void copy_tiles();
    void paste_tiles(Engine& engine);
    void clear_region();
    int region_left() const;
    int region_top() const;
    int region_width() const;
    int region_height() const;
    void primary_action(Engine& engine);
    void erase_action(Engine& engine);
    void pick_action(Engine& engine);
    void paint_brush(Engine& engine, bool erase);
    void place_object(Engine& engine);
    void delete_object_at(int index, Engine& engine);
    void drag_selected(Engine& engine);
    void pick_tile(Engine& engine);
    void pick_object_kind(Engine& engine);
    bool save(Engine& engine);
    void load_saved(Engine& engine);
    void adjust_info(int delta, Engine& engine);
    void page_palettes(int delta);

    void open_object_props(Engine& engine);
    void adjust_object_field(int delta, Engine& engine);
    void activate_prop_field(Engine& engine);
    void link_trigger(Engine& engine);
    int linked_trigger_count(const BoardObject& o) const;
    void draw_object_props(Framebuffer& fb, Engine& engine);
    void open_presets();
    void choose_preset(int index, Engine& engine);
    void draw_presets(Framebuffer& fb, Engine& engine);
    void place_preset(Engine& engine);
    void start_text_prompt(const std::string& title,
                           const std::string& initial);
    void draw_text_prompt(Framebuffer& fb, Engine& engine);
    void save_to_game_data(Engine& engine);
    void draw_game_save_confirm(Framebuffer& fb, Engine& engine);
    void scroll_help(int dy);

    enum class FileMode { Save, Load };
    struct FileRow {
        std::string label;
        std::string name;
        bool original = false;
        bool new_file = false;
    };
    void open_file_picker(FileMode mode, Engine& engine);
    void refresh_file_rows(Engine& engine);
    void choose_file(Engine& engine);
    void draw_file_picker(Framebuffer& fb, Engine& engine);
    void draw_minimap(Framebuffer& fb, Engine& engine);
    void rebuild_minimap(Engine& engine);
    uint8_t minimap_tile_colour(Engine& engine, int tile_id);
    int file_list_start() const;
    std::string editor_dir() const;
    std::string editor_path(const std::string& name) const;
    std::string original_path(const Engine& engine) const;
    std::string unique_level_name(Engine& engine) const;
    bool load_board_path(const std::string& path, Engine& engine,
                         std::string& err);

    void draw_bars(Framebuffer& fb, Engine& engine);
    void draw_board_overlays(Framebuffer& fb, Engine& engine);
    void draw_tile_palette(Framebuffer& fb, Engine& engine);
    void draw_kind_palette(Framebuffer& fb, Engine& engine);
    void draw_info(Framebuffer& fb, Engine& engine);
    void draw_help(Framebuffer& fb, Engine& engine);
    void draw_editor_menu(Framebuffer& fb, Engine& engine);
    bool activate_menu_row(int row, Engine& engine);
    void draw_touch_bar(Framebuffer& fb, Engine& engine);

    bool open_ = false;
    bool playtest_ = false;
    bool dirty_ = false;
    Board board_;
    std::string board_name_;
    std::string game_id_;

    enum class PaintMode { Single, Line, Rect, Fill };
    enum class Preset { None, GemDoor, KeyDoor };
    Mode mode_ = Mode::Tile;
    PaintMode paint_mode_ = PaintMode::Single;
    Preset preset_ = Preset::None;
    bool anchor_valid_ = false;
    int anchor_col_ = 0;
    int anchor_row_ = 0;
    int paint_tile_ = 0;
    int erase_tile_ = 0;
    int brush_kind_ = 1;

    int cam_x_ = 0;
    int cam_y_ = 0;
    int mouse_x_ = 160;
    int mouse_y_ = 100;
    // Cell cursor: mouse updates it from the pointer; arrows/gamepad move it.
    int cursor_col_ = 0;
    int cursor_row_ = 0;
    int cursor_wx_ = 0;
    int cursor_wy_ = 0;

    bool left_down_ = false;
    bool right_down_ = false;
    bool dragging_ = false;
    int drag_obj_ = -1;
    int drag_last_wx_ = 0;
    int drag_last_wy_ = 0;
    int selected_obj_ = -1;

    bool last_paint_valid_ = false;
    int last_paint_col_ = 0;
    int last_paint_row_ = 0;

    // Tile-region selection and clipboard (Region mode).
    bool region_valid_ = false;
    int region_a_col_ = 0;
    int region_a_row_ = 0;
    int region_b_col_ = 0;
    int region_b_row_ = 0;
    std::vector<uint16_t> tile_clip_;
    int tile_clip_w_ = 0;
    int tile_clip_h_ = 0;
    // Consecutive adjustments to the same field share one undo entry.
    uint64_t undo_coalesce_ = 0;

    bool show_grid_ = false;
    bool show_minimap_ = true;
    bool mini_dirty_ = true;
    std::array<uint8_t, Board::kCols * Board::kRows> mini_px_{};
    std::array<uint8_t, 600> tile_colour_{};
    std::array<bool, 600> tile_colour_ready_{};
    bool show_tiles_ = false;
    bool show_objects_ = false;
    bool show_help_ = false;
    bool show_info_ = false;
    bool show_files_ = false;
    bool show_menu_ = false;
    bool show_props_ = false;
    bool show_game_confirm_ = false;
    bool show_presets_ = false;
    int menu_sel_ = 0;
    int prop_sel_ = 0;
    int prop_obj_ = -1;
    int preset_sel_ = 0;
    int help_scroll_ = 0;
    bool has_clipboard_ = false;
    BoardObject clipboard_;
    std::vector<Board> undo_;
    std::vector<Board> redo_;
    static constexpr int kUndoLimit = 64;
    bool text_input_active_ = false;
    std::string text_input_buffer_;
    std::string text_input_title_;
    TextKeyboard keyboard_;
    FileMode file_mode_ = FileMode::Save;
    int file_sel_ = 0;
    std::vector<FileRow> file_rows_;
    std::string save_name_;
    bool touch_controls_ = false;
    bool touch_erase_ = false;
    bool touch_canvas_ = false;
    bool touch_swallow_ = false;  // toolbar/overlay touch: do not paint on up
    bool touch_pan_ = false;      // PAN tool: canvas drag moves the camera
    bool touch_panning_ = false;
    int touch_pan_x_ = 0;
    int touch_pan_y_ = 0;
    int touch_button_ = 1;
    int touch_x_ = 0;
    int touch_y_ = 0;
    int tile_page_ = 0;
    int kind_page_ = 0;
    int kind_count_ = 0;
    int info_sel_ = 0;

    std::string status_;
};
