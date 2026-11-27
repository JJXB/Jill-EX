#pragma once

#include <SDL.h>

#include <cstdint>

class Screen;
class IniFile;

// On-screen controls for touch devices (Android milestone B).
//
// The overlay is drawn into the 320x200 ARGB presentation frame, so it scales
// with the game and stays aligned with the integer-scaled letterbox.  Finger
// positions come back through Screen::touch_to_frame().
//
// Buttons feed the engine directly (movement/jump/throw) and also synthesise
// SDL controller button events, so the existing menu, pause, text-page and
// demo code works unchanged.
//
// The button positions are user-movable (the pause menu's TOUCH CONTROLS row)
// and stored in jill.ini under [touch].
namespace touch {

struct State {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool jump = false;
    bool throw_btn = false;
    bool start = false;
    bool back = false;
};

enum Control {
    Dpad = 0,
    Jump,
    Throw,
    Start,
    Back,
    ControlCount
};

// Button centres in 320x200 frame pixels; the size stays at the original
// per-control radius so saved positions always mean the same thing.
struct Layout {
    int x[ControlCount];
    int y[ControlCount];

    Layout() { reset(); }
    void reset();
};

Layout& layout();
int control_radius(int control);

// Read/write the [touch] section of jill.ini.
void load_ini(const IniFile& ini);
void save_ini(IniFile& ini);

// Track SDL_FINGER* events.  Non-finger events are ignored.
void handle_event(const SDL_Event& e);

// Forget every tracked finger.  Used when a surface that consumes finger
// events takes over (the level editor) and when a window/focus change can drop
// the matching SDL_FINGERUP; otherwise a released finger keeps the pad held.
void release_all();

// Recompute the pressed buttons from the held fingers.
void update(Screen& screen, State& out);

// Store the frame's state for the gameplay input path.
void publish(const State& state);
const State& current();

// Push SDL_CONTROLLERBUTTONDOWN/UP events for changed buttons.
void emit(const State& prev, const State& now);

// Draw the semi-transparent pad/button overlay into the ARGB frame.
void draw(uint32_t* argb, int w, int h, const State& state);

// --- layout calibration ---------------------------------------------------
// `edit_begin` freezes normal touch input; the app feeds finger state through
// `edit_update` once per frame.  It returns false when the DONE button was
// tapped, at which point the app should save the layout and leave the mode.
// `draw_editor` draws the controls plus a grab highlight.
void edit_begin();
bool edit_update(Screen& screen);
void edit_end();
int edit_grabbed();
void draw_editor(uint32_t* argb, int w, int h);

// Calibration banner hit boxes, drawn by the app inside the game frame so its
// font renderer can label them.
constexpr int kEditResetX = 44;
constexpr int kEditResetY = 93;
constexpr int kEditResetW = 48;
constexpr int kEditResetH = 14;
constexpr int kEditDoneX = 228;
constexpr int kEditDoneY = 93;
constexpr int kEditDoneW = 48;
constexpr int kEditDoneH = 14;

}  // namespace touch
