# Progress

## 2026-11-22 (JN3DEM2 throwing-star check: T304 near-miss, T792 bounce kill)

* The two flagged JN3DEM2 throws were traced tick for tick.  Record 23
  (`T299`, B1 release) only ends the jump; the throw is record 24 at `T304`
  (studio tick 304, engine frame 305).
* T304 throw: the star spawns at `(400,438)` (player `(408,436)`, facing left,
  `x - 0xC + 4`, `y + 2`) with `dx = -6`.  It homes horizontally toward
  INCHWORM 33 (`16x8` at `(354,440)`), gains `dy += 1` every frame, and on
  frame 309 sits at `(366,448)` - exactly 1 px below the worm's
  `y + h = 448` - so the strict `<` AABB never fires and the star passes
  under it.  `1A41:2B1A`, `26F4:12B9`, `26F4:263A`/`2772` and the
  `1FAE:2E61` spawn were re-read line by line and the port matches, so this
  is the recording's real trajectory (the star misses), not a port
  divergence.
* T792 throw: the star spawns at `(96,30)`; the `0x153` wall in columns 0-3
  blocks the horizontal probe on frame 803, so `dx` flips to `+7`; on frame
  804 it overlaps BIGANT 20 (`24x16` at `(70,64)`) and `_killaward` kills it
  (score 22 -> 28).  The "bounce instead of hit" case no longer reproduces
  on the current build.
* Removed the temporary `JILL_DBG_BLAD` tracing from `engine.cpp`; no logic
  change from the tested build.  Nine-macro headless regression unchanged
  (JN3DEM1 ends hp=7 inv=2 objs=14, JN3DEM2 hp=3 inv=5 objs=75, etc.).

## 2026-11-21 (demo studio: direct mouse editing of the record table)

* Clicking a cell now edits it in place: `TICK` opens the inline number editor
  (fresh value, `ENTER` applies, `ESC` cancels), `H`/`V` cycle
  `- -> R/D -> L/U -> -`, `B1 J`/`B2 T` toggle jump/throw, `KEY` cycles `-` and
  every key value the recording already uses, and each `MASK` letter toggles
  its channel.  Every click sets the matching mask bit and is one undo step.
* `SHIFT`+click runs the same action backwards; `SHIFT`+right-click clears the
  clicked column's mask bit (on `#`/`TICK`/`D` it clears the whole mask, which
  turns the record into a pure time marker).
* Column headers are now `B1 J` and `B2 T` so the buttons read as jump/throw.
  Help overlay, panel hint lines, footer, `docs/demo-studio.md` and `TODO.md`
  updated.  Both `jill_demo_studio.exe` builds rebuilt and the release staging
  refreshed; `--selftest` still reports `reencode=identical`.

## 2026-11-21 (port: knife handler fidelity)

* KNIFE (kind 2) update now returns 0 while the knife is lying on the ground
  (`p11 == 0`), matching `1644:0fff`: a landed knife is a pickup, not a
  temporary wall.  Only the falling frame (`p11 == -1`) and the flight frames
  still report "blocked".  This stops resting knives from becoming solid cells
  for Jill/enemies.
* KNIFE hit on an enemy uses `_killaward` (score + footprint + kill) instead of
  a bare score popup + kill, so the footprint is part of the map state the
  respawn/checkpoint snapshot keeps.
* The update pass now marks *both* objects `0xC000` on any AABB hit, as
  `26F4:12B9` does.  Flying projectiles therefore act as temporary walls for
  the rest of the frame exactly like the original.
* Headless replay check: all shipped demos produce the same end-of-macro state
  as before the change; JN3DEM1 still completes.
* Demo studio: `StudioOptions` now reads `[gameplay] respawn_mode`,
  `phoenix_water_death` and `throw_from_vines` from `jill.ini` and applies them
  to its engine, with matching `--respawn N`, `--no-phoenix-water-death` and
  `--throw-from-vines` switches.  Since the tool links `port/src/engine.cpp`
  directly, the KNIFE fixes above are part of its build too.  Rebuilt x64 and
  Win32 `jill_demo_studio.exe` and refreshed the copies in `build/release/`
  and `build32/Release32/` together with the matching `jill.exe` builds;
  headless JN3DEM3 output matches the game tick for tick.
* A knife whose fall is blocked only by a *temporary* wall still froze: the port
  turned a `move_obj` result of 0 into `p11 = 0`, so a knife wedged under a
  block (Jill 1 board 3: the 1-tile gap between the 2-tile block and the snake
  platform) rested in mid-air for good.  `26F4:1C89` `_trymove` always runs its
  third probe `(new_x, old_y)`, which for a straight-down step is the object's
  current cell, so a blocked fall returns 4 and the caller stays in the falling
  state.  `move_obj` gained `bool exact` (default false); KNIFE passes true.
  A/B headless check (`exact=false` vs `true`, all shipped demos): JN1DEMO,
  JN2DEM1..3, JN3DEM1/2, END1/END2 are identical at the end tick; JN3DEM3 is the
  only board that changes, and the pre-fix build leaves three knives resting in
  mid-air (`p11 == 0`, `dy == 4`, e.g. `(1028,774)`) where the fixed build keeps
  them falling/homing.  x64 + Win32 `jill.exe` and both `jill_demo_studio.exe`
  builds agree tick for tick on that board; release staging refreshed with
  `make_release.cmd` / `make_release_32.cmd`.

## 2026-11-20 (PS4 data zip renamed)

* `build_ps4.cmd` now writes/copies `JillEX-PS4-Data.zip` (repository root and
  `port\ps4\build`) instead of `JillEX-data.zip`, so the PS4 companion data is
  clearly separated from the desktop and Vita archives.  `.gitignore`,
  `BUILDING.md`, `README.md`, `docs/licensing.md` and `PS4_PORT_PLAN.md`
  updated.

## 2026-11-20 (port: respawn modes, phoenix waterfall, snake hits, page numbers)

* `OPTIONS > GAMEPLAY > RESPAWN` adds `RESET TO CHECKPOINT` (default),
  `RESET TO LEVEL START` and `NO RESET`.  The board (map, objects and header)
  is snapshotted at load and again whenever a CHECKPT is touched; death
  restores the selected snapshot, so elevator/platform positions and opened
  doors rewind consistently.  Score always survives; the reset is applied at
  the start of the next update pass so no handler reference is invalidated.
  INI key `[gameplay] respawn_mode` (`0`/`1`/`2`), CLI `--respawn N`.
* `PHOENIX WATER DEATH` (default ON) makes the phoenix form die on waterfall /
  water-body tiles (`0x198`..`0x1A3`); `--no-phoenix-water-death` keeps the
  old splash-only behaviour.  INI `[gameplay] phoenix_water_death`.
* `THROW FROM VINES` (default OFF) lets Jill throw a knife/blade from the climb
  state; the original restricts throws to states 0 and 2.  INI
  `[gameplay] throw_from_vines`, CLI `--throw-from-vines`.
* Text-page dialogs no longer print `PAGE n` in the title or footer.
* A SNAKE now loses one 8-px body segment per hit; the 35-point award lands
  only when the body is gone.  `docs/jill1-kind-handlers.md` updated.
* The cheat formerly labelled `BECOME BIRD` is now `BECOME PHOENIX`.

## 2026-11-20 (port: gem tracking, checkpoint respawn, exit items, score popups)

* Gems (item 3) now have an authoritative `Engine::gems_` counter.  A board file
  can no longer grant a free gem, changing level does not add one, and the
  header inventory is rebuilt from the counter whenever a board is loaded or
  restored.  Unspent gems still follow Jill between levels.
* Touching a `CHECKPT` now records the player's position as the respawn point,
  so death restarts at the last checkpoint instead of the board entry.
* Leaving a level through `!` drops every non-gem item (keys, knives, etc.).
  Only gems are put back into the restored hub/map header; the `retain_items`
  enhancement still keeps the whole inventory.
* Floating score popups (kind 27) blink and cycle through bright palette pairs,
  one colour pair per digit.
* Text pages are no longer switchable with Left/Right, the D-pad or the stick;
  vertical scrolling and close still work.  `HOTKEYS.md` updated.

## 2026-11-20 (editor: kind-aware level canvas; original-game-safe tiles)

* Preserving the palette words' low bits did not fix the original DOS game, so
  tile palette cloning is no longer exposed in the GUI.  Tile import now has a
  **Dither to existing palette** checkbox (`jill_import.quantize(dither=True)`
  plus `--dither` on `sha-frame`/`sha-tile`): the palette and `.dma` are never
  touched, and new art is approximated with Floyd-Steinberg dithering.  The
  `tile-clone` CLI remains available but is documented as port-only.
* Level canvas gained object overlays: coloured boxes/short kind names
  (**Kinds**), dashed yellow lines between objects sharing a non-zero
  `primary_param` trigger channel (**Links**), a right-click properties menu
  (edit/select links/delete), Shift+left-click to delete an object under the
  cursor, Ctrl+left-click to paint a configurable **Erase tile**, and zoom
  cycling at 1x/2x/4x.  Smoke test exercises painting, both erase modes,
  zoom and the overlays without error dialogs.
* Docs updated in `EDITOR_README.md` and `IMPORT_README.md`.

## 2026-11-20 (Vita / Android: PS4-only fault handler guard)

* `main.cpp` called `install_ps4_fault_handlers()` from the shared
  console/Android log setup, but the helper is compiled only for `JILL_PS4`.
  Guarded the call (and its PS4 quit trace) with `#if defined(JILL_PS4)`, which
  fixes the Vita and Android builds.  Also removed the now-unused
  `lower_ascii` / `text_has_word` helpers to clear the Android warning.
* Rebuilt: `build.cmd`, `build_vita.cmd` and `build_android.cmd` all pass.

## 2026-11-20 (port: STORY menu from intro boards)

* Added a front-end `STORY` row (after `RESTORE GAME`).  It lists every
  installed game/mod, then shows that game's story as a full-screen board page:
  the intro board is loaded in a temporary `Engine`, the `CHECKPT` with
  `p13 = 0` is used as the original page anchor, and the page is rendered at
  its `x,y` (following `p1b` links when a mod has more than one page).  Enter /
  Right advances, Left / Esc returns to the selector.  Uses the selected game's
  palette so Onesimus and total-conversion mods keep their colours.
* The menu is still the classic-only layout from the previous change; the story
  viewer bypasses the menu chrome and draws the board page directly.
* Builds: `build.cmd` clean.  Headless camera renders confirmed the page
  anchors at the expected intro-board regions for Jill 1/2/3.
* No in-game run was performed (user controls interactive testing).

## 2026-11-20 (port: external Jill look, Nuked default, classic-only UI)

* `GAMEPLAY > JILL LOOK` gained an `EXTERNAL PACK` entry (also
  `--jill-look -1`).  It draws a pack's `sprites/player.<ext>` strip (entry 8
  frames) or its normal `kind_0` / `shape_8_*` overrides, so a texture pack can
  replace Jill's art without touching the game files.  Added manifest
  `[sprites] player = ...`, `ExternalVisuals::player_shape()`, and a shared
  `add_entry_sheet()` helper.  Headless check: a 75-frame magenta/white strip
  changed exactly the 16x32 player rectangle and nothing else.
* Nuked OPL3 is now the only default synthesis core; the hand-written
  `port/src/opl2.*` core was removed from every build and `--opl-core` /
  `[audio] opl_core` are now `0` = Nuked, `1` = fmopl.  Both `--dump-music`
  paths render.  Updated the sound docs, notices and Xbox defaults.
* Removed the modern menu layout and the message-box style selector.  The
  front end always uses the classic frame/shape-7 window, in-game menus always
  overlay the live game, and captions/pages/reminders always use the original
  classic windows.  Removed the obsolete MENUS rows (STYLE, BACKGROUND), the
  VIDEO MESSAGE BOX row, their ini keys and CLI switches.
* Builds: `build.cmd` and `tools\build_demo_studio.cmd` both clean.  No
  in-game run was performed (user controls interactive testing).

## 2026-11-20 (importer: optional palette adaptation)

* Diagnosed a user report where a purple/red gradient tile imported as
  grey/brown: tile 0x168 points at shape entry 23, a 4bpp entry whose 16-colour
  table contains only greens/browns/greys, so the importer correctly snapped
  the new art to those colours.  The global VGA palette does contain magentas.
* Added `jill_import.adapt_entry_palette()` plus an `adapt_palette` option on
  `replace_sha_frame()`, `tile_from_png()` and `sha_entry_from_folder()` (and
  `--adapt-palette` on the matching CLI commands).  It median-cuts the source
  image to the entry's colour count and snaps each colour to the nearest
  global VGA index, so genuinely new hues can be imported.  It is opt-in
  because the per-entry palette is shared by every frame of that entry.
* The editor exposes this as an **Adapt palette** checkbox on the Sprites and
  Tiles tabs.  Testing on the real map showed why the tile version must not
  recolour in place: entry 23 has 65 frames and all 16 palette slots are used,
  so every ground tile changed colour.  Tile adaptation now uses the new
  `clone_tile_to_entry()`: the entry is copied into a free shape slot 0..63,
  the palette is adapted on the copy, and the tile's DMA record is repointed;
  both `.sha` and `.dma` are written.  Verified on the real files: entry 23's
  palette and its other frames are byte-identical afterwards, the new tile is
  magenta, and a no-free-slot case reports a clear error.  `sha-tile
  --adapt-palette` was removed; `tile-clone` is the supported CLI path.
* Follow-up after an original-game (non-port) test showed corruption: the
  adapted palette words had zeroed their low 16 bits.  The original engine's
  CGA/EGA paths read those bytes at shifts 0/4/8/12, while the port only uses
  bits 16..23, so JillEX looked right but the DOS game did not.  Palette words
  now preserve the original low 16 bits and only replace the VGA byte at
  bits 16..23.  Verified on the real files: low bytes identical to the
  original, cloned tile still magenta in the port.  Awaiting a retest in the
  original game.

## 2026-11-20 (jill editor: GUI asset editor)

* Added `jill_editor.py`, a standalone Tkinter editor built on the extractor's
  parsers/renderers and the importer's writers.  Tabs cover shape frames
  (preview, PNG replace, folder rebuild, font sheet), DMA tiles (preview, PNG
  export/import, paint-source selection), boards (tile-map painting, object,
  string and header editing, full-render import/export, `.jn1` save), VCL text
  pages (colour-aware line editor + preview + pages.json import) and VCL SFX
  (play, WAV export/import, folder import).  Game discovery and palette loading
  match `jill_extractor.py`; File > Open Files works without a game manifest.
* Added `jill_import.vcl_set_pages()` and `jill_import.save_board()` so the GUI
  reuses validated writers instead of duplicating archive encoding.  All GUI
  actions write a new file via Save dialogs and report failures as dialogs.
* Added `EDITOR_README.md` and `test_jill_editor.py` (headless API round-trip +
  withdrawn-root UI smoke test with synthetic archives).  Both release scripts
  now ship `jill_editor.py`, `EDITOR_README.md` and `IMPORT_README.md`;
  `TOOL_LOCATIONS.md` registers JILLEDITOR.
* Follow-up: added a **Replace Animation from Folder** action on Sprites.
  It uses `jill_import.sha_entry_from_folder()` (new API + `sha-entry-folder`
  CLI) to replace every frame of the selected entry from a folder of images,
  with natural filename sorting, palette-preserving re-encode and validation.
  The Text tab gained an explicit **Add Line** button (also Enter in the text
  box) so lines can be appended to an existing page while another line is
  selected.
* Both Windows release scripts (`make_release.cmd`, `make_release_32.cmd`) and
  the local `copy.cmd` now include `jill_editor.py`; the release scripts also
  ship `EDITOR_README.md` and `IMPORT_README.md`.  Existing release zips are
  stale until those scripts are re-run.
* Tests run: `test_jill_import.py` 4/4, `test_jill_editor.py` OK,
  `jill_editor.py --check` OK.  No game files or in-game test needed.

## 2026-11-20 (jill importer: arbitrary-input hardening)

* `jill_import.py` now validates every input before writing: file/dir existence
  and type, archive directory bounds (`.sha`/`.vcl`), image/WAV decoding, JSON
  metadata, entry/frame/slot/tile indexes, frame dimensions and payload sizes.
  Bad inputs raise `ImportFailure` with the file name and reason instead of raw
  `struct.error`/`IndexError` traces; the CLI prints `error: ...` with exit 1
  (`JILL_IMPORT_DEBUG=1` keeps the traceback).  GUI converters show the same
  messages via their existing handlers.
* Removed silent corruption paths: oversized entries now fail before the 16-bit
  size fields wrap, short raw frame payloads are rejected instead of padded,
  folder commands no longer treat an existing garbage reference as "no
  reference", and `pack` rejects a missing/identical source and writes
  non-ASCII names as UTF-8.
* Added `test_jill_import.py`: synthetic archives cover every valid command,
  garbage/corrupt inputs, size limits and CLI error reporting (4/4 checks pass;
  no game files needed).  Documented validation and error codes in
  `IMPORT_README.md`.

## 2026-11-20 (PS4 quit: blank-group flip before release; fault logging)

* The quit log reaches `startup: exiting (rc=0)` and the fault is after that,
  and the old teardown still showed `videoout: unregister failed 0x80290009`
  (RESOURCE_BUSY).  A buffer group cannot be unregistered while its buffer is
  the one being scanned out, which is exactly the case after the last
  `present()`: the display keeps showing that frame indefinitely.
* The videoout backend now allocates a fourth, never-drawn buffer and registers
  it as its own group.  On quit it flips to that spare buffer, unregisters the
  presentation group, then flips to the **system's** blank frame (buffer index
  `-1`) so the spare group is also off screen and can be unregistered.  Only
  then does it close the port, wait 100 ms and unmap/release the direct memory.
  A console run showed that unregistering only after the spare flip works for
  group 0 but leaves group 1 (the spare, on screen) busy with
  `0x80290009` — hence the second, system-blank flip.
* A first attempt (2026-11-20, previous entry) left the port and memory to
  process exit; the console still reported CE-34878-0, so the resources are now
  released in a display-safe order.
* New PS4 fault handlers write `fatal: signal N addr=... rip=... rsp=...` to
  `/data/JillEX/fatal.log` for SIGSEGV/SIGBUS/SIGILL/SIGFPE/SIGABRT/SIGSYS/
  SIGTRAP.  The console run with the spare-group release produced an empty
  `fatal.log`, so the remaining fault is not a signal in our process; the app
  reaches `_exit(0)` and is terminated inside the exit syscall (the console
  debugger parks on `mov rax,1; syscall` in a system module).
* Audio/shutdown traces: the native mixer waits one buffer before its heap
  buffers are freed, and `MusicPlayer::close`/`ps4_pad_shutdown` log their
  results (`audio: thread stopped`, `audio: device closed`, `pad: close`).
* Rebuilt `build_ps4.cmd` (only the known `stb_vorbis` warning).  Awaiting a
  console quit test; the startup log line
  `videoout: quit blanks the display before releasing` identifies this build.

## 2026-11-19 (Xbox: playable on hardware, stable at 480i/480p and 720p)

* The original-Xbox target now runs the game on a real console: HDD install,
  `default.xbe` from `JillEX-Xbox.zip`, Jill 1 playable with a **stable frame
  rate at 640x480, 480p (720x480) and 720p (1280x720)**, pad input, music and
  sound effects.  720p renders the 320x200 frame at 3x (960x600, pillarboxed).
* Bring-up chain (details in the Xbox entries below): trace output was silenced
  on Xbox and the XBE used NXDK's 64 KiB stack; `CreateFileA` needed `\`
  separators; SDL video needs `XVideoSetMode()` before `SDL_Init`; pdclib's
  `memcpy`/`memset` are byte loops (30 ms per presented frame); and the built-in
  OPL2 core burns ~half the CPU in `sin()`/`pow()` per sample.
* Final performance measures: framebuffer writes ~1121 MB/s (vs 640 MB/s for
  cached RAM), `present` ~4-7 ms at 640x480, audio mix ~1 ms per 21 ms buffer
  with Nuked OPL3, absolute-deadline frame pacer (no more 47 fps cap).
* Remaining console work is tracked in `port/xbox/README.md`: writable data
  root for saves, widescreen stretch option, Jill 2/3 / Onesimus / Noisemaker /
  text pages not yet exercised on hardware, and the unused SDL window surface.

## 2026-11-19 (morph fit test: jn1 demo 13–14 tick desync fixed)

* **Root cause of the jn1 demo's late frog→Jill morph found.** `1FAE:2E61`
  does not morph unconditionally: after resizing the body it probes
  `_cando(x & ~7, y, 1)` at the feet-planted y (`old_bottom - new_h`) first,
  then at the unchanged y, and only applies the morph if one probe fits.  The
  port skipped both probes, so the frog morphed the instant it touched the
  2.dem return token at `(208,928)` while still inside the rock, 13–14 ticks
  before the original.  `Engine::morph_player` now mirrors the two probes and
  applies the successful probe's y.  No `*.mac` file was touched.
* jn1 demo route now matches the video: frog hops out to `(192,924)` and the
  morph happens at **tick 3588** at `(200,912)` (video: tick 3588); Jill walks
  to the chain, jumps at 3628 from x=408, clears level 103 at **tick 4059**
  (`t = 256.3 s`) with **Level 103 Score 229**, then the intro credits run to
  tick 4299 (score 329).
* A/B regression check against the pre-fix build: `end1.mac` (board 50 level
  15, 800 ticks) and `jn2dem1.mac` (1000 ticks) are trace-identical, so the
  fit test only changes the blocked-token case.
* The earlier "frog is stuck at x=224" video reading was wrong — the camera
  follows the frog, so its world x is 224 → 200 → 192 over 3573–3586, exactly
  the port's trace; only the morph timing was off.
* Docs: `_trymove` order corrected (full → y-only → x-only, mask adds the
  fall bit), morph probe documented in `jill1-kind-handlers.md`,
  `port-save-splash-cheats.md` and the new `--dump-objects` studio option in
  `demo-studio.md`.
* Release zips rebuilt (x64 + x86).  `jill.exe` and the studio produce
  byte-identical frames for jn1 ticks 3588 and 4059, so the shipped binaries
  contain the fix.
* **Jill 2 demo re-check (TODO).** The 2026-11-16 10-46-29 video is the Jill 2
  attract demos.  Its score digit changes 0 → 12 at 18.95 s, which pins tick 0
  to ≈3.19 s (the port's pickup is tick 287), and the health bar steps at
  8.25 / 12.25 / 14.75 s → ticks 92 / 165 / 210, matching the port's 89 / 164 /
  210.  So the macro timing and the early logic are in sync, and the port's
  existing camera is the one the demo was recorded with.
  A faithful version of the `1FAE:000D` vertical dead zone was tried
  (camera follows the player's own step while inside `[y-0x50, y-0x20]`,
  snaps to the edge otherwise; start at the board origin) — it moved the
  framing closer to the video but shifted the health drops to 143 / 239 and
  made jn1 diverge (0.dem loop), so it was reverted.  Recorded playback keeps
  the verified snap camera.
  The remaining jn2 difference is local: at tick 342 the recorded `b1` press
  makes the port's Jill jump under a low ceiling (state 2, 2-tick hang, then
  `dy = 0`) and she falls from y=800 at tick 346; the video's Jill stays on
  the ledge until ≈350.  The port then clips the kind-22 frog at (1320,822)
  at tick 350 (2-px AABB overlap) and loses health; the original misses it.
  Everything is deterministic (no RNG in the frog handler), so the open
  question is the exact jump/ceiling result for a blocked up-move in the
  Jill 2 player handler.
* Follow-up: the player's state-0 jump and state-2 ceiling code in
  `1FC1:01E0` were disassembled and match the port exactly (the jump has no
  headroom check; the ceiling snap is `row = (y-1) & ~0xF`, `dy = 0` only
  when the snap move fails).  The real bug found there was in the shared
  BUNNY/FROG handler (`1644:36EE`): gravity is `dy += 1; if (dy > 0x0C)
  dy = 0x0A`, not a cap at 10 — fixed in the port (the hop can reach 11-12
  px/tick).  That changes jn2's later hits (health 2 at 439, 1 at 520) but
  not the tick-350 frog landing.
  The frog only starts moving when it enters the cull margin: it sits at
  `(1344,870)` with `p13 = 0` until ≈tick 160 and then hops, so its phase is
  set by the camera rect.  The port's vertical camera is still ≈25-30 px away
  from the video's in the mid-demo window, which shifts that phase (the
  frog lands on Jill at 350).  A DOSBox save-state / memory dump of
  `cam_y` and the frog's `p13` at a few ticks would settle the remaining
  camera trajectory faster than the 30 fps video allows.
* **jn2dem2: star pickups fixed.** The 10-px macro collectible slack made the
  first jump grab all three spinning-blade stars (`p13 = 8`) at
  `(128,192)`, `(144,176)`, `(160,192)`.  The recording's boxes only touch
  the left/middle pair (ticks 126/128) and leave the third for the second
  pass upward (tick 180), which is what the video (10-46-29, 58.75 s+)
  shows.  The slack is gone: hit tests are plain AABB again.  Verified:
  jn1 is unchanged (the intro feathers now register on their real ticks,
  112/114 and 483, instead of 2 ticks early; morph 3588, level card 4059 and
  229→329 all stand), `end1` and `jn2dem1` traces are byte-identical.
  Remaining in jn2dem2: from ≈tick 498 Jill stands at `(592,208)` while the
  health drains to 0 and the level restarts repeatedly, whereas the video
  keeps progressing — a separate, later divergence.
* jn2dem2 T492/R67 follow-up (damage burst on landing).  The kind 7 DEMON at
  `(582,192)` is boxed in by the `BOULD1` tile at `(608,224)`, so its
  `_justmove` fails every tick and `1644:2974`'s "blocked" branch spits a
  FIREBALL every tick; each new fireball hits the landing Jill (498 demon
  touch, 511–515 fireballs, health 0 at 514).  The video's health bar drops
  only about every 13 ticks (≈519/536/569/583), so the original's demon is
  not cornered there — an upstream entity-phase divergence, not a hit
  cooldown.  Two real bugs found on the way and fixed:
  - `player_kill` OR-ed `0xC000` into the *player object's* flags.  `26F4:05E7`
    raises the HUD damage flash (`DGROUP:0x83D0` = board header+0x2A) instead;
    the object bits mean "off-screen/blocked" and the update pass stamped the
    player's cells into the tile map, freezing her and making adjacent
    enemies look blocked.  The flash itself is still not rendered.
  - The per-frame `0xC000` map/object marks are now cleared at the start of
    `Engine::tick()` (they were cleared at the end of `render()`, so headless
    runs accumulated them across ticks).

## 2026-11-18 (exact 18.2065 Hz tick; deterministic fish bubbles)

* **Tick rate is now the exact original ratio.** `1581:0236` installs the
  custom int 1Ch handler (`1530:0440`).  Without the SB FM driver it programmes
  PIT channel 0 with divisor `65536/64 = 1024` and bumps `DGROUP:0x3E28` every
  64th interrupt; with the driver the frequency argument is 0, which the
  routine turns into divisor 65536 and divider 1.  Both reduce to exactly one
  8253 reload: `1,193,182 / 65,536 = 18.206510... Hz` (the `/18` elsewhere is
  just tick-count-to-seconds display math).  `port/src/timing.h` now derives
  `kLogicHz`/`kLogicStep` from that ratio and both the game loop (`app.cpp`)
  and the demo studio use it instead of the truncated `18.2065` literal.
* Macro playback still consumes one record per `Engine::tick()`, so the replay
  sequence is independent of the wall-clock rate; this change removes the
  last 0.7 ppm pacing error and keeps the game and studio in lockstep.
* Fish form bubbles called `std::rand()` directly, so a fish-form replay used
  the C library's default-seeded generator instead of the original Turbo C
  LCG.  They now use `Engine::rnd(4)`, the generator macro playback already
  seeds to 12345, matching the original `rand() & 3` bubble cadence and its
  effect on the object list.

## 2026-11-18 (morph tokens persist; 2.dem star pickup)

* **Transformation tokens are not pickups.** `1a41:000f` jumps from the morph
  call (`1a41:00b4`) straight to the tail at `0146`, skipping
  `sound(3,0x19)`, `_addinv` **and** `_killobj`.  `Engine::on_hit` case 28 now
  morphs and returns without adding the item or killing the object, so the
  fish/spider/bird/frog and `"FOOF!"` tokens stay on the board and can be
  touched again to re-activate the form.
* `Engine::morph_player` now returns early when the player already has the
  requested kind (`1FAE:005E`), otherwise the persisting `"FOOF!"` token under
  2.dem's spawn point re-ran the morph every frame and pinned the player there.
* 2.dem's spinning blade (p13 = 8, the star-shaped pickup on the water shaft)
  was 4-10 px outside the frog's box on the recorded path, so the demo never
  collected it.  Collectible tokens (p13 1/2/3/8/9/10) now get the same 10-px
  macro-playback slack that the ending's JILL icon already used;
  transformation tokens are excluded so the frog token cannot fire early.
* `jn1demo.mac` still runs intro -> 0.dem -> intro -> 1.dem -> intro -> 2.dem
  with no deaths (end 1312,878) and `end1.mac` still morphs at 232, reverts at
  494/495.

## 2026-11-18 (extractor: Xargon planar tile decode)

* User-supplied screenshots (shape viewer vs. the real DOS game) showed stage
  tiles/objects rendered as scrambled streaks although sprites were fine.
  Cause: `xlate_table()`'s `shm_blflag` 4-plane shuffle is undone by
  `ldrawsh_vga()` at draw time, so the net on-screen pixels are the archive's
  original byte order.  `XargonGraphicsFile` now keeps frame payloads linear
  instead of applying the intermediate plane reorder (the earlier reorder
  happened to look plausible on some banks, which is why it slipped through).
* Indices 250/251 are Xargon's runtime sky colour (`setcolor(250, ...)` in
  XARGON.C / X_EDITOR.C), not fixed palette colours.  Both are now treated as
  transparent in the alpha mask, and `frame_indices()` folds them into 0 so
  the palettised exporters skip them as well.
* Verified against the supplied red-flower frame (bank 14, frame 20): the
  preview now shows the flower top over a transparent background, and the
  full `BOARD_01.XR1` preview shows flowers, waterfalls and clouds matching
  the game screenshot.  Player/enemy sprites unchanged.

## 2026-11-18 (extractor: Xargon shape preview fix)

* `XargonGraphicsFile` now marks 8-bit banks with the identity-palette flag
  that Jill's shape decoder expects (Xargon's `numcolorbits==8` uses
  `init8bit()` regardless of `shm_fontf`); previously those banks rendered
  all black in the shape viewer.  Frames also carry an alpha mask for source
  colour 0 (Xargon's `cmtab[0][0]=255` transparency), so sprites no longer
  appear inside a solid black box.  Original flags remain in `entry.raw_flags`.
* Verified by rendering banks 6 (player sprites), 8 (tiles) and the font bank
  to PNGs with a contrasting background, plus a GUI `draw_shape` pass; board
  previews are unchanged.

## 2026-11-18 (extractor: Xargon format support)

* Added Xargon volume detection to `jill_extractor.discover_games()` via
  `jill.ini [paths] xargon = D:/Emu/DOS/C/Xargon`.  Detected from data files
  (not `games/*.game`) so the C++ port, which has no Xargon engine profile,
  does not offer broken games.  Volumes 1-3 appear as `Xargon 1/2/3` in the
  Game menu and reuse the existing Boards/Shapes/Sounds/Config tabs.
* New parsers in `jill_extractor.py`: `XargonBoardFile` (raw 128x64 tile map,
  31-byte objects, 96-byte `pltype` + `cnt_fruit` trailer instead of Jill's
  70-byte header, plus `save()`), `XargonTilesFile` (900 defaults + XOR flag
  overlays), `XargonGraphicsFile` (12-byte shape sub-header, `colortab`
  translation, `shm_blflag` 4-plane decode, embedded 64x12 DAC palette from
  bank 5), `XargonAudioFile` (VCL header + 128 SB macros), `XargonConfigFile`,
  `XargonMacroFile` (`[bits][values][varint dt]`), `XargonEPCFile`, and
  Xargon kind names from `X_OBJ.DEF`.
* CMF tracks render via the built port using a Jill game as host and an
  absolute `--music-file` path; batch export keeps volumes distinct with
  `SONG_n_XRn.wav` names.  `extract_all.py` was updated to use the Xargon
  parsers for `--game xargon1/2/3`.
* Verified headlessly: parsers, GUI tab population, board rendering (rendered
  `BOARD_01.XR1` to PNG), SFX/text/macro config views, CMF render/play and
  CLI extraction.  Audio output itself still needs a user pass.

## 2026-11-18 (jn1demo frog leap: macro buttons are not edge-latched)

* From the studio recording around T3153/R389: the 2.dem frog section holds
  `b1` for 19–169 ticks (`R386–R388`, `R391–R401`, `R402–R411`, `R414–R428`).
  `Engine::apply_macro_input` was still gating recorded buttons with
  `latch1_`/`latch2_`, so each hold became a single-tick press.  The original
  macro player (`1415:093B`) feeds `3C7A`/`3C7E` directly and never runs the
  normal input poll, so the latches do not apply.  Macro playback now sets
  `in_.btn1`/`in_.btn2` straight from the record; the frog holds the leap
  (`dy = -0x0C`) instead of dropping back into the water.
* `jn1demo.mac` still reaches intro → 0.dem → intro → 1.dem → intro → 2.dem
  with no deaths (final frame 1312,878), and `end1.mac` still morphs at 494 and
  reverts at 495.  Normal-Jill recordings have short `b1` taps, so their route
  is unchanged.

## 2026-11-18 (extractor sound playback)

* `jill_extractor.py`: listbox entries are written as `SFX  1: ...` / `Page  3: ...`
  (zero-padded id plus trailing colon), but the play/save/select/export handlers
  did `int(text.split()[1])`, which raised `ValueError: invalid literal for int()
  with base 10: '1:'`.  New `_entry_index()` helper strips the colon and is used
  by every handler, including the WAV-import slot lookup (which had silently
  returned `None`).
* Music entries (`Music: funky.ddt`, `Title music: ...`) are file-backed, not
  `Music N` VCL indexes; `_entry_filename()` now routes them to the CMF renderer,
  and playback uses a fresh temp WAV each time instead of a fixed preview name
  (reusing it made repeat plays silent because winsound held the file open).
  Awaiting user audio verification in the GUI.

## 2026-11-19 (Xbox: display modes, perf instrumentation)

* **Display works on the console** (engine, board and video init all reach the
  main loop).  The Xbox now offers real output modes instead of only 640x480:
  `VIDEO > RESOLUTION` lists `DESKTOP` (640x480), `480p (640x480)`,
  `480p (720x480)` and `720p (1280x720)`; `[video] resolution` stores the
  choice.  SDL's Xbox driver cannot switch modes, so the mode is set through
  NXDK's `XVideoSetMode()` (before `SDL_Init` for the start-up mode and from
  `Screen::apply()` for menu changes) and the window surface is recreated to
  match the framebuffer.  Modes the AV pack / dashboard does not allow fall
  back to 640x480 and are traced.
* `window_width/height()` return the mode size on Xbox (the window must fit the
  framebuffer; SDL asserts on it) instead of a scale of 320x200.
* The PS4-only debug overlay and `perf:` logging are now enabled for Xbox:
  `[debug] overlay = 1` draws the overlay and the frame/tick/draw/present/audio
  costs are written to `jill.log` every 5 seconds unconditionally (no CLI on
  the console).  The SDL audio callback measures the mixer cost on Xbox too.
* The frame pacer no longer busy-spins on Xbox (single core: the spin starved
  the SDL audio thread each frame).
* Presentation no longer goes through the SDL window surface: the scaled frame
  is written straight into `XVideoGetFB()` (one conversion instead of two, and
  no `SDL_ConvertPixels` pass into write-combined memory), and only the
  letterbox bars are cleared each frame.
* Reported frame rate is ~20 fps; the perf line will show whether that is
  engine draw, presentation or the pacer's sleep.

### Xbox follow-up: on-screen debug text, pacing, presentation

* The debug channel is drawn *on top of the TV picture* by debug BIOSes, and
  `trace()` was calling `debugPrint` — that was the "text console over the
  screen".  The Xbox build now logs to `jill.log` only.
* pdclib's `printf` has no floating-point support (`%f` prints nothing, which
  is why the first `perf:` lines were empty), so the perf line now logs whole
  milliseconds with `%u`.
* Frame pacing uses an **absolute deadline** on Xbox: `Sleep()` overshoots its
  request by several milliseconds there, and sleeping "the remainder" every
  frame let that error accumulate (47 fps cap).  The pacer sleeps to a fixed
  schedule, spins the last ~1 ms, and resyncs after a long frame instead of
  running several frames back to back.
* `present_xbox` builds one integer-scaled row and `memcpy`s it into the
  `scale` framebuffer rows (fewer per-pixel stores into write-combined memory).
* Changing `RESOLUTION` at runtime no longer recreates the SDL window (the Xbox
  driver refuses a second window and the frame goes straight to the
  framebuffer anyway); only `XVideoSetMode` is called.

### Xbox perf: pdclib's memcpy/memset were byte loops

First `perf:` numbers (after fixing the integer formatting) showed
`fps=25 frame=41 tick=0 draw=5 present=35 audio=10/21 ms` — presentation was
the bottleneck, at ~40 MB/s.

Root cause: NXDK's pdclib ships reference `memcpy`/`memset` that copy **one
byte at a time** (`while (n--) *d++ = *s++;`).  Every frame writes ~1 MB into
the video buffer, so the byte loop alone was ~30 ms on the 733 MHz CPU; it also
slowed SDL, libc++ and the audio mixer.

* New `port/xbox/xbox_mem.c` provides `memcpy`, `memset` and `memmove` built on
  `rep movs`/`rep stos` (linked directly, so pdclib's archive members are not
  pulled in; compiled with `-fno-builtin` so clang cannot rewrite them into
  calls to themselves).
* `ScreenSDL::init()` runs a one-off bandwidth check and logs
  `xbox fb bench: cached=.. MB/s framebuffer=.. MB/s` so the memory path can be
  measured on the console.
* The letterbox bars are cleared only when the output geometry changes instead
  of every frame.

### Xbox perf: built-in OPL2 core was half the CPU

After the memcpy fix the console held 67-79 fps in quiet scenes but dropped to
~24 fps where a lot is on screen, with `present` and `audio` both inflated.
The built-in OPL2 core (`port/src/opl2.cpp`) synthesises with doubles and calls
`std::sin()`/`std::pow()` **per operator per sample**: 9 channels at 48 kHz is
~37 transcendental calls per sample, i.e. 10-19 ms per 21 ms audio buffer
(half of the 733 MHz core).  The Xbox build therefore defaults to the integer
**Nuked OPL3** core (`[audio] opl_core = 1`, selectable in OPTIONS > SOUND >
OPL CORE), which measures ~1 ms per buffer and is also the more accurate
emulation.  Desktop/PS4/Vita keep the built-in default.

The perf line now also splits the draw phase:

```
perf: fps=.. frame=.. tick=.. draw=.. render=.. conv=.. present=.. audio=../.. ms
```

`render` is `engine.render()` plus the menu/overlay drawing, `conv` is the
palette -> ARGB conversion + display filter (the remainder of `draw`).

## 2026-11-18 (original Xbox target: first compiling build)

* New `port/xbox/` CMake project builds the shared `port/src` engine with NXDK
  and its SDL2 2.0.9 port, then runs `cxbe` to produce
  `port\xbox\build\default.xbe` (1.66 MB).  `build_xbox.cmd` also stages
  `jill.ini`, `games/*.game` and `profiles/*` into `build\data` and
  `JillEX-Xbox.zip`.  Desktop (`build.cmd`) rebuilds unchanged.
* NXDK's shipped `share/toolchain-nxdk.cmake` cannot be used as-is on Windows
  (it points CMake at the extension-less `bin/nxdk-cc`/`bin/nxdk-link`), so
  `port/xbox/nxdk.toolchain.cmake` overrides the launchers with their `.cmd`
  wrappers, and the project builds libc++ and SDL2 from the SDK's checked-out
  sources (a source checkout has no prebuilt `libc++.lib`/`libSDL2.lib`, and
  NXDK's makefiles refuse to run outside a MinGW shell).
* Portability work in shared code:
  * New `jill_fs` facade (`port/src/platform_fs.{h,cpp}`): desktop/PS4/Vita/
    Android alias `std::filesystem`, Xbox uses a Win32 implementation
    (`FindFirstFileA`/`GetFileAttributesA`/`CreateDirectoryA`) because
    libc++'s `<filesystem>` is POSIX-only and not compiled by NXDK.
    `external.cpp`, `game.cpp`, `profile.cpp`, `platform_paths.cpp` and
    `main.cpp` now use it.
  * New `port/src/platform_except.h`: the three defensive try/catch blocks
    compile as plain blocks on Xbox (no C++ unwinder, `-fno-exceptions`).
  * `port/xbox/xbox_compat.h` (force-included) restores `std::printf` (NXDK's
    libc++ sets `_LIBCPP_HAS_NO_STDOUT`), declares the `<locale>` helpers
    `wcsnrtombs`/`mbsnrtowcs`, and pins pdclib's Annex K guard macro.
  * `port/xbox/xbox_runtime.cpp` provides `std::terminate`,
    `uncaught_exception(s)` and `__call_once`; NXDK's `libxboxrt` EH object
    needs MSVC EH machinery, so libc++'s `exception`, `future`, `thread`,
    `condition_variable`, `mutex` and `random` units are excluded (unused).
  * Windows-only backends guarded: MIDI (`winmm`) and Media Foundation audio
    are disabled on Xbox; `STBI_NO_STDIO`/`DR_MP3_NO_STDIO` (memory decoding,
    no `fopen_s`); `MINIZ_NO_STDIO`/`MINIZ_NO_TIME`; `STBI_NO_SIMD` (Pentium
    III, no SSE2).
  * `screen_sdl.cpp`: Xbox shares the PS4 direct window-surface presentation
    path (the SDL2 Xbox port has no accelerated renderer); the 320x200 frame
    is integer scaled into 640x480.
  * `main.cpp`: no `chdir` on console (data root is absolute), local
    `--music-seconds` parser (pdclib declares but does not implement
    `atof`/`strtod`).
* Xbox data root is the first of `D:\`, `E:\JillEX`, `E:\Apps\Jill-EX`,
  `E:\Apps\JillEX`, `E:\Games\Jill-EX`, `E:\Games\JillEX` that contains a
  `jill.ini` (`D:\` = disc root).  `jill.log`, saves and high scores still
  need a writable root.
* **Not run on hardware or in Cxbx yet.**  Bring-up checklist in
  `port/xbox/README.md`.

### Xbox first hardware attempt (same day)

First console boot returned to the dashboard immediately and `jill.log` stayed
at 0 bytes.  Two causes:

* `jill_paths::trace()` only printed on desktop when `JILL_TRACE` was set; the
  guard was `!PS4 && !VITA && !ANDROID && !XBOX`, so the Xbox build never wrote
  a single diagnostic line.  `JILL_XBOX` is now excluded from that guard and
  `trace()` writes to the debug channel (`debugPrint`) plus `jill.log`.
* The XBE was linked with NXDK's default 64 KiB stack (`-stack:65536`), and
  NXDK's CRT runs `main()` on a thread that takes its stack size from the XBE
  header.  The engine needs far more; the overflow raises
  `EXCEPTION_STACK_OVERFLOW` and the console drops to the dashboard.  The Xbox
  target now links `-stack:1048576` (verified in the PE header).

Startup diagnostics reworked for the Xbox:

* `main()` opens `jill.log` before anything else and no longer calls
  `freopen()`/`setvbuf()` on stdout/stderr: pdclib's standard streams are
  pseudo streams whose handle is `INVALID_HANDLE_VALUE`.
* `jill_paths::log_open()` now reports failure, and the Xbox build falls back
  to `E:\JillEX\`, `E:\` and `C:\` when the XBE sits on a read-only disc.
* The data root is the folder the XBE was launched from (`XeImageFileName`),
  so an HDD folder, a disc or an FTP deployment all work; the fixed
  `D:\`/`E:\...` list is only a fallback.
* `build_xbox.cmd` now stages `default.xbe` inside the install bundle, so
  `JillEX-Xbox.zip` is the complete deployable folder (XBE + `jill.ini`,
  `games/`, `profiles/`, `external/`, README).

Second boot got as far as `startup: engine init game=jill1 data=D:\jillt` and
then exited.  Cause: `join()` builds `D:\jillt/jill.dma`, and NXDK's
`CreateFileA()` hands the name straight to `NtCreateFile`, whose object manager
only treats `\` as a separator (`/` works on desktop Windows but not here), so
every `tiles`/`shapes`/page/board/asset open failed.  `jill.ini` and pack
manifests use `/` too (`palette = profiles/onesimus.pal`).  Fixes:

* New `port/xbox/xbox_fs_glue.cpp` replaces pdclib's `_PDCLIB_open` glue (the
  single funnel behind `fopen`, `std::ifstream` and `std::ofstream`) with the
  same implementation plus `/` -> `\` normalisation.
* The `jill_fs` Win32 helpers normalise names for
  `GetFileAttributesA`/`FindFirstFileA`/`CreateDirectoryA` as well.
* `Engine::init` now traces each load step and the failure reason, so a missing
  data file is visible in `jill.log` instead of vanishing into the pseudo
  stderr.
* External asset scanning tolerates a read-only root (runs without packs
  instead of failing).

Third boot reached `startup: SDL_Init subsystems=0x6220` and stopped: SDL's
Xbox video driver (`XBOX_VideoInit`) reads the current mode with
`XVideoGetMode()` and adds it as the display, but NXDK leaves that mode 0x0
until the application calls `XVideoSetMode()` (the NXDK SDL sample does it
before `SDL_Init`), so video init failed.  The Xbox path now:

* calls `XVideoSetMode(640, 480, 32, REFRESH_DEFAULT)` before SDL init and
  traces the resulting `XVideoGetMode()`,
* initialises SDL one subsystem at a time (`VIDEO`, `JOYSTICK`,
  `GAMECONTROLLER`, `EVENTS`) and traces each result, so a driver failure on
  the console is visible in `jill.log` instead of vanishing into stderr.

## 2026-11-18 (Android: shared /sdcard/JillEX data folder)

* The Android data root moved from app-private storage
  (`/sdcard/Android/data/org.jillex/files`) to `/sdcard/JillEX`, so game data,
  saves and `jill.ini` are visible to any file manager and `adb push` does not
  depend on the package name.  `platform_paths.cpp` now returns that constant.
* Because the folder is outside the app sandbox and targetSdk is 35, the app
  requests "All files access" (`MANAGE_EXTERNAL_STORAGE`) on API 30+; on API
  24-29 it requests `WRITE_EXTERNAL_STORAGE` (plus
  `requestLegacyExternalStorage` for Android 10).  `JillEXActivity` shows a
  GRANT ACCESS screen before SDL starts, rechecks in `onResume` and recreates
  when the grant arrives.  adb alternative:
  `adb shell appops set org.jillex MANAGE_EXTERNAL_STORAGE allow`.
* Asset install, the startup-error screen, `jill.log`, `README.txt`,
  `build_android.cmd` and the docs now use `/sdcard/JillEX`.  Existing installs
  must move `jillt/`, `ONESIM/` and saves from the old app-data folder once.
* Startup failures now name the reason: `tiles.cpp`/`shapes.cpp` append
  `strerror(errno)` (`No such file or directory` vs `Permission denied`), and
  `Engine::init` logs the contents of the game data directory when a required
  `.dma`/`.sha` cannot be opened.  Tile and shape paths also go through
  `jill_paths::resolve_file()`, so an uppercase `JILL.DMA` works on Android's
  case-sensitive storage.

## 2026-11-18 (Android: startup diagnostics and missing-data screen)

* Fire tablet launches returned straight to the launcher.  The S25 Ultra log
  showed why: `engine.init()` runs before the menu and returns 2 when
  `jillt/jill.dma` is missing, and SDLActivity then `finish()`es the activity.
  The Fire tablet had a complete `jillt/` and is now confirmed working; the
  S25 had the folder but not its files.
* The data-folder pre-check was not enough (an empty `jillt/` passes it).
  `main.cpp` now calls `JillEXActivity.showStartupError(rc)` through JNI when
  `SDL_main` returns non-zero.  The activity keeps the screen up (the SDL
  thread's automatic `finish()` is ignored when it runs off the UI thread),
  prints the failure, the `adb push` commands and the tail of `jill.log`, and
  offers RETRY (`recreate()`); Back still exits normally.
* First attempt at that screen still closed: SDL calls `finish()` from the SDL
  thread before the posted UI runnable ran, so the instance flag was not set
  yet.  `showStartupError()` now sets a `static volatile` pending flag
  synchronously before posting; `finish()` checks that flag and only ignores
  calls from non-UI threads.
* Diagnostics: the activity logs device/model/ABI, the data folder and the
  SDL-ready state; `main.cpp` installs SIGSEGV/BUS/ILL/FPE/ABRT handlers that
  report `fatal: native signal N` under `JillEX`, logs
  `startup: native main entered` and `startup: SDL_main returning rc=N`.
  Verify with `adb logcat -d -s JillEX SDL AndroidRuntime libc`.

## 2026-11-18 (Android: universal ABIs and 16 KB page alignment)

* Device feedback fixed two install failures:
  * Fire tablet reported `INSTALL_FAILED_NO_MATCHING_ABIS`: the APK was
    arm64-only.  The build is now universal — `arm64-v8a`, `armeabi-v7a`,
    `x86_64`, `x86` (APK ~6.0 MB).  A single-ABI build is still available with
    `gradlew.bat assembleDebug -PjillAbis=arm64-v8a`.
  * S25 Ultra rejected the native libraries with "16kb load segment
    alignment".  `add_link_options(-Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384)`
    applies to SDL2 and libmain; every LOAD segment in all four ABIs now
    reports `0x4000` alignment (`llvm-readelf -l`).
* `port/android/README.md`, `BUILDING.md` and the build script notes describe
  the ABI list and the 16 KB requirement.  APK is still copied to
  `JillEX-android.apk` (git-ignored).

## 2026-11-17 (Android target: first installable APK)

* New `port/android/` Gradle project builds the shared `port/src` engine for
  Android with the NDK 27 toolchain, SDK 35, minSdk 24 and `arm64-v8a`.  SDL2
  2.32.10 is compiled from the installed source tree (`JILL_SDL2_DIR`), so
  `libSDL2.so` and the copied `org.libsdl.app` Java glue are version-matched.
  `main.cpp` includes `SDL_main.h` on Android; `SDLActivity` resolves the
  exported `SDL_main` in `libmain.so`.
* `build_android.cmd` stages `jill.ini`, `games/*.game`, `profiles/*` and the
  data README into `app/src/main/assets/`, runs `assembleDebug`, and copies the
  result to `JillEX-android.apk` in the repository root (git-ignored).  First
  build is ~3 min (SDL2 + engine), incremental ~6 s.  APK is 1.62 MB and
  contains `libSDL2.so`, `libmain.so` and only the configuration assets.
* Data root is `SDL_AndroidGetExternalStoragePath()`
  (`/sdcard/Android/data/org.jillex/files`) with internal storage as fallback.
  `JillEXActivity` installs the bundled config on first launch without
  overwriting `jill.ini`; `trace()` now also writes to logcat (`JillEX`).
  Game data (`jillt/`, `ONESIM/`) is pushed with adb, as with the console
  builds.
* Touch controls (`port/src/touch.cpp`): D-pad, A=jump, X=throw, START, BACK.
  The overlay is drawn into the 320x200 ARGB frame and fingers are mapped back
  through the new `Screen::touch_to_frame()` (logical size, integer scale and
  letterbox aware).  Gameplay feeds `poll_input()` directly; menus receive
  synthetic `SDL_CONTROLLERBUTTONDOWN/UP` events, so the existing menu, pause,
  text-page and demo code is unchanged.  `screen_sdl.cpp` uses the real display
  mode, keeps landscape and skips desktop window resizing on Android.
* Not yet run on hardware.  Open items: on-device check of rendering, audio
  latency, suspend/resume and the touch zones; hi-score name entry and the
  Noisemaker QWERTY page still need a keyboard or a touch keypad
  (plan Milestone C).  Desktop, PS4 and Vita paths are unchanged apart from the
  shared `trace()` rewrite and the `touch_to_frame()` default no-op.

## 2026-11-17 (Vita build copies its artifacts to the repo root)

* `build_vita.cmd` now copies `jill.vpk` and `JillEX-Vita-data.zip` to the
  repository root after a successful build (matching `build_ps4.cmd`, which
  leaves the `.pkg` and data zip there).  The build-folder copies remain.

## 2026-11-17 (Windows release zips include the demo studio)

* `make_release.cmd` and `make_release_32.cmd` now always rebuild the matching
  demo studio (incremental) with `tools\build_demo_studio.cmd` (x64) or
  `tools\build_demo_studio32.cmd` (Win32), copy `jill_demo_studio.exe` into the
  staged bundle and ship `demo-studio.md` with the other docs.  Both
  `Jill-EX-release-x64.zip` and `Jill-EX-release-x86.zip` contain the studio;
  it shares the bundle's `SDL2.dll`, `games/`, `profiles/` and `jill.ini`.

## 2026-11-17 (demo studio: 32-bit Windows build)

* Added `tools/build_demo_studio32.cmd` (Visual Studio Win32 + `sdl2:x86-windows`)
  and `tools/run_demo_studio32.cmd`.  The 32-bit exe is written to
  `tools/demo_studio/build32/jill_demo_studio.exe`, so it never overwrites the
  standard x64 build in `tools/demo_studio/build/jill_demo_studio.exe`.
* The studio's CMake target now places its exe in the configured binary
  directory (`CMAKE_BINARY_DIR`) for every configuration, which keeps the two
  build trees independent without hard-coding the output folder.
* Verified both: `--selftest` passes on x86 (including the new-file seed) and
  the x86 headless frame at tick 120 is byte-identical to x64.  The x86 GUI
  opens with the 32-bit SDL2.dll deployed beside it.

## 2026-11-17 (Jill 2 demo: real RNG and macro input timing)

* `Engine::rnd` is now the game's own generator (`2CA6:0015`): a 32-bit Turbo C
  LCG, `seed = seed * 0x015A4E35 + 1`, result `(seed >> 16) & 0x7FFF`, and a
  macro start/stop reseeds it to `0x3039` (12345, `1415:05B2`).  The old xorshift
  placeholder made every random enemy/caption diverge from the recording.
* Recorded input is applied one physics update earlier and at most one record
  per frame, matching `1415:093B` (`delta <= be3a - base` after the frame
  counter increments).  The old loop applied all due records at tick T, one
  physics update late; the correction moves the recorded actions onto the same
  update as the original (board changes in `jn1demo.mac` now land at
  209/990/1149/2516/2722, still with no deaths).
* `jn2dem1.mac` (3.jn2) was checked against the original recording.  Up to the
  first apple (score 12, ~tick 286) the path and health line up.  The first
  mismatch is at the first vine detach (~R31/R32): the port lands on the blue
  ledge/black opening about 8-16 px left of the original, then takes a frog
  hit at ~tick 349 (health 4->3) that the original does not.  The original's
  detach carries further right into the black room; the port's R38 jump then
  starts from the higher ledge.  Not fixed yet; the remaining work is the vine
  detach y/ledge geometry (RNG and the camera were both ruled out).

## 2026-11-17 (demo studio: create new .mac files)

* `CTRL+N` (or `N` in the file picker, or `--new FILE`) creates a fresh `.mac`.
  The tool asks for a name, refuses to overwrite an existing file, appends
  `.mac` when there is no extension, and writes a valid seed stream: one tick-0
  record with mask `H V 1 2 K` and all values zero.  An empty file cannot load
  because the format has no header, so the seed record is required.
* The new file opens paused at tick 0 on the selected start state, ready for
  `INS` (add record at playhead) and the normal field editor; save with
  `CTRL+S`.  The file picker rescans immediately so the new name appears.
* `--new` uses the same path for headless/scripted creation; `--selftest` now
  verifies that the seed record round-trips (6 bytes for the neutral record).

## 2026-11-17 (recorded-demo parity: landing pose, macro camera, bird step)

* Jill 1's `jn1demo.mac` no longer desyncs in the 0.dem vine/jump section
  (the reported tick 555..833 window).  Three engine mismatches were
  responsible:
  * State 0 must jump straight to the shared tail while `p11 < 0` (the
    landing pose).  The port still ran the walk/idle block, so held Left/Right
    cut every landing short by 2-4 ticks.  With the landing pose left alone,
    the recorded jump at tick 825 now fires from the ground at `(1192,832)`
    instead of mid-air.  `1FAE:0469`.
  * A blocked walking step still advances the walk pose and keeps `dx`; it
    does not stop the player.  `1FAE:2069B`.
  * Starting to walk jumps directly to the shared tail, so the walk-start tick
    does not run the water/gap test or the jump branch.  `1FAE:04D8`.
* Macro playback now uses the original edge-triggered camera (`1FAE:000D`,
  view 15x11 tiles) instead of the centred follow.  The camera also defines
  the cull rectangle, so the old camera changed when off-screen VINECLIMB
  sparks updated and made one hit Jill at the top of the 0.dem vine.  With the
  original camera and the landing fix the demo reaches `1.dem` without dying
  (previously it stayed in 0.dem, and the first fix alone made it stall on an
  extra spark).
* State 2 ceiling handling now matches `26F4:210E`: a successful snap-up keeps
  `dy`; only a failed move clears it.
* JILLBIRD/JILLFROG first-step fix: the 8-px step requires the old `dx` to be
  non-zero, and the bird only updates `p0f` while moving.  `end1.mac` no longer
  overshoots the JILL/FOOF token: the phoenix reverts to Jill at tick 495 at
  `(1272,649)` instead of flying past it (the old code gained 4 px on every
  stop/restart).  The bird's move/step-up probes were corrected at the same
  time (`1FAE:298B`).
* Demo-studio additions: `--trace FILE` (per-tick player/camera/input CSV) and
  `--dump-map FILE` (live tile map), used for the comparisons above.
* The 1.dem return (`jn1demo.mac` 2415..2630) now lines up: the intro board's
  1.dem CHECKPT is floating 64 px above the floor, and the original's blocking
  "JILL ENTERS THE JUNGLE MAP" card lets the recorded walk resume from the
  floor.  The port free-runs the macro under the caption, so the recorded
  walk resumed mid-fall and lost the landing pose to the 52-tick Right hold,
  stopping at x=1024 instead of the feather column at x=1072.  Macro playback
  now drops the player onto the floor below a restored entrance CHECKPT;
  the intro walk reaches x=1080, collects both feathers (jump height rises
  from -24 to -28) and the demo continues into `2.dem` (level 103).  The
  whole `jn1demo.mac` now visits intro -> 0.dem -> intro -> 1.dem -> intro ->
  2.dem with no deaths.

## 2026-11-17 (demo studio: scalable UI text, options window, F2 fix)

* Fixed `F2` in the demo studio opening the `.mac` file list instead of the
  episode setup (a test-only replace had leaked into the key table).  `F2` is
  now SETUP and `F4` opens the new OPTIONS page; `--selftest` asserts both
  routes so this cannot regress silently.
* Added OPTIONS (`F4`): UI text scale (1x/2x/3x), forced game view scale
  (auto/1x/2x/3x), interpolation mode and classic HUD.  Text scale and game
  view persist in `demo_studio.ini` beside `jill.ini`.
* The whole layout now derives from the text scale: row height, column
  offsets, track height, editor, header/footer and all overlays.  Long info,
  footer, help, setup and file-picker lines wrap (or use a compact reference
  size) instead of running off the panel, and the window grows automatically
  when the bigger timeline needs the room.

## 2026-11-17 (Jill 1 board 50 entrance spawn)

* `50.jn1` is entered from `2.jn1`'s `"50.jn1"` CHECKPT with `p13 = 15`; its
  own entrance is the `&end1.mac` CHECKPT at tile `(8, 58)` (`x=128, y=928`).
* `place_at_checkpoint` no longer shifts the ending-macro entrance 16 px left.
  It now uses the original `26F4:04C7` rule for the macro branch as well
  (`x &= ~7`, `y -= 0x10`), so Jill spawns at `(128, 912)` between the trees
  instead of `(112, 912)` (tile 7, inside the left tree).
* Corrected the demo-studio example for `end1.mac` to `--board 50 --level 15`
  (level 50 has no matching CHECKPT and falls back to the raw player marker).

## 2026-11-17 (external .mac demo studio)

* New `tools/demo_studio/`: a standalone SDL2 viewer/editor for the recorded
  `*.mac` input scripts.  It links the real engine core from `port/src`
  (`engine`, `board`, `tiles`, `shapes`, `profile`, `game`, `external`, ...),
  not a copy, so the rendered 320x200 frame is the game's own output.  A
  headless frame after 120 and 2400 ticks of `jn1demo.mac` is byte-identical
  to `jill.exe --board intro --level 100 --macro jn1demo.mac --dump`.
* Layout: live game view plus a `LIVE STATE` panel (player/previous position,
  interpolated draw position, camera, objects, effective input) and an editable
  timeline.  Timeline shows each record's tick, delta, H/V, jump/throw, key and
  mask bits; the strip above it maps the whole recording and is click-to-seek.
* Editing: row/field selection, typed or +/- values, mask-bit toggles, insert at
  playhead, delete, 64-step undo, save and save-as.  Edits stay in memory until
  saved and only affect playback after `R` replays the run.
* Resizable layout: the game view integer-scales, the timeline rows use
  the game's own 6x8 font (glyph atlas built from the engine's shape bank), and
  the window reflows down to 960x600.
* `F5` cycles interpolation: smooth 60 Hz alpha blend, 60 Hz with alpha held at
  1, and one presented frame per original 18.2 Hz tick.  The live panel prints
  the blend factor and interpolated position so the effect is measurable.
* Seeking and stepping back rebuild the run by replaying the engine headlessly
  from tick 0, so scrubbing is exact and works in both directions.
* Macro parsing/serialising moved out of `Engine` into `port/src/macro.*`
  (`MacroRecord`, `macro_parse/load/encode/save`, `macro_state_at`).  `Engine`
  reads through it and gained `set_macro_records()`, `macro_records()`,
  `macro_index()` and `input()` for the tool.  Existing playback behaviour and
  diagnostics are unchanged; `--selftest` exercises parse -> edit -> encode ->
  parse on the loaded file.
* Startup state comes from `jill.ini` (`[paths]`, HUD and gameplay options), and
  the known demo macros auto-fill their board/level from the front-end demo
  table.  `--headless --ticks N --out FILE.ppm` writes frames without a window.
* Tool docs: `docs/demo-studio.md`; build with `tools\build_demo_studio.cmd`,
  run with `tools\run_demo_studio.cmd`.

## 2026-11-16 (Android toolchain provisioned; port plan written)

* Added `ANDROID_PORT_PLAN.md` — phased plan for an Android port reusing
  `port/src` through SDL2, modelled on `PS4_PORT_PLAN.md`.
* Installed the toolchain under `D:\Apps\AndroidSDK`: Temurin JDK 17.0.21,
  command-line tools 9862592, platform-tools, platform 35, build-tools 35.0.1,
  NDK 27.3.13750724, CMake 3.22.1, Gradle 8.14.7 and SDL2 2.32.10 source
  (including its `android-project/` Gradle template).
* Set user-scope `ANDROID_HOME`, `ANDROID_SDK_ROOT`, `JAVA_HOME`,
  `ANDROID_NDK_HOME`/`ANDROID_NDK_ROOT` and added the tool bin folders to PATH.
  Machine-level `java`/`cmake` entries still win for bare commands because user
  PATH comes after machine PATH; Gradle and sdkmanager use `JAVA_HOME` (17).
* No Android project scaffolding created yet; PC/PS4/Vita builds untouched.

## 2026-11-16 (board change keeps the incoming header, only the score carries)

* A board change now follows `FUN_22AB_1531`: the score is saved and restored
  around the load, while health, keys/gems and the inventory come from the
  incoming board's header (or from the stashed `temp` copy on `!`).  The old
  port carried the source board's whole header and then dropped everything but
  gems, so a level entered from the hub/demo kept the hub's health and lost its
  board-specific item.  Jill 1's demo now starts `0.dem` with the board's
  health 6 and its one item, as the original recording does, and the intro
  return restores the intro's health/items.
* `RETAIN ITEMS` still keeps the live header across the change (enhancement).
* The window title shows `M=<tick>/<duration>` during macro playback so a demo
  frame can be matched to a headless `--macro` run.

## 2026-11-16 (macro playback uses the original simulation viewport)

* The world simulation now always centres the camera and culls objects with
  the original **232x160** game rectangle while a macro plays, independent of
  the HUD style.  The modern HUD's 320x200 view used to feed `follow_camera()`
  and the update-pass cull, so off-screen objects (the VINECLIMB sparks) began
  updating at different ticks, the camera follow differed, and the recorded
  demo drifted: Jill clipped a spark / failed to climb a vine high enough and
  the rest of the recording could not continue.  With `--hud` the same demo
  already played much further, which is what identified the coupling.
* The macro is started before the entry-spawn camera placement (menu demo and
  headless `--macro` paths), so the first frame already uses the simulation
  view.
* A stashed board now remembers its name as well as its objects/map, so the
  demo's `!` return to the intro board restores `intro.jn1` instead of an
  empty board name.
* Confirmed against the original recording: the intro walk, the `0.dem` entry,
  the first tall vine climb (same screen position at the same tick) and the
  exit back to the intro now line up in both HUD styles.
* Still open: deeper in `0.dem` the port makes one extra vine grab the
  original does not (the macro holds Left, the grab's `p0f == 6` ignores
  horizontal input and the demo stalls ~150 ticks), so the level exit happens
  later than recorded.  That is the next pixel-parity item.

## 2026-11-16 (Jill 1 attract demo starts on the intro board)

* The Jill 1 demo is now one entry that starts on `intro.jn1` at level 100
  (the "Jill of the Jungle / Demo" card checkpoint) and plays the single
  `jn1demo.mac`.  The recording itself walks Jill through the three intro
  "Scenes From" CHECKPTs, which warp to `0.dem`, `1.dem` and `2.dem` (levels
  101/102/103).  Starting on `0.dem`/`2.dem`/`1.dem` skipped the first ~12 s
  of the input stream and could never line up.
* A level exit (`!`) now restores **any** stashed board, not just the world
  map: the intro board is cached when Jill enters a demo board, restored on
  return, and the active level stays the finished level (101/102/103) so the
  entrance she is standing on does not fire again.  The return still shows the
  world-map caption ("JILL ENTERS / THE / JUNGLE MAP."), as the original does.
* `board_level_` is set from the CHECKPT level that loaded the board (not the
  file name), so demo boards report 101/102/103 and board 3's level-5 exit
  still reports board level 3.
* Jill 1's VINECLIMB hit handler now matches `1A41:48F9`: the invulnerability
  bit gates the whole block, the push tries `x-8` then falls back to `x+8`,
  and there is no `p13 += 0x0F` stun (that is the Jill 2/3 variant).
* The 8-frame climb pull-up (an input-freezing cosmetic enhancement) is
  skipped during macro playback so recorded climbs keep the original timing.
* The early `0.dem` divergence at ~42 s (Jill clipping a VINECLIMB spark) was
  caused by the HUD-dependent simulation viewport; see the entry above.

## 2026-11-15 (demo/macro playback)

* Jill 1 demos now use their intro-CHECKPT level ids (`0.dem=101`,
  `2.dem=103`, `1.dem=102`) instead of 0; `load_board` accepts a level
  override applied before captions/spawn handling, so demos no longer start
  with the wrong level title/HUD id.
* `start_macro` clears the live input and button latches first, so a key held
  in the DEMO menu cannot leak into the first recorded frames.
* Macro parsing is bounds-checked: LEB128 ticks, all eight mask bits consumed,
  truncated/empty/oversized files reported, descending ticks and record caps
  diagnosed.  Mask bit 4 (last ASCII key) is stored and exposed through
  `Engine::macro_key()`/`take_macro_key()` for future scripts.
* `start_macro` now returns success; a missing/bad demo macro keeps the user in
  the menu instead of silently starting a dead demo.  Playback progress is
  available through `macro_tick()` / `macro_duration()`.
* Spawn fixes: a demo start runs the `26F4:04C7` checkpoint spawn (level id ->
  CHECKPT, state 4) instead of using the board's raw player marker, and a
  fresh map return with no stashed hub places Jill on the map entrance for the
  board she just left and consumes it.  Board 3's level-5 exit now returns to
  the `3.jn1` map entrance instead of the map's first object.
* Documented the Jill 1 demo level ids and the port's playback guarantees.

## 2026-11-15 (in-game .hudlayout HUDs)

* `*.hudlayout` files are now discovered in the mounted pack and the
  `external/autoload` layer.  `OPTIONS > HUD > LAYOUT` lists ORIGINAL plus each
  file stem; the choice is stored as `[hud] layout` and can be started with
  `--hud-layout NAME`.
* The engine draws a selected layout as the CLASSIC HUD: SHA-backed frame tiles
  through the running game's shape bank, plus boxes, text, the live health bar,
  pips, the inventory grid and the hint strip.  Reserved part names
  (`health-bar`, `level-value`, `score-value`, `inventory-grid`, `title`, ...)
  use live values; the layout's own `sha` paths and image-backed sheets are
  editor-only for now.
* A selected layout takes priority over a `hud/classic.png` panel.  The
  camera/view stays 80,16 232x160, so layouts should keep that `view` rect.
* Tested with the user's 320x190 layout in autoload via `--hud-layout classic`
  and `[hud] layout = classic`.

## 2026-11-15 (measured classic HUD layout)

* Rebuilt the classic frame to the measured 320x190 layout: 16-px top strip,
  8-px sides, 12-px bottom art plus a 2-px seam and a 10-px black hint strip.
  The viewport is now 80,16 232x160, with the panel/view divider at x=72 and a
  16-px divider band at y=80..96 (drawn under the boxes, clipped to x=80).
* Frame drawing follows the entry-3 pieces from the layout: top corners 2/3,
  top edge 4, top junction 10, bottom corners 5/7, bottom edge 6/8, vertical
  edges 1/0, divider-row edge 13, band 12/11 and fill 10.  The right border
  uses the notched frames 14/15 in the y=32 and y=144 rows.
* New in-game boxes: health 8,16 64x32 with a 28x8 bar at 44,28; level 8,48
  64x37; inventory 8,91 64x71; caption 8,162 64x14.  Labels/values moved to
  8,28 / 8,56 / 8,66 and SCORE right-aligns to x=72.
* Classic menu viewport uses the same frame/view; its caption moves to 166 and
  the menu window clamps to 176.  Title text and the FPS counter sit in the
  16-px top strip.
* `classic_guide_parts` and `docs/examples/classic-hud.hudlayout` now match the
  new engine coordinates.

## 2026-11-15 (external HUD layout tool)

* New `tools/hud_layout.py`: a standalone Tk/Pillow editor for HUD alignment.
  It overlays a reference screenshot (opacity/scale/offset), draws a
  resizeable snap grid, stamps tiles from any tileset image, and records
  labelled engine-part rectangles (`text`, `rect`, `box`, `pips`, `grid`,
  `image`, `note`).
* Writes `.hudlayout`, a UTF-8 INI-like format with pixel coordinates, grid
  metadata, tile runs (`sheet`, `row`, `cols`) and part sections.  Load, undo,
  PNG export, clipboard summary and `--summary`/`--render` headless modes.
* DMA tilesets: `Tilesets > DMA...` loads a game `.dma` plus the episode's
  `.sha` (and optional `.pal`), decodes through `jill_extractor`, lays the 320
  records out in record order and labels every palette cell with its tile id.
  A find-by-id box jumps to a tile; selecting shows its name and native size.
  DMA sheets are rebuilt when the layout is opened, runs gain a `tiles = ...`
  line, and the summary prints tile ids/names next to the pixel coordinates.
* HUD/UI shape sheets: `Tilesets > Shape...` loads one or more `.sha` entries
  (defaults 3, 4, 7, 14: classic frame, score digits, nine-slice window, item
  icons/pips) as separate `entry:frame` sheets, with a frame finder (`42` or
  `14:42`), auto or fixed cell size, and `frames = ...` on runs.  This covers
  the HUD/menu art that is not present in the `.dma` table.
* Classic guides are split: `Insert classic HUD guide` stays the engine-drawn
  path, while `Insert classic pack panel guide` covers the 64x84 external
  panel (HEALTH/LEVEL/SCORE labels, E14:42/43 pips at y=2, 14-px inventory
  step).  The old rough `pack-pips` marker is gone from the engine guide.
* `File > Insert classic HUD guide` adds the exact rectangles from
  `Engine::render_hud_classic`; the same starting layout is saved as
  `docs/examples/classic-hud.hudlayout`.
* The summary prints pixel and grid coordinates (`col N+R`, `row N+R`) so
  engine changes can be aligned from the text alone.

## 2026-11-15 (save/restore return, menu/HUD scaling)

* SAVE GAME / RESTORE GAME now return to the screen that opened them: the
  pause/main menu from `OPTIONS`, the game for the in-game `S`/`R` keys.  BACK
  and a successful save use the same target; the hotkey restore screen also
  loads its slot labels (it used to open empty on a fresh boot).
* New `OPTIONS > MENUS > SCALE` (whole-menu zoom) and `TEXT SIZE` (text relative
  to the zoom), plus `OPTIONS > HUD > FONT SIZE`.  Values 50/75/100/125/150/200%,
  default 100% (layouts unchanged).  Menu boxes/rows recompute from the scale
  and shrink long labels to fit; the modern HUD bar grows with its font and
  drops the HEALTH word when there is no room.  Stored as `[menus] scale` /
  `[menus] font_scale` / `[hud] font_scale`.
* The classic 64-px HUD panel keeps fixed cells (2x text cannot fit); the
  classic menu still honours the scale with a tighter box.

## 2026-11-15 (title popup pause + stick deadzone)

* The level-title dialog box now freezes the world: `Engine::tick()` advances
  only the caption/hint timers (`advance_dialog_timers()`) while
  `caption_popup_active()`, so enemies, hazards and the water/lava animation
  stop until the box is dismissed or times out.  Interpolation is held at the
  frozen positions so the frame does not smear.
* Macro playback (`--macro` and the attract demos) keeps the old free-running
  behaviour: those scripts were recorded with the world advancing under the
  popup and skipping ticks would desync them.
* **OPTIONS > CONTROLS > STICK DEADZONE** (0..50% in 5% steps, default 25%)
  replaces the hard-coded 8000/32767 threshold.  Stored as
  `[controls] deadzone`; applies to the SDL game-controller, raw joystick and
  native PS4 stick paths.  RESET DEFAULTS restores 25%.

## 2026-11-15 (OPL core selection now persists)

* Fixed `[audio] opl_core` saving back as 0 on every exit.  `MusicPlayer::open()`
  rebuilt the sequencer with `music_ = Music(device_rate)`, which reset
  `Music::core_` to the built-in OPL2 core; the save block then wrote that
  default.  The rebuild now goes through `MusicPlayer::rebuild_at()`, which
  keeps the selected core (SDL and native PS4 paths).
* Verified on desktop with a temporary data directory: `--opl-core 2` and an
  `opl_core = 2` ini value both survive a clean quit (`opl_core = 2` after the
  rewrite), and a restart keeps the selection.

## 2026-11-15 (optional FPS counter)

* `OPTIONS > VIDEO > FPS COUNTER` toggles a small `NN FPS` overlay (default
  OFF, `[video] fps_counter` in `jill.ini`).  It is drawn into the 320x200
  frame before the palette conversion, so the filters and the SDL/Vita2D/PS4
  backends present it like any other pixel: left end of the classic title strip,
  or just under the modern top bar, with a 1-px black shadow.
* The value is the measured presentation rate (start-of-frame interval, 0.9/0.1
  EMA), not the fixed 18.2 Hz world tick, so it reports what HIGH FRAMERATE is
  actually achieving on each target.

## 2026-11-14 (Vita display cap + libvita2d)

* The Vita build now clamps its window/drawable to the real display mode
  (maximum 960x544) before creating the SDL window and always uses borderless
  fullscreen.  The Vita `RESOLUTION` list is reduced to 960x544/640x480/480x272
  and `window_width/height` are capped, so no menu setting can exceed the
  panel.  The Vita default `scale` is now 2.
* Presentation on Vita moved from SDL2's renderer to **libvita2d**
  (`port/vita/screen_vita2d.cpp`): a 320x200 A8B8G8R8 texture is uploaded each
  frame and drawn at the largest integer scale that fits (2x) with a black
  letterbox.  SDL is now initialised without video (events/pad/audio only), so
  libvita2d owns GXM/display.

## 2026-11-14 (CMF translation audit against SBFMDRV)

* Replaced the velocity/TL approximation with the Creative driver's exact
  formula; the old `(127-velocity)/2` form silenced 337 note-ons in `funky.ddt`
  and 256 in `bonzo.ddt`.
* Added SBFMDRV voice allocation (9 melody / 6 rhythm voices), the duplicate
  note-on retie rule, live program-change reprogramming, note-range folding
  and the correct 1/256-semitone transpose scale.
* Pitch now uses the chip's native 49,716 Hz for Nuked/fmopl, and the built-in
  OPL2 EG-type handling was corrected (sustaining vs. percussive envelopes
  were swapped).
* Percussion now writes the additive-connection register and uses the carrier
  scaling byte for velocity; rhythm-mode note-offs ignore stale note numbers.
* The loop wraps at the end-of-track tick and cuts sounding voices / resets
  transpose before replaying tick 0.

## 2026-11-14 (fmopl core, title strip position)

* `port/src/third_party/fmopl.c/.h` vendors fmopl 0.37a (AdPlug's
  LGPL-2.1 fork of Tatsuyuki Satoh's YM3812 core).  `FmOpl` uses the real
  3.579545 MHz chip clock and is gain-matched to the built-in core.
* The OPL core option now has three choices: `BUILTIN OPL2` (0),
  `NUKED OPL3` (1) and `FMOPL` (2), selectable from `OPTIONS > SOUND > OPL
  CORE`, `[audio] opl_core` or `--opl-core 0|1|2`.
* The classic title text moved up from y=13/14 to y=9/10 (8-px/6-px font), so
  it sits in the top strip like the original screens.

## 2026-11-14 (optional Nuked OPL3 music core)

* `port/src/third_party/opl3.c/.h` vendors Nuked OPL3 1.8 (LGPL-2.1, notice
  preserved).  `NukedOpl` exposes the same reset/write/generate surface as the
  built-in `Opl2`, and `Music` switches between them.
* `OPTIONS > SOUND > OPL CORE` and `[audio] opl_core` / `--opl-core 0|1`
  select `BUILTIN OPL2` (default) or `NUKED OPL3`.  `--dump-music` takes the
  same switch.  The Nuked path is gain-matched to the built-in core.
* License check: Nuked (LGPL-2.1) is compatible with the PolyForm project when
  the component source and relink path are shipped.  DOSBox `dbopl` is
  GPL-2.0 and is therefore not vendored; libADLMIDI is LGPL-3.0 and would
  require LGPL-3 relink provisions and a large dependency tree, so it is not
  included.  See THIRD_PARTY_NOTICES.md.

## 2026-11-14 (tester follow-up: classic frame, gem marker, landing)

* **Classic HUD frame**: the frame now uses the original 8-px side borders and
  12-px top/bottom borders (shape entry 3), with a 10-px title strip; the
  bottom 8 px of the screen are a plain black hint strip *below* the frame, and
  `put_bottom_msg` draws there across the full width.  The in-game inventory
  box is the original 64x84 size again (not a full-height column), with its own
  INVENTORY caption strip, and the classic main-menu HI SCORES panel is also
  64x84 with six-pixel rows.
* **Map gem doors**: the 16x16 `MAPDOOR` (0x0BE) tile already carries its own
  red corner outline around the gem.  The redundant 8x8 keyhole overlay is no
  longer drawn for gem doors, so there is nothing left to misalign; key doors
  keep the keyhole at the original `x+4,y+0x0C`.
* **Landing**: after checking the original handler, the one-tick landing tick
  and the negative-`p11` landing pose are restored (4 frames soft / 7 hard,
  cancelled by a held direction on the next ground update).  The temporary
  "keep walking" shortcut was removed.
* **Classic left column**: the view is back to 232 px, separated from the
  64-px column by an 8-px patterned gap (as in the original screens).  The
  column is HEALTH at the top, LEVEL/SCORE below, then the 64x84 inventory
  panel and the INVENTORY caption at the bottom.  The classic main menu keeps
  the HI SCORES panel plus the health/score panel and caption (the controls
  legend was removed).

## 2026-11-14 (tester feedback: general fixes)

Seven fixes from desktop testers, all in shared code (desktop/PS4/Vita):

* **Locked-door marker**: `0x0E05` (the red keyhole/border) is the world-map
  icon, not an in-level overlay.  It is now drawn only when `map_mode_` is set,
  matching the original's `DGROUP:0x120E` gate (`25EB:09C0` sets it on the hub,
  ESC clears it).  The misplaced keyhole at the bottom of level doors is gone.
* **Sound remap table**: `SfxBank::remap()` had the 50-byte table shifted after
  logical id `0x14`, so several effects addressed the wrong (often empty)
  sample.  It is now byte-identical to `DGROUP:0x1212`; in particular the
  stalactite drop (`0x2F -> 0x23`) plays again, and `0x1B` (landing) now maps
  to `0x30`.
* **Map-screen music**: returning to the front end from a level restarts the
  game's title track instead of leaving that level's song playing, and the hub
  restore always re-requests the map track (it no longer trusts a stale
  `last_music_`).
* **Animated water** (new enhancement, default ON): the tile prepass cycles the
  six original water groups `0x190`..`0x1A5` on odd sub-frames, exactly as
  `1F28:0006` mode 2 does.  `OPTIONS > GAMEPLAY > ANIMATED WATER` and
  `[gameplay] animated_water` / `--no-animated-water` control it.
* **Classic HUD message strip**: the area under the 240x160 playfield is now a
  black 12-px bar, and `put_bottom_msg` hints ("APPLES GIVE YOU HEALTH", save /
  restore notices) are centred there instead of over the patterned border.
* **Classic text pages**: with `VIDEO > MESSAGE BOX: CLASSIC`, the news
  bulletin / help / story pages use the screenshot presentation - shape-7
  patterned window, blue title, blue text plate, paging footer.
* **Modern text pages**: a centred translucent card (three of every four body
  pixels darkened, solid title/footer strips) instead of a full-screen black
  page, so the board stays visible behind the popup.

## 2026-11-14 (VitaSDK toolchain + PS Vita build target)

* VitaSDK `2026.08` installed natively on Windows through the vdpm PowerShell
  bootstrap (`C:\vitasdk`, GCC 15.2.0, newlib 4.1) plus `sdl2 2.32.8`.  The
  `.ps1` needed two local patches to run on Windows PowerShell 5.1:
  `Path.IsPathFullyQualified` is a .NET Core-only API, and
  `New-Item -ItemType Directory -Force C:\` throws "not of a legal form" when
  the install dir's parent is a drive root.
* New standalone target `port/vita/`: `CMakeLists.txt` compiles the same
  `port/src` translation units with `-DJILL_VITA=1 -DJILL_HAVE_SDL=1`, builds
  `jill.self` with `vita_create_self` and `jill.vpk` with `vita_create_vpk`
  (title id `JILLEX001`, LiveArea art under `port/vita/sce_sys/`).
* Platform layer: `JILL_VITA` now shares the console code paths with `JILL_PS4`
  in `platform_paths.cpp` (data root `ux0:data/JillEX`, always-on tracing) and
  in `main.cpp` (create the data root, `chdir` into it, `jill.log` beside the
  data, absolute path resolution).  The PS4 `_exit()` path stays PS4-only; the
  Vita path returns from `main()` normally.
* `build_vita.cmd` mirrors `build_ps4.cmd`: checks `VITASDK`, finds Ninja
  (host tool, shipped with Visual Studio), configures with the Vita toolchain,
  builds, and stages `jill.ini` + `games/` + `profiles/` into
  `port/vita/build/data/` and `JillEX-Vita-data.zip`.
* First build succeeds: `jill.vpk` 1.65 MB (`eboot.bin` 1.56 MB) and the data
  zip; only the existing `miniz`/unused-function warnings.  **Not yet run on
  hardware.**
* VPK install failed on the user's Vita with `0xFFFFFFFF`, then `0x8010113D`
  at ~85%: SceShell only accepts **8-bit indexed** LiveArea PNGs, and the
  first artwork was written by System.Drawing as 32-bit RGBA.  `icon0.png`,
  `bg.png` and `startup.png` are now generated by `port/vita/make_livearea.ps1`
  through ffmpeg `palettegen`/`paletteuse` (`pal8`, 256 colours,
  non-interlaced), and the rebuilt VPK is 1.62 MB.

### Vita hardware bring-up (same day)

Runs on hardware now: front end, Jill 1 hub, music (`funky.ddt`) and the SDL
Vita audio device all come up, and external packs load.  Four separate faults
had to be cleared first, each found from `ux0:data/JillEX/jill.log`:

* **Double data root** (`ux0:data/JillEX/ux0:data/JillEX/jillt`):
  `is_absolute_path()` only knew `/x`, `\x` and `C:x`, so a Vita device path
  (`ux0:`, `ur0:`, `uma0:`) looked relative and `resolve()` prefixed it again.
  Vita builds now treat any short `xxx:` prefix as absolute.
* **Silent death before the first board trace**: the Vita main-thread stack is
  256 KiB and `Engine` is 72 784 bytes, so `run_app` alone had a 75 800-byte
  frame.  `sceUserMainThreadStackSize = 1 MiB` is now defined for the
  vita-elf-create step, `Engine` lives on the heap in `run_app`,
  `apply_selection`, `start_game`, `ensure_splash_image` and
  `ensure_theme_backdrop` (largest remaining frame is 16.6 KiB).
* **Death inside one specific trace**: `"board: %s objects=%zu ..."`.  The
  VitaSDK newlib has no C99-wide printf formats, so `%zu` aborted the process
  mid-line.  Both `%zu` uses are gone; no partial log line ever appears because
  `trace()` writes stderr before the log file.
* **Pad dead in game, menus fine**: `load_bindings_ini()` assigned
  `b.pad[a] = -1` whenever the key was missing from `jill.ini`, and the console
  ini files have no `[controls]` section.  Missing keys now keep the built-in
  defaults and only an explicitly empty value unbinds; `IniFile::has()` was
  added for that.  `port/vita/jill.ini` also spells out the full `[controls]`
  block.  Movement never broke because the D-pad/stick are read directly
  instead of through bindings.

Diagnostics kept for the console builds: `startup:` stage lines, one
`load_board:`/`board:` summary per board, `pad:` state changes (Vita) and a
`loop: paused=` line, plus a top-level `try`/`catch` and `std::terminate`
handler that now report thrown exceptions in `jill.log` instead of vanishing.

## 2026-11-14 (front-end DEMO playback)

* Front-end main menu now has `DEMO` at row 2: `START GAME`, `DEMO`,
  `RESTORE GAME`, `HIGH SCORES`, `OPTIONS`, `NOISEMAKER`, `QUIT`.  The row is
  front-end only (the in-game pause menu is unchanged).
* `DEMO` lists the installed episodes that ship attract demos (Onesimus has
  none, so it is skipped), then `DEMO 1/2/3` when the episode has more than one.
  The table is hardcoded from the original demo code (docs/episode-profiles.md
  section 5.1): Jill 1 `0.dem`/`2.dem`/`1.dem` with `jn1demo.mac`, Jill 2
  `3.jn2`/`9.jn2`/`17.jn2` with `jn2dem1..3.mac` (levels 3/9/17), Jill 3
  `1.jn3`/`5.jn3`/`12.jn3` with `jn3dem1..3.mac` (levels 1/5/12).
* Playback reuses `start_game` with board/level/macro overrides and
  `Engine::start_macro`, so the recorded inputs drive the normal update.  The
  macro's key-bit records are ignored by the port; the demo ends when the script
  runs out (or a state-3 CHECKPT fires) and returns to the main menu with `DEMO`
  still selected.  Esc/Enter, pad B/Back/Start leave early.
* `Engine::stop_macro()` and `macro_active()` added for that exit path.
* New debug switch `--macro NAME` replays a `.mac` headlessly with `--ticks`.
  All nine routes were run this way (x64): macros load and Jill moves on every
  board; Jill 2 demos end with a level-id change on some routes (same behaviour
  as the original death/respawn, not yet investigated further).
* x64, x86 and PS4 (`build_ps4.cmd nopkg`) all rebuilt with the change;
  only the known `stb_vorbis` warning.

## 2026-11-13 (exit crash root cause, case-insensitive tracks)

* The real-console quit log showed `videoout: unregister failed 0x80290009`
  (`ORBIS_VIDEO_OUT_ERROR_RESOURCE_BUSY`): the display still owned the buffers,
  and the old code then unmapped and released the direct memory *while the
  videoout port was still open*.  That is the retail CE-34878-0 on quit.
  `shutdown()` now closes the port first, and if the close fails it leaves the
  display memory alone (process exit reclaims it) instead of unmapping it.
* Music: `track_path()` resolves through the new `jill_paths::resolve_file()`,
  which falls back to a case-insensitive match in the game's data directory.
  The PS4 data filesystem is case-sensitive while the DOS-era assets (and FTP
  copies) are not consistently cased; a miss silently kept the previous track
  playing.  Both the fallback and the resolved path are traced
  (`path: ... -> ...`, `music: <name> -> <path>`).

## 2026-11-13 (external pack crash, exit cleanup)

* Selecting an external pack aborted the game (`std::filesystem::filesystem_error:
  in canonical: Function not implemented`).  `ExternalAssets::files()` used
  `fs::relative()`, which libc++ implements through `weakly_canonical()` ->
  `realpath()`, and the PS4 kernel does not implement `realpath` on the data
  filesystem.  Replaced it with a lexical prefix strip (`relative_to()`); the
  iterator only yields paths under the pack root, so this is exact.
* `directory_entry::is_regular_file()` calls now take an `error_code`, and
  `apply_pack` / `refresh_external_visuals` catch `std::exception` so a bad pack
  can no longer terminate the process.
* Exit cleanup: `sceVideoOutUnregisterBuffers` was called for attribute indices
  0, 1 and 2; only 0 exists (shadPS4: "Invalid attribute index 1/2"), so the
  buffers stayed registered while their memory was unmapped - a scanout of freed
  memory, i.e. the retail CE-34878-0 on quit.  Now unregisters index 0 once and
  logs a failure.  `shutdown()` is idempotent (the destructor called it twice,
  which printed "videoout: shut down" twice).

## 2026-11-12 (PS4 music tracing, round 2)

* Round 1 log (22:28) proved track *selection* is correct on console:
  `music: funky.ddt` (hub/menu) then `music: zeppelin.ddt` on board 1.
* Added PS4 traces: `board: <name> objects=<n> track=<music>`,
  `save load: board=... track=...`, `warp !: ... hub track=...`,
  `music: FAILED <name> (<err>)`, and `audio: start <title> (midi=0)` from
  `MusicPlayer::mix` when the OPL sequencer picks up a pending song.
* A new console log now shows whether a track was requested, whether it loaded,
  and whether the mixer actually restarted on it.

## 2026-11-12 (music state after save load / hub return)

* Level music could get stuck on one track because two paths replace the board
  without going through `Engine::load_board()`:
  * `Engine::load_game()` restored the saved map/objects but left
    `music_request_` pointing at whatever was playing (usually the title or hub
    track), so every restored level kept the same music.  It now derives the
    board name from the saved level id (also fixes Ctrl+R reload and the status
    line) and restarts that board's first `*track.ddt`/`*track.ark`.
  * Finishing a level through `!` restores the stashed hub board directly; the
    level track kept playing on the map.  The hub's first track is now
    requested again.
* `play_music()` logs `music: <track>` whenever it changes track, so a real
  run can be checked against `docs/music-catalog.md`.
* Verified headless: `--board map --load <level save>` now reports the saved
  board and its track (`1.jn1` / `zeppelin.ddt`), and `--board 1 --start` on
  either `!` checkpoint returns to `map.jn1` / `funky.ddt`.  PC and PS4 builds
  pass.

## 2026-11-12 (PS4 port: first build)

* Added `PS4_PORT_PLAN.md`: verified toolchain facts, `/data/JillEX` layout,
  the Piglet/EGL OpenGL ES renderer design, a FluidSynth licence/feasibility
  verdict, a numbered risk table and the phase plan.
* New `platform_paths` helper: on PS4 the data root is `/data/JillEX`; the
  process creates it, tries to `chdir` into it, and mirrors
  `stdout`/`stderr` to `jill.log`.  All configured dirs, per-game asset
  folders, palettes, save slots and high scores are resolved to absolute
  paths, because shadPS4 (like some loaders) stubs `chdir`.  Desktop builds
  are unchanged.
* New `Screen` seam (`screen.h`): the SDL window/renderer/texture code moved
  out of `app.cpp` into `screen_sdl.cpp` unchanged, so PC output is identical.
  `app.cpp` only creates the screen, presents frames, sets the title and shuts
  it down.
* New `port/ps4/screen_gl.cpp`: EGL + OpenGL ES 2.0 via `libScePigletv2VSH`,
  using the firmware's precompiled `texmap` shaders (`glShaderBinary`) and a
  streaming 320x200 RGBA texture, integer-scaled and letterboxed into 1080p.
* New `build_ps4.cmd`: compiles every port source plus the GL screen with
  clang (`gnu++17`), links with `ld.lld`, produces
  `port\ps4\build\eboot.bin`, then builds a fake-signed installable
  `IV0000-BREW00199_00-JILLEXPS40000000.pkg` with `create-gp4` +
  `PkgTool.Core` (param.sfo, icon0.png, libc.prx, libSceFios2.prx,
  `sce_sys/about/right.sprx`).  `build_ps4.cmd nopkg` stops after the eboot.
  `build.cmd`/`build32.cmd` are untouched.
* Toolchain portability fixes: guard `SDL_HINT_DIRECTINPUT_ENABLED` (SDL
  2.0.9 does not have it) and cast `std::fabs` to `float` before `std::max`
  (the PS4 libc returns `double`).  PC build re-verified.
* shadPS4 testing: it stubs `chdir`, so all PS4 paths are resolved to absolute
  `/data/JillEX/...` paths.  Startup milestones are traced to `jill.log`.
  `[video] renderer = auto` uses GLES on hardware and the SDL software screen
  under shadPS4 (detected via its `/hostapp` mount), where Piglet init hangs.
* Black-screen fix: the PS4 SDL backend now follows the OpenOrbis sample
  (`SDL_GetWindowSurface` + `SDL_CreateSoftwareRenderer` + explicit
  `SDL_UpdateWindowSurface`, with a per-frame destination rect instead of
  logical-size scaling).  The diagnostic log is written with a real `fopen`
  handle, not `freopen`, so it is populated on both hardware and the emulator.
* The first presented frame is checksummed and dumped to
  `/data/JillEX/first_frame.ppm` (plus `frame: first argb sum=...` in the log)
  to distinguish "black framebuffer" from "black presentation".  Piglet config
  now matches the sample exactly (`flags |= 0x60`, splash hidden after EGL).
* Confirmed with the dump that the engine frame is correct (menu visible) and
  the PS4 window surface is `BGR888`.  The SDL software renderer's conversion
  to that surface produced a black image, so the PS4 SDL backend now writes the
  scaled pixels directly into `SDL_GetWindowSurface()` and calls
  `SDL_UpdateWindowSurface()`; no SDL_Renderer is involved on PS4.
* Root cause of the remaining black screen: the OpenOrbis SDL2 PS4 video driver
  uses `window->x` / `window->y` as the *destination offset* when copying the
  window surface into the video buffer.  shadPS4 reports a 1280x720 video mode,
  so a forced 1920x1080 window is centred at (-320, -180); the driver's
  unsigned/clamped offsets then copy the bottom-right black region.  The PS4
  SDL backend now sizes the window from `SDL_GetDesktopDisplayMode(0)` and
  creates it at (0,0), so surface size == buffer size and offsets are zero.
  `sdl screen: video mode ...` and `sdl screen: presented #n` verify this.
* Input: the bundled SDL2 PS4 joystick driver never produces a game-controller
  mapping (its GUID is malformed), so no menu received any button.  The port now
  initialises libScePad directly (`port/ps4/ps4_pad.cpp`), polls the DualShock 4
  once per frame, synthesises `SDL_CONTROLLERBUTTONDOWN/UP` events for the menu
  code, and feeds the engine directly for gameplay (Cross = jump, Square = throw,
  d-pad/left stick = move).  SDL joystick/controller init is skipped on PS4.
* Audio stability: the PS4 audio device buffer is 2048 samples (was 1024) and the
  frame-pacing busy-spin is disabled on PS4 (it held a core for tens of
  milliseconds and starved the audio thread).
* Audio tempo: SDL2's PS4 audio driver waits for the previous buffer to finish
  before mixing the next one, so the mixing time stretched every playback period
  (audible as slow music on the console, fine on fast shadPS4 hosts).  The PS4
  build now owns the output: a native `sceAudioOut` thread with two buffers
  mixes buffer B while buffer A plays.  `audio: mix X ms / 42.7 ms budget` logs
  the cost; the PS4 SDL audio driver is no longer used (SDL mutex replaces
  `SDL_LockAudioDevice` so all existing state locking still works).
* Added a PS4-only debug overlay (`[debug] overlay = 1` or `--debug`): FPS,
  frame time, engine tick, frame build, present and audio mix/budget.  The same
  numbers are logged as `perf: ...` every 5 s.
* Licensing cleanup: untracked all reverse-engineering artifacts that contain
  Epic-derived binaries/data (`re/img*.bin`, `re/proj/`, `re/export/`,
  `re/*.txt`, `re/*.asm`, `re/*.png`) and the `hiscores_panel.png` screenshot;
  they stay on disk and are ignored via `.gitignore`.  Added a Legal/trademarks
  section to README, an OpenOrbis GPL-3.0 / Sony-module note to
  THIRD_PARTY_NOTICES, updated `LICENSE.md` (Onesimus palette exception) and
  refreshed `docs/licensing.md`.  History still contains the old files; purging
  needs `git filter-repo`.
* The PS4 icon uses Jill's face, so the Epic notice now covers it explicitly:
  `LICENSE.md`/`README.md`/`THIRD_PARTY_NOTICES.md`/`docs/licensing.md` call out
  `port/ps4/sce_sys/icon0.png`, a new root `DISCLAIMER.txt` is copied beside the
  `.pkg` and into `JillEX-data.zip`, and every splash page shows
  "UNOFFICIAL FAN PORT / JILL IS A TRADEMARK OF EPIC GAMES".
* 30 Hz cap: the profile showed `present` at ~25 ms.  SDL2's PS4 video driver
  waits for the previous flip to finish before building the next frame, so the
  loop could only complete one frame per ~1.5 vblanks (30 Hz).  New native
  `sceVideoOut` backend (`port/ps4/screen_videoout.cpp`) with 3 buffers and two
  flips in flight: the CPU never stalls on a finished flip, so the frame pacer
  holds 60 Hz.  `renderer = auto` prefers it (`videoout | gl | sdl`).
* Quit crash (CE-34878-0): the log reached `startup: main loop exited` and the
  fault happened afterwards, i.e. in CRT/SDL teardown after our devices were
  already closed.  PS4 `main` now calls `_exit()` after `run_app()` (everything
  is flushed by then).  Videoout teardown also drains outstanding flip events
  before unregistering buffers.
* `build_ps4.cmd` now stages and zips the companion data
  (`jill.ini`, `games/`, `profiles/`, README) as `port\ps4\build\JillEX-data.zip`
  for transferring to `/data/JillEX` alongside the installed pkg.  The pkg and
  the zip are also copied to the repository root for quick access (ignored by
  git via the new root `.gitignore`).

## 2026-11-12 (modern HUD suit shadow)

* The modern HUD with `BACKGROUND: SUIT` now draws a two-row drop shadow under
  the 12 px bar: a solid dark edge plus a 50% checker row.  Pack-supplied
  modern HUD panels are unchanged and keep their own transparency/shadow art.

## 2026-11-12 (menu text alignment)

* Modern menus with a shape-7 border (CURRENT / a game / an external border)
  now start their item text below the 8 px top border instead of at `by+5`,
  which had clipped the first row and pushed the whole list up into the frame.
  Classic menus, tiled `BORDER: THEME` menus and plain menus already used the
  correct offsets and are unchanged.

## 2026-11-12 (classic menu intro artwork)

* `OPTIONS > MENUS > THEME` game themes now also fill the CLASSIC front-end
  viewport with the original intro-board section at tiles `(113,54)` to
  `(127,63)`: 15x10 cells = 240x160 px, including the intro-board Jill and the
  `THIS GAME IS SHAREWARE` text.  PORT and pack/autoload theme options remain.
* Fixed the classic HUD viewport to that 240x160 rectangle (`x=72`, `y=20`)
  instead of the old 238x166 crop.

## 2026-11-12 (original-board NOISEMAKER and SOUND TEST rename)

* Added `NOISEMAKER` to the main menu.  It first asks which episode to use,
  then opens that episode's original front-end page 99 on the intro board: a
  private `Engine` renders the board at the page-99 CHECKPT, showing the large
  `NOISE` title, Jill portrait and `NOISE BLASTER ... PRESS THE QWERTY KEYS`
  text.  QWERTY keys in the original `2cff:1246` order play raw slots 1..45
  from the selected episode's `.vcl` bank; Esc/pad B returns to the menu.
* Renamed the existing `OPTIONS > SOUND > NOISEMAKER` raw-slot audition screen
  to `SOUND TEST`.

## 2026-11-12 (stereo 3D paused)

* The stereo display is disabled entirely for now: no `STEREO 3D` row, no
  stereo config keys or CLI flags, and the runtime forces stereo off.  The eye
  separation/composition code remains in place behind an internal disabled
  constant for a future revisit.

## 2026-11-12 (Jill 3 map return)

* Fixed the Jill 3 hub spawn after finishing `1.jn3`.  Leaving the map through
  a CHECKPT moved the TINY avatar to the checkpoint's `dx/dy` respawn cache,
  but the shipped map boards leave those fields at `(0,0)`, so the hub was
  stashed with Jill at the non-walkable top-left corner.  Restoring that copy
  made her appear in the wrong place and `tiny_update` could not move her.
  A map CHECKPT now only honours a populated cache; otherwise the avatar keeps
  its current position.  Hub restores also update the map respawn point.

## 2026-11-12 (system MIDI output)

* Added `port/src/midi.*`: a WinMM `midiOut` wrapper plus a CMF sequencer that
  sends note on/off, program changes, volume/pan/expression/sustain and GM
  percussion (CMF channels 11..15 -> channel 10) to a selected system MIDI
  device.  The normal OPL2 path remains the default.
* `OPTIONS > SOUND > MIDI OUT` cycles `OFF (OPL2)` and every installed MIDI
  output.  The choice is stored as `[audio] midi_device`; `--midi N` starts on
  a device and `--list-midi` prints the available indices.
* CMF patches are mapped to GM programs per track: a note-range pass finds the
  register each AdLib instrument plays in, and the OPL envelope/brightness
  picks a bass, piano, guitar, string/pad, brass/reed, flute or lead.  CMF
  channels 11..15 still become GM percussion on channel 10.
* Switching devices restarts the current CMF track on the new output.  Sampled
  SFX still use the SDL audio device, so MIDI music and effects can mix.

## 2026-11-12 (autoload, named visual sets, turtle mode)

* Added `external/autoload/`: everything in that folder is always mounted under
  the selected pack (pack files win on a name collision), and it is not listed
  as a selectable pack.  Headless `--dump` applies autoload too.
* HUD panels, menu themes, borders and fonts can now provide several named
  variants from a pack or from autoload:
  * `hud/classic_<name>` and `hud/modern_<name>` appear in `OPTIONS > HUD >
    PANEL`; unprefixed `hud/<name>` files are classified by aspect.
  * `borders/<name>.png` is a 3x3 nine-slice sheet; `borders/<name>/<1..9>.png`
    also works.  Named borders join the `OPTIONS > MENUS > BORDER` cycle after
    the installed games.
  * `fonts/<name>` and `fonts/<name>_8` add selectable 6 px/8 px font sets;
    `main`/`main_8` still apply automatically.  Selection is `OPTIONS > MENUS >
    FONT`.
* Added TURTLE MODE (`OPTIONS > GAMEPLAY`, `[gameplay] turtle`, `--turtle`),
  the original Jill 3 half-speed option.  High-framerate interpolation spans
  the two-tick slow update, so turtle movement no longer stutters.
* Documented the new pack layout and flags in `EXTASSETS.md`, `HOTKEYS.md`,
  `COMMANDLINE.md`, `jill.ini`.

## 2026-11-11 (ending `.mac` macros)

* Fixed the `*.mac` parser.  Record 0 is `mask, one signed byte per set mask
  bit`; later records start with the absolute tick (base-128 varint) and then
  the mask + values.  A record only updates the inputs named by its mask.  The
  old code cleared every input on each record, which made `end1.mac` /
  `end2.mac` play incorrectly.
* Fixed the ending entrance trigger.  Levels 50.jn1 and 20.jn2 spawn on their
  `&end1.mac` / `&end2.mac` CHECKPT, whose p13 equals the restored level id;
  the per-tick CHECKPT handler skips checkpoints with `p13 == level`, so the
  macro never started.  `place_at_checkpoint` now queues an entrance macro and
  starts Jill one trigger-width left of the flag.
* Fixed the bird form's flight: gravity is applied before the flap, a flap
  keeps the full -6 dy (instead of -5), and either action button flaps, as in
  1fae:298B.  The ending's phoenix now clears the flooded grove instead of
  dipping into the water.  The ending's JILL/FOOF token gets a small pickup
  margin while a macro is running so the recorded path still collects it.
* Ending slides now render with the full 320x200 board view even when the
  classic HUD is selected, instead of inheriting the 74..312 viewport offset
  (which pushed the cards off-screen).

## 2026-11-11 (original portrait and classic message boxes)

* Tracked the Jill portrait down to shape entry 24: sixteen 16x16 tiles (the
  bottom row is 16x22/16x20) laid out as a 4x4 picture, 64x70 px.  It is the
  face on the original title screen and beside level-title dialogs, and is
  shared by Jill 1/2/3 (Onesimus has none).  `export_portrait_asset` now emits
  this exact art as `portrait.png` instead of composing the idle sprite.
* `VIDEO > MESSAGE BOX` is now a three-way option: OFF, MODERN, CLASSIC.
  CLASSIC draws the original shape-7 windows: the portrait caption card
  (`JILL BOUNDS / THROUGH / THE BOULDERS`) and the titled `Reminder` card
  (`YOU CAN PRESS F1 AT ANY TIME TO GET HELP.`), both with the 6 px font and
  blue text plate.  The reminder appears after the first caption of a fresh
  game and from F1 when no text pages are installed.
* Level-title dialogs in both styles now use the real entry-24 portrait; the
  modern box keeps its frame.  The portrait can still be overridden by a pack
  `portrait.<ext>`.
* `[gameplay] message_box` accepts `0/1/2`; `--classic-message-box` and
  `--modern-message-box` set it from the command line.

## 2026-11-11 (splash screens)

* Start-up splashes are enabled again.  Jill 1/2/3 use the original
  "A NEW RELEASE FROM Epic MegaGames" loading card with the game title,
  `PRODUCED BY TIM SWEENEY`, `NOW LOADING, PLEASE WAIT...`, and each episode's
  own shape-7 window colours.  Onesimus uses the ARK / Multimedia Publishing
  Presents card followed by the rainbow ONESIMUS title and Bible-quote
  copyright card.
* The startup choice is now `[video] splash`: `-1` auto (current game), `0`
  off, `N` = game N; `--no-splash` disables it.  `OPTIONS > MENUS > SPLASH`
  cycles AUTO / OFF / each installed game.  Any input skips the pages.

## 2026-11-11 (classic menu border and ending cards)

* Shape entry 7 is a scan-order nine-slice: frames `1/2/3` are the top row,
  `4/5` the left/right edges, `6/7/8` the bottom row and `9` the checker fill.
  The CLASSIC menu now uses those tiles, fixing its right edge, bottom edge and
  bottom corners.
* Ending cards now start only when the player hits a board's state-3 ending
  CHECKPT (`50.jn1`, `20.jn2`, `14.jn3`, the Onesimus ending and mod boards
  using the same marker); simply loading the board no longer starts them.
  The TEXT6/TEXT8 objects are grouped around the p13 >= 200 CHECKPT camera
  anchors and shown as full-screen cards.  Cards advance every seven seconds,
  Enter/click/A advances immediately, and Esc/B skips to the main menu.
* Per-game high scores: `HIGH SCORES` on the main menu, plus a classic front-end
  side panel.  Tables are stored in `hiscore_<game id>.txt`, ten entries each,
  and a qualifying score is recorded when a game ends or `QUIT TO MAIN MENU` is
  used.  A qualifying score opens a name prompt (up to 9 characters); blank or
  Esc saves it as `JILL`.
* Main menu now always offers `RESTORE GAME`; Left/Right changes the episode
  on both RESTORE GAME and HIGH SCORES.  In-game menus also expose
  `QUIT TO MAIN MENU`.
* Modern menus have `BACKGROUND: THEME / LIVE GAME`, and an in-game classic
  menu is now a window overlay on the paused game instead of redrawing the
  full classic backdrop.
* The world-map `MAP` label sits below the modern 12-px HUD bar instead of
  being clipped by it.
* Menu THEME and BORDER are independent: THEME chooses the classic backdrop's
  shape 3 frame, BORDER chooses the shape 7 window (`THEME`, `CURRENT` or a
  specific game).  Selecting an explicit BORDER now also applies to the modern
  menu window, while `BORDER: THEME` keeps the old theme-tile border.

## 2026-11-10 (menu theme shift and NOISEMAKER)

* Menu THEME backdrops now bake the intro-board picture panel at tiles
  `(36,0)`..`(55,13)` (20x14 cells, 320x224 px), one screen wide, so only the
  vertical centre crop remains.  TEXT6/TEXT8 story and credits copy stay
  hidden.
* NOISEMAKER is a new front end screen under `OPTIONS > SOUND`: it lists every
  installed game under `SET`, steps through the 50 raw `.vcl` slots, and plays
  the selected sample through `MusicPlayer::play_sfx_sample()` (raw index, no
  logical-id remap).  Labels come from `docs/profiles.json` `sound_names`,
  emitted as `kSfxNames` by `re/gen_port_data.py`.  Music is muted while the
  screen is open and the game's own bank is restored on exit.
* `SfxVoice::play()` now delegates to `play_sample()`, so the runtime path and
  the sound test share the same voice/priority logic.

## 2026-11-09 (score table and gem carry-over)

* Scores follow the design table: Big Ant (kind 4) is 6, and the Epic Logo
  awards 25 per touch for 11 touches (275 total, then it explodes).  The
  per-touch value is `epic_score` in `docs/profiles.json`; kind 4's value is
  a `score_overrides` entry, applied by `re/gen_port_data.py` and
  `re/gen_games.py` when they emit the compiled and runtime kind tables.
  The rest of the table already matched the kind-table counts: Phoenix 4,
  Frog 15, Beehive 11, Giant Ant 6, Evil Fish 7, Giant Slug 5, Crab 2,
  Snake 35, Gator 3, Keygem 23, Apple 12, Flag 6.
* Unused gems (inventory item 3, the keygem) now carry across level warps even
  with the default "drop keys/weapons on exit" rule.  `retain_items` still
  keeps the whole inventory.

## 2026-11-09 (remaining object kinds)

* Implemented every remaining kind handler: 3 KILLME, 6 MACROTRIG, 13 PAUL,
  31 FIRE, 39 SNAKE, 40 SEAROCK, 41 BOLL, 44 KNIGHT, 45 BEENEST, 46 BEESWARM,
  48 CROC, 51 SKULL, 53 PAC, 58 BUBBLE, 59 JELLYFISH, 60 BADFISH, 61 ELEV.
  Each has draw, update, hit and trigger branches where the original has them.
* New per-episode handler tables (`snake_head/mid/tail`, `boll`,
  `knight_xoff/frame`, `beeswarm`, `croc_head/tail`, `skull_a/b`, `jelly`,
  `badfish`) live in `docs/profiles.json`, are emitted by
  `re/gen_port_data.py`, and are read from `profile_->tables`.
* BEESWARM homes only on its 3-tick frame wrap and grows by the generated
  table; JELLYFISH kicks once per 10-tick pulse; PAC follows the decompiled
  corridor neighbour probes; BOLL starts from its current `dx`.  ELEV treats
  an unsigned `p13 == 0xFFFF` as "do not auto-descend" and caches the last
  input in `p0f`.
* Headless runs: Jill 1 boards 1/2/3/6/9 and Jill 3 boards 3/9/11/13/14 show
  sane snake/croc/boll/knight/pac/bubble/jelly/badfish/elev motion and no
  runaway velocities; smoke tests pass for jill1/jill2/jill3/onesimus.

## 2026-11-09 (nine gameplay/HUD requests)

* Death particles: EXPL1/EXPL2 now use the kind-table tilesets (`0x2E` / `0x0E`)
  instead of the wrong `0x5A` / `0x5C` bases; the debris bursts render.
* Camera: follows up to 16 px/tick horizontally and 24 px/tick vertically,
  snapping when the target moves more than 96 px; respawn snaps immediately.
* HUD: `HUD -> TEXT` cycles the 16 EGA text colours (`--hud-text N`,
  `[hud] text`), and `SUIT VIBRANT` / `--hud-bg vibrant` uses the bright shade
  of the episode suit colour (`-3` in the ini as `vibrant`).
* Switches (kind 32) now sort behind every other object, so Jill is drawn in
  front of the panel.
* Jill 3's menu theme frame ring uses the red brick tile (`menu_block = REDB`,
  a new manifest field) instead of the grey castle block.
* `INV` / `FLY` debug labels moved below the HUD bar (below the classic panel).
* `SUPER JUMP` cycles `OFF, 1..15` with a hard cap of 15; `STUPID JUMP` is
  removed from the engine, menu, CLI and docs.  Bare `--super-jump` = 15.
* Raise animation: the documented climb still ends at the same tick, but the
  port plays an eight-frame pull-up on the last upward step (climb poses
  `0x1A`/`0x19`/`0x18` then the standing frame), shifting the sprite up the
  last few pixels and redrawing solid ledge cells over her legs so she emerges
  from behind the block below.  `BoardObject::raise` drives it; input is held
  for those frames, and respawn/morph/warp clear it.

## 2026-11-08 (jill.ini, morph tokens, interpolation, popup fix)

* `jill.ini` now stores the front-end state: game id, `[paths]` (games and
  profiles directories plus one asset directory per game id), video
  mode/scale/resolution/high framerate, HUD style/background, gameplay
  enhancements and key/gamepad bindings.  It is written on exit; CLI flags
  still win for a run.  `run.cmd` leaves the file in the repository root.
* Level-title popups no longer vanish the instant a board loads: input cannot
  dismiss the caption for its first `0x28` ticks, so a direction held while
  walking through an exit no longer eats the "JILL BOUNDS ..." box.
* Transformation tokens work: TOKEN `p13` 4/5/6/7 morph Jill to frog/bird/
  spider/fish (kinds 57/56/55/54) and token 0 ("FOOF!") reverts to normal
  Jill.  New `form_update`/`form_draw` implement the documented handlers:
  fish swim/steer/fire FISHBULLET, bird flap/fire FIREBULLET, frog hop, spider
  stub; morphing resets the player fields and uses the kind-table size.
  Kinds 62/63 FIREBULLET and FISHBULLET are also implemented (arc/flat shots).
* High-framerate modes now interpolate object and camera positions between
  logic ticks (`render(..., alpha)`); animations still advance at 18.2 Hz.
* CHEATS -> `ADD BLADE` grants item 8 (spinning blade).  `MAX KNIVES` remains
  the 3/5 gameplay enhancement and is now persisted.

## 2026-11-08 (pad text pages, high framerate, spinning blade)

* Text-page viewer now takes gamepad input: D-pad/left stick scroll, left/right
  step pages, A/B/Start/Back closes; raw DirectInput pads use their hat.  The
  keyboard path is unchanged.
* `VIDEO -> HIGH FRAMERATE` (`OFF` / `60` / `90` / `120` / `144`, default 60)
  gates presentation pacing.  The world still ticks at 18.2065 Hz; `OFF`
  presents at that rate.  Added `--high-fps N`.
* Ported kind 50 SPINBLAD from `1a41:2b1a`: p13 spin follows travel, p0f ages
  the blade out at `0x40`, nearest flag-`0x400` target within `0x60` steers it
  (dy always falls, both clamped +/-0x0C), `0xA7` tiles are eaten (+10 p0f
  each), x/y bounces with the wall/floor sounds, and it self-destructs when
  stuck.  Hit behaviour kills it on Jill after 8 ticks and awards kills on
  `0x400` targets.  Draw uses bank `0x2D`, frames `p13 & 3`.  `--blade` spawns
  one for headless checks.
* Verified headless: blade flies and homes toward a PHOENIX (dx 6 -> 12), and
  expires after 64 ticks; ep1/2/3 and Onesimus smoke runs still pass.

## 2026-11-08 (VCL text pages: news bulletins and the Onesimus verses)

* Found the "News bulletin" mechanism: the `.vcl` sampled-sound bank has a
  40-entry text-page table at `0x190` (u32 offsets) / `0x230` (u16 sizes),
  loaded by `FUN_1581_03ef(page)` and shown by `FUN_22ab_0f0a(page)` (framed,
  centred 6-px lines, up/down scrolling, Enter/Esc).  Page numbers are 0-based;
  the menu passes 1 for INSTRUCTIONS.
* How pages appear in game: kind 1 APPLE is also a page pickup.  With
  `state > 0` the hit handler plays sound (6,0x19), calls the viewer with
  `state` as the page number, sets `ttl = 4` and kills the object (normal
  `state == 0` apples give health).  Jill 1's boards place states 3-15
  ("Reminder" ... "News bulletin" x4 ... "Mega Hint"); Onesimus places the
  Philemon verses at states 10-29 across its 16 boards.  That is the screen the
  user remembered after level 1, and the same viewer as the bible verses.
* Port: new `src/textpages.cpp` (`TextPages`) parses the bank; `ProfileDef`
  gained `sound_bank` (generated from `profiles.json` sound.bank),
  `GameDef::pages_file` (`pages` manifest key, defaults to the profile bank).
  `Engine::show_text_page/close/scroll/cycle` draw a framed full-screen page
  (title, coloured centred lines, PAGE/LINE footer); the world freezes while it
  is open.  F1 opens page 1; Up/Down scroll, Left/Right step pages, Esc closes.
  `--text-page N` opens page N headlessly.
* `tools/mkprofile.py` now records the single `.vcl` it finds (`pages = ...`);
  `games/onesimus.game` was updated with `pages = ONESIMUS.VCL`.
* Verified headless: `--text-page` renders pages 1/3/4/10/15 (Jill 1) and
  1/10/11/29 (Onesimus); walking the player into the state-3 and state-5 page
  apples in `1.jn1` produces pixel-identical output to `--text-page 3/5`.
* Portraits with the level-title boxes were abandoned at the user's request.

## 2026-11-08 (level-title dialogs, bottom message box)

* Corrected the previous "faster dialogs" target.  The bottom message line
  (`APPLES GIVE YOU HEALTH`, token hints) now keeps the original behaviour:
  `0x50` ticks, shown all at once.  FASTER DIALOG BOXES now affects the real
  dialog boxes:
  - level-title captions ("JILL BOUNDS THROUGH THE BOULDERS", "JILL ENTERS THE
    JUNGLE MAP.") from the 36-entry table, drawn as a centred framed box and
    revealed at 1 char/tick (4 with the enhancement); any input dismisses it;
  - TEXT6/TEXT8 board text pages (the intro/ending "news" pages) now type out
    at the same speed, so long pages reveal progressively.
* New `TEXT_REVEAL` path: `Engine::text_reveal_` grows 1/4 per tick and clamps
  kind 20/21 drawing to that many characters.  `finish_dialogs()` shows the
  whole page for the static menu theme backdrop.
* Level titles are generated data now: `re/gen_port_data.py` emits
  `kTitles1..3` from `frontend.title_table` (index = level id + 4), `ProfileDef`
  gained `level_titles/title_count`, and runtime profiles accept
  `level_title = N, text` rows (also emitted in the reference profiles).
* `VIDEO -> MESSAGE BOX: ON/OFF` (default ON, `--no-message-box`) draws a
  framed box behind the bottom message line.
* Verified headless: caption reveal grows between 8 and 60 ticks and the frame
  is present (430 grey frame pixels); default vs `--fast-dialogs` differs at
  tick 8; MESSAGE BOX on/off differs by 1239 pixels on the apple hint.
* Fixed the palette written by `tools/mkprofile.py`: the extracted VGA palette
  is 6-bit, but the port's `--palette` path is raw 8-bit RGB, so Onesimus was
  rendering at quarter brightness.  The tool now expands the values
  (`v * 255 / 63`); `profiles/onesimus.pal` was rescaled.  Onesimus captions
  now show ("ONESIMUS ENTERS THE WORLD MAP.").
* Build clean; ep1/2/3 + Onesimus smoke runs pass.

## 2026-11-08 (Onesimus kinds, dialogs, faster dialogs)

* Onesimus named properly: `games/onesimus.game` `title = Onesimus: A Quest for
  Freedom` (the menu uppercases it; `--list-games` shows the real casing).
* Finished the Onesimus content work: kinds 64-68 are now drawn, updated and
  hit-tested per `docs/jill23-new-kinds.md`:
  - EYE: body + pupil, `vectdist(...,3)` tracking with `dy = +/-1`, `dx >= -2`.
  - VINECLIMB: `p13` bob, `cando(...,4)` reversal, hit pushes the player 8 px,
    adds `0x0F` to the player `p13`, then damages.
  - FLAG: 6-tick animation, one-shot `+6` score + sound + 5-particle burst.
  - MAPDEMO: two runs (`0x10 x4`, `0x14 x3`), pinned to the viewport corner.
  - ROMAN: walk cycle, 1-in-30 re-aim, `crawl` + reverse, step sound.
  Verified headless: `6.ONE` EYE pupil moves to `(0,-1)` / `(-2,-1)`, VINECLIMB
  `dy=2`, `28.ONE` ROMAN walks/reverses, `10.ONE` FLAG switches to `state=1`.
* In-game text works now:
  - bottom message line (`_putbotmsg`, 22AB:05b4) with the original 0x50-tick
    lifetime, centred at y=186 with a shadow;
  - TXTMSG 34, WISEMAN 16 and the TOKEN/APPLE/SWITCH one-time hints feed it;
  - TEXT6/TEXT8 (20/21) draw their board strings in the 6 px / 8 px fonts, so
    the ending boards (e.g. `50.jn1`) show their text.
* New GAMEPLAY enhancement `FASTER DIALOG BOXES` (`--fast-dialogs`): 0x28-tick
  lifetime and a 4-chars-per-tick reveal.  Verified: the same apple hint shows
  320 px of glyphs at default and 122 px after two ticks with the enhancement.
* `Engine::draw_text_font` added (bank 2/advance 6, bank 1/advance 8).
* Docs/README updated; build clean; ep1/2/3 + Onesimus smoke runs pass.

## 2026-11-08 (HUD default, video menu, themed frame, GAMEPLAY)

* MODERN/CLASSIC HUD background now defaults to SUIT (`-2`, the episode's
  leotard shade) instead of grey.  `Engine::hud_bg_` and `Options::hud_bg`
  defaults changed; `--hud-bg N` still overrides and BACKGROUND still cycles.
  Verified headlessly: the top bar renders the suit index for ep1/2/3.
* New `OPTIONS -> VIDEO` submenu (replaces the old FULLSCREEN row):
  `WINDOW TYPE` (WINDOWED / FULLSCREEN / BORDERLESS), `WINDOW SIZE` (1x..6x,
  AUTO when a resolution is set), `RESOLUTION` (DESKTOP, 640x480, 800x600,
  1024x768, 1280x720, 1600x900, 1920x1080) and a `FILTERS` placeholder.
  Left/Right steps a value, Enter/Right steps forward.  Fullscreen uses the
  resolution as the display mode, windowed resizes to it.  CLI:
  `--windowed`, `--borderless`, `--resolution WxH|desktop`; `--fullscreen` and
  `--scale` remain.
* Menu themes now frame the panel with the theme board's own tiles: the most
  common blocking tile (class `flags & 7 == 6`) forms an 8 px ring around the
  panel and the most common walkable tile fills the interior, both 50% dimmed
  for legibility.  Falls back to the plain panel when the theme has no tiles.
* `OPTIONS -> GAMEPLAY` (was GAMEPLAY ENHANCEMENTS); the submenu title follows.
* Build clean; HUD suit default and profile loading warnings verified headless.

## 2026-11-08 (runtime profiles + mkprofile tool)

* Engine profiles are now runtime data.  `profiles/<id>.profile` is a
  `key = value` file loaded at start-up (`--profiles DIR`, default `profiles`);
  it supports `base = <id>` inheritance, per-kind overrides, item icons, the
  nine handler tables, EPIC score and object limits.  `profile_by_id` prefers a
  runtime profile and falls back to the compiled table, so a bad/missing file
  cannot brick a built-in game.  The shipped reference profiles are generated
  next to the manifests by `re/gen_games.py`.
* Shipped reference profiles are behaviourally identical to the compiled data:
  headless object dumps for ep1/2/3 (60 ticks, boards 4/3/4) match with and
  without `profiles\`.
* New `tools/mkprofile.py`: profiles a Jill-engine EXE with no manual RE.
  Detects MZ/image, kind-name block, kind-table initialiser (`c7 06` run),
  DGROUP image base + stored segment, the width/height/flags/tileset/count
  tables, the handler-table block, the item-icon table, the palette at
  `DGROUP:0x0194`, the nearest base profile, and (with `--data`) the board
  extension/list, `.sha`/`.dma` names, first/intro boards and a `.pal`.  Writes
  `profiles/<id>.profile` + `games/<id>.game`.
  - Validated on jill1: DGROUP `0x1cff0`, base `jill1`, 69 names, 0 spurious
    overrides beyond the 5 unused extra kinds; the generated game is
    object-identical to the compiled Jill 1.
  - Validated on Onesimus: DGROUP `0x1d850` (stored `0x1d85`, runtime `0x2d85`),
    base `jill3`, 69 kinds with **0 overrides**, handler block at
    `DGROUP:0xdd6`, palette found, 16 `.ONE` boards, first `MAP`, intro
    `INTRO`.  `--game onesimus --board MAP|6 --ticks 4` loads and renders.
  - `profile.cpp` ignores provenance keys (`handler_offsets`, `dgroup_base`,
    `board_ext`, `episode`) and accepts `board_ext` from a profile.
* `re/gen_games.py` now also emits `profiles/jill{1,2,3}.profile`, including
  `handler_offsets` and `dgroup_base` used by the tool for base selection.
* Docs: `docs/game-manifests.md` profile section + tool usage + Onesimus
  walkthrough; `port/README.md`; `TOOL_LOCATIONS.md`.
* Build clean.  Kinds 64..68 still draw statically (handlers are code).

## 2026-11-08 (game manifests / mod structure)

* The episode structure is now data-driven.  `port/src/game.*` adds a runtime
  `GameDef` (assets + board list + palette + suit index + HUD icon bank + the
  compiled `ProfileDef` it reuses), `Engine::init` takes that struct, and the
  three-episode assumptions in the front end are gone.
* New manifest format `games/<id>.game` (`key = value`): `profile` is required;
  `data`, `shapes`, `tiles`, `board_ext`, `first_board`, `intro_board`,
  `palette`, `suit`, `hud_icon_bank` and `boards` fall back to the profile.
  Parser/discovery in `port/src/game.cpp`; unknown games are skipped and
  reported, and an empty/missing directory falls back to the compiled episodes
  as built-in games.
* CLI: `--game ID`, `--games DIR`, `--list-games`; `--episode N` is now the
  N-th installed game (default first), `--data`/`--palette` override the
  manifest, and `--menu-theme` is clamped to the installed count.
* Front end: START GAME lists the manifests, F3 cycles games (not a fixed 3),
  MENUS/THEME lists `PORT` plus every game, the menu title uses the current
  game's title, and the level select/menu draw from `GameDef::boards`.
* Engine: board extension, first board and status text come from the active
  game; `suit` overrides the derived suit colour; item icons and health pips
  use `hud_icon_bank` (`Engine::icon_shape`).
* Generated data: `docs/games.json` + `re/gen_games.py` emit
  `games/jill1..3.game`; `ProfileDef` gained a stable `id` emitted by
  `re/gen_port_data.py` (`profile_by_id`, `profile_count`, `profile_at`).
  `run.cmd` now `cd`s to the repo root and no longer forces `--data`.
* Verified headlessly: `--list-games` shows 3 games; ep1/2/3 smoke runs are
  unchanged; a hand-written mod manifest in a temp directory
  (`profile = jill1`, `data` absolute, `boards = 2, map`) loads board `2.jn1`
  and ticks.  Build clean.
* Docs: new `docs/game-manifests.md` (format, discovery, Onesimus checklist),
  `port/README.md` run/menu/layout sections, `docs/episode-profiles.md`.

## 2026-11-08 (health-pip anchor, super-jump cheat)

* MODERN HUD health pips are anchored to a fixed slot (`kHpX = 203`) instead of
  being placed relative to the score, so the bar no longer shifts when health
  changes.  `SCORE` stays right-aligned independently.
* New cheat `SUPER JUMP` (Cheats menu row 2, `--super-jump N`): cycles
  `OFF, 1..3` and guarantees at least that many feathers (item 9).  The jump
  still runs the original `-(0x10 + 4 * itemcount(9))` formula, so found
  feathers above the selection help further.  `Engine::super_jump_` is
  preserved across episode changes.  (Superseded 2026-11-09: the range is now
  0..15 and `STUPID JUMP` has been removed.)
* `OPTIONS -> MENUS -> THEME`: `PORT` (plain) or `JILL 1/2/3`.  The episode
  themes render that game's `intro.jnN` front-end board into a cached 320x200
  buffer, dimmed with a 50% checkerboard so the menu stays readable.  The
  backdrop uses a second `Engine` with `render(fb, false)` (no HUD) and a fixed
  camera; selection is `--menu-theme N`.
* Renamed the Options row and submenu title to `GAMEPLAY ENHANCEMENTS`.
* TOKEN (kind 28) pickups are now honoured: `p13` is the inventory item id
  (1 key, 2 knife, 3 gem, 8 spin-blade, 9 feather, 10 shield) and the icon is
  `0x0E00 + item_icons[p13]`.  The morph tokens (0 and 4..7) only grant the
  inventory entry for now; the fish/spider/bird/frog forms are not ported.
* Verified: collecting a board-1 feather raises the jump from -16 to -20 and
  the HUD shows the red feather icon; super-jump levels 0/1/2/3 give
  -16/-20/-24/-28, STUPID JUMP gives -36, and `--super-jump 2 --stupid-jump`
  stays at -24 (super wins).  ep1/2/3 smoke tests clean.

## 2026-11-08 (HUD styles, background colours, knife cap)

* The MODERN top bar is now the default/minimal HUD; the original 64x84 panel
  came back as `CLASSIC` (`--hud`/F2 still toggles).  `render()` dispatches to
  `render_hud_modern` / `render_hud_classic`.
* `OPTIONS -> HUD` is a submenu: `STYLE: MODERN/CLASSIC`,
  `BACKGROUND: GREY/BLACK/DARK BLUE/DARK GREEN/DARK RED/BROWN/DARK PURPLE/SUIT`,
  `BACK`.  Background is an `Engine` palette index (`-2` = suit); the HUD menu
  draws a live sample bar at y=150.
* SUIT resolves the darkest shade of Jill's leotard from the player idle frame
  (tileset `0x08`, frame `0x10`; torso rows 11..18 / cols 4..11).  The palette
  indices differ per episode and are derived from the loaded shape bank: Jill 1
  dark green (28), Jill 2 dark maroon (58), Jill 3 dark blue (176).
* New enhancement `MAX KNIVES: 3/5` (off by default).  `Engine::max_knives_`
  gates the knife pickup check that was hardcoded to `< 3`.
* CLI: `--max-knives N` and `--hud-bg N|suit`; `apply_selection`/`start_episode`
  preserve the HUD style/background and knife cap across episode changes.
* Verified: `--max-knives 5` lets a fourth knife pickup land (`inv 4 -> 5`),
  default stays at 3 (`inv 4`); MODERN SUIT bars render green/maroon/blue for
  ep1/2/3, classic panel still renders.  Build clean.

## 2026-11-08 (top-row HUD + cheat menu)

* Classic HUD (`--hud`/F2) is now a single 12-px strip across the top of the
  screen instead of the 64x84 panel.  Left to right: `LEVEL n` first, one icon
  per distinct inventory id followed by the picked-up count, `HEALTH` pips, and
  `SCORE n` last (right-aligned to the screen edge).  Duplicate item ids are
  aggregated, so 5 knives show one knife icon and `5`; the icon art still comes
  from the per-episode `item_icon_table` (ids 1/2/3 -> 12x12 frames).
* Added `OPTIONS -> CHEATS` in the pause menu: `INVINCIBLE`, `DEBUG FLY`,
  `ADD KNIFE`, `ADD KEY`, `ADD GEM`, `REFILL HEALTH`, `BACK`.  The two toggles
  mirror F5/F4; item rows call the existing `add_item`, and `Engine::debug_heal`
  refills health to 8.  Options indices shifted (CHEATS=2, CONTROLS=3,
  ENHANCEMENTS=4, BACK=5); the new menu is also keyboard/pad navigable.
* Build clean; headless ep1/2/3 smoke tests pass and the new bar was rendered to
  PPM/PNG to check the layout (LEVEL first, item icon+count, HEALTH pips,
  SCORE last).

## 2026-11-08 (gamepad menus + inventory retention)

* Gamepad front end: `UPDOWN/LEFT/RIGHT` D-pad navigates the menus and the F3
  level select, A confirms, B cancels/backs out, and `START` opens the pause
  menu (or closes it again).  Menu handling moved into a shared `menu_nav`
  lambda driven by both keyboard scancodes and `SDL_CONTROLLERBUTTONDOWN`.
* New enhancement `RETAIN ITEMS BETWEEN LEVELS`, off by default.  Default
  behaviour now follows the original: `process_pending_warp` keeps health and
  score but clears the key/weapon inventory (header `0x04` count and `0x06`
  ids) after a board warp, including `!` level exits.  The enhancement keeps
  the live inventory; `--retain-items` starts with it on.
* `Engine::status()` now reports `inv=N`; added `--items N` (debug: start with N
  knives) so inventory-dropping can be headless-tested.
* Verified: map -> 1.jn1 with `--items 2` gives `inv=3` on the map and `inv=0`
  after the warp by default, `inv=3` with `--retain-items`.  Build clean.

## 2026-11-08 (control rebinding + gamepad)

* Added an `OPTIONS -> CONTROLS` screen (`app.cpp`).  `Bindings` holds two
  keyboard scancodes per action (slot 2 may be `SDL_SCANCODE_UNKNOWN`) plus one
  SDL game-controller button.  Enter on a row captures the next key or pad
  button (Esc cancels), labels show `KEY1/KEY2 [PAD]`, and `RESET DEFAULTS`
  restores arrows/AD/WS/ZX defaults.  Bindings are runtime-only.
* Replaced `key_state` with `poll_input`, which reads the bound keys plus the
  gamepad.  Pad movement is left-stick (deadzone 8000) and the D-pad.
* Enabled SDL game-controller support: `SDL_INIT_GAMECONTROLLER |
  SDL_INIT_JOYSTICK`, explicit `SDL_HINT_XINPUT_ENABLED` /
  `SDL_HINT_DIRECTINPUT_ENABLED`, controller hotplug re-scan, and a raw
  `SDL_Joystick` fallback (axes 0/1, hat 0, buttons 0/2) for pads SDL does not
  map.  Devices are closed on exit.
* Headless smoke tests still pass for episodes 1/2/3.

## 2026-11-08 (main menu + enhancements)

* Added an SDL front end menu (`app.cpp`): MAIN MENU (RESUME / START GAME /
  OPTIONS / QUIT), EPISODE SELECT (Jill 1/2/3, starts on the profile's
  `frontend.first_board`), OPTIONS (HUD classic/minimal, fullscreen), and
  ENHANCEMENTS.
* First enhancement: `FIRE WHILE CROUCHED`.  `Engine` keeps the last non-zero
  player facing (`last_facing_`); when the enhancement is on and the player is
  crouched (state 0, `dy == 3`, `p1b == 0`) the weapon check uses that facing.
  Left/Right while crouched now change `last_facing_` instead of standing up
  and walking, and the weapon spawns at `y + 0x12` (the hand height of crouch
  frame `0x3D`) rather than the standing `y + 2`.  Off by default;
  `--fire-crouched` starts with it on.  The option survives episode changes.
* Esc in game now pauses and opens the main menu; Esc at the main menu resumes
  (or quits before a game has started).  The F3 level selector still works as a
  debug tool and now also marks the game as started.

## 2026-11-08 (debug invincibility, health bar, map start, CHECKPT warps)

* Debug invincibility: `Engine::invincible_` short-circuits `player_kill`;
  F5 toggles it, `--invincible` starts with it on, and `status()`/the SDL title
  show `INV` (plus a small on-screen marker).  Verified headless: ep1 board 2
  spikes take hp 6 -> 0 without it and leave hp 6 with it.
* Minimal HUD now uses the classic health-bar art on its own: shapes `0x0E2A`
  (segment) / `0x0E2B` (end cap) over a 32x10 grey strip, same spacing as the
  original panel.  `--hud`/F2 still show the full panel.
* Jill 1 now starts on `map.jn1` like Jill 3: `docs/profiles.json`
  `frontend.first_board` updated and `gamedata.inc` regenerated.  `docs/episode-profiles.md`
  notes that this is a port choice (the original first board is inferred).
* CHECKPT 12 hit + warp queue implemented (`Engine::pending_warp_`,
  `process_pending_warp`, `place_at_checkpoint`):
  - bare board string: load it, keep the live header, set the level id to the
    checkpoint's `p13`, then place the player on the target board's entrance
    CHECKPT (`x & ~7`, `y - 0x10`; TINY keeps `y`);
  - `!`: load `profile.first_board`, place at the matching entrance and kill
    it (completed marker);
  - `*`, `#`, `&`: consumed as no-ops (no music/macros).
  Verified: map.jn1 entrance p13=1 -> 1.jn1 player at (112,192); `!` from 1.jn1
  -> map.jn1 player at (352,192) with the p13=1 entrance killed; ep3 map.jn3
  entrance p13=1 -> 1.jn3 player at (272,192).
* TINY 23 is now a player kind: `has_player_` accepts 0 or 23, `tiny_update`
  moves it 2 px/tick with the `0x200` walkable mask and advances the `p11`
  animation, and `render` draws `0x10<<8 | frame` at `y+2`.  `map.jn3` is
  playable and renders without crashing.
* Added `--level N` (headless) to override the header level id for exit tests.
* Build clean; headless regressions re-run for default ep1, map warps, `!`,
  ep3 map warp and the spike/invincibility pair.
* Object record loader bug fixed: `x/y/dx/dy` are i16 (docs #6.1) but were
  read as u16, so every loaded negative velocity became `65534`.  `_crawl`
  then computed `x + dx` as e.g. `1072 + 65534`, clamped to `x = 2000` and the
  enemy appeared to teleport/freeze.  The loader now sign-extends those four
  fields, and the `state` byte is read as `int8_t` (so `0xFF` stays `-1`, used
  by PAD/BUTTON-style handlers).  Verified: ep1 board 2 ANT 43 now walks
  `1072 -> 1066` over 3 ticks with `dx = -2` instead of jumping to 2000; 400
  ticks leave every walker position sane.
* Item 3: with the loader fix the walkers keep their intended velocity and
  facing.  The port also exempts spike/thorn tiles from the `_crawl` "hole"
  test so crawling enemies cross them instead of pacing at the edge; this is
  a deliberate deviation from the original (the original treats them as
  holes).  Needs a player-side confirmation of the exact enemy if anything
  still looks stuck.

## 2026-11-08 (enemies, hazards, platform branches)

* Ported the enemy and hazard handlers (kind 4, 5, 7, 8/22, 9, 10, 11, 17, 18,
  19, 29, 30, 35, 38, 43, 47) with draw, update and hit.  ZAPPER 10 is the
  "metal spikes" hazard: static 5-frame animation, `_playerkill(0x10, 2)` on
  contact.  All movement goes through the existing `_justmove` / `_crawl` /
  `_moveobj` services, so enemies stay on floors and reverse on walls exactly
  like the original.
* Ported the platform and trigger branches: PAD 15 (`_sendtrig` 3/4/5 per
  state), DOOR 24 (key/gem, `0x0BE` gem tile path), FALLDOOR 25 (drop then tile
  write), BRIDGER 26 (build `0x1B0`/`0x8C` runs, erase restoring the first
  solid perpendicular neighbour, `[inferred]`), SWITCH 32, MEGA 42, BUTTON 52.
* DEMON 7 was re-read from `1644:2974`: it walks with `_justmove` and spits
  FIREBALL 18 via `_vectdist(player, self, 4)`, not `_crawl`.  `docs/jill1-kind-handlers.md`
  corrected; the port follows the decompiled code.
* Added the generated per-episode `HandlerTables` (FLY, PHOENIX, DEMON, FATSO,
  EXPL2, BAT frame/offset lists) to `docs/profiles.json`,
  `re/gen_port_data.py` and `port/src/gamedata.inc`; the three episodes are
  identical, only the DGROUP delta differs (0 / 156 / 428).
* `_explode2` centres a fixed 16x32 EXPL2 (the kind-37 size), matching
  `1A41:0930`'s use of the 0x10/0x20 constants.
* Added `--dump-objects FILE` for headless motion checks.  Verified: PHOENIX
  patrols on board 1, ANT/CLOUD walk on board 4, the player walks and picks up
  apples, and contact damage lands on ep2 boards 1 and 9.  Only on-camera
  objects update, so boards whose enemies start off screen show no motion.
* Build clean; headless runs complete on ep1 boards 1/2/3/4/6/9/50, ep2
  0/1/3/9 and ep3 1/5/12.

### Hazards and STALAG release

* `FUN_26f4_23e1` (tile contact scan) and `1F28:0006` mode 1 are now in
  `Engine::tile_touch` / `Engine::tile_hook`: `SPIKE` `0xA6`,
  `STALACBR/STALAGBR` `0x141/0x142`, `WTHORN(2)` `0x155/0x156` and the water
  surface call `_playerkill(0x10, 2/1)`; lava `0x186..0x18B` also spawns
  `EXPL2`; water body `0x198..0x1A3` only plays the throttled splash.
* Re-read the argument order at those call sites: the amount is pushed last,
  so the hazards deal **16** damage (`_playerkill(0x10, 2)`), not 2.  Same for
  the water surface (`_playerkill(0x10, 1)`).
* STALAG 38: trigger actions 3/5 set `dy = 2` + `sound(3, 0x2F)` and kill the
  sender pad; action 4 is a no-op.  The update tries to move first, then adds
  2 to `dy` (cap `0x10`); a blocked move plays `sound(2, 0x1B)` and kills it.
* Fixed `Engine::cando`: the bottom-edge `extra = 2` test uses the object's
  **current** y, not the candidate y (`26F4:2227` reads `DGROUP:0x96AF`).  With
  the old code the player sank through `BRIDGE` tiles and drowned at the ep3
  board 12 spawn; it now stands on the bridge.
* `_playerkill` now spawns the `_explode1(x, y, 10)` burst.
* `Engine::tile_prepass` now runs the mode-2 decorative hook: lava
  `0x186..0x18A` advances one tile id every 4th frame (wrapping after `0x18A`),
  which is the original's 5-frame lava animation.  The port draws the whole map
  per frame, so the cell write is enough.
* Added `--start X,Y` (headless, with `--ticks`) to teleport the player.
  Verified: ep1 board 2 spike kills the player (hp 6 -> 0, death state 5),
  ep1 board 9 spike too, ep1 board 1 PAD sets the matching STALAG 38 falling
  and both objects die (`kind 3`), and the 15-board spawn regression list keeps
  `hp=6`.

## 2026-11-07 (no mirror bit, masked HUD icons)

* Fixed the "flipped graphics" reports (trees, arrows, level numbers, rocks,
  vines, beam ends).  Bit `0x4000` in a shape id is **not** a horizontal-flip
  flag: it is bit 6 of the tileset byte (tilesets `0x40..0x7F` select .sha
  entries `0x00..0x3F`).  `1123:00eb` XORs it out before the entry lookup and
  the 8bpp blitter (`11f4:0563`) never reverses pixels; the mode-3 pen table is
  the identity table (`1123:0674` writes `DAT_2cff_3832[i] = i`).  Directional
  art uses separate frames (e.g. player `0x0800|frame`, CROC head/tail tables).
  The port no longer mirrors any shape and no longer auto-flips objects with
  `dx < 0`.
* HUD inventory icons now draw masked (colour 0 transparent) against the grey
  panel instead of painting a black box, matching the original blitter's
  transparent pen.  The health pips are masked the same way.
* Docs corrected: `jill1-kind-handlers.md` (drawshape row, ZAPPER) and
  `jill23-new-kinds.md` (MAPDEMO tile runs) now describe the bit as part of the
  tileset byte.  `shapes.h` comment updated.

## 2026-11-07 (level select + debug fly)

* Added a debug level/episode select (F3).  The overlay lists the episode's
  shipped boards (`frontend.board_list`, now generated as `kBoards1..3` and
  exposed through `ProfileDef::boards/board_count`), switches episodes with
  Left/Right (rebuilding the Engine because shape/tile/kind data is
  per-episode) and loads the chosen board with Enter.  The world pauses while
  the menu is up.
* Added debug fly (F4): arrows/WASD reposition the player 4 px per tick with no
  collision or gravity, clamped to the 128x64 board, so elements deeper into a
  level can be reached for testing.  A `FLY` marker shows while it is active.
* `Engine::draw_text` is now public so the app can draw the overlay; added
  `ProfileDef::boards/board_count` to the generated profile data.
* Docs/README updated.

## 2026-11-07 (HUD option + EPIC)

* HUD art (`22ab:05dd`) is now an option: `--hud` (and F2 at runtime) draws the
  original panel - health pips `0x0E2A`/`0x0E2B`, `HEALTH`/`SCORE`/`LEVEL`
  labels and values, and the inventory icons from the per-episode
  `item_icon_table` (DGROUP `0x0ED4`/`0x0F70`/`0x1080`, emitted as
  `kItemIcons1..3`). Positions are the literal draw-call arguments; the glyph
  colour ramp is approximated (level 3 pixels are skipped).
* EPIC (kind 49) is now rendered with its 8-frame spin (`0x28 << 8 |
  frame & 7`) instead of a static frame, and its hit handler is ported:
  `_addscore(2)` per touch, explode after 11 touches
  (`explode1(x, y, 10)` + `_killobj` + sound 3/0x30). Jill 3's `_addscore`
  value (25) comes from `ProfileDef::epic_score`.
* Added `explode1` (`1a41:09a8`) and a deterministic RNG stand-in for
  `FUN_2ca6_0015`; EXPL1 (kind 36) now animates and falls as documented
  (`1a41:0a3b`), so the EPIC burst is visible.
* `re/gen_port_data.py` now emits the item-icon tables and the EPIC score;
  `docs/profiles.json` gained `anchors.item_icon_table` and `epic_score`.
* Docs: HUD section 14 rewritten with the verified coordinates, table offsets
  and colour handling; `episode-profiles.md` records the new table anchor.
* Verified headless with a synthetic board (EPIC next to the player): the
  8-frame spin advances between ticks 1 and 4, and the HUD option renders the
  panel, pips, labels, score/level and inventory icons.

## 2026-11-07 (knife spawn check + return)

* Player knife throw now validates the spawn cell (`_cando(obj, x, y, 1) == 1`)
  before setting `dx`/`p11`; if the cell is blocked the object is killed and
  the knife is refunded with `sound(2, 8)`, matching `1FAE:0x0264`.
* `move_obj` (`26F4:1C89`) backs the knife flight, so blocked axes use the
  `_trymove` fallbacks (mask 1 for horizontal/up, mask 3 when moving down)
  instead of dropping the knife immediately. It keeps trying and homes back.
* Docs: `docs/jill1-player-physics.md` throw pseudocode updated with the
  spawn check and refund path.

## 2026-11-07 (pickups + knife)

* Ported the pickup/projectile handlers from `docs/jill1-kind-handlers.md`
  (all addresses verified against the decompiled code):
  * APPLE (1): health +1 capped 8, `_addscore(count = 12)`, kill, bob.
  * KNIFE (2): straight flight for `p11` 3..14, then homes on the player
    (`_pointvect`, dx +/-8, dy -4..+4), falls after `p11 > 0x40`; the hit path
    re-adds it to the inventory (only if fewer than 3 and `p11` is outside
    1..10) and kills kind flags `0x0080` enemies with `_killaward`.
  * KEY (14): adds inventory item 1; GEM (33): adds item 3 + `_addscore(23)`.
  * SCORE (27): decimal digits from font bank 4 (`0x400 + digit`), falls and
    expires after 17 ticks; `_killaward` uses the kind table "default count"
    (`KindInfo::count`, DGROUP:`0xBBC8`) as the score value.
* Hit pass is now symmetric and only marks `0xC000` when the handler returns
  non-zero; pairs where both objects carry flag `0x0008` are processed once.
* `_killobj` sets `kind = 3` (KILLME) and `flags |= 0xC000`; object storage is
  reserved to the engine cap so appended popups cannot invalidate references.
* Sprite/draw fixes: GEM uses the kind tileset (`0x09<<8 | p13/2+4`), KEY uses
  the absolute `0x0E06 + p13/2`, SCORE digits use shape bank 4.
* Docs/README updated (gem shape was wrong, apple update return, kscore table).
* Verified with a temporary synthetic board: apple `hp 6 -> 7, score 0 -> 12`;
  thrown knife leaves the inventory and kills a phoenix (score +8, kind 3).
  Build clean; all three episodes still tick headless.

## 2026-11-07 (later)

* Player draw frames fixed against the disassembled handler (`1FAE:0x0264`
  state 0, `0x04BD` state 2, `0x0599` state 3, `0x05BA` state 4, `0x0673`
  state 5).  The handler starts with `DI = DGROUP:0xBB3E[kind] << 8` (0x800 for
  the player) and each state adds its offset, so the walk cycle is
  `(p0f & 7) + 8*(dx > 0)` (`0x00..0x07` left, `0x08..0x0F` right) and not the
  climb poses.
  * `0x11D0` (`18 18 19 1A 1A 19 19`) is the **climb** animation used by
    state 3; `0x11DE` (`48..4A`) is the **death** animation used by state 5
    only (`p0f == 2` variant, indexed by `p11`).
  * State 0 idle is `0x10` (`0x11` after `p11 > 0x96`), the just-stopped pose
    is `0x14/0x15`, the landing pose is `0x24/0x2C` (`0x3C` with no facing)
    with `+1` at `p11 == -4/-5`, jump `0x13`, fall `0x12`, `dy == 3` -> `0x3D`.
  * `docs/jill1-player-physics.md` #4.1 table and #7 draw table corrected
    (they had the walk and climb tables swapped).
* Verified: build clean, all three episodes still tick headless.  Sprite sheet
  check is `re/player_check.png` (walk `0x00-0x0F`, idle `0x10/0x11`, climb
  `0x18-0x1A`, jump `0x20-0x24`, landing `0x3C`).

## 2026-11-07

* New `port/` - native C++17 + SDL2 skeleton, built and verified with MSVC
  2022 and vcpkg (`sdl2:x64-windows@2.32.10#1`).
  * `port/src/profile.*` + generated `src/gamedata.inc`: per-episode board
    extension, shape/tile files, start board, 64/69-entry kind tables and the
    documented DGROUP anchors.  Regenerate with `py -3 re/gen_port_data.py`.
  * `board.cpp`: `.jn1/.jn2/.jn3` parser - 0x4000 map, object count, 31-byte
    objects, 0x46 header, trailing per-object strings, EOF check.
  * `tiles.cpp`: `jill.dma` loader (shape + `0x4006 ^ attr` flags).
  * `shapes.cpp`: `.sha` directory + frame decoder for 2/3/4/8 bpp, opcodes
    0/1/2, colour-table translate (mode 4).
  * `engine.cpp`: active list (player first, visible test with the original
    +/-0x60/+/-0x30 margins), update pass, hit scan, `cando`, `set_origin`,
    `try_move`, `just_move`, item add/delete, `player_kill`, respawn, camera
    follow, tile pre-pass stub, tilemap + sprite render, minimal HUD.
  * `player.cpp`: kind 0 states 0-5 and shared tail (walk, gravity, jump,
    climb, death/respawn, spinblad/knife throw), draw frames.
  * `app.cpp`: SDL window/texture, 18.2065 Hz fixed timestep, nearest/integer
    scaling, keyboard mapping, F11/R/F1, headless `--ticks` and `--dump PPM`.
* Build/run: `build.cmd` (first time `build.cmd install`), `run.cmd`;
  usage and limitations in `port/README.md`.
* Verified: all three episodes load their start board and tick 30+ times
  headless (`--ticks`), and episode 1 board 1 renders a full frame
  (`re/port_ep1_board1.png`).  Map `0xC000` bits are stripped on load (they are
  redraw flags in the shipped files, not collision), and boards without a
  player (`map.jn3` world map) render statically instead of failing.
* Palette solved: DGROUP:`0x0194` is static data in the load module (nothing
  writes it except the unused 64x12 shape case), so the initial 768 bytes are
  the default game palette.  It is 6-bit RGB, identical in all three
  executables, and is the usual 16 EGA + grey ramp + 6x6x6 cube, *not* the
  standard VGA ramp.  `re/gen_port_data.py` now emits `port/src/palette.inc`;
  episode 1 board 1 renders in the same colours as the DOSBox reference
  (`re/port_ep1_board1.png` vs the caller's screenshot).  Documented in
  `docs/jill1-engine.md` 4.1/9.4.
* Still `[open]`: per-kind handlers beyond the player, HUD art, sound, front
  end, demos, saves.

## 2026-11-06 (later 6)

* `docs/jill1-player-physics.md` - new document: per-frame order of `_play`,
  the `FUN_26f4_12b9` update pass (active list, `0xC000` semantics, symmetric
  hit pass, footprints, final sweep), handler ABI, the player record, the
  6-state machine with pseudo-code, tile masks/classes, draw-frame summary,
  idle-caption timer, determinism notes and the J1/J2/J3 offset table.
* Corrections to older notes:
  * `FUN_26f4_11d7` is a **tile pre-pass** (tiles whose attribute has bit
    `0x20` run the `1F28:0006` hook and may get `0x4000` OR-ed in), not input
    handling.
  * kind 0 state 2 = airborne/water/ladder grab, state 5 = death (not
    ladder/water).
  * `DGROUP:0xBAB0` is the camera auto-scroll step written by `1FAE:000D`, not
    an air/bubble timer.
  * the update pass starts the active list with the player
    (`visible_count = 1`, `active[0] = 0`).
* New facts:
  * `DGROUP:0xBE2C` = 6-word state-flag table `{1,0,1,0,2,2}`, reset by
    `25D3:0000` (linear `0x25E60`); bit 0 = may attack, bit 1 = invulnerable /
    skip tile events.
  * `_playerkill(26F4:05E7)`: shield item 10, TINY kind 0x17 immune, state 5,
    `p0f` = direction, `dy = -0xC`, respawn at `p11 >= 0x14` -> health 6 ->
    `FUN_26f4_04c7(1)` -> state 4. `dir = 2` only from ZAPPER kind 10.
  * Jump `dy = -(0x10 + 4*itemcount(9))`, gravity `+2` capped `0x10`, walk
    8 px/frame, landing `p11 = -4/-7 + (moving)`, climb steps
    `{4,0,0,6,4,4,0}`.
  * Weapon tail: SPINBLAD (kind 0x32) if count < `itemcount(8)`, else KNIFE
    (kind 2) consuming item 2 and setting `p11 = 3`, else a dry click.
  * Pose tables `0x11D0/11D7/11DE` are byte-identical in J1/J2/J3 (J2
    `126C/1273/127A`, J3 `136A/1371/1378`).
  * Tile classes from `jillt/jill.dma` (`0x4006 ^ attr`): `0x6` solid, `0x7`
    open/hint/water, `0x3` WHITECLIMB, `0x1` VINEBR, `0x5` BRIDGE; mask 4 =
    "no climb tile present".
* Cross-linked from `jill1-engine.md`, `jill1-kind-handlers.md`,
  `jill2-differences.md` and `jill3-differences.md`; the two stale statements
  above were fixed in place.

## 2026-11-06 (later 5)

* `docs/episode-profiles.md` + `docs/profiles.json` (`re/gen_profiles.py`) - the
  single per-episode constant table the port should consume: exe/image/module
  sizes, DGROUP segments and bases, kind count and all 9 kind-table bases, every
  board/object anchor, `_sound` entry and remap table, asset names, board lists,
  demo and title tables, save prefix.
* New values found while building it:
  * Jill 1 level-title table: image `0x1F481`, DGROUP `0x2491`, 36 entries,
    strings from `0x2549` (same `index = level + 4` scheme; entries 0-3 are the
    four demo captions). Jill 2/3 table DGROUP offsets are `0x2229` / `0x2315`.
  * Jill 2/3 camera far pointer: `0xB52C` / `0xB28E` (`les bx,[slot]`, then
    `[bx+8]` for x; 108/60/95 such loads per image).
  * Jill 2/3 pending level-transition string: `0xB858` / `0xB5FA`, i.e. object
    count + 2, each tested by exactly one `cmp byte [slot],0` in the main loop.
* JSON cross-checks pass: `map+0x4000=header`, `header+0x46=tile table`,
  `tile+600*8=object array`, `object count+2=pending string`,
  `frame counter+2=kind flags`, `dgroup base = stored seg*16`,
  `title table = image - dgroup base`.
* Cross-linked from `jill1-engine.md`, `jill2-differences.md`,
  `jill3-differences.md`.

## 2026-11-06 (later 4)

* `docs/shared-handler-equivalence.md` - compared every base kind handler across
  episodes. Jill 1 vs Jill 3: 62/64 identical mnemonic streams, 61/64 identical
  small-constant multisets; Jill 2 is a rebuild (codegen only). Real deltas:
  DEMON 7 (`obj[0x1B]` guard), TOKEN 28 (ROMAN/DEMON link and `obj[0x13]==6`),
  EPIC 49 (`_addscore` 2 -> 25). Tools `re/hcmp.py`, `re/hsig.py`.
* `docs/jill23-frontend-flow.md` - boot assets, identical menu prompt and keys,
  demo tables (J2 3/9/17 + `jn2dem*.mac`; J3 1/5/12 + `jn3dem*.mac`), the
  CHECKPT-derived level graph (J2 hubs 1/10, finale 20; J3 `map.jn3` hub,
  13 -> 14 ending), the 36-entry level-title table (index = level + 4) and
  save/restore naming.
* `docs/jill23-sounds.md` - `_sound(chan,id)` ABI, the shared 50-byte id remap,
  all 45/46/47 call sites by kind, FLAG/ROMAN additions, BUTTON merge, VCL bank
  counts and the intentionally silent ids `0x11`/`0x25`.
* Cross-linked the three new docs from `jill1-engine.md`,
  `jill1-kind-handlers.md`, `jill2-differences.md`, `jill3-differences.md`.

## 2026-11-06 (later 3)

* Documented the five Jill 2/3-only kinds in `docs/jill23-new-kinds.md`:
  * **EYE (64)** - two-shape draw (body + pupil); update runs `_vectdist(0,obj,&dx,&dy,3)`,
    forces `dy` to +/-1, clamps `dx >= -2`, and returns 1 only when the pupil moved.
  * **VINECLIMB (65)** - bobbing hazard; `p13=(p13+1)&7`, `dy=2` on phase 0, `_cando(...,4)`
    drives reversal / `_setorigin`; contact pushes the player 8 px sideways, adds `0x0F`
    to the player parameter word and calls `_hitplayer`.
  * **FLAG (66)** - six-tick one-shot pickup; frame `p13/2+1`, hit path fires
    `_addscore(6)`, `_sound(2,0x20)`, `_explode1(...,5)` once via the state byte.
  * **MAPDEMO (67)** - viewport-pinned overlay; two 2-word DGROUP tables
    (J2 `0x10FC`/`0x1100`, J3 `0x120C`/`0x1210`, both `[0x10,0x14]`/`[4,3]`) drawn
    flipped via `|0x4000`, guarded by video-mode byte `DGROUP:0x34EB`.
  * **ROMAN (68)** - ground walker; 8-step animation, 1-in-30 `_pointvect` re-aim,
    `_crawl` with direction reversal on block, footstep `_sound(1,0x11)`.
* Confirmed Jill 2 and Jill 3 handler code is instruction-for-instruction identical for
  these five kinds; only handler module addresses and DGROUP table offsets move.
* Placement: J3 boards use EYE 3x, VINECLIMB 21x, FLAG 21x; MAPDEMO/ROMAN unused in
  both episodes.
* Open: ROMAN art (`.sha` entry 44) has 7 frames but the handler requests 8-15 when
  facing right; writer of `DGROUP:0x13A8` (map-demo pin flag) not identified.
* Cross-linked from `jill1-kind-handlers.md`, `jill2-differences.md`,
  `jill3-differences.md`.

## 2026-11-06 (later 2)

* Finished the last two handler unknowns from raw 16-bit disassembly:
  * **BRIDGER (26, `1644:2503`)** - a pad-driven bridge builder. `dx==0` picks tile
    `0x8C`, otherwise `0x1B0`; actions 4/5 erase/build a run of map cells along
    `(dx,dy)`, action 3 toggles and kills a PAD sender; draw is shape `0x123`
    behind `DGROUP:0x120E`.
  * **KNIGHT (44, `1a41:1bb4`)** - 11-frame swing (`0x2400..0x2404`), `dx` is a
    hold-frame selector (0 idle / 6 lethal / 0x0A animate), frame 6 deals 16 damage
    ("The knight slices Jill in half"), immune to `kflags & 0x4000` projectiles with
    a one-shot taunt. Trigger codes 4/5 = idle / freeze.
* Documented that actions 3/4/5 are the `_sendtrig` codes, not extra protocols.
* `docs/jill1-kind-handlers.md` is now complete: 64/64 kinds, 37.9 KB, ASCII-only.

## 2026-11-06 (later)

* Wrote `docs/jill1-kind-handlers.md`: the handler call protocol, a verified table of
  ~30 shared object routines (addresses), the object-field legend, and per-kind notes
  for **all 64 kinds** (58 distinct handlers; only BUNNY/FROG share one).
* Player (kind 0) decoded from disassembly: 6 draw states (`1fae:0x258`) and 6 update
  states (`1fae:0x868`), input globals `3C76/3C78/3C7A/3C7E`, the four idle-taunt
  strings at DGROUP:`0x2491`, and the `p0f`/`p11`/`p13` field roles.
* Kinds 28/38/42/48/56 (no Ghidra output) recovered by hand; their real entry points
  are `1a41:000f`, `1a41:0c54`, `1a41:165a`, `1a41:269f`, `1fae:298b`.
* Still open: BRIDGER (26) and KNIGHT (44) decompile unusably; PLAYER states 2/3/5
  (`p0f` 2 vs 6) need a targeted disassembly pass.

## 2026-11-06

* Wrote `docs/jill1-engine.md` - full engine analysis: EXE/overlay layout, DGROUP map,
  boot flow, main loop (+cheats/timing), video backends and blitter, board/tile map,
  object record + all 64 kind handlers, `.jn1`/`.dem` board format (verified: every
  shipped board parses exactly to EOF), `jill.dma`, `jill1.sha`, `jill1.vcl`,
  CMF `.ddt`, sound/input/macros, HUD/menus/save, editor, open questions, porting notes.
* New helper scripts: `re/kindtable.py` (dumps the 64-entry kind table from the
  initialiser), `re/check_jn1.py` (validates board files end-to-end).
* Key correction vs earlier notes: object strings are **not** a separate trailing
  script table - each length-prefixed string belongs to an object whose
  `+0x17|+0x19` word pair is non-zero, and they follow the 70-byte board header in
  object order.
* Added `tools/capture.cmd` (DOSBox + ffmpeg desktop capture for visual verification,
  user drives the run).
* Board header fully mapped: 70 bytes at DGROUP:`0x83A6` = level, health, key count,
  `keys[16]`, 32-bit score, damage-flash flag, saved-score copy, plus a 22-byte dead
  block at `0x83D6` that is memset to 0 and never read (only address reference in the
  image is that memset). Confirmed against `JN1SAVE.0` and the three `.dem` demos.
* Shape format decoded (`FUN_1069_0271`, real extent 0x1069:0x271-0x94c): 12-byte
  header (frame count, 2/4/8bpp block sizes, source bpp, flags), optional 4-byte
  colour table with the per-video-mode shift/mask, then `u8 w, u8 h, u8 opcode`
  records (op 0 = raw 8-bit, op 1 = `bpp`-bit LSB-first, op 2 = empty). Every entry
  also carries one trailing record the runtime never decodes. New `re/shadec.py`
  parses all 63 populated entries with **0 mismatches** and exports PNG contact
  sheets (`python re/shadec.py 5`); the 2bpp and 8bpp size words match all 63 entries
  exactly, the 4bpp word in 47/63 (16 store an unrelated value; only the word for the
  active mode is used).

* Kind flag alphabet complete: the 64-entry table at DGROUP:`0xBE3C` only ever uses
  six bits, all now named - `0x0008` hit-scan participant (AABB overlap sends action 1
  to both sides), `0x0040` has an attached board string ("Text Inside"), `0x0080`
  killable by KNIFE/FIREBULLET, `0x0100` receives `_sendtrig`, `0x0400` SPINBLAD
  target, `0x4000` is an attack/projectile. Table of test addresses added to #6.2.
* `jill.dma` corrected: **no 8-byte header** - 320 records of
  `u16 tile_id | u16 shape_id | u16 attr | u8 len | name`, record 0 being the default
  id-0 slot (shape 0x4700, attr 0x0201, name "0"). The loader writes the 600-entry
  tile table at `0x83EC` as `+0 = shape`, `+2 = 0x4006 ^ attr`, `+4 = malloc(name)`.
* `jill1.cfg` decoded (`_loadcfg` 0x2340B / `_savecfg` 0x234E9): four sequential
  blocks 120 (names) / 40 (u32 scores) / 72 (restore-slot labels) / 22 (options).
  Defaults zero names+scores+slots and set `0x3DCE = 1` only when the file is
  missing/empty. The shipped file is truncated inside the 22-byte block (only 6 bytes:
  words 4,1,1).
* `.mac` format decoded and verified by a parser over all nine shipped files:
  `rec[0] = mask + values`, `rec[n] = time varint + mask + values`; `time` is an
  **absolute** tick of DGROUP:`0xBE3A` (base-128: `(v&0x7F)|0x80, v>>7`), mask bits
  0..4 map to `0x3C76/0x3C78/0x3C7A/0x3C7E/0x3C86`. No header. jn1demo.mac = 519
  records, last tick 4295. Playback loader at linear 0x1474D, recorder at 0x147F4,
  write-out at 0x1483B (the symbol names `_playmac`/`_recordmac` are swapped).
* Fixed the special-key table: `R` starts recording, `C` replays, `0` stops and
  writes the buffer.

Next: port skeleton (SDL window + 320x200 framebuffer + board/asset loaders).

## Jill 2 assessment (`docs/jill2-differences.md`)

* `jill2.exe` is a stripped build: 185,574 B = 7,168 header + 178,406 image, no
  Turbo Debugger block (Jill 1 appends 24,158 B of symbols). Re-extracted correctly
  with `e_cparhdr`; the earlier `e_cp*16` slice was off by 0x550.
* DGROUP shifted uniformly by -0x2C6 (base 0x2CFF -> 0x2D32): map 0x40E0, header
  0x80E0, tile table 0x8126, objects 0x93E6, kind flags 0xBB76, frame 0xBB74,
  object count 0xB856. Verified by identical reference counts at shifted addresses.
* Kind table = 69 entries (initializer 0x16717). Kinds 0..63 are data-identical to
  Jill 1 (w/h/flags/tileset/count/name); only handler segments moved
  (0x1644->0x1671, 0x1A41->0x1A6C, 0x1FAE->0x1FC1). New: EYE, VINECLIMB, FLAG,
  MAPDEMO, ROMAN. Shipped boards use EYE and FLAG only.
* `.jn2` boards parse exactly with the `.jn1` parser (0x4000 map, u16 count, 31-byte
  records, 0x46 header, length-prefixed strings) for all 11 boards.
* `jill2.sha` has identical directory/size tables; only entries 3, 7, 8, 25 differ
  (7 = terrain sheet, 8 = PLAYER sheet). `jill.dma` is shared. `jill2.cfg` is the
  same 254-byte format (fresh defaults). `jill2.vcl` same header, 24 SFX and only
  1 second-bank entry.
* 3 demos (jn2dem1-3.mac), `peaks.ddt`, `jn2save`, `/TEST` + `/NOSB` switches, and
  level titles built from fragments are the remaining runtime deltas.

Follow-up (done below): same treatment for `jill3.exe` (`re/img3.bin`, 16,856 B debug
tail), then a profile table covering all three episodes.

## Jill 3 assessment (`docs/jill3-differences.md`)

* `jill3.exe` = 204,174 B = 7,680 header + 179,638 image + 16,856 B Turbo Debugger
  block (unlike the stripped Jill 2). Its image is only 1,232 B larger than Jill 2's.
* Same 69 kinds as Jill 2, with identical names/dimensions/tilesets/counts and only
  two flag changes: DEMON `0x0480 -> 0x0000`, TOKEN `0x0000 -> 0x0008`. Handler
  modules moved to `0x1650` / `0x1A51` / `0x1FCA`.
* DGROUP base `0x2D75`, but the layout is re-derived, not shifted: map `0x3E42`,
  header `0x7E42`, tile table `0x7E88`, objects `0x9148`, object count `0xB5F8`,
  frame `0xBC16`, flags `0xBC18`, visible count `0xBCA8`.
* All 16 `.jn3` boards parse exactly with the `.jn1` format (levels 1-14 + `intro` +
  `map`). `map.jn3` returns after Jill 2 omitted it; boards place kinds 64/65/66
  (EYE/VINECLIMB/FLAG).
* `jill3.sha` keeps the directory format and all sizes except entries 57 (PAUL,
  10 -> 11 frames, 3,053 -> 6,128 B) and 58 (BUNNY, 737 -> 993 B); terrain, EPIC,
  SKULL, PAUL and BUNNY art was redrawn. `jill3.vcl` has 23 SFX and 3 bank-2 entries.
  `jill3.cfg` is identical to `jill2.cfg`.
* Runtime additions: `/NOSND`, `/SB`, `/DEMO`, sound-card detection/help text, and a
  new VOC/DIGI path (`DIGI.ASM`, `SBCRWIO.ASM`, `SCANCARD.ASM`, `_voc*`, `_Dac*`).
  Title text is "Jill Saves The Prince"; new fragment-based level names plus some
  Jill 1 phrases.
* 3 demos (139/138/159 records), `wild.ddt`.

Next: port skeleton + a three-episode profile table (asset names, kind count, DGROUP
offsets, board list, title table).

---

## Gameplay fixes round (frog / blade / PAC / flames)

* SPINBLAD (1A41:2B1A): vertical bounce snap is `(y+dy) & ~0x0F` (the`n  `+0x10 - b7a4` term cancels because b7a4 = 0x10); the vertical probes use`n  the object's current x after the horizontal `just_move`, and the final`n  `just_move(x,y)` deactivates a blade that ended inside a wall.
* PAC (1A41:3402): forward step accepted only when the next cell equals the`n  current tile; the two perpendicular turns and the reverse are each accepted`n  when their cell equals the current tile, otherwise the PAC stops.
* Flame traps (1F28:0006 mode 2): FFLOOR `(frame+col)&0x1F` and FROOF`n  `(frame-col)&0x1F` spawn FIRE (kind 0x1F) at phase 0x10, at`n  `row*16-0x20` / `row*16+0x10` with dy -1 / +1. WOODTORCH spawns when`n  `(frame&0x3F)==(col&0x3F)` at `row*16-0x18` with dy -1. All use sound`n  (3,0x1A). Headless checks on Jill 1 board 3 and Jill 3 board 9 confirm the`n  spawned FIRE objects and their dy direction.
* Frog: JILLFROG kind 57 hops on solid ground (dy -8 / -12) and falls under`n  gravity; enemy FROG kind 22 idles 17 ticks then hops toward Jill. Headless`n  placement on a solid floor confirms both.


## Blue fading blocks (BLINK 0x81)

* The blue blocks that fade in and out are tile 0x81 BLINK, not BLUE/BLUEHINT.
  Its 1F28:0006 mode-2 branch (Jill 1 0x1E0, Jill 2 0x1F61A) runs every 8th
  frame: phase = (frame>>3)&0xF, the tile's own shape word becomes bank-41
  frame [8,8,8,8,9,10,11,12,38,38,38,38,38,11,10,9][phase] (frame 38 is
  blank), and flags bit 1 is set only for phases 8-12.
* Flags bit 1 is the walk-through bit that the mask-3 mover (26F4:1E70, used
  while airborne) tests, so the block is solid while its art is visible and
  falls away while it is blank. The mask-1 helpers (just_move) are unaffected,
  which is why no tile-id swap was needed.
* Implemented in Engine::tile_prepass (writes TileDefs::at(0x81)); the frame
  table is identical in all three episodes (img 0x1E178 / 0x1E544 / 0x1EA72).
* New debug aid: --watch-tile N prints that tile id's live shape/flags every
  headless tick. Headless check, Jill 2 board 3: flags 0x4025 (solid) for
  phases 0-7 and 13-15, 0x4027 (passable) for 8-12; a player placed inside
  the block's cell stands on its bottom edge while visible and drops to the
  castle floor at row 58 when it fades.
---

## Front-end and save round (save/restore, splash, cheats, Jill 2 titles)

* Save/restore matches the original board-file layout: FUN_22ab_0c9a writes
  map (0x4000), object count, 31-byte objects, the 0x46-byte header, then
  u16 length + text + NUL for each object with a string. FUN_22ab_0aa5 reads
  the same block. Save names are prefix.0..prefix.5 (jn1save / jn2save /
  jn3save / onesave); save_prefix and save_slots are generated profile fields.
* Added Board::save, Engine::save_game/load_game, six-slot SAVE GAME / RESTORE
  GAME menu screens, and the original S/R keys (the jump table is CS:0x19DF:
  keys 1B/51/52/53/BB, handlers at CS:0x1A66/0x1A20/0x19F3). Ctrl+R keeps
  the debug board reload. The old default move-down binding S is dropped so
  S can save; custom bindings are untouched. Headless --save/--load verify a
  round trip.
* Splash: MENUS > SPLASH (OFF/JILL1/JILL2/JILL3/ONESIMUS) renders the selected
  game's intro page 12 at board (240,0); shown for 2.5 s or until input and
  saved as [video] splash.
* Level titles: numeric board names set the level id (3.jn2 -> 3), and the
  caption is recomputed after a CHECKPT warp restores the id. Jill 2 board 1
  now selects JILL GOES / UNDERGROUND.
* CHEATS: BECOME JILL/FROG/BIRD/SPIDER/FISH call the token morph path
  (items 0/4/5/6/7).
* MAPDEMO only draws/updates on the game's first (hub) board; ROMAN uses real
  right-facing frames 8-15 when present (Onesimus) and mirrors the 7-frame
  Jill 2/3 sheet otherwise.

---

## Gameplay enhancement and bug-fix round

* AIR JUMP BUFFER enhancement (default OFF): a jump pressed in player state 2 is
  consumed, so holding jump through a fall does not auto-jump on landing. ON
  keeps the original behaviour. `--air-jump-buffer` / `[gameplay] air_jump_buffer`.
* MINIMAP enhancement (default OFF): 128x64 one-pixel-per-tile board overview in
  the lower-right corner, with water/lava/spike colours, green checkpoints and a
  white player dot. `--minimap` / `[gameplay] minimap`.
* Menu rows tightened from 12 px to 9 px, with panel/footer heights derived from
  the item count, so the 14-row CHEATS screen fits on 320x200 again.
* Weapons/particles stopping: the original's post-draw FUN_26f4_2506 compacts
  the object array by dropping kind-3 (KILLME) records. Added
  Engine::compact_objects at the start of tick and raised the spawn cap to the
  original 0x100 records (board files still cap at 192). Verified with a
  pickup: object count 88 -> 87 after the kill is compacted.
* Hub map persistence: entering a level from the hub stashes map/objects/header;
  the `!` return restores that copy instead of reloading the board, preserving
  consumed entrances and other map state.
* SPIKE 0xA6 animation: the tile repaint path uses
  base_shape + ((frame >> 2) & 3); the port now cycles 0x5410..0x5413 every four
  ticks. Headless --watch-tile 166 on Jill 1 board 2 confirms the cycle.
* Docs: port-save-splash-cheats.md sections 6-11, plus the object-array and
  compaction correction in jill1-engine.md.

---

## Fish / frog / spider / splash / interpolation round

* Fish: p0f is a facing memory again (only written when dx != 0), so stopping
  before firing no longer resets the shot direction to right. Both directions
  verified headlessly (left bullet dx=-8 at x+0x18, right dx=+8 at x-8).
  Added GAMEPLAY > CENTER FISH SHOT (`--center-fish-shot`); headless bullet y
  is fish_y + (16-5)/2.
* Morph: 1FAE:2E61 keeps the feet planted (y += old_h - new_h) and aligns x to
  8 px. The 10-px frog / 16-px fish no longer float after morphing from Jill.
* Frog: p1b only while moving, state=1 airborne / state=0 after landing so the
  hop frames are drawn. move_obj now returns blocked when no horizontal move was
  requested, letting the frog snap to ((y & ~0xF)+0x10 - h); headless landing
  is y=950 on board 2 (bottom 960), then it auto-hops.
* Spider: original kind 55 is an empty stub with the empty shape entry 0. Added
  procedural 16x16 spider art and 2-px 8-way wall-climb movement. Reachable from
  BECOME SPIDER and the p13=6 tokens on Onesimus 17/28. The Jill 3 p13=6
  alternate-award path remains open.
* Interpolation: VIDEO > INTERPOLATE (default ON) is separate from HIGH
  FRAMERATE; only active when high fps is on. `--no-interpolate` / `[video]
  interpolate`.
* Splash: now two data-driven pages: the largest MEGA/EPIC logo cluster
  (publisher logo) and the top-most title text object containing the game's first
  word. Added Engine::render_at so arbitrary board regions can be rendered
  regardless of the demo player's position; menu theme also uses it.
  `--dump-splash P` writes P1.ppm/P2.ppm. Headless dump produced two distinct
  pages for all four games.


## Endings / Onesimus / map return / Jill Select round

* Jill 1 profile kind_count corrected from 64 to 69 after re-reading the
  initializer (it stores fields for kinds 64..68 as well).  `re/gen_port_data.py`
  now emits EYE/VINECLIMB/FLAG/MAPDEMO/ROMAN for Jill 1, which fixes the blue
  VINECLIMB sparks on Jill 1's chains/vines (headless board 1: blue pixels at
  the 1840/1904 chain columns).
* New `map_board` manifest key.  Onesimus now starts on `1.ONE` and keeps
  `MAP.ONE` as the hub; verified `--game onesimus --ticks 2` reports `1.ONE`.
* Hub return keeps the stashed player position and skips
  `place_at_checkpoint(0)`, so the Jill 1 map no longer teleports Jill to the
  p13=0 music CHECKPT at (0,16).  Camera is re-centred on the restored player.
* Endings: state=3 CHECKPT sets an ending-done flag consumed by the front end
  (returns to the main menu).  Added the `*.mac` parser/player
  (`start_macro`/`apply_macro_input`); `&end1.mac` and `&end2.mac` now replay.
  Headless board 50 shows the macro's jump/right records firing.
* Text viewer: Left/Right now advance one screenful ("box"), then the next
  non-empty page; footer is `PAGE n / SCREEN k/m`.  Up/Down still line-scrolls.
* Jill Select: GAMEPLAY > JILL LOOK / JILL COLOUR.  Reads the other episode's
  16-entry player colour table from its `.sha` and remaps player pixels at draw
  time (no second bank).  Verified Jill 1 default green, look 2 red, look 3 blue,
  Onesimus grey in headless dumps.  Saved in `jill.ini`; CLI `--jill-look`,
  `--jill-colour`.

## Folder cleanup and jill.ini paths round

* Removed the repository copies of the game data (`jillt/`), the old MSVC object
  files in the repository root, ad-hoc debug dumps (`f*.txt`, `m*.txt`,
  `p*.txt`, `pout*.txt`, `frog.txt`, `dump.txt`), `test_output/`,
  `test_output2/`, `test_sfx1.wav`, `__pycache__/` and the temporary `re/_tmp_*`
  scripts.
* Runtime asset directories now come from `jill.ini` `[paths]`:
  `jill1`, `jill2`, `jill3` and `onesimus` each map to a directory.  Missing
  keys keep the manifest's local default (`jillt` for the trilogy, `ONESIM` for
  Onesimus), so a fresh checkout can still use local folders.
* The shipped `jill.ini` points the trilogy at `D:/Emu/DOS/C/jillt` and
  Onesimus at `D:/Emu/DOS/C/ONESIM`; `--list-games` confirms the resolved
  directories.  The generator/tooling default paths were updated to match.
* `docs/games.json` and the generated `games/*.game` manifests keep the local
  defaults; `re/gen_games.py` falls back to `jillt`.

## HUD health colour

* Added `HUD > HEALTH` (Options > HUD) plus `--hud-health N|original` and the
  `[hud] health` ini key.  `ORIGINAL` keeps the shipped pip art; 0..15 repaint
  the health pips in that palette index in both the modern strip and the
  classic panel (`Engine::draw_health_pip`).
* The HUD menu preview now shows a "HEALTH COLOUR" swatch line.

## OPL2 + CMF music

* Added `port/src/opl2.*`: register-level YM3812 core (9 two-operator channels,
  four waveforms, ADSR + key scaling, feedback/connection, vibrato/tremolo and
  the five rhythm voices).
* Added `port/src/cmf.*`: CMF parser for `*.ddt` and the Onesimus
  `SONG*.ARK` tracks (header, 11-byte OPL instrument records, MIDI events with
  running status).
* Added `port/src/music.*`: MIDI-to-OPL sequencer (note on/off, programs,
  controllers 0x07/0x63/0x67/0x68/0x69, melody and rhythm channel mapping,
  loop at end of track) and `port/src/audio.*`: SDL audio device + offline WAV
  renderer.
* Board CHECKPT `*name.ddt` strings now start level music
  (`Engine::take_music_request`), and a new `title_music` manifest key plays a
  front-end track (`funky.ddt` / `peaks.ddt` / `wild.ddt` / `SONG5.ARK`).
* New `OPTIONS > AUDIO` screen (MUSIC ON/OFF, VOLUME), `[audio]` ini keys,
  `--no-music`, `--music-volume`, `--music-file`, `--music-seconds` and
  `--dump-music FILE` for offline verification.
* The map music checkpoint (p13 = 0) is now always allowed to fire and repeats
  are suppressed, so hub tracks start once instead of every tick.
* Docs: `docs/port-sound.md`; `title_music` documented in
  `docs/game-manifests.md` and `MODS.md`.

## Sound menu + audio device fixes

* Renamed the options screen to **OPTIONS > SOUND** and labelled its second row
  `MUSIC VOLUME: N%` (the first row is `MUSIC: ON/OFF`).
* SDL audio open is now tolerant: it allows the endpoint to return 44.1 kHz (or
  another rate/format) and rebuilds the sequencer at that rate instead of
  failing a strict 48 kHz request.  The callback handles 16-bit and float
  device formats and any channel count.
* Added `--audio-check`: opens the device, plays the title track for 1.5 s and
  reports device format, mixed frame count and peak level.
* Raised the OPL mix headroom (0.18 -> 0.30) so the shipped tracks sit at a
  comfortable level at the default 80% volume.
* `ProfileDef` now carries `title_music` (generated from
  `docs/profiles.json` frontend); built-in fallback games and manifests that
  omit the key still get the correct title track.

## Sampled sound effects

* Added `port/src/sfx.*`: `.vcl` bank loader (50 unsigned 8-bit mono PCM
  samples, 6000 Hz), the shared logical-id remap table, and a single priority
  voice that resamples to the device rate and mixes with the OPL music.
* `Engine::sound(priority, id)` now queues requests (bounded to 64 per tick);
  the app drains them after each tick.  The two intentionally empty samples
  (`0x11`/`0x25`) remain silent no-ops.
* `OPTIONS > SOUND` gained `SFX: ON/OFF` and `SFX VOLUME: N%`; settings are
  `[audio] sfx` / `[audio] sfx_volume`, plus `--no-sfx` and `--sfx-volume`.
* Headless runs print `sfx requests: N`; `--audio-check` now also plays an
  effect and reports its peak level.
* Docs: `docs/port-sound.md` now covers the sampled path.

## JillE total conversion limits

* Diagnosed the `object count out of range` failures on the JillE mod: its
  boards carry 245-258 objects, while `Board::load` still used the shipped
  engine's 192-object assumption.  The original loader actually reads any u16
  count straight into the 0x100-record array (no cap).  `Board::kMaxObjects`
  is now 384 and `Engine::kObjectLimit` follows it, so all 21 JillE boards
  load and run (verified 300 ticks each).
* F3 level select now scrolls after 16 rows instead of growing past the
  320x200 screen (JillE lists 21 boards).
* Added per-manifest `save_prefix` / `save_slots`; JillE sets
  `save_prefix = jillesave` so its saves no longer collide with Jill 1's
  `jn1save` slots.
* Save/load round-trip verified on a 258-object board.

## Display / modding round

* Doubled the kind-id ceiling (`ProfileDef::kind_limit()` = max(138, table)) so
  mods can carry ids beyond the compiled 69; unknown ids use the safe dummy
  metadata.  Doubled the visible/active object cap (`kActiveLimit` 0xC0 ->
  0x180) to match the 384-object pool.
* Added `OPTIONS > CHEATS > LEVEL SELECT`, which opens the same scrolling board
  picker as F3.
* Added `OPTIONS > VIDEO > BACKGROUND GRADIENT` (default OFF), a vertical
  82%-100% brightness ramp applied during palette conversion.
* Added `OPTIONS > VIDEO > FILTER` with `NONE / SMOOTH / CRT`:
  - SMOOTH: separable 3-tap blur blended 75/25 with the original.
  - CRT: 1-2-1 horizontal bleed, 72% scanlines and a radial vignette.
  Filters run on the 320x200 frame before upload (works with the software
  renderer too).  `--dump` renders the same path headlessly.
* Settings: `[video] gradient`, `[video] filter`; CLI `--gradient`,
  `--no-gradient`, `--filter 0|1|2`.
* Docs: `docs/port-display.md`.

## Sparks, entrance rise, paths and bindings

* Removed the BACKGROUND GRADIENT video option and its palette-conversion
  shader; the VIDEO menu is back to NONE/SMOOTH/CRT only.
* VINECLIMB (kind 65): the handler seeds `dy = 2` only when `dy == 0` (the
  earlier port tested `p13`), so the blue sparks now patrol the full climbable
  span up and down instead of jittering every eight ticks.
* Player state 4 (entry / respawn recover) now draws the sprite at
  `y + 0x21 - p11`, rising one pixel per frame out of the floor, with solid
  tiles redrawn over the lower half while she is still behind the ground.
  This restores the level-entrance rise animation.
* Launch directory: `jill.ini` is searched in the working directory, the
  executable directory and up to four parents; the working directory is moved
  to the folder that holds it, so running `port\build\Release\jill.exe`
  directly finds `games/`, `profiles/` and the configured asset paths.
* Per-game controls: `[controls.<game id>]` overrides any of the six actions
  over `[controls]`; editing CONTROLS writes the active game's section and
  removes it when it matches the base.
* Docs: `docs/game-manifests.md`; `MODS.md` example.

## Splash temporarily disabled

* Start-up splash and MENUS > SPLASH are hidden behind
  kSplashEnabled = false; --dump-splash still renders the old pages.
* Menu theme next step: use intro-board tiles (35,0)..(55,13) without the
  TEXT6/TEXT8 objects, as requested.

## Intro-board menu panel

* Menu THEME backdrops now render only the intro-board picture panel
  (35,0)..(55,13) (21x14 cells), centre-cropped to 320x200.
* Engine::set_hide_text_kinds(true) skips TEXT6/TEXT8 objects so the story
  and credits copy is not baked into the menu artwork.

## External enhanced assets

* New pack backend: `port/src/external.*` (scan/mount, zip via miniz,
  `pack.ini`, image decode + palette quantise, ExternalVisuals) and
  `port/src/extaudio.*` (OGG/MP3/M4A/WAV). Vendored `stb_image`,
  `stb_vorbis`, `dr_mp3` and miniz under `port/src/third_party`.
* Packs live in `<base>\external` as folders or `.zip`; one mounted at a
  time.  Select from `OPTIONS > ASSETS`, or `--pack ID` /
  `[external] pack`; `--external DIR`, `--list-packs`.
* Overrides: kind strips and single shapes, tile ids, whole-board
  backgrounds, extra menu themes, raw SFX slots and music tracks.
* Engine draws pack art through the 8-bit framebuffer; board backgrounds
  make tile colour 0 transparent so the picture shows through.  See
  `docs/port-external-assets.md`.
* Pack art now also covers the text font (`fonts/main.png` glyph strip or
  per-glyph files), HUD panels (`hud/classic`, `hud/modern`) and text-page
  backdrops (`pages/page_<n>`).  Overlays honour PNG alpha and keep the
  normal labels/icons/text drawn on top.
* Pack authoring: `EXTASSETS.md` (naming, manifest, transparency, zip
  packaging, testing) plus a working `external/ExamplePack/` covering every
  asset folder.
* Extractor: `jill_extractor.py` now exports the remaining data-driven
  assets - fonts (`fonts/main.png` glyph strip + per-glyph PNGs), VCL text
  pages (`.txt`, JSON, font-rendered previews), tile images from `.dma`,
  HUD panels/icons, intro menu theme, 8-bit palettes and full board previews.
  `Tools > Export Reference Asset Pack` and `extract_all.py --pack` write an
  editable external pack skeleton.
* Color fix: reference-pack boards/themes were mapped straight through the
  global palette instead of each shape entry's 4/8/16-colour translation
  table, and the engine colour-keyed opaque board/theme pictures on their
  top-left pixel.  Both paths are fixed; 320x200 board exports now use
  nearest sampling so every pixel remains an exact palette entry.
* Extractor music: reference packs now render every CMF track (`.ddt` and
  Onesimus `SONG*.ARK`) to `music/<stem>.wav` via the port's `--dump-music`.
  The Sound tab's Play Selected / Save as WAV now work for music rows;
  previously `.ddt` rows fell through and did nothing.  Added
  `--music-wavs` and `--music-seconds` to `extract_all.py`.
* Music routing: every board starts the first `*track.ddt`/`*track.ark`
  CHECKPT on load, and music CHECKPTs fire even when their p13 matches the
  current level id.  Added `docs/music-catalog.md` with the per-board start
  and section tracks for all four games.
* Audio: fixed OPL2 hiss by only using the LFSR for rhythm voices while
  rhythm mode is enabled (channels 7-9 were noisy otherwise).  Added
  PC SPEAKER mode (`[audio] pc_speaker`, `--pc-speaker`, SOUND menu), a
  1-bit square-wave rendition of sampled effects.
* Dialog portrait: level-title/dialog boxes now draw a 64x84 character
  portrait beside them.  The default is composed from the player idle sprite;
  packs override it with `portrait.<ext>` / `[portrait] file`.  The extractor
  writes `portrait.png` into the HUD/reference-pack output.
* Licensing: added `LICENSE.md` (PolyForm Noncommercial 1.0.0) and
  `THIRD_PARTY_NOTICES.md` (SDL2 zlib, stb/dr_mp3/miniz public-domain or MIT,
  Python tool packages, game-content/trademark disclaimer).  Assessment in
  `docs/licensing.md`.
* Portrait work paused.  Analysis of the reference shot: its colours are not
  kDefaultPalette entries (each is within a few counts of a 6-bit triple), and
  no 32x35 / 64x84 match exists in the board tile renders or the SHA frames.
  The title-screen art therefore looks like a separate palette/image source,
  not board or shape data.  Current port default still uses the player idle
  sprite until the real source is identified.
* Verified the original games have no external MIDI support: no MIDI/MPU-401/
  Roland/General MIDI strings (the only "mpu" hits are inside "computer"), no
  MPU port setup, and the setup text/manual only offer Sound Blaster music.
  Music is CMF (MIDI-style events) played on OPL2 FM via `_sbfm_*`.  Noted in
  `docs/jill23-sounds.md`.
* Added `make_release.cmd`: stages the Release exe + SDL2.dll, the
  extractor/importer tools, profiles/, games/, external/ and the user docs
  into `build/release/Jill-EX`, then writes `Jill-EX-release.zip`.
  Game data and extracted assets are deliberately excluded.
* Fixed the level-start invisibility: a music CHECKPT (`*track.ddt`) was still
  running the entrance/warp branch, so touching the spawn checkpoint reset the
  player to state 4 every tick and the rise never completed.  Music commands now
  only queue the track and leave the player alone.
* HUD option PORTRAIT: ON/OFF (OPTIONS > HUD, `[hud] portrait`, `--no-portrait`)
  controls the character portrait beside level-title dialogs.
* Music static: OPL2 rhythm voices now ignore the instrument sustain level and
  decay to silence like the chip, removing the constant hi-hat/snare/cymbal
  wash.  External pack music now uses linear interpolation instead of nearest
  resampling.
* Classic HUD rebuilt: the CLASSIC style now draws the original screen frame
  (per-game dithered border, title strip), a 64-px status column with a red
  health bar, LEVEL/SCORE and the 3-column inventory, plus the INVENTORY
  caption.  The camera centres the player in the visible viewport only while
  CLASSIC is active.  MODERN is unchanged.  `hud/classic.<ext>` pack panels
  keep the legacy 64x84 placement.
* Classic HUD frame now uses shape entry 3 frame 0x0A, the original window
  border noise texture, instead of a generated dither.  The frame therefore
  picks up each episode's own palette (grey/blue/pink).
* Classic HUD frame now composes the 9-slice pieces from shape entry 3
  (0x00/0x03 vertical edges, 0x06/0x08 horizontal edges, 0x04 corners, 0x0A
  fill).  The board renders into the framed viewport and the camera clamps to
  that viewport, fixing Jill disappearing behind the panel at the left/right
  board edges.  HEALTH text and the bar now share the same top row.
* Extractor Shape Viewer: added a Grid toggle that renders every frame of the
  selected entry with hex frame labels and click-to-select, plus explicit `-`
  / `+` step buttons and fixed frame status updates.  Spinbox Up/Down now step
  by one instead of re-running the current value.
* Extractor GUI: the board viewer canvas now uses a grid layout so the
  horizontal scrollbar sits along the bottom of the viewport.  Selecting a
  left file-category tab (Boards/Shapes/Sounds/Config) switches the right
  notebook to the matching viewer; shape and sound selections do the same.
* jill_import: accepts the extractor's flat `sprites/shape_NNN_FFF.png`
  layout, plus new `sha-font`, `sha-tile`, `board-image` and `pack` commands
  for fonts, DMA tile art, full board renders and external pack folders.
  Existing entries keep their bpp/flags/palette during folder import.
* jill_import also gained `vcl-pages` for re-importing `text_pages/pages.json`
  (colours preserved) or plain `page_NN.txt` files into a `.vcl`.
* Documentation: added `COMMANDLINE.md` (port, build/packaging, extractor and
  importer arguments), `HOTKEYS.md` (gameplay, menus, text pages, extractor
  controls) and `IMPORT_README.md` (all jill_import commands, input sizes,
  workflows and limits).
* Classic menu style: `OPTIONS > MENUS > STYLE` (and `--classic-menu`) draws
  the original frame/title with shape entry 3 and the menu window as a
  shape-entry-7 9-slice (scan order: 1/2/3 top, 4/5 edges, 6/7/8 bottom,
  9 checker fill).
  The existing menu items and navigation are unchanged; MODERN remains the
  default.
* Added `build32.cmd` / `run32.cmd`: Win32/x86 Release build through
  `vcvars32.bat`, `-A Win32`, `sdl2:x86-windows`, writing to
  `port/build32/Release32` (jill.exe + SDL2.dll).  The x64 build is unchanged.
* Spider form now draws shape entry 38 (four-frame crawl cycle) instead of the
  procedural placeholder.
* World-map `MAPDEMO` label now draws only "MAP" at a fixed viewport corner;
  the overlapping DEMO run no longer leaves a trailing O and the label does not
  drift with the camera.
* Classic HUD and menu frames can borrow another installed game's shape bank:
  `OPTIONS > HUD > FRAME` / `OPTIONS > MENUS > BORDER`, `--hud-frame N`,
  `--menu-frame N`, `[hud] frame` and `[menus] frame`.

* PS4 music diagnosis: play_music() now reports the requested track through
  jill_paths::trace (always on for PS4, flushed to jill.log) instead of
  printf.  The 22:20 console log had no music: lines at all, which points at
  the stdout redirect on PS4 rather than at track selection.  PC still prints
  music: zeppelin.ddt for --board 1.

* Demo-fidelity diagnostics: three headless debug switches.
  --dump-map FILE writes the live 128x64 tile map (grid + col row id raw
  flags shape records) after the requested tick, so runtime tile mutation
  (bridges, fades, hint walls) can be inspected; --camera X,Y forces the
  camera used for --dump; --classic-camera swaps the centred follow for the
  original edge-triggered scrolling rule (1FAE:000D: 15x11 tile scroll window,
  dead zone player_x-cam_x in [0x58, 0x88], 8 px steps).  All are opt-in and
  console-safe.
* jn1demo.mac status: the port reproduces the original frame for frame
  through the first 90 s of the demo - the SCORE/LEVEL readouts change on the
  same ticks (120 at tick 276, 132 at 472, 135 at 680 = video 55 s).  Past
  tick ~880 the two diverge: the port's Jill ends up on the board floor
  (y = 928) while the original stays up on the ledges, so the port reaches the
  ! CHECKPT at (640,784) only at tick 2090 instead of ~985.  Cross-checked
  against re/img.bin: _cando (26F4:2227), the state-0 ground tail
  (26F4:07CA) and the state-2 entry (26F4:0809) are byte-for-byte equivalent to
  port/src/engine.cpp + player.cpp, so the difference is in the level script /
  object interaction, not in the tile tests or the player state machine.

* Fixed: gems/keys picked up inside a level now carry over to the world map.
  The `!` exit restores the hub board stashed when the level was entered, so
  the hub's header still held the inventory Jill had *before* the level.  When
  the restored hub is the map board (`hub_map_mode_`) the live item list is
  copied into the map header; the demo's intro-board restore is left untouched
  so recorded playback is unaffected.  The same merge runs when `!` reloads the
  map from disk (no stashed hub, e.g. an older save).
* Fixed: loading a save made inside a level threw away the world map's progress
  (`hub_valid_ = false`), so every finished entrance re-opened.  Port saves can
  now carry a magic-tagged extra block ("JLX1" + u32 length) holding the stashed
  hub board plus both board names.  `Board::save` writes it, `Board::load`
  accepts it, and shipped boards still reject trailing bytes.  The block also
  records the board the save was made on, so saves of demo boards (`0.dem`) no
  longer restore with a stale board name or wrong Ctrl+R target.
* `Tab` (and pad Select / Back / touch) toggles the minimap in game; listed in
  HOTKEYS.md.

* Gems are now the player's, not the board's: the gem items (id 3) survive
  every board change and are only ever removed by a gem door, so collecting a
  gem on the map, spending it in level 1 or carrying it into level 2 all work.
  `load_board` re-applies the live gem count to the incoming header (shipped
  boards carry no starting gems, so this only restores what Jill already had)
  and the `!` hub restores do the same for the braces that bypass `load_board`.
  Recorded macros still get the shipped per-board inventory so recorded
  playback is byte-identical.  New `--gems N` debug flag for testing.

* **Macro camera: faithful `1FAE:000D` dead zone + 16x11 view (JN3DEM1 fix).**
  Supersedes the earlier "faithful camera" attempt (see the 2026-11-16 note
  above), which used the full `p.y - p.prev_y` delta and the 232x160 view and
  therefore reverted.  Two things were wrong:
  * `DGROUP:0xBAB2` is not the player's movement delta.  `1FAE:1031` sets it
    to `+2 / -2 / 0` from `dy` inside the state-0 vertical-input branch (the
    crouch/gravity nudge), and every other path passes the zero initialised
    at `1FAE:0226`.  `26F4:12B9` zeroes it before the update loop and
    `26F4:0D52` applies it after.
  * The original's scroll window is 16 x 11 tiles, not 15 x 10.  `b998 = 0xF`
    and `b99a = 0xB`; `26F4:215F` centres with `cam_x = p.x - 120` and
    `cam_y = p.y - 72`, and the clamps `(0x80-0xF-1)<<4 = 1792` /
    `(0x41-0xB)<<4 = 864` only make sense with 256x176.  Macro playback now
    uses that `sim_view_w/h` (was 232x160); normal play is unchanged.
  * Verified rule: `lo = clamp(p.y-0x50,0,0x360)`,
    `hi = clamp(p.y-0x20,0,0x360)`; if `cam_y + step` is in `[lo,hi]` the
    camera moves by `step`, else it snaps to the edge it is outside of.
  * Results: JN3DEM1's knife now reaches the crab (crab gone at tick 526,
    `killaward` runs; the 4x5 score popup can be cancelled by the knife's own
    AABB the same tick, the points still count).  JN2DEM1's recorded health
    drops still land at ticks 90/165/211 (video 89/164/210).  jn1demo still
    ends on the jungle map at lvl 102 and end1/end2 are unchanged.  All nine
    shipped macros run to their normal end state with no deaths.
  * Demo-studio and both release zips rebuilt.

* Demo studio: `--ticks`, `--trace`, `--dump-map` and `--dump-objects` now
  imply `--headless`.  Previously `--ticks 527 --dump-objects FILE` without
  `--headless` opened the window and exited without writing the dump, which
  looked like a broken dump.  Verified on jn3dem1 tick 527: the studio's
  object table matches `jill.exe --dump-objects` (crab gone, knife alive).

* **Crawlers no longer cross spike/thorn pits (`_crawl` fidelity).**  The port
  exempted `SPIKE`/`WTHORN` tiles from `_crawl`'s "hole" test as a deliberate
  deviation (see the 2026-11-08 note); that note was written while the board
  loader's sign-extension bug was making walkers look stuck, and the original
  plainly treats those tiles as holes.  Exemption removed.
  * It mattered: Jill 3 level 1's crab could walk straight across the
    one-tile thorn pit at (36,5).  The port's crab reached x=640, was culled
    there, thawed late, and was still at x=604 when JN3DEM1's knife arrived,
    so the kill landed ~7 ticks later than the recording - by then Jill had
    landed, and the returning knife homed *down* into the thorn notch and
    stayed there (p11 = -1 forever).
  * With the exemption gone the crab paces between the platform's left end
    and the thorn (x 512..560), exactly as in the original video, and the
    knife kills it at tick 518 while Jill is still airborne.  The knife then
    climbs left to her, is picked up (inventory 1 -> 2) and never touches the
    thorn.  JN3DEM1 now ends hp=7 inv=2 objs=14 (was 6/1/16).
  * The other eight shipped macros are unchanged (jn2dem1..3, jn3dem2/3,
    end1/end2, jn1demo), x64 and Win32 agree.
