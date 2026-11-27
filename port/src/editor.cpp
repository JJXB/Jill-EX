#include "editor.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "engine.h"
#include "framebuffer.h"
#include "platform_fs.h"
#include "platform_paths.h"
#include "profile.h"

namespace {

constexpr int kTopBar = 12;
constexpr int kBottomBar = 188;
constexpr int kTouchCols = 10;
constexpr int kTouchRows = 2;
constexpr int kTouchButtons = kTouchCols * kTouchRows;
constexpr int kTouchBarH = kTouchRows * 20;
constexpr int kTouchBarY = Framebuffer::H - kTouchBarH;
constexpr int kEditorMenuRows = 19;
constexpr int kPropFields = 15;
constexpr int kPresetRows = 2;
constexpr int kHelpVisible = 14;
constexpr int kInfoFields = 6;
constexpr int kTilePalCols = 10;
constexpr int kTilePalRows = 7;
constexpr int kTilePalPage = kTilePalCols * kTilePalRows;
constexpr int kKindPalCols = 6;
constexpr int kKindPalRows = 4;
constexpr int kKindPalPage = kKindPalCols * kKindPalRows;

int clamp_int(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

void outline(Framebuffer& fb, int x, int y, int w, int h, uint8_t colour) {
    if (w <= 0 || h <= 0) {
        return;
    }
    fb.fill_rect(x, y, w, 1, colour);
    fb.fill_rect(x, y + h - 1, w, 1, colour);
    fb.fill_rect(x, y, 1, h, colour);
    fb.fill_rect(x + w - 1, y, 1, h, colour);
}

uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

void wr16(uint8_t* p, int v) {
    const uint16_t u = static_cast<uint16_t>(v & 0xFFFF);
    p[0] = static_cast<uint8_t>(u & 0xFF);
    p[1] = static_cast<uint8_t>((u >> 8) & 0xFF);
}

uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

void wr32(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

const char* const kHelpLines[] = {
    "[KEYBOARD]",
    "1 TILE  2 OBJECT  3 SELECT  4 TILE REGION",
    "TAB/E OBJECTS   T TILES   M/F6 PAINT TOOL",
    "SPACE/ENTER ACTION   DEL ERASE   P PICK",
    "C/V OBJECT COPY/PASTE OR TILE REGION COPY/PASTE",
    "REGION: CLICK A THEN B   C COPY TILES   V PASTE",
    "O PROPS: KIND X Y DX DY W H STATE PARAMS STRING",
    "R PRESETS: GEM DOOR / KEY DOOR PLACEABLES",
    "I BOARD INFO   G GRID   N MINIMAP   Z UNDO   Y REDO",
    "ARROWS CURSOR   WASD PAN   WHEEL PAN",
    "ESC MENU: SAVE TO GAME DATA WRITES GAME FOLDER",
    "LMB PAINT/PLACE/SELECT   RMB ERASE/DELETE",
    "SHIFT+LMB PICK   SHIFT+RMB ERASE TILE",
    "F2 SAVE  F3 SAVE AS  F4 LOAD  F9 QUICK LOAD",
    "F5 PLAYTEST   H/F1 HELP   ESC/B MENU",
    "[CONTROLLER]",
    "D-PAD/LEFT STICK CURSOR OR PANEL",
    "RIGHT STICK PAN BOARD",
    "A ACTION/CONFIRM   X ERASE/DELETE",
    "Y PALETTE   LB/RB MODE",
    "L3 PICK   R3 TOOL   BACK PLAYTEST",
    "START MENU   B PROPS/CLOSE   Y BACKSPACE",
    "[TOUCH]",
    "TOUCH BAR ROW 1: MODE PAL PICK TOOL PAN SAVE LOAD PLAY INFO MENU",
    "TOUCH BAR ROW 2: UNDO REDO COPY PASTE PROPS PRESET GRID HELP MINI ERASE",
    "PAN DRAGS THE CAMERA; TAP PAN AGAIN TO GO BACK TO PAINTING",
    "TAP CANVAS TO PAINT; ERASE TOGGLES PAINT/ERASE",
    "LINE/RECT/FILL TOOLS: TAP A, THEN TAP B",
    "TAP A PALETTE TILE/OBJECT TO SELECT IT",
};
constexpr int kHelpLineCount =
    static_cast<int>(sizeof(kHelpLines) / sizeof(kHelpLines[0]));

std::string shortened(const std::string& s, size_t n) {
    if (s.size() <= n) {
        return s;
    }
    return s.substr(0, n);
}

std::string lower_ascii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return s;
}

bool ends_with_ci(const std::string& text, const std::string& suffix) {
    if (suffix.empty() || text.size() < suffix.size()) {
        return suffix.empty();
    }
    return lower_ascii(text.substr(text.size() - suffix.size())) ==
           lower_ascii(suffix);
}

}  // namespace

bool LevelEditor::open(Engine& engine, const std::string& board_name,
                       std::string& err) {
    board_ = engine.board();
    if (board_.objects.empty()) {
        err = "board has no objects";
        return false;
    }
    board_name_ = !engine.board_name().empty() ? engine.board_name()
                                               : board_name;
    save_name_ = board_name_;
    game_id_ = engine.game().id;
    if (!engine.apply_editor_board(board_)) {
        err = "could not apply the board to the engine";
        return false;
    }
    engine.finish_dialogs();
    engine.close_message_box();
    engine.close_text_page();

    paint_tile_ = engine.theme_tile(true);
    if (paint_tile_ < 0) {
        paint_tile_ = 0;
    }
    erase_tile_ = 0;
    brush_kind_ = board_.objects[0].kind == 0 ? 1 : 0;
    if (brush_kind_ <= 0) {
        brush_kind_ = 1;
    }
    kind_count_ = engine.profile().kind_count;
    if (kind_count_ < 1) {
        kind_count_ = 1;
    }
    tile_page_ = (paint_tile_ / kTilePalCols) * kTilePalCols;
    kind_page_ = (brush_kind_ / kKindPalPage) * kKindPalPage;

    if (engine.has_player()) {
        cam_x_ = board_.objects[0].x - Framebuffer::W / 2;
        cam_y_ = board_.objects[0].y - Framebuffer::H / 2;
    } else {
        cam_x_ = Board::kCols * 16 / 2 - Framebuffer::W / 2;
        cam_y_ = Board::kRows * 16 / 2 - Framebuffer::H / 2;
    }
    cam_x_ = clamp_int(cam_x_, 0, Board::kCols * 16 - Framebuffer::W);
    cam_y_ = clamp_int(cam_y_, 0, Board::kRows * 16 - Framebuffer::H);
    cursor_col_ = clamp_int((cam_x_ + Framebuffer::W / 2) >> 4, 0,
                            kBoardCols - 1);
    cursor_row_ = clamp_int((cam_y_ + Framebuffer::H / 2) >> 4, 0,
                            kBoardRows - 1);
    cursor_wx_ = cursor_col_ * 16 + 8;
    cursor_wy_ = cursor_row_ * 16 + 8;

    mode_ = Mode::Tile;
    show_grid_ = false;
    show_tiles_ = false;
    show_objects_ = false;
    show_help_ = false;
    show_info_ = false;
    show_files_ = false;
    show_menu_ = false;
    show_props_ = false;
    show_game_confirm_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    preset_sel_ = 0;
    text_input_active_ = false;
    help_scroll_ = 0;
    prop_obj_ = -1;
    prop_sel_ = 0;
    has_clipboard_ = false;
    tile_clip_.clear();
    tile_clip_w_ = 0;
    tile_clip_h_ = 0;
    region_valid_ = false;
    undo_coalesce_ = 0;
    clear_history();
    anchor_valid_ = false;
    paint_mode_ = PaintMode::Single;
    menu_sel_ = 0;
    file_sel_ = 0;
    file_rows_.clear();
    left_down_ = false;
    right_down_ = false;
    dragging_ = false;
    drag_obj_ = -1;
    selected_obj_ = -1;
    last_paint_valid_ = false;
    dirty_ = false;
    playtest_ = false;
    open_ = true;
    mini_dirty_ = true;
    tile_colour_ready_.fill(false);
    set_status("F2 SAVE F3 SAVE AS F4 LOAD F5 PLAY T TILES H HELP");
    return true;
}

bool LevelEditor::open_path(Engine& engine, const std::string& path,
                            const std::string& board_name, std::string& err) {
    if (!open(engine, board_name, err)) {
        return false;
    }
    if (!load_board_path(path, engine, err)) {
        close();
        return false;
    }
    const size_t slash = path.find_last_of("/\\");
    const std::string fname =
        slash == std::string::npos ? path : path.substr(slash + 1);
    board_name_ = fname;
    save_name_ = fname;
    return true;
}

bool LevelEditor::new_level(Engine& engine, std::string& err) {
    if (!open_) {
        err = "editor is not open";
        return false;
    }
    board_ = Board{};
    // A two-row floor makes the new board immediately playable; everything
    // else is left empty for the author to paint.
    const int floor = engine.theme_tile(true);
    const uint16_t floor_id =
        static_cast<uint16_t>((floor >= 0 ? floor : 0) & 0x3FFF);
    for (int c = 0; c < kBoardCols; ++c) {
        board_.map[static_cast<size_t>(c) * kBoardRows + (kBoardRows - 1)] =
            floor_id;
        board_.map[static_cast<size_t>(c) * kBoardRows + (kBoardRows - 2)] =
            floor_id;
    }
    BoardObject player;
    player.kind = 0;
    const KindInfo& k = engine.profile().kind(0);
    player.w = k.width > 0 ? k.width : 16;
    player.h = k.height > 0 ? k.height : 32;
    player.x = 64;
    player.y = (kBoardRows - 2) * 16 - player.h;
    player.prev_x = player.x;
    player.prev_y = player.y;
    board_.objects.push_back(player);
    board_.header.fill(0);
    board_.header[2] = 6;  // default health

    selected_obj_ = -1;
    anchor_valid_ = false;
    dirty_ = true;
    show_tiles_ = show_objects_ = show_help_ = show_info_ = show_files_ =
        show_props_ = show_game_confirm_ = show_presets_ = false;
    preset_ = Preset::None;
    text_input_active_ = false;
    region_valid_ = false;
    undo_coalesce_ = 0;
    clear_history();
    save_name_ = unique_level_name(engine);
    cam_x_ = clamp_int(player.x - Framebuffer::W / 2, 0,
                       Board::kCols * 16 - Framebuffer::W);
    cam_y_ = clamp_int(player.y - Framebuffer::H / 2, 0,
                       Board::kRows * 16 - Framebuffer::H);
    cursor_col_ = clamp_int((cam_x_ + Framebuffer::W / 2) >> 4, 0,
                            kBoardCols - 1);
    cursor_row_ = clamp_int((cam_y_ + Framebuffer::H / 2) >> 4, 0,
                            kBoardRows - 1);
    cursor_wx_ = cursor_col_ * 16 + 8;
    cursor_wy_ = cursor_row_ * 16 + 8;

    if (!engine.apply_editor_board(board_)) {
        err = "could not apply the new board";
        return false;
    }
    mini_dirty_ = true;
    tile_colour_ready_.fill(false);
    engine.finish_dialogs();
    engine.close_message_box();
    engine.close_text_page();
    set_status("NEW LEVEL " + save_name_ + " - F2 SAVE");
    return true;
}

void LevelEditor::close() {
    open_ = false;
    playtest_ = false;
    left_down_ = false;
    right_down_ = false;
    dragging_ = false;
    drag_obj_ = -1;
    selected_obj_ = -1;
    board_ = Board{};
    board_name_.clear();
    save_name_.clear();
    game_id_.clear();
    show_files_ = false;
    show_menu_ = false;
    show_props_ = false;
    show_game_confirm_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    text_input_active_ = false;
    touch_pan_ = false;
    touch_panning_ = false;
    touch_canvas_ = false;
    touch_swallow_ = false;
    touch_erase_ = false;
    anchor_valid_ = false;
    has_clipboard_ = false;
    region_valid_ = false;
    tile_clip_.clear();
    tile_clip_w_ = 0;
    tile_clip_h_ = 0;
    undo_coalesce_ = 0;
    clear_history();
    file_rows_.clear();
    status_.clear();
}

bool LevelEditor::handle_key(Key key, Engine& engine) {
    if (!open_) {
        return false;
    }
    if (playtest_) {
        if (key == Key::Playtest) {
            stop_playtest(engine);
        } else if (key == Key::Escape) {
            // Escape/back/circle first dismisses an in-game dialog and leaves
            // the playtest running; a second press returns to the editor.
            if (engine.text_page_open()) {
                engine.close_text_page();
                set_status("PLAYTEST - DIALOG CLOSED");
            } else if (engine.caption_popup_active() ||
                       engine.message_box_open()) {
                engine.finish_dialogs();
                set_status("PLAYTEST - DIALOG CLOSED");
            } else {
                stop_playtest(engine);
            }
        } else if (key == Key::Save) {
            save(engine);
        }
        return false;
    }

    if (text_input_active_) {
        if (textkey::controller_only()) {
            switch (key) {
                case Key::Escape:
                    text_input_cancel();
                    break;
                case Key::Up:
                    keyboard_.move(0, -1);
                    break;
                case Key::Down:
                    keyboard_.move(0, 1);
                    break;
                case Key::Left:
                    keyboard_.move(-1, 0);
                    break;
                case Key::Right:
                    keyboard_.move(1, 0);
                    break;
                case Key::Confirm:
                    if (keyboard_.press()) {
                        if (keyboard_.cancelled()) {
                            text_input_cancel();
                        } else {
                            text_input_commit(engine);
                        }
                    }
                    break;
                default:
                    break;
            }
        } else if (key == Key::Escape) {
            text_input_cancel();
        } else if (key == Key::Confirm) {
            text_input_commit(engine);
        }
        return false;
    }

    // Overlay dismissal first: Esc/Menu closes the panel, not the editor.
    if (show_tiles_ || show_objects_ || show_help_ || show_info_ ||
        show_files_ || show_menu_ || show_props_ || show_presets_ ||
        show_game_confirm_) {
        if (key == Key::Escape || key == Key::Menu) {
            show_tiles_ = false;
            show_objects_ = false;
            show_help_ = false;
            show_info_ = false;
            show_files_ = false;
            show_menu_ = false;
            show_props_ = false;
            show_presets_ = false;
            show_game_confirm_ = false;
            return false;
        }
    }
    if (show_presets_) {
        if (key == Key::Up) {
            preset_sel_ = (preset_sel_ + kPresetRows - 1) % kPresetRows;
        } else if (key == Key::Down) {
            preset_sel_ = (preset_sel_ + 1) % kPresetRows;
        } else if (key == Key::Confirm) {
            choose_preset(preset_sel_, engine);
        }
        return false;
    }
    if (show_game_confirm_) {
        if (key == Key::Confirm) {
            save_to_game_data(engine);
            show_game_confirm_ = false;
        }
        return false;
    }
    if (show_props_) {
        if (key == Key::Up) {
            prop_sel_ = (prop_sel_ + kPropFields - 1) % kPropFields;
            undo_coalesce_ = 0;
        } else if (key == Key::Down) {
            prop_sel_ = (prop_sel_ + 1) % kPropFields;
            undo_coalesce_ = 0;
        } else if (key == Key::Left) {
            adjust_object_field(-1, engine);
        } else if (key == Key::Right) {
            adjust_object_field(1, engine);
        } else if (key == Key::Confirm) {
            activate_prop_field(engine);
        }
        return false;
    }
    if (show_help_) {
        if (key == Key::Up) {
            scroll_help(-1);
        } else if (key == Key::Down) {
            scroll_help(1);
        } else if (key == Key::PageUp) {
            scroll_help(-kHelpVisible);
        } else if (key == Key::PageDown) {
            scroll_help(kHelpVisible);
        } else if (key == Key::Confirm) {
            show_help_ = false;
        }
        return false;
    }
    if (show_menu_) {
        if (key == Key::Up) {
            menu_sel_ = (menu_sel_ + kEditorMenuRows - 1) % kEditorMenuRows;
        } else if (key == Key::Down) {
            menu_sel_ = (menu_sel_ + 1) % kEditorMenuRows;
        } else if (key == Key::Confirm) {
            if (activate_menu_row(menu_sel_, engine)) {
                return true;
            }
        }
        return false;
    }
    if (show_files_) {
        if (key == Key::Up) {
            navigate(0, -1, engine);
            return false;
        }
        if (key == Key::Down) {
            navigate(0, 1, engine);
            return false;
        }
        if (key == Key::PageUp) {
            navigate(0, -5, engine);
            return false;
        }
        if (key == Key::PageDown) {
            navigate(0, 5, engine);
            return false;
        }
        if (key == Key::Confirm) {
            choose_file(engine);
            return false;
        }
    }
    if (show_info_) {
        switch (key) {
            case Key::Up:
                navigate(0, -1, engine);
                return false;
            case Key::Down:
                navigate(0, 1, engine);
                return false;
            case Key::Left:
                navigate(-1, 0, engine);
                return false;
            case Key::Right:
            case Key::Confirm:
                navigate(1, 0, engine);
                return false;
            default:
                break;
        }
    }
    if (show_tiles_ || show_objects_) {
        if (key == Key::PageUp) {
            page_palettes(-1);
            return false;
        }
        if (key == Key::PageDown) {
            page_palettes(1);
            return false;
        }
        if (key == Key::Up) {
            navigate(0, -1, engine);
            return false;
        }
        if (key == Key::Down) {
            navigate(0, 1, engine);
            return false;
        }
        if (key == Key::Left) {
            navigate(-1, 0, engine);
            return false;
        }
        if (key == Key::Right) {
            navigate(1, 0, engine);
            return false;
        }
        if (key == Key::Confirm) {
            show_tiles_ = false;
            show_objects_ = false;
            return false;
        }
    } else {
        int dx = 0;
        int dy = 0;
        if (key == Key::Up) {
            dy = -1;
        } else if (key == Key::Down) {
            dy = 1;
        } else if (key == Key::Left) {
            dx = -1;
        } else if (key == Key::Right) {
            dx = 1;
        }
        if (dx != 0 || dy != 0) {
            if (mode_ == Mode::Select && selected_obj_ >= 0) {
                move_selected(dx, dy, engine);
            } else {
                navigate(dx, dy, engine);
            }
            return false;
        }
        if (key == Key::Confirm) {
            primary_action(engine);
            return false;
        }
        if (key == Key::Erase) {
            erase_action(engine);
            return false;
        }
        if (key == Key::Pick) {
            pick_action(engine);
            return false;
        }
    }

    switch (key) {
        case Key::Escape:
            if (preset_ != Preset::None) {
                preset_ = Preset::None;
                set_status("PRESET CANCELLED");
                break;
            }
            if (mode_ == Mode::Tile && paint_mode_ == PaintMode::Line &&
                anchor_valid_) {
                anchor_valid_ = false;
                set_status("LINE CANCELLED");
                break;
            }
            if (mode_ == Mode::Region &&
                (region_valid_ || anchor_valid_)) {
                clear_region();
                set_status("REGION CLEARED");
                break;
            }
            if (mode_ == Mode::Select && selected_obj_ >= 0) {
                selected_obj_ = -1;
                set_status("SELECTION CLEARED");
                break;
            }
            show_menu_ = true;
            menu_sel_ = 0;
            show_tiles_ = show_objects_ = show_info_ = show_help_ =
                show_files_ = show_presets_ = false;
            break;
        case Key::ModeTile:
            mode_ = Mode::Tile;
            preset_ = Preset::None;
            anchor_valid_ = false;
            show_tiles_ = show_objects_ = show_info_ = show_help_ = show_presets_ =
                false;
            break;
        case Key::ModeObject:
            mode_ = Mode::Object;
            anchor_valid_ = false;
            show_tiles_ = show_objects_ = show_info_ = show_help_ = show_presets_ =
                false;
            break;
        case Key::ModeSelect:
            mode_ = Mode::Select;
            preset_ = Preset::None;
            anchor_valid_ = false;
            show_tiles_ = show_objects_ = show_info_ = show_help_ = show_presets_ =
                false;
            break;
        case Key::ModeRegion:
            mode_ = Mode::Region;
            preset_ = Preset::None;
            anchor_valid_ = false;
            show_tiles_ = show_objects_ = show_info_ = show_help_ = show_presets_ =
                false;
            set_status("TILE REGION MODE - CLICK A THEN B");
            break;
        case Key::ModeNext:
        case Key::ModePrev: {
            const int dir = key == Key::ModeNext ? 1 : -1;
            int mode = static_cast<int>(mode_) + dir;
            if (mode < 0) {
                mode = 3;
            } else if (mode > 3) {
                mode = 0;
            }
            mode_ = static_cast<Mode>(mode);
            preset_ = Preset::None;
            show_tiles_ = show_objects_ = show_info_ = show_help_ = show_presets_ =
                false;
            set_status(mode_ == Mode::Tile
                           ? "TILE MODE"
                           : (mode_ == Mode::Object
                                  ? "OBJECT MODE"
                                  : (mode_ == Mode::Select ? "SELECT MODE"
                                                           : "REGION MODE")));
            break;
        }
        case Key::Palette:
            if (mode_ == Mode::Tile) {
                show_tiles_ = !show_tiles_;
                show_objects_ = show_info_ = show_help_ = false;
            } else {
                show_objects_ = !show_objects_;
                show_tiles_ = show_info_ = show_help_ = false;
            }
            break;
        case Key::TilePalette:
            show_tiles_ = !show_tiles_;
            show_objects_ = show_help_ = show_info_ = false;
            break;
        case Key::ObjectPalette:
            show_objects_ = !show_objects_;
            show_tiles_ = show_help_ = show_info_ = false;
            break;
        case Key::Help:
            show_help_ = !show_help_;
            if (show_help_) {
                help_scroll_ = 0;
            }
            show_tiles_ = show_objects_ = show_info_ = false;
            break;
        case Key::Info:
            show_info_ = !show_info_;
            if (show_info_) {
                undo_coalesce_ = 0;
            }
            show_tiles_ = show_objects_ = show_help_ = false;
            break;
        case Key::Grid:
            show_grid_ = !show_grid_;
            break;
        case Key::Minimap:
            show_minimap_ = !show_minimap_;
            set_status(show_minimap_ ? "MINIMAP ON" : "MINIMAP OFF");
            break;
        case Key::Menu:
            show_menu_ = true;
            menu_sel_ = 0;
            show_tiles_ = show_objects_ = show_info_ = show_help_ =
                show_files_ = show_presets_ = show_props_ = false;
            break;
        case Key::PaintMode:
            toggle_paint_mode();
            break;
        case Key::Undo:
            undo(engine);
            break;
        case Key::Redo:
            redo(engine);
            break;
        case Key::Copy:
            if (mode_ == Mode::Region) {
                copy_tiles();
            } else {
                copy_object(engine);
            }
            break;
        case Key::Paste:
            if (mode_ == Mode::Region) {
                paste_tiles(engine);
            } else {
                paste_object(engine);
            }
            break;
        case Key::ObjectProps:
            open_object_props(engine);
            break;
        case Key::Presets:
            open_presets();
            break;
        case Key::Save:
            save(engine);
            break;
        case Key::SaveAs:
            open_file_picker(FileMode::Save, engine);
            break;
        case Key::LoadPick:
            open_file_picker(FileMode::Load, engine);
            break;
        case Key::Playtest:
            start_playtest_at_cursor(engine);
            break;
        case Key::LoadSaved:
            load_saved(engine);
            break;
        case Key::PageUp:
            page_palettes(-1);
            break;
        case Key::PageDown:
            page_palettes(1);
            break;
        default:
            break;
    }
    return false;
}

void LevelEditor::set_mouse(int x, int y, Engine& engine) {
    mouse_x_ = x;
    mouse_y_ = y;
    if (playtest_ || show_tiles_ || show_objects_ || show_help_ ||
        show_info_ || show_files_ || show_menu_ || show_props_ ||
        show_presets_ || show_game_confirm_ || text_input_active_) {
        return;
    }
    if (y < kTopBar || y >= kBottomBar) {
        return;
    }
    cursor_wx_ = cam_x_ + x;
    cursor_wy_ = cam_y_ + y;
    cursor_col_ = clamp_int(cursor_wx_ >> 4, 0, kBoardCols - 1);
    cursor_row_ = clamp_int(cursor_wy_ >> 4, 0, kBoardRows - 1);
    if (left_down_) {
        if (mode_ == Mode::Tile) {
            paint_brush(engine, false);
        } else if (mode_ == Mode::Select && dragging_) {
            drag_selected(engine);
        }
    }
    if (right_down_ && mode_ == Mode::Tile) {
        paint_brush(engine, true);
    }
}

void LevelEditor::mouse_button(int button, bool down, int x, int y, bool shift,
                               bool ctrl, Engine& engine) {
    (void)ctrl;
    set_mouse(x, y, engine);
    if (!open_ || playtest_) {
        return;
    }
    if (text_input_active_) {
        return;
    }
    if (show_game_confirm_) {
        if (down && button == 3) {
            show_game_confirm_ = false;
            return;
        }
        if (down && button == 1) {
            if (x >= 40 && x < 140 && y >= 120 && y < 134) {
                save_to_game_data(engine);
                show_game_confirm_ = false;
            } else if (x >= 180 && x < 280 && y >= 120 && y < 134) {
                show_game_confirm_ = false;
            }
        }
        return;
    }
    if (show_props_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_props_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        if (prop_obj_ < 0 ||
            prop_obj_ >= static_cast<int>(board_.objects.size())) {
            show_props_ = false;
            return;
        }
        const int row = (y - (12 + 22)) / 9;
        if (y >= 12 + 22 && row >= 0 && row < kPropFields) {
            prop_sel_ = row;
            undo_coalesce_ = 0;
            if (row == kPropFields - 1) {
                start_text_prompt(
                    "OBJECT STRING",
                    board_.objects[static_cast<size_t>(prop_obj_)].text);
            } else if (row == 10) {
                link_trigger(engine);
            } else {
                adjust_object_field(x < 160 ? -1 : 1, engine);
            }
        } else {
            show_props_ = false;
        }
        return;
    }
    if (show_presets_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_presets_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        const int row = (y - (20 + 30)) / 22;
        if (y >= 20 + 30 && row >= 0 && row < kPresetRows) {
            preset_sel_ = row;
            choose_preset(row, engine);
        } else {
            show_presets_ = false;
        }
        return;
    }
    if (show_help_) {
        if (down && (button == 1 || button == 3)) {
            show_help_ = false;
        }
        return;
    }
    if (show_tiles_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_tiles_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        const int gx = 4 + 8;
        const int gy = 16 + 22;
        const int col = (x - gx) / 28;
        const int row = (y - gy) / 18;
        if (col >= 0 && col < kTilePalCols && row >= 0 && row < kTilePalRows) {
            const int idx = tile_page_ + row * kTilePalCols + col;
            if (idx >= 0 && idx < kTileCount) {
                paint_tile_ = idx;
                tile_page_ = (idx / kTilePalCols) * kTilePalCols;
                char buf[64];
                std::snprintf(buf, sizeof(buf), "TILE %03X %s",
                              idx, engine.tile(idx).name.c_str());
                set_status(buf);
            }
        } else {
            show_tiles_ = false;
        }
        return;
    }
    if (show_objects_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_objects_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        const int gx = 4 + 8;
        const int gy = 16 + 24;
        const int col = (x - gx) / 48;
        const int row = (y - gy) / 36;
        if (col >= 0 && col < kKindPalCols && row >= 0 && row < kKindPalRows) {
            const int idx = kind_page_ + row * kKindPalCols + col;
            if (idx >= 0 && idx < engine.profile().kind_count) {
                brush_kind_ = idx;
                preset_ = Preset::None;
                kind_page_ = (idx / kKindPalPage) * kKindPalPage;
                char buf[64];
                std::snprintf(buf, sizeof(buf), "OBJECT %03d %s", idx,
                              engine.profile().kind(idx).name);
                set_status(buf);
            }
        } else {
            show_objects_ = false;
        }
        return;
    }
    if (show_menu_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_menu_ = false;
            return;
        }
        if (button == 1) {
            const int row = (y - (2 + 18)) / 8;
            if (y >= 2 + 18 && row >= 0 && row < kEditorMenuRows) {
                menu_sel_ = row;
                activate_menu_row(row, engine);
            } else {
                show_menu_ = false;
            }
        }
        return;
    }
    if (show_files_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_files_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        const int row_y = 16 + 28;
        const int row = (y - row_y) / 12;
        const int start = file_list_start();
        const int index = start + row;
        if (y >= row_y && row >= 0 && row < 10 && index >= 0 &&
            index < static_cast<int>(file_rows_.size())) {
            file_sel_ = index;
            choose_file(engine);
        } else {
            show_files_ = false;
        }
        return;
    }
    if (show_info_) {
        if (!down) {
            return;
        }
        if (button == 3) {
            show_info_ = false;
            return;
        }
        if (button != 1) {
            return;
        }
        const int row_y = 16 + 26;
        const int row = (y - row_y) / 16;
        if (y >= row_y && row >= 0 && row < kInfoFields) {
            info_sel_ = row;
            if (x < 4 + 70) {
                adjust_info(-1, engine);
            } else if (x > 4 + 312 - 70) {
                adjust_info(1, engine);
            }
        } else {
            show_info_ = false;
        }
        return;
    }
    if (y < kTopBar || y >= kBottomBar) {
        return;
    }

    const int wx = cam_x_ + x;
    const int wy = cam_y_ + y;

    if (button == 1) {
        if (down) {
            if (mode_ == Mode::Tile) {
                if (shift) {
                    pick_tile(engine);
                } else if (paint_mode_ == PaintMode::Line ||
                           paint_mode_ == PaintMode::Rect ||
                           paint_mode_ == PaintMode::Fill) {
                    primary_tile_action(engine);
                } else {
                    push_undo();
                    left_down_ = true;
                    last_paint_valid_ = false;
                    paint_brush(engine, false);
                }
            } else if (mode_ == Mode::Object) {
                if (shift) {
                    pick_object_kind(engine);
                } else {
                    push_undo();
                    place_object(engine);
                }
            } else if (mode_ == Mode::Region) {
                if (anchor_valid_) {
                    region_a_col_ = anchor_col_;
                    region_a_row_ = anchor_row_;
                    region_b_col_ = cursor_col_;
                    region_b_row_ = cursor_row_;
                    region_valid_ = true;
                    anchor_valid_ = false;
                    set_status("TILE REGION SELECTED - C COPY  V PASTE");
                } else {
                    anchor_valid_ = true;
                    anchor_col_ = cursor_col_;
                    anchor_row_ = cursor_row_;
                    region_valid_ = false;
                    set_status("REGION A SET - CLICK B");
                }
            } else {
                if (shift) {
                    pick_object_kind(engine);
                } else {
                    selected_obj_ = object_at(wx, wy);
                    if (selected_obj_ >= 0) {
                        push_undo();
                        dragging_ = true;
                        drag_obj_ = selected_obj_;
                        drag_last_wx_ = wx;
                        drag_last_wy_ = wy;
                        left_down_ = true;
                        set_status("DRAGGING OBJECT - RELEASE TO PLACE");
                    } else {
                        set_status("NO OBJECT AT CURSOR");
                    }
                }
            }
        } else {
            left_down_ = false;
            if (dragging_) {
                drag_obj_ = -1;
                dragging_ = false;
                set_status("OBJECT MOVED");
            }
            last_paint_valid_ = false;
        }
        return;
    }
    if (button == 3) {
        if (!down) {
            right_down_ = false;
            last_paint_valid_ = false;
            return;
        }
        if (mode_ == Mode::Region) {
            clear_region();
            set_status("REGION CLEARED");
            return;
        }
        if (mode_ == Mode::Tile) {
            if (shift) {
                const int col = wx >> 4;
                const int row = wy >> 4;
                if (col >= 0 && col < kBoardCols && row >= 0 &&
                    row < kBoardRows) {
                    erase_tile_ = tile_at(col, row);
                    char buf[48];
                    std::snprintf(buf, sizeof(buf), "ERASE TILE %03X",
                                  erase_tile_);
                    set_status(buf);
                }
            } else {
                push_undo();
                right_down_ = true;
                last_paint_valid_ = false;
                paint_brush(engine, true);
            }
        } else {
            const int index = object_at(wx, wy);
            if (index >= 0) {
                push_undo();
            }
            delete_object_at(index, engine);
        }
    }
}

void LevelEditor::release_mouse() {
    const bool was_dragging = dragging_;
    left_down_ = false;
    right_down_ = false;
    dragging_ = false;
    drag_obj_ = -1;
    last_paint_valid_ = false;
    // A lost SDL_FINGERUP (focus loss, backgrounding) must not leave the canvas
    // drag armed; touch_move would keep painting and edge-scrolling the view.
    touch_canvas_ = false;
    touch_swallow_ = false;
    if (was_dragging) {
        set_status("OBJECT MOVED");
    }
}

bool LevelEditor::touch_down(int x, int y, Engine& engine) {
    if (!open_ || playtest_ || text_input_active_) {
        return text_input_active_;
    }
    const bool overlay_before = overlay_open();
    touch_x_ = x;
    touch_y_ = y;
    touch_canvas_ = false;
    touch_swallow_ = false;
    auto cancel_paint = [&]() {
        left_down_ = false;
        right_down_ = false;
        dragging_ = false;
        drag_obj_ = -1;
        last_paint_valid_ = false;
        touch_canvas_ = false;
    };
    if (touch_controls_ && !overlay_before && y >= kTouchBarY) {
        const int row = clamp_int((y - kTouchBarY) / 20, 0, kTouchRows - 1);
        const int col =
            clamp_int(x * kTouchCols / Framebuffer::W, 0, kTouchCols - 1);
        switch (row * kTouchCols + col) {
            case 0:  // MODE
                handle_key(Key::ModeNext, engine);
                break;
            case 1:  // PAL
                handle_key(Key::Palette, engine);
                break;
            case 2:  // PICK
                handle_key(Key::Pick, engine);
                break;
            case 3:  // TOOL
                handle_key(Key::PaintMode, engine);
                break;
            case 4:  // PAN
                touch_pan_ = !touch_pan_;
                set_status(touch_pan_ ? "PAN: DRAG CANVAS"
                                      : "PAN OFF: PAINT CANVAS");
                break;
            case 5:  // SAVE
                handle_key(Key::SaveAs, engine);
                break;
            case 6:  // LOAD
                handle_key(Key::LoadPick, engine);
                break;
            case 7:  // PLAY
                handle_key(Key::Playtest, engine);
                break;
            case 8:  // INFO
                handle_key(Key::Info, engine);
                break;
            case 9:  // MENU
                show_menu_ = true;
                menu_sel_ = 0;
                show_tiles_ = show_objects_ = show_info_ = show_help_ =
                    show_files_ = show_presets_ = false;
                break;
            case 10:
                handle_key(Key::Undo, engine);
                break;
            case 11:
                handle_key(Key::Redo, engine);
                break;
            case 12:
                handle_key(Key::Copy, engine);
                break;
            case 13:
                handle_key(Key::Paste, engine);
                break;
            case 14:
                handle_key(Key::ObjectProps, engine);
                break;
            case 15:
                handle_key(Key::Presets, engine);
                break;
            case 16:
                handle_key(Key::Grid, engine);
                break;
            case 17:
                handle_key(Key::Help, engine);
                break;
            case 18:
                handle_key(Key::Minimap, engine);
                break;
            default:  // ERASE
                touch_erase_ = !touch_erase_;
                set_status(touch_erase_ ? "TOUCH ERASE ON" : "TOUCH PAINT ON");
                break;
        }
        touch_swallow_ = true;
        cancel_paint();
        return true;
    }
    if (touch_pan_ && !overlay_before) {
        touch_panning_ = true;
        touch_pan_x_ = x;
        touch_pan_y_ = y;
        return true;
    }
    touch_canvas_ = !overlay_before;
    if (touch_canvas_) {
        touch_button_ = touch_erase_ ? 3 : 1;
        mouse_button(touch_button_, true, x, y, false, false, engine);
    } else {
        // Overlay/pause-menu tap: consume the whole touch sequence so the
        // finger-up (or a small drag) cannot fall through and paint a tile
        // just as the menu closes.
        touch_swallow_ = true;
        cancel_paint();
        mouse_button(1, true, x, y, false, false, engine);
    }
    return true;
}

void LevelEditor::touch_move(int x, int y, Engine& engine) {
    if (!open_ || playtest_ || text_input_active_) {
        return;
    }
    if (touch_panning_) {
        pan(touch_pan_x_ - x, touch_pan_y_ - y);
        touch_pan_x_ = x;
        touch_pan_y_ = y;
        return;
    }
    if (touch_swallow_ || !touch_canvas_) {
        return;
    }
    touch_x_ = x;
    touch_y_ = y;
    set_mouse(x, std::min(y, kTouchBarY - 1), engine);
}

void LevelEditor::touch_up(Engine& engine) {
    if (!open_ || playtest_ || text_input_active_) {
        return;
    }
    if (touch_panning_) {
        touch_panning_ = false;
    }
    if (touch_swallow_) {
        touch_swallow_ = false;
        touch_canvas_ = false;
        return;
    }
    if (touch_canvas_) {
        mouse_button(touch_button_, false, touch_x_, touch_y_, false, false,
                     engine);
    }
    touch_canvas_ = false;
}

void LevelEditor::wheel(int dy, bool shift) {
    if (!open_ || playtest_ || show_game_confirm_ || show_presets_ ||
        text_input_active_) {
        return;
    }
    if (show_help_) {
        scroll_help(-dy);
        return;
    }
    if (show_props_) {
        prop_sel_ = clamp_int(prop_sel_ + dy, 0, kPropFields - 1);
        return;
    }
    if (show_tiles_) {
        tile_page_ -= dy * kTilePalCols;
        tile_page_ = clamp_int(tile_page_, 0, kTileCount - 1);
        tile_page_ = (tile_page_ / kTilePalCols) * kTilePalCols;
        return;
    }
    if (show_objects_) {
        kind_page_ -= dy * kKindPalPage;
        kind_page_ = clamp_int(kind_page_, 0, kind_count_ - 1);
        kind_page_ = (kind_page_ / kKindPalPage) * kKindPalPage;
        return;
    }
    if (shift) {
        cam_x_ -= dy * 32;
    } else {
        cam_y_ -= dy * 32;
    }
    cam_x_ = clamp_int(cam_x_, 0, Board::kCols * 16 - Framebuffer::W);
    cam_y_ = clamp_int(cam_y_, 0, Board::kRows * 16 - Framebuffer::H);
    keep_cursor_in_view();
}

void LevelEditor::pan(int dx, int dy) {
    cam_x_ += dx;
    cam_y_ += dy;
    cam_x_ = clamp_int(cam_x_, 0, Board::kCols * 16 - Framebuffer::W);
    cam_y_ = clamp_int(cam_y_, 0, Board::kRows * 16 - Framebuffer::H);
    keep_cursor_in_view();
}

void LevelEditor::keep_cursor_in_view() {
    // One cell of slack so the highlight box (drawn one tile wide) stays fully
    // inside the editing canvas, which starts below the top bar.
    const int c0 = cam_x_ >> 4;
    const int c1 = (cam_x_ + Framebuffer::W - 1) >> 4;
    const int r0 = (cam_y_ + kTopBar) >> 4;
    const int r1 = (cam_y_ + kBottomBar - 1) >> 4;
    cursor_col_ = clamp_int(cursor_col_, c0, std::min(c1, kBoardCols - 1));
    cursor_row_ = clamp_int(cursor_row_, r0, std::min(r1, kBoardRows - 1));
    cursor_wx_ = cursor_col_ * 16 + 8;
    cursor_wy_ = cursor_row_ * 16 + 8;
}

void LevelEditor::move_cursor(int dx, int dy) {
    cursor_col_ = clamp_int(cursor_col_ + dx, 0, kBoardCols - 1);
    cursor_row_ = clamp_int(cursor_row_ + dy, 0, kBoardRows - 1);
    cursor_wx_ = cursor_col_ * 16 + 8;
    cursor_wy_ = cursor_row_ * 16 + 8;
    // Keep the cursor on screen by scrolling one tile at a time.
    const int sx = cursor_col_ * 16 - cam_x_;
    const int sy = cursor_row_ * 16 - cam_y_;
    if (sx < 16) {
        cam_x_ = std::max(0, cam_x_ - 16);
    } else if (sx > Framebuffer::W - 32) {
        cam_x_ = std::min(Board::kCols * 16 - Framebuffer::W, cam_x_ + 16);
    }
    if (sy < 16) {
        cam_y_ = std::max(0, cam_y_ - 16);
    } else if (sy > Framebuffer::H - 32) {
        cam_y_ = std::min(Board::kRows * 16 - Framebuffer::H, cam_y_ + 16);
    }
    // Moving the cursor without a held button starts a new stamp.
    last_paint_valid_ = false;
}

void LevelEditor::navigate(int dx, int dy, Engine& engine) {
    if (!open_ || playtest_ || text_input_active_) {
        return;
    }
    if (show_game_confirm_) {
        return;
    }
    if (show_presets_) {
        if (dy != 0) {
            preset_sel_ = (preset_sel_ + dy + kPresetRows) % kPresetRows;
        }
        return;
    }
    if (show_help_) {
        if (dy != 0) {
            scroll_help(dy);
        } else if (dx != 0) {
            scroll_help(dx * kHelpVisible);
        }
        return;
    }
    if (show_props_) {
        if (dy != 0) {
            prop_sel_ = (prop_sel_ + dy + kPropFields) % kPropFields;
        } else if (dx != 0) {
            adjust_object_field(dx, engine);
        }
        return;
    }
    if (show_menu_) {
        if (dy != 0) {
            menu_sel_ = (menu_sel_ + dy + kEditorMenuRows) % kEditorMenuRows;
        }
        return;
    }
    if (show_files_) {
        if (dy != 0) {
            file_sel_ = clamp_int(file_sel_ + dy, 0,
                                  static_cast<int>(file_rows_.size()) - 1);
        } else if (dx != 0) {
            file_sel_ = clamp_int(file_sel_ + dx * 5, 0,
                                  static_cast<int>(file_rows_.size()) - 1);
        }
        return;
    }
    if (show_info_) {
        if (dy < 0) {
            info_sel_ = (info_sel_ + kInfoFields - 1) % kInfoFields;
            undo_coalesce_ = 0;
        } else if (dy > 0) {
            info_sel_ = (info_sel_ + 1) % kInfoFields;
            undo_coalesce_ = 0;
        }
        if (dx != 0) {
            adjust_info(dx, engine);
        }
        return;
    }
    if (show_tiles_) {
        if (dx != 0) {
            paint_tile_ = clamp_int(paint_tile_ + dx, 0, kTileCount - 1);
        }
        if (dy != 0) {
            paint_tile_ = clamp_int(paint_tile_ + dy * kTilePalCols, 0,
                                    kTileCount - 1);
        }
        tile_page_ = (paint_tile_ / kTilePalCols) * kTilePalCols;
        return;
    }
    if (show_objects_) {
        const int hi = kind_count_ > 1 ? kind_count_ - 1 : 1;
        if (dx != 0) {
            brush_kind_ = clamp_int(brush_kind_ + dx, 1, hi);
        }
        if (dy != 0) {
            brush_kind_ = clamp_int(brush_kind_ + dy * kKindPalCols, 1, hi);
        }
        kind_page_ = (brush_kind_ / kKindPalPage) * kKindPalPage;
        preset_ = Preset::None;
        return;
    }
    if (mode_ == Mode::Select && selected_obj_ >= 0) {
        move_selected(dx, dy, engine);
        return;
    }
    move_cursor(dx, dy);
}

void LevelEditor::move_selected(int dx, int dy, Engine& engine) {
    if (selected_obj_ < 0 ||
        selected_obj_ >= static_cast<int>(board_.objects.size())) {
        move_cursor(dx, dy);
        return;
    }
    push_undo();
    BoardObject& o = board_.objects[static_cast<size_t>(selected_obj_)];
    const int w = o.w > 0 ? o.w : 8;
    const int h = o.h > 0 ? o.h : 8;
    o.x = clamp_int(o.x + dx * 16, 0, Board::kCols * 16 - w);
    o.y = clamp_int(o.y + dy * 16, 0, Board::kRows * 16 - h);
    dirty_ = true;
    apply(engine);
    set_status("OBJECT MOVED");
}

void LevelEditor::toggle_paint_mode() {
    switch (paint_mode_) {
        case PaintMode::Single:
            paint_mode_ = PaintMode::Line;
            break;
        case PaintMode::Line:
            paint_mode_ = PaintMode::Rect;
            break;
        case PaintMode::Rect:
            paint_mode_ = PaintMode::Fill;
            break;
        default:
            paint_mode_ = PaintMode::Single;
            break;
    }
    anchor_valid_ = false;
    const char* name = paint_mode_ == PaintMode::Single
                           ? "SINGLE"
                           : (paint_mode_ == PaintMode::Line
                                  ? "LINE A-B"
                                  : (paint_mode_ == PaintMode::Rect
                                         ? "RECT FILL"
                                         : "FLOOD FILL"));
    std::string text = "TILE PAINT: ";
    text += name;
    set_status(text);
}

void LevelEditor::paint_line(Engine& engine, int c0, int r0, int c1, int r1,
                             uint16_t id) {
    int x = c0;
    int y = r0;
    const int dx = std::abs(c1 - c0);
    const int sx = c0 < c1 ? 1 : -1;
    const int dy = -std::abs(r1 - r0);
    const int sy = r0 < r1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        if (x >= 0 && x < kBoardCols && y >= 0 && y < kBoardRows) {
            board_.map[static_cast<size_t>(x) * kBoardRows + y] =
                static_cast<uint16_t>(id & 0x3FFF);
        }
        if (x == c1 && y == r1) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
    dirty_ = true;
    apply(engine);
}

void LevelEditor::paint_rect(Engine& engine, int c0, int r0, int c1, int r1,
                             uint16_t id) {
    const int x0 = std::max(0, std::min(c0, c1));
    const int x1 = std::min(kBoardCols - 1, std::max(c0, c1));
    const int y0 = std::max(0, std::min(r0, r1));
    const int y1 = std::min(kBoardRows - 1, std::max(r0, r1));
    const uint16_t tile = static_cast<uint16_t>(id & 0x3FFF);
    for (int c = x0; c <= x1; ++c) {
        for (int r = y0; r <= y1; ++r) {
            board_.map[static_cast<size_t>(c) * kBoardRows + r] = tile;
        }
    }
    dirty_ = true;
    apply(engine);
}

void LevelEditor::flood_fill(Engine& engine, int c0, int r0, uint16_t id) {
    if (c0 < 0 || c0 >= kBoardCols || r0 < 0 || r0 >= kBoardRows) {
        return;
    }
    const uint16_t from =
        board_.map[static_cast<size_t>(c0) * kBoardRows + r0] & 0x3FFF;
    const uint16_t to = static_cast<uint16_t>(id & 0x3FFF);
    if (from == to) {
        return;
    }
    std::vector<int> stack;
    stack.push_back(c0 * kBoardRows + r0);
    int changed = 0;
    while (!stack.empty()) {
        const int at = stack.back();
        stack.pop_back();
        const int c = at / kBoardRows;
        const int r = at % kBoardRows;
        if (c < 0 || c >= kBoardCols || r < 0 || r >= kBoardRows) {
            continue;
        }
        if ((board_.map[static_cast<size_t>(at)] & 0x3FFF) != from) {
            continue;
        }
        board_.map[static_cast<size_t>(at)] = to;
        ++changed;
        stack.push_back(at - kBoardRows);
        stack.push_back(at + kBoardRows);
        if (r > 0) {
            stack.push_back(at - 1);
        }
        if (r + 1 < kBoardRows) {
            stack.push_back(at + 1);
        }
    }
    if (changed > 0) {
        dirty_ = true;
        apply(engine);
        set_status("FILL COMPLETE");
    }
}

void LevelEditor::primary_tile_action(Engine& engine) {
    if (paint_mode_ == PaintMode::Single) {
        push_undo();
        // Keyboard/pad placement must not join the previous cell.
        last_paint_valid_ = false;
        paint_brush(engine, false);
        return;
    }
    if (paint_mode_ == PaintMode::Fill) {
        push_undo();
        flood_fill(engine, cursor_col_, cursor_row_,
                   static_cast<uint16_t>(paint_tile_));
        return;
    }
    if (!anchor_valid_) {
        anchor_valid_ = true;
        anchor_col_ = cursor_col_;
        anchor_row_ = cursor_row_;
        set_status(paint_mode_ == PaintMode::Line ? "LINE A SET - CLICK B"
                                                  : "RECT A SET - CLICK B");
        return;
    }
    push_undo();
    if (paint_mode_ == PaintMode::Line) {
        paint_line(engine, anchor_col_, anchor_row_, cursor_col_, cursor_row_,
                   static_cast<uint16_t>(paint_tile_));
        set_status("LINE PAINTED");
    } else {
        paint_rect(engine, anchor_col_, anchor_row_, cursor_col_, cursor_row_,
                   static_cast<uint16_t>(paint_tile_));
        set_status("RECTANGLE FILLED");
    }
    anchor_valid_ = false;
}

void LevelEditor::open_object_props(Engine& engine) {
    (void)engine;
    int index = selected_obj_;
    if (index < 0 || index >= static_cast<int>(board_.objects.size())) {
        index = object_at(cursor_wx_, cursor_wy_);
    }
    if (index < 0) {
        set_status("NO OBJECT AT CURSOR");
        return;
    }
    prop_obj_ = index;
    prop_sel_ = 0;
    preset_ = Preset::None;
    undo_coalesce_ = 0;
    show_props_ = true;
    show_tiles_ = show_objects_ = show_info_ = show_help_ = show_files_ =
        show_menu_ = show_game_confirm_ = show_presets_ = false;
    text_input_active_ = false;
}

void LevelEditor::adjust_object_field(int delta, Engine& engine) {
    if (prop_obj_ < 0 ||
        prop_obj_ >= static_cast<int>(board_.objects.size())) {
        show_props_ = false;
        return;
    }
    if (prop_sel_ == kPropFields - 1) {
        start_text_prompt("OBJECT STRING",
                          board_.objects[static_cast<size_t>(prop_obj_)].text);
        return;
    }
    // Repeated tweaks of the same property share one undo entry.
    push_undo_coalesced(0x2000000ull +
                        static_cast<uint64_t>(prop_obj_ + 1) * 0x100ull +
                        static_cast<uint64_t>(prop_sel_));
    BoardObject& o = board_.objects[static_cast<size_t>(prop_obj_)];
    switch (prop_sel_) {
        case 0: {
            const int kind = clamp_int(o.kind + delta, 0,
                                       kind_count_ > 0 ? kind_count_ - 1
                                                       : 0);
            o.kind = kind;
            const KindInfo& k = engine.profile().kind(kind);
            if (k.width > 0) {
                o.w = k.width;
            }
            if (k.height > 0) {
                o.h = k.height;
            }
            break;
        }
        case 1:
            o.x = clamp_int(o.x + delta, 0, Board::kCols * 16 - 1);
            break;
        case 2:
            o.y = clamp_int(o.y + delta, 0, Board::kRows * 16 - 1);
            break;
        case 3:
            o.dx = clamp_int(o.dx + delta, -2048, 2047);
            break;
        case 4:
            o.dy = clamp_int(o.dy + delta, -2048, 2047);
            break;
        case 5:
            o.w = clamp_int(o.w + delta, 1, 0x7F0);
            break;
        case 6:
            o.h = clamp_int(o.h + delta, 1, 0x3F0);
            break;
        case 7:
            o.state = clamp_int(o.state + delta, -128, 127);
            break;
        case 8:
            o.p0f = clamp_int(o.p0f + delta, 0, 0xFFFF);
            break;
        case 9:
            o.p11 = clamp_int(o.p11 + delta, 0, 0xFFFF);
            break;
        case 10:
            o.p13 = clamp_int(o.p13 + delta, 0, 0xFFFF);
            break;
        case 11:
            o.flags = clamp_int(o.flags + delta, 0, 0x3FFF);
            break;
        case 12:
            o.p1b = clamp_int(o.p1b + delta, 0, 0xFFFF);
            break;
        default:
            o.ttl = clamp_int(o.ttl + delta, 0, 0xFFFF);
            break;
    }
    dirty_ = true;
    apply(engine);
    set_status("OBJECT UPDATED");
}

void LevelEditor::activate_prop_field(Engine& engine) {
    if (prop_obj_ < 0 ||
        prop_obj_ >= static_cast<int>(board_.objects.size())) {
        show_props_ = false;
        return;
    }
    if (prop_sel_ == kPropFields - 1) {
        start_text_prompt("OBJECT STRING",
                          board_.objects[static_cast<size_t>(prop_obj_)].text);
        return;
    }
    if (prop_sel_ == 10) {  // P13 trigger channel: link to the nearest peer
        link_trigger(engine);
        return;
    }
    adjust_object_field(1, engine);
}

int LevelEditor::linked_trigger_count(const BoardObject& o) const {
    int count = 0;
    for (int i = 0; i < static_cast<int>(board_.objects.size()); ++i) {
        const BoardObject& other = board_.objects[static_cast<size_t>(i)];
        if (&other == &o) {
            continue;
        }
        const bool other_trigger =
            (other.flags & 0x0100) || other.kind == 15 || other.kind == 32 ||
            other.kind == 52;
        if (other_trigger && other.p13 == o.p13) {
            ++count;
        }
    }
    return count;
}

void LevelEditor::link_trigger(Engine& engine) {
    if (prop_obj_ < 0 ||
        prop_obj_ >= static_cast<int>(board_.objects.size())) {
        show_props_ = false;
        return;
    }
    BoardObject& src = board_.objects[static_cast<size_t>(prop_obj_)];
    const bool src_receiver =
        (engine.profile().kind(src.kind).flags & 0x0100) != 0;
    int best = -1;
    long best_dist = 0;
    for (int i = 0; i < static_cast<int>(board_.objects.size()); ++i) {
        if (i == prop_obj_) {
            continue;
        }
        const BoardObject& other = board_.objects[static_cast<size_t>(i)];
        const bool other_receiver =
            (engine.profile().kind(other.kind).flags & 0x0100) != 0;
        const bool other_sender =
            other.kind == 15 || other.kind == 32 || other.kind == 52;
        const bool candidate =
            src_receiver ? other_sender : other_receiver;
        if (!candidate) {
            continue;
        }
        const long dx = static_cast<long>(other.x - src.x);
        const long dy = static_cast<long>(other.y - src.y);
        const long dist = dx * dx + dy * dy;
        if (best < 0 || dist < best_dist) {
            best = i;
            best_dist = dist;
        }
    }
    if (best < 0) {
        set_status(src_receiver ? "NO TRIGGER SENDER FOUND"
                                : "NO TRIGGER RECEIVER FOUND");
        return;
    }
    const BoardObject& peer = board_.objects[static_cast<size_t>(best)];
    if (peer.p13 == src.p13) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "ALREADY LINKED TO %s #%d",
                      engine.profile().kind(peer.kind).name, best);
        set_status(buf);
        return;
    }
    push_undo();
    src.p13 = peer.p13;
    dirty_ = true;
    apply(engine);
    char buf[80];
    std::snprintf(buf, sizeof(buf), "LINKED TO %s #%d  CHANNEL %d",
                  engine.profile().kind(peer.kind).name, best, peer.p13);
    set_status(buf);
}

void LevelEditor::draw_object_props(Framebuffer& fb, Engine& engine) {
    const int x = 8;
    const int y = 12;
    const int w = 304;
    const int h = 168;
    engine.draw_menu_window(fb, x, y, w, h);
    if (prop_obj_ < 0 ||
        prop_obj_ >= static_cast<int>(board_.objects.size())) {
        engine.draw_text(fb, x + 10, y + 10, 15, "NO OBJECT");
        return;
    }
    const BoardObject& o = board_.objects[static_cast<size_t>(prop_obj_)];
    char title[64];
    std::snprintf(title, sizeof(title), "OBJECT %d  KIND %d %s", prop_obj_,
                  o.kind, engine.profile().kind(o.kind).name);
    engine.draw_text(fb, x + 10, y + 8, 15, shortened(title, 46).c_str());

    static const char* const labels[kPropFields] = {
        "KIND", "X", "Y", "DX", "DY", "W", "H", "STATE", "PARAM",
        "P11", "P13", "FLAGS", "P1B", "TTL", "STRING"};
    char values[kPropFields][32] = {};
    std::snprintf(values[0], sizeof(values[0]), "%d", o.kind);
    std::snprintf(values[1], sizeof(values[1]), "%d", o.x);
    std::snprintf(values[2], sizeof(values[2]), "%d", o.y);
    std::snprintf(values[3], sizeof(values[3]), "%d", o.dx);
    std::snprintf(values[4], sizeof(values[4]), "%d", o.dy);
    std::snprintf(values[5], sizeof(values[5]), "%d", o.w);
    std::snprintf(values[6], sizeof(values[6]), "%d", o.h);
    std::snprintf(values[7], sizeof(values[7]), "%d", o.state);
    std::snprintf(values[8], sizeof(values[8]), "%d", o.p0f);
    std::snprintf(values[9], sizeof(values[9]), "%d", o.p11);
    std::snprintf(values[10], sizeof(values[10]), "%d L%d", o.p13,
                  linked_trigger_count(o));
    std::snprintf(values[11], sizeof(values[11]), "%d", o.flags);
    std::snprintf(values[12], sizeof(values[12]), "%d", o.p1b);
    std::snprintf(values[13], sizeof(values[13]), "%d", o.ttl);
    std::snprintf(values[14], sizeof(values[14]), "%s", o.text.c_str());
    for (int i = 0; i < kPropFields; ++i) {
        char line[512];
        std::snprintf(line, sizeof(line), "%c %-7s %s",
                      i == prop_sel_ ? '>' : ' ', labels[i], values[i]);
        engine.draw_text(fb, x + 12, y + 22 + i * 9,
                         i == prop_sel_ ? 15 : 10,
                         shortened(line, 47).c_str());
    }
    engine.draw_text(fb, x + 10, y + h - 10, 10,
                     "UP/DOWN FIELD  LEFT/RIGHT CHANGE  ENTER STRING/P13 LINK");
}

void LevelEditor::open_presets() {
    show_presets_ = true;
    preset_sel_ = 0;
    show_tiles_ = show_objects_ = show_help_ = show_info_ = show_files_ =
        show_menu_ = show_props_ = show_game_confirm_ = false;
    text_input_active_ = false;
}

void LevelEditor::choose_preset(int index, Engine& engine) {
    (void)engine;
    preset_ = index == 0 ? Preset::GemDoor : Preset::KeyDoor;
    mode_ = Mode::Object;
    anchor_valid_ = false;
    show_presets_ = false;
    set_status(index == 0 ? "GEM DOOR: CLICK TO PLACE"
                          : "KEY DOOR: CLICK TO PLACE");
}

void LevelEditor::draw_presets(Framebuffer& fb, Engine& engine) {
    const int x = 20;
    const int y = 20;
    const int w = 280;
    const int h = 110;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 10, y + 8, 15, "PLACE PRESET");

    static const char* const labels[kPresetRows] = {
        "GEM DOOR  (GEM + DOOR TILE)",
        "KEY DOOR  (KEY + TWO DOOR TILES)"};
    for (int i = 0; i < kPresetRows; ++i) {
        char line[64];
        std::snprintf(line, sizeof(line), "%c %s",
                      i == preset_sel_ ? '>' : ' ', labels[i]);
        engine.draw_text(fb, x + 12, y + 30 + i * 22,
                         i == preset_sel_ ? 15 : 10, line);
    }
    const char* current = preset_ == Preset::GemDoor
                              ? "CURRENT: GEM DOOR"
                              : (preset_ == Preset::KeyDoor
                                     ? "CURRENT: KEY DOOR"
                                     : "CURRENT: NONE");
    engine.draw_text(fb, x + 10, y + 66, 14, current);
    engine.draw_text(fb, x + 10, y + h - 10, 10,
                     "UP/DOWN SELECT  ENTER/A SET  ESC/B BACK");
}

void LevelEditor::start_text_prompt(const std::string& title,
                                    const std::string& initial) {
    text_input_active_ = true;
    text_input_title_ = title;
    text_input_buffer_ = initial;
    if (textkey::controller_only()) {
        keyboard_.open(&text_input_buffer_, 255);
    }
}

void LevelEditor::draw_text_prompt(Framebuffer& fb, Engine& engine) {
    if (textkey::controller_only() && keyboard_.active()) {
        const int x = 8;
        const int y = 14;
        const int w = 304;
        const int h = 42 + TextKeyboard::kHeight + 16;
        engine.draw_menu_window(fb, x, y, w, h);
        engine.draw_text(fb, x + 10, y + 8, 15, text_input_title_.c_str());
        fb.fill_rect(x + 10, y + 20, w - 20, 12, 0);
        std::string shown = text_input_buffer_;
        constexpr size_t kMaxShown = 44;
        if (shown.size() > kMaxShown) {
            shown = shown.substr(shown.size() - kMaxShown);
        }
        engine.draw_text(fb, x + 12, y + 23, 15, shown.c_str());
        engine.draw_text(fb, x + 12 + static_cast<int>(shown.size()) * 6,
                         y + 23, 14, "_");
        keyboard_.draw(fb, engine, x + (w - TextKeyboard::kWidth) / 2, y + 40);
        engine.draw_text(fb, x + 10, y + h - 10, 10,
                         "D-PAD MOVE  A TYPE  Y DELETE  B CANCEL");
        return;
    }
    const int x = 28;
    const int y = 58;
    const int w = 264;
    const int h = 84;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 10, y + 10, 15, text_input_title_.c_str());
    fb.fill_rect(x + 10, y + 28, w - 20, 12, 0);
    std::string shown = text_input_buffer_;
    constexpr size_t kMaxShown = 38;
    if (shown.size() > kMaxShown) {
        shown = shown.substr(shown.size() - kMaxShown);
    }
    engine.draw_text(fb, x + 12, y + 31, 15, shown.c_str());
    engine.draw_text(fb, x + 12 + static_cast<int>(shown.size()) * 6,
                     y + 31, 14, "_");
    engine.draw_text(fb, x + 10, y + h - 10, 10,
                     "ENTER SAVE  ESC CANCEL  BACKSPACE DELETE");
}

void LevelEditor::text_input_text(const char* utf8) {
    if (!text_input_active_ || !utf8) {
        return;
    }
    for (const char* p = utf8; *p != '\0' &&
                              text_input_buffer_.size() < 255;
         ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 32 && c < 127) {
            text_input_buffer_.push_back(static_cast<char>(c));
        }
    }
}

void LevelEditor::text_input_backspace() {
    if (text_input_active_ && !text_input_buffer_.empty()) {
        text_input_buffer_.pop_back();
    }
}

void LevelEditor::text_input_commit(Engine& engine) {
    if (!text_input_active_) {
        return;
    }
    if (prop_obj_ >= 0 &&
        prop_obj_ < static_cast<int>(board_.objects.size())) {
        push_undo();
        board_.objects[static_cast<size_t>(prop_obj_)].text =
            text_input_buffer_;
        apply(engine);
    }
    text_input_active_ = false;
    keyboard_.close();
    dirty_ = true;
    set_status("STRING UPDATED");
}

void LevelEditor::text_input_cancel() {
    text_input_active_ = false;
    keyboard_.close();
    set_status("STRING EDIT CANCELLED");
}

void LevelEditor::save_to_game_data(Engine& engine) {
    const std::string name = save_name_.empty() ? board_name_ : save_name_;
    if (name.empty() || engine.game().data_dir.empty()) {
        set_status("NO GAME DATA FOLDER");
        return;
    }
    const std::string path = engine.game().data_dir + "/" + name;
    const jill_fs::path fs_path(path);
    if (!fs_path.parent_path().empty()) {
        std::error_code ec;
        jill_fs::create_directories(fs_path.parent_path(), ec);
    }
    std::string err;
    if (!board_.save(path, err)) {
        std::fprintf(stderr, "editor: game save: %s\n", err.c_str());
        set_status("GAME DATA SAVE FAILED");
        return;
    }
    dirty_ = false;
    set_status("SAVED TO GAME DATA: " + name);
    std::printf("editor: saved game data %s\n", path.c_str());
}

void LevelEditor::draw_game_save_confirm(Framebuffer& fb, Engine& engine) {
    const int x = 20;
    const int y = 58;
    const int w = 280;
    const int h = 84;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 10, y + 10, 15, "OVERWRITE GAME DATA?");
    const std::string name = save_name_.empty() ? board_name_ : save_name_;
    engine.draw_text(fb, x + 10, y + 28, 7, name.c_str());
    engine.draw_text(fb, x + 10, y + 40, 10,
                     "THIS WRITES INTO THE GAME'S DATA FOLDER.");
    engine.draw_text(fb, x + 10, y + 52, 10,
                     "ENTER/A SAVE   ESC/B CANCEL");
    fb.fill_rect(x + 20, y + h - 22, 100, 14, 4);
    engine.draw_text(fb, x + 48, y + h - 19, 15, "SAVE");
    fb.fill_rect(x + w - 120, y + h - 22, 100, 14, 8);
    engine.draw_text(fb, x + w - 92, y + h - 19, 15, "CANCEL");
}

void LevelEditor::scroll_help(int dy) {
    const int max_scroll = std::max(0, kHelpLineCount - kHelpVisible);
    help_scroll_ = clamp_int(help_scroll_ + dy, 0, max_scroll);
}

void LevelEditor::copy_object(Engine& engine) {
    (void)engine;
    int index = selected_obj_;
    if (index < 0 || index >= static_cast<int>(board_.objects.size())) {
        index = object_at(cursor_wx_, cursor_wy_);
    }
    if (index < 0) {
        set_status("NO OBJECT TO COPY");
        return;
    }
    clipboard_ = board_.objects[static_cast<size_t>(index)];
    has_clipboard_ = true;
    set_status("OBJECT COPIED");
}

void LevelEditor::paste_object(Engine& engine) {
    if (!has_clipboard_) {
        set_status("CLIPBOARD IS EMPTY");
        return;
    }
    if (clipboard_.kind == 0 || clipboard_.kind == 3) {
        set_status("THAT OBJECT CANNOT BE PASTED");
        return;
    }
    if (static_cast<int>(board_.objects.size()) >= Board::kMaxObjects) {
        set_status("OBJECT LIMIT REACHED");
        return;
    }
    push_undo();
    BoardObject o = clipboard_;
    o.x = cursor_col_ * 16;
    o.y = cursor_row_ * 16;
    o.prev_x = o.x;
    o.prev_y = o.y;
    o.flags &= 0x3FFF;
    board_.objects.push_back(o);
    selected_obj_ = static_cast<int>(board_.objects.size()) - 1;
    dirty_ = true;
    apply(engine);
    set_status("OBJECT PASTED");
}

int LevelEditor::region_left() const {
    return std::min(region_a_col_, region_b_col_);
}

int LevelEditor::region_top() const {
    return std::min(region_a_row_, region_b_row_);
}

int LevelEditor::region_width() const {
    return std::abs(region_b_col_ - region_a_col_) + 1;
}

int LevelEditor::region_height() const {
    return std::abs(region_b_row_ - region_a_row_) + 1;
}

void LevelEditor::clear_region() {
    region_valid_ = false;
    anchor_valid_ = false;
}

void LevelEditor::copy_tiles() {
    if (!region_valid_) {
        set_status("NO TILE REGION - CLICK A THEN B");
        return;
    }
    tile_clip_w_ = region_width();
    tile_clip_h_ = region_height();
    tile_clip_.clear();
    tile_clip_.reserve(static_cast<size_t>(tile_clip_w_ * tile_clip_h_));
    const int left = region_left();
    const int top = region_top();
    for (int r = 0; r < tile_clip_h_; ++r) {
        for (int c = 0; c < tile_clip_w_; ++c) {
            const int col = left + c;
            const int row = top + r;
            tile_clip_.push_back(
                static_cast<uint16_t>(
                    board_.map[static_cast<size_t>(col) * kBoardRows + row] &
                    0x3FFF));
        }
    }
    char buf[64];
    std::snprintf(buf, sizeof(buf), "TILES COPIED %dx%d", tile_clip_w_,
                  tile_clip_h_);
    set_status(buf);
}

void LevelEditor::paste_tiles(Engine& engine) {
    if (tile_clip_.empty() || tile_clip_w_ <= 0 || tile_clip_h_ <= 0) {
        set_status("TILE CLIPBOARD IS EMPTY");
        return;
    }
    push_undo();
    for (int r = 0; r < tile_clip_h_; ++r) {
        const int row = cursor_row_ + r;
        if (row < 0 || row >= kBoardRows) {
            continue;
        }
        for (int c = 0; c < tile_clip_w_; ++c) {
            const int col = cursor_col_ + c;
            if (col < 0 || col >= kBoardCols) {
                continue;
            }
            board_.map[static_cast<size_t>(col) * kBoardRows + row] =
                tile_clip_[static_cast<size_t>(r * tile_clip_w_ + c)];
        }
    }
    dirty_ = true;
    apply(engine);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "TILES PASTED %dx%d", tile_clip_w_,
                  tile_clip_h_);
    set_status(buf);
}

void LevelEditor::primary_action(Engine& engine) {
    if (mode_ == Mode::Tile) {
        primary_tile_action(engine);
    } else if (mode_ == Mode::Object) {
        push_undo();
        place_object(engine);
    } else if (mode_ == Mode::Region) {
        if (anchor_valid_) {
            region_a_col_ = anchor_col_;
            region_a_row_ = anchor_row_;
            region_b_col_ = cursor_col_;
            region_b_row_ = cursor_row_;
            region_valid_ = true;
            anchor_valid_ = false;
            set_status("TILE REGION SELECTED - C COPY  V PASTE");
        } else {
            anchor_valid_ = true;
            anchor_col_ = cursor_col_;
            anchor_row_ = cursor_row_;
            region_valid_ = false;
            set_status("REGION A SET - CLICK B");
        }
    } else {
        const int index = object_at(cursor_wx_, cursor_wy_);
        if (index >= 0) {
            if (selected_obj_ == index) {
                // Pressing the button again while the cursor is still over the
                // highlighted object lets go of it instead of re-selecting.
                selected_obj_ = -1;
                set_status("SELECTION CLEARED");
            } else {
                selected_obj_ = index;
                char buf[64];
                std::snprintf(buf, sizeof(buf), "SELECTED OBJECT %d KIND %d",
                              index,
                              board_.objects[static_cast<size_t>(index)].kind);
                set_status(buf);
            }
        } else {
            selected_obj_ = -1;
            set_status("SELECTION CLEARED");
        }
    }
}

void LevelEditor::erase_action(Engine& engine) {
    if (mode_ == Mode::Tile) {
        push_undo();
        paint_brush(engine, true);
    } else if (mode_ == Mode::Region) {
        clear_region();
        set_status("REGION CLEARED");
    } else if (selected_obj_ >= 0) {
        push_undo();
        delete_object_at(selected_obj_, engine);
    } else {
        const int index = object_at(cursor_wx_, cursor_wy_);
        if (index >= 0) {
            push_undo();
        }
        delete_object_at(index, engine);
    }
}

void LevelEditor::pick_action(Engine& engine) {
    if (mode_ == Mode::Tile) {
        pick_tile(engine);
    } else {
        pick_object_kind(engine);
    }
}

void LevelEditor::apply(Engine& engine) {
    mini_dirty_ = true;
    if (!engine.apply_editor_board(board_)) {
        set_status("BOARD APPLY FAILED");
        return;
    }
}

void LevelEditor::push_undo() {
    undo_coalesce_ = 0;
    undo_.push_back(board_);
    if (static_cast<int>(undo_.size()) > kUndoLimit) {
        undo_.erase(undo_.begin());
    }
    redo_.clear();
}

void LevelEditor::push_undo_coalesced(uint64_t token) {
    if (token != 0 && token == undo_coalesce_) {
        return;
    }
    undo_coalesce_ = token;
    undo_.push_back(board_);
    if (static_cast<int>(undo_.size()) > kUndoLimit) {
        undo_.erase(undo_.begin());
    }
    redo_.clear();
}

void LevelEditor::undo(Engine& engine) {
    if (undo_.empty()) {
        set_status("NOTHING TO UNDO");
        return;
    }
    redo_.push_back(board_);
    if (static_cast<int>(redo_.size()) > kUndoLimit) {
        redo_.erase(redo_.begin());
    }
    board_ = undo_.back();
    undo_.pop_back();
    selected_obj_ = -1;
    prop_obj_ = -1;
    anchor_valid_ = false;
    show_props_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    undo_coalesce_ = 0;
    text_input_active_ = false;
    dirty_ = true;
    apply(engine);
    engine.finish_dialogs();
    engine.close_message_box();
    set_status("UNDO");
}

void LevelEditor::redo(Engine& engine) {
    if (redo_.empty()) {
        set_status("NOTHING TO REDO");
        return;
    }
    undo_.push_back(board_);
    if (static_cast<int>(undo_.size()) > kUndoLimit) {
        undo_.erase(undo_.begin());
    }
    board_ = redo_.back();
    redo_.pop_back();
    selected_obj_ = -1;
    prop_obj_ = -1;
    anchor_valid_ = false;
    show_props_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    undo_coalesce_ = 0;
    text_input_active_ = false;
    dirty_ = true;
    apply(engine);
    engine.finish_dialogs();
    engine.close_message_box();
    set_status("REDO");
}

void LevelEditor::clear_history() {
    undo_.clear();
    redo_.clear();
}

void LevelEditor::set_status(std::string text) {
    status_ = shortened(text, 52);
}

std::string LevelEditor::editor_dir() const {
    if (game_id_.empty()) {
        return std::string();
    }
    return jill_paths::resolve("editor/" + game_id_);
}

std::string LevelEditor::editor_path(const std::string& name) const {
    const std::string dir = editor_dir();
    if (dir.empty() || name.empty()) {
        return std::string();
    }
    return dir + "/" + name;
}

std::string LevelEditor::original_path(const Engine& engine) const {
    if (board_name_.empty()) {
        return std::string();
    }
    return jill_paths::resolve_file(engine.game().data_dir + "/" + board_name_);
}

std::string LevelEditor::unique_level_name(Engine& engine) const {
    std::string ext = engine.game().board_ext;
    if (ext.empty() && !board_name_.empty()) {
        const size_t dot = board_name_.rfind('.');
        if (dot != std::string::npos) {
            ext = board_name_.substr(dot);
        }
    }
    if (ext.empty()) {
        ext = ".jn1";
    }
    for (int i = 1; i < 1000; ++i) {
        const std::string name = "custom_" + std::to_string(i) + ext;
        if (name != save_name_ && !jill_paths::exists(editor_path(name))) {
            return name;
        }
    }
    return "custom" + ext;
}

std::string LevelEditor::save_path() const {
    if (game_id_.empty()) {
        return std::string();
    }
    const std::string name = save_name_.empty() ? board_name_ : save_name_;
    if (name.empty()) {
        return std::string();
    }
    return editor_path(name);
}

bool LevelEditor::load_board_path(const std::string& path, Engine& engine,
                                  std::string& err) {
    if (path.empty()) {
        err = "no board file selected";
        return false;
    }
    Board loaded;
    if (!loaded.load(path, err)) {
        return false;
    }
    if (loaded.objects.empty()) {
        err = path + ": board has no objects";
        return false;
    }
    board_ = std::move(loaded);
    dirty_ = false;
    selected_obj_ = -1;
    anchor_valid_ = false;
    left_down_ = false;
    right_down_ = false;
    dragging_ = false;
    last_paint_valid_ = false;
    show_props_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    show_game_confirm_ = false;
    text_input_active_ = false;
    region_valid_ = false;
    undo_coalesce_ = 0;
    clear_history();
    if (!engine.apply_editor_board(board_)) {
        err = "could not apply " + path;
        return false;
    }
    engine.finish_dialogs();
    engine.close_message_box();
    engine.close_text_page();
    if (!board_.objects.empty()) {
        const BoardObject& o = board_.objects[0];
        cam_x_ = clamp_int(o.x - Framebuffer::W / 2, 0,
                           Board::kCols * 16 - Framebuffer::W);
        cam_y_ = clamp_int(o.y - Framebuffer::H / 2, 0,
                           Board::kRows * 16 - Framebuffer::H);
        cursor_col_ = clamp_int((cam_x_ + Framebuffer::W / 2) >> 4, 0,
                                kBoardCols - 1);
        cursor_row_ = clamp_int((cam_y_ + Framebuffer::H / 2) >> 4, 0,
                                kBoardRows - 1);
        cursor_wx_ = cursor_col_ * 16 + 8;
        cursor_wy_ = cursor_row_ * 16 + 8;
    }
    return true;
}

int LevelEditor::file_list_start() const {
    constexpr int kRowsShown = 10;
    if (file_sel_ < kRowsShown) {
        return 0;
    }
    return std::min(file_sel_ - kRowsShown + 1,
                    std::max(0, static_cast<int>(file_rows_.size()) - kRowsShown));
}

void LevelEditor::refresh_file_rows(Engine& engine) {
    file_rows_.clear();
    if (file_mode_ == FileMode::Load) {
        FileRow original;
        original.label = "ORIGINAL " + board_name_;
        original.name = board_name_;
        original.original = true;
        file_rows_.push_back(original);
    }

    std::vector<std::string> names;
    const std::string dir = editor_dir();
    std::error_code dir_ec;
    if (!dir.empty() &&
        jill_fs::is_directory(jill_fs::path(dir), dir_ec)) {
        std::error_code ec;
        for (const auto& entry :
             jill_fs::directory_iterator(jill_fs::path(dir), ec)) {
            if (ec) {
                break;
            }
            std::error_code fec;
            if (!entry.is_regular_file(fec)) {
                continue;
            }
            const std::string name = entry.path().filename().string();
            if (!engine.game().board_ext.empty() &&
                !ends_with_ci(name, engine.game().board_ext)) {
                continue;
            }
            names.push_back(name);
        }
        if (ec) {
            std::fprintf(stderr, "editor: cannot list %s\n", dir.c_str());
        }
    }
    std::sort(names.begin(), names.end());
    if (file_mode_ == FileMode::Save && !save_name_.empty()) {
        bool found = false;
        for (const std::string& name : names) {
            if (name == save_name_) {
                found = true;
                break;
            }
        }
        if (!found) {
            FileRow current;
            current.label = "CURRENT " + save_name_;
            current.name = save_name_;
            file_rows_.push_back(current);
        }
    }
    for (const std::string& name : names) {
        FileRow row;
        row.label = name;
        row.name = name;
        file_rows_.push_back(row);
    }
    if (file_mode_ == FileMode::Save) {
        const std::string new_name = unique_level_name(engine);
        FileRow row;
        row.label = "NEW FILE " + new_name;
        row.name = new_name;
        row.new_file = true;
        file_rows_.push_back(row);
    }

    file_sel_ = 0;
    for (int i = 0; i < static_cast<int>(file_rows_.size()); ++i) {
        if (!file_rows_[static_cast<size_t>(i)].new_file &&
            !file_rows_[static_cast<size_t>(i)].original &&
            file_rows_[static_cast<size_t>(i)].name == save_name_) {
            file_sel_ = i;
            break;
        }
    }
}

void LevelEditor::open_file_picker(FileMode mode, Engine& engine) {
    file_mode_ = mode;
    show_files_ = true;
    show_tiles_ = false;
    show_objects_ = false;
    show_info_ = false;
    show_help_ = false;
    show_menu_ = false;
    show_presets_ = false;
    preset_ = Preset::None;
    refresh_file_rows(engine);
}

void LevelEditor::choose_file(Engine& engine) {
    if (file_sel_ < 0 ||
        file_sel_ >= static_cast<int>(file_rows_.size())) {
        return;
    }
    const FileRow row = file_rows_[static_cast<size_t>(file_sel_)];
    if (file_mode_ == FileMode::Save) {
        save_name_ = row.name;
        if (save(engine)) {
            show_files_ = false;
        } else {
            refresh_file_rows(engine);
        }
        return;
    }
    const std::string path =
        row.original ? original_path(engine) : editor_path(row.name);
    std::string err;
    if (!load_board_path(path, engine, err)) {
        std::fprintf(stderr, "editor: %s\n", err.c_str());
        set_status(row.original ? "LOAD ORIGINAL FAILED" : "LOAD FAILED");
        return;
    }
    save_name_ = row.original ? board_name_ : row.name;
    show_files_ = false;
    set_status(row.original ? "LOADED ORIGINAL BOARD"
                            : "LOADED " + row.name);
}

void LevelEditor::draw_file_picker(Framebuffer& fb, Engine& engine) {
    const int x = 4;
    const int y = 16;
    const int w = 312;
    const int h = 172;
    engine.draw_menu_window(fb, x, y, w, h);
    const char* title =
        file_mode_ == FileMode::Save ? "SAVE LEVEL AS" : "LOAD LEVEL";
    engine.draw_text(fb, x + 8, y + 8, 15, title);
    const std::string folder = "editor/" + game_id_ + "/";
    engine.draw_text(fb, x + 8, y + 17, 10, folder.c_str());

    const int start = file_list_start();
    const int line_h = 12;
    for (int i = start; i < static_cast<int>(file_rows_.size()) &&
                        i < start + 10;
         ++i) {
        const FileRow& row = file_rows_[static_cast<size_t>(i)];
        const int ry = y + 28 + (i - start) * line_h;
        std::string line = row.label;
        if (!row.new_file && !row.original && row.name == save_name_) {
            line += " *";
        }
        char text[64];
        std::snprintf(text, sizeof(text), "%c %s",
                      i == file_sel_ ? '>' : ' ', line.c_str());
        engine.draw_text(fb, x + 10, ry, i == file_sel_ ? 15 : 10,
                         shortened(text, 47).c_str());
    }
    engine.draw_text(fb, x + 8, y + h - 9, 10,
                     "UP/DOWN SELECT  ENTER CHOOSE  ESC BACK");
}

int LevelEditor::tile_at(int col, int row) const {
    if (col < 0 || col >= kBoardCols || row < 0 || row >= kBoardRows) {
        return 0;
    }
    return board_.map[static_cast<size_t>(col) * kBoardRows + row] & 0x3FFF;
}

int LevelEditor::object_at(int wx, int wy) const {
    for (int i = static_cast<int>(board_.objects.size()) - 1; i >= 0; --i) {
        const BoardObject& o = board_.objects[static_cast<size_t>(i)];
        const int w = std::max(0, o.w);
        const int h = std::max(0, o.h);
        bool hit = w > 0 && h > 0 && wx >= o.x && wx < o.x + w &&
                   wy >= o.y && wy < o.y + h;
        if (!hit) {
            // Small/zero-sized objects (TEXT8, BRIDGER, markers) are hard to
            // hit with the pointer; let their anchor cell select them too.
            const int gx = (o.x >> 4) * 16;
            const int gy = (o.y >> 4) * 16;
            hit = wx >= gx && wx < gx + 16 && wy >= gy && wy < gy + 16;
        }
        if (hit) {
            return i;
        }
    }
    return -1;
}

void LevelEditor::paint_brush(Engine& engine, bool erase) {
    const int c = cursor_col_;
    const int r = cursor_row_;
    if (c < 0 || c >= kBoardCols || r < 0 || r >= kBoardRows) {
        return;
    }
    int c0 = c;
    int r0 = r;
    if (last_paint_valid_) {
        c0 = last_paint_col_;
        r0 = last_paint_row_;
    }
    const uint16_t id =
        static_cast<uint16_t>((erase ? erase_tile_ : paint_tile_) & 0x3FFF);
    int x = c0;
    int y = r0;
    const int dx = std::abs(c - c0);
    const int sx = c0 < c ? 1 : -1;
    const int dy = -std::abs(r - r0);
    const int sy = r0 < r ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        board_.map[static_cast<size_t>(x) * kBoardRows + y] = id;
        if (x == c && y == r) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
    last_paint_col_ = c;
    last_paint_row_ = r;
    last_paint_valid_ = true;
    dirty_ = true;
    apply(engine);
}

void LevelEditor::place_object(Engine& engine) {
    if (preset_ != Preset::None) {
        place_preset(engine);
        return;
    }
    const int col = cursor_col_;
    const int row = cursor_row_;
    if (col < 0 || col >= kBoardCols || row < 0 || row >= kBoardRows) {
        return;
    }
    if (brush_kind_ <= 0 || brush_kind_ == 3) {
        set_status("THAT KIND IS NOT PLACEABLE");
        return;
    }
    if (static_cast<int>(board_.objects.size()) >= Board::kMaxObjects) {
        set_status("OBJECT LIMIT REACHED");
        return;
    }
    BoardObject o;
    o.kind = brush_kind_;
    o.x = col * 16;
    o.y = row * 16;
    const KindInfo& k = engine.profile().kind(brush_kind_);
    o.w = k.width > 0 ? k.width : 16;
    o.h = k.height > 0 ? k.height : 16;
    o.prev_x = o.x;
    o.prev_y = o.y;
    board_.objects.push_back(o);
    selected_obj_ = static_cast<int>(board_.objects.size()) - 1;
    dirty_ = true;
    apply(engine);
    char buf[64];
    std::snprintf(buf, sizeof(buf), "PLACED %03d %s", brush_kind_,
                  engine.profile().kind(brush_kind_).name);
    set_status(buf);
}

void LevelEditor::place_preset(Engine& engine) {
    const int col = cursor_col_;
    const int row = cursor_row_;
    if (col < 0 || col >= kBoardCols || row < 0 || row >= kBoardRows ||
        preset_ == Preset::None) {
        return;
    }
    if (static_cast<int>(board_.objects.size()) >= Board::kMaxObjects) {
        set_status("OBJECT LIMIT REACHED");
        return;
    }
    const bool gem = preset_ == Preset::GemDoor;
    uint16_t* cell =
        &board_.map[static_cast<size_t>(col) * kBoardRows + row];
    if (gem) {
        // Gem doors are DOOR objects standing on the 16x16 MAPDOOR tile.
        *cell = 0x0BE;
    } else {
        // Key doors use the two stacked 0x0A1/0x0A2 door tiles.
        *cell = 0x0A1;
        if (row + 1 < kBoardRows) {
            board_.map[static_cast<size_t>(col) * kBoardRows + row + 1] =
                0x0A2;
        }
    }
    BoardObject o;
    o.kind = 24;
    o.x = col * 16;
    o.y = row * 16;
    // Gem doors restore the tile at x+dx/y+dy when they open; default to the
    // floor cell below the door.  Key doors ignore dx/dy.
    o.dx = 0;
    o.dy = 1;
    const KindInfo& k = engine.profile().kind(24);
    o.w = k.width > 0 ? k.width : 16;
    o.h = k.height > 0 ? k.height : 24;
    o.prev_x = o.x;
    o.prev_y = o.y;
    board_.objects.push_back(o);
    selected_obj_ = static_cast<int>(board_.objects.size()) - 1;
    dirty_ = true;
    apply(engine);
    set_status(gem ? "GEM DOOR PLACED" : "KEY DOOR PLACED");
}

void LevelEditor::delete_object_at(int index, Engine& engine) {
    if (index < 0) {
        set_status("NO OBJECT AT CURSOR");
        return;
    }
    if (index == 0) {
        set_status("THE PLAYER OBJECT CANNOT BE DELETED");
        return;
    }
    if (board_.objects.size() <= 1) {
        set_status("A BOARD NEEDS AT LEAST ONE OBJECT");
        return;
    }
    board_.objects.erase(board_.objects.begin() + index);
    selected_obj_ = -1;
    dirty_ = true;
    apply(engine);
    set_status("OBJECT DELETED");
}

void LevelEditor::drag_selected(Engine& engine) {
    if (drag_obj_ < 0 ||
        drag_obj_ >= static_cast<int>(board_.objects.size())) {
        return;
    }
    const int wx = cursor_wx_;
    const int wy = cursor_wy_;
    BoardObject& o = board_.objects[static_cast<size_t>(drag_obj_)];
    const int w = o.w > 0 ? o.w : 8;
    const int h = o.h > 0 ? o.h : 8;
    o.x = clamp_int(o.x + (wx - drag_last_wx_), 0, Board::kCols * 16 - w);
    o.y = clamp_int(o.y + (wy - drag_last_wy_), 0, Board::kRows * 16 - h);
    drag_last_wx_ = wx;
    drag_last_wy_ = wy;
    dirty_ = true;
    apply(engine);
}

void LevelEditor::pick_tile(Engine& engine) {
    paint_tile_ = tile_at(cursor_col_, cursor_row_);
    tile_page_ = (paint_tile_ / kTilePalCols) * kTilePalCols;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "PICKED TILE %03X %s", paint_tile_,
                  engine.tile(paint_tile_).name.c_str());
    set_status(buf);
}

void LevelEditor::pick_object_kind(Engine& engine) {
    const int index = object_at(cursor_wx_, cursor_wy_);
    if (index < 0) {
        set_status("NO OBJECT AT CURSOR");
        return;
    }
    const int kind = board_.objects[static_cast<size_t>(index)].kind;
    if (kind <= 0 || kind == 3) {
        set_status("THAT KIND IS NOT PLACEABLE");
        return;
    }
    brush_kind_ = kind;
    preset_ = Preset::None;
    kind_page_ = (kind / kKindPalPage) * kKindPalPage;
    mode_ = Mode::Object;
    char buf[64];
    std::snprintf(buf, sizeof(buf), "PICKED OBJECT %03d %s", kind,
                  engine.profile().kind(kind).name);
    set_status(buf);
}

void LevelEditor::start_playtest(Engine& engine) {
    engine.finish_dialogs();
    engine.close_message_box();
    engine.close_text_page();
    if (!engine.apply_editor_board(board_)) {
        set_status("PLAYTEST FAILED");
        return;
    }
    engine.set_input(0, 0, false, false);
    // With the Classic HUD the in-game view is 232x160 at (80,16), not the
    // editor's 320x200 canvas, so hand the playtest a camera centred on the
    // player: the game's own view then opens where the editor expects it
    // instead of showing a different slice of the board.
    if (engine.hud_classic() && !engine.hud_hidden()) {
        const auto& objs = engine.objects();
        if (!objs.empty()) {
            const BoardObject& p = objs[0];
            const int vw = engine.board_view_w();
            const int vh = engine.board_view_h();
            cam_x_ = clamp_int(p.x + 8 - vw / 2, 0, Board::kCols * 16 - vw);
            cam_y_ = clamp_int(p.y + 16 - vh / 2, 0, Board::kRows * 16 - vh);
        }
    }
    engine.set_camera(cam_x_, cam_y_);
    playtest_ = true;
    set_status("PLAYTEST - F5 OR ESC RETURNS TO THE EDITOR");
}

void LevelEditor::start_playtest_at_cursor(Engine& engine) {
    int idx = -1;
    for (int i = 0; i < static_cast<int>(board_.objects.size()); ++i) {
        if (board_.objects[static_cast<size_t>(i)].kind == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        start_playtest(engine);
        return;
    }
    const BoardObject saved = board_.objects[static_cast<size_t>(idx)];
    BoardObject temp = saved;
    const int w = temp.w > 0 ? temp.w : 16;
    const int h = temp.h > 0 ? temp.h : 32;
    temp.x = clamp_int(cursor_col_ * 16 + (16 - w) / 2, 0,
                       Board::kCols * 16 - w);
    temp.y = clamp_int(cursor_row_ * 16 + 16 - h, 0, Board::kRows * 16 - h);
    temp.prev_x = temp.x;
    temp.prev_y = temp.y;
    board_.objects[static_cast<size_t>(idx)] = temp;
    start_playtest(engine);
    board_.objects[static_cast<size_t>(idx)] = saved;
}

void LevelEditor::stop_playtest(Engine& engine) {
    playtest_ = false;
    if (!engine.apply_editor_board(board_)) {
        set_status("RETURN TO EDITOR FAILED");
        return;
    }
    engine.finish_dialogs();
    engine.close_message_box();
    engine.close_text_page();
    set_status("BACK IN THE EDITOR");
}

bool LevelEditor::save(Engine& engine) {
    (void)engine;
    const std::string path = save_path();
    if (path.empty()) {
        set_status("NO BOARD NAME TO SAVE");
        return false;
    }
    const jill_fs::path fs_path(path);
    if (!fs_path.parent_path().empty()) {
        std::error_code ec;
        jill_fs::create_directories(fs_path.parent_path(), ec);
        if (ec) {
            std::fprintf(stderr, "editor: cannot create %s\n",
                         fs_path.parent_path().generic_string().c_str());
            set_status("SAVE FAILED: CANNOT CREATE FOLDER");
            return false;
        }
    }
    std::string err;
    if (!board_.save(path, err)) {
        std::fprintf(stderr, "editor: %s\n", err.c_str());
        set_status("SAVE FAILED");
        return false;
    }
    dirty_ = false;
    const std::string name = save_name_.empty() ? board_name_ : save_name_;
    set_status("SAVED editor/" + game_id_ + "/" + name);
    std::printf("editor: saved %s\n", path.c_str());
    return true;
}

void LevelEditor::load_saved(Engine& engine) {
    const std::string path = save_path();
    if (path.empty()) {
        set_status("NO BOARD NAME TO LOAD");
        return;
    }
    std::string err;
    if (!load_board_path(path, engine, err)) {
        std::fprintf(stderr, "editor: %s\n", err.c_str());
        set_status("NO SAVED EDIT FOR THIS LEVEL");
        return;
    }
    const std::string name = save_name_.empty() ? board_name_ : save_name_;
    set_status("LOADED " + name);
}

void LevelEditor::adjust_info(int delta, Engine& engine) {
    // Repeated tweaks of the same field share one undo entry.
    push_undo_coalesced(0x1000000ull + static_cast<uint64_t>(info_sel_));
    uint32_t value32 = 0;
    int lo = 0;
    int hi = 0;
    switch (info_sel_) {
        case 0:
            lo = 0;
            hi = 0xFFFF;
            break;
        case 1:
            lo = 0;
            hi = 999;
            break;
        case 2:
            lo = 0;
            hi = 16;
            break;
        case 3:
            lo = 0;
            hi = 999999999;
            break;
        case 4:
            lo = 0;
            hi = 255;
            break;
        default:
            lo = 0;
            hi = 999999999;
            break;
    }
    switch (info_sel_) {
        case 0: {
            int v = rd16(&board_.header[0]) + delta;
            wr16(&board_.header[0], clamp_int(v, lo, hi));
            break;
        }
        case 1: {
            int v = rd16(&board_.header[2]) + delta;
            wr16(&board_.header[2], clamp_int(v, lo, hi));
            break;
        }
        case 2: {
            int v = rd16(&board_.header[4]) + delta;
            wr16(&board_.header[4], clamp_int(v, lo, hi));
            break;
        }
        case 3: {
            value32 = rd32(&board_.header[38]);
            value32 = static_cast<uint32_t>(
                clamp_int(static_cast<int>(value32) + delta, lo, hi));
            wr32(&board_.header[38], value32);
            break;
        }
        case 4: {
            int v = rd16(&board_.header[42]) + delta;
            wr16(&board_.header[42], clamp_int(v, lo, hi));
            break;
        }
        default: {
            value32 = rd32(&board_.header[44]);
            value32 = static_cast<uint32_t>(
                clamp_int(static_cast<int>(value32) + delta, lo, hi));
            wr32(&board_.header[44], value32);
            break;
        }
    }
    dirty_ = true;
    apply(engine);
    set_status("BOARD HEADER CHANGED");
}

void LevelEditor::page_palettes(int delta) {
    if (show_tiles_) {
        tile_page_ += delta * kTilePalPage;
        tile_page_ = clamp_int(tile_page_, 0, kTileCount - 1);
        tile_page_ = (tile_page_ / kTilePalCols) * kTilePalCols;
    } else if (show_objects_) {
        kind_page_ += delta * kKindPalPage;
        kind_page_ = clamp_int(kind_page_, 0, kind_count_ - 1);
        kind_page_ = (kind_page_ / kKindPalPage) * kKindPalPage;
    }
}

void LevelEditor::render(Framebuffer& fb, Engine& engine, float alpha) {
    if (!open_) {
        return;
    }
    if (playtest_) {
        engine.render(fb, true, alpha);
        fb.fill_rect(0, 0, 166, 10, 0);
        engine.draw_text(fb, 2, 1, 15, "PLAYTEST  F5 EDIT");
        return;
    }

    // The canvas is the editor's own coordinate space (grid, cursor, bars all
    // use it), so render the board full screen even when the in-game HUD is
    // Classic: that mode offsets the view by (view_x, view_y) and clips it to
    // 232x160, which used to shove the board out of step with the editor.
    engine.render_at_fullscreen(fb, cam_x_, cam_y_);
    draw_board_overlays(fb, engine);
    draw_bars(fb, engine);
    draw_minimap(fb, engine);
    if (show_help_) {
        draw_help(fb, engine);
    } else if (show_tiles_) {
        draw_tile_palette(fb, engine);
    } else if (show_objects_) {
        draw_kind_palette(fb, engine);
    } else if (show_info_) {
        draw_info(fb, engine);
    } else if (show_files_) {
        draw_file_picker(fb, engine);
    } else if (show_menu_) {
        draw_editor_menu(fb, engine);
    } else if (show_props_) {
        draw_object_props(fb, engine);
    } else if (show_presets_) {
        draw_presets(fb, engine);
    } else if (show_game_confirm_) {
        draw_game_save_confirm(fb, engine);
    }
    if (text_input_active_) {
        draw_text_prompt(fb, engine);
    }
    draw_touch_bar(fb, engine);
}

void LevelEditor::draw_bars(Framebuffer& fb, Engine& engine) {
    fb.fill_rect(0, 0, Framebuffer::W, kTopBar, 0);
    fb.fill_rect(0, kTopBar - 1, Framebuffer::W, 1, 8);
    fb.fill_rect(0, kBottomBar, Framebuffer::W, Framebuffer::H - kBottomBar,
                 0);
    fb.fill_rect(0, kBottomBar, Framebuffer::W, 1, 8);

    char top[96];
    if (mode_ == Mode::Tile) {
        const char* tool = paint_mode_ == PaintMode::Single
                               ? "1"
                               : (paint_mode_ == PaintMode::Line
                                      ? "LINE"
                                      : (paint_mode_ == PaintMode::Rect
                                             ? "RECT"
                                             : "FILL"));
        std::snprintf(top, sizeof(top), "JILL MAKER%s  TILE-%s  %03X %s",
                      dirty_ ? "*" : "", tool, paint_tile_,
                      engine.tile(paint_tile_).name.c_str());
    } else if (mode_ == Mode::Object) {
        if (preset_ != Preset::None) {
            std::snprintf(top, sizeof(top), "JILL MAKER%s  PRESET  %s",
                          dirty_ ? "*" : "",
                          preset_ == Preset::GemDoor ? "GEM DOOR"
                                                     : "KEY DOOR");
        } else {
            std::snprintf(top, sizeof(top),
                          "JILL MAKER%s  OBJECT  %03d %s", dirty_ ? "*" : "",
                          brush_kind_, engine.profile().kind(brush_kind_).name);
        }
    } else if (mode_ == Mode::Select) {
        std::snprintf(top, sizeof(top), "JILL MAKER%s  SELECT  OBJ %d",
                      dirty_ ? "*" : "", selected_obj_);
    } else {
        if (region_valid_) {
            char buf[96];
            std::snprintf(buf, sizeof(buf),
                          "JILL MAKER%s  REGION %dx%d  CLIP %dx%d",
                          dirty_ ? "*" : "", region_width(), region_height(),
                          tile_clip_w_, tile_clip_h_);
            std::strncpy(top, buf, sizeof(top) - 1);
            top[sizeof(top) - 1] = '\0';
        } else {
            std::snprintf(top, sizeof(top),
                          "JILL MAKER%s  REGION  CLICK A THEN B",
                          dirty_ ? "*" : "");
        }
    }
    engine.draw_text(fb, 2, 2, 15, top);
    if (top[0] != '\0') {
        char cam[32];
        std::snprintf(cam, sizeof(cam), "%d,%d", cam_x_, cam_y_);
        engine.draw_text(fb, Framebuffer::W - 2 -
                                static_cast<int>(std::strlen(cam)) * 6,
                         2, 10, cam);
    }

    const char* hint = status_.c_str();
    if (status_.empty()) {
        if (mode_ == Mode::Tile) {
            hint = "LMB/SPACE PAINT  RMB/DEL ERASE  P PICK  H HELP";
        } else if (mode_ == Mode::Object) {
            hint = "LMB/A PLACE  RMB/X DELETE  P PICK  E OBJECTS";
        } else if (mode_ == Mode::Region) {
            hint = "CLICK A/B  RMB CLEAR  C COPY TILES  V PASTE";
        } else {
            hint = "LMB/A SELECT  ARROWS MOVE  RMB/X DELETE  ESC CLEAR";
        }
    }
    engine.draw_text(fb, 2, kBottomBar + 2, 7, hint);
}

void LevelEditor::draw_board_overlays(Framebuffer& fb, Engine& engine) {
    if (show_grid_) {
        const int c0 = cam_x_ / 16;
        const int c1 = (cam_x_ + Framebuffer::W) / 16;
        const int r0 = cam_y_ / 16;
        const int r1 = (cam_y_ + Framebuffer::H) / 16;
        for (int c = c0; c <= c1; ++c) {
            const int x = c * 16 - cam_x_;
            if (x >= 0 && x < Framebuffer::W) {
                fb.fill_rect(x, kTopBar, 1, kBottomBar - kTopBar, 8);
            }
        }
        for (int r = r0; r <= r1; ++r) {
            const int y = r * 16 - cam_y_;
            if (y >= kTopBar && y < kBottomBar) {
                fb.fill_rect(0, y, Framebuffer::W, 1, 8);
            }
        }
    }

    if (mode_ != Mode::Tile) {
        for (int i = 0; i < static_cast<int>(board_.objects.size()); ++i) {
            const BoardObject& o = board_.objects[static_cast<size_t>(i)];
            const int w = o.w > 0 ? o.w : 8;
            const int h = o.h > 0 ? o.h : 8;
            const int sx = o.x - cam_x_;
            const int sy = o.y - cam_y_;
            if (sx + w < 0 || sx >= Framebuffer::W || sy + h < kTopBar ||
                sy >= kBottomBar) {
                continue;
            }
            uint8_t colour = 12;
            if (i == selected_obj_) {
                colour = 15;
            } else if (o.kind == 0) {
                colour = 14;
            }
            outline(fb, sx, sy, w, h, colour);
        }
    }

    if (mode_ == Mode::Tile && paint_mode_ == PaintMode::Line &&
        anchor_valid_) {
        const TileInfo& t = engine.tile(paint_tile_);
        int x = anchor_col_;
        int y = anchor_row_;
        const int dx = std::abs(cursor_col_ - anchor_col_);
        const int sx = anchor_col_ < cursor_col_ ? 1 : -1;
        const int dy = -std::abs(cursor_row_ - anchor_row_);
        const int sy = anchor_row_ < cursor_row_ ? 1 : -1;
        int err = dx + dy;
        for (;;) {
            engine.draw_shape(fb, t.shape, x * 16 - cam_x_,
                              y * 16 - cam_y_, true);
            if (x == cursor_col_ && y == cursor_row_) {
                break;
            }
            const int e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y += sy;
            }
        }
        outline(fb, anchor_col_ * 16 - cam_x_, anchor_row_ * 16 - cam_y_, 17,
                17, 14);
    } else if (mode_ == Mode::Tile && paint_mode_ == PaintMode::Rect &&
               anchor_valid_) {
        const int x0 = std::min(anchor_col_, cursor_col_) * 16 - cam_x_;
        const int y0 = std::min(anchor_row_, cursor_row_) * 16 - cam_y_;
        const int w = (std::abs(cursor_col_ - anchor_col_) + 1) * 16;
        const int h = (std::abs(cursor_row_ - anchor_row_) + 1) * 16;
        outline(fb, x0, y0, w, h, 15);
        outline(fb, anchor_col_ * 16 - cam_x_, anchor_row_ * 16 - cam_y_, 17,
                17, 14);
    } else if (mode_ == Mode::Region) {
        if (region_valid_ || anchor_valid_) {
            const int c0 = anchor_valid_ ? anchor_col_ : region_left();
            const int r0 = anchor_valid_ ? anchor_row_ : region_top();
            const int c1 = anchor_valid_ ? cursor_col_
                                         : region_left() + region_width() - 1;
            const int r1 = anchor_valid_ ? cursor_row_
                                         : region_top() + region_height() - 1;
            outline(fb, std::min(c0, c1) * 16 - cam_x_,
                    std::min(r0, r1) * 16 - cam_y_,
                    (std::abs(c1 - c0) + 1) * 16,
                    (std::abs(r1 - r0) + 1) * 16, 14);
        }
        if (!tile_clip_.empty()) {
            for (int r = 0; r < tile_clip_h_; ++r) {
                for (int c = 0; c < tile_clip_w_; ++c) {
                    const int id = tile_clip_[static_cast<size_t>(
                        r * tile_clip_w_ + c)];
                    if (id < 0 || id >= kTileCount) {
                        continue;
                    }
                    engine.draw_shape(fb, engine.tile(id).shape,
                                      (cursor_col_ + c) * 16 - cam_x_,
                                      (cursor_row_ + r) * 16 - cam_y_, true);
                }
            }
        }
    }

    const int col = cursor_col_;
    const int row = cursor_row_;
    if (col < 0 || col >= kBoardCols || row < 0 || row >= kBoardRows) {
        return;
    }
    const int sx = col * 16 - cam_x_;
    const int sy = row * 16 - cam_y_;
    if (mode_ == Mode::Tile && !right_down_) {
        const TileInfo& t = engine.tile(paint_tile_);
        engine.draw_shape(fb, t.shape, sx, sy, true);
    } else if (mode_ == Mode::Object) {
        const int shape = engine.kind_icon_shape(brush_kind_);
        if (shape >= 0) {
            engine.draw_shape(fb, shape, sx, sy, true);
        }
        const KindInfo& k = engine.profile().kind(brush_kind_);
        outline(fb, sx, sy, k.width > 0 ? k.width : 16,
                k.height > 0 ? k.height : 16, 13);
    }
    outline(fb, sx, sy, 17, 17, 15);
}

uint8_t LevelEditor::minimap_tile_colour(Engine& engine, int tile_id) {
    const int id = clamp_int(tile_id, 0, kTileCount - 1);
    if (!tile_colour_ready_[static_cast<size_t>(id)]) {
        uint8_t colour = 8;
        std::vector<uint8_t> px;
        int w = 0;
        int h = 0;
        if (engine.tile_pixels(id, px, w, h) && w > 0 && h > 0 &&
            px.size() >= static_cast<size_t>(w) * static_cast<size_t>(h)) {
            colour = px[static_cast<size_t>(h / 2) * static_cast<size_t>(w) +
                        static_cast<size_t>(w / 2)];
        }
        tile_colour_[static_cast<size_t>(id)] = colour;
        tile_colour_ready_[static_cast<size_t>(id)] = true;
    }
    return tile_colour_[static_cast<size_t>(id)];
}

void LevelEditor::rebuild_minimap(Engine& engine) {
    for (int col = 0; col < Board::kCols; ++col) {
        for (int row = 0; row < Board::kRows; ++row) {
            const int id = board_.map[static_cast<size_t>(col) * Board::kRows +
                                      static_cast<size_t>(row)] &
                           0x3FFF;
            mini_px_[static_cast<size_t>(col) + static_cast<size_t>(row) *
                                                  Board::kCols] =
                minimap_tile_colour(engine, id);
        }
    }
    mini_dirty_ = false;
}

// 1 px per board cell in the top-right corner, with the camera rectangle,
// cursor and selected object marked.  The tile colour cache makes a rebuild
// cheap enough for every edit, and it is only rebuilt when the board changes.
void LevelEditor::draw_minimap(Framebuffer& fb, Engine& engine) {
    if (!show_minimap_) {
        return;
    }
    if (mini_dirty_) {
        rebuild_minimap(engine);
    }
    const int mx = Framebuffer::W - Board::kCols - 6;
    const int my = kTopBar + 2;
    fb.fill_rect(mx - 2, my - 2, Board::kCols + 4, Board::kRows + 4, 7);
    fb.fill_rect(mx - 1, my - 1, Board::kCols + 2, Board::kRows + 2, 0);
    for (int row = 0; row < Board::kRows; ++row) {
        for (int col = 0; col < Board::kCols; ++col) {
            fb.put(mx + col, my + row,
                   mini_px_[static_cast<size_t>(col) +
                            static_cast<size_t>(row) * Board::kCols]);
        }
    }
    // Camera viewport.
    const int vx0 = clamp_int(cam_x_ / 16, 0, Board::kCols - 1);
    const int vy0 = clamp_int(cam_y_ / 16, 0, Board::kRows - 1);
    const int vx1 = clamp_int((cam_x_ + Framebuffer::W - 1) / 16, 0,
                              Board::kCols - 1);
    const int vy1 = clamp_int((cam_y_ + Framebuffer::H - 1) / 16, 0,
                              Board::kRows - 1);
    outline(fb, mx + vx0, my + vy0, vx1 - vx0 + 1, vy1 - vy0 + 1, 8);
    // Selected object anchor.
    if (selected_obj_ >= 0 &&
        selected_obj_ < static_cast<int>(board_.objects.size())) {
        const BoardObject& o = board_.objects[static_cast<size_t>(selected_obj_)];
        fb.fill_rect(mx + clamp_int(o.x / 16, 0, Board::kCols - 1),
                     my + clamp_int(o.y / 16, 0, Board::kRows - 1), 1, 1, 14);
    }
    // Cursor.
    fb.fill_rect(mx + clamp_int(cursor_col_, 0, Board::kCols - 1),
                 my + clamp_int(cursor_row_, 0, Board::kRows - 1), 2, 2, 15);
}

void LevelEditor::draw_tile_palette(Framebuffer& fb, Engine& engine) {
    const int x = 4;
    const int y = 16;
    const int w = 312;
    const int h = 172;
    engine.draw_menu_window(fb, x, y, w, h);

    char title[80];
    std::snprintf(title, sizeof(title), "TILES %03X-%03X  TILE %03X %s",
                  tile_page_, std::min(kTileCount - 1,
                                       tile_page_ + kTilePalPage - 1),
                  paint_tile_, engine.tile(paint_tile_).name.c_str());
    engine.draw_text(fb, x + 8, y + 8, 15, shortened(title, 48).c_str());

    const int gx = x + 8;
    const int gy = y + 22;
    for (int row = 0; row < kTilePalRows; ++row) {
        for (int col = 0; col < kTilePalCols; ++col) {
            const int idx = tile_page_ + row * kTilePalCols + col;
            if (idx < 0 || idx >= kTileCount) {
                continue;
            }
            const int cx = gx + col * 28;
            const int cy = gy + row * 18;
            const TileInfo& t = engine.tile(idx);
            engine.draw_shape(fb, t.shape, cx + 5, cy + 1, false);
            if (idx == paint_tile_) {
                outline(fb, cx, cy, 26, 17, 15);
            }
        }
    }
    engine.draw_text(fb, x + 8, y + h - 10, 10,
                     "CLICK TILE  WHEEL/[/] PAGE  ESC CLOSE");
}

void LevelEditor::draw_kind_palette(Framebuffer& fb, Engine& engine) {
    const int x = 4;
    const int y = 16;
    const int w = 312;
    const int h = 172;
    engine.draw_menu_window(fb, x, y, w, h);

    const int count = kind_count_ > 0 ? kind_count_
                                      : engine.profile().kind_count;
    char title[80];
    std::snprintf(title, sizeof(title), "OBJECTS %03d-%03d  %03d %s",
                  kind_page_, std::min(count - 1, kind_page_ + kKindPalPage - 1),
                  brush_kind_, engine.profile().kind(brush_kind_).name);
    engine.draw_text(fb, x + 8, y + 8, 15, shortened(title, 48).c_str());

    const int gx = x + 8;
    const int gy = y + 24;
    for (int row = 0; row < kKindPalRows; ++row) {
        for (int col = 0; col < kKindPalCols; ++col) {
            const int idx = kind_page_ + row * kKindPalCols + col;
            if (idx < 0 || idx >= count) {
                continue;
            }
            const int cx = gx + col * 48;
            const int cy = gy + row * 36;
            const int shape = engine.kind_icon_shape(idx);
            if (shape >= 0) {
                engine.draw_shape(fb, shape, cx + 8, cy + 2, true);
            } else {
                fb.fill_rect(cx + 8, cy + 2, 24, 24, 8);
            }
            char id[16];
            std::snprintf(id, sizeof(id), "%d", idx);
            engine.draw_text(fb, cx + 2, cy + 27, 14, id);
            if (idx == brush_kind_) {
                outline(fb, cx, cy, 46, 35, 15);
            }
        }
    }
    engine.draw_text(fb, x + 8, y + h - 10, 10,
                     "CLICK OBJECT  WHEEL/[/] PAGE  ESC CLOSE");
}

void LevelEditor::draw_info(Framebuffer& fb, Engine& engine) {
    const int x = 4;
    const int y = 16;
    const int w = 312;
    const int h = 172;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 8, y + 8, 15, "BOARD INFO");

    const char* labels[kInfoFields] = {"LEVEL ID", "HEALTH", "ITEM COUNT",
                                       "SCORE", "DAMAGE FLASH", "SCORE COPY"};
    char values[kInfoFields][24] = {};
    std::snprintf(values[0], sizeof(values[0]), "%u", rd16(&board_.header[0]));
    std::snprintf(values[1], sizeof(values[1]), "%u", rd16(&board_.header[2]));
    std::snprintf(values[2], sizeof(values[2]), "%u", rd16(&board_.header[4]));
    std::snprintf(values[3], sizeof(values[3]), "%lu",
                  static_cast<unsigned long>(rd32(&board_.header[38])));
    std::snprintf(values[4], sizeof(values[4]), "%u", rd16(&board_.header[42]));
    std::snprintf(values[5], sizeof(values[5]), "%lu",
                  static_cast<unsigned long>(rd32(&board_.header[44])));

    for (int i = 0; i < kInfoFields; ++i) {
        const int ry = y + 26 + i * 16;
        char line[256];
        std::snprintf(line, sizeof(line), "%c %-14s %s",
                      i == info_sel_ ? '>' : ' ', labels[i], values[i]);
        engine.draw_text(fb, x + 10, ry, i == info_sel_ ? 15 : 10, line);
    }
    engine.draw_text(fb, x + 8, y + h - 20, 10,
                     "UP/DOWN FIELD  LEFT/RIGHT CHANGE");
    engine.draw_text(fb, x + 8, y + h - 10, 10,
                     "CHANGES APPLY TO THE LIVE BOARD  ESC CLOSE");
}

bool LevelEditor::activate_menu_row(int row, Engine& engine) {
    switch (row) {
        case 0:
            if (save(engine)) {
                show_menu_ = false;
            }
            break;
        case 1:
            open_file_picker(FileMode::Save, engine);
            break;
        case 2:
            open_file_picker(FileMode::Load, engine);
            break;
        case 3:
            show_menu_ = false;
            show_game_confirm_ = true;
            break;
        case 4:
            toggle_paint_mode();
            break;
        case 5:
            undo(engine);
            break;
        case 6:
            redo(engine);
            break;
        case 7:
            copy_object(engine);
            break;
        case 8:
            paste_object(engine);
            break;
        case 9:
            copy_tiles();
            break;
        case 10:
            paste_tiles(engine);
            break;
        case 11:
            show_menu_ = false;
            open_object_props(engine);
            break;
        case 12:
            show_menu_ = false;
            open_presets();
            break;
        case 13:
            show_menu_ = false;
            start_playtest(engine);
            break;
        case 14:
            show_menu_ = false;
            show_info_ = true;
            break;
        case 15:
            show_menu_ = false;
            help_scroll_ = 0;
            show_help_ = true;
            break;
        case 16:
            show_minimap_ = !show_minimap_;
            set_status(show_minimap_ ? "MINIMAP ON" : "MINIMAP OFF");
            break;
        case 17:
            // PC option: type letters for editor strings with the D-pad
            // keyboard instead of the native keyboard/IME.
#if defined(JILL_PS4) || defined(JILL_XBOX) || defined(JILL_VITA)
            set_status("PAD TEXT INPUT: ALWAYS ON");
#else
            {
                const bool on = !textkey::pad_keyboard_enabled();
                textkey::set_pad_keyboard_enabled(on);
                set_status(on ? "PAD TEXT INPUT ON" : "PAD TEXT INPUT OFF");
            }
#endif
            break;
        default:
            close();
            return true;
    }
    return false;
}

void LevelEditor::draw_editor_menu(Framebuffer& fb, Engine& engine) {
    const int x = 12;
    const int y = 2;
    const int w = 296;
    const int h = 190;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 10, y + 7, 15, "JILL MAKER");

    static const char* const rows[kEditorMenuRows] = {
        "SAVE",          "SAVE AS...",   "LOAD...",
        "SAVE TO GAME DATA...", "",      "UNDO",
        "REDO",          "COPY OBJECT",  "PASTE OBJECT",
        "COPY TILES",    "PASTE TILES",  "OBJECT PROPS",
        "PLACE PRESETS...", "PLAYTEST",  "BOARD INFO",
        "HELP",          "MINIMAP",      "PAD TEXT INPUT",
        "EXIT TO MAIN MENU"};
    char paint_row[32];
    const char* tool = paint_mode_ == PaintMode::Single
                           ? "SINGLE"
                           : (paint_mode_ == PaintMode::Line
                                  ? "LINE A-B"
                                  : (paint_mode_ == PaintMode::Rect
                                         ? "RECT FILL"
                                         : "FLOOD FILL"));
    std::snprintf(paint_row, sizeof(paint_row), "PAINT: %s", tool);
    char minimap_row[32];
    std::snprintf(minimap_row, sizeof(minimap_row), "MINIMAP: %s",
                  show_minimap_ ? "ON" : "OFF");
    char pad_text_row[40];
    std::snprintf(pad_text_row, sizeof(pad_text_row), "PAD TEXT: %s",
                  textkey::controller_only() ? "ON" : "OFF");
    for (int i = 0; i < kEditorMenuRows; ++i) {
        char line[48];
        const char* label = i == 4    ? paint_row
                            : i == 16 ? minimap_row
                            : i == 17 ? pad_text_row
                                      : rows[i];
        std::snprintf(line, sizeof(line), "%c %s",
                      i == menu_sel_ ? '>' : ' ', label);
        engine.draw_text(fb, x + 16, y + 18 + i * 8,
                         i == menu_sel_ ? 15 : 10, line);
    }
    const std::string target =
        "TARGET " + (save_name_.empty() ? board_name_ : save_name_);
    engine.draw_text(fb, x + 10, y + h - 20, 10,
                     shortened(target, 42).c_str());
    engine.draw_text(fb, x + 10, y + h - 10, 10,
                     "UP/DOWN MOVE  ENTER SELECT  ESC CLOSE");
}

void LevelEditor::draw_touch_bar(Framebuffer& fb, Engine& engine) {
    if (!touch_controls_ || overlay_open()) {
        return;
    }
    static const char* const labels[kTouchButtons] = {
        "MODE", "PAL",   "PICK", "TOOL", "PAN",  "SAVE", "LOAD", "PLAY",
        "INFO", "MENU",  "UNDO", "REDO", "COPY", "PASTE", "PROPS", "PRST",
        "GRID", "HELP",  "MINI", "ERASE"};
    fb.fill_rect(0, kTouchBarY, Framebuffer::W, kTouchBarH, 0);
    fb.fill_rect(0, kTouchBarY, Framebuffer::W, 1, 8);
    for (int i = 0; i < kTouchButtons; ++i) {
        const int row = i / kTouchCols;
        const int col = i % kTouchCols;
        const int x0 = col * Framebuffer::W / kTouchCols;
        const int x1 = (col + 1) * Framebuffer::W / kTouchCols;
        const int y0 = kTouchBarY + row * 20;
        const bool active = (i == 4 && touch_pan_) ||
                            (i == 19 && touch_erase_) ||
                            (i == 3 && paint_mode_ != PaintMode::Single);
        fb.fill_rect(x0 + 1, y0 + 1, x1 - x0 - 2, 18, active ? 4 : 8);
        engine.draw_text(fb, x0 + 2, y0 + 6, active ? 15 : 7, labels[i]);
    }
    fb.fill_rect(0, kTouchBarY + 19, Framebuffer::W, 1, 0);
}

void LevelEditor::draw_help(Framebuffer& fb, Engine& engine) {
    const int x = 4;
    const int y = 8;
    const int w = 312;
    const int h = 170;
    engine.draw_menu_window(fb, x, y, w, h);
    engine.draw_text(fb, x + 8, y + 8, 15, "CONTROLS - KEYBOARD / CONTROLLER");

    const int max_scroll = std::max(0, kHelpLineCount - kHelpVisible);
    help_scroll_ = clamp_int(help_scroll_, 0, max_scroll);
    for (int i = 0; i < kHelpVisible; ++i) {
        const int index = help_scroll_ + i;
        if (index >= kHelpLineCount) {
            break;
        }
        const char* line = kHelpLines[index];
        const bool heading = line[0] == '[';
        engine.draw_text(fb, x + 8, y + 22 + i * 9, heading ? 14 : 7, line);
    }
    char footer[64];
    std::snprintf(footer, sizeof(footer), "UP/DOWN SCROLL  ENTER CLOSE  %d-%d/%d",
                  help_scroll_ + 1,
                  std::min(kHelpLineCount, help_scroll_ + kHelpVisible),
                  kHelpLineCount);
    engine.draw_text(fb, x + 8, y + h - 9, 10, footer);
}

