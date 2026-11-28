# Keyboard and gamepad shortcuts [verified]

Defaults come from `Bindings` in `port/src/app.cpp` and `jill.ini`.  The six
gameplay actions can be rebound in **OPTIONS > CONTROLS** (per game).  The
system keys below are fixed.

## Gameplay

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Move left | Left arrow or `A` | D-pad left / left stick |
| Move right | Right arrow or `D` | D-pad right / left stick |
| Climb / aim up | Up arrow or `W` | D-pad up / left stick |
| Crouch / aim down | Down arrow | D-pad down / left stick |
| Jump | `Z` or `Space` | A |
| Throw / fire | `X` or `Left Alt` | X |

Movement is held; jump and throw are edge-triggered.  Bindings are stored in
`[controls]` and `[controls.<game id>]`.

## Front-end menus

Menus always use the classic frame and shape-7 window style.  `OPTIONS > MENUS`
has these rows:

| Row | Meaning |
| --- | --- |
| BORDER | THEME, CURRENT, an installed game's shape entry 7 window, or an external border |
| THEME | Backdrop artwork: port, a game intro board, or a pack/autoload theme. A game theme also fills the 240x160 viewport with the intro-board section at tiles (113,54)..(127,63) |
| SPLASH | AUTO, OFF, or a specific game's start-up splash |
| FONT | ORIGINAL, or any named external `fonts/<name>` set |

The border source is stored as `[menus] frame = -1/0/1/2...` (`-1` = THEME) and
the font as `[menus] font`.  They can be started with `--menu-frame N` /
`--menu-font NAME`.  THEME selects the classic backdrop's shape 3 frame; BORDER
selects the shape 7 window independently of it.  Border values above the
installed-game count select named `borders/<name>` assets.

`OPTIONS > HUD > PANEL` selects an external HUD panel matching the current
CLASSIC/MODERN style (ORIGINAL, or any `hud/classic_<name>` /
`hud/modern_<name>`); it is stored as `[hud] panel` and can be set with
`--hud-panel NAME`.  `OPTIONS > HUD > FRAME` selects the shape source used by
the CLASSIC HUD (CURRENT, or another installed game).  It is stored as
`[hud] frame` and can also be set with `--hud-frame N`.

`OPTIONS > HUD > LAYOUT` selects a `.hudlayout` from the mounted pack or
`external\autoload\` (ORIGINAL plus each file stem).  It is a full classic HUD,
so switch STYLE to CLASSIC to see it.  Stored as `[hud] layout`; also
`--hud-layout NAME`.  The editor is `tools\hud_layout.py`; see
[docs/hud-layout-tool.md](docs/hud-layout-tool.md) for the live part names.
The built-in CLASSIC HUD uses the same positions as
`games/Classic.hudlayout` (health bar 46,27 23x7, score right edge 70).

`OPTIONS > HUD > NO HUD` hides the HUD completely and uses the full 320x200
view; it defaults to OFF.  Stored as `[hud] hidden`, with `--no-hud` /
`--hud-visible` for command-line runs.

`OPTIONS > HUD > BACKGROUND: SUIT` gives the modern bar a two-row drop shadow
(one solid dark edge plus a 50% checker row) so it reads as a raised strip over
the board.  Pack-supplied modern panels keep their own shadow/artwork.

`OPTIONS > SOUND > MIDI OUT` selects a system MIDI output device (such as
Microsoft GS Wavetable Synth or VirtualMIDISynth) for the CMF/MIDI tracks;
`OFF (OPL2)` keeps the software OPL2 emulation.  The selection is stored as
`[audio] midi_device` (the device index from `--list-midi`; `-1` = OPL2) and
can be started with `--midi N`.  MIDI music and sampled SFX can play together.

`OPTIONS > SOUND > OPL CORE` selects the software synthesiser: `NUKED OPL3`
is the default cycle-accurate LGPL-2.1 core, and `FMOPL` is the older
AdPlug YM3812 emulator kept for comparison.  It is stored as
`[audio] opl_core` (`0`/`1`) and can be started with `--opl-core N`.  The
choice does not affect SFX or MIDI output.
The port maps each CMF/AdLib instrument to a General MIDI program using the
track's actual note ranges and the OPL patch envelope, rather than sending the
AdLib instrument index as a GM program.

`OPTIONS > GAMEPLAY > TURTLE MODE` reproduces Jill 3's original TURTLE option:
the simulation advances at half speed (about 9.1 Hz).  High-framerate
interpolation stretches each update across two 18.2 Hz ticks, so movement stays
continuous instead of stepping.  It is stored as `[gameplay] turtle = 0/1` and
can be started with `--turtle`.

`OPTIONS > GAMEPLAY > ANIMATED WATER` cycles the four-frame water tile groups
(`0x190`..`0x1A5`) like the original tile hook; retail releases differ on
whether the water moves.  It defaults to ON, is stored as
`[gameplay] animated_water = 1`, and can be frozen with `--no-animated-water`.

`OPTIONS > GAMEPLAY > RESPAWN` chooses what death resets: `RESET TO CHECKPOINT`
(the default, matching the base game's registered level status such as elevator
and platform positions), `RESET TO LEVEL START`, or `NO RESET` (keep the live
board).  It is stored as `[gameplay] respawn_mode` (`0`/`1`/`2`) and can be set
with `--respawn N`.

`OPTIONS > GAMEPLAY > PHOENIX WATER DEATH` (default ON) makes the phoenix form
die when it touches a waterfall / water body tile, as in the base game.  Turn
it off to keep the old splash-only behaviour.  It is stored as
`[gameplay] phoenix_water_death = 1` and `--no-phoenix-water-death` disables it.

`OPTIONS > GAMEPLAY > THROW FROM VINES` (default OFF) lets Jill throw a knife or
blade while climbing a vine/ladder.  The original only allows throws in states
0 and 2.  It is stored as `[gameplay] throw_from_vines = 1` and can be enabled
with `--throw-from-vines`.

Message boxes always use the original classic style: the 64x70 Jill portrait
caption, the titled `Reminder` card, and the news-bulletin/text pages
(patterned shape-7 frame + blue plate).  The original portrait is shape entry
24, also exported as `portrait.png` by the extractor.  A fresh game shows the
`Reminder` card after the first level-title caption.

The front-end main menu is `START GAME`, `DEMO`, `RESTORE GAME`, `STORY`,
`HIGH SCORES`, `OPTIONS`, `NOISEMAKER`, `JILL MAKER`, `QUIT`.  It always has
`RESTORE GAME` and `HIGH SCORES`.  On `RESTORE GAME`, Left/Right changes the episode and the
slots then show that game's saves.  `HIGH SCORES` also uses Left/Right to switch
episodes.  Each game/mod keeps its own table in `hiscore_<game id>.txt`.

`START GAME` lists the installed episodes plus **CUSTOM LEVELS**, which plays a
board saved by JILL MAKER (`editor/<game id>/<name>`).  Menus with more than
ten rows are split into pages (a `page/total` counter sits beside the title);
Up/Down moves through all rows and flips the page at the boundary.  `OPTIONS >
HUD > NO HUD` hides the HUD for a full-frame view.

`STORY` opens the installed-game selector and then shows that game's
intro-board story page at full screen, exactly like the original front-end
page (`CHECKPT` page 0).  Esc/Enter closes it and returns to the selector.

While a game is running the main menu also shows `QUIT TO MAIN MENU`, which
returns to the front end and records the current score if it makes the table.

When a score qualifies, a name prompt appears first:

| Key | Action |
| --- | --- |
| A-Z, 0-9, space | Add a character (up to 9) |
| Backspace | Delete the character before the cursor |
| Left / Right | Move the text cursor |
| Enter | Save the score |
| Esc | Save with the default name `JILL` |

Leaving the name blank also saves it as `JILL`.  On Vita and Android the prompt
opens the platform text input (Vita IME / Android soft keyboard); text arrives
through SDL's native text-input events, so physical and on-screen keyboards both
work.  Vita physical keyboards are also accepted through key events.  PS4 and
Xbox use the on-screen keyboard instead: D-pad moves, A types, Y deletes,
LB/RB move the text cursor, START accepts, B keeps `JILL`.  `SHIFT` toggles
lowercase, and the bottom-right keys type `!` and `?`.

## In-game system keys

| Key | Action |
| --- | --- |
| `Esc` | Pause / open the main menu |
| `F1` | Open text page 1 (`How the Heck to Play`) |
| `F2` | Toggle CLASSIC / MODERN HUD |
| `F3` | Level/board select panel |
| `F4` | Toggle debug fly |
| `F5` | Toggle invincibility |
| `F8` | Print the current engine status to the console |
| `Tab` | Toggle the minimap |
| `F11` | Toggle exclusive fullscreen / window |
| `S` | Save game (when a game is running) |
| `R` | Restore game |
| `Ctrl+R` | Reload the current board |

## Menus

| Key | Action |
| --- | --- |
| Up / Down | Move the selection |
| Left / Right | Change a value, episode, pack or sound set |
| Enter / Keypad Enter | Confirm / enter |
| Esc | Back one screen; from the main menu it closes the menu |

Menus also accept the gamepad: D-pad or left stick moves, **A** confirms,
**B** goes back, **Start** pauses/resumes.  While a game is running, **Select**
(pad Back / touch pad) toggles the minimap just like `Tab`.

### Level select (F3)

| Key | Action |
| --- | --- |
| Up / Down | Previous / next board |
| Left / Right | Previous / next episode |
| Enter | Load the selected board |
| Esc or F3 | Close the panel |

### Level editor (JILL MAKER)

`JILL MAKER` on the front-end main menu opens an episode selector, then a
board selector.  Saved custom levels appear there as `CUSTOM <name>` rows after
`NEW LEVEL`.  The editor draws the board without the HUD and uses the mouse
for painting.

| Key / input | Action |
| --- | --- |
| `1` | Tile paint mode |
| `2` | Object place mode |
| `3` | Object select/drag mode |
| `4` | Tile region mode (click A then B) |
| `T` | Tile palette (click a tile, wheel/`[`/`]` pages) |
| `Tab` or `E` | Object palette (click a kind, wheel/`[`/`]` pages) |
| Left click | Paint tile / place object / select object |
| Right click | Erase to the erase tile / delete object |
| Shift + left click | Pick the tile or object kind under the cursor |
| Shift + right click | Pick the erase tile from the map |
| Left drag | Paint tiles, or move a selected object |
| Arrows / D-pad / left stick | Move the cell cursor |
| `WASD` / right stick | Pan the board |
| `Space`, `Enter` or pad `A` | Paint / place / select at the cursor |
| `Backspace`, `Delete` or pad `X` | Erase tile / delete object |
| `P` or pad `L3` | Pick the brush at the cursor |
| Pad `LB` / `RB` | Previous / next mode (tile, object, select, region) |
| Pad `Y` | Palette for the current mode |
| Pad `R3` | Cycle the tile paint tool (single / line / rect / flood) |
| Pad `Back` | Playtest (also dismisses playtest dialogs) |
| Pad `Start` | Open the editor menu |
| Pad `B` | Object properties; closes an open panel first |
| Mouse wheel | Pan vertically (`Shift` for horizontally); pages an open palette |
| `G` | Toggle the tile grid |
| `N` | Toggle the board minimap panel |
| `M` or `F6` | Cycle paint tool: single / line A-B / rectangle fill / flood fill |
| `C` / `V` | Copy / paste the selected or hovered object, or the tile region in region mode |
| `Z` / `Y` | Undo / redo |
| `O` | Object properties (kind, position, DX/DY, size, state, params, flags, TTL, string) |
| `R` | Place presets (gem door / key door) |
| `I` | Board info (level id, health, item count, score, flash, score copy) |
| `F2` | Quick-save to the current edit target |
| `F3` | Save As picker (existing edits or a new file) |
| `F4` | Load picker (original board or any saved edit) |
| `F9` | Quick-load the current edit target |
| `F5` | Playtest: tap starts Jill at the cursor cell, hold (~0.6 s) restarts from the board's spawn; `F5`, `Esc` or pad `Back`/`B` returns |
| `H` or `F1` | Scrollable control reference (keyboard / controller / touch) |
| `Esc` | Close the overlay; otherwise open the editor menu |
| Editor menu: `PAD TEXT` | Use the on-screen D-pad keyboard for object strings (PC; consoles always use it); stored as `[editor] pad_text` |

Select mode: pressing the action button again while the cursor is still over the
highlighted object lets go of it (toggle).  Panning the board (right stick,
wheel, `WASD`) drags the highlight cell along so it stays in view.

Mouse coordinates use the renderer's actual viewport/scale and saturate at the
frame edges; dragging is captured so it keeps painting while the button is held.
The pointer is confined to the game image while the editor is open; playtest
releases it.  In the tile/object palettes, the file pickers, the editor menu and
the board-info panel, the arrow keys, D-pad or left stick navigate the
highlighted entry; `Enter`/pad `A` accepts or closes the panel.  The `H`/`F1`
control reference is scrollable with Up/Down, Page Up/Page Down, the mouse wheel
or the D-pad; `Enter`/`A` closes it.  Keyboard, controller and touch bindings
are listed in separate sections.

**Touch bar (Android / Vita / PC):** the bottom bar has two rows of ten
buttons:

* Row 1: `MODE PAL PICK TOOL PAN SAVE LOAD PLAY INFO MENU`
* Row 2: `UNDO REDO COPY PASTE PROPS PRST GRID HELP MINI ERASE`

On PC the same bar is clickable with the mouse.  The board viewport ends
above it, so the bottom row of the map is not hidden behind the bar.
`PAN` is a toggle: while it is lit, dragging the canvas moves the camera;
tap it again to go back to painting.  `ERASE` toggles the canvas between paint
and erase.  Tapping a tile/object palette cell selects it (tap outside to
close the palette).  The file picker, editor menu, board info, object props and
presets also accept direct taps; props and info use the left/right side of a
row to decrease/increase the selected field.

`Esc` (or pad `Start`) opens the **editor menu**: SAVE, SAVE AS, LOAD, SAVE TO GAME
DATA, PAINT TOOL, UNDO, REDO, COPY/PASTE OBJECT, COPY/PASTE TILES, OBJECT
PROPS, PLACE PRESETS, PLAYTEST, BOARD INFO, HELP, MINIMAP and EXIT TO MAIN MENU.
SAVE TO
GAME DATA asks for confirmation and writes the current board into the game's
data folder (it can overwrite the original board).  This is the console-friendly
path to every save/load and editing action.

`M` or the editor menu cycles the tile brush through **single**, **line A-B**,**rectangle fill** and **flood fill**.  For line/rectangle tools, click/tap the
first cell, then the second, and the line or rectangle between them is filled
with the selected tile.  Flood fill replaces the connected same-tile region.
`Esc`/pad `B` cancels a pending A point.  Line and rectangle shapes are
previewed before committing.

`Z`/`Y` undo and redo map, object and header changes.  `C` copies the selected
or hovered object and `V` pastes it at the cursor.  `O` opens the object
property panel: Up/Down selects a field, Left/Right adjusts it, and Enter on the
STRING row opens the native text prompt (Vita IME, Android soft keyboard or
desktop keyboard).  On PS4 and Xbox the prompt is an on-screen keyboard:
D-pad moves, A types, Y deletes, LB/RB move the text cursor, SHIFT toggles
lowercase, B cancels.  Object W/H are editable there, which covers object resizing;
DX/DY are exposed for doors and moving objects.  The P13 row shows how many
trigger objects share the channel, and Enter on P13 links this object to the
nearest trigger sender/receiver.  The editor menu also has UNDO, REDO, COPY,
PASTE and OBJECT PROPS rows for controller-only use.

`N` toggles the **minimap** panel in the top-right corner: one pixel per board
cell in the live palette, with the camera rectangle, cell cursor and selected
object marked.  It is also an editor-menu row; the panel is redrawn only when
the board changes.

`R` (or the menu's PLACE PRESETS row) opens the placeable presets.  **GEM DOOR**
writes the 0x0BE map door tile and places a `DOOR` object on it.  **KEY DOOR**
writes the two stacked 0x0A1/0x0A2 door tiles and places a `DOOR` object.  Pick
the preset, then click/tap the board cells; the preset stays active like the
other object brushes until `Esc` cancels it or another mode/object is chosen.
See `docs/editor-doors.md` for a complete key/gem door setup guide and trigger
channel rules.

`4` enters **tile region** mode (also reachable by cycling modes with pad
`LB`/`RB` or the touch MODE button).  Click/tap the first and second corners;
the rectangle is outlined.  `C` copies those tiles and `V` pastes them at the
cursor; the clipboard size and a preview are shown in the top bar.  Right click
or `Esc` clears the region.  The editor menu has COPY TILES / PASTE TILES rows
for controller-only use.  Repeated property/header tweaks are grouped into one
undo step per field.

During playtest, `Esc`/pad `B`/`Back` first dismisses an open level-title,
message or text-page dialog and stays in the playtest.  Press again to return to
the editor; `F5` always returns to the editor.

Vita and Android show an on-screen toolbar along the bottom with MODE, PAL,
PICK, ERASE, SAVE, LOAD, PLAY and MENU, covering the console buttons that do
not exist (L2/L3/R2/R3).  The TOOL button cycles the tile paint tool
(single / line A-B / rectangle / flood fill); pad `R3` and `F6` do the same.

Edits are written as normal board files under
`editor/<game id>/<name>` (for example `editor/jill1/1.jn1`), leaving the
installed game data untouched.  `F3` opens the **Save As** picker,
which lists the existing edits and a `NEW FILE` entry; `F4` opens the **Load**
picker, which lists the original board plus every saved edit.  `F2` and
`F9` quick-save/load the current target.  The episode board selector also has a
`NEW LEVEL` row that starts a blank level with a two-row floor, a player object
and default health, then picks a `custom_N` file name for saving.

Copy a saved file over the original board in the game's data folder when it is
ready to ship, or start the editor again to keep tuning it.  The board header
fields are edited with `I`: Up/Down selects a field and Left/Right changes it.

While playtesting, the edited map and object list are used in memory; the
editor copy is restored when you return.

### Save / restore

| Key | Action |
| --- | --- |
| Up / Down | Select slot |
| Enter | Save or load |
| Esc | Cancel |

### Controls rebinding

| Key | Action |
| --- | --- |
| Up / Down | Select an action |
| Enter | Start listening for a new key |
| Any key / gamepad button | Assign it |
| Esc | Cancel the assignment |

## Start-up splash

The publisher/loading screens play before the main menu.  Any key, mouse click
or controller button skips the remaining pages.  `--no-splash` disables them.

## Ending cards

Ending boards show their text as full-screen cards.

| Key | Action |
| --- | --- |
| Enter, Space, click, pad A/Start | Next card |
| Esc, pad B/Back | Skip to the main menu |

Cards also advance automatically after seven seconds.

## Text pages

| Key | Action |
| --- | --- |
| Up / Down | Scroll one line |
| Page Up / Page Down | Scroll eight lines |
| Esc, Enter, F1 | Close the viewer |

Gamepad: D-pad/stick scrolls, A/B/Start/Back close.  The page itself cannot be
switched: the original shows one page per help screen.

## Front-end MUSIC PLAYER

`OPTIONS > MUSIC PLAYER` lists every `*.ddt` track in the running game's data
folder (plus its title track if that lives elsewhere), sorted by file name.  It
plays through the same CMF/OPL or MIDI path as the game, which makes A/B
testing a single track easy.

| Key | Action |
| --- | --- |
| Up / Down | Select a track (or BACK) |
| Left / Right | Music volume -10% / +10% |
| Enter | Play the track; press again on the playing track (`*`) to stop |
| Esc / pad B | Back to OPTIONS (the current track keeps playing) |

Starting a game or returning to the front end swaps in that screen's music as
usual.

## Front-end SOUND TEST

`OPTIONS > SOUND > SOUND TEST` is the raw-slot audition screen.

| Key | Action |
| --- | --- |
| Up / Down | Select SET, SOUND, PLAY or BACK |
| Left / Right | Change the selected value |
| Enter | Play the selected raw SFX slot, or leave via BACK |
| Esc | Back to SOUND |

## Front-end NOISEMAKER

`NOISEMAKER` on the main menu opens the original board page-99 sound keyboard.
The next screen selects the episode whose bank is used; choosing a game loads
its `intro` board at the `NOISE BLASTER` page.

| Key | Action |
| --- | --- |
| QWERTY row, digits, `-`, `=`, brackets, punctuation | Play raw sound slots 1..45 |
| Esc / pad B / Back | Return to the main menu |

## Front-end DEMO

`DEMO` on the front-end main menu plays the attract demos shipped with the
installed games.  The first screen lists the episodes that have demos
(`Jill of the Jungle`, `Jill goes Underground`, `Jill Saves the Prince`;
Onesimus ships none and is omitted).  An episode with a single demo starts it
directly, otherwise `DEMO 1/2/3` picks one.

| Key | Action |
| --- | --- |
| Up / Down | Choose the episode or the demo |
| Left / Right | Change the episode on the first screen |
| Enter / pad A | Start the highlighted demo |
| Esc, Enter / pad B, Back, Start | Leave the demo |

| Episode | Demos (board / macro) |
| --- | --- |
| Jill of the Jungle | `0.dem`, `2.dem`, `1.dem` - all `jn1demo.mac` |
| Jill goes Underground | `3.jn2`, `9.jn2`, `17.jn2` - `jn2dem1..3.mac` |
| Jill Saves the Prince | `1.jn3`, `5.jn3`, `12.jn3` - `jn3dem1..3.mac` |

Playback loads the demo board and level, replays the macro, and returns to the
main menu when the script runs out or the route reaches an ending marker.
`--macro NAME` replays the same scripts headless (`--ticks`), e.g.
`jill.exe --game jill2 --board 3.jn2 --level 3 --macro jn2dem1.mac --ticks 1100`.

## Extractor viewer

The extractor has no global accelerators; the following apply while the
relevant widget has focus [verified].

| Shortcut | Action |
| --- | --- |
| Up / Down in the frame box | Step one shape frame forward/back |
| `-` / `+` buttons | Step one frame |
| Grid checkbox | Show all frames of the selected shape |
| Click a grid cell | Select that frame |
| Click the board canvas | Inspect the tile under the cursor |
| Double-click a board list row | Open it in the Board Viewer |
| Left tab click | Switches the right viewer tab automatically |

The importer and batch extractor are command-line only; see `COMMANDLINE.md`
and `IMPORT_README.md`.
