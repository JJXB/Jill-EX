#pragma once

// Native DualShock 4 input for the PS4 port.  The bundled SDL2 PS4 joystick
// driver never posts SDL_CONTROLLER events (its GUID does not match any game
// controller mapping), so the menus received no input at all.  This module
// talks to libScePad directly and lets the app synthesise controller events.

struct Ps4PadState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool cross = false;
    bool circle = false;
    bool square = false;
    bool triangle = false;
    bool l1 = false;
    bool r1 = false;
    bool l2 = false;
    bool r2 = false;
    bool options = false;
    bool l3 = false;
    bool r3 = false;
    bool touch = false;
    // Left stick, -1..+1 (0 when centred).
    float stick_x = 0.0f;
    float stick_y = 0.0f;
    bool connected = false;
};

bool ps4_pad_init();
void ps4_pad_poll(Ps4PadState& out);
void ps4_pad_shutdown();
