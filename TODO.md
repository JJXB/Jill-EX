* Small

  * Fix Endings - DONE: state-3 CHECKPT returns to the main menu; `&end*.mac`
    input scripts are parsed and replayed (movement/jump/fire).
  * Intro.jn2 Theme Background based on Tile(35,0) to Tile(55,13)? maybe (0,35) to (19,46)? (93, 51) to (112, 62)? without rendering any text?
  * Noisemaker
* Sound?

  * FM / CMF Music - DONE: register-level OPL2 core + CMF sequencer
    (`port/src/opl2.*`, `cmf.*`, `music.*`, `audio.*`), title and level tracks,
    rhythm mode, `OPTIONS > AUDIO`, `[audio]` ini keys, `--dump-music`.
    See `docs/port-sound.md`.
  * PC-speaker `_noisemaker` fallback synthesis (optional; the sampled path is
    used on all modern systems).
  * Midi (external MIDI output)
  * External Soundtrack and Sample Replacement
* Splash Screens (On/Off/Custom/Onesimus) - DONE: MENUS > SPLASH cycles
  OFF/JILL1/JILL2/JILL3/ONESIMUS and shows two data-driven pages (publisher
  logo cluster, then the title card) found from the intro board; saved as
  `[video] splash` in `jill.ini` (`--splash N`, `--dump-splash P`).
* Character Portraits
* Xargon Support? - no. Separate Project

* Classic/Modern Menu Layout
* Enhancements

  * Load External Enhanced Resource Toggles