* Fixes
  * Neaten up Classic Menus
  * Stereo Rendering - SBS, Interlaced - Code Disabled,
  * Fix PS4 Crashes
    * Quit: Still broken.
  * Check Version? 
  * Demo option on main menu for original demos (Inputs wrong)
  * Level 1 > MAP music carryover issue? - Check with the person
  * Tiny Landing pause with kneeling animation - Check with others if visible
  * Menu Sizes? (separate Menus themselves and the game render?)
* Least Important Ideas
  * Other Games
    * Xargon Support? - no. Separate Project. Use this and xargon's own source code as a base for it.
    * Vinyl Goddess? - no. Separate Project, Check compatibility.
    * Alien Rampage - no. Separate Project, Check compatibility.
  * Extractor
    * Select what to export with list and tickboxes (tick/untick all as an option)
    * Double check sound and music playback
  * HUD/Menu 
    * Custom HUD/Menu - Using hudlayout files
    * External HUD Customizer - Done
    * Menu/Font Scale - Done
    * Classic HUD - Health Bar a little off
  * Level Editor? - Base stuff off of HUD editor?
  * Demo
    * External Playback Editor/Viewer (backwards compatible with original exe if applicable)?
    * Dedicated Demo Playback mode (*.mac and *.mex?) - May help with ending key macros.
    * Record Playback mode (.mac or .mex?)
  * Build Other Platforms
    * Mac
    * Linux
    * Android? - NDK?
    * PS4
      * Builds and runs; quit crash + pack loading fixed 2026-11-13 (awaiting hardware confirmation)
    * Vita
      * Verify the rest on hardware: NOISEMAKER (needs inputs) - Working- Save/Restore, Hiscores, Ending, Minimap, Jill2/3, Onesimus
      * Check screen fit/letterboxing and readability at Vita resolution - 320x200 works. Max vita res =  950x544
      * Quit path: Works
      * Audio latency / SDL Vita device reopen after suspend-resume - Audio reopens after suspend-resume
      * Real LiveArea artwork (keep it 8-bit indexed in `sce_sys/`)
    * PSP? - Look into PSPSDK
    * XBOX - OpenXDK?
* Way Out there Ideas
  * Multiplayer.