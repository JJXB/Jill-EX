# HUD layout tool

`tools/hud_layout.py` is a standalone editor for HUD alignment.  It overlays a
reference screenshot under the game's 320x200 frame, lets you lay tiles on a
resizeable grid (from a PNG sheet or straight from the game's `.dma` + `.sha`
tiles), and saves a small text layout (`.hudlayout`) that can be read back
when the engine drawing needs to line up with an original screen or a
hand-drawn panel.

The editor itself does not modify the engine.  Its format is an authoring
aid: the port can load a saved layout as an in-game classic HUD (see
[Using a layout in the game](#using-a-layout-in-the-game)), and it keeps the
normal external HUD panels (`hud/classic.png`, `hud/modern.png`, ...) as a
separate option.

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

**File > Insert classic pack panel guide** adds the other classic path: the
elements the engine draws over an external `hud/classic.png`.  A pack panel
replaces the boxes, so the engine only draws the `HEALTH` label at (2,2), the
health pips at y=2 (entry 14 frames 42/43, last pip offset by the cap), `LEVEL`
and `SCORE` labels at y=10, their values at y=16, and the 3x3 inventory icons
from (1,26) on a 14-px step.  The example is
`docs/examples/classic-pack-hud.hudlayout`.

The old `pack-pips` part in the main guide was a rough 2x6 stand-in for that
external-panel pip row; it is now `pack-health-pips` in the pack guide, with
the real 4x6 pip frames and the 3-px advance noted.

## DMA tilesets (original game tiles)

The *DMA...* button under *Tilesets* loads the original tile art instead of a
prepared PNG.  Jill's `.dma` file is only a **tile-id to shape-id table**; the
pixels live in the episode's `.sha` shape bank, so the dialog asks for both:

| Field | Example |
| --- | --- |
| DMA tile table | `jillt\jill.dma` (shared by Jill 1-3) |
| Shape bank | `jillt\jill1.sha`, `jill2.sha` or `jill3.sha` |
| Palette | blank for Jill 1-3; `profiles\onesimus.pal` for Onesimus |
| Sheet columns | how many tiles per row (default 16) |
| Cell size | `16x16` (the board tile size) |

Browsing the DMA file guesses the matching SHA beside it (`jill.dma` to
`jill1.sha`), but you can pick another episode.  Tiles are laid out in **DMA
record order** (the order in the file, not sorted by id), one per cell.  A
small yellow number in each palette cell is the **tile id**; the selected tile
is described as `TILE 45 SHORE 16x16`.  The *Find DMA tile id* box jumps to a
tile by its id.  *Match grid to cell* (on by default) sets the layout grid to
the tile cell size so stamps land on exact 16x16 boundaries; turn it off to
keep a grid you already set.

Most Jill tiles are 16x16.  A handful (16x8 platforms, 16x20/16x22 decorative
tiles) are drawn clipped into a 16x16 cell by default; use *Size* to rebuild
the sheet at e.g. `16x22` if you need to see all of a tall tile.  That also
changes the stamping cell height, so 16x22 is an inspection mode rather than
the normal board alignment.

Loading a layout rebuilds DMA sheets from the stored `.dma`/`.sha` paths.  If
those files are missing, the layout still opens but the sheet is empty and a
warning is printed.  Uncompressed `.sha`/`.dma` paths are stored relative to
the layout file when possible, so keep the game data folder beside your
layouts if you want them to travel.  The editor imports the decoders from
`jill_extractor.py`, which must sit beside `hud_layout.py` or in its parent
folder (the release zip already keeps the two together).

When a DMA sheet is used, each `[run]` also records the **tile ids**:

```ini
[run]
sheet = jill
x = 16
y = 16
row = 0
cols = 11,11
tiles = 45,45
```

`cols` is still the palette cell; `tiles` is what a board's tile map or the
engine would see, and the summary prints both ids and names.

## HUD/UI shape entries

HUD and menu art is **not** in the `.dma` table.  It is made of frames inside
shape entries, which is why a DMA-only import shows board tiles but no panel
pieces.  Use *Tilesets > Shape...* to load those frames:

| Entry | Art |
| --- | --- |
| `1` | 8x8 text font (128 glyphs) |
| `2` | 6x8 text font (128 glyphs) |
| `3` | classic HUD/frame patterns: 8-px edges, corners, 16x12 strips, fill |
| `4` | 4x5 score digits |
| `7` | nine-slice menu window |
| `14` | item icons, health pips and the inventory art |

The dialog defaults to entries `3,4,7,14` (the usual HUD/UI set); change the
comma-separated list to load others.  Each entry becomes its own sheet,
labelled `entry:frame` in the palette (e.g. `3:10` is the 16x16 frame fill,
`14:42` is a health pip).  *Find tile/frame* accepts either `42` (frame in the
active entry) or `14:42` (entry and frame).  Cell `16x16` suits HUD pieces;
`auto` uses the largest frame in that entry.

Frames are top-left aligned, matching the engine blitter, and the sheet uses
one cell size per entry.  A piece smaller than the cell keeps transparent
padding on the right/bottom, so stamping at the cell's top-left puts the art
exactly where the engine draws it.  Oversized frames are clipped unless you
set a larger cell or use `auto`.

Like DMA sheets, shape sheets are rebuilt from the `.sha` path when the layout
is opened.  Each `[run]` records the frame numbers:

```ini
[run]
sheet = sha3
x = 8
y = 22
row = 0
cols = 10,0
frames = 10,0
```

`cols` addresses the generated palette sheet; `frames` is the shape frame the
engine would draw.  The summary prints both.

## Using a layout in the game

Save the layout and copy the `.hudlayout` into `external\autoload\` (or
`external\autoload\hud\`, or a pack's `hud\` folder).  Restart the port so the
external layer is scanned, then choose it in **OPTIONS > HUD > LAYOUT**
(switch **STYLE** to CLASSIC first).  The selection is stored as
`[hud] layout = <file stem>` and can be started with `--hud-layout <stem>`.
Names are matched case-insensitively.

The port draws the layout's frame tiles with the **running game's** shape
bank; the `sha` paths in the file are only used by the editor, so the layout is
portable between Jill 1/2/3 as long as the entry/frame ids match the art you
want.  Image-backed sheets and `image` parts are not drawn in-game; use SHA
shape sheets for HUDs.

These part names are driven by live game values (other parts are drawn as
authored):

| Part | Meaning |
| --- | --- |
| `health-bar` (rect) | current health as a framed bar |
| `level-value` (text) | current level, `MAP` for 0, blank for `0x7F` |
| `score-value` (text) | current score, right-aligned to the part's x+w |
| `inventory-grid` (grid) | the carried item icons |
| `title` (text) | the game title; also suppresses the default title |
| `hint-strip` (rect) | black strip behind the bottom hint |
| `pips` | health pips using the game's icon bank |
| `box` | a classic 1-px framed HUD box |
| `frame`, `view` (rects) | guides only |

The camera still uses the classic viewport 80,16 232x160, so keep your `view`
part at those coordinates.  A selected in-game layout takes priority over a
`hud/classic.png` image panel.

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

`tile` is the size of one cell.  A plain image sheet uses:

```ini
[sheet.frame]
file = art/frame.png
tile = 8x8
```

A DMA sheet stores the source files instead; the editor rebuilds it on load:

```ini
[sheet.jill]
source = dma
dma = ../jillt/jill.dma
sha = ../jillt/jill1.sha
palette = ../profiles/onesimus.pal   # omit for the built-in Jill palette
tile = 16x16
columns = 16
```

A HUD/UI shape sheet stores one entry of the same bank:

```ini
[sheet.sha14]
source = sha
sha = ../jillt/jill1.sha
palette = ../profiles/onesimus.pal   # optional
entry = 14
tile = 16x16
columns = 16
```

The id is the name used by `[run]` sections; it is sanitised to letters,
digits, `_` and `-`.  `columns` fixes the palette layout, so the cell to
tile-id/frame mapping stays stable between sessions.

### `[run]`

One horizontal run of identical-sheet tiles.  `x`,`y` is the first tile's
top-left pixel, `row` is the tile row in the sheet, and `cols` is the list of
source columns in left-to-right order.  Runs are emitted for contiguous x
positions; a repeated fill tile is written as e.g. `cols = 2,2,2`.  This is
the form to read when checking one row of a frame or a filled bar.

For a DMA sheet an extra `tiles` line lists the original tile ids:

```ini
[run]
sheet = jill
x = 8
y = 24
row = 0
cols = 0,1,2
tiles = 45,46,47
```

`cols` addresses the generated palette sheet; `tiles` is the id a board map or
the engine uses.  The summary prints both, plus the tile names.

For a shape sheet the extra line is `frames` (shape entry frames):

```ini
[run]
sheet = sha3
x = 8
y = 22
row = 0
cols = 10,0
frames = 10,0
```

The sheet's `[sheet]` section names the entry, so `frames` is enough to redraw
the same piece; the summary prints `frames` next to the pixel/grid position.

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
from `cell` x `cells`.  The summary always prints the effective bounds.

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
Tile runs print the same way, plus the source row, the covered x/y span and the
DMA ids when the sheet has them:

```text
    jill     row 0   x 16..48  y 16..32  cols 11..12  (col 2, row 2)  tiles 45,46
      names: SHORE, BASESHORE
```

The summary also lists the reference scale/position and any missing image
warnings, so a layout can be checked without opening the editor.

## Limits

* The editor is a layout aid, not a paint program.  Tile stamps are on the
  grid at the moment they are placed; moving the grid afterwards does not
  reposition them, and tiles cannot be dragged individually (erase and stamp
  again).
* One reference image per layout, and any number of tileset sheets (one image
  each, or one DMA/SHA pair each).  Use several sheets for mixed art.
* A DMA sheet needs the episode's `.sha`; the `.dma` alone is only a lookup
  table and contains board tiles, not HUD/UI panel pieces.  Sheet cells follow
  DMA record order, so tile ids are not sequential across a row.  Oversized
  tiles (16x8, 16x20, 16x22) are clipped to the cell unless you rebuild the
  sheet with a taller cell size.
* HUD/UI art comes from shape entries (3, 4, 7, 14, fonts 1/2).  Shape sheets
  store one entry each and use a fixed cell size; a frame larger than the cell
  is clipped unless you use `auto` or a larger cell.
* Part guides and the reference are editor-only.  PNG export contains tiles
  and `image` parts, not the reference or guide rectangles.
* Part names must be unique; they become section names.
* The classic HUD has fixed engine coordinates.  This tool records and checks
  those coordinates; changing them still requires an engine edit, and the
  defaults in `port/src/engine.cpp` must stay unchanged unless you intend to
  change the classic layout.
* DMA decoding comes from `jill_extractor.py`; keep it beside
  `hud_layout.py` (or in its parent folder).  Loading a layout with DMA sheets
  needs the referenced `.dma`/`.sha` files, otherwise the sheet stays empty.
