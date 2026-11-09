# Jill of the Jungle - object kind handlers

Companion to `jill1-engine.md` section 6. This file documents what each of the 64
object kinds actually *does*: its 3-call protocol, per-tick movement, animation,
collision response, sounds and spawns.

The handlers are shared with Jill 2 and Jill 3. See
`shared-handler-equivalence.md` for the comparison result and the three Jill 3
edits (DEMON 7, TOKEN 28, EPIC 49), `jill23-new-kinds.md` for kinds 64-68, and
`jill23-sounds.md` for the `_sound` id remap and per-kind call-site table. The
player's own state machine and the per-frame update order (kind 0) are covered
in detail by `jill1-player-physics.md`.

Confidence markers: **[verified]** = read out of the disassembly/decompiled code and
consistent across call sites, **[inferred]** = reading of intent, **[open]** = not
confirmed.

Every handler is called as

```
result = handler(dummy_str_ptr, obj_index, action, other_index)
  action 0  draw yourself          other = 0
  action 1  you were hit by object other_index (0 = by the player)
  action 2  update yourself        other = 0
  action 3  trigger (neutral)      other = the object that sent it
  action 4  trigger "on"/pressed
  action 5  trigger "off"/released
  non-zero return = "blocked / stop"
```

Actions 3/4/5 are the three trigger codes `_sendtrig(id, code, self)` sends out
(code 3 = neutral, 4 = on, 5 = off). Most handlers only implement 3, but the
jump tables are 6 entries wide and a few use 4/5 as separate steps - BRIDGER (26)
and KNIGHT (44) do exactly that.

## The shared object API

Handlers are thin: nearly every one is a state machine over these engine routines
(all in segment `0x26F4`, `JOBJ.C`). Names are inferred from behaviour and from the
Turbo C symbol list; addresses are exact.

| Address | Name (inferred) | Behaviour |
|--|--|--|
| `26F4:1892` | `_addobj(kind,x,y)` | append a new object, init x/y/size/flags from the kind table, returns its index (array cap `0x100`) **[verified]** |
| `26F4:1ad4` | `_setorigin(obj,x,y)` | set x/y, clamped to `0..0x7F0` / `0..0x3F0` **[verified]** |
| `26F4:2227` | `_cando(obj,x,y,mask)` | AABB test against the per-tile attribute table at DGROUP:`0x83EE` (8 bytes/tile); returns `mask` if every covered cell satisfies `attr & mask`, else 0 **[verified]** |
| `26F4:1d73` | `_justmove(obj,x,y)` | `_cando(...,1)` then `_setorigin` **[verified]** |
| `26F4:1c89` | `_trymove(obj,x,y)` | try the full move, then x-only, then y-only (wall sliding); returns 1/2/4 for the axis that worked, 0 if stuck **[verified]** |
| `26F4:1f35` | `_crawl(obj,dx,dy)` | `_trymove` plus a floor/step probe (`26F4:1b8f`); used by walking enemies **[inferred]** |
| `26F4:2605` | `_fishdo(obj,x,y)` | move test with mask `0x2001` (water is walkable) **[inferred]** |
| `26F4:1da8` | `_onscreen(obj)` | AABB vs the camera rect pointer at DGROUP:`0xB7F2` **[verified]** |
| `26F4:06b4` | `_pointvect(obj,&dx,&dy)` | writes -1/0/+1 signs of the direction from `obj` to object 0 (the player) **[verified]** |
| `26F4:263a` | `_vectdist(a,b,&dx,&dy,speed)` | normalised velocity vector of the given magnitude towards object b **[verified]** |
| `26F4:2772` | `_dist(a,b)` | Manhattan distance between two objects **[verified]** |
| `26F4:1863` | `_killobj(obj)` | `flags |= 0xC000`, `state = 3` (the "poof" state) **[verified]** |
| `26F4:0091` | `_killaward(obj)` | `_addscore(kscore[kind], x, y)` + `_stampfootprint` + `_killobj`; `kscore` is the kind table's "default count" at DGROUP:`0xBBC8` **[verified]** |
| `26F4:1fb2` | `_addscore(n,x,y)` | spawn a kind-27 `SCORE` popup carrying `n` and add `n` to the 32-bit score **[verified]** |
| `26F4:25b8` | `_hitplayer(obj)` | if `obj.ttl == 0` and the player is not invincible: `_playerkill(1,0)`; then `obj.ttl = 3` (hit cooldown) **[verified]** |
| `26F4:05e7` | `_playerkill(dmg,dir)` | subtract health, set the damage flash, start the death sequence at 0 **[verified]** |
| `26F4:1a13` / `1a38` | `_addinv` / `_takeinv` | append / remove a key-inventory entry **[verified]** |
| `26F4:1a89` | `_invcount(id)` | how many inventory entries equal `id` **[verified]** |
| `26F4:20ef` | `_sendtrig(id,a,b)` | call the handler of every object that has `kflags & 0x100` and `p13 == id` **[verified]** |
| `26F4:27dd` | `_touchbkgnd(obj,x,y)` | clear tile `0xA7` cells under the footprint, first hit spawns `_explode1(5)` + sound `2/0x31`; returns the count **[verified]** |
| `26F4:012d` | `_stampfootprint(obj)` | OR `0xC000` into every map cell the AABB covers **[verified]** |
| `26F4:020c` | `_setobjsize(obj)` | reset w/h from the kind table; text kinds (`0x14`,`0x15`) derive width from the string, `SCORE` from its digits **[verified]** |
| `1A41:09a8` | `_explode1(x,y,n)` | spawn `n` particles (kind `0x24`) with random dx/dy **[verified]** |
| `1A41:0930` | `_explode2(obj)` | spawn one particle (kind `0x25`) at the object's centre **[verified]** |
| `2CA6:0015` | `_rand()` | 15-bit pseudo-random **[verified]** |
| `1123:00eb` | `_drawshape(vp,shape<<8\|frame,x,y)` | blit; the high tileset bit `0x40` (id bit `0x4000`) is XORed out before the entry lookup and is **not** a mirror - the blitter never reverses pixels **[verified]** |
| `1581:0178` | `_sound(chan,id)` | play sample `id` on channel/priority `chan` **[verified]** |
| `22AB:05b4` | `_putbotmsg(str,seg,col)` | show a timed bottom-of-screen message **[verified]** |

Per-kind statics baked into DGROUP (copied to the stack at handler entry with
`memcpy`): animation frame tables (`0xDGROUP:C2A`, `0xC32`, `0xC62`, ...) and
per-frame y/wiggle offsets. Several handlers therefore animate identically but with
different tables.

The object record layout is in `jill1-engine.md` 6.1; `+0x0D` is `state`, `+0x13` is
the single free parameter (`p13`) most handlers use as a frame/animation counter, and
`+0x1D` (`ttl`) doubles as a hit cooldown for enemies.

---

## Per-kind notes

Format: kind, name, handler, then `draw` / `hit` / `update`.
Shapes are written `tileset<<8 | frame`. `O(i).f` is object field `f` (see 6.1).

### 0. PLAYER (1fae:01d8) [verified]
- draw: switch on `state` via table `1fae:0x258` (0->0x264 walk/frame picker, 1->0x792 shared blit tail, 2,3,4,5); all paths end in `_drawshape(vp, 0x08<<8 | frame, x, y)`. Frame comes from `p0f` (pose: -1 unsupported, 0/1 idle, 2 water, 6 climb, 7 walk), `p11`, `dy` and `dx` sign.
- update: switch on `state` via table `1fae:0x868` (0->0x874, 1->0x19f0, 2->0x12aa, 3->0x1627, 4->0x105d, 5->0x1104).
- inputs are read from DGROUP (filled by the keyboard/joystick handler): `0x3C76` = left/right (-1/0/+1), `0x3C78` = up/down, `0x3C7A` = button 1 edge, `0x3C7E` = button 2 edge.
- states (full description in `jill1-player-physics.md` #4): 0 = ground/air control (walk sets `dx = 0x3C76`, `p0f = 7`; `_cando(x, (y & ~0xF) + 0x10, 3)` selects state 2), 1 = shared tail (never stored; reached by `jmp` from every other state: facing update, weapon throw, `p11`/`p13` tick, tile events, camera nudge), 2 = airborne / water / ladder grab (`_cando(...,4) == 0` -> state 3, `p0f = 6`; gravity `dy += 2` capped 0x10; landing sets state 0 and `p11 = -4/-7`), 3 = climb (button 1 jumps off: `state = 2`, `p0f = 2`, `dy = -12`; climb steps `{4,0,0,6,4,4,0}`), 4 = recover (`w`/`h` restored from the kind table, after 0x20 ticks back to state 0, `dx = dy = p1b = 0`; invulnerable), 5 = death (`p0f` = death variant 0/1/2, `p11` counts to 0x14 then respawn; invulnerable). [verified]
- `dx` is a **direction** (-1/0/+1) in the player, `dy` is **pixels/frame**. `p11` is the sub-state/timer: `3` = "moved" (weapon-throw block), `0x102` = idle taunt, `0x10C` logo, `> 0x12C` reset to `0x14`; negative values are landing poses (-3..-7).
- at `p11 == 0x102` it shows one of four idle lines: `_rand() % 4` indexes a far-pointer table at DGROUP:`0x2491` ("Look, an airplane!", "Are you just gonna sit there?", "Have you seen Jill anywhere?", "Hey,  your shoes are untied.") via `_putbotmsg(...,7)` for 0x19 ticks. [verified]
- hit: no action-1 branch (the player cannot be "hit" through this path; damage goes through `_hitplayer`/`_playerkill` on the other object).

### 1. APPLE (1644:0e14) [verified]
- draw: `_drawshape(0x09<<8 | p13/2, x, y)`; `p13` advances by the sign of `dx`, mod 8.
- hit: player (`other == 0`) and `state < 1`: health +1 (capped 8), `_addscore(kscore, x, y)`, `flags |= 0xC000`, sound (2,0x0B), `_killobj`; first apple of the level shows a message (DGROUP:`0x0C20`).
- hit: other objects with `ttl == 0`: sound (6,0x19), `22AB:0f0a(state)`, `ttl = 4`, `_killobj`.
- update: `p13` advances by the sign of `dx` (mod 8); returns 1 when `p13` is even (2-frame bob).

### 2. KNIFE (1644:0fff) [verified]
- draw: fixed `_drawshape(0x0D<<8, x, y)`.
- hit: player, `p11` outside `1..10` and fewer than 3 knives in inventory (`_invcount(2) < 3`): add knife (`_addinv(2)`), sound (2,7), `_killobj` (plus a hint message).
- hit: other object with `kflags & 0x80` and `p11 > 0`: zero dx/dy, `p11 = 0x0F`, `_killaward(other)`, sound (3,10) - this is the knife killing an enemy.
- update: `p11` counts up to 0x0E then the knife homes (`_pointvect`, dx clamped +/-8, dy clamped -4..+4), `_trymove`; `p11 = -1` after 0x40 ticks or when stuck, and it falls (probes `y+1`) until it lands.

### 3. KILLME (1644:0e0b) [verified]
- Empty handler: returns 0 for every action. Used as a "nothing" object / marker.

### 4. BIGANT (1644:13cf) [verified]
- draw: `_drawshape(0x3B<<8 | frame, x, y)`; walking frames = `p13` (0..4) plus 10 when `dx > 0`; `state` 1..10 selects the death/curl frames on either side.
- hit: player -> `_hitplayer`.
- update: `p13` counts 0..4, `_crawl(dx, 0)`; on block `state = 5`, `dx = -dx`, then `state` counts down to 0 (the ant turns around).

### 5. FLY (1644:1789) [verified]
- draw: 4-entry frame table `DGROUP:0x0C2A` and a y-offset table `DGROUP:0x0C32`, indexed `p13 & 3`; +4 when `dx > 0` (facing).
- hit: player -> `_explode1(x, y, 4)` + `_playerkill(1, 0)` + `_killobj`.
- update: `_justmove(x + dx, y + ytable[p13])`; on block `dx = -dx`, sound (1,0x0F); `p13 = (p13+1) & 7`.

### 6. MACROTRIG (1a41:4c75) [verified]
- draw: nothing. hit: action 1 returns 1 (blocked). update: returns 0.
- Four instructions; it exists only so a macro/demo can refer to a position and block the player.

### 7. DEMON (1644:2974) [verified]
- draw: frame table `DGROUP:0x0C62` indexed `p13/2`, +4 when `dx < 0`; shape `0x3D<<8`.
- hit: player -> `_hitplayer`; other object with `kflags & 0x4000`: `p11` counts to 6, then `_explode2` + `_killobj`, `state = 4`.
- update: `p13 = (p13+1) % 12` (animation); `_justmove(x+dx, y+dy)`; when blocked or
  `_rnd % 0x24 == 0` it retargets with `_vectdist(player, self, dx, dy, 4)`, spawns a
  FIREBALL (kind 18) carrying the same `dx/dy`, then either randomises `dx/dy` in
  -2..2 (`_rnd % 2 == 0`) or halves them; finally `state -= (state > 0)`.  It does
  **not** use `_crawl`, and `state` only tracks the hurt reaction (set to 4 on a
  knife hit); it is not a movement mode. **[verified]**

### 8. BUNNY (1644:36ee) [verified]
- Shared handler with FROG (kind 22); everything below applies to both, only the kind byte (and therefore the tileset) differs.
- draw: `_drawshape(tileset<<8 | (dx<0 ? 3:0) + (state==1 ? (dy>0?2:0)+(dy<1?1:0) : 0), x, y)` - a 2x2 walk strip plus a hop frame.
- hit: player -> `_hitplayer`.
- update: state 0: `p13++`; once `p13 > 0x10` -> `state = 1`, `_pointvect` towards the player, `dx <<= 2`, `dy = -10` (hop), return 1. state 1: gravity `dy += 1` (capped 10), `p13 = (p13+1) % 4`, `_crawl(dx, 0)`, bounce `dx` on contact, and re-land (`state = 0`) when the tile under it is clear.

### 9. INCHWORM (1644:1c33) [verified]
- draw: `_drawshape(0x3E<<8 | (dx<0 ? 3:0) + p11, x, y)` - 3 walk frames per side.
- hit: player -> `_hitplayer`.
- update: `p13 = (p13+1) & 3`; every 4th tick flips `p11 ^= 1` and `_crawl(dx, 0)`; reverse `dx` when blocked.

### 10. ZAPPER (1644:1d91) [verified]
- draw: `_drawshape(0x4000 | 0x3F<<8 | p13, x, y)` (the `0x4000` bit is folded into the tileset byte: entry `0x3F`, no mirror).
- hit: `_playerkill(0x10, 2)` - 16 damage, no cooldown check (touching a live zapper is lethal).
- update: `p13 = (p13+1) % 5` (5-frame animation), never moves.

### 11. BOBSLUG (1644:1e50) [verified]
- draw: `_drawshape(0x40<<8 | (dx<1 ? 6-state : state), x, y)` (mirrored 6-frame crawl).
- hit: player -> `_hitplayer`.
- update: state 0: `_crawl(dx,0)`; on block `state = 1` (start bobbing). Otherwise `p11 ^= 1`, and on the odd tick `state++`; at `state > 5` `state = 0`, `dx = -dx`.

### 12. CHECKPT (1644:1fe2) [verified]
- draw: text/blit path only (`0x120E` gate, `1353:0b64` + `1353:074e`), shape 5 frames - it draws the checkpoint flag.
- hit: only `other == 0`. `state == 3` is the ending marker (`1415:06eb` save/score path + `DGROUP:0xBCAA = 2`); the port ignores it.
- otherwise, if `p13 != DGROUP:0x83A6` (the active level id): play sound (4, `level+0x32`, or 5 when `level == 0`); if `p13 != 0` store it as the level id; teleport the player; copy the attached string (`s17/s19`) into the pending-transition buffer `DGROUP:0xBB1E`; return 1.
- teleport: `kind(player) == 0x17` (TINY) -> `_setorigin(0, dx, dy)` (the checkpoint's dx/dy hold the map position), else write `x = obj.x`, `y = obj.y - 0x10`, `state = 4`, `p11 = 0`.
- The pending string is consumed at the top of the next main-loop iteration (`22ab:1531`): `*` music, `#` conditional load, `&` macro, `!` end of level, bare filename = board load. See `jill23-frontend-flow.md` section 4.
- update (`param_2 == 2`): if the player AABB overlaps the checkpoint AABB, copy the player position into the checkpoint `dx/dy` (a respawn cache).
- Port: `on_hit` case 12 + `Engine::pending_warp_` / `process_pending_warp` / `place_at_checkpoint`. After a bare-filename warp the player is placed at the target board's CHECKPT whose `p13` equals the new level id; after `!` the episode's first board is loaded and that entrance is killed.

### 13. PAUL (1644:22d0) [verified]
- draw: `_drawshape(0x42<<8 | state, x, y)` - one frame per state byte.
- hit / update: both return 0, no motion, no damage.
- Pure decoration/NPC (the "Paul" cameo); `state` selects the pose from the board data.

### 14. KEY (1644:2790) [verified]
- draw: `_drawshape(0x0E06 + p13/2, x, y)` (note: absolute shape id, not tileset-based).
- hit: player -> `_addinv(1)` (key), sound (3,6), `_killobj`; first key shows a hint (DGROUP:`0x0C24`).
- update: `p13 = (p13+1) & 7`, returns 1 on even `p13` (2-frame blink).

### 15. PAD (1644:28a4) [verified]
- draw: only when `DGROUP:0x120E != 0`: `_drawshape(0x140, x+4, y+4)`.
- hit: player -> `_sendtrig(p13, 3|4|5, self)` where the trigger id is 4 if `state == -1`, 5 if `state == 1`, else 3.
- update: returns 0. `p13` = trigger channel, `state` = pad state from board data.

### 16. WISEMAN (1644:234b) [verified]
- draw: `_drawshape(0x44<<8 | (dx>0 ? 5:0) + state, x, y)`; `state = (state+1) & 3` once per tick.
- hit: player and `ttl == 0` -> `_putbotmsg(s17, s19, 7)` (the per-board string), `ttl = 3` - this is the talking NPC; `s17`/`s19` is the 2-word far pointer stored in the object.
- update: with `p13 == 0` -> `state = (state+1) & 3`, `p13 = 1`, `_crawl(dx, 0)`, `dx = -dx` on a wall; returns 1. With `p13 != 0` the countdown branch decrements it instead (a pace between steps).

### 17. FATSO (1644:2d43) [verified]
- draw: frame table `DGROUP:0x0C6E` indexed `p13/4`, +3 when `dx > 0`; shape `0x46<<8`.
- hit: player -> `_hitplayer`.
- update: `p13 = (p13+1) & 0x0F`; on even `p13` there is a 1-in-0x1E chance to leap: `_pointvect`, `dy = 0`, `dx <<= 2`, `_crawl(dx,0)`, reverse on block, sound (1,0x11).

### 18. FIREBALL (1644:3105) [verified]
- draw: `_drawshape(0x48<<8 | p13/2, x, y)` - 4-frame flame.
- hit: player -> `_hitplayer` + `_explode2`.
- update: `_onscreen` check (killed when off screen); `p13 = (p13+1) % 8`; on wrap `_justmove(x+dx, y+dy)`, killed if blocked.

### 19. CLOUD (1644:324e) [verified]
- draw: fixed `_drawshape(0x0E0A, x, y)`.
- hit: no action-1 branch (harmless).
- update: `p13 = (p13+1) & 0xF`; on even `p13` `_justmove(x+dx, y)` and reverse `dx` when blocked (slow horizontal drift).

### 20. TEXT6 (1644:3371) [verified]
- draw: text object - `1353:0b64(vp, dx, dy)` sets the text cursor, then `1353:074e(vp, x, y+1, 2, s17, s19)` prints the 2-word string pointer carried in `s17`/`s19`.
- update: `p13` counts down; at 0 `_killobj`.
- Used for board-authored floating text (6-pixel font).

### 21. TEXT8 (1644:35df) [verified]
- Same as TEXT6 but `1353:074e(..., 1, s17, s19)` = the 8-pixel font, and no `y+1` bias.

### 22. FROG (1644:36ee) [verified]
- Identical code to kind 8 BUNNY (shared handler) - see the notes there.

### 23. TINY (1fae:1d15) [verified]
- draw: `_drawshape(0x1000 | frame, x, y+2)`; frame = 4/6 when `dx == 0` (plus `p11 & 1`), 0/8 when moving (plus `p11 & 3`) - a small character with up/down/left/right frames.
- update: `p11 = (p11+1) & 3`, `dx = 0x3C76`, `dy = 0x3C78`; moves `2*input` per tick but also does a collision/channel test with mask `0x200` (`_cando`) and adjusts `p13` to `0x10 +/- 1` when it changes channel.
- `p13 == 0x10` is the "switch side" marker: `0x3C76` is multiplied by `(mode+1)` (mode = 1, or 3 in EGA/VGA since `0x3A4D & 0xFE == 2`) and then zeroed.
- `DGROUP:0xBAB0` (air/bubble timer) picks `p13 = 0x10 - mode` or `0x10 + mode`; `26F4:23e1(0)` is called every tick when the object is off the main map. [inferred: the small/underwater Jill form]
- Port: used as the world-map avatar (`map.jn3`). `tiny_update` keeps the 2 px/tick 8-way movement with the `0x200` walkable mask, the `p11` animation counter and `tile_touch`; the `p13` swim/run channel (never entered on the map because it starts at 0) is omitted. `has_player` accepts kind 23 as well as kind 0.

### 24. DOOR (1644:3a00) [verified]
- draw: `_drawshape(0x0E05, x+4, y+0xC)` behind the inventory flag `DGROUP:0x120E`.  While
  it opens it also draws two panels at `y-state` and `y+state+0x10` using the words at
  `DGROUP:0x88F4`/`0x88FC` and repaints the map cells around it (`22AB:140b`).  Those
  two words are zero in the on-disk image and no writer was found in the exported
  decompilation, so the panel art is **[open]**; the port draws only the marker and
  keeps the map cells visible.
- hit: nothing (no action-1 branch).
- update: `state++` up to 0x10 then `_killobj`.
- action 3 ("use"): `state` must be 0.  If the cell under the door is map id `0x0BE` it
  is a gem door and needs inventory id 3 (`_takeinv(3)`, `_invcount`); any other tile is
  a key door and needs inventory id 1 (`_takeinv(1)`).  Gem door: sound (3,0x24), the
  cell at `(col+dx, row+dy)` is copied into the current cell with `0xC000` set,
  `_killobj`.  Key door: sound (3,0x0C), `state = 1` and the two cells of the door
  column are replaced by the column to the left (`col-1`) with `0xC000` set.  Missing
  the item shows the timed hint message and returns 0. **[verified]**

### 25. FALLDOOR (1644:3da3) [verified]
- draw: fixed `_drawshape(0x0E10, x, y)`.
- update: if `state == 0` it does nothing; otherwise it falls (`_justmove(x, y+4)`) and on contact writes the tile id `p13` (from board data) into the map cell, plays sound (3,0x0E) and dies; while falling it keeps the cell above it stamped with `p13 | 0xC000`.
- action 3 arms it: `state = 1` and `p13 = map[col*0x40 + row-1] & 0x3FFF` (the tile it will become).

### 26. BRIDGER (1644:2503) [verified]
- Frame: `sub sp,0x0E`; `col = x>>4` (`di`), `row = y>>4` (`[bp-0x0E]`); `newtile = (dx == 0) ? 0x8C : 0x1B0`. These are **map tile ids**, not shapes.
- draw: only when `DGROUP:0x120E != 0`; `_drawshape(0x123, x+4, y+4)` - a small trigger marker sprite.
- action 1 (hit) and 2 (update): no-op, return 0. The object is inert until something triggers it.
- action 5 ("off"): build - walk the map along `(dx,dy)` from the object's cell and write `newtile | 0xC000` into every cell that still holds the id of the starting cell; stop at the first different cell. `map[col*64 + row]` lives at DGROUP:`0x43A6` (column-major, 128x64 words), and the loop has no bounds check - it relies on the terrain changing.
- action 4 ("on"): erase - the same walk, except the run being replaced is `newtile` itself and the replacement is the tile id of the first neighbour at `(col +- dy, row +- dx)` (i.e. perpendicular to the bridge) that is solid, where solid = `(DGROUP:0x83EC + id*16, word +2) & 3 == 3`; 0 if no neighbour qualifies.
- action 3 (neutral): toggle - if the current cell already holds `newtile` take the erase path, otherwise build. If `other` is a PAD (kind 15) it also `_killobj(other)`, so a pad's trigger flips the bridge and consumes the pad.
- Result: a pad-driven bridge over water/lava. A horizontal bridger (`dx != 0`) lays tile `0x1B0` and samples the rows above/below for the restore tile; a vertical one (`dx == 0`, `dy = +-1`) lays `0x8C` and samples the columns either side.

### 27. SCORE (1644:3482) [verified]
- draw: the score digits are a 1bpp font - `1353:0b64(vp, (p13 & 3)+1, -1)` picks the font, `2b85:0085(value, buf)` formats the number, then one `_drawshape(0x3D0 + ascii_digit, x + i*4, y)` per character (i.e. shape bank 4, frames 0-9 for digits).
- update: `p13` counts down; while it is still running and on screen, `x += dx`, `y += dy`, `dy -= 1` (a small arc), then `_killobj`.
- No action-1 branch: score popups cannot be hit.

### 28. TOKEN (1a41:000f) [verified]
- draw: `_drawshape(0x0E00 + table[p13], x, y)` with the 8-entry word table at DGROUP:`0x0ED4` (38,12,13,11,14,15,18,20).
- hit: player only. If `table1[p13]` (DGROUP:`0x0EEA`) != 0 it calls `1fae:2E61(p13)`; else it decrements `table2[p13]` (DGROUP:`0x0F2C`) and shows `_putbotmsg(farstr[p13], 7)` from the far-pointer table at DGROUP:`0x0F00`. Then `_sound(3,0x19)`, `_addinv(p13)`, `_killobj`, `flags |= 0xC000`.
- the four strings are `"FOOF!"`, `"USE KEYS TO OPEN DOORS"`, `"YOU FOUND A KNIFE"`, `"YOU FOUND A GEM"`; entries 4-7 have `table1 = 1` so they go through `1fae:2E61` instead.
- the same far-pointer table has entries 8-10: `"YOU FOUND A SPINNING BLADE"`, `"EXTRA JUMPING POWER"` and `"SHIELD OF INVINCIBILITY"` (items 8, 9, 10). **[verified]**
- `if not table1[p13]` decrements `table2[p13]` and prints the hint only while it is still positive, i.e. each hint appears once per level (`table2` is 1 for entries 0-3 and 8-10, -1 for 4-7). **[verified]**
- `1fae:2E61(item)` (linear `0x12941`) is the morph helper the special entries call: 4-7 set the player kind to `0x39/0x38/0x37/0x36` (57 JILLFROG, 56 JILLBIRD, 55 JILLSPIDER, 54 JILLFISH) and 0 (the `"FOOF!"` token) reverts a morphed player to kind 0.  Both branches still run `_addinv(p13)`, so the item also enters the inventory. **[verified]**
- shipped boards use p13 values 0, 4, 5, 7, 8 and 9; 8 grants the spin-blade and 9 the feather that raises the jump by 4 per copy (`docs/jill1-player-physics.md` #4). **[verified]**
- update: returns 0 (static pickup).

### 29. ANT (1644:15a8) [verified]
- draw: `_drawshape(0x4A<<8 | frame, x, y)`; walking frames = `p13` (0..3, +5 when `dx < 0`), `state` 1..10 selects the death frames (`state*5+0x0E`, mirrored `state*-5+0x0E`).
- hit: player -> `_hitplayer`.
- update: `p13` counts 0..3, `_crawl(dx, 0)`; on block `state = 2` and `dx = -dx`; `state` counts down to 0.

### 30. PHOENIX (1644:198a) [verified]
- draw: two tables `DGROUP:0x0C42` (frames) and `DGROUP:0x0C52` (mirror offsets), indexed `p13`; `0x4C<<8`.
- hit: player -> `_explode1(x, y, 4)` + `_playerkill(1, 0)` + `_explode2` + `_killobj`.
- update: `_cando(x, y, 1)` gate; `p13 = (p13+1) % 8`; on wrap `_justmove(x+dx, y)` and reverse `dx` with sound (1,0x0F) when blocked; also sets `p13 = 6` when the tile ahead is solid.

### 31. FIRE (1a41:0160) [verified]
- draw: `_drawshape(0x4E<<8 | p13/2 + (dy>0 ? 6:0), x, y)` - 6 frames per direction.
- hit: player, `state != 1`: `_playerkill(2, 0)` + `_explode2`, `state = 1` (one-shot).
- update: `p13++`; killed at `p13 > 0x0B`.

### 32. SWITCH (1a41:02ab) [verified]
- draw: `_drawshape(0x50<<8 | state, x, y)` - two frames, up/down.
- hit: player *and* up/down held (`0x3C78 != 0`): zero the player's `dy`; first use shows "Press UP/DOWN to toggle switch" (DGROUP:`0x0ECC`). Down with `state == 1` -> `state = 0`, sound (2,0x17), `_sendtrig(p13, 3 or 5, self)` (3 when `dx == 1`, else 5). Up with `state == 0` -> `state = 1`, sound (2,0x18), `_sendtrig(p13, 3 or 4, self)`.
- update: returns 0.

### 33. GEM (1a41:04b2) [verified]
- draw: `_drawshape(tileset[kind]<<8 | (p13/2 + 4), x, y)` (the kind-table tileset is `0x09` in all three episodes, so frames 4-7).
- hit: player -> first gem shows "USE GEMS TO OPEN DOORS ON THE MAP" (DGROUP:`0x0ED2`), `_addinv(3)`, sound (3,0x10), `_addscore(kscore, x, y)`, `_killobj`.
- update: `p13 = (p13+1) & 7`, returns 1 on even `p13` (sparkle).

### 34. TXTMSG (1a41:4c8f) [verified]
- draw: nothing of its own (text-only object).
- hit: player and `state == 0`: `_putbotmsg(s17, s19, 7)` (the per-board string), sound (3,0x14), `state = 8`.
- update: `state` counts down; the message stays up for 8 ticks.

### 35. BOULDER (1a41:05f6) [verified]
- draw: `_drawshape(0x0E18 + p13, x, y)` (4 rolling frames).
- hit: player -> `_hitplayer`.
- update: `_cando(x, y+1, 3) == 3` means "supported": `dy += 2` capped 0x0C, `_trymove(x+dx, y+dy)`; when it cannot move down it snaps to the tile row (`((y+dy-1) & ~0xF) + 0x10 - h`) and `dx = 0`; while rolling it plays sound (2,0x2D) once per landing, picks a direction with `_pointvect` (dx <<= 2, `p13` +1/-1 & 3 for the frame) and `_trymove(x+dx, y)`, reversing `dx` on a wall.

### 36. EXPL1 (1a41:0a3b) [verified]
- draw: `_drawshape(0x5A<<8 | state + 0x0C - (p13/8)*3, x, y)` (a shrinking explosion sheet).
- update: `p13++`; while `p13 < 0x28` and on screen, `dy += 1` capped 0x0C and `_setorigin(x+dx, y+dy)` (no collision - it passes through walls); otherwise `_killobj`.

### 37. EXPL2 (1a41:0b91) [verified]
- draw: frame table `DGROUP:0x0F42`, `_drawshape(0x5C<<8 | table[p13], x, y)` (9 frames).
- update: `p13++`; killed at `p13 > 8` or when off screen. No movement, no damage.

### 38. STALAG (1a41:0c54) [verified]
- draw: fixed `_drawshape(0x0E21, x, y)` - a hanging stalactite; the frame
  comes from `obj+3`, so it follows the falling animation without any board
  tile.
- hit: player (`other == 0`) -> `_playerkill(2, 2)` (2 damage). Any other
  object is ignored.
- update: does nothing while `dy == 0` and reports "unchanged". Once `dy` is
  non-zero it tries `26F4:1E70(x, y + dy)` first; if that move fails it plays
  `sound(2, 0x1B)` and `_killobj(self)`. Then `dy += 2` (capped at `0x10`)
  and the handler returns non-zero (so the cell gets a `0xC000` footprint).
- trigger actions 3 and 5: if `dy == 0` set `dy = 2` and play
  `sound(3, 0x2F)`; in both cases `_killobj(sender)` so the pad/handle is
  consumed. Action 4 returns immediately (no effect).
- placed as a ceiling object; the matching PAD shares its `p13` channel.

### 39. SNAKE (1a41:0dcb) [verified]
- draw: three tables at DGROUP:`0x0F54` (head), `0x0F5C` (mid), `0x0F64` (tail), all indexed `p13`, plus the frame-9/5/0 offsets; body segments are drawn in a loop every 8 px up to `(w-0x18)/8`, mirrored when `dx >= 1`.
- hit: player -> `_hitplayer`.
- update: `p13` cycles the three tables (the snake "crawls" by re-assigning head/mid/tail frames); moves like the other crawlers.

### 40. SEAROCK (1a41:46d5) [verified]
- draw: fixed `_drawshape(0x5E<<8 | 0x22, x, y)`.
- update: 1-in-12 chance per tick to `_addobj(0x3A BUBBLE, x+2, y+4)`.
- No hit branch: a static bubble emitter on the sea floor.

### 41. BOLL (1a41:1261) [verified]
- draw: `_drawshape(0x60<<8 | p13, x, y)` (6 frames).
- hit: player -> `_hitplayer`.
- update: `w = h = table[p13]` (DGROUP:`0x0F6C`) - the collision box shrinks/grows with the frame; picks the nearest kind-41 object with a higher `p13` within 0x60 manhattan (`_dist`), `_vectdist` towards it, dx/dy clamped +/-0x0C; `_justmove` axis by axis, bouncing and playing sound (1,9).

### 42. MEGA (1a41:165a) [verified]
- draw: `_drawshape(0x62<<8 | dx, x, y)` - `dx` doubles as the frame index.
- update: dormant while `state == 0`. Once triggered, `dy += 2` capped 0x10 and `_justmove(x, y+dy)`; on landing `dy = 2 - dy` (bounce), and when the bounce dies (`dy == 0`) it returns 0; otherwise sound (2,0x1C).
- action 3 sets `state = 1`: this is the "mega" falling/bouncing block started by a trigger.

### 43. BAT (1a41:17ee) [verified]
- draw: hanging (`state == 0`) uses frame 6 at `y+2`; flying uses two tables at DGROUP:`0x0F78` (frame) and `0x0F80` (y offset) indexed `p13/2`, +3 when `dx < 0`.
- hit: player -> `_hitplayer`.
- update: `p13 = (p13+1) & 7`; while hanging, a 1-in-0x28 chance per tick to take off (`state = 1`, `dx = -4/+4`, `dy = 2`); while flying `_trymove(x+dx, y+dy)` with random direction changes (1-in-0x23 to flip `dx`, 1-in-0x14 to flip `dy`), and a 1-in-3 chance to re-hang when descending.

### 44. KNIGHT (1a41:1bb4) [verified]
- Frame: `sub sp,0x2C`; copies two 11-word tables to the stack on every call: x offsets `DGROUP:0xF88` = 8,8,0,0,0,0,0,0,0,0,8 and frames `DGROUP:0xF9E` = 0,0,1,2,3,4,4,3,2,1,0.
- draw: `_drawshape(vp, (DGROUP:0xBB96<<8) | frame[p11], x + xoff[p11], y)` - base shape `0x2400`, frames `0x00..0x04`; `p11` is an 11-step swing loop that runs 0->4 and back.
- update: `state = (p11 == 6) ? 1 : 0`, then only runs on odd global frames (`DGROUP:0xBE3A & 1`), else returns 0. If `p11 == dx` the animation is frozen (return 0). Otherwise `p11++`; once `p11 >= 0x0A` -> `p11 = 0`, `dx = 0`, return 1.
- `dx` here is a **hold-frame selector**, not a velocity: `0` = idle pose 0, `6` = frozen at the extended-sword frame (and `state` stays 1, i.e. permanently lethal), `0x0A` = never holds, runs the full loop.
- action 3 -> `dx = 0x0A` (start swinging), action 4 -> `dx = 0` (stand idle), action 5 -> `dx = 6` (freeze mid-swing). These are the `_sendtrig` codes 3/4/5, so a lever/pad in the board drives the knight.
- hit: player (`other == 0`) only while `state == 1`: `_putbotmsg("The knight slices Jill in half", DGROUP:0x178B, 7)` then `_playerkill(0x10, 2)` - 16 damage, instant death, and the death animation index 2 picks the "cut in half" sound `4/0x29`.
- hit: if `kflags[kind(other)] & 0x4000` - i.e. a thrown KNIFE (2), SPINBLAD (50), FIREBULLET (62) or FISHBULLET (63) - and `DGROUP:0x0ED0 != 0`: `_putbotmsg("YOUR FEEBLE ATTEMPT FAILS.", 0x114F, 2)` and clear the flag. The knight never takes damage; it taunts once per level.
- So it is a hazard, not an enemy: either a static statue (`dx = 6`) or a lever-driven swinging blade (`dx = 0x0A`). It has no pathfinding, no `_crawl`, no death state.

### 45. BEENEST (1a41:1e13) [verified]
- draw: `_drawshape(0x4E??<<8 | f, x, y)` where the base is DGROUP:`0xBB98` (dx in the kind table = 0x4E) and f = 0, 1, or 2/3 (2 when `dx < 1`, 3 otherwise) - a three-frame opening nest.
- hit: any object with `kflags & 0x4000` (i.e. the knife/player attack): `_addscore(kscore, x, y)`, `_explode1(x, y, 10)`, `_killobj`, sound (3,0x20).
- update: only on every 4th global frame (`DGROUP:0xBE3A & 3 == 2`). With `p13 == 0` there is a 1-in-0x20 chance to `_pointvect` towards the player (sets `dx`/`dy`) and `p13 = 1`; otherwise `p13++`, and past 2 it resets to 0 and spawns `_addobj(0x2E BEESWARM, x + dx*8, y + ...)`.
- So the nest wakes, aims, and then spits one BEESWARM per 3 steps.

### 46. BEESWARM (1a41:2069) [verified]
- draw: `_drawshape(0x25<<8 | p13 + 4 + (dx >= 1 ? 3:0), x, y)` - frames 4..9; 32-entry anim table copied from DGROUP:`0x0FB4`.
- hit: player only -> `_hitplayer`; non-player hits ignored, no score.
- update: `p13 = (p13+1) % 3`; `_pointvect` towards the player, `dx = (dx*2 + rand%2) * tbl[state&31] / 2`, `dy = (dy*2 + rand%2) * tbl[(state+16)&31] / 2`, `_justmove(x+dx, y+dy)`; `state++`, killed at `state >= 0xA1` (161).
- jittered homing that ignores walls (only `_justmove`/mask 1).

### 47. CRAB (1a41:22f3) [verified]
- draw: `_drawshape(0x26<<8 | p13, x, y)`; `p13 = (p13+1) & 3` in update (4 walk frames).
- hit: player only -> `_hitplayer`.
- update: state 0 (walk): random `dx`; on a 16-px x boundary, if `_cando(x, y, 4) == 0` and `rand%2 == 0` -> `state = 1`; else `_crawl(dx, 0)`, reversing on a wall. State 1 (ledge): drop a row with `_justmove(x, y+dy)`, flipping `dy` when blocked; re-attach to the floor (`_crawl`) one row at a time and go back to state 0.

### 48. CROC (1a41:269f) [verified]
- draw: two shape groups at DGROUP:`0xFF4` (head) and `0xFFC` (tail), mirrored by the sign of `dx`; the tail is drawn again at `x+0x20`, so the 64x8 croc is stitched from two 32-px halves.
- hit: player -> `_hitplayer`; other object with `kflags & 0x4000` and `state == 0`: `_stampfootprint`, `state = 0x10`, `_pointvect`; if the attacker is in front -> `_killaward(self)`, otherwise just reverse `dx`.
- update: while `state > 0`: `state--`, `p13 = (p13+1) & 3`, `_crawl(dx, 0)`, reverse on block. Only 3 in the level tables.

### 49. EPIC (1a41:29fa) [verified]
- draw: `_drawshape(0x28<<8 | (DGROUP:0xBE3A & 7), x, y)` - 8-frame spin driven by the global frame counter.
- hit: player only: `_addscore(2, x, y)` then `state++`; below 0x0B plays sound (2,0x20), at 0x0B -> `_explode1(x, y, 10)` + `_killobj` + sound (3,0x30).
- update: returns 1, no motion, never expires. Eleven player touches to destroy, 2 points each (22 total).

### 50. SPINBLAD (1a41:2b1a) [verified]
- draw: `_drawshape(0x2D<<8 | p13, x, y)`; `p13 = (p13 + sign(dx)) & 3` - the spin follows travel.
- hit: player (`other == 0`) and `p0f > 7` -> `_killobj` only (never damages the player); other kinds with `kflags & 0x400` -> `_killobj` + `_killaward(other)` + sound (3,10).
- update: `p0f++`; `p0f >= 0x40` or off screen -> `_killobj`. Else finds the nearest kind with `kflags & 0x400` (not object 0) within manhattan 0x60, `_vectdist` towards it, `dx += dir`, `dy` forced >= +1, both clamped +/-0x0C; `_touchbkgnd` at the target cell (each 0xA7 tile eaten adds 10 to `p0f`), `_justmove` x with bounce, `_justmove` y with sound (1,0x23) and bounce; kills itself if still stuck.

### 51. SKULL (1a41:3029) [verified]
- draw: `_drawshape(0x2F<<8 | tblA[(p11/2) & 3], x, y)` (4 ints at DGROUP:`0x1014`); when `state != 0` also two small sprites `tblB[p11]` at `(x, y+5)` and `tblB[(p11+4) & 7]` at `(x+10, y+6)` (8 ints at DGROUP:`0x1004`).
- hit: no action-1 branch.
- update: `state == 0` -> frozen (return 0); else `p11 = (p11+1) & 7`, return 1.
- action 3 ("trigger") is the only way in: `state == 0` -> `state = 1`, `p11 = 0`, sound (4,0x21). Pure decoration - no damage, no score.

### 52. BUTTON (1a41:3261) [verified]
- draw: `_drawshape((0x31<<8 | 1) - state + (dx>0 ? 2:0), x, y)` - 4 frames; `dx` picks the side, `state` up/down.
- hit: with `p0f == 0` (3-tick debounce): `state = 1-state`; `state == 1` -> `_sendtrig(p13, 4, self)`, else `_sendtrig(p13, 5, self)`; sound (3,0x2C); `p0f = 3`.
- update: decrement `p0f`, return 0. Trigger 4 = pressed, 5 = released, to every object whose `p13` matches.

### 53. PAC (1a41:3402) [verified]
- draw: `_drawshape(0x32<<8 | f, x, y)` with f = 1 if `dx<0`, 2 if `dy<0`, 3 if `dy>=1`, else 0.
- hit: player -> `_hitplayer`; non-player of kind 0x3E (FIREBULLET) -> `_killobj`.
- update: moves only on tile centres (`(x & 0xF) == 0 && (y & 0xF) == 0`): reads the current cell `map[(x>>4)*0x40 + (y>>4)] & 0x3FFF` and probes the neighbours along `sign(dx), sign(dy)` (random axis if both are 0) for a cell whose id differs; tries the two sign flips, then the reverse. Then `dx = p13*sgnX`, `dy = p13*sgnY` (p13 = speed from the board) and `_trymove`.
- Pac-Man style corridor follower; map is column-major `[col*0x40 + row]`.

### 54. JILLFISH (1fae:202a) [verified]
- draw: `_drawshape(0x33<<8 | tbl[p13] + (p0f>0 ? 3:0), x, y)`, 4-entry table at DGROUP:`0x11F3` (two facing sets).
- update: `p13 = (p13+1) & 3`; `_fishdo` water test; `p0f = dx` remembers facing **only when `dx != 0`**, then `dx = 0x3C76 << 3`. Failed water test -> `dy += 2` capped 0x10 (sinking out of water); else `dy += 0x3C78*2 + (dy<2)` capped +/-8. `rand%4 == 0` -> `_addobj(0x3A BUBBLE, x+6, y-2)`. Button 1 (in water) -> `dy = -2`, sound (1,0x25), `DGROUP:0x3D9E = 1`. Button 2 -> `_addobj(0x3F FISHBULLET, x + (p0f<0 ? 0x20:0) - 8, y)` with `dx = sign(p0f)*8 + sign(dx)*4`, sound (1,0x26), `DGROUP:0x3DA0 = 1`. Then `_justmove` on y (snapping a row when blocked) and `_fishdo(x+dx, y)`.
- hit: no action-1 branch (the player's fish form is never "hit").
- Always returns 1.
- Port: the facing memory is preserved; stopping no longer resets `p0f` to 0
  (which made every shot go right). `CENTER FISH SHOT` optionally spawns the
  bullet at the fish's vertical centre.

### 55. JILLSPIDER (1fae:262f) [verified]
- draw: none - falls straight through and returns 0 without calling `_drawshape`.
- hit/update: return 1 / 0. An empty 12-byte stub; kind 55's tileset is the empty
  shape entry 0, so the original spider form is cut content.
- Port: a procedural 16x16 spider (body/legs/eyes) plus a 2 px, eight-direction
  wall-climb movement. Reachable from `BECOME SPIDER` and the p13=6 tokens in
  Onesimus boards 17/28. The Jill 3 p13=6 alternate award path is `[open]`.

### 56. JILLBIRD (1fae:298b) [verified]
- draw: `_drawshape(0xBB7A<<8 | tbl[p13] + (p0f<0 ? 4:0), x, y)`; table at DGROUP:`0x11FB`, 4-px mirror offset.
- update: `p0f = dx` (facing), `p13 = (p13+1) % 6`; `dx = 0x3C76 << 3` on an 8-px boundary (else `<< 2`); `dy += 1` gravity; button 1 -> `dy = -6`, sound (1,0x0F), `DGROUP:0x3D9E = 1`; button 2 -> `_addobj(0x3E FIREBULLET, x + sign(p0f)*0x18, y)` with `dx = sign(p0f)*8`, `dy = 0x3C78*2`, sound (1,0x22), `DGROUP:0x3DA0 = 1`; `dy` clamped -8..+8.
- movement: `_justmove(x+dx, y)`, then on block `_justmove(x, y+dy)` and a step-up probe (`(y+0x10) & ~0xF`, or `((y & ~0xF)+0x10) - h` when descending), zeroing `dy` when nothing works.
- `26F4:232E(x, y, 0x4000)` != 0x4000 -> `_playerkill(0x100, 1)`: flown out of the water/into a hazard.

### 57. JILLFROG (1fae:2648) [verified]
- draw: base `0x3F<<8 | (p1b < 1 ? 3:0)`; when `state == 1` add 1 if `dy < 1`, else 2 (hop frames).
- update: `p1b = dx` (facing memory); `dx = 0x3C76 << 3` when `(x & 7) == 0` and moving, else `0x3C76 << 2`. `_cando(x, y+1, 3)`: match -> `dy += 1` capped 0x0C; no match -> `dy = -8` + sound (1,0x12), or `dy = -0x0C` + sound (1,0x2B) while buttons 1/2 are held. `_trymove(x+dx, y+dy)`; fully blocked -> `dy < 0` ? `dy = 0` : snap onto `((y & ~0xF)+0x10 - h)`, `p13 = 0`, `dy = 0`.
- hit: no branch. Returns 1 every tick.
- Port: `p1b` is only updated while moving; `state = 1` while airborne (so the
  hop frames 1/2 are used) and `state = 0` after the landing snap. `_trymove`
  returns 0 when a blocked fall has no horizontal component, which lets the
  snap `((y & ~0xF)+0x10 - h)` fire and puts the 10-px frog on the floor.

### 58. BUBBLE (1a41:3834) [verified]
- draw: `_drawshape(0x33<<8 | p13 + 6, x, y)` - frames 6..8.
- update: `p13++` with 1-in-15 probability; while `p13 < 3` and on screen, wobble `+/-1` px horizontally with `_fishdo` and die when that leaves the water; otherwise `_killobj`.
- `p13` = growth step; never rises.

### 59. JELLYFISH (1a41:3957) [verified]
- draw: `_drawshape(0x33<<8 | tbl[p13], x, y)`; 10-entry table copied from DGROUP:`0x101C`.
- hit: player -> `_hitplayer`; other kind with `kflags & 0x4000` -> `_explode1(x, y, 0x14)` + `_killobj`.
- update: `p13 = (p13+1)` wrapping at 9; every 10th tick a random kick (`dx += (rand%3-1) * (|dx|<3 ? 2:1)` clamped +/-6, `dy += (rand%3-1) * ((dy<2)+1)` clamped +/-6), then `_fishdo(x+dx)` (bounce `dx`) and `_fishdo(y+dy)` (bounce `dy`).

### 60. BADFISH (1a41:3d03) [verified]
- draw: `_drawshape(0x33<<8 | tbl[p13 & 3] + (dx>0 ? 3:0) + 0x0F, x, y)`; 4-entry table at DGROUP:`0x1030`.
- hit: player -> `_hitplayer`; other kind with `kflags & 0x4000` -> `_killaward(self)`.
- update: `p13++`; 1-in-20 to re-roll `dx` in {-4,0,4} and `dy` in {-2,0,2} (if `dx == 0` then `dy` in {-2,2}); `_fishdo(x+dx)` bounce, `_fishdo(y+dy)` bounce; 1-in-4 to `_addobj(0x3A BUBBLE, x+6, y-2)`.

### 61. ELEV (1a41:3fe8) [verified]
- draw: fixed `_drawshape(0xE2C, x, y)`.
- hit: only `kind(other) == 0` (the player): `state = 6`; the first use of the level (`DGROUP:0x0ECE != 0`) shows "Press UP/DOWN to use elevator" and clears the flag.
- up: `_justmove(0, player_x, (y>>4 - 2) << 4)` then `_setorigin(x, y-0x10)` and `map[col*0x40+row] = 0xC08A`. down: requires `map[col*0x40+row+1] & 0x3FFF == 0x8A`, then writes `map[...] & 0x3FFF | 0xC000` into the row below, `_setorigin(x, y+0x10)` and `_justmove(0, player_x, row<<4)`. Sounds (2,0x1D)/(2,0x1E) only when the up/down input changes (`p0f` caches it).
- update: `state--`; at 0, if the cell below is still tile 0x8A and `p13 != -1`, copy the tile down, `_setorigin(x, y+0x10)` and return 1.
- The platform carries floor tile 0x8A with it; `p13 == -1` marks an elevator that must not auto-descend.

### 62. FIREBULLET (1a41:436d) [verified]
- draw: `_drawshape(0x1A<<8 | p13, x, y)` (4 frames).
- hit: non-player with `kflags & 0x80` -> `_killobj(self)` + `_killaward(other)` + sound (3,10); player hits do nothing. Always returns 1.
- update: off screen -> `_killobj`. `p13 = (p13+1) & 3`; `state = (state+1) % 0x14`; `_touchbkgnd` at the pre-move position; `_justmove(x, y + dy + arc[state])` with the 20-entry arc table at DGROUP:`0x1038`; killed if the horizontal move is blocked; `p0f++` and killed at `p0f > 0x4F`.
- Arcing projectile (baked per-frame y offset, not gravity); eats 0xA7 tiles; inert against the player.

### 63. FISHBULLET (1a41:45ed) [verified]
- draw: `_drawshape(0x33<<8 | 0x15 + (dx>0), x, y)` - frames 0x15/0x16 for the two directions.
- update: off screen -> `_killobj`; else `_justmove(x+dx, y)`, killed when blocked; `y` never changes.
- hit: no action-1 branch: no damage or score of its own. Flat, gravity-free shot fired by JILLFISH.

---

## Porting checklist for this file

* 58 distinct handler bodies cover the 63 populated kinds (only BUNNY/FROG are
  shared, `1644:36ee`).
* Every handler except PLAYER is a state machine over `state` / `p13` / `p11` plus
  `_crawl`/`_trymove`/`_justmove` and a handful of DGROUP tables. Port the 30 shared
  routines in the API table first, then the handlers fall out almost mechanically.
* Remaining uncertainty: only the exact `p0f`/`p11` semantics inside PLAYER states
  2/3/5. All 64 kinds now have a handler description; kinds 26 and 44 were finished
  from raw disassembly (`1644:2503` / `1a41:1bb4`) because Ghidra's output is unusable.
* Actions 3/4/5 are not separate protocols: they are the three `_sendtrig` codes.
  BRIDGER (26) maps 4 = erase / 5 = build, KNIGHT (44) maps 4 = idle / 5 = freeze
  mid-swing; everything else treats 3/4/5 the same.
* Triggers used by boards: `_sendtrig(id, code, self)` with code 3 = off/neutral,
  4 = on/pressed, 5 = off/released; `action 3` is the "trigger" call used by
  DOOR/FALLDOOR/ELEV/MEGA/SKULL.

The five kinds added after Jill 1 (64-68: `EYE`, `VINECLIMB`, `FLAG`, `MAPDEMO`,
`ROMAN`) are documented in `jill23-new-kinds.md`; they share this ABI unchanged.

