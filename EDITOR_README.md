# Jill Editor [verified]

`jill_editor.py` is a Tkinter GUI for editing the same archives handled by
`jill_extractor.py` (read/export) and `jill_import.py` (write/import).  It works
on the files you point it at; the game itself is not required.

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
overwritten unless you explicitly choose the same path.  Bad input is reported
as a short error dialog/status message, not a traceback.  Set
`JILL_EDITOR_DEBUG=1` to print a traceback for unexpected errors.

## Tabs

### Sprites

* Pick one of the 128 shape entries, step through its frames and export the
  current frame as a PNG.
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
* **Rebuild from Folder** accepts `shape_NNN/frame_NNN.png` trees and the
  extractor's flat `sprites/shape_NNN_FFF.png` layout (`sha-folder`) when you
  want to update several entries at once.
* **Font from Sheet** replaces shape entry 2 from `fonts/main.png`
  (`sha-font`), preserving the foreground index.

### Tiles

The DMA list shows each tile id, its name and the shape entry/frame holding its
art.  Export a tile as PNG or replace its art from a PNG (`sha-tile`).  Only the
`.sha` is written; the `.dma` is read-only.

A shape entry's 16-colour palette is shared by every frame in that entry, so
recolouring it changes other tiles in the original game too.  The editor
therefore never changes the tile palette: tick **Dither to existing palette**
to spread the new art across the colours that are already there (Floyd-Steinberg
dithering) instead of snapping each pixel to the nearest one.  This is safe in
the original game but cannot create genuinely new hues.  The `tile-clone` CLI
command can give a tile its own palette by cloning the entry into a free shape
slot and repointing the DMA record (writes both `.sha` and `.dma`), but the
original game still rejects that layout, so it is not exposed in the GUI.
**Use in Level Editor** sets the paint tile on the Levels tab.

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

## Workflows

**Change one sprite**

1. Select the game (or open its `.sha`), pick the entry/frame on Sprites.
2. Export the PNG, edit it in an image editor, then Replace from PNG.
3. Put the saved `.sha` beside the game data or mount it as a pack.

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

* No DMA write API: tile ids/names/attributes cannot be changed here.
* Sprite import quantises to the existing palette; use the extractor's palette
  assets when preparing art from scratch.
* The level editor draws tiles only; object placement is numeric (no dragging
  objects on the canvas).
* Text import is VCL pages only; there is no WAV-to-CMF/DDT music converter.
* This edits files on disk; keep backups of archives you replace.

See `IMPORT_README.md` for the equivalent CLI commands and
`EXTRACTOR_README.md` for exporting reference packs.
