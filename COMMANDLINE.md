# Command-line reference [verified]

All options below come from the current `--help` output and argument parser.
Settings in `jill.ini` provide the defaults; command-line options override
them.  Relative paths resolve against the directory that holds `jill.ini`
(the launcher moves there).

## Game executable

```
port\build\Release\jill.exe [options]
run.cmd [options]
```

`run.cmd` simply starts the Release build with the arguments you pass.

### Game and data

| Option | Meaning |
| --- | --- |
| `--game ID` | Installed manifest id (`jill1`, `jill2`, `jill3`, `onesimus`) |
| `--games DIR` | Directory of `*.game` manifests (default `games`) |
| `--profiles DIR` | Directory of `*.profile` engine data (default `profiles`) |
| `--list-games` | Print installed games and exit |
| `--external DIR` | External asset directory (default `external`) |
| `--pack ID` | Mount a pack folder or `.zip` at start-up |
| `--list-packs` | Print discovered packs and exit |
| `--episode N` | N-th installed game, 1-based |
| `--data DIR` | Override the manifest asset directory |
| `--board NAME` | Board name without extension (default: game start) |
| `--palette FILE` | Optional raw 768-byte VGA palette |
| `--menu-theme N` | `0` = port backdrop, `N` = N-th game's intro board |
| `--splash N` | Publisher page `N`; display is currently disabled [verified] |

### Video and presentation

| Option | Meaning |
| --- | --- |
| `--scale N` | Windowed integer scale, 1..6 (default 3) |
| `--high-fps N` | Presentation rate: `0` (18.2 Hz), 60, 90, 120 or 144 |
| `--no-interpolate` | Disable high-framerate sprite smoothing |
| `--filter NAME` | `0`/none, `1`/smooth, `2`/crt |
| `--fullscreen` | Exclusive fullscreen |
| `--windowed` | Windowed (default) |
| `--borderless` | Borderless fullscreen |
| `--resolution WxH` | Window/display size, or `desktop` |

### HUD and dialogs

| Option | Meaning |
| --- | --- |
| `--hud` | Start with the CLASSIC HUD (F2 toggles) |
| `--no-portrait` | Hide the character portrait beside level-title dialogs |
| `--portrait` | Show the portrait (default) |
| `--classic-menu` | Draw menus with the original frame/window art |
| `--modern-menu` | Use the default modern menu layout |
| `--menu-frame N` | Classic menu border source: 0 = current game, N = installed game N |
| `--hud-frame N` | Classic HUD frame source: 0 = current game, N = installed game N |
| `--hud-bg N` | HUD background palette index, `suit` or `vibrant` |
| `--hud-text N` | HUD label colour palette index (default 7) |
| `--hud-health N` | Health colour 0..15, or `original` |
| `--minimap` | Draw the board overview in the lower-right corner |
| `--no-message-box` | Plain bottom messages, no background frame |

### Gameplay and enhancements

| Option | Meaning |
| --- | --- |
| `--fire-crouched` | Start with fire-while-crouched enabled |
| `--retain-items` | Keep keys/weapons between levels |
| `--fast-dialogs` | Four characters per tick for captions/pages |
| `--air-jump-buffer` | Keep the original hold-jump-through-a-fall buffer |
| `--center-fish-shot` | Centre the fish form's shot vertically |
| `--max-knives N` | Knife cap: 3 or 5 |
| `--jill-look N` | `0` = game default, `N` = installed game N (Jill Select) |
| `--jill-colour N` | `0` = look colours, 1..8 = custom suit presets |

### Audio

| Option | Meaning |
| --- | --- |
| `--no-music` | Disable the OPL2/CMF music player |
| `--music-volume N` | Music volume 0..100 (default 80) |
| `--no-sfx` | Disable sampled effects |
| `--sfx-volume N` | Effect volume 0..100 (default 80) |
| `--pc-speaker` | Play effects as 1-bit PC-speaker square waves |
| `--audio-check` | Open the device, play 1.5 s and report format/levels |

### Debug and headless testing

| Option | Meaning |
| --- | --- |
| `--ticks N` | Run N logic ticks headlessly, then print state and exit |
| `--dump FILE` | With `--ticks`: write the final frame as binary PPM |
| `--dump-objects FILE` | With `--ticks`: write the object table as text |
| `--dump-splash PREFIX` | Write the selected splash pages as `PREFIX1.ppm`, ... |
| `--start X,Y` | With `--ticks`: place the player at X,Y |
| `--level N` | With `--ticks`: override the board header level id |
| `--watch-tile N` | With `--ticks`: print tile N's live record each tick |
| `--invincible` | Debug: no damage (F5 toggles) |
| `--super-jump N` | Treat N feathers as carried (0..15) |
| `--items N` | Start with N knives |
| `--blade` | Spawn one spinning blade in front of the player |
| `--morph N` | `0` normal, `4` frog, `5` bird, `6` spider, `7` fish |
| `--face N` | Force facing `-1` (left) or `1` (right) |
| `--fire` | Press throw once at the start |
| `--text-page N` | Open `.vcl` text page N (1 = How to Play) |
| `--save N` | Write save slot N after the run / on exit |
| `--load N` | Restore save slot N before the run |

### Music rendering

| Option | Meaning |
| --- | --- |
| `--dump-music FILE` | Render the selected track to a WAV and exit |
| `--music-file NAME` | Track for `--dump-music` (default: game title track) |
| `--music-seconds N` | Seconds to render (default 10) |

Examples:

```
jill.exe --game jill1 --hud --minimap
jill.exe --game jill3 --board 14 --pc-speaker
jill.exe --game jill1 --dump-music zeppelin.wav --music-file zeppelin.ddt --music-seconds 30
jill.exe --game jill2 --board 1 --ticks 40 --dump frame.ppm --dump-objects objects.txt
```

## Build and packaging

| Command | Meaning |
| --- | --- |
| `build.cmd` | Configure/build the 64-bit Release executable |
| `build.cmd install` | Install `sdl2:x64-windows` through vcpkg first |
| `build32.cmd` | Configure/build the 32-bit Release into `port\build32\Release32` |
| `build32.cmd install` | Install `sdl2:x86-windows` through vcpkg first |
| `run32.cmd` | Run the 32-bit build |
| `make_release.cmd [zip]` | Stage a release folder and write a zip |
| `copy.cmd` | Local developer copy to `D:\Games\JillEX` |

`build.cmd` uses `VCVARS` (default Visual Studio 2022 Community) and
`VCPKG_ROOT` (default `C:\vcpkg`).

## Extractor and tools

### GUI extractor

```
py -3 jill_extractor.py [base_dir]
```

`base_dir` optionally points at a directory containing game folders; without
it the extractor uses the configured manifests.

### Batch extractor

```
py -3 extract_all.py [output_dir] [--game ID ...] [--all] [flags]
```

| Flag | Export |
| --- | --- |
| `--boards` | Board renders |
| `--shapes` | Shape frames |
| `--sounds` | SFX |
| `--music` | Music data |
| `--music-wavs` | Render music to WAV through the built port |
| `--music-seconds N` | Seconds per rendered track (default 60) |
| `--demos` | Demo macros |
| `--config` | Config data |
| `--dma` | DMA tile table |
| `--fonts` | `fonts/main.png` + glyphs |
| `--pages` | VCL text pages |
| `--tiles` | `tiles/<id>.png` + `tiles.json` |
| `--hud` | HUD panels/icons and intro theme |
| `--palette` | `palette.pal` / JSON |
| `--previews` | Full 2048x1024 board previews |
| `--pack` | Ready-to-edit external pack |
| `--all` | Every exporter |
| `--list` | List known games and exit |

### Importer

```
py -3 jill_import.py <command> ...
```

See `IMPORT_README.md` for the full command list, input sizes and examples.

## Notes

* `--help` (`-h`) prints the executable's current option list [verified].
* `--dump-splash` still renders pages even though the in-game splash display
  is currently turned off [verified].
* The game requires its own data files; no copyrighted assets are shipped.
