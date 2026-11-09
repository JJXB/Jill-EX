// Player (kind 0) handler.  Behaviour follows docs/jill1-player-physics.md #4
// and #5, which was derived from the player handler at 1FAE:01D8.
#include "engine.h"

#include <algorithm>
#include <cstdlib>

namespace {

// 0x11D0: climb animation poses (also used by the climb draw path).
const uint8_t kClimbPose[7] = {0x18, 0x18, 0x19, 0x1A, 0x1A, 0x19, 0x19};
const uint8_t kClimbStep[7] = {4, 0, 0, 6, 4, 4, 0};
// 0x11DE: 21 death frames, indexed by p11 for the p0f == 2 kill variant.
const uint8_t kDeathFrame[21] = {0x48, 0x49, 0x48, 0x49, 0x48, 0x48, 0x49,
                                 0x48, 0x49, 0x49, 0x48, 0x48, 0x48, 0x49,
                                 0x49, 0x49, 0x4A, 0x4A, 0x4A, 0x4A, 0x4A};
const uint16_t kStateFlags[6] = {1, 0, 1, 0, 2, 2};

// JILLSPIDER has no art in the original (kind 55's tileset is the empty shape
// entry 0), so the port draws a small procedural spider.  '#' body, '+' legs,
// 'o' eyes; the two frames alternate while moving.
const char* const kSpiderFrames[2][16] = {
    {"................", "................", "................", "................",
     "..+..+.....+..+.", "...++.......++..", "...++######.++..", "..+..#o##o#+..+.",
     ".....######.....", ".....######.....", "..+..+.....+..+.", "...++.......++..",
     "...++.......++..", "..+..+.....+..+.", "................", "................"},
    {"................", "................", "................", "................",
     ".....+.....+....", "..+.+.......+.+.", "...+.######..+..", "..+.+#o##o#.+.+.",
     ".....######+....", ".....######.....", ".....+.....+....", "..+.+.......+.+.",
     "...+.........+..", "..+.+.......+.+.", ".....+.....+....", "................"},
};

void draw_spider_sprite(Framebuffer& fb, int x, int y, int frame) {
    const char* const* rows = kSpiderFrames[frame & 1];
    for (int r = 0; r < 16; ++r) {
        for (int c = 0; c < 16; ++c) {
            const char ch = rows[r][c];
            if (ch == '.') {
                continue;
            }
            const uint8_t colour = ch == '#' ? 5 : (ch == 'o' ? 15 : 13);
            fb.put(x + c, y + r, colour);
        }
    }
}

// Top-of-climb pull-up ("raise").  The documented state 3 ends the instant the
// box leaves the climb tile, so Jill used to snap from a climb pose straight to
// standing.  These eight frames finish the climb visually: she keeps rising the
// last few pixels (the final climb step is at most 6 px) with the hands-over-
// hands poses, then settles into the standing frame.  Solid cells overlapping
// the shifted sprite are redrawn in front of her, so where a ledge block sits
// below her feet she emerges from behind it.
const int kRaiseFrames = 8;
const uint8_t kRaisePose[kRaiseFrames] = {0x1A, 0x1A, 0x19, 0x19,
                                          0x18, 0x18, 0x10, 0x10};
const int kRaiseOffset[kRaiseFrames] = {6, 5, 4, 3, 2, 1, 0, 0};

int pose_index(int p0f, int lo, int hi) {
    if (p0f < lo) {
        return lo;
    }
    if (p0f > hi) {
        return hi;
    }
    return p0f;
}

}  // namespace

// The shared tail is state 1 (1FAE:19F0 / linear 0x214D0).  It is not stored
// in obj.state; every state except 4/5 jumps here at the end.
void Engine::player_update(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    int changed = 0;

    // Finishing a climb: hold the body still on the ledge and let the render
    // pass animate the pull-up.  Input is ignored for these few frames.
    if (o.state == 0 && o.raise > 0) {
        --o.raise;
        o.dx = 0;
        o.dy = 0;
        o.p1b = 0;
        o.p11 = 0;
        o.flags |= 0xC000;
        return;
    }

    switch (o.state) {
        case 0: {
            bool on_ground = cando(i, o.x, o.y, 1) != 0;
            if (!on_ground) {
                o.p0f = -1;
                o.p1b = 0;
            }
            if (o.dy != 0) {
                o.dy -= (o.dy > 0) ? 1 : -1;
                changed = 1;
            }
            if (o.p11 < 0) {
                changed = 1;
                if (o.p11 == -1) {
                    o.p11 = 3;
                }
            }

            // Enhancement: while crouched, Left/Right change the aim instead
            // of standing up and walking.  dy was decremented above, so a
            // tracked crouch has dy >= 1 here and reaches 3 in the down branch.
            const bool crouch_aim =
                fire_while_crouched_ && in_.v > 0 && o.dy >= 1;

            if (o.dx == 0) {
                if (in_.h != 0 && !crouch_aim) {
                    changed = 1;
                    o.p0f = 7;
                    o.dx = in_.h;
                    o.p11 = 0;
                    goto common_tail;
                }
                if (o.p11 > 0x12C) {
                    o.p1b = 0;
                    o.p11 = 0x14;
                    changed = 1;
                } else if (o.p11 >= 0x10C) {
                    changed = 1;
                } else if (o.p11 == 3) {
                    changed = 1;
                }
            } else if (in_.h == 0) {
                if (o.p11 >= 2 && o.p0f != 0) {
                    changed = 1;
                    o.dx = 0;
                    o.p0f = 0;
                    o.p11 = 0;
                }
            } else if (o.dx == in_.h) {
                changed = 1;
                if (cando(i, o.x + 8 * o.dx, o.y, 1) != 0) {
                    o.x += 8 * o.dx;
                    o.dy = 0;
                    o.p0f = (o.p0f + 1) & 7;
                    o.p11 = 0;
                } else {
                    o.p1b = o.dx;
                    o.dx = 0;
                    o.p11 = 4;
                }
            } else {
                o.p1b = o.dx;
                o.dx = 0;
                o.p11 = 4;
            }

        common_tail:
            if (cando(i, o.x, (o.y & ~0x0F) + 0x10, 3) == 3) {
                changed = 1;
                o.state = 2;
                o.dy = 0;
                o.p0f = 0;
            }
            if (in_.btn1) {
                latch1_ = true;
                changed = 1;
                o.state = 2;
                // The original impulse is -(0x10 + 4 * itemcount(9)): each
                // feather raises the jump by 4.  SUPER JUMP sets a minimum
                // feather count (menu: OFF/1..15).  Collected feathers still
                // count if they exceed the cheat.
                int feathers = inv_count(9);
                if (super_jump_ > feathers) {
                    feathers = super_jump_;
                }
                o.dy = -(0x10 + 4 * feathers);
                o.p0f = 0;
                o.dx = in_.h;
                sound(1, 1);
            } else if (in_.v != 0) {
                int ny = o.y + 8 * in_.v;
                if ((o.x & 0x0F) == 0 && cando(i, o.x, ny, 1) != 0 &&
                    cando(i, o.x, ny, 4) == 0) {
                    changed = 1;
                    set_origin(i, o.x, ny);
                    o.state = 3;
                    o.p0f = 3;
                } else if (o.state == 0 && o.dx == 0) {
                    o.dy += 2 * in_.v;
                    if (o.dy < -3) {
                        o.dy = -3;
                    } else if (o.dy > 3) {
                        o.dy = 3;
                    }
                    o.dx = 0;
                    o.p1b = 0;
                    o.p11 = 3;
                    // Enhancement: remember the aim direction while crouched.
                    if (fire_while_crouched_ && in_.h != 0) {
                        last_facing_ = in_.h;
                        changed = 1;
                    }
                }
            }
            break;
        }

        case 2: {  // airborne / water / ladder grab
            changed = 1;
            o.p13 = 0;
            // Enhancement (default off): a jump pressed while airborne is
            // consumed, so holding it through a fall does not auto-jump on
            // landing.  Turn AIR JUMP BUFFER on for the original
            // hold-to-bounce behaviour.
            if (in_.btn1 && !air_jump_buffer_) {
                latch1_ = true;
            }
            if ((o.x & 0x0F) == 0 && cando(i, o.x, o.y, 4) == 0) {
                changed = 1;
                o.state = 3;
                o.p0f = 6;
                break;
            }
            ++o.p0f;
            if (o.p0f <= 2) {
                break;
            }
            o.dy += 2;
            if (o.dy > 0x10) {
                o.dy = 0x10;
            }
            if (in_.h != 0) {
                o.p0f = 2;
                o.dx = in_.h;
            }
            if (o.p0f > 8) {
                o.dx = 0;
            }
            if (try_move(i, o.x + 8 * in_.h, o.y + o.dy)) {
                break;
            }

            if (o.dy < 0) {  // ceiling
                int row = (o.y - 1) & ~0x0F;
                if (o.y == row) {
                    o.dy = 0;
                } else if (try_move(i, o.x + 8 * in_.h, row)) {
                    o.dy = 0;
                }
                break;
            }

            bool on_floor = (o.y & 0x0F) == 0;
            if (!on_floor &&
                !try_move(i, o.x + 8 * in_.h, (o.y & ~0x0F) + 0x10)) {
                on_floor = true;
            }
            if (on_floor) {
                o.state = 0;
                o.p13 = 6;
                o.p11 = ((o.dy >= 0x10) ? -7 : -4) + ((in_.h != 0) ? 1 : 0);
                o.dx = 0;
                o.dy = 0;
                sound(1, 2);
            }
            break;
        }

        case 3: {  // climb
            // Same enhancement as state 2: a jump press that is not used to
            // launch off the vine is consumed instead of staying buffered
            // until the climb ends.
            if (in_.btn1 && !air_jump_buffer_) {
                latch1_ = true;
            }
            if (in_.h != 0) {
                if (o.p0f != 6 && cando(i, o.x + 8 * in_.h, o.y, 1) != 0) {
                    o.p13 = 0;
                    changed = 1;
                    set_origin(i, o.x + 8 * in_.h, o.y);
                    o.state = 2;
                    o.p0f = 2;
                    o.dx = in_.h;
                    if (in_.btn1) {
                        latch1_ = true;
                        o.dy = -0x0C;
                        sound(1, 1);
                    } else {
                        o.dy = -4;
                    }
                }
                break;
            }

            o.dx = 0;
            if (o.p0f == 6) {
                o.p0f = 0;
            }
            if (in_.v == 0) {
                break;
            }

            int newy = o.y;
            if (in_.v < 0) {
                newy -= kClimbStep[pose_index(o.p0f, 0, 6)];
            } else {
                newy += 4;
            }
            if (cando(i, o.x, newy, 1) == 0) {
                newy = (in_.v > 0) ? ((o.y & ~0x0F) + 0x10) : ((o.y - 1) & ~0x0F);
                if (cando(i, o.x, newy, 1) == 0) {
                    newy = 0;
                }
            }
            if (newy == 0) {
                break;
            }
            if (in_.v < 0) {
                --o.p0f;
                if (o.p0f >= 6) {
                    o.p0f = 0;
                }
            } else {
                o.p0f = 2;
            }
            changed = 1;
            set_origin(i, o.x, newy);
            if (cando(i, o.x, o.y, 4) != 0) {
                o.state = 0;
                o.dx = 0;
                o.p1b = 0;
                if (in_.v < 0) {
                    o.raise = kRaiseFrames;  // pull up over the ledge
                }
            }
            break;
        }

        case 4: {  // recover after respawn
            changed = 1;
            o.w = profile_->kind(0).width;
            o.h = profile_->kind(0).height;
            if (o.p11 >= 0x20) {
                o.state = 0;
                o.dx = 0;
                o.p1b = 0;
                o.dy = 0;
            }
            break;
        }

        case 5:  // death
            if (o.p11 == 0 && o.p0f == 2) {
                o.dy += 2;
                if (o.dy > 0x10) {
                    o.dy = 0x10;
                }
                if (just_move(i, o.x, o.y + o.dy)) {
                    o.p11 = -1;
                }
            }
            if (o.p11 == 1 && o.p0f == 2) {
                o.w = 0x20;
                o.h = 0x20;
                set_origin(i, o.x, o.y + 0x10);
            }
            if (o.p11 >= 0x14) {
                respawn();
            }
            ++o.p11;
            return;

        default:
            o.state = 0;
            break;
    }

    // Clamp the climb pose, then run the shared tail.
    if (o.state == 3) {
        if (o.p0f < 0) {
            o.p0f = 5;
        } else if (o.p0f > 6) {
            o.p0f = 6;
        }
    }

    if (o.dx != 0) {
        o.p1b = o.dx;
    }
    if (o.p1b != 0) {
        last_facing_ = o.p1b;
    }
    // Enhancement: crouching clears p1b, so the normal check cannot fire.  When
    // enabled, use the last direction the player faced instead and spawn the
    // weapon at the crouch-pose hand height (frame 0x3D occupies rows 12..31).
    const bool crouched = o.state == 0 && o.dy == 3;
    int facing = o.p1b;
    if (fire_while_crouched_ && crouched) {
        facing = last_facing_;
    }
    if (in_.btn2 && o.state >= 0 && o.state < 6 &&
        (kStateFlags[o.state] & 1) && facing != 0) {
        latch2_ = true;
        int blades = 0;
        for (const BoardObject& b : objects_) {
            if (b.kind == 0x32) {
                ++blades;
            }
        }
        int sx = o.x + facing * 0x0C + 4;
        int sy = o.y + ((fire_while_crouched_ && crouched) ? 0x12 : 2);
        if (blades < inv_count(8)) {
            int n = add_obj(0x32, sx, sy);
            if (n >= 0) {
                objects_[static_cast<size_t>(n)].dx = facing * 6;
                sound(2, 0x22);
            }
        } else if (del_item(2)) {
            int n = add_obj(2, sx, sy);
            if (n >= 0) {
                if (cando(n, sx, sy, 1) == 1) {
                    objects_[static_cast<size_t>(n)].dx = facing * 8;
                    objects_[static_cast<size_t>(n)].p11 = 3;
                    sound(2, 3);
                } else {
                    // The knife would start inside a wall: put it back.
                    kill_obj(n);
                    add_item(2);
                    sound(2, 8);
                }
            } else {
                add_item(2);
                sound(2, 8);
            }
        } else {
            sound(2, 8);
        }
    }

    ++o.p11;
    if (o.p13 != 0) {
        changed = 1;
        --o.p13;
    }
    // 26F4:23E1: hazard tiles (spikes, thorns, lava, water) under the player.
    tile_touch(i);
    (void)changed;
    // The return value is the original "changed" flag; the update pass turns it
    // into a 0xC000 footprint mark for this frame.
    if (changed) {
        o.flags |= 0xC000;
    }
}

void Engine::player_draw(int i, Framebuffer& fb) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    // The handler starts with DI = tileset << 8 (0x800 for the player) and the
    // per-state code adds a frame offset to it (1FAE:0x0264 onwards).
    int f = 0;

    if (o.state == 0 && o.raise > 0) {
        // Pull-up after a climb: rise from behind the block below, then stand.
        const int step = kRaiseFrames - std::min(o.raise, kRaiseFrames);
        const int px = o.x - cam_x_;
        const int py = o.y - cam_y_ + kRaiseOffset[step];
        draw_player_shape(fb, 0x0800 | kRaisePose[step], px, py, true);
        // The ledge block is foreground while she is still behind it, so redraw
        // the solid cells that overlap the shifted sprite.  Climb tiles (class
        // 3) stay behind her, which is what makes the raise read correctly.
        const int c0 = o.x >> 4;
        const int c1 = (o.x + 15) >> 4;
        const int r0 = (o.y + kRaiseOffset[step]) >> 4;
        const int r1 = (o.y + kRaiseOffset[step] + 31) >> 4;
        for (int c = c0; c <= c1; ++c) {
            for (int r = r0; r <= r1; ++r) {
                if (c < 0 || c >= Board::kCols || r < 0 || r >= Board::kRows) {
                    continue;
                }
                const int id = map_[c * Board::kRows + r] & 0x3FFF;
                if (id < TileDefs::kCount &&
                    (tiles_.at(id).flags & 0x0007) == 0x0006) {
                    draw_shape(fb, tiles_.at(id).shape, c * 16 - cam_x_,
                               r * 16 - cam_y_, false);
                }
            }
        }
        return;
    }

    switch (o.state) {
        case 0:
            if (o.p11 < 0) {
                // Landing pose: front if there is no facing, else side view,
                // with one extra frame at p11 == -4/-5.
                f = (o.p1b == 0) ? 0x3C : (0x24 + ((o.p1b > 0) ? 8 : 0));
                if (o.p11 == -4 || o.p11 == -5) {
                    ++f;
                }
            } else if (o.dy < 0) {
                f = 0x13;
            } else if (o.dy == 3) {
                f = 0x3D;
            } else if (o.dy > 0) {
                f = 0x12;
            } else if (o.p1b != 0 && (o.dx != 0 || o.p11 < 3)) {
                // Walk: 0x00..0x07 facing left, 0x08..0x0F facing right.
                if (o.p0f == 8) {
                    f = 0x14 + ((o.dx > 0) ? 1 : 0);
                } else {
                    f = (o.p0f & 7) + ((o.dx > 0) ? 8 : 0);
                }
            } else if (o.p1b != 0 && o.p11 < 0x14) {
                // Just stopped / turning.
                f = 0x14 + ((o.p1b > 0) ? 1 : 0);
            } else if (o.p11 > 0x96) {
                // The original switches to 0x11); later still the demo-caption
                // logo is overlaid (p11 > 0x10C), which the port drops.
                f = 0x11;
            } else {
                f = 0x10;
            }
            break;
        case 2:
            if (o.dx == 0) {
                if (o.p0f == 0) {
                    f = 0x38;
                } else if (o.p0f == 1) {
                    f = 0x39;
                } else {
                    f = (o.dy <= 0) ? 0x3A : 0x3C;
                }
            } else {
                if (o.p0f == 0) {
                    f = 0x20;
                } else if (o.p0f == 1) {
                    f = 0x21;
                } else {
                    f = (o.dy <= 0) ? 0x22 : 0x24;
                }
                if (o.dx > 0) {
                    f += 8;
                }
            }
            break;
        case 3:
            f = kClimbPose[pose_index(o.p0f, 0, 6)];
            break;
        case 4:
            if (o.p11 < 0x10) {
                f = 0x13;
            } else if (o.p11 < 0x18) {
                f = 0x10;
            } else {
                f = 0x12;
            }
            break;
        case 5:
            if (o.p11 == 0) {
                f = 0x42;
            } else if (o.p0f == 0) {
                f = 0x30 + (o.p11 / 4);
            } else if (o.p0f == 1) {
                f = 0x40 + (o.p11 / 4);
            } else {
                f = kDeathFrame[pose_index(o.p11, 0, 20)];
            }
            break;
        default:
            f = 0x18;
            break;
    }

    draw_player_shape(fb, 0x0800 | (f & 0xFF), o.x - cam_x_, o.y - cam_y_, true);
}

// Morphed Jill forms (kinds 54-57), behaviour from
// docs/jill1-kind-handlers.md #54-#57.  The object is still slot 0.
void Engine::form_update(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    switch (o.kind) {
        case 54: {  // JILLFISH (1fae:202a)
            o.p13 = (o.p13 + 1) & 3;
            const int facing = o.dx;
            const bool water = fishdo(i, o.x, o.y);
            // 1FAE:2E61 only overwrites p0f when dx != 0, so a fish that has
            // stopped still remembers which way it faces.
            if (facing != 0) {
                o.p0f = facing;
            }
            o.dx = in_.h * 8;
            if (!water) {
                o.dy = std::min(o.dy + 2, 0x10);
            } else {
                o.dy += in_.v * 2 + (o.dy < 2 ? 1 : 0);
                o.dy = std::max(-8, std::min(o.dy, 8));
            }
            if ((std::rand() & 3) == 0) {
                add_obj(0x3A, o.x + 6, o.y - 2);  // bubble
                o = objects_[static_cast<size_t>(i)];
            }
            if (in_.btn1 && water) {
                o.dy = -2;
                sound(1, 0x25);
                latch1_ = true;
            }
            if (in_.btn2) {
                int sy = o.y;
                if (center_fish_shot_) {
                    sy += (o.h - profile_->kind(0x3F).height) / 2;
                }
                int idx = add_obj(0x3F, o.x + (o.p0f < 0 ? 0x20 : 0) - 8, sy);
                if (idx >= 0) {
                    const int shot =
                        (o.p0f > 0 ? 8 : (o.p0f < 0 ? -8 : 0)) +
                        (o.dx > 0 ? 4 : (o.dx < 0 ? -4 : 0));
                    objects_[static_cast<size_t>(idx)].dx = shot;
                    objects_[static_cast<size_t>(idx)].p13 = 3;
                }
                sound(1, 0x26);
                latch2_ = true;
                o = objects_[static_cast<size_t>(i)];
            }
            if (!just_move(i, o.x, o.y + o.dy)) {
                o.dy = 0;
            }
            if (!fishdo(i, o.x + o.dx, o.y)) {
                o.dx = 0;
            }
            tile_touch(i);
            return;
        }

        case 55: {  // JILLSPIDER: port-added wall-climbing form
            // The original handler is an empty stub and kind 55's tileset is
            // the empty shape entry 0, so the port supplies movement and art.
            if (in_.h == 0 && in_.v == 0) {
                return;
            }
            o.p13 = (o.p13 + 1) & 3;
            if (in_.h != 0) {
                o.p0f = in_.h;
            }
            const int step = 2;
            const int nx = o.x + in_.h * step;
            const int ny = o.y + in_.v * step;
            if (cando(i, nx, ny, 1) == 1) {
                set_origin(i, nx, ny);
            } else if (in_.h != 0 && cando(i, nx, o.y, 1) == 1) {
                set_origin(i, nx, o.y);
            } else if (in_.v != 0 && cando(i, o.x, ny, 1) == 1) {
                set_origin(i, o.x, ny);
            }
            tile_touch(i);
            return;
        }

        case 56: {  // JILLBIRD (1fae:298b)
            o.p0f = o.dx;
            o.p13 = (o.p13 + 1) % 6;
            o.dx = in_.h * (((o.x & 7) == 0) ? 8 : 4);
            if (in_.btn1) {
                o.dy = -6;
                sound(1, 0x0F);
                latch1_ = true;
            }
            if (in_.btn2) {
                const int facing = o.p0f;
                int idx = add_obj(0x3E, o.x + (facing < 0 ? -0x18 : 0x18), o.y);
                if (idx >= 0) {
                    objects_[static_cast<size_t>(idx)].dx =
                        (facing < 0 ? -8 : 8);
                    objects_[static_cast<size_t>(idx)].dy = in_.v * 2;
                }
                sound(1, 0x22);
                latch2_ = true;
                o = objects_[static_cast<size_t>(i)];
            }
            o.dy = std::max(-8, std::min(o.dy + 1, 8));
            if (!just_move(i, o.x + o.dx, o.y)) {
                if (!just_move(i, o.x, o.y + o.dy)) {
                    const int row = (o.y + 0x10) & ~0x0F;
                    if (!just_move(i, o.x, row)) {
                        o.dy = 0;
                    }
                }
            } else {
                just_move(i, o.x, o.y + o.dy);
            }
            tile_touch(i);
            return;
        }

        case 57: {  // JILLFROG (1fae:2648)
            if (o.dx != 0) {
                o.p1b = o.dx;  // facing memory, like the original
            }
            o.dx = in_.h * (((o.x & 7) == 0) ? 8 : 4);
            if (cando(i, o.x, o.y + 1, 3) == 3) {
                o.dy = std::min(o.dy + 1, 0x0C);
                o.state = 1;  // airborne / falling pose
            } else if (in_.btn1 || in_.btn2) {
                o.dy = -0x0C;
                o.state = 1;
                sound(1, 0x2B);
                latch1_ = true;
                latch2_ = true;
            } else {
                o.dy = -8;
                o.state = 1;
                sound(1, 0x12);
            }
            if (move_obj(i, o.x + o.dx, o.y + o.dy) == 0) {
                if (o.dy < 0) {
                    o.dy = 0;
                } else {
                    o.y = ((o.y & ~0x0F) + 0x10 - o.h);
                    o.p13 = 0;
                    o.dy = 0;
                    o.state = 0;  // grounded idle frame
                    set_origin(i, o.x, o.y);
                }
            }
            tile_touch(i);
            return;
        }

        default:
            return;
    }
}

void Engine::form_draw(int i, Framebuffer& fb) {
    const BoardObject& o = objects_[static_cast<size_t>(i)];
    const int ts = profile_->kind(o.kind).tileset;
    const int x = o.x - cam_x_;
    const int y = o.y - cam_y_;
    switch (o.kind) {
        case 54: {  // fish: two facing sets, table 0x11F3 = {0,1,2,1}
            static const uint8_t kFishFrame[4] = {0, 1, 2, 1};
            int f = kFishFrame[o.p13 & 3] + (o.p0f > 0 ? 3 : 0);
            draw_shape(fb, (ts << 8) | f, x, y, true);
            return;
        }
        case 55:  // spider: procedural placeholder art
            draw_spider_sprite(fb, x, y, (o.p13 >> 1) & 1);
            return;
        case 56: {  // bird: table 0x11FB = {0,1,2,3}, +4 when facing left
            static const uint8_t kBirdFrame[4] = {0, 1, 2, 3};
            int f = kBirdFrame[o.p13 & 3] + (o.p0f < 0 ? 4 : 0);
            draw_shape(fb, (ts << 8) | f, x, y, true);
            return;
        }
        case 57: {  // frog: idle frame 3, hop frames 1/2
            int f = (o.p1b < 1 ? 3 : 0);
            if (o.state == 1) {
                f += (o.dy < 1) ? 1 : 2;
            }
            draw_shape(fb, (ts << 8) | f, x, y, true);
            return;
        }
        default:
            return;
    }
}

// Kind 23 TINY (1FAE:1D15): the Jill 3 map avatar.  It has no gravity and no
// state machine: while a direction is held it moves 2 px per tick if every
// tile under the destination has the 0x200 "map walkable" bit.
void Engine::tiny_update(int i) {
    BoardObject& o = objects_[static_cast<size_t>(i)];
    bool changed = false;
    if (in_.h != 0 || in_.v != 0) {
        o.p11 = (o.p11 + 1) & 3;
        o.dx = in_.h;
        o.dy = in_.v;
        if (cando(i, o.x + o.dx * 2, o.y + o.dy * 2, 0x200) != 0) {
            o.x += o.dx * 2;
            o.y += o.dy * 2;
            changed = true;
        }
    }
    tile_touch(i);
    if (changed) {
        o.flags |= 0xC000;
    }
}
