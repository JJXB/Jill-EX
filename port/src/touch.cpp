#include "touch.h"

#include <SDL.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

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

// Overlay layout in 320x200 frame coordinates.
constexpr int kDpadX = 44;
constexpr int kDpadY = 150;
constexpr int kDpadR = 34;
constexpr int kJumpX = 284;
constexpr int kJumpY = 146;
constexpr int kJumpR = 22;
constexpr int kThrowX = 238;
constexpr int kThrowY = 172;
constexpr int kThrowR = 17;
constexpr int kStartX0 = 296;
constexpr int kStartY0 = 3;
constexpr int kStartX1 = 317;
constexpr int kStartY1 = 21;
constexpr int kBackX0 = 3;
constexpr int kBackY0 = 3;
constexpr int kBackX1 = 24;
constexpr int kBackY1 = 21;

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

}  // namespace

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

void update(Screen& screen, State& out) {
    out = State{};
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
        if (in_circle(x, y, kDpadX, kDpadY, kDpadR)) {
            const int dx = x - kDpadX;
            const int dy = y - kDpadY;
            const int threshold = kDpadR / 3;
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
        if (in_circle(x, y, kJumpX, kJumpY, kJumpR)) {
            out.jump = true;
        }
        if (in_circle(x, y, kThrowX, kThrowY, kThrowR)) {
            out.throw_btn = true;
        }
        if (in_rect(x, y, kBackX0, kBackY0, kBackX1, kBackY1)) {
            out.back = true;
        }
        if (in_rect(x, y, kStartX0, kStartY0, kStartX1, kStartY1)) {
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
    const uint32_t white = 0xE8E8E8;
    const uint32_t accent = 0x9FE8FF;

    // D-pad body and arrows.
    disc(argb, w, h, kDpadX, kDpadY, kDpadR, 0x101010, 105);
    ring(argb, w, h, kDpadX, kDpadY, kDpadR, white, 120);
    disc(argb, w, h, kDpadX, kDpadY, 7, 0x101010, 90);

    const int arrow = 10;
    triangle(argb, w, h, kDpadX, kDpadY - 20, arrow, 0, -1,
             state.up ? accent : white, state.up ? 230 : 130);
    triangle(argb, w, h, kDpadX, kDpadY + 20, arrow, 0, 1,
             state.down ? accent : white, state.down ? 230 : 130);
    triangle(argb, w, h, kDpadX - 20, kDpadY, arrow, -1, 0,
             state.left ? accent : white, state.left ? 230 : 130);
    triangle(argb, w, h, kDpadX + 20, kDpadY, arrow, 1, 0,
             state.right ? accent : white, state.right ? 230 : 130);

    // Jump (A) and throw (X).
    disc(argb, w, h, kJumpX, kJumpY, kJumpR, 0x101010, 120);
    ring(argb, w, h, kJumpX, kJumpY, kJumpR, white, 130);
    triangle(argb, w, h, kJumpX, kJumpY + 8, 9, 0, -1,
             state.jump ? accent : white, state.jump ? 240 : 150);

    disc(argb, w, h, kThrowX, kThrowY, kThrowR, 0x101010, 120);
    ring(argb, w, h, kThrowX, kThrowY, kThrowR, white, 130);
    bar(argb, w, h, kThrowX - 6, kThrowY - 1, kThrowX + 6, kThrowY + 1,
        state.throw_btn ? accent : white, state.throw_btn ? 240 : 150);
    bar(argb, w, h, kThrowX - 1, kThrowY - 6, kThrowX + 1, kThrowY + 6,
        state.throw_btn ? accent : white, state.throw_btn ? 240 : 150);

    // Start (two bars) and Back (left arrow), kept at the frame corners.
    bar(argb, w, h, kStartX0, kStartY0, kStartX1, kStartY1, 0x101010, 110);
    bar(argb, w, h, kStartX0 + 7, kStartY0 + 5, kStartX0 + 9, kStartY1 - 5,
        state.start ? accent : white, state.start ? 230 : 150);
    bar(argb, w, h, kStartX0 + 12, kStartY0 + 5, kStartX0 + 14, kStartY1 - 5,
        state.start ? accent : white, state.start ? 230 : 150);

    bar(argb, w, h, kBackX0, kBackY0, kBackX1, kBackY1, 0x101010, 110);
    triangle(argb, w, h, kBackX0 + 11, kBackY0 + 9, 6, -1, 0,
             state.back ? accent : white, state.back ? 230 : 150);
}

}  // namespace touch
