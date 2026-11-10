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
