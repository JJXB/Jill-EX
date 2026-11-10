# Importer guide [verified]

`jill_import.py` converts pictures and audio back into Jill/Onesimus data,
and installs extractor output as an external asset pack.  It is a command-line
tool; the extractor's **Convert** menu still covers the four original
converters (shape frame, shape folder, VCL slot, VCL folder).

## Requirements

* Python 3 with Pillow (`py -3 -m pip install -r requirements.txt`).
* A reference `.sha`, `.dma` or `.vcl` from the game you are editing.
* The built port is **not** required for importing.

## Quick start

```
py -3 jill_import.py sha-frame  jill1.sha 8 16 frame.png -o jill1_new.sha
py -3 jill_import.py sha-folder jill1.sha sprites -o jill1_new.sha
py -3 jill_import.py sha-font   jill1.sha fonts/main.png -o jill1_new.sha
py -3 jill_import.py sha-tile   jill1.sha jill.dma 0x9E tiles/158.png -o jill1_new.sha
py -3 jill_import.py board-image 1.jn1 jill1.sha jill.dma board_previews/1.png -o 1_new.jn1
py -3 jill_import.py vcl-sfx    jill1.vcl 6 jump.wav -o jill1_new.vcl
py -3 jill_import.py vcl-pages  jill1.vcl text_pages -o jill1_new.vcl
py -3 jill_import.py pack       extracted external/MyPack --game jill1 --name "My Pack"
```

Without `-o`, the input file is overwritten.  Keep a backup or always write a
new output while testing.

## Commands

### `sha-frame <sha> <entry> <frame> <image> [-o out.sha]`

Replaces one frame of an existing shape entry.  The image is resized to the
old frame size with nearest-neighbour sampling, quantised to the entry's
palette, and stored as opcode 0.  Transparent pixels become index 0.

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

### `sha-font <sha> <fonts/main.png> [-o out.sha] [--entry 2] [--fg 1]`

Replaces the font entry from the extractor's 128-cell strip.  `font.json` in
the same directory supplies the cell and glyph sizes.  Opaque pixels become
`--fg` (default index 1); transparent pixels become 0.  Indices 0 and 3 are
the original font's transparent entries, so do not use 3 as the foreground.

### `sha-tile <sha> <dma> <tile-id> <image> [-o out.sha]`

Looks up the DMA record for `tile-id` (decimal or `0x` hex), finds the shape
entry/frame it references, and replaces that frame.  The image is resized to
the old tile frame (normally 16x16) and quantised to the entry palette.  Only
the `.sha` is written; the `.dma` is unchanged.

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

See also `COMMANDLINE.md` for the extractor and port options, and
`EXTRACTOR_README.md` for the viewer and export workflows.
