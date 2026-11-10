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

