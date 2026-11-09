# Jill 2 / Jill 3 - the five new object kinds

Companion to `jill1-kind-handlers.md`, `jill2-differences.md` and
`jill3-differences.md`. Jill 2 adds five handlers on top of the 64 Jill 1 kinds;
Jill 3 reuses them unchanged. This file documents what kinds 64-68 actually do.

Confidence markers: **[verified]** = read out of the disassembly and consistent
across call sites; **[inferred]** = reading of intent; **[open]** = not confirmed.

Everything not listed here is identical to Jill 1: the same 3-call ABI
(`handler(obj, action, other)` with actions 0 draw / 1 hit / 2 update / 3-5
trigger), the same 31-byte object record, the same 69-entry kind table. Only the
DGROUP offsets and the handler module addresses differ between the two episodes.

---

## 1. Kind table entries

| Kind | Name | w x h | flags | tileset | count | J2 handler | J3 handler |
|--|--|--|--|--|--|--|--|
| 64 | EYE | 16 x 12 | `0x0180` | 62 | 3 | `1A6C:46A9` | `1A51:483A` |
| 65 | VINECLIMB | 16 x 8 | `0x0000` | 61 | 0 | `1A6C:481E` | `1A51:49B1` |
| 66 | FLAG | 36 x 16 | `0x0008` | 5 | 0 | `1A6C:4A18` | `1A51:4BAA` |
| 67 | MAPDEMO | 64 x 16 | `0x0000` | 3 | 0 | `1A6C:4C4D` | `1A51:4DF2` |
| 68 | ROMAN | 16 x 32 | `0x0480` | 44 | 12 | `1671:2F36` | `1650:2F6B` |

Addresses are runtime segments as stored in the kind table; the file offset is
`linear - 0x10000` with `linear = 0x10000 + segment*16 + offset`. **[verified]**

The `count` column is the number of extra animation bytes appended to the kind
table entry (the same convention as Jill 1). Only EYE (3) and ROMAN (12) carry
per-kind extra data here.

Flag delta between the episodes: none for 64-67. Jill 3 changes DEMON (58)
`0x0480 -> 0x0000` and TOKEN (63) `0x0000 -> 0x0008`, which is outside this set.

---

## 2. Shared helpers used

All five handlers are ordinary object handlers and use the same helpers the Jill 1
handlers do. Jill 3 addresses shown; the Jill 2 image has the identical code at
shifted module addresses.

| Helper | J3 far address | Purpose |
|--|--|--|
| `_drawshape` | `110C:00DE` | blit a shape; `(tileset<<8) | frame` |
| `_vectdist` | `273B:2634` | `vectdist(kind, obj, &dx, &dy, scale)` - signed pixel offsets toward an object |
| `_cando` | `273B:2221` | test whether a move is free of blocking cells |
| `_setorigin` | `273B:1D6D` | snap x/y back to the last unblocked position |
| `_justmove` | `273B:2323` | move without collision test |
| `_crawl` | `273B:1F2F` | move with collision test, returns non-zero on success |
| `_hitplayer` | `273B:25B2` | apply damage to the player |
| `_addscore` | `273B:1FAC` | add score and place a floating score popup |
| `_sound` | `0576:01E8` | play SFX (bank, index) |
| `_rand` | `2D18:0015` | pseudo-random word |
| `_explode1` | `110C:AF70` | spawn explosion particles |

---

## 3. EYE (64)

16 x 12, tileset 62 (5 frames), kind flags `0x0180`. Placed 3x in Jill 3 boards;
eye art is a wall-mounted eye whose pupil follows Jill. **[verified]**

### draw (action 0)

Two shapes per tick: the eye body, then the pupil on top of it.

```
drawshape(tileset, 0, obj.x,            obj.y)
drawshape(tileset, 1, obj.x + obj.dx+5, obj.y + obj.dy+4)
```

`obj.dx` / `obj.dy` are the pupil offsets maintained by `update`; at spawn they
are 0, so the pupil sits at a fixed position inside the body. **[verified]**

### update (action 2)

```
_vectdist(0, obj, &dx, &dy, 3)      ; offsets toward the player, scale 3
dy = (dy >= 1) ? 1 : -1
dx = max(dx, -2)                    ; clamp left travel only
if dx != old_dx or dy != old_dy:
    obj.dx = dx; obj.dy = dy
    return 1                        ; "moved" -> keep the object flagged active
return 0
```

Notes:
* The pupil is clamped asymmetrically: it can travel at most 2 px left of centre
  but is not clamped to the right (`_vectdist` already bounds it by the eye's own
  extent). **[verified]**
* Returning 1 only on a change means the object marks itself "blocked"
  (`0xC000` in the flags word) on frames where the pupil is stationary, and the
  engine's map-flag OR sweep skips it. **[inferred]**
* The eye has no hit response of its own; it is a static hazard/scenery body.
  Damage to the player comes from the object's normal AABB collision with the
  player handler, not from this handler. **[inferred]**

---

## 4. VINECLIMB (65)

16 x 8, tileset 61 (4 frames), kind flags `0x0000`. This is the climbing vine /
snake hazard: it bobs up and down and hurts Jill on contact. Placed 21x in Jill 3
and 10x in Jill 3 board 12 alone. **[verified]**

### draw (action 0)

```
drawshape(tileset, obj.p13 / 2, obj.x, obj.y)
```

Frame is `p13/2`, so the 8-value animation counter steps through the 4 frames at
half speed. **[verified]**

### update (action 2)

```
obj.p13 = (obj.p13 + 1) & 7
if obj.p13 == 0:
    obj.dy = 2
if _cando(obj, obj.x, obj.y + obj.dy, 4):
    _setorigin(obj)                 ; blocked -> snap back, stop bobbing
else:
    obj.y += obj.dy                 ; free -> move
    if obj.y hit the end of travel:
        obj.dy = -obj.dy
```

The `p13 == 0` reload of `dy = 2` restarts a downward stroke once per 8 ticks;
each stroke reverses when the move is illegal. Net effect is a slow sine-like bob
of a couple of pixels. The `4` mask passed to `_cando` tests tile attribute bit 2
(see `jill1-engine.md` 6.3); because stored attrs are XOR `0x4006`, mask 4 means
"the cell is solid for this object". **[verified]**

### hit by player (action 1)

```
if player-state flags & 0x04 == 0:      ; not invulnerable
    x = player.x - 8
else:
    x = player.x + 8
_justmove(0, x, player.y)               ; push the player sideways off the vine
player.state = 0
player.p13  += 0x0F
_hitplayer(this)                        ; damage 1, ttl cooldown 3
return 1
```

The `+0x0F` on the player's parameter word is the knock-back/stun counter used by
several Jill 1 handlers. The check of bit 2 of the player's flag table is the same
invulnerability test the Jill 1 hazards use. **[verified]**

---

## 5. FLAG (66)

36 x 16, tileset 5 (4 frames), kind flags `0x0008` (collides with other objects).
This is the checkpoint / level-complete flag. Placed 21x in Jill 3. **[verified]**

### draw (action 0)

```
drawshape(tileset, obj.p13 / 2 + 1, obj.x + 2, obj.y)
```

Frame base 1 means the flag art occupies frames 1-4 of the tileset (frame 0 is
unused or is the bare pole). `+2` on x is the small visual offset seen in the
board data. **[verified]**

### update (action 2)

```
obj.p13 = obj.p13 + 1
if obj.p13 >= 6: obj.p13 = 0
return (obj.p13 & 1) ? 0 : 1
```

Six-tick cycle, returning 1 on even values only: the flag toggles between the
"blocked" and "transparent" state three times per cycle. This is a **one-shot**
animation - unlike the vine it never moves. **[verified]**

### hit by player (action 1)

```
if obj.state == 0:
    obj.state = 1
    _addscore(6, obj.x, obj.y)
    _sound(2, 0x20)
    _explode1(obj.x, obj.y, 5)
    return 1
```

The `state` byte gates it, so the score/sound/explosion fire once per flag. Score
id 6 and SFX bank 2 index `0x20` are the "found item" pair used by the gem and
knife pickups in Jill 1. `_explode1` with count 5 is the small sparkle burst.
**[verified]**

Note the flag is **not** a checkpoint save in the engine sense - it is a pickup
worth score. Map-progress/checkpoint state is separate. **[inferred]**

---

## 6. MAPDEMO (67)

64 x 16, tileset 3 (25 frames), kind flags `0x0000`. Not placed in any shipped
board; it is the animated "map" decoration used by the overworld screen.
**[verified]**

### draw (action 0)

The handler copies two 2-word tables out of DGROUP at entry (both Jill 2 and
Jill 3 hold the same values):

| Episode | base table | length table |
|--|--|--|
| Jill 2 | `DGROUP:0x10FC` | `DGROUP:0x1100` |
| Jill 3 | `DGROUP:0x120C` | `DGROUP:0x1210` |

Both hold `[0x10, 0x14]` for the base frames and `[4, 3]` for the run lengths.
Then:

```
if byte[DGROUP:0x34EB] == 0: return     ; not in the map/video mode
for run in 0..1:
    base   = frames[run]
    length = lengths[run]
    for i in 0..length-1:
        drawshape(tileset, (base + i) | 0x4000, x + i*16, y)
```

The `0x4000` bit is part of the tileset byte (tilesets `0x40`..`0x7F` select shape
entries `0x00`..`0x3F`); the blitter XORs it out and does not mirror pixels, so the
two runs are not flipped. `.sha` entries `0x40`..`0x7F` are empty when decoded. Runs are
drawn 16 px apart, matching the 16 px tile grid. The guard byte at `0x34EB` is the
video-mode selector set by the title/menu/intro code (values 0/2/4 written at
`re/img3.bin` linear `0x11C10`, `0x11C17`, `0x11C1E`, and from `main` at
`0x150C2`); MAPDEMO only draws while it is non-zero. **[verified]**

### update (action 2)

```
if word[DGROUP:0x13A8] == 0:
    obj.x = viewport.x + 0x10
    obj.y = viewport.y + 4
    return 1
return 0
```

While the map demo flag is clear the object is pinned to the top-left of the
viewport, so the ribbon scrolls with the screen and never leaves it. The `+0x10,
+4` is the same inset the intro/title overlay uses. **[verified]**

There is no hit response; MAPDEMO is pure overlay. **[verified]**

---

## 7. ROMAN (68)

16 x 32, tileset 44 (7 decoded frames, kind claims 12), kind flags `0x0480`.
Not placed in any shipped board - it is the "Roman" walker enemy that only
appears in the unreleased/debug content. **[verified]**

### draw (action 0)

```
frame = (obj.dx > 0) ? 8 : 0
drawshape(tileset, frame + obj.p13, obj.x, obj.y)
```

`obj.p13` is an 8-step walk cycle, so the handler asks for frames 0-7 facing
left and 8-15 facing right. **[verified]**

**Art inconsistency [open]:** `jill1.sha`, `jill2.sha` and `jill3.sha` entry 44
all declare **7 frames** (1008 / 2380 / 3948 byte size table), yet frames 8-15
are requested when facing right. Either the shipped art is truncated relative to
the handler, or the right-facing frames are aliased onto the same 7 frames by
the shape reader. ROMAN is placed in no shipped board, so this never manifests in
the release data.

### hit by player (action 1)

```
if obj[0x1B] == 0:
    _hitplayer(this)
```

Guarded by the `0x1B` word so the contact only damages once per engagement.
**[verified]**

### update (action 2)

```
obj.p13 = (obj.p13 + 1) & 7
if _rand() % 30 == 0:                  ; 1-in-30 re-aim
    _pointvect(obj)                    ; re-aim dx/dy toward the player
obj.dy = 0
obj.dx = abs(obj.dx) * sign(obj.dx)    ; keep direction, drop magnitude sign issues
if _crawl(obj, obj.dx, 0):
    _sound(1, 0x11)
else:
    obj.dx = -obj.dx                   ; reverse on blocked
return 1
```

Behaviour: a ground walker that re-aims at the player roughly once every 30
ticks, otherwise continues in its current horizontal direction and reverses when
it hits a wall. The `_sound(1, 0x11)` on a successful step is the footstep/step
SFX, not a hit sound. **[verified]**

`_crawl(obj, dx, 0)` is the same collision-aware mover the Jill 1 ground
enemies use, and it returns non-zero only when the move succeeded.

---

## 8. Placement in shipped boards

Counts over all boards per episode. **[verified]**

| Kind | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| EYE (64) | - | 0 | 3 |
| VINECLIMB (65) | - | 0 | 21 |
| FLAG (66) | - | 0 | 21 |
| MAPDEMO (67) | - | 0 | 0 |
| ROMAN (68) | - | 0 | 0 |

Jill 3 uses the new kinds heavily in the later levels: board 12 places ten
VINECLIMB objects and board 7 places fifteen FLAGs. EYE appears three times.
Jill 2's eleven boards place none of them, so the handlers existed before the
content that uses them. MAPDEMO and ROMAN are unreferenced in both episodes -
they belong to the map/intro overlay and to cut content respectively.

Board objects spawned with these kinds start with `state = 0` and `p13 = 0`
(VINECLIMB boards 9 and 12 use `p13` 2/4 to de-synchronise the bob phase).
**[verified]**

---

## 9. Jill 2 vs Jill 3

The five handlers are **behaviourally identical** between the two episodes; only
addresses and DGROUP constants move. **[verified]** by disassembling both images
and comparing instruction-for-instruction.

| Item | Jill 2 | Jill 3 |
|--|--|--|
| kind table | `DGROUP:0xB878`... | `DGROUP:0xB61A`... |
| objects array | `DGROUP:0x93E6` | `DGROUP:0x9148` |
| EYE handler | `1A6C:46A9` | `1A51:483A` |
| VINECLIMB handler | `1A6C:481E` | `1A51:49B1` |
| FLAG handler | `1A6C:4A18` | `1A51:4BAA` |
| MAPDEMO handler | `1A6C:4C4D` | `1A51:4DF2` |
| ROMAN handler | `1671:2F36` | `1650:2F6B` |
| MAPDEMO base table | `0x10FC` | `0x120C` |
| MAPDEMO length table | `0x1100` | `0x1210` |
| video-mode byte | `0x34EB` | `0x34EB` |
| map-demo flag word | `0x13A8` | `0x13A8` |

The two MAPDEMO tables hold the same words (`[0x10,0x14]`, `[4,3]`) in both
episodes, so a port can read them from the image rather than hardcode them.

Jill 3's debug block names the handlers `_msg_eyes`, `_msg_vineclimb`,
`_msg_flag`, `_msg_mapdemo`, `_msg_roman` and `_msg_nullkind`, confirming the
kind-to-handler mapping. **[verified]**

---

## 10. Port status

Implemented in `port/src/engine.cpp` (draw / update / hit all present):

| kind | port notes |
|--|--|
| 64 EYE | body frame 0, pupil frame 1 at `x+dx+5, y+dy+4`; `vectdist` pupil tracking with the `dy = +/-1`, `dx >= -2` clamp |
| 65 VINECLIMB | `p13` bob, `_cando(...,4)` reversal, hit pushes the player 8 px and adds `0x0F` to `p13` before `hit_player` |
| 66 FLAG | 6-tick `p13` cycle, one-shot `state` gate, `+6` score, `explode1(...,5)` |
| 67 MAPDEMO | two runs `{0x10 x4, 0x14 x3}` at 16 px, pinned to `cam+0x10, cam+4`; drawn/updated only on the game's first (hub/world-map) board, the port's stand-in for `DGROUP:0x34EB != 0` |
| 68 ROMAN | `p13` walk cycle, 1-in-30 `pointvect`, `crawl` + reverse, step sound; right-facing art uses frames 8-15 when the sheet has them (Onesimus has 17), otherwise the left frames are mirrored in place |

Verified headlessly on `6.ONE` (EYE/VINECLIMB), `28.ONE` (ROMAN) and `MAP.ONE`
(MAPDEMO); the FLAG pickup fires once and raises the score.  MAPDEMO's port guard
is the hub-board test (the port has no separate video-mode byte), so it never
appears on ordinary levels even though the object is present in `MAP.ONE`.
ROMAN's 7-frame Jill 2/3 sheet is mirrored for `dx > 0`; Onesimus supplies real
right-facing frames 8-15. **[inferred]**

## 11. Open questions

1. Why entry 44 (ROMAN art) declares 7 frames while the handler requests 16 in
   Jill 2/3 (Onesimus ships 17). The port mirrors the left frames when the real
   right-facing set is absent. **[open]**
2. What writes `DGROUP:0x13A8` (the map-demo / viewport pin flag). It is zero in
   the shipped data, so MAPDEMO's update always pins; the draw guard at `0x34EB`
   is what actually suppresses it outside the map screen. **[open]**
3. Whether FLAG's score id 6 is the same table entry as the Jill 1 gem pickup, or
   a Jill 2-specific score value. The `_kindscore` table is per-episode. **[open]**
4. Whether the EYE pupil offsets are clamped on the right in practice; the code
   only clamps `dx >= -2`, so a player far right of the eye may push the pupil
   past the body. Not observable in the three EYE placements. **[open]**
