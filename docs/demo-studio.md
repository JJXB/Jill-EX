# Demo studio (`.mac` viewer/editor)

`tools/demo_studio` is a standalone desktop tool that opens, plays and edits
the engine's recorded input scripts (`*.mac`).  It links the real engine core
from `port/src` and renders the actual game frame, so what you see is the game
as it plays, not a re-implementation.  Nothing in the tool is linked into the
shipped `jill` executable.

The tool is aimed at the work still on the TODO list: checking recorded
attract demos and ending macros, fixing input timing, and seeing what the
high-framerate interpolation actually changes.

## Build and run

```bat
tools\build_demo_studio.cmd          REM 64-bit (x64), needs SDL2 x64-windows
tools\build_demo_studio32.cmd        REM 32-bit (Win32), needs SDL2 x86-windows
tools\run_demo_studio.cmd            REM opens and plays the episode's first demo
tools\run_demo_studio32.cmd          REM same, using the 32-bit exe
tools\run_demo_studio.cmd --mac D:\Emu\DOS\C\jillt\jn2dem1.mac
tools\run_demo_studio.cmd --game jill3 --mac jn3dem2.mac
tools\run_demo_studio.cmd --mac end1.mac --board 50 --level 15
```

The two builds are independent: x64 output is
`tools\demo_studio\build\jill_demo_studio.exe` and Win32 output is
`tools\demo_studio\build32\jill_demo_studio.exe`, so building one never
overwrites the other.  If SDL2 for the matching triplet is missing, run the
build script once with `install` (it calls `vcpkg install sdl2:x64-windows` or
`sdl2:x86-windows`).

Both Windows release zips (`Jill-EX-release-x64.zip` and `-x86.zip`) now ship
the matching `jill_demo_studio.exe` in the package root.  It shares the bundle's
`SDL2.dll`, `games/`, `profiles/` and `jill.ini` paths with the game.

It reads `jill.ini` exactly like the game (including `[paths]` install
directories and the HUD/gameplay options), so no extra configuration is
normally needed.  `--help` lists every switch.

`tools\build_demo_studio.cmd` and `tools\build_demo_studio32.cmd` use the same
Visual Studio + vcpkg toolchain as `build.cmd` and write
`tools\demo_studio\build\jill_demo_studio.exe` (x64) and
`tools\demo_studio\build32\jill_demo_studio.exe` (Win32) respectively.
CMake can also build them directly:

```bat
cmake -S tools\demo_studio -B tools\demo_studio\build -A x64 ^
      -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build tools\demo_studio\build --config Release

cmake -S tools\demo_studio -B tools\demo_studio\build32 -A Win32 ^
      -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ^
      -DVCPKG_TARGET_TRIPLET=x86-windows
cmake --build tools\demo_studio\build32 --config Release
```

## Window

* **Left:** the live 320x200 game frame, integer-scaled (2x when the window is
  wide enough).  It includes the HUD, captions, text pages, music-driven
  animation and everything else the normal render path draws.
* **Under the game:** live state - board, level, score, health, player and
  previous-tick positions, camera, object count, the effective input and the
  interpolated draw position for the current alpha.
* **Right:** the input timeline.

The window is resizable; the layout reflows and the game view falls back to
1x when the timeline would get too narrow.  Text and row heights scale with
the `UI TEXT SCALE` option (`F4`), so the timeline is readable on high-DPI
screens and still shows more rows when set to `1X`.

## Options (`F4`)

| row | values |
|---|---|
| TEXT SCALE | `1X` compact, `2X` default, `3X` large |
| GAME VIEW | `AUTO`, or a forced `1X`/`2X`/`3X` integer scale |
| INTERPOLATION | same three modes as `F5` |
| CLASSIC HUD | on/off (same as `F6`) |

Changing the text scale resizes the window when the timeline needs more room
and grows the row height, column widths, editor and overlays together.  The
text scale and game view are remembered in `demo_studio.ini` next to
`jill.ini`; delete that file to return to the defaults.


## Setup (`F2`)

A recording only makes sense with the board and level spawn it was recorded
on.  `F2` edits the four start fields:

| field | meaning |
|---|---|
| GAME | installed episode (`jill1`, `jill2`, `jill3`, mods) |
| BOARD | any board file in the game folder (the manifest list plus extras such as `0.dem` and `50`) |
| LEVEL | CHECKPT level id; `-1` uses the board's own player marker |
| MACRO | open another `.mac` |

`LEFT`/`RIGHT` changes the selected row, `ENTER` applies it and restarts from
tick 0.  Opening a known demo file fills the fields from the front-end demo
table automatically:

| macro | game | board | level |
|---|---|---|---|
| `jn1demo.mac` | jill1 | `intro` | 100 |
| `jn2dem1/2/3.mac` | jill2 | `3.jn2` / `9.jn2` / `17.jn2` | 3 / 9 / 17 |
| `jn3dem1/2/3.mac` | jill3 | `1.jn3` / `5.jn3` / `12.jn3` | 1 / 5 / 12 |

An explicit `--game`, `--board` or `--level` pins only that field.  Other
macros (`end1.mac`, hand-made files) keep whatever start state is selected.

## Timeline

Each row is one record from the file:

```
#    TICK   D     H  V  B1 B2 KEY  MASK
0    0      -     -  -  X  .  -    ..1..
1    1      +1    -  -  .  .  -    ..1..
2    29     +28   -  U  .  .  -    .V...
```

* `TICK` is the absolute tick when the record takes effect.
* `D` is the tick difference from the previous record.
* `H`/`V` show the direction as `L`/`R`, `U`/`D` or `-`.
* `B1`/`B2` are the action buttons; `KEY` is the recorded ASCII key.
* `MASK` is the five channels the record actually sets (`H V 1 2 K`); a `.`
  means the channel keeps its previous value.  Mask-0 records are pure time
  markers and are legitimate in the format.

The strip above the table is the whole recording: one marker per record,
coloured by what it changes (blue = direction, yellow = jump, red = throw), the
green line is the playhead.  Click anywhere in the strip to seek.  Seeking
rebuilds the state from tick 0 by replaying the engine headlessly, so it works
backwards and forwards and lands on exactly the frame the game would show.

Records applied at the playhead are marked with a green edge.

## Editing

All edits live in memory until a save; the header shows `*` and the panel says
`EDITED - PRESS R TO RELOAD AND REPLAY THE CHANGES`.  The running preview keeps
using the old record list until `R` (or `F2`/`F3` starts a new run).

| key | action |
|---|---|
| `UP` / `DOWN` | select a record |
| `PAGEUP` / `PAGEDOWN` | move by one screen |
| `TAB` / `SHIFT+TAB` | next / previous field |
| type digits, `ENTER` | edit the selected field (`ESC` cancels) |
| `+` / `-` | step the selected field |
| `1`..`5` | with the `MASK` field selected: toggle H, V, jump, throw, key |
| `INS` | insert a record at the playhead with the current effective input |
| `DEL` | delete the selected record |
| `CTRL+Z` | undo (64 steps) |

Editing `H`, `V`, `B1`, `B2` or `KEY` sets that mask bit automatically, so a
value typed into a record cannot silently fail to serialise.

### Saving

* `CTRL+S` writes the loaded file (a `.mac` next to the game is overwritten in
  place).
* `CTRL+SHIFT+S` (or `CTRL+S` with no file open) asks for a name; the default
  is `<name>.edited.mac` beside the current file.

The writer emits the same layout the engine reads: record 0 is
`mask + values`, every later record is `varint tick + mask + values`, one signed
byte per set bit in bit order.  When the first record starts after tick 0, a
leading mask-0 marker at tick 0 is written so the delay is preserved.  Records
are written in stable tick order.

## Interpolation (`F5`)

The world tick is always 18.2065 Hz; interpolation only changes how the 60 Hz
presentation smooths between ticks.  `F5` cycles:

1. **SMOOTH (60 HZ, ALPHA BLEND)** - the engine's default high-framerate mode:
   sprites and the camera blend from the previous tick to the current one.
2. **OFF (60 HZ, HOLD LAST TICK)** - the presentation runs at 60 Hz but
   `alpha` is forced to 1, so each tick is held until the next.  Gameplay
   timing is identical; only smoothness changes.
3. **ORIGINAL SPEED (18.2 HZ)** - only one logic tick and one presented frame
   per 18.2 Hz step, matching the original cadence.

The `LIVE STATE` panel prints the current blend factor and the interpolated
draw position, which makes a stutter or a wrong smoothing span easy to spot.
Because the tick list and the rendered frame come from the engine, a recording
can be played to the tick in the original and in the port and compared.

For a side-by-side review, `tools\capture_demo_studio.cmd` opens the studio and
records the desktop with ffmpeg while you press `F5` to cycle modes:

```bat
tools\capture_demo_studio.cmd --interp smooth --mac jn1demo.mac
tools\capture_demo_studio.cmd --interp original --mac jn1demo.mac
```

The clips land in `captures\demo_studio_<timestamp>.mp4`.

## New files (`CTRL+N`)

`CTRL+N` (or `N` in the file picker) creates a fresh `.mac`.  The tool asks for
a name and writes a valid one-record file immediately: tick 0 with neutral
input (`H/V/1/2/K` all zero).  The `.mac` format has no header, so an empty file
would not load; this seed record is the starting point.

The new file is created in the folder of the currently open `.mac`, or in the
episode's asset folder when nothing is open.  A name with a path separator is
used as given, and `.mac` is appended when the name has no extension.  An
existing file is never overwritten: choose another name.

From there:

1. `F2` sets the episode/board/level the recording belongs to.
2. Move the playhead (`RIGHT` or click the strip) and press `INS` to add a
   record at that tick with the current effective input.
3. `TAB`/typing edits the record (direction, jump/throw, key, tick).
4. `R` replays the edited run; `CTRL+S` writes it.

`--new FILE` does the same from the command line (without the dialog), which
is handy for scripted setup or smoke tests:

```bat
tools\demo_studio\build\jill_demo_studio.exe --new %TEMP%\blank.mac --headless ^
    --ticks 0 --out %TEMP%\blank.ppm
```

## Playback keys

| key | action |
|---|---|
| `SPACE` / click the game view | play / pause |
| `R` | reload the board and replay from tick 0 |
| `LEFT` / `RIGHT` | step one tick (stepping back rebuilds the run) |
| `SHIFT+LEFT` / `SHIFT+RIGHT` | step 18 ticks (one second) |
| `HOME` / `END` | seek to tick 0 / the last record |
| `F5` | interpolation mode |
| `F6` | classic HUD on/off (classic also switches the simulation viewport) |
| `F4` | options: text scale, game view, interpolation, classic HUD |
| `CTRL+N` | new `.mac` from scratch |
| `F9` | write a PPM screenshot into `tools/demo_studio/shots` |

`F10` quits; with unsaved edits it asks for a second press, and opening another
file while edited asks for a second `ENTER`.

## Files (`F3`)

The picker scans `*.mac` in the episode's asset folder, the current directory
and the folder of the open file.  `F3` rescans and `N` starts a new file.  Bare names passed to `--mac`
are also looked up in the episode's folder, so
`--mac jn1demo.mac` works from the repository root.

## Headless checks

The same walk is available without a window, which is handy for regression
tests and scripted comparisons:

```bat
REM write the frame after N ticks
tools\demo_studio\build\jill_demo_studio.exe --headless --mac jn1demo.mac ^
    --ticks 120 --out shot.ppm

REM per-tick player/state CSV (tick, board, position, state, p0f/p11/p13,
REM input, camera) and the live 128x64 tile map at the final tick
tools\demo_studio\build\jill_demo_studio.exe --headless --mac jn1demo.mac ^
    --ticks 900 --trace run.csv --dump-map map.txt

REM parse -> edit -> serialise -> parse, report record/byte counts
tools\demo_studio\build\jill_demo_studio.exe --selftest --mac jn1demo.mac

REM the same, plus write/reload a real .mac at --out
tools\demo_studio\build\jill_demo_studio.exe --selftest --mac jn1demo.mac ^
    --out %TEMP%\edited.mac
```

`--trace` and `--dump-map` are for comparing a playback against the original:
they expose the exact per-tick state the engine used, not just the picture.
Macro playback always uses the original edge-triggered camera (the same one as
the game's `--classic-camera`), because that camera also decides which
off-screen objects update.  `--classic-camera` still selects it for a run with
no macro.

`--selftest` also re-encodes the untouched parse and reports
`reencode=identical` when the writer reproduces the original bytes exactly
(true for every shipped demo and ending macro).

The headless frame and the game's own `--dump` frame are byte-identical with
the same settings (`--board intro --level 100 --macro jn1demo.mac`), which is
the simplest way to prove a change did not alter playback.

## Notes and limits

* The engine is driven by the same `Engine::tick()` / `Engine::render()` as the
  game; the tool only replaces the input poller with the record list.
* There is no audio.  Sound effects and music requests are simply ignored.
* Boards that carry an entrance macro (`50.jn1`, `20.jn2`) start that board's
  own script on the first tick; the panel flags this if it replaces the file
  you opened.  Use `--board`/`F2` to preview the board itself.
* The tool does not record new input.  `INS` adds a keyframe at the playhead
  with the current input, which is enough to branch or fix a recording by hand.
* `.mac` has no header or checksum, so an edited file is just a new record
  stream; the engine's parser reports truncated or descending-tick files the
  same way it always has.
