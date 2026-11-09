* Small

  * Jill Raise Animation - DONE: an eight-frame pull-up finishes an upward
    climb (climb poses `0x1A/0x19/0x18` then standing), rising the last few
    pixels out from behind the solid ledge cells below her.
  * Fix Endings - DONE: state-3 CHECKPT returns to the main menu; `&end*.mac`
    input scripts are parsed and replayed (movement/jump/fire).
  * Gameplay Option - Jill Select - DONE: GAMEPLAY > JILL LOOK cycles GAME
    DEFAULT/JILL 1/JILL 2/JILL 3/ONESIMUS via the episodes' own player colour
    tables, plus JILL COLOUR presets (green, red, blue, yellow, orange, cyan,
    purple, silver).
  * Make Onesimus board 1 first map - DONE: starts on `1.ONE`; new `map_board`
    manifest key keeps `MAP.ONE` as the hub.
  * Jill1 Map Spawn glitchy - DONE: hub restore no longer runs
    `place_at_checkpoint(0)` on the map's music trigger; the player keeps the
    entrance position and the camera re-centres.
  * blue orbs on vines/chains need fixing - DONE: Jill 1's kind table has 69
    kinds; VINECLIMB (blue sparks, tileset 0x3D) now draws/animates on Jill 1.
* Sound?

  * Midi
  * FM
  * External Soundtrack and Sample Replacement
* Splash Screens (On/Off/Custom/Onesimus) - DONE: MENUS > SPLASH cycles
  OFF/JILL1/JILL2/JILL3/ONESIMUS and shows two data-driven pages (publisher
  logo cluster, then the title card) found from the intro board; saved as
  `[video] splash` in `jill.ini` (`--splash N`, `--dump-splash P`).
* Character Portraits
* Xargon Support? - no. Separate Project
* Text pages - DONE: `.vcl` 40-entry page bank (`TextPages`), framed viewer with
  one screenful per box (Left/Right steps a box then a page), `PAGE n /
  SCREEN k/m` footer, page apples (kind 1 with `state > 0`) for Jill 1's
  "News bulletin" screens and the Onesimus Philemon verses, F1 help,
  `--text-page N`; gamepad D-pad/stick scrolling and page stepping

* Classic/Modern Menu Layout
* Enhancements

  * Display Filters
  * Load External Enhanced Resource Toggles
  * Mods - DONE: `games/*.game` manifests + `profiles/*.profile` runtime engine
    data (`docs/game-manifests.md`)
  * Profile tool - DONE: `tools/mkprofile.py` (EXE + asset folder -> profile +
    manifest, no rebuild).  Verified on Onesimus.
  * Remaining mod work: exe-relative paths / per-game bindings
    (`docs/game-manifests.md` Open).  Per-game save names are DONE
    (`save_prefix`/`save_slots` in each profile).
* Linux? Mac? Other System Builds?

* Tools to import/view/export resources