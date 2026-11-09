# Jill-EX

A standalone attempt at reverse engineering and recreating the Jill of the Jungle engine.


## AI
This did use Deepseek 4.1 Flash to construct and make. Disclaimer Over.

## Supported Games

* Jill of the Jungle
* Jill Goes Underground
* Jill Saves The Prince
* Onesimus: A Quest for Freedom

No copyright assets shipped, provide your own.

## Features

### Modernization
* Modern HUD and Menus with Customization
* High framerate support - 60/90/120/144 Hz - Interpolation as a separate option - Logic still at 18.2hz
* Expanded Limits (Board Obj Cap = 384, KIND Limit = 138, Visible/Active Obj Cap = 384)
* Windowed, exclusive fullscreen and borderless modes; resolution presets and 1x-6x window scaling.
* Extra Features (All Optional)
  * Minimap
  * Centred fish shot
  * Fire while crouched
  * 5 knife limit maximum
  * Retain items between levels
  * faster dialog boxes
  * Air jump buffer
  * Jill Select: play as the Jill 1/2/3 or Custom Colour in any episode. Onesimus Support WIP
* Keyboard and gamepad input with rebindable movement, jump and throw.
* SDL Sound Output
* Cheat menu:
  * Invincible
  * Debug Fly
  * Super Jump
  * Add Items
  * Refill Health 
  * BECOME JILL/FROG/BIRD/SPIDER/FISH (Spider Unused but logic present in files, generated temporary art).
  * Level/Board Select

## TODO

### Remaining
* Splash screens to be fixed.
* Noisemaker Modes
* Work on Display Filters to be continued 
* Verify music playback is right.
* Character portraits.
* External enhanced asset loading
* Mac/Linux/console builds.

## Engine and gameplay Info
* One executable, four games: Jill 1, Jill 2, Jill 3 and Onesimus.
* Original 18.2 Hz logic with the full 69-kind object system, tile collision,
  hazards, doors, elevators, switches, triggers and object compaction.
* All shipped boards, level titles, checkpoint warps, hub/world-map flow and
  ending sequences (`&end*.mac` playback, ending return to menu).
* Save/restore: six slots, the pause-menu SAVE/RESTORE screens and the original
  S/R hotkeys. Saves use the original board layout and per-game prefixes.
* Text pages: `.vcl` help/news/Philemon pages, page apples, F1 help, one
  screenful per box, page stepping and scrolling.
* Morph forms: fish, bird, frog and the restored spider form, plus TINY map
  avatar support.

## Tools and data
* Data-driven runtime: `games/<id>.game` manifests plus
  `profiles/<id>.profile` engine profiles. `tools/mkprofile.py` profiles
  another Jill-engine EXE into that pair.
* Asset paths live in `jill.ini` `[paths]` (per game id); manifests keep local
  `jillt` / `ONESIM` defaults.
* Headless debugging/verification CLI (`--ticks`, `--board`, `--start`,
  `--dump`, `--dump-objects`, `--watch-tile`, `--morph`, `--face`, `--save`,
  `--load`, `--text-page`, `--dump-splash`, `--list-games`).
* `jill_extractor.py` viewer/extractor [readme here](EXTRACTOR_README.md) and the reverse-engineering notes in
  `docs/`.

## Adding Mods
[See MODS](MODS.md)

## Building
[See BUILDING](BUILDING.md)