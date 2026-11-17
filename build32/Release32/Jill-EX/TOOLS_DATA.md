\# Tools and data

\* Data-driven runtime: `games/<id>.game` manifests plus

&#x20; `profiles/<id>.profile` engine profiles. `tools/mkprofile.py` profiles

&#x20; another Jill-engine EXE into that pair.

\* Asset paths live in `jill.ini` `\[paths]` (per game id); manifests keep local

&#x20; `jillt` / `ONESIM` defaults.

\* Headless debugging/verification CLI (`--ticks`, `--board`, `--start`,

&#x20; `--dump`, `--dump-objects`, `--watch-tile`, `--morph`, `--face`, `--save`,

&#x20; `--load`, `--text-page`, `--dump-splash`, `--list-games`).

\* `jill\_extractor.py` viewer/extractor \[readme here](EXTRACTOR\_README.md) and the reverse-engineering notes in

&#x20; `docs/`.

\* `tools/hud\_layout.py` HUD alignment editor (Tk + Pillow): overlays a

&#x20; reference image, stamps tiles from a PNG or the game's `.dma` + `.sha`

&#x20; tile table, and writes `.hudlayout` files. `--summary` prints pixel/grid

&#x20; coordinates and DMA tile ids/names for engine work; see

&#x20; `docs/hud-layout-tool.md`.

\* `jill_demo_studio.exe` external `.mac` playback viewer/editor, built from

&#x20; `tools/demo_studio` for x64 and Win32 and included in both Windows release

&#x20; zips. See `docs/demo-studio.md`.

