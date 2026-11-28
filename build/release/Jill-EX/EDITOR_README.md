# Jill Editor [verified]

`jill_editor.py` is a Tkinter GUI for editing the same archives handled by
`jill_extractor.py` (read/export) and `jill_import.py` (write/import).  It works
on the files you point it at; the game itself is not required.

The port also has an in-engine editor: `LEVEL EDITOR` on the front-end main menu
opens a mouse/keyboard/gamepad tile/object/header editor with an `F5` playtest
toggle.  Its board selector has a `NEW LEVEL` row; `Esc`/pad `B` opens an editor
menu with Save, Save As, Load, Save to Game Data, Paint Tool, Undo/Redo,
Copy/Paste, Object Props, Place Presets, Playtest, Info, Help and Exit.
The Save As / Load pickers choose among the original board and saved edits under
`editor/<game>/`, and the menu can write the board into the game data folder
after confirmation.  The editor also has undo/redo, object copy/paste,
single/line/rectangle/flood tile tools, tile-region copy/paste, gem-door/key-door
placeable presets, trigger-channel helpers, object property/string editing and a
scrollable keyboard/controller/touch control reference.  Vita and Android also
show an on-screen editor toolbar.  See the "Level editor (JILL MAKER)" section
of `HOTKEYS.md` for the controls, `docs/editor-doors.md` for a worked guide
to key/gem doors, trigger channels and the P13 link helper, and
`docs/editor-object-properties.md` for the per-kind field reference.

## Launch

```
py -3 jill_editor.py
py -3 jill_editor.py D:\Emu\DOS\C\jillt
py -3 jill_editor.py --check
```

Requirements are the same as the extractor: Python 3 with Pillow and Tkinter
(`py -3 -m pip install -r requirements.txt`; on Linux, `python3-tk`).
The optional `base_dir` argument is the folder containing `jill.ini` and
`games/*.game`; it defaults to the script's folder.  Games are discovered with
the same code as the extractor.  **File > Open Files...** lets you load a
`.sha`, `.dma` and `.vcl` directly instead of picking a game.

Every tab writes a **new file** via a Save dialog; the originals are never
overwritten unless you explicitly choose the same path.  Before any operation
writes over an existing file, the editor copies it to
`<name>.<YYYYmmdd-HHMMSS>.bak` in the same folder, so overwriting in place still
leaves a timestamped backup of every file about to change (`.sha`, `.dma`,
`.vcl`, board).  Bad input is reported as a short error dialog/status message,
not a traceback.  Set `JILL_EDITOR_DEBUG=1` to print a traceback for unexpected
errors.

## Tabs

### Sprites

* Pick one of the 128 shape entries, step through its frames and export the
  current frame as a PNG.
* **Grid view** replaces the single frame with a clickable contact sheet of
  every frame in the entry (the best column count is chosen automatically).
  Click a cell to select that frame; untick it to return to the single-frame /
  animation view.  **Play** animation is unchanged and automatically leaves
  grid mode.
* **Replace from PNG** imports a PNG into that frame (same logic as
  `jill_import.py sha-frame`): nearest-neighbour resize to the old frame size,
  quantised to the entry palette, transparent pixels become index 0.
* **Adapt palette (recolours whole entry)** is off by default.  Normally the
  PNG is quantised to the entry's *existing* 16/256 colours, so new hues become
  the nearest old ones (e.g. purple may become grey/brown).  Tick this to build
  a new per-entry palette from the image instead.  The new colours are snapped
  to the nearest colours in the game's global VGA palette, and **all frames of
  that entry change**, since the palette is shared.  Only entries with fewer
  than 8 bits per pixel can be adapted.
* **Replace Animation from Folder** replaces *all frames of the selected
  entry* with a folder of images (`frame_0.png`, `frame_1.png`, ...).  Files are
  sorted naturally by name, scaled to the first image's size and quantised to
  the entry palette; bpp, flags, palette and the trailing frame are preserved.
  PNG/BMP/GIF/JPEG/TGA inputs are accepted.
* **Export Palette** / **Import Palette** read and write the selected entry's
  colour table as JASC `.pal`, GIMP `.gpl` or Adobe `.act`.  Importing snaps
  each colour to the nearest global VGA index, preserves the palette words' low
  16 bits and leaves slot 0 (transparent/key) unchanged; it **recolours every
  frame of that entry** (the table is shared) and asks for confirmation first.
  Entries without a per-entry table (8bpp identity-mapped) cannot be imported.
* **Portrait (entry 24 as one image)**: **Export Portrait PNG** writes Jill's
  dialog face as a single 64x70 image; **Import Portrait PNG** reads one image
  (resized to 64x70) and splits it across the sixteen 16x16 portrait tiles.
* **Rebuild from Folder** accepts `shape_NNN/frame_NNN.png` trees and the
  extractor's flat `sprites/shape_NNN_FFF.png` layout (`sha-folder`) when you
  want to update several entries at once.
* **Font from Sheet** replaces shape entry 2 from `fonts/main.png`
  (`sha-font`), preserving the foreground index.

### Tiles

The DMA list shows each tile id, its name and the shape entry/frame holding its
art.  **Grid view** shows a paged 10x6 contact sheet of the DMA tiles; click a
cell to select it, and use **<** / **>** to change page.  Untick it for the
single-tile preview.  Export a tile as PNG or replace its art from a PNG.

A shape entry's palette is shared by every frame in that entry, so recolouring
it in place changes every tile that uses the entry.  Import has two modes:

* **Dither to existing palette** (default off) spreads new colours across the
  colours already present with Floyd-Steinberg dithering instead of snapping
  each pixel.  Safe, but cannot create genuinely new hues.
* **Allow new colours (append 8bpp frame, original-safe)** is on by default.
  It quantises the image to the fixed global VGA palette and appends it as a
  new frame of an existing 8bpp identity-mapped shape entry below 64 (entry 24
  in the Jill games), then repoints only that tile's DMA record.  No palette
  table is written or changed and no free shape slot is needed, so the original
  DOS EXE keeps its normal palette layout.  Both `.sha` and `.dma` are written;
  save and install both.  Untick it to use the old in-place `sha-tile` path
  (`.sha` only, palette unchanged).

**Export Tile Palette** / **Import Tile Palette** work on the selected tile's
shape entry in `.pal`, `.gpl` or `.act` form; importing recolours every tile
that shares the entry and asks for confirmation.  The `tile-clone` CLI command
still exists for ports/packs that support cloned entries, but the original game
rejects that layout, so it is not exposed in the GUI.  **Use in Level Editor**
sets the paint tile on the Levels tab.

### Levels

* Open a board from the discovered game folder, paint tile ids onto the
  128x64 tile map by clicking/dragging, scroll around with the scrollbars and
  toggle the grid.  Painting preserves only the tile id (not tile flags).
  **Zoom - / Zoom +** cycles the canvas between 1x, 2x and 4x.
* Canvas mouse actions: plain left click/drag paints the selected tile id;
  **Shift + left click** removes the placed object under the cursor;
  **Ctrl + left click** paints the **Erase tile** value (set it to the level's
  background tile id) into the map; **right click** opens a properties menu
  for the object under the cursor (edit properties, select objects sharing its
  trigger/param, delete).
* **Kinds** draws a coloured box and short name on every object (colour keyed
  by kind id); **Links** draws dashed yellow lines between objects that share
  the same non-zero `primary_param` (trigger channel, e.g. DOOR + PAD +
  CHECKPT).  Both overlays are toggles and do not change board data.
* **Import Render** rebuilds the tile map from a full 2048x1024 board render
  (`board-image`); 320x200 previews are rejected.  **Export Render** writes the
  full composite PNG via the extractor's renderer.
* **Objects**: select, edit, add and delete object records (kind, x/y, size,
  state, param, flags, TTL, string pointer fields, KS2).  `kind_specific_1` and
  `attached_string` are preserved but not exposed.
* **Strings**: edit/add/delete the board's strings.  Strings are written in
  list order; point an object at one with its String off / Segment fields.
* **Header**: edit level id, health, keys, score, damage flash, score copy and
  the 16 key ids.  Board objects, header and strings are saved with
  `BoardFile.save()` alongside the painted tile map.

Unsaved board changes are marked with `*` in the board status and prompt on
board switch/game change.  Xargon boards are loaded with `XargonBoardFile` when
a Xargon game is selected.

### Text

The VCL text-page bank is shown as 40 pages (`*` marks pages with data).  Each
page is a list of lines with a colour code 0-7 and text; select a line to edit
it and press Apply, or use **Add Line** (also bound to Enter in the text box)
to append a new line to the current page even while another line is selected.
**Save .vcl** writes the pages with
colour codes back into a `.vcl` (`vcl_set_pages`), **Import pages folder**
reads `text_pages/pages.json` or `page_NN.txt` (`vcl-pages`), and **Export page
TXT** writes a plain-text copy.  The preview uses the game font when a `.sha`
is loaded.  Note that the extractor's parser trims trailing blank lines, so
re-saving an untouched page may drop them.

### SFX

Lists all 50 sampled-sound slots with size and rate.  Play a slot (Windows
`winsound`), export one or all slots as WAV files (`VCLFile.create_wav`), import
a WAV into the selected slot (`vcl-sfx`: mono, unsigned 8-bit, resampled to the
slot rate, normally 6000 Hz), import a folder of `sfx_NN.wav` (`vcl-folder`) or
save the resulting `.vcl`.

## Asset packs

**Tools > Export Asset Pack...** opens a checklist of the external-pack
elements (sprites, fonts, text pages, tiles, board previews, menu theme, HUD,
portrait, global palette, page art, SFX and music).  **Export All** ticks or
clears everything, and the music section also has a rendered-length field
(30 seconds by default) because rendering is slow.  The destination defaults to
`<data folder>/reference_pack`; any folder can be chosen.  The result is the
same tree the extractor writes and the port mounts, so it can be edited and
packaged with `jill_import.py pack` or dropped into `external/`.

**Tools > Export Reference Pack (all)...** skips the checklist and exports
every section.  **Tools > Export Palettes for Image Editor...** writes every
shape entry's palette to a folder (`entry_NNN.pal`) plus the global palette, for
batch palette work in an image editor.

## Workflows

**Change one sprite**

1. Select the game (or open its `.sha`), pick the entry/frame on Sprites.
2. Export the PNG, edit it in an image editor, then Replace from PNG.
3. Put the saved `.sha` beside the game data or mount it as a pack.

**Change one tile (new colours)**

1. On Tiles, pick the tile and **Export PNG**.
2. Edit it and **Replace from PNG** with **Allow new colours** ticked.
3. Save both the `.sha` and `.dma` the editor asks for and install both beside
   the game data.  The appends only a new frame to the 8bpp portrait entry and
   repoints this one DMA record.

**Change the portrait**

1. On Sprites, **Export Portrait PNG**, edit the 64x70 image, then
   **Import Portrait PNG** and save the new `.sha`.

**Change a palette**

1. On Sprites or Tiles, **Export Palette**, edit it in an image editor and
   **Import Palette**.  Remember that the colour table is shared by every
   frame/tile of the entry.  **Tools > Export Palettes** writes them all at
   once.

**Change a level**

1. Open the board on Levels, choose a tile on Tiles and click **Use in Level
   Editor**, paint, then edit objects/strings/header if needed.
2. Save As a `.jn1` and copy it over the original in the game data folder.

**Change dialogue**

1. On Text, select a page, edit lines/colours and Save .vcl.
2. Copy the saved `.vcl` beside the game data.

**Replace sounds**

1. On SFX, pick a slot, Import WAV into Slot and Save .vcl As.
2. Copy the saved `.vcl` beside the game data.

## Limits

* Tile ids/names/attributes still cannot be edited; the safe tile importer only
  rewrites the selected record's shape reference.
* Sprite import quantises to the existing palette unless **Adapt palette** is
  ticked; palette import writes the entry table, not the global VGA palette.
* The level editor draws tiles only; object placement is numeric (no dragging
  objects on the canvas).
* Text import is VCL pages only; there is no WAV-to-CMF/DDT music converter.
* This edits files on disk.  Automatic timestamped `.bak` copies are made
  before overwriting existing files, but keep your own archive copies too.

See `IMPORT_README.md` for the equivalent CLI commands and
`EXTRACTOR_README.md` for exporting reference packs.
