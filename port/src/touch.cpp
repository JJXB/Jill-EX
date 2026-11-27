#include "touch.h"

#include <SDL.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

#include "config.h"
#include "screen.h"

namespace touch {
namespace {

struct Finger {
    SDL_FingerID id = 0;
    float nx = 0.0f;
    float ny = 0.0f;
    bool active = false;
};

constexpr int kMaxFingers = 16;
Finger g_fingers[kMaxFingers];
State g_current;

// Overlay layout in 320x200 frame coordinates.  The first three controls are
// round buttons (hit radius); Start/Back are small rounded rectangles.
struct ControlDef {
    int x;
    int y;
    int radius;   // circular controls
    int half_w;   // rectangular controls
    int half_h;
    bool circle;
};

constexpr ControlDef kDefault[ControlCount] = {
    {44, 150, 34, 0, 0, true},    // D-pad
    {284, 146, 22, 0, 0, true},   // Jump
    {238, 172, 17, 0, 0, true},   // Throw
    {306, 12, 0, 11, 9, false},   // Start
    {13, 12, 0, 11, 9, false},    // Back
};

Layout g_layout;
bool g_editing = false;
bool g_ignore_fingers = false;
int g_grabbed = -1;
SDL_FingerID g_grab_finger = 0;
int g_grab_dx = 0;
int g_grab_dy = 0;

Finger* find_finger(SDL_FingerID id) {
    for (Finger& f : g_fingers) {
        if (f.active && f.id == id) {
            return &f;
        }
    }
    return nullptr;
}

Finger* free_finger() {
    for (Finger& f : g_fingers) {
        if (!f.active) {
            return &f;
        }
    }
    return nullptr;
}

bool in_circle(int x, int y, int cx, int cy, int r) {
    const int dx = x - cx;
    const int dy = y - cy;
    return dx * dx + dy * dy <= r * r;
}

bool in_rect(int x, int y, int x0, int y0, int x1, int y1) {
    return x >= x0 && x <= x1 && y >= y0 && y <= y1;
}

int radius_of(int control) {
    return control >= 0 && control < ControlCount ? kDefault[control].radius : 0;
}

int half_w_of(int control) {
    return control >= 0 && control < ControlCount ? kDefault[control].half_w : 0;
}

int half_h_of(int control) {
    return control >= 0 && control < ControlCount ? kDefault[control].half_h : 0;
}

bool hit_control(int control, int x, int y) {
    if (control < 0 || control >= ControlCount) {
        return false;
    }
    if (kDefault[control].circle) {
        return in_circle(x, y, g_layout.x[control], g_layout.y[control],
                         radius_of(control) + 4);
    }
    const int hw = half_w_of(control);
    const int hh = half_h_of(control);
    return in_rect(x, y, g_layout.x[control] - hw, g_layout.y[control] - hh,
                   g_layout.x[control] + hw, g_layout.y[control] + hh);
}

void blend(uint32_t* argb, int w, int h, int x, int y, uint32_t rgb,
           int alpha) {
    if (x < 0 || y < 0 || x >= w || y >= h || alpha <= 0) {
        return;
    }
    uint32_t& dst = argb[static_cast<size_t>(y) * w + x];
    const int a = alpha > 255 ? 255 : alpha;
    const int inv = 255 - a;
    const int dr = (dst >> 16) & 0xFF;
    const int dg = (dst >> 8) & 0xFF;
    const int db = dst & 0xFF;
    const int sr = (rgb >> 16) & 0xFF;
    const int sg = (rgb >> 8) & 0xFF;
    const int sb = rgb & 0xFF;
    const int r = (sr * a + dr * inv) / 255;
    const int g = (sg * a + dg * inv) / 255;
    const int b = (sb * a + db * inv) / 255;
    dst = 0xFF000000u | (static_cast<uint32_t>(r) << 16) |
          (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
}

void ring(uint32_t* argb, int w, int h, int cx, int cy, int r, uint32_t rgb,
          int alpha) {
    // Midpoint circle outline (2 px thick).
    int x = r;
    int y = 0;
    int err = 1 - r;
    while (x >= y) {
        blend(argb, w, h, cx + x, cy + y, rgb, alpha);
        blend(argb, w, h, cx + y, cy + x, rgb, alpha);
        blend(argb, w, h, cx - y, cy + x, rgb, alpha);
        blend(argb, w, h, cx - x, cy + y, rgb, alpha);
        blend(argb, w, h, cx - x, cy - y, rgb, alpha);
        blend(argb, w, h, cx - y, cy - x, rgb, alpha);
        blend(argb, w, h, cx + y, cy - x, rgb, alpha);
        blend(argb, w, h, cx + x, cy - y, rgb, alpha);
        ++y;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

void disc(uint32_t* argb, int w, int h, int cx, int cy, int r, uint32_t rgb,
          int alpha) {
    for (int y = -r; y <= r; ++y) {
        for (int x = -r; x <= r; ++x) {
            if (x * x + y * y <= r * r) {
                blend(argb, w, h, cx + x, cy + y, rgb, alpha);
            }
        }
    }
}

void triangle(uint32_t* argb, int w, int h, int cx, int cy, int size,
              int dir_x, int dir_y, uint32_t rgb, int alpha) {
    // Small solid arrow: sample a triangle by scanning a square and testing
    // the half-plane on the pointing side.
    for (int y = -size; y <= size; ++y) {
        for (int x = -size; x <= size; ++x) {
            const int along = x * dir_x + y * dir_y;
            const int across = std::abs(x * dir_y - y * dir_x);
            if (along >= 0 && along <= size && across <= size - along) {
                blend(argb, w, h, cx + x, cy + y, rgb, alpha);
            }
        }
    }
}

void bar(uint32_t* argb, int w, int h, int x0, int y0, int x1, int y1,
         uint32_t rgb, int alpha) {
    for (int y = y0; y <= y1; ++y) {
        for (int x = x0; x <= x1; ++x) {
            blend(argb, w, h, x, y, rgb, alpha);
        }
    }
}

void draw_controls(uint32_t* argb, int w, int h, const State& state,
                   bool editing) {
    const uint32_t white = 0xE8E8E8;
    const uint32_t accent = 0x9FE8FF;
    const uint32_t edit_tint = 0x60C0FF;
    const uint32_t grab_tint = 0xFFC040;

    const int dpad_x = g_layout.x[Dpad];
    const int dpad_y = g_layout.y[Dpad];
    const int dpad_r = radius_of(Dpad);
    const int jump_x = g_layout.x[Jump];
    const int jump_y = g_layout.y[Jump];
    const int jump_r = radius_of(Jump);
    const int throw_x = g_layout.x[Throw];
    const int throw_y = g_layout.y[Throw];
    const int throw_r = radius_of(Throw);

    // D-pad body and arrows.
    disc(argb, w, h, dpad_x, dpad_y, dpad_r, 0x101010, 105);
    ring(argb, w, h, dpad_x, dpad_y, dpad_r, white, 120);
    disc(argb, w, h, dpad_x, dpad_y, 7, 0x101010, 90);

    const int arrow = 10;
    triangle(argb, w, h, dpad_x, dpad_y - 20, arrow, 0, -1,
             state.up ? accent : white, state.up ? 230 : 130);
    triangle(argb, w, h, dpad_x, dpad_y + 20, arrow, 0, 1,
             state.down ? accent : white, state.down ? 230 : 130);
    triangle(argb, w, h, dpad_x - 20, dpad_y, arrow, -1, 0,
             state.left ? accent : white, state.left ? 230 : 130);
    triangle(argb, w, h, dpad_x + 20, dpad_y, arrow, 1, 0,
             state.right ? accent : white, state.right ? 230 : 130);

    // Jump (A) and throw (X).
    disc(argb, w, h, jump_x, jump_y, jump_r, 0x101010, 120);
    ring(argb, w, h, jump_x, jump_y, jump_r, white, 130);
    triangle(argb, w, h, jump_x, jump_y + 8, 9, 0, -1,
             state.jump ? accent : white, state.jump ? 240 : 150);

    disc(argb, w, h, throw_x, throw_y, throw_r, 0x101010, 120);
    ring(argb, w, h, throw_x, throw_y, throw_r, white, 130);
    bar(argb, w, h, throw_x - 6, throw_y - 1, throw_x + 6, throw_y + 1,
        state.throw_btn ? accent : white, state.throw_btn ? 240 : 150);
    bar(argb, w, h, throw_x - 1, throw_y - 6, throw_x + 1, throw_y + 6,
        state.throw_btn ? accent : white, state.throw_btn ? 240 : 150);

    // Start (two bars) and Back (left arrow).
    const int start_x = g_layout.x[Start];
    const int start_y = g_layout.y[Start];
    const int start_hw = half_w_of(Start);
    const int start_hh = half_h_of(Start);
    bar(argb, w, h, start_x - start_hw, start_y - start_hh,
        start_x + start_hw, start_y + start_hh, 0x101010, 110);
    bar(argb, w, h, start_x - 4, start_y - 4, start_x - 2, start_y + 4,
        state.start ? accent : white, state.start ? 230 : 150);
    bar(argb, w, h, start_x + 1, start_y - 4, start_x + 3, start_y + 4,
        state.start ? accent : white, state.start ? 230 : 150);

    const int back_x = g_layout.x[Back];
    const int back_y = g_layout.y[Back];
    const int back_hw = half_w_of(Back);
    const int back_hh = half_h_of(Back);
    bar(argb, w, h, back_x - back_hw, back_y - back_hh, back_x + back_hw,
        back_y + back_hh, 0x101010, 110);
    triangle(argb, w, h, back_x, back_y, 6, -1, 0,
             state.back ? accent : white, state.back ? 230 : 150);

    if (editing) {
        for (int c = 0; c < ControlCount; ++c) {
            const uint32_t tint = (c == g_grabbed) ? grab_tint : edit_tint;
            if (kDefault[c].circle) {
                const int r = radius_of(c);
                disc(argb, w, h, g_layout.x[c], g_layout.y[c], r, tint, 26);
                ring(argb, w, h, g_layout.x[c], g_layout.y[c], r, tint, 210);
            } else {
                const int hw = half_w_of(c);
                const int hh = half_h_of(c);
                bar(argb, w, h, g_layout.x[c] - hw, g_layout.y[c] - hh,
                    g_layout.x[c] + hw, g_layout.y[c] - hh, tint, 210);
                bar(argb, w, h, g_layout.x[c] - hw, g_layout.y[c] + hh,
                    g_layout.x[c] + hw, g_layout.y[c] + hh, tint, 210);
                bar(argb, w, h, g_layout.x[c] - hw, g_layout.y[c] - hh,
                    g_layout.x[c] - hw, g_layout.y[c] + hh, tint, 210);
                bar(argb, w, h, g_layout.x[c] + hw, g_layout.y[c] - hh,
                    g_layout.x[c] + hw, g_layout.y[c] + hh, tint, 210);
            }
        }
    }
}

int clamp_int(int v, int lo, int hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

}  // namespace

void Layout::reset() {
    for (int c = 0; c < ControlCount; ++c) {
        x[c] = kDefault[c].x;
        y[c] = kDefault[c].y;
    }
}

Layout& layout() { return g_layout; }

int control_radius(int control) { return radius_of(control); }

void load_ini(const IniFile& ini) {
    static const char* const kKeys[ControlCount] = {"dpad", "jump", "throw",
                                                    "start", "back"};
    for (int c = 0; c < ControlCount; ++c) {
        const std::string key = std::string(kKeys[c]);
        g_layout.x[c] = ini.get_int("touch", key + "_x", kDefault[c].x);
        g_layout.y[c] = ini.get_int("touch", key + "_y", kDefault[c].y);
        const int hw = half_w_of(c);
        const int hh = half_h_of(c);
        const int r = radius_of(c);
        const int min_x = kDefault[c].circle ? r : hw;
        const int max_x = 319 - min_x;
        const int min_y = kDefault[c].circle ? r : hh;
        const int max_y = 199 - min_y;
        g_layout.x[c] = clamp_int(g_layout.x[c], min_x, max_x);
        g_layout.y[c] = clamp_int(g_layout.y[c], min_y, max_y);
    }
}

void save_ini(IniFile& ini) {
    static const char* const kKeys[ControlCount] = {"dpad", "jump", "throw",
                                                    "start", "back"};
    for (int c = 0; c < ControlCount; ++c) {
        const std::string key = std::string(kKeys[c]);
        ini.set_int("touch", key + "_x", g_layout.x[c]);
        ini.set_int("touch", key + "_y", g_layout.y[c]);
    }
}

void handle_event(const SDL_Event& e) {
    if (e.type != SDL_FINGERDOWN && e.type != SDL_FINGERMOTION &&
        e.type != SDL_FINGERUP) {
        return;
    }
    const SDL_FingerID id = e.tfinger.fingerId;
    if (e.type == SDL_FINGERUP) {
        if (Finger* f = find_finger(id)) {
            f->active = false;
        }
        return;
    }
    Finger* f = find_finger(id);
    if (f == nullptr) {
        f = free_finger();
        if (f == nullptr) {
            return;
        }
        f->active = true;
        f->id = id;
    }
    f->nx = e.tfinger.x;
    f->ny = e.tfinger.y;
}

void release_all() {
    for (Finger& f : g_fingers) {
        f.active = false;
    }
}

void update(Screen& screen, State& out) {    out = State{};
    for (const Finger& f : g_fingers) {
        if (!f.active) {
            continue;
        }
        int x = 0;
        int y = 0;
        if (!screen.touch_to_frame(f.nx, f.ny, x, y)) {
            continue;
        }
        // D-pad: eight-way, with a small centre deadzone.
        if (in_circle(x, y, g_layout.x[Dpad], g_layout.y[Dpad],
                      radius_of(Dpad))) {
            const int dx = x - g_layout.x[Dpad];
            const int dy = y - g_layout.y[Dpad];
            const int threshold = radius_of(Dpad) / 3;
            if (dx < -threshold) {
                out.left = true;
            }
            if (dx > threshold) {
                out.right = true;
            }
            if (dy < -threshold) {
                out.up = true;
            }
            if (dy > threshold) {
                out.down = true;
            }
        }
        if (in_circle(x, y, g_layout.x[Jump], g_layout.y[Jump],
                      radius_of(Jump))) {
            out.jump = true;
        }
        if (in_circle(x, y, g_layout.x[Throw], g_layout.y[Throw],
                      radius_of(Throw))) {
            out.throw_btn = true;
        }
        if (hit_control(Back, x, y)) {
            out.back = true;
        }
        if (hit_control(Start, x, y)) {
            out.start = true;
        }
    }
}

void publish(const State& state) { g_current = state; }

const State& current() { return g_current; }

void emit(const State& prev, const State& now) {
    struct Map {
        bool before;
        bool after;
        SDL_GameControllerButton button;
    };
    const Map map[] = {
        {prev.left, now.left, SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {prev.right, now.right, SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
        {prev.up, now.up, SDL_CONTROLLER_BUTTON_DPAD_UP},
        {prev.down, now.down, SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {prev.jump, now.jump, SDL_CONTROLLER_BUTTON_A},
        {prev.throw_btn, now.throw_btn, SDL_CONTROLLER_BUTTON_X},
        {prev.start, now.start, SDL_CONTROLLER_BUTTON_START},
        {prev.back, now.back, SDL_CONTROLLER_BUTTON_BACK},
    };
    for (const Map& m : map) {
        if (m.before == m.after) {
            continue;
        }
        SDL_Event e{};
        e.type = m.after ? SDL_CONTROLLERBUTTONDOWN : SDL_CONTROLLERBUTTONUP;
        e.cbutton.which = 0;
        e.cbutton.button = static_cast<Uint8>(m.button);
        e.cbutton.state = m.after ? SDL_PRESSED : SDL_RELEASED;
        SDL_PushEvent(&e);
    }
}

void draw(uint32_t* argb, int w, int h, const State& state) {
    draw_controls(argb, w, h, state, false);
}

void edit_begin() {
    g_editing = true;
    // The OPTIONS row was tapped with a finger that is usually still down;
    // ignore existing fingers so entering the mode cannot grab a button.
    g_ignore_fingers = true;
    g_grabbed = -1;
    g_grab_finger = 0;
    g_grab_dx = 0;
    g_grab_dy = 0;
}

bool edit_update(Screen& screen) {
    if (!g_editing) {
        return false;
    }
    if (g_ignore_fingers) {
        for (const Finger& f : g_fingers) {
            if (f.active) {
                return true;
            }
        }
        g_ignore_fingers = false;
    }
    for (const Finger& f : g_fingers) {
        if (!f.active) {
            continue;
        }
        if (g_grabbed >= 0 && f.id == g_grab_finger) {
            continue;
        }
        int x = 0;
        int y = 0;
        if (!screen.touch_to_frame(f.nx, f.ny, x, y)) {
            continue;
        }
        if (g_grabbed < 0) {
            if (in_rect(x, y, kEditDoneX, kEditDoneY, kEditDoneX + kEditDoneW,
                        kEditDoneY + kEditDoneH)) {
                return false;
            }
            if (in_rect(x, y, kEditResetX, kEditResetY,
                        kEditResetX + kEditResetW,
                        kEditResetY + kEditResetH)) {
                g_layout.reset();
                continue;
            }
            for (int c = 0; c < ControlCount; ++c) {
                if (hit_control(c, x, y)) {
                    g_grabbed = c;
                    g_grab_finger = f.id;
                    g_grab_dx = x - g_layout.x[c];
                    g_grab_dy = y - g_layout.y[c];
                    break;
                }
            }
        }
    }

    if (g_grabbed >= 0) {
        Finger* f = find_finger(g_grab_finger);
        if (f == nullptr) {
            g_grabbed = -1;
        } else {
            int x = 0;
            int y = 0;
            if (screen.touch_to_frame(f->nx, f->ny, x, y)) {
                const int c = g_grabbed;
                const int hw = kDefault[c].circle ? radius_of(c) : half_w_of(c);
                const int hh = kDefault[c].circle ? radius_of(c) : half_h_of(c);
                g_layout.x[c] = clamp_int(x - g_grab_dx, hw, 319 - hw);
                g_layout.y[c] = clamp_int(y - g_grab_dy, hh, 199 - hh);
            }
        }
    }
    return true;
}

void edit_end() {
    g_editing = false;
    g_ignore_fingers = false;
    g_grabbed = -1;
}

int edit_grabbed() { return g_grabbed; }

void draw_editor(uint32_t* argb, int w, int h) {
    draw_controls(argb, w, h, State{}, true);
}

}  // namespace touch
