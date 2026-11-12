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

`OPTIONS > MENUS` has six rows:

| Row | Meaning |
| --- | --- |
| STYLE | MODERN or CLASSIC; CLASSIC uses the original frame and shape-7 window |
| BORDER | THEME, CURRENT, an installed game's shape entry 7 window, or an external border |
| THEME | Backdrop artwork: port, a game intro board, or a pack/autoload theme |
| BACKGROUND | THEME, or LIVE GAME to show the paused game behind a modern menu |
| SPLASH | AUTO, OFF, or a specific game's start-up splash |
| FONT | ORIGINAL, or any named external `fonts/<name>` set |

The style is stored as `[menus] classic = 0/1`, the border source as
`[menus] frame = -1/0/1/2...` (`-1` = THEME), the font as `[menus] font`, and
the live background as `[menus] live_background = 0/1`.  They can be started
with `--classic-menu` / `--modern-menu` / `--menu-frame N` / `--menu-font NAME`
/ `--live-menu` / `--theme-menu`.  THEME selects the classic backdrop's shape 3
frame; BORDER selects the shape 7 window independently unless it is set to
THEME.  Border values above the installed-game count select named
`borders/<name>` assets.

`OPTIONS > HUD > PANEL` selects an external HUD panel matching the current
CLASSIC/MODERN style (ORIGINAL, or any `hud/classic_<name>` /
`hud/modern_<name>`); it is stored as `[hud] panel` and can be set with
`--hud-panel NAME`.  `OPTIONS > HUD > FRAME` selects the shape source used by
the CLASSIC HUD (CURRENT, or another installed game).  It is stored as
`[hud] frame` and can also be set with `--hud-frame N`.

`OPTIONS > SOUND > MIDI OUT` selects a system MIDI output device (such as
Microsoft GS Wavetable Synth or VirtualMIDISynth) for the CMF/MIDI tracks;
`OFF (OPL2)` keeps the built-in OPL2 emulation.  The selection is stored as
`[audio] midi_device` (the device index from `--list-midi`; `-1` = OPL2) and
can be started with `--midi N`.  MIDI music and sampled SFX can play together.
The port maps each CMF/AdLib instrument to a General MIDI program using the
track's actual note ranges and the OPL patch envelope, rather than sending the
AdLib instrument index as a GM program.

`OPTIONS > GAMEPLAY > TURTLE MODE` reproduces Jill 3's original TURTLE option:
the simulation advances at half speed (about 9.1 Hz).  High-framerate
interpolation stretches each update across two 18.2 Hz ticks, so movement stays
continuous instead of stepping.  It is stored as `[gameplay] turtle = 0/1` and
can be started with `--turtle`.

`OPTIONS > VIDEO > STEREO 3D` cycles `OFF` and `INTERLACED`.  The two eye
views use a small camera separation: the backdrop stays fixed, tiles move half
as far and objects move the full distance.  INTERLACED alternates even lines
from the left eye and odd lines from the right (for line-interleaved
displays/shutter glasses).  Menus and the level-select panel stay 2D.

SBS output is currently disabled and its extra depth/convergence rows are
hidden from the menu.  The values still exist for the interlaced path:
`[video] stereo`, `[video] stereo_depth` (1..8, default 2) and
`[video] stereo_convergence` (0..8, default 0); they can be set with
`--stereo interlaced`, `--stereo-depth N` and `--stereo-convergence N`.

`OPTIONS > VIDEO > MESSAGE BOX` cycles three styles:

| Style | Behaviour |
| --- | --- |
| OFF | Plain text hints; the level-title card keeps its port frame |
| MODERN | Port frame behind the bottom hint and the level-title box |
| CLASSIC | Original shape-7 windows: the 64x70 Jill portrait caption and the titled `Reminder` card |

CLASSIC is stored as `[gameplay] message_box = 2` (`1` = MODERN, `0` = OFF) and
can be selected with `--classic-message-box`.  The original portrait is shape
entry 24, also exported as `portrait.png` by the extractor.  A fresh game shows
the `Reminder` card after the first level-title caption.

The main menu always has `RESTORE GAME` and `HIGH SCORES`.  On `RESTORE GAME`,
Left/Right changes the episode and the slots then show that game's saves.
`HIGH SCORES` also uses Left/Right to switch episodes.  Each game/mod keeps its
own table in `hiscore_<game id>.txt`.

While a game is running the main menu also shows `QUIT TO MAIN MENU`, which
returns to the front end and records the current score if it makes the table.

When a score qualifies, a name prompt appears first:

| Key | Action |
| --- | --- |
| A-Z, 0-9, space | Add a character (up to 9) |
| Backspace | Delete the last character |
| Enter | Save the score |
| Esc | Save with the default name `JILL` |

Leaving the name blank also saves it as `JILL`.

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
**B** goes back, **Start** pauses/resumes.

### Level select (F3)

| Key | Action |
| --- | --- |
| Up / Down | Previous / next board |
| Left / Right | Previous / next episode |
| Enter | Load the selected board |
| Esc or F3 | Close the panel |

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
| Left / Right | Previous / next text page |
| Esc, Enter, F1 | Close the viewer |

Gamepad: D-pad/stick scrolls, left/right changes pages, A/B/Start/Back close.

## Front-end NOISEMAKER

| Key | Action |
| --- | --- |
| Up / Down | Select SET, SOUND, PLAY or BACK |
| Left / Right | Change the selected value |
| Enter | Play the selected raw SFX slot, or leave via BACK |
| Esc | Back to SOUND |

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
