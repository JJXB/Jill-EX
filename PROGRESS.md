# Progress

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
