# Port additions: save/restore, splash, transform cheats, title fix

This note records the Win/SDL2 port features added on top of the original engine
data.  Confidence markers follow the other notes: `[verified]` from disassembly
or shipped bytes, `[inferred]` from consistent evidence, `[open]` unresolved.

## 1. In-game save/restore

### 1.1 Original keys and files **[verified]**

The main loop (`FUN_22ab_1531`) compares the key against a `CS:0x19DF` table:

| key | near handler | action |
|--|--|--|
| `Esc` | `CS:0x1A66` | quit prompt |
| `Q` | `CS:0x1A66` | quit prompt |
| `R` | `CS:0x1A20` | `FUN_22ab_22b2` = LOAD GAME |
| `S` | `CS:0x19F3` | `FUN_22ab_23c2` = SAVE GAME |

The table is in the code segment, not DGROUP: the loop uses `cs: cmp ax,[bx]`
at file offset `0x1447D`; the five keys are `1B, 51, 52, 53, BB`.

`FUN_22ab_23c2` builds the file name as:

```
strcpy(buf, <profile save prefix>)   ; jn1save / jn2save / jn3save / onesave
strcat(buf, ".")                     ; DGROUP:0x1451
strcat(buf, itoa(slot))              ; 0..5
FILE *f = fopen(buf, "wb")
FUN_22ab_0c9a(buf)                   ; serialiser
```

So the shipped names are `JN1SAVE.0` .. `JN1SAVE.5` (the workspace contains
`jillt/JN1SAVE.0` and `jillt/JN1SAVEM.0`; the `M` form is the temporary copy
built from the second scratch name).  All four profiles carry
`save_prefix`/`save_slots` in their generated data.

### 1.2 File layout **[verified]**

`FUN_22ab_0c9a` writes exactly the board layout:

```
u16 map[128][64]          0x4000 bytes, raw tile ids
u16 object_count
object[count]             31 bytes each
u8  header[0x46]          70-byte live HUD/state block
for each object whose string offset/segment words are non-zero:
    u16 length
    u8  text[length]
    u8  0
```

`FUN_22ab_0aa5` reads the same block and allocates fresh string storage; the
saved offset/segment words are only used as "has string" flags.  The port uses
`Board::save` for the write side, so a port save is a normal board file.

### 1.3 Port UI

* Pause menu (`Esc`): RESUME, SAVE GAME, RESTORE GAME, START GAME, OPTIONS,
  QUIT.
* In-game `S` opens SAVE GAME; `R` opens RESTORE GAME.  `Ctrl+R` keeps the old
  debug board reload.
* Six slots are listed as `SLOT n: LV x  score` (empty slots show `EMPTY`).
  Slot labels are derived from the saved header; the original's 12-byte
  player-entered labels are not reproduced.
* The default move-down alternate was `S` in the first port builds, which
  collided with the original SAVE key.  The loader now drops that one legacy
  default; existing custom bindings are kept.
* Headless checks: `--save N` writes slot N, `--load N` restores it before the
  run.  A round trip reproduces the map, object table (apart from normal tick
  movement) and header. **[verified by headless diff]**

## 2. Splash screens

The intro boards carry a publisher/title page as `CHECKPT` page 12 at board
coordinate `(240, 0)`.  Rendering that board page gives:

| game | page content |
|--|--|
| Jill 1 | starburst, title and credit text |
| Jill 2 | EPIC MegaGames art |
| Jill 3 | EPIC MegaGames art |
| Onesimus | ARK MULTIMEDIA art |

**[verified by board data and rendered pages]**

Port option: `MENUS` > `SPLASH: OFF / JILL1 / JILL2 / JILL3 / ONESIMUS`
(the list follows the installed game manifests).  It is saved as
`[video] splash` in `jill.ini`; `--splash N` selects it from the command line.
The screen is shown once at start-up for about 2.5 s or until a key, mouse click
or gamepad button, and is skipped in headless runs.  The port renders the
selected game's intro page rather than shipping separate PCX/LBM art.

**Temporarily disabled:** the start-up splash and the `MENUS > SPLASH` row are
turned off in `app.cpp` (`kSplashEnabled = false`) while the publisher screen is
reworked.  The renderer and `--dump-splash` remain for inspection.
**[inferred]**

## 3. Transform cheats

The cheat menu now has one entry per form, each calling the same
`1FAE:2E61` token path the pickups use:

| menu entry | item id | player kind |
|--|--|--|
| BECOME JILL | 0 | 0 (normal) |
| BECOME FROG | 4 | 57 |
| BECOME BIRD | 5 | 56 |
| BECOME SPIDER | 6 | 55 |
| BECOME FISH | 7 | 54 |

`--morph N` accepts the same item ids for headless tests. **[verified by the
original token dispatch and port handler table]**

## 4. Jill 2 level-title popups

The 36-entry title table is indexed by `level id + 4`, but most Jill 2/3 board
headers store 0.  The port now takes the level id from a numeric board file name
(`3.jn2` -> 3) before looking up the title, and recomputes the caption after a
CHECKPT warp restores the id.  Jill 2 board 1 therefore shows
`JILL GOES / UNDERGROUND` and board 3 shows
`JILL ENTERS / MONTEZUMA'S / CASTLE`. **[verified by header dump; inferred for
the warp path]**

## 5. MAPDEMO and ROMAN

* MAPDEMO (kind 67) is drawn and updated only on the game's first (hub /
  world-map) board.  This is the port's stand-in for the original draw guard at
  `DGROUP:0x34EB != 0`; the object is still present in `MAP.ONE`.
  **[inferred]**
* ROMAN (kind 68) uses frames 8-15 for `dx > 0` when the shape bank has them
  (Onesimus entry 44 has 17 frames).  On the 7-frame Jill 2/3 sheet the left
  frames are mirrored horizontally, so the walker always has valid art.
  **[verified frame counts; inferred for the mirror fallback]**

## 6. Air jump buffer (default off)

A jump that begins while the player is airborne is now consumed by default, so
holding the jump button through a fall does not make Jill bounce automatically
on landing.  `GAMEPLAY > AIR JUMP BUFFER: ON` restores the original
hold-to-bounce behaviour; `--air-jump-buffer` sets the same default.  The change
is confined to player state 2 (airborne); jumping off a vine with a direction
still uses the state-3 button path. **[inferred from the requested default]**

## 7. Minimap

`GAMEPLAY > MINIMAP: ON` (or `--minimap`) draws a 128x64 overview at one pixel
per 16x16 tile in the lower-right corner: dark blue open cells, grey solid
cells, cyan water, red lava/spikes, green checkpoints and a white player dot.
It is drawn only with the in-game HUD, beneath the bottom message line.
**[inferred: presentation-only feature]**

## 8. Tighter menu line spacing

Menu rows use 9 px instead of 12 px and the panel/footer heights are computed
from the row count.  The 14-row CHEATS screen now fits on the 320x200 screen.
**[verified by layout arithmetic]**

## 9. Weapons stopping after many spawns

`FUN_26f4_2506` compacts the object array after every draw pass, dropping every
record whose kind is `3` (KILLME).  The port never did that, so corpses, score
popups, explosion particles and expired blades accumulated until `_addobj`
returned -1 and knives/blades appeared to stop firing.  `Engine::compact_objects`
now runs at the start of every tick and the spawn cap follows the original
`0x100` records (board files still cap at 192). **[verified against the
original update tail]**

## 10. Hub map state survives a level

When a level is entered from the hub, the original writes the current board to
its `temp` save (`FUN_22ab_0c9a`), and the `!` return path reloads it.  The port
now keeps the hub map/objects/header in memory and restores that copy on `!`,
so completed entrances and any other map changes are preserved instead of
reloading `map.jnX` from disk.  The finished level's entrance is still consumed
after the restore. **[inferred from the original temp-save flow]**

## 11. SPIKE animation (Jill 1 board 2)

Tile `0xA6` SPIKE is drawn by the mode-0 tile repaint path as
`base_shape + ((frame >> 2) & 3)` (`1F28:08BF`).  Mode 2 just marks the cell for
repaint once every fourth frame.  The port now rewrites the shared tile record
to frames `0x5410..0x5413`, four ticks each.  Headless `--watch-tile 166` on
`2.jn1` shows the full cycle. **[verified]**

## 12. Fish, frog, spider, splash and video round

* **Fish facing:** `1FAE:2E61` only writes `p0f = dx` when `dx != 0`.  The port
  used to overwrite `p0f` every tick, so stopping before firing reset the facing
  to 0 and every shot went right.  `p0f` is now a true facing memory, and the
  shot vector uses the original `sign(p0f)*8 + sign(dx)*4`.
* **Fish shot height:** new `GAMEPLAY > CENTER FISH SHOT` option
  (`[gameplay] center_fish_shot`, `--center-fish-shot`) spawns the bullet at
  `y + (fish_h - bullet_h)/2` instead of the fish's top edge.
* **Morph alignment:** `1FAE:2E61` keeps the feet planted; the port now adds
  `old_h - new_h` to `y` on every morph.  The 10-px frog and 16-px fish no
  longer float after transforming from 32-px Jill.
* **Frog:** only update `p1b` when moving, set `state = 1` while airborne so the
  hop frames are actually used, and set `state = 0` on landing.  `_trymove` now
  returns blocked (0) when no horizontal movement was requested, which lets the
  frog's landing snap fire (`((y & ~0xF)+0x10 - h)`) instead of hanging one
  pixel above the floor.
* **Spider:** kind 55 is an empty stub in the original and its tileset is the
  empty shape entry 0.  The port now provides a small procedural 16x16 spider
  (body, eight legs, two eyes) and a two-pixel, eight-direction wall-climb
  movement using the mask-1 collision test.  It is reachable from the
  `BECOME SPIDER` cheat and from the p13=6 tokens in Onesimus boards 17 and 28.
  The Jill 3-specific p13=6 alternate-award path is still `[open]`.
* **Interpolation:** `OPTIONS > VIDEO > INTERPOLATE` (default ON) is a separate
  toggle for high-framerate sprite smoothing.  It only has an effect when HIGH
  FRAMERATE is not OFF; `--no-interpolate` disables it.
* **Splash:** the splash is now a two-page sequence found from the intro board:
  1. the publisher/logo cluster (largest MEGA/EPIC cluster; the EPIC MEGA logo
     in Jill 1/2/3, the top ARK/Onesimus logo cluster), then
  2. the title card (top-most TEXT6/TEXT8 object containing the first word of
     the game title).  This replaces the old fixed credits page at `(240,0)`,
     which looked like the menu backdrop.  `--dump-splash P` writes the pages
     as `P1.ppm`, `P2.ppm` for inspection.
* **Menu/splash camera:** `Engine::render_at` builds the draw list for an
  arbitrary camera, so the menu backdrop and splash no longer depend on where
  the intro board's demo player happens to stand.  The menu theme uses the
  intro-board picture panel at tiles `(36,0)`..`(55,13)` (20x14 cells,
  320x224 px), centre-cropped to the 320x200 frame.  `Engine::set_hide_text_kinds`
  suppresses the TEXT6/TEXT8 story/credits copy so only the tile artwork is
  baked into the backdrop.

## 13. Endings, Onesimus start, map return, Jill Select

* **Jill 1 kind table:** the Jill 1 initializer fills 69 kinds, not 64; the
  profile generator now emits entries 64..68 (`EYE`, `VINECLIMB`, `FLAG`,
  `MAPDEMO`, `ROMAN`).  This is what was missing on Jill 1's vines/chains: the
  blue `VINECLIMB` sparks (tileset `0x3D`) now draw, collide and animate.
  **[verified from the initializer stores and the board data]**
* **Onesimus start:** a new manifest key `map_board` separates the START GAME
  board from the hub.  Onesimus now starts on `1.ONE` (the executable's start
  string is `1.one`) and uses `MAP.ONE` as its world map; `1.ONE`'s exit warp
  loads the map.  Other games default `map_board` to `first_board`.
* **Jill 1 map spawn:** returning with `!` restores the stashed map and keeps
  the player at the entrance it left from.  The old code also ran
  `place_at_checkpoint(0)`, which teleported Jill to the map's `p13 = 0` music
  CHECKPT at `(0, 16)` - the glitchy spawn.  Hub restores now skip checkpoint
  placement and re-centre the camera on the restored player.
* **Endings:** a `state = 3` CHECKPT now sets the ending-done flag, which the
  front end turns into a return to the main menu (the original sets
  `DGROUP:0xBCAA = 2` and fades at `22AB:266F`).  `&end1.mac` / `&end2.mac`
  strings are no longer ignored: the port parses the recorded-input format
  (`jill1-engine.md` 13.1) and replays movement/jump/fire records while the
  ending sequence runs.
* **Text pages:** the `.vcl` viewer now advances one screenful ("box") at a
  time with Left/Right, wrapping to the next/previous non-empty page at the
  edges, and the footer shows `PAGE n` plus `SCREEN k/m`.  Up/Down still scrolls
  a single line (or eight with Shift).  This keeps each page inside one box and
  reports the bank page number actually shown.
* **Jill Select:** `GAMEPLAY > JILL LOOK` cycles GAME DEFAULT / JILL 1 / JILL 2 /
  JILL 3 / ONESIMUS (any installed game).  The player art bytes are identical
  across the episodes; only the entry's 16-colour translation table differs
  (green/red/blue/grey outfit).  The port reads that table from the selected
  game's `.sha` and remaps the live player pixels, so no second shape bank is
  loaded.  `JILL COLOUR` applies a custom suit preset (green, red, blue, yellow,
  orange, cyan, purple, silver) and also drives the SUIT HUD background.
  Saved as `[gameplay] player_look` / `player_suit`; CLI `--jill-look N` and
  `--jill-colour N`.  **[verified tables read from all four .sha files]**
