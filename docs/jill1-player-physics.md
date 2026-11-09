# Jill 1: frame order, update pass and player physics

Status: derived from the Jill 1 image (`re/img.bin`, load module base `0x1000`).
Companion documents: `jill1-engine.md` (map, objects, boards),
`jill1-kind-handlers.md` (kind 0 = PLAYER), `episode-profiles.md` /
`profiles.json` (per-episode DGROUP offsets), `shared-handler-equivalence.md`
(proof that J2/J3 run the same code).

Confidence markers: **[verified]** = read directly from the disassembly
(`re/export/decomp/*`, capstone over `re/img.bin`); **[inferred]** = behaviour
consistent with the code but not proven by a run; **[open]** = not determined.

Addressing: `linear = 0x10000 + stored_segment * 16 + offset`, and the file
offset inside `img.bin` is `linear - 0x10000`. The player handler is
`1FAE:01D8` = linear `0x1FCB8`. The object engine is module `0x16F4` in the
image = `0x26F4` at run time.

---

## 1. Frame timeline (`_play`, `22AB:1531`) **[verified]**

One iteration, in this exact order:

| # | Step | Effect |
|--|--|--|
| 1 | level-transition check | `DGROUP:0xBB1E` holds a pending target; a non-empty string dispatches the board change and skips the rest of the frame |
| 2 | `t = DGROUP:0x3E28` | BIOS tick (18.2 Hz) snapshot for the frame limiter |
| 3 | `FUN_1581_063e()` | music / sound service |
| 4 | `DGROUP:0xBE3A++` | frame counter (used by demos and animation phases) |
| 5 | `FUN_1415_026f(1)` | keyboard + joystick + macro playback -> input block (`0x3C76` ...) |
| 6 | `FUN_2ce7_000d(key)` | macro/demo key stream |
| 7 | special keys | `X` x3, `Z` x3, `W`+digit, `T`, `N`, `P` (see `jill1-engine.md` #3) |
| 8 | bat spawn | if `param_1 != 0` and no kind `0x43` object exists: `_addobj(0x43, player.x, player.y)` |
| 9 | `FUN_26f4_11d7()` | tile pre-pass, **not** input handling (see below) |
| 10 | `FUN_26f4_12b9(1)` | update pass: cull, update, hit tests, footprint blocking |
| 11 | `FUN_26f4_259b()` | sort the active list by `y` (draw order) |
| 12 | `FUN_26f4_0d52(page)` | draw the active list into the hidden page |
| 13 | `FUN_26f4_2506()` | HUD / overlays |
| 14 | key jump table `0x19DF` | five menu-ish keys, dispatched after drawing |
| 15 | `DGROUP:0xBCAA = 1` | request page flip / retrace sync |
| 16 | busy-wait `while (DGROUP:0x3E28 - t < DGROUP:0x1210 + 1)` | frame limiter |

`DGROUP:0x1210` is 0 or 1 and is toggled by the `T` key, so the game runs at
**18.2 fps** (1 tick/frame) or **9.1 fps** (2 ticks/frame). All the per-frame
constants below are at 18.2 fps. The loop is not time-compensated: a port must
run the update at a fixed rate to keep physics identical. **[verified]**

### 1.1 `FUN_26f4_11d7` is a tile pre-pass **[verified]**

Contrary to an earlier note in `jill1-engine.md`, this function does not read
the input block. It walks every 16x16 cell in the camera rectangle and, for
cells whose tile-table attribute word (`DGROUP:0x83EE + tile*8`) has bit
`0x20` set, calls `FUN_1f28_0006(col, row, 2)`; if that returns non-zero the
map word gets `0x4000` OR-ed in:

```
for col in cam.x>>4 .. min(0x7F, (cam.x + cam.w)>>4):
    for row in cam.y>>4 .. min(0x3F, (cam.y + cam.h)>>4):
        t = map[col*0x40 + row]
        if attr(t) & 0x20:
            if FUN_1f28_0006(col, row, 2):
                map[col*0x40 + row] |= 0x4000
```

So bit `0x20` in the tile attribute is "this tile runs a script/animation"
(the consumer is `1F28:0006`, not analysed). This pre-pass runs **before** the
object update, so objects see the `0x4000` bits of the current frame.

---

## 2. Update pass (`FUN_26f4_12b9(mode)`) **[verified]**

```
; --- 2.1 build the active list
visible_count = 1                       ; DGROUP:0xBECC
active[0]     = 0                       ; DGROUP:0xBCAC, the player is always first
for i = 1 .. objcount-1:                ; DGROUP:0xBB1C
    if obj.x + obj.w >= cam.x - 0x60 and obj.x <= cam.x + cam.w + 0x60 and
       obj.y + obj.h >= cam.y - 0x30 and obj.y <= cam.y + cam.h + 0x30:
        active[visible_count++] = i     ; capped at 0xC0 entries
        obj.flags &= ~0xC000
    else:
        obj.flags |= 0xC000

; --- 2.2 per active object, in list order
for each a in active[0..visible_count-1]:
    if mode == 0:                       ; draw/mark variant
        obj.flags |= 0xC000
    else:                               ; mode 1, the normal update
        if obj.ttl > 0: obj.ttl--       ; +0x1D
        if handler(obj, 2, 0) != 0:
            obj.flags |= 0xC000
    if kind_flags[obj.kind] & 8:        ; DGROUP:0xBE3C, 8 = "touch damages"
        for each b in active[] with b != a:
            if AABB(a) overlaps AABB(b):
                handler(a, 1, b); a.flags |= 0xC000
                handler(b, 1, a); b.flags |= 0xC000
    if obj.flags & 0xC000:
        for every cell covered by the AABB:
            map[cell] |= 0xC000

; --- 2.3 final sweep
for each a in active[]:
    if any cell under AABB(a) has bit 0xC000:
        a.flags |= 0xC000
```

Consequences for a port:

* object 0 (the player) is updated **first** every frame, before any enemy;
* the update pass is in list order, and the active list is rebuilt from scratch
  each frame (so `handler` order is deterministic);
* `0xC000` on the object means "blocked / remove me": the object is skipped by
  drawing and, if it survives one more frame, becomes a solid map region for
  everything else. Kinematic objects (knife, spinblade, Jill herself) thus act
  as temporary walls;
* the hit pass is symmetric: both objects get `handler(...,1,...)` even though
  only one of them asked for it;
* mode 0 is the "draw only" variant used when the game wants the sorted list
  without running game logic.

`_addobj` (26F4:1892) appends at `objcount` (cap `0x100`), and the collision
test in 2.2 uses strict `<` on both axes (so touching edges do not collide).

### 2.1 Handler ABI **[verified]**

```
handler(obj_index, action, other_index) -> int
  action 0 = draw
  action 1 = "was hit by other"; return value is informational
  action 2 = update; non-zero return = object is blocked/done
  action 3/4/5 = _sendtrig codes, unused by the player
```

The kind-0 handler returns, for action 1, the *other* object's kind in `AL`
(`1FAE:0x1CF` tail: `mov ax,[bp+0xa]; ... mov al,obj.kind`). It returns 0 for
actions 3/4/5. Action 0 and 2 dispatch through jump tables at
`1FAE:0x0258` (draw) and `1FAE:0x0868` (update).

---

## 3. Player record and kind-table entry **[verified]**

Kind 0 ("JILL", the main player) in the kind tables:

| Field | Value |
|--|--|
| width table `DGROUP:0xB6B6[0]` | `0x10` |
| height table `DGROUP:0xB740[0]` | `0x20` |
| kind flags `DGROUP:0xBE3C[0]` | `0x0008` (contact damage -> participates in the AABB hit pass) |
| tileset `DGROUP:0xBB3E[0]` | `8` (shape base `0x800`) |
| handler | `1FAE:01D8` |

Record fields the player code uses (record = 31 bytes, base `DGROUP:0x96AC`):

| Off | Name | Meaning in the player |
|--|--|--|
| `+0x00` | kind | 0 |
| `+0x01` | x | pixels |
| `+0x03` | y | pixels |
| `+0x05` | dx | **-1 / 0 / +1 direction**, not a pixel delta |
| `+0x07` | dy | **pixels per frame** (jump/fall speed) |
| `+0x09` | w | 0x10 (0x20 while the drowned body swells; 0x10 in TINY form) |
| `+0x0B` | h | 0x20 |
| `+0x0D` | state | 0..5, see #4 |
| `+0x0F` | p0f | pose / animation index (state dependent) |
| `+0x11` | p11 | sub-state counter, incremented by the shared tail each frame |
| `+0x13` | p13 | secondary timer, decremented by the shared tail |
| `+0x15` | flags | bit `0xC000` = blocked |
| `+0x1B` | p1b | last horizontal direction (facing) |

The handler copies three tiny tables into its stack frame on entry, so the
draw/update code can index them directly (all three are byte tables, same
content in J1/J2/J3):

| DGROUP | len | contents | use |
|--|--|--|--|
| `0x11D0` | 7 | `18 18 19 1A 1A 19 19` | climb animation shapes (`0x800 + table[p0f]`, state 3) |
| `0x11D7` | 7 | `04 00 00 06 04 04 00` | climb-up step in pixels, indexed by `p0f` |
| `0x11DE` | 21 | `48 49 48 49 48 48 49 48 49 49 48 48 48 49 49 49 4A 4A 4A 4A 4A` | death frames, indexed by `p11` (state 5, `p0f == 2` variant) |

The copies are 7, 7 and 0x15 bytes at `1FAE:0x1CD0` (J1). J2 uses
`0x126C/0x1273/0x127A`, J3 `0x136A/0x1371/0x1378`.

---

## 4. State machine **[verified]**

Update table `1FAE:0x0868` (word per state):

| state | entry | name |
|--|--|--|
| 0 | `1FAE:0874` | ground / idle / walk |
| 1 | `1FAE:19F0` | shared tail (never stored in `obj.state`; reached by `jmp`) |
| 2 | `1FAE:12AA` | airborne / water / ladder grab |
| 3 | `1FAE:1627` | climb |
| 4 | `1FAE:105D` | recover (respawn) |
| 5 | `1FAE:1104` | death |

State flags table `DGROUP:0xBE2C`, six words, reset on level load by
`25D3:0000` (at linear `0x25E60`) to `{1,0,1,0,2,2}`:

```
state_flags[state] & 1  -> "may throw a weapon in this state" (states 0 and 2)
state_flags[state] & 2  -> "invulnerable / skip tile events"  (states 4 and 5)
```

`_playerkill` (26F4:05E7) tests `state_flags[player.state] & 2` and returns
without damage, which is what makes the recover and death states immune. The
same table is indexed with an *enemy's* state by `FUN_26f4_23e1`, so for
objects in state 4/5 the tile-event scan is skipped as well. **[verified]**

### 4.1 State 0 - ground control (`0x20354`) **[verified]**

```
on_ground = _cando(obj, x, y, 1)            ; mask 1 = "open/air class" box test
if !on_ground:
    p0f = -1                                ; falling pose
    p1b = 0
if dy != 0:
    dy -= sign(dy)                          ; settle to 0 by 1 px/frame
    changed = 1
if p11 < 0:
    changed = 1
    if p11 == -1: p11 = 3                   ; end of the landing pose
if dx == 0:
    if input.h != 0:                        ; start walking
        changed = 1; p0f = 7; dx = input.h; p11 = 0
        goto shared
    ; idle timer (also the attract-mode caption timer, see #9)
    if p11 > 0x12C: p1b = 0; p11 = 0x14; changed = 1
    elif p11 >= 0x10C: changed = 1
    elif p11 == 0x102: show_caption(rand() & 3)
    elif p11 == 3: changed = 1
elif input.h == 0:                          ; released -> stop
    if p11 >= 2 and p0f != 0:
        changed = 1; dx = 0; p0f = 0; p11 = 0
elif dx == input.h:                         ; keep walking
    changed = 1
    if _cando(obj, x + 8*dx, y, 1):
        x += 8*dx; dy = 0
        p0f = (p0f + 1) & 7
        p11 = 0
    else:                                   ; hit a wall
        p1b = dx; dx = 0; p11 = 4
else:                                       ; opposite direction
    p1b = dx; dx = 0; p11 = 4
common_tail:                                ; 0x207CA, reached by every path
    if _cando(obj, x, (y & ~0xF) + 0x10, 3) == 3:
        changed = 1; state = 2; dy = 0; p0f = 0   ; water or climb tile below
    if btn1:                                ; jump (Ctrl)
        latch[0] = 1; changed = 1; state = 2
        dy      = -(0x10 + 4 * itemcount(9))
        p0f     = 0
        dx      = input.h
        sound(1, 1)
    elif input.v != 0:                      ; ladder mount / vertical nudge
        if (x & 0xF) == 0 and _cando(obj, x, y + 8*input.v, 1) and
                              !_cando(obj, x, y + 8*input.v, 4):
            changed = 1; y += 8*input.v; state = 3; p0f = 3
        elif state == 0 and dx == 0:
            dy = clamp(dy + 2*input.v, -3, 3)
            dx = 0; p1b = 0; p11 = 3
            result = dy > 1 ? 2 : (dy < -1 ? -2 : result)
    goto shared
```

Numbers: walking moves 8 px/frame (`input << 3`), the walk animation uses
frames `0x00..0x07` (`p0f & 7`), plus 8 for the right-facing half `0x08..0x0F`;
`p0f = 7` is the walk-start pose.
The jump impulse is `-(16 + 4*itemcount(9))` px/frame, i.e. -16 with no
feather item and up to -76 with a full inventory of item 9. The jump keeps
`dx = +/-1` for air control. `latch[0]` = `DGROUP:0x3D9E` is the "button 1
consumed" flag read by the input poll (`btn ^= latch`), which is how holding
Ctrl produces exactly one jump.

### 4.2 State 2 - airborne / water / ladder grab (`0x20D8A`) **[verified]**

```
changed = 1; p13 = 0
if (x & 0xF) == 0 and _cando(obj, x, y, 4) == 0:
    changed = 1; state = 3; p0f = 6; goto shared     ; inside a climb tile
p0f += 1
if p0f <= 2: goto shared                    ; 2 frames of hang, no motion
dy += 2
if dy > 0x10: dy = 0x10                     ; terminal velocity 16 px/frame
if input.h != 0: p0f = 2; dx = input.h
if p0f > 8: dx = 0
if _move(obj, x + 8*input.h, y + dy):       ; 26F4:1E70, mask 3
    goto shared
; blocked
if dy < 0:                                  ; ceiling
    row = (y - 1) & ~0xF
    if y == row: dy = 0
    elif _move(obj, x + 8*input.h, row): dy = 0
    goto shared
on_floor = (y & 0xF) == 0
if !on_floor and !_move(obj, x + 8*input.h, (y & ~0xF) + 0x10):
    on_floor = 1
if on_floor:
    state = 0
    p13   = 6
    p11   = (dy >= 0x10 ? -7 : -4) + (input.h != 0)   ; -7/-6 or -4/-3
    dx = 0; dy = 0
    sound(1, 2)
goto shared
```

`_move` is `FUN_26f4_1e70(obj, x, y)`: it tries `_cando(x, y, 3)`, then
`_cando(x, y_old, 3)` for a y-only move; on success it calls `_setorigin` and
stores `dx = input.h` (full move) or `dx = 0` (y-only), returning 1; blocked
returns 0. `p0f` counts frames since leaving the ground and doubles as the
jump/fall pose index; the landing pose length is encoded in the sign and
magnitude of `p11` (-4 = soft, -7 = hard landing, +1 when still steering).

The same state is entered from state 0 when the tile row under the feet is
class 3 (water / climb / hint wall), which is the "swim" entry: no gravity for
2 frames, then `dy` grows to 16, and the player can steer with left/right.

### 4.3 State 3 - climb (`0x21107`) **[verified]**

```
if input.h != 0:
    if p0f == 6: goto shared                ; ignore horizontal right after grab
    if _cando(obj, x + 8*input.h, y, 1):
        p13 = 0; changed = 1
        _setorigin(obj, x + 8*input.h, y)
        state = 2; p0f = 2; dx = input.h
        if btn1: latch[0] = 1; dy = -0xC; sound(1, 1)
        else:    dy = -4
    goto shared
dx = 0
if p0f == 6: p0f = 0
if input.v == 0: goto leave
newy = y
if input.v < 0: newy -= climb_step[p0f]     ; table 0x11D7
else:           newy += 4
if _cando(obj, x, newy, 1) == 0:            ; not open
    newy = input.v > 0 ? (y & ~0xF) + 0x10 : (y - 1) & ~0xF
    if _cando(obj, x, newy, 1) == 0: newy = 0
if newy == 0: goto leave
if input.v < 0:
    p0f -= 1
    if p0f >= 6: p0f = 0
else:
    p0f = 2
changed = 1
_setorigin(obj, x, newy)
if _cando(obj, x, y, 4) != 0:               ; no climb tile left
    state = 0; dx = 0; p1b = 0
leave:
if p0f < 0: p0f = 5
elif p0f > 6: p0f = 6
goto shared
```

Climb-up steps by pose: `4,0,0,6,4,4,0` (hands-over-hands animation), climb-down
is a flat 4 px per frame with pose 2. Reaching a cell without a climb tile
(bit 2 set) drops back to state 0, so the top of a vine/ladder is the first
frame where the box no longer contains the climb tile.

### 4.4 State 4 - recover (`0x20B3D`) **[verified]**

```
changed = 1
w = kind_width[0]                           ; 0x10
h = kind_height[0]                          ; 0x20
if p11 >= 0x20:
    state = 0; dx = 0; p1b = 0; dy = 0; changed = 1
goto shared
```

Entered only from `FUN_26f4_04c7` (level reset / respawn, which sets state 4
and `p11 = 0`) and from the "warp/checkpoint" object handler at `1644:18699`
that places the player at another object's position and sets state 4. It
restores the normal 16x32 body (the death state uses the 0x20x0x20 corpse),
uses `p11` as a 0x20-frame timer and then returns to ground control.

### 4.5 State 5 - death (`0x20BE4`) **[verified]**

```
if p11 == 0 and p0f == 2:                   ; zapper death (dir = 2)
    dy = min(dy + 2, 0x10)
    if _justmove(obj, x, y + dy): p11 = -1
if p11 == 1 and p0f == 2:
    w = 0x20; h = 0x20
    _setorigin(obj, x, y + 0x10)
if p11 >= 0x14:
    health = 6
    FUN_26f4_04c7(1)                        ; reset level -> player state 4
p11 += 1
return 1                                    ; always "changed"
```

`_playerkill(amount, dir)` (`26F4:05E7`) is the only writer:

```
if obj.kind == 0x17: return                 ; TINY form is immune
if obj.kind == 0 and state_flags[state] & 2: return
if amount - itemcount(10) <= 0: return      ; item 10 = shield
health -= amount - itemcount(10)
flags |= 0xC000; hud_flash = 1
if health >= 1: sound(4, 0x13); return
health  = 0
kind    = 0
w = 0x10; h = 0x20
state   = 5
p11     = 0
p0f     = dir                              ; 0/1 = normal, 2 = zapper
if dir == 1: y = (y - 1) & ~0xF
dy      = -0xC
sound(4, dir + 0x27)
_explode1(x, y, 10)
```

`kind 10 = ZAPPER` and the hazard-tile hook (spikes, thorns, stalactite
beds, water surface) call `_playerkill(0x10, 2)`, which is the only `dir = 2`
caller, hence the swelling/sinking variant of the death animation; all other
callers pass 0 or 1 and use the plain corpse frames `0x30 + p11/4` (dir 0) or
`0x40 + p11/4` (dir 1). The respawn happens 20 frames after death
(`p11 >= 0x14`), health is restored to 6 and `FUN_26f4_04c7(1)` reloads the
board state, which leaves the player in state 4 (recover) for 0x20 frames.

---

## 5. Shared tail (`state 1`, `0x214D0`) **[verified]**

Every state except 4 and 5 ends by jumping here:

```
if dx != 0: p1b = dx                        ; remember facing
if btn2 and state_flags[state] & 1 and p1b != 0:
    latch[1] = 1                            ; 0x3DA0
    n = number of active objects with kind 0x32 (SPINBLAD)
    if n < itemcount(8):
        o = _addobj(0x32, x + p1b*0xC + 4, y + 2)
        o.dx = p1b * 6
        sound(2, 0x22)
    elif delitem(2):                        ; KNIFE in inventory
        o = _addobj(2, x + p1b*0xC + 4, y + 2)
        if _cando(o, o.x, o.y, 1) == 1:    ; the spawn cell must be open
            o.dx  = p1b * 8
            o.p11 = 3
            sound(2, 3)
        else:
            _killobj(o); _addinv(2); sound(2, 8)   ; refund if placement fails
    else:
        sound(2, 8)                         ; dry click
p11 += 1
if p13 != 0:
    changed = 1
    p13 -= 1
FUN_26f4_23e1(0)                            ; tile events for this object
out = result
1FAE:000D(out)                              ; camera edge nudge (below)
return changed
```

Notes:

* SPINBLAD placement is capped by `itemcount(8)` (max 15 blades in flight) and
  does **not** set `p11`; KNIFE placement sets `p11 = 3`.
* `delitem` = `FUN_26f4_1a38(test-and-remove)`.
* `FUN_26f4_23e1(obj)` scans the cells under the object; for every cell whose
  tile attribute has bit 8 it calls `FUN_1f28_0006(col, row, 1)` (the same
  script hook as #1.1), after the `state_flags & 2` early-out.
* `1FAE:000D` (near call, arg = the update result) is the camera-edge
  auto-scroll: it sets `DGROUP:0xBAB0` to `+8` (or `+4` for the 4-px-wide TINY
  form on EGA/VGA) when the player is near one edge of the viewport, and to
  the negative step at the other edge. The value is consumed by the scrolling
  code. **[inferred]**
* Hazard tiles (`FUN_1f28_0006(col, row, 1)`, reached through
  `FUN_26f4_23e1`) - the tile id picks the effect: **[verified]**

  | tiles | name(s) | effect |
  |---|---|---|
  | `0x186..0x18B` | `LAVA1..LAVAB` | `_playerkill(0x10, 1)` + `_explode2(0)` |
  | `0x141/0x142` | `STALACBR`/`STALAGBR` | `_playerkill(0x10, 2)` |
  | `0x155/0x156` | `WTHORN`/`WTHORN2` | `_playerkill(0x10, 2)` |
  | `0x0A6` | `SPIKE` | `_playerkill(0x10, 2)` |
  | `0x190..0x197` | water surface | `_playerkill(0x10, 1)` unless the player kind is `0x36` (JILLFISH) or `0x39` (JILLFROG) |
  | `0x198..0x1A3` | water body | `sound(2, 4)` only, throttled to one per `0xA` frames (`DGROUP:0x439E` = last frame) |

  Mode 1 returns before the scan when `state_flags[state] & 2` is set (dying /
  recovering). Mode 0 is the cell repaint. Mode 2 is the decorative pre-pass
  (tile attribute bit `0x20`): for `LAVA1..LAVA5` (`0x186..0x18A`) it advances
  the cell id by one every 4th frame (`frame & 3 == 0`), wrapping `0x18A` back
  to `0x186`, which is the 5-frame lava animation. Mode 2 also returns 1 for
  `SPIKE` (so the cell is repainted) and `MAPDOOR`/`MIST` special cases.
* The attack latch is what makes a held Alt throw only one weapon: the input
  poll does `btn ^= latch` and clears the latch on release.

---

## 6. Tile tests and collision classes **[verified from data + code]**

`_cando(obj, x, y, mask)` (`26F4:2227`) walks every 16x16 cell covered by the
box `(x, y, w, h)` and computes

```
result = 0xFFFF
for each cell:
    result &= (tile_flags[cell] | extra) & mask
return result
```

`tile_flags` is the word at `DGROUP:0x83EE + tile*8` (= `0x4006 ^ attr` of the
`.dma` record, see `jill1-engine.md` #8). `extra` is 2 for rows inside the
object's height and 0 for rows at/below its bottom edge; the edge test uses
the object's **current** y (`DGROUP:0x96AF`), not the candidate y, so an
object resting exactly on a tile boundary cannot sink into the tile below it.
Masks used by the player:

| mask | meaning |
|--|--|
| `1` | "open" box: every cell has bit 0 (air/blank tiles and climb tiles pass, solid tiles do not) |
| `3` | water/climb/hint class (bits 0+1); used for the "water below" test |
| `4` | climb surface test: returns `4` only if **every** cell has bit 2 set, i.e. `0` means "a climb tile is in the box" |
| `0x2001` | fish/water movement (`_fishdo`), water and open tiles pass |
| `0x200` | channel/water test used by the TINY form |

The low three bits of the tile flags for the level-1 table (`jill.dma`):

| flags | tiles | class |
|--|--|--|
| `0x6` | blank default, `STONEWALL`, `WHITEWALL`, `BREAKWALL`, `BRFLOOR`, `FFLOOR`, `BRIDGEM/L/R` (0x7 after `^ attr` for bridges) | solid |
| `0x7` | `HINTWALL`, `BRIDGE`, `WATERTL*`, `WATERML*`, `STILLWATER` | solid + special (bit 0 also set) |
| `0x3` | `WHITECLIMB` | climbable |
| `0x1` | `VINEBR` | climbable / vine |
| `0x5` | `BRIDGE` | bridge |

(Some rows depend on the `.dma` used by the loaded board; the parser should
read `0x4006 ^ attr` rather than hardcode these.) Note the two-component
scheme: bit 0 is "no solid block", bit 2 is "not a climb surface", so
`WHITECLIMB` (`0x3`) is both open and climbable, while a floor (`0x6`) is
solid.

---

## 7. Drawing summary (state -> frame) **[verified]**

Draw table `1FAE:0x0258`: `0 -> 0x264, 1 -> 0x792, 2 -> 0x4BD, 3 -> 0x599,
4 -> 0x5BA, 5 -> 0x673`; the handler sets `DI = tileset << 8` (`0x800` for the
player, from `DGROUP:0xBB3E`) and each state adds a frame offset to it; all
paths end in `_drawshape(vp, DI, x, y)`.

| state | frame selection |
|--|--|
| 0 | `p11 < 0` -> landing frames `0x3C` (`p1b == 0`), `0x24`/`0x2C` (`p1b <0`/`>0`), `+1` at `p11 == -4/-5`; `dy < 0` -> `0x13`; `dy == 3` -> `0x3D`; `dy > 0` -> `0x12`; walking -> `(p0f & 7)` plus 8 when `dx > 0` (so `0x00..0x07` left, `0x08..0x0F` right), `p0f == 8` -> `0x14/0x15`; just stopped (`p1b != 0`, `p11 < 0x14`) -> `0x14/0x15`; idle -> `0x10` (`0x11` when `p11 > 0x96`), plus the demo-caption logo when `p11 > 0x10C` |
| 2 | `dx == 0`: `0x38`/`0x39` for `p0f` 0/1, else `0x3A`/`0x3C` by `dy` sign; `dx != 0`: `0x20 + p0f` (0/1) or `0x22`/`0x24` by `dy`, `+8` when `dx > 0` |
| 3 | `0x11D0[p0f]` (`0x18/0x19/0x1A`, climb) |
| 4 | `p11 < 0x10` -> `0x13`, `< 0x18` -> `0x10`, else `0x12`; plus a 16x16 overlay blitted from `DGROUP:0xB986` |
| 5 | `p11 == 0` -> `0x42`; `p0f == 0` -> `0x30 + p11/4`; `p0f == 1` -> `0x40 + p11/4`; `p0f == 2` -> `0x11DE[p11]` |

`p0f` therefore means "pose" only in states 0/2/3; in state 5 it is the death
variant and in state 4 the timer `p11` selects the frame.

---

## 8. Idle / caption timer (`p11` in state 0) **[verified code, inferred purpose]**

While the player stands still, `p11` counts up and at

* `0x102` (258 frames) a random one of the four demo captions is printed from
  the level-title table at `DGROUP:0x2491` (`rand() & 3`, string shown for
  `0x19` frames via `DGROUP:0xB6B4`);
* `0x10C` a matching logo sprite (far pointers at `DGROUP:0x11B0`) is drawn;
* `> 0x12C` `p1b` is cleared and `p11` restarts at `0x14`.

This is the attract-mode overlay that appears during the demo playback; a port
that does not replay demos can drop it, but the counter itself must exist
because `p11` is also the landing/climb sub-state (see #4.1).

---

## 9. Determinism notes

* Fixed 18.2 Hz update (or 9.1 Hz with `T`), not delta-time based.
* `rand()` = `FUN_1CA6_0015` (Turbo C `rand`, LCG seeded at boot). It is used
  for the jump-caption index, bubble spawns and enemy AI; the port needs the
  same LCG (or a recorded seed) for demo replay.
* Update order is fully determined by the active list: player first, then
  ascending object index.
* The input edge latches (`0x3D9E`/`0x3DA0`) are game state: the player sets
  them when a jump/throw is consumed, the input poll clears them when the
  button is released. A port that maps keys to "held" state without these
  latches will auto-repeat jumps and throws.
* `_play` services the sound driver before the input poll and the object
  update, so a sound-triggered wait (SB DSP) stalls a frame at a different
  point than the rest of the logic.

---

## 10. J2 / J3 deltas **[verified]**

The player handler is the same code in all three episodes (see
`shared-handler-equivalence.md`); only the DGROUP layout and a few call
offsets differ. Verified per-episode values relevant to this document:

| item | J1 | J2 | J3 |
|--|--|--|--|
| player handler | `1FAE:01D8` | `1FC1:01E0` | `1FCA:01D0` |
| update / draw tables | `1FAE:0868` / `0258` | same code, `1FC1` | same code, `1FCA` |
| pose table copies | `11D0/11D7/11DE` | `126C/1273/127A` | `136A/1371/1378` |
| input h / v | `3C76/3C78` | `39B4/39B6` | `3714/3716` |
| button 1 / 2 | `3C7A/3C7E` | `39B8/39BC` | `3718/371C` |
| input latch 1 / 2 | `3D9E/3DA0` | `3ADC/3ADE` | `383C/383E` |
| state flags (`BE2C`) | `BE2C` | `BB66` | `BC08` |
| kind flags (`BE3C`) | `BE3C` | `BB76` | `BC18` |
| frame counter | `BE3A` | `BB74` | `BC16` |
| tileset / palette | `BB3E` | `B878` | `B61A` |
| player w / h tables | `B6B6/0xB740` | `B3F0/0xB47A` | `B152/0xB1DC` |
| camera far pointer | `B7F2` | `B52C` | `B28E` |
| health (header+2) | `83A8` | `80E2` | `7E44` |
| map / header | `43A6/83A6` | `40E0/80E0` | `3E42/7E42` |
| tile flags (`map+0x4048`) | `83EE` | `8128` | `7E8A` |
| object array / count | `96AC/BB1C` | `93E6/B856` | `9148/B5F8` |
| visible count / active list | `BECC/BCAC` | `BC06/[open]` | `BCA8/[open]` |
| death `_playerkill` | `26F4:05E7` | same module | same module |

There is **no single delta**: the low data area follows `J2 = J1 - 0x2C2`,
`J3 = J1 - 0x562` for the input block, the map/object arena follows `-0x2C6`
and `-0x564`, and the flag area follows `-0x2C6` / `-0x224`. This is why the
port must take every offset from `docs/profiles.json` instead of computing it.

Unresolved per-episode offsets: J2/J3 active-list base, J2/J3 caption logo
pointer table (`0x11B0`), J2/J3 idle-caption random index (`0x43A0`).

---

## 11. Port checklist

1. Load the episode profile, not the J1 constants in this file.
2. Implement the update pass exactly as in #2 (player first, active list
   rebuilt per frame, `0xC000` semantics, symmetric hit pass).
3. Implement the player as the state machine in #4 plus the shared tail in #5;
   `dx` is a direction and `dy` is px/frame.
4. Keep `p0f`, `p11`, `p13`, `p1b` as separate 16-bit fields with the same
   meanings; the draw code and enemy code (kind 23/54-57 reuse the player
   handler) depend on them.
5. Keep the input edge latches and the `state_flags` table
   `{1,0,1,0,2,2}` (attack allowed in states 0/2, invulnerable in 4/5).
6. Fixed 18.2 Hz tick, same `rand()` LCG, same sound service point in the
   frame, if demo replay is a goal.
