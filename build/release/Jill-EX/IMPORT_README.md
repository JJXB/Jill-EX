# Importer guide [verified]

`jill_import.py` converts pictures and audio back into Jill/Onesimus data,
and installs extractor output as an external asset pack.  It is a command-line
tool; the extractor's **Convert** menu still covers the four original
converters (shape frame, shape folder, VCL slot, VCL folder).  For a GUI that
edits sprites, tiles, levels, text and SFX, see `EDITOR_README.md`.

## Requirements

* Python 3 with Pillow (`py -3 -m pip install -r requirements.txt`).
* A reference `.sha`, `.dma` or `.vcl` from the game you are editing.
* The built port is **not** required for importing.

## Quick start

```
py -3 jill_import.py sha-frame  jill1.sha 8 16 frame.png -o jill1_new.sha
py -3 jill_import.py sha-folder jill1.sha sprites -o jill1_new.sha
py -3 jill_import.py sha-entry-folder jill1.sha 8 frames -o jill1_new.sha
py -3 jill_import.py sha-font   jill1.sha fonts/main.png -o jill1_new.sha
py -3 jill_import.py sha-tile   jill1.sha jill.dma 0x9E tiles/158.png -o jill1_new.sha
py -3 jill_import.py tile-safe  jill1.sha jill.dma 0x9E tiles/158.png --out-sha jill1_new.sha --out-dma jill_new.dma
py -3 jill_import.py palette-export jill1.sha 8 entry8.pal
py -3 jill_import.py palette-import jill1.sha 8 entry8.pal -o jill1_new.sha
py -3 jill_import.py portrait   jill1.sha portrait.png -o jill1_new.sha
py -3 jill_import.py board-image 1.jn1 jill1.sha jill.dma board_previews/1.png -o 1_new.jn1
py -3 jill_import.py vcl-sfx    jill1.vcl 6 jump.wav -o jill1_new.vcl
py -3 jill_import.py vcl-pages  jill1.vcl text_pages -o jill1_new.vcl
py -3 jill_import.py pack       extracted external/MyPack --game jill1 --name "My Pack"
```

Without `-o`, the input file is overwritten.  Keep a backup or always write a
new output while testing.

Every input is validated before an output is written.  Missing files, bad
archives/images/audio, malformed metadata, out-of-range indexes and data that
would overflow a Jill archive all produce a short `error: ...` message and a
non-zero exit code (1), not a traceback.  Set `JILL_IMPORT_DEBUG=1` to get the
full traceback.  `sha-folder`/`vcl-folder` still allow a *missing* reference
path to build a fresh archive; an existing path must be a readable archive.

## Commands

### `sha-frame <sha> <entry> <frame> <image> [-o out.sha] [--adapt-palette]`

Replaces one frame of an existing shape entry.  The image is resized to the
old frame size with nearest-neighbour sampling, quantised to the entry's
palette, and stored as opcode 0.  Transparent pixels become index 0.

`--dither` uses Floyd-Steinberg dithering against the existing palette instead
of nearest-colour mapping.  `--adapt-palette` builds a new per-entry palette from the image instead of
snapping to the old colours.  New colours are mapped to the nearest entry in
the game's global VGA palette, and **every frame of that entry changes** (all
tiles sharing a shape entry are recoloured).  It only applies to entries with
fewer than 8 bits per pixel.  For tiles, use `tile-clone` below instead: it
gives the tile its own palette without touching the other frames.

### `sha-folder <sha> <folder> [-o out.sha]`

Rebuilds entries from a PNG folder.  Two layouts are accepted:

```
shape_000/frame_000.png
shape_000/trailing.png          (optional)

sprites/shape_000_000.png       (the extractor's flat output)
```

Existing entries keep their source bpp, flags and palette.  Entries not in the
folder are copied from the reference `.sha`.  The font entry (`2`) is skipped
in the flat layout because its exported glyphs are white masks; use
`sha-font` instead.

### `sha-entry-folder <sha> <entry> <frames> [-o out.sha] [--adapt-palette]`

Replaces **all frames of one shape entry** with a folder of images
(`frame_0.png`, `frame_1.png`, ...; PNG/BMP/GIF/JPEG/TGA).  Files are sorted
naturally by name, all scaled to the first image's size, and quantised to the
entry palette.  The entry's bpp, flags, palette and trailing frame are kept,
and other entries are copied from the reference `.sha`.  This is the CLI used
by the editor's **Replace Animation from Folder** button.

### `sha-font <sha> <fonts/main.png> [-o out.sha] [--entry 2] [--fg 1]`

Replaces the font entry from the extractor's 128-cell strip.  `font.json` in
the same directory supplies the cell and glyph sizes.  Opaque pixels become
`--fg` (default index 1); transparent pixels become 0.  Indices 0 and 3 are
the original font's transparent entries, so do not use 3 as the foreground.

### `sha-tile <sha> <dma> <tile-id> <image> [-o out.sha] [--dither]`

Looks up the DMA record for `tile-id` (decimal or `0x` hex), finds the shape
entry/frame it references, and replaces that frame.  The image is resized to
the old tile frame (normally 16x16) and quantised to the entry palette.  Only
the `.sha` is written; the `.dma` is unchanged.  `--dither` approximates new
colours with the existing palette (safe in the original game); the tile palette
itself is never changed.

### `tile-clone <sha> <dma> <tile-id> <image> [--out-sha out.sha] [--out-dma out.dma]`

Like `sha-tile`, but for art with colours the tile's palette does not have.
A shape entry's palette is shared by all its frames, so recolouring it in place
changes every other tile using that entry.  This command copies the entry into
a free shape slot, adapts the palette there and repoints the tile's DMA record
at the copy.  **Both `.sha` and `.dma` are written**; install both beside the
game data.  DMA shape references only allow entries 0..63, so the command fails
if no free slot remains there.  Note: the original DOS game still renders the
cloned layout incorrectly; use this only for ports/packs that support it.

### `tile-safe <sha> <dma> <tile-id> <image> [--out-sha out.sha] [--out-dma out.dma]`

Original-EXE-safe recoloured tile import.  The image (normally 16x16) is
quantised to the fixed **global VGA palette** and appended as a new frame to an
existing 8bpp identity-mapped shape entry below 64 (entry 24 in the Jill games,
which is the portrait entry).  Only that tile's DMA record is repointed at the
new frame, so **no palette table changes and no free shape entry is needed**;
the original DOS EXE keeps its normal palette layout.

Both `.sha` and `.dma` are written (defaults: the input paths, which are
overwritten).  The command prints the entry/frame used.  This is the path the
editor's **Allow new colours (append 8bpp frame, original-safe)** option uses.
The one visible side effect is that the portrait entry gains extra frames; its
palette and the first 16 frames are untouched.

### `palette-export <sha> <entry> <out.pal|.gpl|.act>`

Writes one shape entry's colour table in an image-editor format chosen by the
output extension: JASC-PAL (`.pal`), GIMP (`.gpl`) or Adobe colour table
(`.act`).  Colours are written in palette-slot order (4, 8 or 16 entries for a
small table).  Slot 0 is the transparent/key colour and is restored unchanged by
`palette-import`, so edit the visible colours from slot 1 onward.

### `palette-import <sha> <entry> <palette> [-o out.sha]`

Reads `.pal`, `.gpl` or `.act` into the entry's colour table.  Each colour is
snapped to the nearest index in the game's global VGA palette and the palette
word's low 16 bits are preserved; slot 0 (transparent/key) is left exactly as it
was.  **Every frame/tile that shares the entry
changes colour** because the table is per-entry; the command warns about this
in its output.  Only entries that carry a per-entry table can be imported (fewer
than 8 bpp and not identity-mapped); use `tile-safe` when a tile needs genuinely
new colours without touching other tiles.

### `portrait <sha> <portrait.png> [-o out.sha]`

Replaces Jill's dialogue portrait from a single image instead of sixteen tile
entries.  The image is resized to 64x70 and split across shape entry 24's
sixteen 16x16 tiles (the bottom row is 16x22/16x20).  The extractor's
`portrait.png` can be edited and fed straight back.  Games without an entry-24
portrait (Onesimus) fail with a short error.

### `board-image <board> <sha> <dma> <render.png> [-o out.jn1]`

Rebuilds a board's tile map from a **full 2048x1024** render, such as
`board_previews/<name>.png`.  The 320x200 `boards/<name>.png` files are not
enough (they are viewport crops) and are rejected.

Each 16x16 block is matched against the DMA tile images.  Exact matches are
preferred; near matches use a 4x4 colour signature.  Uniform background
blocks and poor matches keep the original cell.  Duplicate tile art may map to
an equivalent tile id, so the file can differ even when the rendered board is
pixel-identical.

The board's objects, header and strings are preserved.

### `vcl-sfx <vcl> <slot> <audio.wav> [-o out.vcl]`

Replaces one of the 50 sampled-sound slots 0..49.  Audio is mixed to mono,
resampled to the slot's existing rate (normally 6000 Hz), and stored as
unsigned 8-bit PCM.

### `vcl-folder <vcl> <folder> [-o out.vcl]`

Reads `sfx_NN.wav` for the 50 sound slots and `music_NN.raw` for the 40
second-bank raw entries.  The historical `music_` name is kept for
compatibility; in the shipped banks that bank holds the text pages.
Unspecified slots are copied from the reference `.vcl`.

### `vcl-pages <vcl> <text_pages> [-o out.vcl]`

Rebuilds the text-page bank from the extractor output:

* `pages.json` is preferred; it preserves each line's colour code.
* Otherwise `page_NN.txt` files are read with the default colour 7.

Lines are written back as colour code + text + `0x0D`.  Pages not present in
the folder are preserved.

### `pack <extractor-out> <external/pack> [--game ID] [--name NAME]`

Copies a reference/extractor tree into an external pack folder:

```
sprites/  fonts/  tiles/  boards/  themes/  hud/  pages/  sfx/  music/
portrait.png  palette.pal  palette_6bit.bin  palette.json
```

It checks common sizes (portrait 64x84, HUD panels 64x84 / 320x12, boards and
pages 320x200, tiles 16x16) and prints warnings for mismatches.  If the pack
has no `pack.ini`, one is written with `[pack] name` and an optional `game`.

Pack music is copied, not converted: the port plays `music/<stem>.wav`,
`.mp3`, `.ogg`, etc. directly.  Use the extractor's rendered WAVs or your own
audio file.

## Recommended workflows

**Edit one sprite**

1. Extract the reference pack.
2. Edit `sprites/shape_008_016.png`.
3. `sha-folder jill1.sha <pack> -o jill1_new.sha`.
4. Put `jill1_new.sha` beside the game data (or repack it with your mod).

**Edit a tile**

1. Extract `tiles/`.
2. Edit `tiles/158.png` (tile 0x9E).
3. `sha-tile jill1.sha jill.dma 0x9E tiles/158.png -o jill1_new.sha`.

If the edited art uses colours the tile's shared palette does not have, use the
safe importer instead and install both outputs:

```
py -3 jill_import.py tile-safe jill1.sha jill.dma 0x9E tiles/158.png \
    --out-sha jill1_new.sha --out-dma jill_new.dma
```

**Edit the portrait**

1. Extract the pack (or use the editor's **Export Portrait PNG**) to get a
   64x70 `portrait.png`.
2. Edit it, then `portrait jill1.sha portrait.png -o jill1_new.sha`.

**Edit a palette**

1. `palette-export jill1.sha 8 entry8.pal`.
2. Edit the palette in an image editor and save as `.pal`, `.gpl` or `.act`.
3. `palette-import jill1.sha 8 entry8.pal -o jill1_new.sha` (all frames of the
   entry change colour).

**Edit a board**

1. Extract `board_previews/`.
2. Edit the full 2048x1024 PNG.
3. `board-image 1.jn1 jill1.sha jill.dma board_previews/1.png -o 1_new.jn1`.

**Build a pack from an extractor export**

```
py -3 extract_all.py extracted --game jill1 --all
py -3 jill_import.py pack extracted external/MyPack --game jill1 --name "My Pack"
```

Then mount it with `--pack MyPack` or **OPTIONS > ASSETS**.

## Limits and notes

* Images are quantised to the game palette (or the target entry's palette
  when replacing a frame).  Colours outside it are approximated.
* Alpha below 128 becomes the transparent index (0).
* `sha-font` flattens the font to one foreground index; multi-level font
  shading is not preserved.
* `board-image` needs the full board render, not a 320x200 preview.
* Duplicate tile art can change tile ids while keeping the rendered board
  identical.
* Music import is pack-only audio; there is no WAV-to-CMF/DDT converter.
* Importing does not modify the original game files unless you overwrite them.

## Troubleshooting

| Problem | Check |
| --- | --- |
| `shape entry N is empty` | Pick an entry that exists in the reference `.sha` |
| `tile ... not in ...dma` | Use a tile id from `tiles.json` or the DMA list |
| `board image must be the full 2048x1024 render` | Use `board_previews/`, not `boards/` |
| Colours look wrong | The image may use colours outside the entry palette; check `palette.pal` |
| `pack` prints size warnings | Fix the named file to the expected dimensions |
| Imported font is blank | Use `sha-font` (not the flat sprite dump) and `--fg 1` |
| `error: ... entry N points outside the file` | The reference archive is truncated or not a Jill archive |
| `error: ... not a readable image` | The PNG/WAV is corrupt or in an unsupported format |

`py -3 test_jill_import.py` runs self-contained import checks using synthetic
archives (no game files required).

See also `COMMANDLINE.md` for the extractor and port options, and
`EXTRACTOR_README.md` for the viewer and export workflows.
