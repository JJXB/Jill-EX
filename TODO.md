* Fixes
  * Neaten up Classic Menus - Mostly there?
  * Stereo Rendering - SBS, Interlaced - Code Disabled,
  * Fix PS4 Crashes
    * Quit: videoout now flips to the spare group, unregisters it, then flips
      to the system blank (`-1`) before closing/freeing.  Built 2026-11-20
      (startup marker "quit diag 4") - awaiting test.
  * Check Version? 
  * Demos
    * JN2DEM2 
      * Landing after T497/R67 damage kills jill where it shouldn't
    * JN3DEM2
      * Jump throw misses around T299/R23
        * The throw is record 24 at T304.  The star (spawned at `(400,438)`,
          `dx=-6`) homes toward INCHWORM 33 (`16x8` at `(354,440)`) and on
          frame 309 is 1 px below the worm's bottom edge (`448`), so the strict
          AABB never fires.  Handler and spawn match `1A41:2B1A` /
          `1FAE:2E61` exactly - recheck on the current build.
      * Jump throw bounces off wall instead of hitting enemy (Jump around T792/R116)
        * Current build: the wall on frame 803 flips `dx` to `+7` and the star
          hits BIGANT 20 on frame 804 (score 22 -> 28) - recheck on the
          current build.
  * Level 1 > MAP music carryover issue? - Check with the person - Not happening for me
  * Tiny Landing pause with kneeling animation - Check with others if visible
  * Menu Sizes? (separate Menus themselves and the game render?)
  * Docs/info readability?

  * Tile issue with editor
  * No Transition Text boxes on menu demos

  * Demo studio:

* Least Important Ideas
  * Other Games
    * Xargon Support - Extractor: done (boards, tiles, shapes+palette, AUDIO SFX/text/EPC, CMF music, macros, CONFIG, saves). Engine port is still a separate project.
    * Vinyl Goddess? - no. Separate Project, Check compatibility.
    * Alien Rampage - no. Separate Project, Check compatibility.
  * Extractor
    * Select what to export with list and tickboxes (tick/untick all as an option)
    * Double check sound and music playback
  * HUD/Menu 
    * Classic HUD - Health Bar a little off
  * Level Editor? - Base stuff off of HUD editor?
  * Demo
    * External Playback Editor/Viewer - Done: tools\demo_studio (see docs\demo-studio.md)
    * Dedicated Demo Playback mode (*.mac and *.mex?) - May help with ending key macros.
    * Record Playback mode (.mac or .mex?)
  * Build Other Platforms
    * Mac
    * Linux
    * Android? - NDK? Installed
      * Builds and installs (`build_android.cmd` → `JillEX-android.apk`); needs an on-device pass - Not working yet. Change the location of data files to somewhere on internal storage instead (/storage/emulated/0/JillEX)
      * Fire Tablet - Working
      * S25 Ultra - Working
      * Touch: D-pad + jump + throw + START/BACK work; hi-score name entry / Noisemaker need a keyboard or touch keypad - Fix Touch
      * Confirm audio latency, suspend/resume and letterboxing on real devices
    * PS4
      * Builds and runs; quit crash + pack loading fixed 2026-11-13 (awaiting hardware confirmation)
    * Vita
      * Verify the rest on hardware: NOISEMAKER (needs inputs) - Working- Save/Restore, Hiscores, Ending, Minimap, Jill2/3, Onesimus
      * Check screen fit/letterboxing and readability at Vita resolution - 320x200 works. Max vita res =  950x544
      * Quit path: Works
      * Audio latency / SDL Vita device reopen after suspend-resume - Audio reopens after suspend-resume
      * Real LiveArea artwork (keep it 8-bit indexed in `sce_sys/`)
    * PSP? - Look into PSPSDK
    * XBOX - NXDK Installed
      * Runs on hardware: `build_xbox.cmd` → `JillEX-Xbox.zip` (contains default.xbe) - stable frame rate at 480i/480p/720p, pad + music + sfx working (2026-11-19)
      * Remaining: writable data root for saves/hiscores/log, widescreen stretch option, Jill 2/3 + Onesimus + Noisemaker + text pages untested on console
      * Details and perf notes in `port\xbox\README.md`

* Way Out there Ideas
  * Jill Maker Mode (Mouse Controls for editor, hotswitching between editor and gameplay, object selection)
    * Mouse Cursor highlight by grid
    * Item/Entity Pallette opened with a hotkey
    * Click to place, right click to remove, Shift click, alt click for any options (linked triggers etc)
    * Gameplay/Editor hotswap hotkey
    * Load/Save map files in native format (jn*, one)
  * Multiplayer.