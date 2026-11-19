* Fixes
  * Neaten up Classic Menus
  * Stereo Rendering - SBS, Interlaced - Code Disabled,
  * Fix PS4 Crashes
    * Quit: Still broken.
  * Check Version? 
  * Demos - Jill2 still desyncing JN2DEMO3 fine? JN3DEM1 Knife misses but completes anyway Desync after T460/R45, JN3DEM2 - blade desyncs somewhere after T250/R16 JN3DEM3 desync after T265/R34
  * Level 1 > MAP music carryover issue? - Check with the person - Not happening for me
  * Tiny Landing pause with kneeling animation - Check with others if visible
  * Menu Sizes? (separate Menus themselves and the game render?)
  * Docs/info readability?
  * Blinking multicoloured numbers for score 
  * Transform tokens need to stay on the board instead of being picked up when activated
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
      * Touch: D-pad + jump + throw + START/BACK work; hi-score name entry / Noisemaker need a keyboard or touch keypad
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
      * Builds: `build_xbox.cmd` → `JillEX-Xbox.zip` (contains default.xbe) - Fix xbox hdd first.
      * First console boot dropped to the dashboard with an empty log: trace output was suppressed on Xbox and the XBE used NXDK's 64 KiB stack (now 1 MiB). Awaiting retest.
      * Bring-up checklist in `port\xbox\README.md`
      * Saves/hiscores/log need a writable data root (disc root is read-only)
* Way Out there Ideas
  * Jill Maker Mode (Mouse Controls for editor, hotswitching between editor and gameplay, object selection)
    * Mouse Cursor highlight by grid
    * Item/Entity Pallette opened with a hotkey
    * Click to place, right click to remove, Shift click, alt click for any options (linked triggers etc)
    * Gameplay/Editor hotswap hotkey
    * Load/Save map files in native format (jn*, one)
  * Multiplayer.