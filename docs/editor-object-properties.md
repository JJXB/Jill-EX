# Object property reference (Jill 1 / Jill 2 / Jill 3)

What the object-property panel fields actually do, per object kind.

The table is for the **Jill 1** kind table (kinds 0-68). Jill 2 and Jill 3 use
the same record layout and the same kind properties; only kinds 64-68 are new
content. A few episode-specific flags/handlers differ (DEMON, TOKEN, EPIC); see
`docs/jill2-differences.md` and `docs/jill3-differences.md`. The source of truth
behind this is `docs/jill1-kind-handlers.md`, `docs/jill23-new-kinds.md` and the
port handlers in `port/src/engine.cpp`.

Use `—` as "this handler does not use the field". Fields are often reused for
something completely different from object to object, so always read the row
for the object you are editing.

---

## Read this first

### Global fields

| Panel field | Board field | Meaning |
| --- | --- | --- |
| `KIND` | `kind` | Object type, 0-68 |
| `X`, `Y` | `x`, `y` | Pixel position, 16 pixels per cell. The editor places on the grid; triggers and doors read `x >> 4`, `y >> 4`. |
| `W`, `H` | `w`, `h` | Size in pixels. Normally copied from the kind table. A few handlers overwrite it (`BOLL`, `SNAKE`, `TEXT6/8`, `SCORE`). |
| `DX`, `DY` | `dx`, `dy` | Most often velocity, facing or a selector. Some objects use one of them as a frame index or position. |
| `PARAM` | `p0f` | A free parameter. Different per kind: pose, facing memory, timer, debounce. |
| `P11` | `p11` | Usually an animation/sub-state counter. A few objects use it as a mode. |
| `P13` | `p13` | The main free parameter. Usually animation counter, trigger channel, item id, level id or speed. |
| `P1B` | `p1b` | Rarely used. Player/frog facing memory and the ROMAN hit guard. |
| `TTL` | `ttl` | Usually a **hit cooldown**, not a lifetime. A few objects use it for message/page cooldowns. |
| `STRING` | `text` | Attached board string. Only text objects, the wise man and checkpoints read it. |
| `FLAGS` | `flags` | Runtime field. `0xC000` is set by the engine for blocked/redraw footprints. Leave it at `0` in the editor. |

`FLAGS` in the panel is **not** the kind-table `0x0100` "receives triggers"
flag. That flag belongs to the object *type*; it is what makes DOOR/BRIDGER/etc.
listen on `P13`.

### What `STATE` usually means

`STATE` is not a universal "object state". Common meanings:

| Use | Examples |
| --- | --- |
| Movement/animation mode | PLAYER, BUNNY/FROG, BAT, BOBSLUG, CRAB |
| Death or recovery animation | BIGANT, ANT, DEMON, CROC, PLAYER state 4/5 |
| Trigger on/off | SWITCH, BUTTON, MEGA, SKULL, PAD |
| Closed/opening | DOOR, FALLDOOR |
| Pickup vs text page | APPLE (`state >= 1` = page number) |
| Message countdown | TXTMSG |
| Checkpoint mode | CHECKPT (`3` ending marker, `4` spawn/recover) |
| Hit counter | EPIC |
| Pose selector | PAUL, WISEMAN |

The table below gives the specific meaning for every kind.

### What `P13` usually means

| Use | Examples |
| --- | --- |
| Animation phase | APPLE, FLY, DEMON, KEY, GEM, BAT, etc. |
| Trigger channel | PAD, SWITCH, BUTTON, DOOR, BRIDGER, STALAG, MEGA, KNIGHT, SKULL |
| Item id | TOKEN (0-10) |
| Level id | CHECKPT |
| Speed | PAC |
| Tile id | FALLDOOR (written when armed) |
| Platform flag | ELEV (`-1` = no auto-descend) |

Editing an animation `P13` only changes the starting frame. Editing a channel
or item/level id changes behaviour.

### What `P11`, `P1B` and `TTL` usually mean

* `P11`: animation/sub-state, or a mode (`KNIGHT` swing loop, `SKULL` anim).
* `P1B`: player/frog facing memory, or `ROMAN`'s one-hit guard.
* `TTL`: hit cooldown for anything that calls `_hitplayer`; a message/page
  cooldown for `WISEMAN`/`APPLE`.

---

## Per-object table 1: kinds 0-33

| Kind | Object | Size | STATE | DX / DY | PARAM | P11 | P13 | P1B | TTL | STRING |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | PLAYER | 16×32 | 0 ground/air, 2 air/water, 3 climb, 4 recover, 5 death | dx = horizontal direction -1/0/+1; dy = vertical speed | pose selector (0/1 idle, 2 water, 6 climb, 7 walk, -1 falling) | sub-state/timer; landing pose when negative; 0x102 idle taunt | secondary timer; reset on climb/land | facing memory | — | — |
| 1 | APPLE | 12×12 | 0 normal apple; >=1 = text-page number | dx sign advances the animation | — | — | 8-phase bob/sparkle | — | page-apple re-show cooldown | — |
| 2 | KNIFE | 10×10 | — | dx/dy = flight velocity | — | flight/pickup state: 0 resting pickup, 1-10 not collectable, 14/15 flying, -1 falling | — | — | — | — |
| 3 | KILLME | 0×0 | — | — | — | — | — | — | — | — |
| 4 | BIGANT | 24×16 | 0 walking; 1-10 death/turn frames | dx = direction/speed | — | — | 0-4 walk animation | — | hit cooldown | — |
| 5 | FLY | 16×14 | — | dx = direction/speed | — | — | 0-7 flight animation | — | — | — |
| 6 | MACROTRIG | 0×0 | — | — | — | — | — | — | — | — |
| 7 | DEMON | 32×32 | hurt reaction countdown (set to 4 on hit) | dx/dy = velocity | — | projectile damage counter | 0-11 animation | — | hit cooldown | — |
| 8 | BUNNY | 8×8 | 0 charging, 1 hopping | dx = hop direction/speed; dy = hop velocity | — | — | hop charge timer | — | hit cooldown | — |
| 9 | INCHWORM | 16×8 | — | dx = direction/speed | — | 0/1 walk frame | tick counter | — | hit cooldown | — |
| 10 | ZAPPER | 32×16 | — | — | — | — | 0-4 animation | — | — | — |
| 11 | BOBSLUG | 24×24 | 0 crawl; 1-5 bob/turn | dx = direction/speed | — | ticker | — | — | hit cooldown | — |
| 12 | CHECKPT | 16×16 | 0 normal; 3 ending marker; 4 spawn/recover | dx/dy = map-avatar spawn position for TINY | — | reset to 0 on spawn | level id that this checkpoint enters | — | — | warp/music/macro/end command |
| 13 | PAUL | 24×32 | pose/frame selector | — | — | — | — | — | — | — |
| 14 | KEY | 16×8 | — | — | — | — | 8-phase blink | — | — | — |
| 15 | PAD | 16×16 | -1 sends code 4; 0 sends code 3; 1 sends code 5 | — | — | — | trigger channel | — | — | — |
| 16 | WISEMAN | 16×24 | 0-3 animation | dx > 0 = face right | — | — | pacing: 0 = start moving, else countdown | — | message cooldown | message text |
| 17 | FATSO | 20×28 | — | dx = direction/speed | — | — | anim/leap timer | — | hit cooldown | — |
| 18 | FIREBALL | 16×16 | — | dx/dy = velocity | — | — | 8-phase flame animation | — | hit cooldown | — |
| 19 | CLOUD | 16×16 | — | dx = drift direction/speed | — | — | drift timer | — | — | — |
| 20 | TEXT6 | 6×7 | — | dx/dy = text cursor offset | — | — | lifetime; at 0 the object is removed | — | — | text (6-pixel font) |
| 21 | TEXT8 | 8×8 | — | dx/dy = text cursor offset | — | — | lifetime | — | — | text (8-pixel font) |
| 22 | FROG | 14×10 | same as BUNNY | same as BUNNY | — | — | hop charge timer | — | hit cooldown | — |
| 23 | TINY | 4×10 | — | dx/dy = movement (input-driven) | — | animation counter | map channel `0x10 ± mode`; not used on the hub map | — | — | — |
| 24 | DOOR | 16×24 | 0 closed; >0 opening animation | dx/dy = gem-door target cell offset | — | — | trigger channel | — | — | — |
| 25 | FALLDOOR | 16×16 | 0 dormant; 1 armed/falling | — | — | — | tile id it becomes (written when armed) | — | — | — |
| 26 | BRIDGER | 0×0 | — | dx/dy = bridge axis/direction (`dx == 0` vertical) | — | — | trigger channel | — | — | — |
| 27 | SCORE | 4×5 | — | dx/dy = popup arc velocity | — | — | lifetime countdown | — | — | — |
| 28 | TOKEN | 16×16 | — | — | — | — | item id 0-10 | — | — | — |
| 29 | ANT | 32×16 | 0 walk; 1-10 death frames | dx = direction/speed | — | — | 0-3 walk animation | — | hit cooldown | — |
| 30 | PHOENIX | 16×16 | — | dx = direction/speed | — | — | 0-7 animation | — | — | — |
| 31 | FIRE | 16×32 | 0 normal; 1 already spent | dy = frame direction (>0 adds 6) | — | — | lifetime/animation | — | — | — |
| 32 | SWITCH | 16×16 | 0 up; 1 down | dx = 1 sends code 3; other values send 4/5 | — | — | trigger channel | — | — | — |
| 33 | GEM | 16×16 | — | — | — | — | 8-phase sparkle | — | — | — |

---

## Per-object table 2: kinds 34-68

| Kind | Object | Size | STATE | DX / DY | PARAM | P11 | P13 | P1B | TTL | STRING |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 34 | TXTMSG | 16×16 | 0 ready; >0 message countdown | — | — | — | — | — | — | message text |
| 35 | BOULDER | 16×16 | — | dx/dy = velocity | — | — | 0-3 rolling frame | — | hit cooldown | — |
| 36 | EXPL1 | 16×32 | visual frame offset (set by spawner) | dx/dy = velocity | — | — | lifetime; removed past 0x28 | — | — | — |
| 37 | EXPL2 | 16×32 | — | — | — | — | frame/lifetime 0-8 | — | — | — |
| 38 | STALAG | 16×16 | — | dy = 0 dormant, >0 falling | — | — | trigger channel | — | — | — |
| 39 | SNAKE | 56×16 | — | dx = direction/speed | — | — | head/mid/tail animation | — | hit cooldown | — |
| 40 | SEAROCK | 16×16 | — | — | — | — | — | — | — | — |
| 41 | BOLL | 14×14 | — | dx/dy = velocity | — | — | frame **and** collision size (W/H from a table) | — | hit cooldown | — |
| 42 | MEGA | 20×24 | 0 dormant; 1 triggered/falling | dx = frame index; dy = fall/bounce velocity | — | — | trigger channel | — | — | — |
| 43 | BAT | 26×32 | 0 hanging; 1 flying | dx/dy = velocity | — | — | 0-7 wing animation | — | hit cooldown | — |
| 44 | KNIGHT | 32×32 | derived from `P11 == 6` (lethal pose) | dx = hold selector: 0 idle, 6 freeze extended, 10 full swing | — | 11-step swing loop | trigger channel | — | — | — |
| 45 | BEENEST | 16×16 | — | dx/dy = aim offsets | — | — | aim/spawn cycle | — | — | — |
| 46 | BEESWARM | 16×16 | lifetime counter (dies near 0xA0) | dx/dy = jittered homing velocity | — | — | 0-2 animation | — | hit cooldown | — |
| 47 | CRAB | 16×16 | 0 walking; 1 dropping from a ledge | dx = direction/speed; dy = drop direction | — | — | 0-3 walk animation | — | hit cooldown | — |
| 48 | CROC | 64×8 | attack/death timer (>0) | dx = direction/speed | — | — | 0-3 animation | — | hit cooldown | — |
| 49 | EPIC | 32×16 | hit counter 0-0x0B (11 touches) | — | — | — | — | — | — | — |
| 50 | SPINBLAD | 16×16 | — | dx/dy = velocity | lifetime 0-0x40; +10 per eaten `0xA7` tile | — | 0-3 spin frame | — | — | — |
| 51 | SKULL | 22×26 | 0 dormant; 1 active | — | — | 0-7 animation | trigger channel | — | — | — |
| 52 | BUTTON | 16×16 | 0 up; 1 down | dx = side (`>0` selects the other frame pair) | debounce, 3 ticks | — | trigger channel | — | — | — |
| 53 | PAC | 16×16 | — | dx/dy = direction × speed | — | — | speed | — | hit cooldown | — |
| 54 | JILLFISH | 24×16 | — | dx/dy = movement | facing memory | — | 0-3 swim animation | — | — | — |
| 55 | JILLSPIDER | 16×16 | player form; fields are internal to the morph | player form; fields are internal | — | player form; internal | — | — | — | — |
| 56 | JILLBIRD | 16×16 | — | dx/dy = movement | facing memory | — | 0-5 wing animation | — | — | — |
| 57 | JILLFROG | 14×10 | 0 grounded; 1 airborne | dx/dy = movement | — | — | reset to 0 on landing | facing memory | — | — |
| 58 | BUBBLE | 8×8 | — | dx/dy = wobble | — | — | growth step 0-2 | — | — | — |
| 59 | JELLYFISH | 16×24 | — | dx/dy = velocity | — | — | 0-9 animation | — | hit cooldown | — |
| 60 | BADFISH | 28×16 | — | dx/dy = velocity | — | — | swim animation | — | hit cooldown | — |
| 61 | ELEV | 16×16 | descent timer (use sets 6) | — | up/down input cache | — | -1 = never auto-descend | — | — | — |
| 62 | FIREBULLET | 16×16 | arc phase 0-0x13 | dx/dy = movement | lifetime; removed past 0x4F | — | 0-3 flame animation | — | — | — |
| 63 | FISHBULLET | 12×5 | — | dx = horizontal speed; dy unused | — | — | — | — | — | — |
| 64 | EYE | 16×12 | — | dx/dy = pupil offset | — | — | — | — | — | — |
| 65 | VINECLIMB | 16×8 | — | dy = bob direction/speed | — | — | 0-7 bob phase | — | hit cooldown | — |
| 66 | FLAG | 36×16 | 0 untouched; 1 collected | — | — | — | 0-5 animation | — | — | — |
| 67 | MAPDEMO | 64×16 | — | — | — | — | — | — | — | — |
| 68 | ROMAN | 16×32 | — | dx = direction/speed; dy forced to 0 | — | — | 0-7 walk animation | 0 = can damage; nonzero = already hit | hit cooldown | — |

---

## Notes on the unusual rows

### PLAYER (0)

The only object with a full movement state machine. `STATE` selects the
movement/physics branch; `DX` is a direction, `DY` is a signed pixel velocity.
`PARAM` is the pose index, `P11` is the shared sub-state/idle/landing timer,
`P13` is a secondary timer and `P1B` remembers the last facing direction.
Do not edit the player's counters unless you are intentionally testing a pose.

### CHECKPT (12)

`P13` is the **level id** this checkpoint enters, not a channel. `STRING` is
the pending transition: a bare board name, `*music`, `&macro`, `#conditional`
or `!` ending. `DX/DY` hold the world-map position used when the player is the
TINY avatar. `STATE 3` is a special ending marker; `STATE 4` is the
spawn/recover state, so normally leave it at 0.

### DOOR (24) / FALLDOOR (25)

`DOOR`: `P13` is the trigger channel, `STATE 0` is closed, `DX/DY` is the
gem-door destination cell. Key doors ignore `DX/DY` and copy the column to
their left. See `editor-doors.md`.

`FALLDOOR`: `P13` is not authored as a channel. When armed, the handler writes
the tile id it will become into `P13`, then falls and stamps that tile. `STATE`
must be 0 in the editor; trigger code 3 arms it. Do not try to pre-set `P13` as
a trigger channel.

### BRIDGER (26)

`P13` is the channel. `DX/DY` select the bridge direction; `dx == 0` means a
vertical bridge, otherwise horizontal. Codes 4/5 erase/build directly; code 3
toggles. A PAD trigger also consumes itself when it toggles a bridge.

### PAD (15) / SWITCH (32) / BUTTON (52)

These are the only normal trigger senders:

* PAD `STATE 0` sends code 3 (useful for doors). `-1` sends 4, `1` sends 5.
* SWITCH sends code 3 only when `DX == 1`; otherwise 4/5. It needs Up/Down
  while the player is touching it.
* BUTTON sends 4/5 only; it cannot open a DOOR.

### KNIGHT (44)

`DX` is a hold-frame selector, not a velocity: 0 = idle, 6 = frozen with the
sword extended (permanently lethal), 10 = run the full swing. `P11` is the
11-step swing loop. `P13` is the channel. Trigger codes 3/4/5 map to
swing/idle/freeze.

### SPINBLAD (50) / FIREBULLET (62)

`PARAM` (`p0f`) is a **lifetime**, not a pose:
SPINBLAD expires at 0x40 and gains +10 per background tile eaten; FIREBULLET
expires past 0x4F. `P13` is only animation.

### ELEV (61)

`STATE` is a descent timer set when the player uses the elevator. `P13 == -1`
marks an elevator that must not auto-descend. `PARAM` caches the last Up/Down
input for sound selection. `DX/DY` are not authored controls.

### BOLL (41) / SNAKE (39)

`BOLL` sets its own `W/H` from a table indexed by `P13`, so changing `W/H` by
hand will be overwritten. `SNAKE` uses `W` as its remaining body length; the
art is drawn in 8-px segments.

### TEXT6 (20) / TEXT8 (21) / TXTMSG (34) / WISEMAN (16)

These are the string objects. `STRING` is the attached board string.
TEXT objects use `P13` as a lifetime and `DX/DY` as a text cursor offset.
TXTMSG uses `STATE` as its message countdown. WISEMAN uses `STATE` for the
animation, `DX` for facing, `P13` as a pacing countdown and `TTL` as a
message cooldown.

### APPLE (1)

`STATE 0` is a normal apple. `STATE >= 1` is a text-page number: touching it
opens that page in the `.vcl` viewer. `TTL` is used to stop the page reopening
instantly. Use `STRING`-bearing text objects for ordinary floating text.

### KILLME (3) and MACROTRIG (6)

Markers with zero size and no behaviour. `KILLME` is a "nothing" object;
`MACROTRIG` exists so a recorded macro/demo can refer to a position and block
the player. Neither needs properties.

### Kinds 64-68 (Jill 2/3)

`EYE` uses `DX/DY` as pupil offsets. `VINECLIMB` uses `P13` for the bob phase
and `DY` for the bob direction. `FLAG` uses `STATE` as a one-shot collected
gate and `P13` as its six-tick animation. `MAPDEMO` is a pure overlay and uses
no authored properties. `ROMAN` uses `P13` for its walk cycle, `DX` for
direction and `P1B` as a one-hit contact guard.

---

## Property-specific quick list

### `STATE` values worth knowing

* `0` is usually "normal/idle/closed/off".
* `1` often means "active/airborne/opening".
* `2..5` are movement modes for the player and a few enemies.
* `3` on CHECKPT is the ending marker; `4` is the spawn/recover state.
* `-1/0/1` on PAD select trigger codes 4/3/5.
* `>= 1` on APPLE selects a text page number.
* `1` on SWITCH/BUTTON is the down/pressed pose.
* `1` on FIRE means the one-shot damage already fired.

### `P13` values worth knowing

* Trigger channels: 15, 24, 25, 26, 32, 38, 42, 44, 51, 52, 61.
* Item id: 28 (TOKEN).
* Level id: 12 (CHECKPT).
* Tile id it becomes: 25 (FALLDOOR).
* Speed: 53 (PAC).
* Animation/counter: everything else marked "animation/timer" in the tables.

### `TTL`

Mostly a hit cooldown after `_hitplayer`; it stops the same enemy damaging the
player every frame. It is not a general lifetime. `WISEMAN` uses it for the
message cooldown and `APPLE` for the page-apple cooldown. Text objects time out
on `P13`, not `TTL`.

### `STRING`

Only meaningful for `CHECKPT`, `WISEMAN`, `TEXT6`, `TEXT8` and `TXTMSG` in the
Jill 1 set. Do not use it as a label on a door or an enemy; those ignore it.

---

## Editor tips

* Changing an animation `P13` is safe but will start the animation at a
  different phase; it is useful for de-synchronising several copies.
* Changing a `STATE` to a non-default value can start an object in a death,
  opening or already-used state. If an object vanishes or behaves strangely in
  playtest, set `STATE` back to 0 first.
* `DX`/`DY` are signed. Negative values are valid and common for left/up
  movement.
* Leave `FLAGS` at 0. The engine fills in `0xC000` while it runs.
* `W`/`H` are profile defaults. Only change them when you specifically want a
  different collision footprint; a few handlers overwrite them.
* The `P13` link helper in the object property panel is for trigger channels.
  It is not useful for objects that use `P13` as an animation frame.
