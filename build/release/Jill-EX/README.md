# Jill-EX

A standalone attempt at reverse engineering and recreating the Jill of the Jungle engine.

## AI
This did use Deepseek 4.1 Flash to construct and make. Disclaimer Over.

## Supported Games

* Jill of the Jungle
* Jill Goes Underground
* Jill Saves The Prince
* Onesimus: A Quest for Freedom

No game data or artwork is shipped - supply your own legally obtained copy of the games (see Legal / trademarks).

## Features

### Modernization
* Modern HUD and Menus with Customization
* High framerate support - 60/90/120/144 Hz - Interpolation as a separate option - Logic still at 18.2hz
* Expanded Limits (Board Obj Cap = 384, KIND Limit = 138, Visible/Active Obj Cap = 384)
* Windowed, exclusive fullscreen and borderless modes; resolution presets and 1x-6x window scaling.
* Noisemaker Menu in Options/Sound (Old Style Noisemaker TBD)
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
  * OPL2 FM or Midi out (CMF to Midi)
* Cheat menu:
  * Invincible
  * Debug Fly
  * Super Jump
  * Add Items
  * Refill Health 
  * BECOME JILL/FROG/BIRD/SPIDER/FISH (Spider Unused but logic present in files, uses shape entry 38).
  * Level/Board Select

## Command Line Arguments and Keyboard Hotkeys
[See COMMANDLINE](COMMANDLINE.md)
[See HOTKEYS](HOTKEYS.md)

## TODO

### Remaining Realistic Goals
* Classic Menu work
* Old Style Noisemaker
* Work on Display Filters to be continued 
* Mac/Linux/Console builds

### Future Ideas
* Xargon Port based on this and it's own source

## License
[See LICENSE](LICENSE.md) - PolyForm Noncommercial 1.0.0 for the port, tools and
documentation.  Third-party components are listed in
[THIRD_PARTY_NOTICES](THIRD_PARTY_NOTICES.md).

## Legal / trademarks

This is an unofficial, non-commercial fan project.  It is **not** affiliated
with, endorsed by or licensed by Epic Games.

* "Jill of the Jungle", "Jill Goes Underground", "Jill Saves the Prince",
  "Onesimus" and the associated character names are trademarks of Epic Games.
* No original game data or artwork is distributed in this repository.  The
  `games/` and `profiles/` folders contain reconstructed engine configuration
  (tables, board lists, manifests); `profiles/onesimus.pal` is a 768-byte
  palette taken from the original Onesimus executable and is included only so
  the game can use its intended colours.  It is not covered by this project's
  licence.
* The PS4 package icon (`port\ps4\sce_sys\icon0.png`) uses artwork derived from
  the original game.  It stays the property of its owner, is used only to
  identify this fan port, and is not covered by this project's licence.
* `DISCLAIMER.txt` repeats the notice for binary/data distributions; the build
  copies it next to the `.pkg` and into `JillEX-data.zip`.
* Reverse-engineering scratch output (extracted executables, Ghidra projects,
  decompiled code, disassembly listings, string dumps and rendered game art)
  is excluded from the repository by `.gitignore` and must not be
  redistributed.
* You must supply your own legally obtained copy of the games.  Extracted,
  edited or rendered assets remain the property of their respective owners.

## Engine and gameplay Info
[See ENGINE](ENGINE.md)

## Tools and data
[See TOOLS_DATA](TOOLS_DATA.md)

## Adding Mods and External Enhanced Assets
* [See MODS](MODS.md)
* [See EXTASSETS](EXTASSETS.md)
* [See IMPORT_README](IMPORT_README.md)
* [See EXTRACTOR_README.md](EXTRACTOR_README.md)

## Building
[See BUILDING](BUILDING.md)