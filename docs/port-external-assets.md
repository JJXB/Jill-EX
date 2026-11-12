# External enhanced assets [verified]

See `EXTASSETS.md` for the pack-author guide and the example pack in
`external/ExamplePack/`.  `jill_extractor.py` / `extract_all.py --pack` can
generate an editable reference pack from any installed game.

The port can replace artwork and audio at runtime from a user-supplied
"pack".  Packs live under `<base folder>\external`, i.e. the directory that
holds `jill.ini` (the port changes to it at start-up).  A pack is either a
sub-folder or a `.zip` file.  One pack is mounted at a time.  In addition,
`external/autoload/` is always active under the selected pack and is never
listed as a pack; a pack file of the same relative name wins.

* `OPTIONS > ASSETS` lists the packs and mounts/unmounts them.
* `--pack ID` mounts a pack at start-up; `[external] pack` in `jill.ini`
  does the same for normal launches.
* `--external DIR` changes the search root; `[external] dir` in `jill.ini`
  sets it permanently.
* `--list-packs` prints the discovered packs and exits.

The search folder is created automatically when it is missing.  Packs are
matched by id without regard to case, so `TestPack`, `testpack` and
`TESTPACK.zip` are the same id [verified].

## Supported formats

* Images: PNG, JPG/JPEG, GIF, APNG [verified].
  `stb_image` decodes the default (first) frame of an animated GIF/APNG.
  Animated game art is supplied as a horizontal strip instead, see below.
* Audio: OGG/Vorbis, MP3, M4A/AAC, WAV [verified].
  M4A/AAC uses Windows Media Foundation, so it is Windows-only.
* A zip pack is mounted in memory with miniz; no unpacking is required
  [verified].

## Pack layout

Files are found by relative path inside the pack.  Both `\` and `/` work.

```
external/
  MyPack/                 (or MyPack.zip)
    pack.ini              optional manifest
    sprites/              kind sheets and single shape overrides
    tiles/                per-tile images
    boards/               whole-level backgrounds
    themes/               extra front-end menu backdrops
    borders/              named nine-slice menu borders
    fonts/                selectable font sets / glyphs
    hud/                  default and named HUD panels
    pages/                text-page backgrounds
    portrait.png          optional dialog portrait (64x84)
    sfx/                  per-slot sampled effects
    music/                replacement music tracks
```

### sprites/

* `kind_<n>.<ext>` replaces every frame of kind `n` (0-based, the profile's
  kind table).  The image is a horizontal strip: frame `i` occupies columns
  `[i*w/frames, (i+1)*w/frames)`.  A single-frame image is used for frame 0
  only.  Each cell is rescaled to that frame's original size [verified].
* `shape_<entry>_<frame>.<ext>` replaces one shape-bank entry/frame directly.
  `entry` is the `.sha` entry (`tileset & 0x3F`), `frame` the frame index.

Note: internal and player shapes are keyed by the same entry/frame ids, so a
kind override also applies wherever that shape id is used [inferred].

### tiles/

`tiles/<id>.<ext>` replaces tile id `id` (0..599) from the game's `.dma`
table.  Animated tiles are matched by their original (base) shape, so the
override survives the tile script rewriting the live record [verified].

### boards/

`boards/<board>.<ext>` replaces the whole-level backdrop.  `<board>` is the
board name without the `.jn1`/`.jn2`/`.jn3`/`.ONE` extension (`3`, `MAP`,
...).  The image is scaled to 320x200 and drawn before the tiles; with a
background present, tile colour 0 is treated as transparent so the picture
shows through open areas [verified].  Opaque pack art therefore covers the
tile layer; transparent PNGs let it show.

### themes/

Every image under `themes/` becomes an extra `MENUS > THEME` choice, after
the installed games.  Images are scaled to 320x200.  The menu dims them with
the same 50% checkerboard as the built-in intro-board themes [verified].

### sfx/

* `sfx/<slot>.<ext>` replaces the raw sample slot 0..49 (the same slots the
  NOISEMAKER auditions).
* `<slot>_<label>.<ext>` is also accepted.
* Logical effects go through the original 50-byte remap table, so a replaced
  raw slot is heard wherever that sample is used [verified].

### fonts/

* `fonts/main.<ext>` replaces the default 6 px text font (shape entry 2);
  `fonts/main_8.<ext>` the 8 px font.  Each image is a horizontal strip of up
  to 128 glyph cells (ASCII 0..127), each scaled to the original glyph size.
  Glyph pixels are used as a mask and drawn in the requested text colour
  [verified].
* `fonts/<name>.<ext>` / `fonts/<name>_8.<ext>` add a selectable font set,
  listed in `OPTIONS > MENUS > FONT`.  A missing bank falls back to the game's
  own glyphs.
* `fonts/glyph_<code>.<ext>` replaces one character code [verified].

### hud/

* `hud/classic.<ext>` replaces the default 64x84 CLASSIC panel.  It keeps the
  old top-left placement; the engine's full-frame classic HUD is used when no
  panel is present.
* `hud/modern.<ext>` replaces the default 320x12 MODERN bar.
* `hud/classic_<name>.<ext>` / `hud/modern_<name>.<ext>` add named panels,
  selectable in `OPTIONS > HUD > PANEL`.  An unprefixed `hud/<name>.<ext>` is
  accepted and classified by aspect (wide and short = MODERN).
* The image is scaled to the panel size.  Labels, inventory icons, health
  pips and score are still drawn on top; transparent pixels let the game
  show through [verified].

### borders/

* `borders/<name>.<ext>` is a 3x3 nine-slice sheet, scaled to 8x8 cells.  The
  centre cell fills the window; the other eight are corners/edges.
* `borders/<name>/<1..9>.<ext>` supplies individual shape-7 frames.
* Named borders appear after the installed games in
  `OPTIONS > MENUS > BORDER`.

### pages/

* `pages/page_<n>.<ext>` or `pages/<n>.<ext>` replaces the background art of
  text page `n` (0..39).  The image is scaled to 320x200 and the page title,
  body text and footer are still drawn over it [verified].

### portrait.<ext>

Replaces the 64x84 character portrait drawn beside level-title/dialog boxes
(`JILL BOUNDS THROUGH THE BOULDERS`).  Auto-discovered as
`portrait.<ext>` or `portraits/portrait.<ext>`, or set with
`[portrait] file = ...`.  Without a pack the engine composes a default from
the player's idle sprite [verified].  The portrait can be hidden with
`OPTIONS > HUD > PORTRAIT` or `[hud] portrait = 0` in `jill.ini`.

### music/

`music/<name>.<ext>` replaces the track whose file stem is `<name>`.  Both
the title track (`funky.ddt` -> `music/funky.ogg`) and level music requested
by the board's `*name.ddt` / `*name.ark` music CHECKPTs use this lookup.  The
per-board start tracks are catalogued in `docs/music-catalog.md`.  The track
loops until the game requests another one [verified].

## pack.ini

All sections are optional.  Values are paths relative to the pack root.

```ini
[pack]
name = My Enhanced Pack
game = jill1

[sprites]
kind_4 = art/frog.png
shape_8_16 = art/jill.png

[tiles]
12 = art/grass.png

[boards]
3 = art/hut.png

[sfx]
1 = audio/throw.ogg

[music]
funky = audio/title.mp3

[fonts]
main = art/font.png
main_8 = art/font_8.png
pixel = art/font_pixel.png
65 = art/a.png

[hud]
classic = art/panel_classic.png
modern = art/panel_modern.png
classic_gold = art/panel_gold.png

[borders]
vine = art/border_vine.png

[pages]
1 = art/help_page.png
```

`[pack] name` is the display name in `OPTIONS > ASSETS`.  `[pack] game` is
informational: it records which game the pack was authored for, and packs
are not blocked when it differs.

Explicit manifest entries win over auto-discovered files [verified].
Images are quantised to the running game's VGA palette with a nearest-colour
match; sprite/kind/tile transparency (alpha < 128, or the top-left colour on
opaque images) becomes palette index 0.  Board and menu-theme pictures are
never colour-keyed: they stay opaque unless they carry real alpha [verified].

## Limitations

* Animated GIF/APNG files decode their first frame only; use a horizontal
  strip for sprite animation [verified].
* Music replacement is looked up by track stem.  A CMF rendered directly by
  `--dump-music` is not affected [verified].
* M4A/AAC decoding requires Windows Media Foundation [inferred].
* Font art supplies glyph shapes only; the text renderer still uses the
  original 6 px advance and per-glyph sizes [verified].
* Text-page art replaces the backdrop, not the text layout [verified].
