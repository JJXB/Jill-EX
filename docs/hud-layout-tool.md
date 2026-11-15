# HUD layout tool

`tools/hud_layout.py` is a standalone editor for HUD alignment.  It overlays a
reference screenshot under the game's 320x200 frame, lets you lay tiles on a
resizeable grid, and saves a small text layout (`.hudlayout`) that can be read
back when the engine drawing needs to line up with an original screen or a
hand-drawn panel.

It does not read game data and it does not modify the engine.  The editor and
its format are authoring aids; the game keeps its normal external HUD panels
(`hud/classic.png`, `hud/modern.png`, ...) and always draws labels, icons and
pips itself.  See [EXTASSETS.md](../EXTASSETS.md) for the pack layout.

Requirements: Python 3.10+ with tkinter, and Pillow (`pip install pillow`).
Both are already needed by `jill_extractor.py`.

## Launching

```bat
py -3 tools\hud_layout.py                          REM empty editor
py -3 tools\hud_layout.py docs\examples\classic-hud.hudlayout
py -3 tools\hud_layout.py --summary my-hud.hudlayout
py -3 tools\hud_layout.py --render my-hud.png my-hud.hudlayout
```

`--summary` prints the coordinate report without opening a window, and
`--render` writes the stamped tiles (no reference, no guide parts) to a PNG.
The GUI wraps the same functions, so a saved file always round-trips through
both modes.

## Editing workflow

1. **Canvas and grid.**  The default is 320x200 with an 8x8 grid at (0,0).
   Set the grid cell and origin in the *Grid* box; `Show` toggles the overlay
   and `Snap` controls whether stamped tiles and dragged parts round to it.
   The grid is a placement aid: changing it later does not move tiles that
   were already stamped.
2. **Reference.**  Pick a screenshot (or mock-up) with `...` in the
   *Reference* box.  `Fit` scales it into the canvas, `Centre` positions it,
   and the opacity slider keeps the HUD readable while you trace.  Scale and
   position are saved with the layout, so the reference lands in the same
   place next time.
3. **Tiles.**  Use *Add* under *Tilesets* to open a PNG/JPG and enter its tile
   size.  Pick a cell in the palette, choose **Stamp**, then click or drag on
   the canvas.  Right-click erases the tile under the pointer.  One sheet per
   image; tiles from several sheets can be mixed in one layout.  A stamp lands
   at the grid cell's top-left, so keep the sheet tile size equal to the grid
   cell when building an exact tile map.
4. **Parts.**  *Add* creates a guide rectangle.  Choose a kind:
   `text`, `rect`, `box`, `pips`, `grid`, `image` or `note`.  Drag it in
   **Select** mode (corner handles resize it) or type exact pixels in *Part
   fields*.  Parts are named after the engine element they describe; that is
   the name the summary prints.
5. **Save.**  `Ctrl+S` writes the `.hudlayout`.  *File > Copy summary* puts the
   report on the clipboard, and *File > Export PNG* writes the tile art on a
   transparent background.

`Ctrl+Z`/`Ctrl+Y` undo and redo tile strokes and part edits.  `Delete`
removes the selected part.  `Ctrl+mouse-wheel` changes the canvas zoom.

**File > Insert classic HUD guide** adds the rectangles the engine currently
uses for the classic frame, status column, inventory and hint strip.  The same
data is in `docs/examples/classic-hud.hudlayout`.  It is a useful starting
point for a new panel image or for checking a reference screenshot.

## The file format

UTF-8 text, INI-like `[section]` / `key = value` lines.  `#` and `;` start a
comment line.  Repeated `[run]` sections are deliberate.  Coordinates are
pixels from the top-left of the frame; `size` is `WxH`, `cell` and `cells` are
`WxH` pairs, and `grid_origin` is `X,Y`.

The current engine layout is at
`docs/examples/classic-hud.hudlayout`.  A minimal file is:

```ini
[layout]
format = jillex-hud-layout
version = 1
name = my-hud
style = classic
canvas = 320x200
grid = 8x8
grid_origin = 0,0
snap = 1
show_grid = 1
zoom = 2
reference = shots/classic.png
reference_opacity = 55
reference_scale = 100
reference_pos = 0,0
reference_visible = 1

[sheet.frame]
file = art/frame.png
tile = 8x8

[run]
sheet = frame
x = 8
y = 22
row = 0
cols = 0,1,2,3,4,5,6,7

[part.health-label]
kind = text
x = 10
y = 27
size = 36x8
colour = 7
text = HEALTH

[part.health-bar]
kind = rect
x = 48
y = 27
size = 20x7
colour = 4
```

### `[layout]`

| Key | Meaning |
| --- | --- |
| `format`, `version` | `jillex-hud-layout`, `1` |
| `name` | display name |
| `style` | `classic`, `modern` or `custom` (informational) |
| `canvas` | frame size, normally `320x200` |
| `grid`, `grid_origin` | snap cell size and its origin |
| `snap`, `show_grid` | editor toggles (`1`/`0`) |
| `zoom` | canvas zoom to restore |
| `reference` | reference image; stored relative to the layout when possible |
| `reference_opacity` | 0..100 |
| `reference_scale` | percent of the source image |
| `reference_pos` | `X,Y` in frame pixels |
| `reference_visible` | `1`/`0` |

### `[sheet.<id>]`

`file` is the tileset image and `tile` is the size of one cell.  The id is the
name used by `[run]` sections; it is sanitised to letters, digits, `_` and `-`.

### `[run]`

One horizontal run of identical-sheet tiles.  `x`,`y` is the first tile's
top-left pixel, `row` is the tile row in the sheet, and `cols` is the list of
source columns in left-to-right order.  Runs are emitted for contiguous x
positions; a repeated fill tile is written as e.g. `cols = 2,2,2`.  This is
the form to read when checking one row of a frame or a filled bar.

### `[part.<name>]`

| Key | Used by | Meaning |
| --- | --- | --- |
| `kind` | all | `text`, `rect`, `box`, `pips`, `grid`, `image`, `note` |
| `x`, `y` | all | top-left pixel |
| `size` | all | reported width/height in pixels |
| `colour` | text/rect/box/pips | EGA palette index 0..15 |
| `text` | text | label text |
| `cell` | grid/pips | one cell's `WxH` |
| `cells` | grid/pips | `COLSxROWS`; `pips` uses the column count |
| `file` | image | placed image |
| `note` | any | free text shown in the editor and the summary |

`size` is written for human readability: `text` derives its width from its
contents when the stored size is zero, and `grid`/`pips` derive their bounds
from `cell` × `cells`.  The summary always prints the effective bounds.

## Reading a layout in code-alignment work

`--summary` (or *Copy summary*) is the intended hand-off format.  It prints
each part with both pixel and grid coordinates, using `col N+R` / `row N+R`
for a position that is `R` pixels past the grid line.  For example:

```text
    health                 text  x=10   y=27   w=36   h=8    (col 1+2, row 3+3)  "HEALTH"
    bar                    rect  x=48   y=27   w=20   h=7    (col 6, row 3+3)  colour 4
```

means the health label starts at pixel `(10,27)`, which is two pixels right of
grid column 1 and three pixels below grid row 3 (the grid is 8x8 from 0,0).
Tile runs print the same way, plus the source row and the covered x/y span.

The summary also lists the reference scale/position and any missing image
warnings, so a layout can be checked without opening the editor.

## Limits

* The editor is a layout aid, not a paint program.  Tile stamps are on the
  grid at the moment they are placed; moving the grid afterwards does not
  reposition them, and tiles cannot be dragged individually (erase and stamp
  again).
* One reference image per layout, and any number of tileset sheets (one image
  each).  Use several sheets for mixed art.
* Part guides and the reference are editor-only.  PNG export contains tiles
  and `image` parts, not the reference or guide rectangles.
* Part names must be unique; they become section names.
* The classic HUD has fixed engine coordinates.  This tool records and checks
  those coordinates; changing them still requires an engine edit, and the
  defaults in `port/src/engine.cpp` must stay unchanged unless you intend to
  change the classic layout.
