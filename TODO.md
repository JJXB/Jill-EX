* Small

  * Jill Raise Animation - DONE: an eight-frame pull-up finishes an upward
    climb (climb poses `0x1A/0x19/0x18` then standing), rising the last few
    pixels out from behind the solid ledge cells below her.
  * Fix Endings - DONE: state-3 CHECKPT returns to the main menu; `&end*.mac`
    input scripts are parsed and replayed (movement/jump/fire).
  * blue orbs on vines/chains need fixing - DONE: Jill 1's kind table has 69
    kinds; VINECLIMB (blue sparks, tileset 0x3D) now draws/animates on Jill 1.
  * Health Bar HUD Colour select
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
