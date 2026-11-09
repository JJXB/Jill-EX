# Jill port (skeleton)

A native Windows C++17/SDL2 port of the three Jill of the Jungle games.  The
goal is behavioural parity with the DOS originals, so all game constants come
from `docs/` (and `docs/profiles.json`), not from per-episode code.

Current state: the engine core runs and renders boards for all three episodes.
The player, the weapon (knife / spin-blade), the pickups, the score popups and
every object kind (0-68) are ported: enemies, hazards, multi-part crawlers,
swimming fish, levers, elevators and OneSimus's extra kinds.  The original HUD
panel is available as an option (`--hud`).

## Build

Requirements: Visual Studio 2022 (MSVC), CMake 3.18+, and vcpkg with SDL2.

```
build.cmd            build (Release)
build.cmd install    first time: install sdl2:x64-windows, then build
```

Output: `port\build\Release\jill.exe` (SDL2.dll is copied next to it).

`build.cmd` expects vcpkg at `C:\vcpkg` (override with `VCPKG_ROOT`) and the
VS2022 `vcvars64.bat` at its default path.  SDL2 is optional: without it the
code still builds, but only headless mode (`--ticks`) works.

## Run

```
run.cmd                       first installed game (Jill 1)
run.cmd --game jill2          a game by manifest id (--list-games shows them)
run.cmd --episode 3           N-th installed game (1-based shorthand)
run.cmd --list-games          print id / title / profile / data / board count
run.cmd --games MODS          use a different manifest directory
run.cmd --profiles DATA       use a different engine-profile directory
run.cmd --board 4             a specific board (extension optional)
run.cmd --hud                 use the original CLASSIC HUD panel (F2 toggles)
run.cmd --hud-bg suit         force the SUIT HUD background (now the default)
run.cmd --hud-bg vibrant      brightest shade of Jill's suit for the HUD
run.cmd --hud-text N          HUD label colour palette index (default 7)
run.cmd --max-knives 5        start with the 5-knife enhancement on
run.cmd --invincible          debug: no damage (F5 toggles)
run.cmd --super-jump N        cheat: treat N feathers (item 9) as carried, 0..15
run.cmd --menu-theme N        front-end theme: 0 port, N = N-th game's intro board
run.cmd --fire-crouched       start with the crouch-fire enhancement on
run.cmd --retain-items        start with the keep-inventory enhancement on
run.cmd --fast-dialogs        start with the faster dialog boxes enhancement on
run.cmd --no-message-box      plain bottom messages without the frame
run.cmd --text-page N         open text page N (1 = "How to Play" in Jill 1)
run.cmd --fullscreen
run.cmd --windowed             windowed at --scale (default)
run.cmd --borderless           borderless fullscreen
run.cmd --resolution 1280x720  window/display size, or 'desktop'
run.cmd --scale 4              windowed integer scale, 1..6
run.cmd --high-fps 144         presentation rate: 0 (18.2 Hz), 60, 90, 120, 144
run.cmd --blade                debug: spawn one spinning blade
run.cmd --morph 5              debug: transform (0,4=frog,5=bird,6=spider,7=fish)
run.cmd --fire                 debug: press throw on the first tick
```

The game files and the starting board come from a manifest in `games\`
(`games\jill1.game` etc.); `--data DIR` overrides the manifest's asset
directory.  `run.cmd` changes to the repository root first, so `games\` and
`jillt\` resolve for mods and for a double-click start.

`jill.ini` holds the saved front-end state: game id, video mode/scale/
resolution/high framerate, HUD style/background, gameplay enhancements, key
and gamepad bindings, and a `[paths]` section with the `games`/`profiles`
directories plus one asset directory per game id.  Edit it to relocate game
data; command-line arguments still win for a single run.  The file is rewritten
when the game exits.

Controls: arrows or WASD move, Up/Down climb, Z / Ctrl / Space jump, X / Alt
throw, F11 toggle windowed/fullscreen, F2 toggle HUD art, F5 toggle
invincibility, R reload board, F1 open the help text page (page 1), Esc returns
to the main menu.  Every movement/jump/throw
action can also be bound to a gamepad button; see **OPTIONS -> CONTROLS**.

Gamepads: Xbox pads use SDL's XInput path and legacy/DirectInput pads use the
DirectInput path (`SDL_HINT_XINPUT_ENABLED` / `SDL_HINT_DIRECTINPUT_ENABLED` are
set before `SDL_Init`).  A pad SDL does not map as a controller still works
through a raw joystick fallback (left stick and hat 0 for movement, buttons 0/2
for jump/throw).  Devices can be plugged in or removed while the game runs.

In menus the D-pad moves, A confirms, B goes back, and **START** opens the pause
menu (and closes it again).  The same pad buttons drive the F3 level select.

## Menus

The game opens on a main menu.  Esc pauses and returns to it at any time.

* **START GAME** -> game select.  The list is built from `games\*.game`, so it
  contains the three shipped episodes plus any mods you install.  Choosing an
  entry starts on that game's `first_board` (the world map for Jill 1 and 3).
* **OPTIONS** -> `HUD`, `VIDEO`, `CHEATS`, `CONTROLS`, `MENUS`, and
  `GAMEPLAY`.
* **VIDEO** -> `WINDOW TYPE` (`WINDOWED` / `FULLSCREEN` / `BORDERLESS`),
  `WINDOW SIZE` (1x..6x, or `AUTO` when a resolution is set), `RESOLUTION`
  (`DESKTOP`, `640x480`, `800x600`, `1024x768`, `1280x720`, `1600x900`,
  `1920x1080`), `HIGH FRAMERATE` (`OFF`, `60`, `90`, `120`, `144`), `MESSAGE
  BOX` (ON/OFF: frame behind bottom messages) and a `FILTERS` placeholder.
  High framerate only changes presentation: the world still ticks at 18.2065
  Hz, and `OFF` presents at that same rate.  Object and camera positions are
  interpolated between ticks, so 60/90/120/144 Hz smooth the motion instead of
  repeating the same frame.  Fullscreen uses the chosen
  resolution as the display mode; windowed resizes to it.  Left/Right steps a
  value backwards/forwards; `--windowed`, `--fullscreen`, `--borderless`,
  `--resolution WxH`, `--scale N` and `--high-fps N` set the initial state.
* **MENUS** -> `THEME`: `PORT` plus one entry per installed game.  A game theme
  uses that game's own `intro` board as a dimmed backdrop (the same board the
  original shows around its menu), and frames the menu panel with that board's
  own tiles: its most common blocking tile for the border ring and its most
  common walkable tile inside.  Themes load from the game's own data directory;
  a missing file falls back to the plain background.
  `--menu-theme N` selects one at startup.
* **HUD** -> `STYLE` (MODERN / CLASSIC), `BACKGROUND` and `TEXT`.  Background
  cycles GREY, BLACK, DARK BLUE, DARK GREEN, DARK RED, BROWN, DARK PURPLE,
  SUIT and SUIT VIBRANT; the default is SUIT.  SUIT picks the darkest shade of
  Jill's leotard for the current episode (green in Jill 1, red in Jill 2, blue
  in Jill 3), derived from the player sprite rather than hardcoded; SUIT VIBRANT
  picks the brightest shade of the same hue.  `TEXT` cycles the 16 EGA text
  colours used for the HUD labels.  A sample bar at the bottom of the menu
  previews the current choice.
* **CHEATS** (in the pause menu) -> `INVINCIBLE`, `DEBUG FLY`, `SUPER JUMP`,
  `ADD KNIFE`, `ADD KEY`, `ADD GEM`, `ADD BLADE`,
  `REFILL HEALTH`, `BACK`.
  `ADD BLADE` grants one spinning-blade item (item 8); each blade in the
  inventory allows one blade in flight.
  The debug toggles are the same ones bound to F4/F5.  `SUPER JUMP` cycles
  `OFF, 1..15` and guarantees at least that many feathers (item 9): the jump
  still runs the original `-(16 + 4*feathers)` formula, so picking up more
  feathers raises it further.  `--super-jump N` starts at level N (bare
  `--super-jump` = 15).
* **CONTROLS** -> one row per action (`MOVE LEFT/RIGHT/UP/DOWN`, `JUMP`,
  `THROW`) showing both keyboard slots and the pad button.  Enter on a row
  captures the next key press (or controller button) into slot 1; Esc cancels the
  capture.  `RESET DEFAULTS` restores the built-in bindings and `BACK` returns to
  OPTIONS.
* **GAMEPLAY** -> optional gameplay changes.
  * `FIRE WHILE CROUCHED`: the original clears the facing field while crouching,
    so Jill cannot throw.  With this on she keeps the last facing direction,
    Left/Right while crouched aims without standing up, and the thrown
    knife/spin-blade starts at crouch hand height (`frame 0x3D`, `y + 0x12`)
    instead of the standing offset.
  * `MAX KNIVES`: the original caps knife pickups at 3; turn this on to allow
    up to 5 (`--max-knives 5` starts with it on).  The choice is saved in
    `jill.ini` with the other gameplay options.
  * `RETAIN ITEMS BETWEEN LEVELS`: off by default, so keys and weapons are
    dropped when a level is exited (matching the original header handling).
    Turn it on to carry the whole inventory across board warps;
    `--retain-items` starts with it on.  Unused gems (item 3) always carry
    over, even with this option off.
  * `FASTER DIALOG BOXES`: level-title dialogs and board text pages (the
    intro/ending "news" pages, TEXT6/TEXT8) type out at 1 character per tick
    by default; with this on they reveal at 4 per tick and the title caption
    also stays up for less time (0x50 instead of 0x78 ticks).
    `--fast-dialogs` starts with it on.  The bottom message line keeps the
    original `0x50`-tick, all-at-once timing.
* **RESUME** returns to the paused game; **QUIT** exits.

`--board NAME` skips the main menu so a specific board can be loaded directly.

The default MODERN HUD is one horizontal strip across the top of the screen,
left to right: `LEVEL` number, one icon per distinct item with its picked-up
count, `HEALTH` pips, then `SCORE` right-aligned at the last position.  Its
background defaults to the episode's SUIT colour (green/red/blue); pick another
under `HUD -> BACKGROUND`.  Select
`HUD -> STYLE: CLASSIC` (or `--hud` / F2) for the original 64x84 top-left panel
with the vertical item grid.

Debug tools:

* **F3** opens the level select (game + shipped board list).  Left/Right
  change the game, Up/Down the board, Enter loads, Esc/F3 closes.  The world
  pauses while the menu is open, and the list is the selected game's `boards`
  key from its manifest.
* **F4** toggles debug fly: the arrow/WASD keys reposition the player directly
  (4 px per tick, no collision or gravity) so elements further into a board can
  be reached quickly.  A `FLY` marker is drawn in the top-right corner.
* **F5** toggles debug invincibility: `player_kill` ignores all damage, so
  spikes, lava, enemies and falls cannot hurt the player.  An `INV` marker is
  drawn next to `FLY`.  `--invincible` starts with it enabled.
* **F8** prints the current status line (board, health, objects, camera).
* **F1** opens text page 1 (F1 help); while a page is open Esc/F1 closes it.

## Headless mode (no window)

```
jill --episode 1 --board 1 --ticks 60 --dump frame.ppm
```

Runs `N` fixed updates with no input, prints the state every 60 ticks, and can
write the final 320x200 frame as a binary PPM.  Useful for regression checks
and for checking rendering on a machine without a display.

`--dump-objects FILE` writes the live object table after the last tick
(`idx kind x y dx dy w h state p0f p11 p13 flags ttl`), which makes it easy to
diff motion between two run lengths:

```
jill --episode 1 --board 1 --ticks 1   --dump-objects a.txt
jill --episode 1 --board 1 --ticks 400 --dump-objects b.txt
```

Remember that only objects inside the camera plus margins are updated (the
original's active list), so off-screen enemies legitimately stay put in a
headless run.

`--start X,Y` (with `--ticks`) teleports the player to a pixel position for a
run; it is a testing aid for touching hazards and triggers without input:

```
jill --episode 1 --board 2 --start 48,864 --ticks 1
```

`--level N` (with `--ticks`) overrides the board header level id, which makes
the `!` exit and CHECKPT warps reachable without walking the whole hub:

```
jill --episode 1 --board 1 --level 1 --start 1872,208 --ticks 4
```

Warps and level entrances/exits are live.  A CHECKPT with a bare board string
(and `p13 != level id`) loads that board and puts the player on the target
board's entrance CHECKPT.  `!` loads the episode's first board and puts the
player on the entrance that led into the finished level.

Tile hazards are live: `SPIKE` (`0xA6`), `STALACBR/STALAGBR`, `WTHORN(2)` and
the water surface kill or damage the player, lava adds an `EXPL2`, and water
body tiles only make the throttled splash sound. See
`docs/jill1-player-physics.md` #5 for the tile table.

## Games and mods (game manifests)

An installed game is one text file, `games\<id>.game`.  It names the asset
files, the installed board list, the starting/backdrop boards and the engine
profile whose kind tables and handler tables drive it.  Adding or removing a
file changes the game list; there is no hardcoded episode range in the code.

The engine data itself lives in `profiles\<id>.profile` and is loaded at
start-up too, so a total conversion can ship its own kind table, handler
tables, item icons and palette without rebuilding the port.  A same-engine
conversion is usually four lines:

```
id = onesimus
base = jill3
kind_count = 69
```

Produce both files from an executable and an asset folder with:

```
py -3 tools/mkprofile.py --exe PATH\GAME.EXE --id mytc --data PATH\ASSETS
```

The tool finds the MZ image, kind-name block, kind-table initialiser, DGROUP
base, field tables, handler tables, item icons, palette, board extension/list
and first/intro boards, picks the nearest shipped base profile, and writes
`profiles\mytc.profile` + `games\mytc.game`.  It reports anything it could not
prove; `--force` overwrites, `--board-ext` / `--boards` / `--first-board` /
`--intro-board` override a heuristic.  See `docs/game-manifests.md`.

```
id = mymod
title = MY MOD
profile = jill1          # compiled engine data to reuse (jill1/jill2/jill3)
data = mods/mymod        # asset directory, relative to the working dir
shapes = mymod.sha       # defaults to the profile's shape bank
tiles = jill.dma         # defaults to the profile's tile table
pages = jill1.vcl        # optional: text-page bank, defaults to the profile's
board_ext = .jn1         # defaults to the profile's extension
first_board = 1          # no extension: board_ext is appended
intro_board = intro
palette = mymod.pal      # optional 768-byte raw palette
suit = 28                # optional: overrides the derived suit colour
hud_icon_bank = 14       # optional: shape bank for item icons/health pips
boards = 1, 2, map, intro
```

Only `profile` is required; every other key falls back to that profile's
compiled defaults.  Keys are case-insensitive, `#`/`;` start a comment line,
and an unknown key prints a warning without rejecting the file.  A manifest
whose `profile` is unknown is skipped and reported on stderr.

To install a mod:

1. run `py -3 tools/mkprofile.py --exe GAME.EXE --id mymod --data DATA`;
2. review `profiles\mymod.profile` / `games\mymod.game` (the report lists any
   value it had to inherit instead of proving);
3. run `jill --game mymod` (or pick it in START GAME).

The three shipped manifests and reference profiles are generated from
`docs/games.json` + `docs/profiles.json` so the derived keys stay in sync:

```
py -3 re/gen_games.py
```

Mods do **not** need to rebuild the executable.  A `.ONE` asset set (e.g.
Onesimus) profiles with the same command; the board format is byte-compatible
with `.jn1`/`.jn2`/`.jn3`, so the tool detects `board_ext = .ONE` automatically.

## Layout

| file | contents |
|--|--|
| `src/main.cpp` | command line |
| `src/app.cpp` | SDL window, texture, fixed 18.2 Hz timestep, palette |
| `src/engine.cpp` | active list, update pass, collision, camera, render |
| `src/player.cpp` | player state machine (kind 0) and draw frames |
| `src/board.cpp` | `.jn1/.jn2/.jn3` parser (map, objects, 0x46 header, strings) |
| `src/tiles.cpp` | `jill.dma` tile table (600 slots) |
| `src/shapes.cpp` | `.sha` directory and frame decoder (2/3/4/8 bpp) |
| `src/textpages.cpp` | `.vcl` text-page table (`0x190`/`0x230`) and line parser |
| `src/profile.cpp` | compiled/runtime engine-profile lookup (`profiles/*.profile`) |
| `src/game.cpp` | runtime game manifests: parser, discovery, built-in fallback |
| `src/gamedata.inc` | **generated** - compiled fallback profiles and kind tables |
| `src/palette.inc` | **generated** - default 256-colour palette |
| `games/*.game` | **generated** - installed game manifests (mods live here) |
| `profiles/*.profile` | **generated** - runtime engine data (mods live here) |
| `tools/mkprofile.py` | profile a new EXE + asset folder (no rebuild) |

Never edit `src/gamedata.inc`, `src/palette.inc` or the shipped `games\*.game` /
`profiles\*.profile` files by hand; regenerate them with

```
py -3 re/gen_port_data.py
py -3 re/gen_games.py
```

after changing `docs/profiles.json` / `docs/games.json`.

## Behaviour notes

* Fixed update tick: 18.2065 Hz (one BIOS tick), 320x200 8-bit framebuffer,
  ARGB8888 texture, nearest-neighbour integer scaling.
* Shape ids are `tileset << 8 | frame`; bit `0x4000` is bit 6 of the tileset
  byte (it selects `.<sha>` entries `0x40..0x7F`), **not** a mirror flag.  The
  blitter never reverses pixels; directional art uses separate frames.
  `tileset` indexes the `.sha` directory, `frame` the decoded frame list.
* Map words contain the tile id in the low 14 bits.  The shipped boards set
  `0xC000` on almost every cell; those are the original's "redraw" flags, so
  the port strips them on load and only uses the bits for per-frame object
  footprints.
* Collision uses `(tile_flags | extra) & mask` over the object's tile span,
  with `extra = 0` on rows below the kind's own height (see
  `docs/jill1-player-physics.md`).
* The player starts in state 0 and the shared state tail is implemented:
  walking, gravity, jump, climbing, death and respawn, plus the weapon tail.
  The documented climb ends the instant the box leaves a climb tile; the port
  adds a short eight-frame pull-up ("raise") on the last upward step, showing
  the `0x1A`/`0x19`/`0x18` climb poses before the standing frame and redrawing
  solid ledge cells in front of her while she is still behind them.
* Jill 1 and Jill 3 start on their world map (`map.jn1` / `map.jn3`).  Jill 3's
  map avatar is kind 23 TINY (2 px/tick, `0x200` walkable mask); Jill 1's map
  uses the normal player.
* CHECKPT 12 drives level entrances/exits: a bare board string warps to that
  board, and `!` returns to the episode's first board (the hub).
* The bottom message line (`_putbotmsg`, `22AB:05b4`) is shown centred at
  `y = 186`, `0x50` ticks, all at once; `MESSAGE BOX` in the VIDEO menu puts a
  framed box behind it (`--no-message-box` starts without).  Level titles are
  a dialog box in the middle of the screen (`index = level id + 4` in the
  36-entry title table) and type out at the dialog speed; any input dismisses
  it after a short hold, so a direction still held from the previous level
  cannot eat the popup.  Board text (kinds 20/21) uses the same fonts (bank 2 =
  6 px, bank 1 = 8 px) and types out at the same speed.
* Text pages (`FUN_22ab_0f0a`): the `.vcl` sound bank also carries 40 optional
  text pages (offset/size table at `0x190`/`0x230`).  A page is a title plus
  `\r`-separated body lines with a leading `0`-`7` colour digit; the port draws
  it as a framed full-screen page with centred lines.  Up/Down scrolls,
  Left/Right steps to the previous/next non-empty page, Esc closes.  In game,
  kind 1 APPLE with `state > 0` shows page `state` when touched (Jill 1's
  "News bulletin" pages; Onesimus's "Book of Philemon" verses), and F1 opens
  page 1 = "How the Heck to Play".  The world is frozen while a page is up.
  `--text-page N` opens page N headlessly (1 = the help page in Jill 1).
  Gamepad: D-pad/left stick scrolls, left/right steps pages, A/B/Start/Back
  closes (raw pads use their hat).

## Implemented object kinds

Pickups and effects:

| kind | name | behaviour |
|--|--|--|
| 1 | APPLE | +12 score, health pip |
| 2 | KNIFE | throw/return weapon |
| 14 | KEY | inventory |
| 27 | SCORE | floating score popup |
| 28 | TOKEN | inventory item `p13` (key/knife/gem/spin-blade/feather/shield) + first-time hint; items 4-7 morph Jill, item 0 reverts |
| 33 | GEM | inventory |
| 36 | EXPL1 | thrown debris particle |
| 37 | EXPL2 | single explosion puff |
| 49 | EPIC | 8-frame spin, 25 per touch, 11 touches then a 10-particle burst |
| 16 | WISEMAN | talking NPC, shows its board text |
| 20 / 21 | TEXT6 / TEXT8 | static board text in the 6 px / 8 px font |
| 34 | TXTMSG | touch-triggered board message (8-tick cooldown) |
| 64 | EYE | body + pupil that tracks the player |
| 65 | VINECLIMB | bobbing vine hazard, pushes and damages |
| 66 | FLAG | one-shot 6-tick animation, +6 score |
| 67 | MAPDEMO | viewport-pinned two-run ribbon |
| 68 | ROMAN | ground walker, re-aims every ~30 ticks |

Enemies and hazards (draw + update + hit):

| kind | name | notes |
|--|--|--|
| 4 | BIGANT | walk, pause on block |
| 5 | FLY | sine drift, bounces off walls |
| 7 | DEMON | walks, spits FIREBALL 18, knife counter |
| 8 / 22 | BUNNY / FROG | hop towards the player |
| 9 | INCHWORM | 4-tick stride |
| 10 | ZAPPER | static 5-frame metal-spike hazard, 16 damage |
| 11 | BOBSLUG | slow crawl, pauses |
| 17 | FATSO | long-interval pounce |
| 18 | FIREBALL | flies, dies off screen or on contact |
| 19 | CLOUD | slow horizontal drift |
| 29 | ANT | walk, pause on block |
| 30 | PHOENIX | patrols, dies to the knife |
| 35 | BOULDER | rolls, falls, reverses |
| 38 | STALAG | falls when released |
| 43 | BAT | hangs, then flutters |
| 47 | CRAB | walks, sometimes drops a ledge row |
| 50 | SPINBLAD | homing thrown blade, bounces, expires after `0x40` ticks |
| 54 / 57 | JILLFISH / JILLFROG | morphed forms: swim/steer/fire, hop |
| 55 / 56 | JILLSPIDER / JILLBIRD | morphed forms: stub, flap/fire |
| 62 / 63 | FIREBULLET / FISHBULLET | bird/fish projectiles: arcing, flat |
| 31 | FIRE | six-frame flame, one-shot burn, expires after `0x0B` |
| 39 | SNAKE | three-part crawler (head/body/tail), knife shortens it |
| 40 | SEAROCK | bubble emitter, spawns BUBBLE 58 |
| 41 | BOLL | rolls and seeks the next boll, shatters or bounces on blocks |
| 44 | KNIGHT | lever-driven 11-step sword swing, frozen variant is lethal |
| 45 | BEENEST | aims, then spits BEESWARM 46; scores 11 |
| 46 | BEESWARM | jittered homing swarm, expires after `0xA1` ticks |
| 48 | CROC | two-part crawler, wakes when hit, reverses or dies |
| 51 | SKULL | decoration triggered by action 3, then animates |
| 53 | PAC | Pac-Man style corridor follower |
| 58 | BUBBLE | grows and wobbles in water, pops out of it |
| 59 | JELLYFISH | pulsed swim, bounces off walls, drifts |
| 60 | BADFISH | swims, blows bubbles, bounces off walls |

Platforms and triggers (update + hit + actions 3/4/5):

| kind | name | notes |
|--|--|--|
| 15 | PAD | sends trigger code 3/4/5 on touch |
| 12 | CHECKPT | level entrance/exit: warps on a board string, `!` ends the level |
| 23 | TINY | Jill 3 map avatar |
| 24 | DOOR | opens with a key or gem (trigger 3) |
| 25 | FALLDOOR | drops and writes a tile |
| 26 | BRIDGER | builds/erases a run of `0x1B0` / `0x8C` tiles |
| 32 | SWITCH | up/down toggles the trigger channel |
| 42 | MEGA | trigger starts a bouncing block |
| 52 | BUTTON | debounced channel toggle |
| 61 | ELEV | ride up/down on the UP/DOWN keys, carries tile `0x8A` |

The per-episode animation tables these handlers use (FLY, PHOENIX, DEMON,
FATSO, EXPL2, BAT, SNAKE, BOLL, KNIGHT, BEESWARM, CROC, SKULL, JELLYFISH,
BADFISH) are generated from `docs/profiles.json` by `re/gen_port_data.py`, so
no episode offsets are baked into the code.

## Known gaps

* Every kind 0-68 now has an update/draw/hit/trigger implementation.  The
  remaining `[open]` items are art details rather than missing handlers:
  KNIGHT's decompiled draw path is partly mangled but the documented frame
  tables match observation; PAC's corridor probe follows the decompiled
  neighbour tests.
* The DOOR panel frames at `0x88F4` / `0x88FC` and the BRIDGER erase tile
  choice are read as `[inferred]`; see `docs/jill1-kind-handlers.md`.
* `_crawl` treats spike/thorn tiles as walkable for enemies (the original
  treats them as holes, so walkers pace at the pit edge).  This is a
  deliberate port deviation; see `PROGRESS.md`.
* Enemy contact damage and death use the player state machine; knockback and
  invulnerability timers follow `docs/jill1-player-physics.md`.
* The HUD (`--hud`) uses the original layout and pips, but the glyph colour
  ramp is approximated; inventory icons and pips are colour-0 masked so the
  grey panel shows through.
* TOKEN pickups grant their inventory item (including the feather that raises
  the jump) and show the original one-time hint line.  Tokens 4-7 transform
  Jill into the frog (4), bird (5), spider (6) and fish (7) forms (kinds
  54-57); the `FOOF!` token (0) reverts to normal Jill.  The forms have their
  original movement, jump/throw behaviour and art.
* No sound, no saves, no demo playback and no tile pre-pass hooks (attribute
  bit `0x20`).  The front end is the port's own text menu; the F3 selector and
  F4/F5 debug modes are tools, not the original front end.  Warps themselves
  are data-driven (CHECKPT strings), not a hardcoded table.
* The port does not yet load `.vcl`/`.ddt`/`.cfg`; board data is all it needs
  so far.
