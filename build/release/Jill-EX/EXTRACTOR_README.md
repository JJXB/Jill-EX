# Jill of the Jungle - Game File Viewer & Extractor

A comprehensive Python GUI utility for viewing and extracting game assets from the Jill of the Jungle trilogy (Episodes 1, 2, and 3).

## Features

### Supported File Formats

| Format | Extension | Description |
|--------|-----------|-------------|
| **Board Files** | `.jn1`, `.jn2`, `.jn3`, `.dem` | Level maps with tile data, objects, headers, and strings |
| **Shape Archives** | `.sha` | Sprite/shape data with multiple frames and video modes |
| **Tile Definitions** | `jill.dma` | Maps tile IDs to shape IDs and attributes |
| **Sound Banks** | `.vcl` | Sampled sound effects, text pages, and level-help copy |
| **Music Files** | `.ddt`, `.ARK` | Creative Music Format (CMF/CTMF) for AdLib/OPL2; Onesimus uses `SONG*.ARK` |
| **Config Files** | `.cfg` | High scores, save slots, and game options |
| **Input Macros** | `.mac` | Recorded input demos/macros |

### Episode Support

| Game | Title | Board Ext | Shape File | Sound File | Config | Title Music |
|------|-------|-----------|------------|------------|--------|-------------|
| `jill1` | Jill of the Jungle | `.jn1` | `jill1.sha` | `jill1.vcl` | `jill1.cfg` | `funky.ddt` |
| `jill2` | Jill Goes Underground | `.jn2` | `jill2.sha` | `jill2.vcl` | `jill2.cfg` | `peaks.ddt` |
| `jill3` | Jill Saves The Prince | `.jn3` | `jill3.sha` | `jill3.vcl` | `jill3.cfg` | `wild.ddt` |
| `onesimus` | Onesimus: A Quest for Freedom | `.ONE` | `ONESIMUS.SHA` | `ONESIMUS.VCL` | — | — |

Games are discovered from the port's `jill.ini` + `games/*.game` manifests,
so mods/other data sets work without code changes.

### GUI Features

- **Board Viewer**: Renders real tiles (via `.dma` → `.sha` mapping) at 128×64 with
  zoom, grid, and object overlays
- **Shape Viewer**: Decoded sprites with checkerboard transparency, synchronized
  frame spinner, VGA/EGA/CGA modes, and **animation playback** (adjustable FPS).
  The frame number and animation position are linked: editing the frame jumps the
  animation, and playback updates the frame number / `n/N` status live.
- **Sound Browser**: List, extract, save as WAV, and **play sounds in-app** (Windows)
- **Hex Viewer**: Raw hex/ASCII view of any file
- **Batch Extraction**: Export all boards, shapes, or sounds at once, plus
  enhanced pack assets: fonts, text pages, tile images, HUD panels, menu
  themes, palettes and full board previews
- **Reference Pack Export**: Write an editable `external/` asset pack tree
  (shape frames, tiles, boards, fonts, HUD, pages, SFX WAVs, palette)
- **JSON Export**: Structured export of board data for analysis

### Converters (PNG/WAV → Jill)

Two-way asset editing via the **Convert** menu or the `jill_import.py` CLI:

| Action | GUI | CLI |
|--------|-----|-----|
| Replace one shape frame with a PNG | Convert → Import Image into Shape Frame | `jill_import.py sha-frame <sha> <entry> <frame> <img.png> -o out.sha` |
| Build a `.sha` from a `shape_NNN/frame_NNN.png` tree | Convert → Build .sha from PNG Folder | `jill_import.py sha-folder <sha> <folder> -o out.sha` |
| Replace one VCL sound slot with a WAV | Convert → Import WAV into Sound Slot | `jill_import.py vcl-sfx <vcl> <slot> <audio.wav> -o out.vcl` |
| Build a `.vcl` from `sfx_NN.wav` / `music_NN.raw` | Convert → Build .vcl from WAV Folder | `jill_import.py vcl-folder <vcl> <folder> -o out.vcl` |

Imported images are quantized to the game palette (and to the target entry's
palette when replacing a frame). WAV audio is mixed to mono, resampled to the
slot's rate (normally 6000 Hz), and stored as unsigned 8-bit PCM. Other entries
in the archive are preserved. Exporting shapes via **Export All Frames** writes
the exact folder layout the `.sha` builder expects.

## Enhanced asset exporters

The **Tools** menu and `extract_all.py` can export everything the external
asset-pack system understands, not just the original archive formats:

| Output | Files | Use |
|--------|-------|-----|
| Fonts | `fonts/main.png` (128-cell strip) + `fonts/glyphs/glyph_NNN.png` + `fonts/font.json` | Edit the in-game text font |
| Text pages | `text_pages/page_NN.txt`, `text_pages/pages.json`, `text_pages/preview/page_NN.png` | Read/redraw F1 help and menu pages |
| Tile images | `tiles/<tile id>.png` + `tiles/tiles.json` | Tile art with attributes and names |
| HUD | `hud/classic.png`, `hud/modern.png`, previews and `hud/icons/` | Editable HUD panels and item icons |
| Portrait | `portrait.png` | 64x84 dialog portrait (default is the player idle sprite) |
| Theme | `themes/intro.png` | The intro-board menu backdrop (320x200) |
| Music | `music/<track>.wav` | OPL2-rendered `.ddt` tracks (uses the built port) |
| Palette | `palette.pal`, `palette_6bit.bin`, `palette.json` | Exact game DAC palette |
| Board previews | `boards/<name>.png` (320x200) + `board_previews/<name>.png` (2048x1024) | Reference art / pack backdrops (tile layer) |
| Reference pack | all of the above in pack layout + `pack.ini` + `sfx/<slot>.wav` + `music/<track>.wav` | Starting point for a custom pack |

`Export Reference Asset Pack` writes a complete tree that can be copied to
`external/<name>` and mounted with `--pack <name>` or `OPTIONS > ASSETS`.
Board previews are large and are never written by `--all`; request them
explicitly.

CLI equivalents:

```bash
python extract_all.py out --game jill1 --fonts --pages --tiles --hud --palette
python extract_all.py out --game jill1 --previews          # 2048x1024 boards
python extract_all.py out --game jill1 --music-wavs        # render .ddt -> WAV
python extract_all.py out --game jill1 --pack              # reference pack
```

Music rendering uses the built port executable
(`port/build/Release/jill.exe --dump-music`), which contains the OPL2/CMF
synthesiser.  If the port has not been built, raw `.ddt` files are still
copied by `--music`/`--all`, but no WAV is produced.

In the Sound tab, selecting a `Title music: *.ddt` or `Music: *.ddt` row now
uses **Play Selected** and **Save as WAV** through the same port renderer.

## Requirements

- Python 3.8+
- Pillow (PIL) for image processing

```bash
pip install -r requirements.txt
```

## Usage

```bash
python jill_extractor.py            # GUI
python extract_all.py --list        # list discovered games
python extract_all.py out --all     # extract everything into ./out
python extract_all.py out --game jill1 --boards --shapes
python extract_all.py out --game jill1 --fonts --pages --tiles --hud
python extract_all.py out --game jill1 --music-wavs --music-seconds 30
python extract_all.py out --game jill1 --pack    # editable external pack
python test_parsers.py              # validate every parser

# Converters
python jill_import.py sha-frame  <sha> <entry> <frame> image.png -o out.sha
python jill_import.py vcl-sfx    <vcl> <slot> audio.wav        -o out.vcl
python jill_import.py sha-folder <sha> <folder>                -o out.sha
python jill_import.py vcl-folder <vcl> <folder>                -o out.vcl
```

Asset directories come from `[paths]` in `jill.ini` (one key per game id); manifests default to local `jillt` / `ONESIM`. The shipped `jill.ini` points the episodes at `D:/Emu/DOS/C/jillt` and Onesimus at `D:/Emu/DOS/C/ONESIM`. Use the **Game** menu to switch games, **File → Open Game Directory** to point at a different data folder, or pass a base directory: `python jill_extractor.py <base_dir>`.

## Keyboard Shortcuts

- **Game switching**: Use the **Game** menu to switch between all discovered games
- **Board Viewer**:
  - Click on tiles to see detailed info
  - Zoom slider to adjust view
  - Toggle Grid/Objects checkboxes
- **Shape Viewer**:
  - Select a shape entry from the left panel
  - Type a frame number (or use the arrows) to jump to a frame
  - **Play** animates the entry; editing the frame while playing jumps the animation
  - FPS spinner controls playback speed (1–60)
  - Switch video mode (VGA/EGA/CGA)

## File Format Details

### Board Files (`.jn1`, `.jn2`, `.jn3`, `.dem`)

```
Offset  Size    Description
0x0000  0x4000  Tile map (128×64 u16, column-major)
0x4000  2       Object count (u16)
0x4002  N×31    Object records (31 bytes each)
...     0x46    Board header (70 bytes, live HUD state)
...     ...     Strings (u16 length + bytes + NUL)
```

### Object Record (31 bytes)

| Offset | Type | Description |
|--------|------|-------------|
| 0x00 | u8 | Kind (0-68) |
| 0x01 | i16 | X position (world pixels) |
| 0x03 | i16 | Y position |
| 0x05 | i16 | X delta/velocity |
| 0x07 | i16 | Y delta/velocity |
| 0x09 | u16 | Collision width |
| 0x0B | u16 | Collision height |
| 0x0D | u8 | State/status byte |
| 0x0E | 5 bytes | Kind-specific data |
| 0x13 | u16 | Primary parameter |
| 0x15 | u16 | Flags |
| 0x17 | u16 | String offset (far pointer) |
| 0x19 | u16 | String segment |
| 0x1B | u16 | Kind-specific |
| 0x1D | u16 | TTL/countdown |

### Shape Archive (`.sha`)

```
Offset  Size    Description
0x0000  0x200   Directory: 128 × u32 offsets
0x0200  0x100   Directory: 128 × u16 sizes
0x300   ...     Shape data
```

Each shape entry contains frame count, source BPP (2/3/4/8), flags, optional palette, and multiple frames with opcodes (0=raw 8bpp, 1=packed bits, 2=empty).

### DMA File (`jill.dma`)

320 records of: `u16 tile_id`, `u16 shape_id`, `u16 attr`, `u8 name_len`, `char name[name_len]`

Maps tile IDs (from board) to shape IDs (from .sha) with attributes.

### VCL File (`.vcl`)

```
Offset  Size    Description
0x0000  0xC8    50 × u32 SFX offsets
0x00C8  0x64    50 × u16 SFX sizes
0x012C  0x64    50 × u16 SFX sample rates
0x0190  0xA0    40 × u32 Text-page offsets
0x0230  0x50    40 × u16 Text-page sizes
```

The second table is the text-page archive used by the F1/help screens, not
music. Each page is a CR-separated block whose first line is the title and
whose lines may begin with a `0`-`7` colour digit.

### DDT/ARK Files (`.ddt`, `.ARK`)

Standard Creative Music Format (CMF) with CTMF header for OPL2/AdLib playback.

### CFG Files (`.cfg`)

254 bytes: 10×10-byte names, 10×u32 scores, 6×12-byte save labels, 11×u16 options.

### MAC Files (`.mac`)

Variable-length records: first record has mask only, subsequent records have varint timestamp + mask + values for set bits (horiz_dir, vert_dir, action1, action2, ascii_key).

## Documentation References

This tool is based on extensive reverse-engineering documentation:

- `docs/jill1-engine.md` - Complete engine documentation for Episode 1
- `docs/jill2-differences.md` - Episode 2 deltas
- `docs/jill3-differences.md` - Episode 3 deltas
- `docs/shared-handler-equivalence.md` - Handler code sharing analysis
- `docs/episode-profiles.md` / `docs/profiles.json` - Per-episode constants
- `docs/jill23-frontend-flow.md` - Menu, demo, level graph
- `docs/jill23-sounds.md` - Sound system details

## License

The extractor code is covered by `LICENSE.md` (PolyForm Noncommercial 1.0.0).
It is a reverse-engineering tool for educational purposes. Jill of the Jungle
and the other supported games are copyright their respective owners; no game
data is licensed or distributed.