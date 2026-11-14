* Fixes
  * Neaten up Classic Menus/HUD
  * Stereo Rendering - SBS, Interlaced - Code Disabled,
  * Fix PS4 Crashes
    * Quit: Still broken.
  * Check Version? 
  * Demo option on main menu for original demos (Inputs wrong)
* Least Important Ideas
  * Other Games
    * Xargon Support? - no. Separate Project. Use this and xargon's own source code as a base for it.
    * Vinyl Goddess? - no. Separate Project, Check compatibility.
    * Alien Rampage - no. Separate Project, Check compatibility.
  * Demo
    * Playback Editor (backwards compatible with original exe if applicable)?
    * Dedicated Demo Playback mode (*.mac and *.pbk?) - May help with ending key macros.
    * Record Playback mode (.mac or .pbk?)
  * Build Other Platforms
    * Mac
    * Linux
    * Android? - NDK?
    * PS4
      * Builds and runs; quit crash + pack loading fixed 2026-11-13 (awaiting hardware confirmation)
    * Vita
      * Builds and runs on hardware (2026-11-14): front end, Jill 1 hub, music,
        external packs, pad (cross jump / square throw)
      * Verify the rest on hardware: Jill 2/3, Onesimus, save/restore,
        high scores, text pages, endings, minimap, NOISEMAKER
      * Check screen fit/letterboxing and readability at Vita resolution
      * Quit path: use the PS4 `_exit()` style teardown if the exit crash shows up
      * Audio latency / SDL Vita device reopen after suspend-resume
      * Real LiveArea artwork (keep it 8-bit indexed in `sce_sys/`)
    * PSP? - Look into PSPSDK
    * XBOX - OpenXDK?