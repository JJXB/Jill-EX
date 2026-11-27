* Last Fixes for 1.0 without confirmation
  * Demos
    * JN2DEM2
      * 2026-11-27: late fireball/demon contact no longer forces a respawn;
        JN2DEM2 playback floors health at 1 so the recorded roll survives the
        low-health section.  Needs a demo pass.

* Fixes that still need feedback
  * Docs/info readability?

* Done

* Least Important Ideas
  * Stereo Rendering - SBS, Interlaced - Code Disabled assess 
  * Other Games
    * Xargon Support - Extractor: done (boards, tiles, shapes+palette, AUDIO SFX/text/EPC, CMF music, macros, CONFIG, saves). Engine port is still a separate project.
    * Vinyl Goddess? - no. Separate Project, Check compatibility.
    * Alien Rampage - no. Separate Project, Check compatibility.
  * Extractor
    * Select what to export with list and tickboxes (tick/untick all as an option)
    * Double check sound and music playback
  * Build Other Platforms
    * Mac
    * Linux
    * Android? - NDK? Installed
      * Fire Tablet - Working
      * S25 Ultra - Working
    * PS4
      * Builds and runs; quit crash + pack loading fixed 2026-11-13 (awaiting hardware confirmation)
    * Vita
      * Check screen fit/letterboxing and readability at Vita resolution - 320x200 works. Max vita res =  950x544
      * Quit path: Works
      * Real LiveArea artwork (keep it 8-bit indexed in `sce_sys/`)
    * PSP? - Look into PSPSDK
    * XBOX - NXDK Installed
      * Remaining: writable data root for saves/hiscores/log, widescreen stretch option
  * Tiny Landing pause with kneeling animation - Check with others if visible now - assuming fixed since person didn't mention it in followup.

* Way Out there Ideas
  * Multiplayer.