* Small

  * Fix Endings - DONE: state-3 CHECKPT returns to the main menu; `&end*.mac`
    input scripts are parsed and replayed (movement/jump/fire).
  * Intro.jn2 Theme Background based on Tile(35,0) to Tile(55,13) - DONE:
    menu themes now bake the intro-board picture panel at tiles
    `(35,0)`..`(55,13)` and hide the TEXT6/TEXT8 story text. - Shift Right 1 
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
* Splash Screens - Temporarily Disabled in the code
* Character Portraits
* Xargon Support? - no. Separate Project
* Classic/Modern Menu Layout
* Enhancements

  * Load External Enhanced Resource Toggles