# External Asset Packs

This document explains how to create, name and package asset packs for the
Jill of the Jungle port.  A pack can replace artwork (sprites, tiles, level
backgrounds, menu themes, fonts, HUD panels, text-page art) and audio
(music and sound effects) without touching the original game files.

A working example pack is included in `external/ExamplePack/`.

---

## 1. Where packs live

Packs are searched for in:

```
<base folder>\external
```

`<base folder>` is the directory that holds `jill.ini`.  At start-up the port
switches to that directory, so launching `port\build\Release\jill.exe` from
anywhere still finds the same `external` folder.

Inside `external`, each immediate child is one pack:

* a sub-folder, e.g. `external\MyPack\`
* a zip file, e.g. `external\MyPack.zip`

The folder name or zip file stem is the pack id.  Ids are compared without
regard to case, so `MyPack`, `mypack` and `MYPACK.zip` all mean the same pack.
Only one pack is mounted at a time.

The port creates `external\` automatically if it is missing.

### Autoload

`external\autoload\` is a special folder that is **always active**.  It is not
listed as a pack and needs no id.  Drop any of the folders described below
(`sprites\`, `hud\`, `themes\`, `borders\`, `fonts\`, `sfx\`, ...) straight
into `external\autoload\` and they are applied at start-up, whether or not a
pack is selected.

When both provide the same relative file, the selected pack wins; otherwise
the two layers are combined.  This is useful for a personal HUD, font, menu
border or a handful of sounds without making a full pack.

---

## 2. Quick start

1. Copy `external\ExamplePack` to `external\MyPack`.
2. Edit `external\MyPack\pack.ini` and set `name` (and optionally `game`).
3. Replace the sample files with your own art/audio, keeping the folder
   names and file names described below.
4. Launch the game:
   * in the front end: `OPTIONS > ASSETS`, then select `MyPack`
   * or directly: `jill.exe --pack MyPack`
5. Check the console output.  A successful mount prints:

   ```
   external: mounted MyPack
   ```

Use `--list-packs` to see what the port discovered:

```
jill.exe --list-packs
```

---

## 3. Pack layout

```
external\MyPack\
  pack.ini            optional manifest (recommended)
  sprites\            kind strips and single shape frames
  tiles\              per-tile images
  boards\             whole-level backgrounds
  themes\             extra front-end menu backdrops
  borders\            extra shape-7 nine-slice menu borders
  fonts\              text font sets / single glyphs
  hud\                HUD panels (default and named variants)
  pages\              text-page backgrounds
  portrait.png        optional dialog portrait (64x84)
  sfx\                sampled sound effects
  music\              music tracks
```

Only the folders you need have to exist.  Everything is optional.

Both `\` and `/` are accepted inside zip files and paths in `pack.ini`.
File names are matched case-insensitively.

---

## 4. Supported formats

### Images

| Extension | Notes |
| --- | --- |
| `.png` | recommended; supports alpha |
| `.jpg`, `.jpeg` | no alpha |
| `.gif` | first frame only |
| `.apng` | first frame only |

Animated GIF/APNG files are not played as animation.  For animated game art,
use a horizontal strip (see sprite kinds below).

### Audio

| Extension | Notes |
| --- | --- |
| `.ogg` | Vorbis |
| `.mp3` | MPEG audio |
| `.m4a` | AAC; Windows only (Media Foundation) |
| `.wav` | PCM or float |

Music loops until the game requests another track.  Effects play once.

### Images and the game palette

The game renders in 8-bit VGA mode.  External images are converted to the
running game's palette with a nearest-colour match, so exact RGB values are
not preserved.  Images with more colours than the palette will band or dither
visually; this is expected.

Transparency rules:

* **Sprite/kind/tile art** with an alpha channel: pixels with
  alpha < 128 become palette index 0 (transparent).
* **Sprite/kind/tile art without alpha**: the top-left pixel's colour is
  treated as the transparent key.
* **Board and menu-theme art**: an opaque image is drawn as-is; only real
  alpha makes pixels transparent.  It is never colour-keyed on its top-left
  pixel, so full-screen pictures keep every colour.
* **HUD and text-page overlays**: only real alpha is transparent; an opaque
  image is drawn fully opaque.

---

## 5. Asset naming reference

Paths below are relative to the pack root.  `<ext>` is any supported image or
audio extension.

### 5.1 Sprites: `sprites\`

| File | Replaces |
| --- | --- |
| `kind_<n>.<ext>` | every frame of kind `n` |
| `shape_<entry>_<frame>.<ext>` | one shape-bank entry/frame |

`kind_<n>` is a horizontal strip.  The number of cells is the original
shape's frame count; cell `i` occupies columns
`[i * width / frames, (i + 1) * width / frames)`.  Each cell is rescaled to
the original frame size.  If the original shape has one frame, the whole image
is frame 0.

Example: a 4-frame creature is one image `128 x 32`, split into four `32 x 32`
cells.

`shape_<entry>_<frame>` overrides a single shape directly.  `entry` is the
`.sha` bank entry (0..63) and `frame` is its frame index.  This is the escape
hatch for art that is not tied to a kind.

### 5.2 Tiles: `tiles\`

| File | Replaces |
| --- | --- |
| `tiles\<id>.<ext>` | tile id 0..599 from `jill.dma` |

Animated tiles are matched against their original (base) shape, so an
override keeps working when the tile script rewrites the live tile record.

Tile images are rescaled to the tile's original size (normally 16x16).

### 5.3 Level backgrounds: `boards\`

| File | Replaces |
| --- | --- |
| `boards\<board>.<ext>` | the backdrop of one board |

`<board>` is the board name without the episode extension: `3` for `3.jn1`,
`MAP` for `MAP.ONE`, and so on.  Names are case-insensitive.

The image is scaled to 320x200 and drawn before the tiles.  With a background
present, tile colour 0 is treated as transparent so the picture shows through
open areas.  Opaque areas of tiles still cover the background.

### 5.4 Menu themes: `themes\`

Every image under `themes\` becomes an extra choice in
`OPTIONS > MENUS > THEME`, listed after the installed games.  Images are
scaled to 320x200.  The menu dims them with the usual 50% checkerboard.

### 5.5 Fonts: `fonts\`

| File | Replaces |
| --- | --- |
| `fonts\main.<ext>` | the default 6 px text font (shape entry 2) |
| `fonts\main_8.<ext>` | the default 8 px font (shape entry 1) |
| `fonts\<name>.<ext>` | a selectable 6 px font set |
| `fonts\<name>_8.<ext>` | the 8 px counterpart of that set |
| `fonts\glyph_<code>.<ext>` | one character code (all fonts) |

Each sheet is a horizontal strip of up to 128 glyph cells, in ASCII order
(0..127).  Cell `i` is width/128 of the image (or width/frame-count if the
original font has fewer frames).  Each cell is rescaled to the original glyph
size.

`main` is applied automatically and keeps the old behaviour.  Any other name
appears as a selectable font in `OPTIONS > MENUS > FONT`; selecting a set
replaces both text banks, falling back to the game's own glyph when a bank is
not supplied.  The `_8` file is optional.

The glyph image supplies only the shape; the game draws it in the requested
text colour.  A white-on-transparent sheet is the easiest starting point.
Anti-aliased edges become either opaque or transparent at alpha 128, so a
hard-edged bitmap font gives the cleanest result.

`glyph_<code>` uses a decimal character code, e.g. `glyph_65.png` for `A`.

### 5.6 HUD panels: `hud\`

| File | Replaces | Size |
| --- | --- | --- |
| `hud\classic.<ext>` | default CLASSIC top-left panel | 64x84 |
| `hud\modern.<ext>` | default MODERN top bar | 320x12 |
| `hud\classic_<name>.<ext>` | extra CLASSIC panel | 64x84 |
| `hud\modern_<name>.<ext>` | extra MODERN bar | 320x12 |

Images are scaled to those sizes.  Labels, inventory icons, health pips and
the score are still drawn on top.  Use PNG alpha if you want the game visible
through parts of the panel.  `hud\classic.<ext>` keeps the legacy 64x84
top-left placement; without it the engine draws the full classic frame
(dithered border, title strip, health bar, level/score and inventory).

Named panels appear in `OPTIONS > HUD > PANEL`, filtered to the current
CLASSIC/MODERN style.  An unprefixed `hud\<name>.<ext>` is also accepted and
classified by its aspect ratio (wider than 96 px and at most 20 px tall is a
MODERN bar, otherwise CLASSIC).  The manifest form is `classic_<name> = file`
or `modern_<name> = file`.

### 5.7 Menu borders: `borders\`

| File | Replaces |
| --- | --- |
| `borders\<name>.<ext>` | one 3x3 nine-slice border sheet |
| `borders\<name>\<frame>.<ext>` | individual shape-7 frames (1..9) |

A single image is split into a 3x3 grid: top-left/top-middle/top-right,
middle-left/centre/middle-right, bottom-left/bottom-middle/bottom-right.
Each cell is scaled to 8x8.  The centre cell is the checkerboard fill; the
other eight are the border edges and corners.

Named borders appear after the installed games in `OPTIONS > MENUS > BORDER`.
The manifest form is `[borders] <name> = file`.

### 5.8 Text-page art: `pages\`

| File | Replaces |
| --- | --- |
| `pages\page_<n>.<ext>` or `pages\<n>.<ext>` | backdrop of text page `n` |

`n` is 0..39.  The image is scaled to 320x200.  The page title, body text and
footer are still drawn over it.  When page art is present the default box
frame is not drawn.

Formatting inside the page text (colours, line breaks, scrolling) still comes
from the original `.vcl` data.

### 5.9 Dialog portrait: `portrait.<ext>`

| File | Replaces |
| --- | --- |
| `portrait.<ext>` or `portraits/portrait.<ext>` | the 64x84 portrait beside level-title dialogs |

The portrait is drawn next to boxes such as `JILL BOUNDS THROUGH THE
BOULDERS`, at the same screen position the in-game inventory panel uses.
The default is composed from the player's idle sprite (2x inside a frame);
a pack image replaces it.  Manifest form: `[portrait] file = art/jill.png`.
Turn it off with `OPTIONS > HUD > PORTRAIT` or `[hud] portrait = 0`.

### 5.10 Sound effects: `sfx\`

| File | Replaces |
| --- | --- |
| `sfx\<slot>.<ext>` | raw sample slot 0..49 |
| `sfx\<slot>_<label>.<ext>` | same, with a descriptive label |

Effects are addressed by the original logical-id table, so replacing raw slot
`1` is heard wherever that sample is used.  The SOUND TEST screen in
`OPTIONS > SOUND` auditions raw slots and is handy for finding the right one.

### 5.11 Music: `music\`

| File | Replaces |
| --- | --- |
| `music\<track stem>.<ext>` | the matching title/level track |

The stem is the original track name without its extension.  For example,
`funky.ddt` is replaced by `music\funky.ogg`.  Level music requested by a
`*name.ddt` CHECKPT is looked up the same way.

---

## 6. The manifest: `pack.ini`

`pack.ini` is optional but recommended.  It has two jobs:

1. Give the pack a display name and record its target game.
2. Map a non-standard file name onto a game asset.

Syntax: INI sections, `key = value`, `#` or `;` starts a comment line.
Section and key names are case-insensitive.

```ini
[pack]
name = My Enhanced Pack
game = jill1

[sprites]
kind_4 = art/frog_strip.png
shape_8_16 = art/jill_torso.png

[tiles]
12 = art/grass.png

[boards]
3 = art/hut.png

[themes]
; not needed: every image under themes/ is already a menu theme

[fonts]
main = art/font.png
main_8 = art/font_8.png
pixel = art/font_pixel.png
65 = art/a.png

[hud]
classic = art/panel_classic.png
modern = art/panel_modern.png
classic_gold = art/panel_gold.png
modern_slim = art/bar_slim.png

[borders]
vine = art/border_vine.png

[pages]
1 = art/how_to_play.png

[portrait]
file = art/jill_portrait.png

[sfx]
1 = audio/throw.ogg

[music]
funky = audio/title_theme.mp3
```

Notes:

* `[pack] game` is informational; it records the game the pack was authored
  for but does not block loading it with another game.
* Explicit manifest entries win over auto-discovered files.
* Values are paths relative to the pack root; `\` and `/` both work.
* Unknown sections and keys are ignored.
* The manifest itself can live inside a zip pack and is read from the
  archive.

---

## 7. Packaging as a zip

A zip pack must contain the pack files at the archive root.  Do **not** wrap
everything in an extra top-level folder.

Correct:

```
MyPack.zip
  pack.ini
  sprites/kind_1.png
  tiles/1.png
```

Wrong:

```
MyPack.zip
  MyPack/
    pack.ini
```

PowerShell example:

```powershell
Compress-Archive -Path .\external\MyPack\* `
                 -DestinationPath .\external\MyPack.zip -Force
```

7-Zip example:

```bat
7z a external\MyPack.zip .\external\MyPack\*
```

Remove or rename the source folder if you want the zip to be the only pack
with that id, otherwise both are listed.

---

## 8. Testing a pack

### List discovered packs

```
jill.exe --list-packs
```

Prints one line per folder/zip with its id and path.

### Mount from the command line

```
jill.exe --pack MyPack
```

### Render a test frame without playing

```
jill.exe --game jill1 --board 3 --ticks 1 --pack MyPack --dump shot.ppm
```

`--ticks` runs the headless path, which still applies pack/autoload art and
writes the final frame as a binary PPM.  This is the quickest way to check
sprite, tile, board, HUD, font and text-page overrides.

Named HUD panels and font sets can be selected without opening the menus:

```
jill.exe --game jill1 --board 3 --ticks 1 --hud --hud-panel gold --dump hud.ppm
jill.exe --game jill1 --board 3 --ticks 1 --menu-font pixel --dump font.ppm
```

Use a specific text page:

```
jill.exe --game jill1 --board 3 --ticks 1 --pack MyPack --text-page 1 --dump page.ppm
```

Classic HUD:

```
jill.exe --game jill1 --board 3 --ticks 1 --pack MyPack --hud --dump hud.ppm
```

### Audio check

```
jill.exe --game jill1 --pack MyPack --audio-check
```

This opens the audio device briefly and reports the music title and levels.
If pack music replaced the CMF track, the title reads `external`.

### In-game

`OPTIONS > ASSETS` lists all packs plus the original art.  Selecting a row
mounts/unmounts immediately; `RESCAN FOLDER` re-reads `external\`.

---

## 9. Troubleshooting

**The pack is not listed**

* Is it directly inside `external\`, not `external\sub\MyPack`?
* Is the extension really `.zip`?
* Run `--list-packs`; there may be a path error printed to stderr.

**The pack mounts but nothing changes**

* Check the id/key matches the game's data.  Kind numbers, tile ids and board
  names differ between episodes.
* Shapes are replaced by shape id, not by object: a kind override only affects
  objects that use that kind's tileset.
* Board names are without extensions (`3`, not `3.jn1`).
* Check the console: decode failures print `external: <file>: <reason>`.
* Some art (menus, text layout, splash screens) is not data-driven and is not
  replaceable yet.

**A font looks clipped or distorted**

* Each source cell is rescaled to the original glyph's size; keep glyphs
  centred in their cells and leave a 1 px margin.
* Use hard edges (no anti-aliasing) and white pixels on transparency.

**A HUD panel covers too much**

* The classic panel is exactly 64x84 and the modern bar 320x12; anything
  outside those rectangles is not drawn.
* Use alpha for cut-outs.

**Zip pack fails to mount**

* The archive must be a normal deflate/store zip.
* Keep `pack.ini` and the folders at the archive root.

---

## 10. Included example pack

`external/ExamplePack/` demonstrates every folder:

| File | Shows |
| --- | --- |
| `pack.ini` | manifest with display name |
| `sprites/kind_1.png` | 4-cell horizontal kind strip |
| `tiles/1.png` | single 16x16 tile |
| `boards/3.png` | 320x200 board backdrop |
| `themes/menu.png` | extra menu theme |
| `fonts/main.png` | 128-cell bitmap font sheet |
| `hud/classic.png` | 64x84 classic panel |
| `hud/modern.png` | 320x12 modern bar |
| `pages/page_1.png` | 320x200 text-page art |
| `sfx/1.wav` | short beep in raw slot 1 |
| `music/funky.wav` | short title-theme loop |

Try it with:

```
jill.exe --game jill1 --board 3 --ticks 1 --pack ExamplePack --dump example.ppm
```

or mount it in `OPTIONS > ASSETS`.  The art is intentionally simple so the
effect of each override is easy to spot.

## 11. Generating a reference pack

`jill_extractor.py` can write a ready-to-edit pack from an installed game:

* GUI: `Tools > Export Reference Asset Pack...`
* CLI: `python extract_all.py out --game jill1 --pack`

It exports all shape frames, tiles, 320x200 board backdrops, the font strip,
HUD panels/icons, the dialog portrait, text-page templates and previews, the
intro menu theme, `palette.pal`, SFX WAVs, rendered `music/<track>.wav` files
and a `pack.ini`.  Copy the generated `reference_pack` folder into
`external/` and rename it to your pack id.
