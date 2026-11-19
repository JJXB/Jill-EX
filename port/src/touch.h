#pragma once

#include <SDL.h>

#include <cstdint>

class Screen;

// On-screen controls for touch devices (Android milestone B).
//
// The overlay is drawn into the 320x200 ARGB presentation frame, so it scales
// with the game and stays aligned with the integer-scaled letterbox.  Finger
// positions are mapped back through Screen::touch_to_frame().
//
// Buttons feed the engine directly (movement/jump/throw) and also synthesise
// SDL controller button events, so the existing menu, pause, text-page and
// demo code works unchanged.
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

// Track SDL_FINGER* events.  Non-finger events are ignored.
void handle_event(const SDL_Event& e);

// Recompute the pressed buttons from the held fingers.
void update(Screen& screen, State& out);

// Store the frame's state for the gameplay input path.
void publish(const State& state);
const State& current();

// Push SDL_CONTROLLERBUTTONDOWN/UP events for changed buttons.
void emit(const State& prev, const State& now);

// Draw the semi-transparent pad/button overlay into the ARGB frame.
void draw(uint32_t* argb, int w, int h, const State& state);

}  // namespace touch
