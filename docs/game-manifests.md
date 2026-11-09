# Game manifests (episode and mod data)

Status: the runtime loader, CLI and front end are implemented in the port.
The three shipped manifests are generated; user manifests are hand-written.

## Why

The port used to derive everything from a compiled `ProfileDef` indexed by an
episode number 1..3.  That made a fourth game (Onesimus) a code change
(`profile_for(4)`, a menu row, an episode range check).  A manifest splits the
two different kinds of data:

| layer | contains | where it lives | can a mod change it |
|--|--|--|--|
| engine profile | kind table (name/w/h/flags/tileset/count), handler tables, DGROUP anchors, item icons, EPIC score | `docs/profiles.json` -> `re/gen_port_data.py` -> `src/gamedata.inc` | no; needs a RE pass |
| game manifest | asset files, board list, start/intro board, palette, suit index, HUD icon bank | `docs/games.json` -> `re/gen_games.py` -> `games/<id>.game` | yes; text only |

A mod therefore names an existing engine profile and replaces art, boards and
metadata.  A game whose engine code differs (e.g. Onesimus) needs a new profile
first, then its manifest is ordinary data.

## File format

`games/<id>.game`, one `key = value` per line:

| key | required | meaning |
|--|--|--|
| `id` | no | game id shown by `--list-games`; defaults to the file stem |
| `title` | no | name in the game list and window title; defaults to `id` |
| `profile` | **yes** | compiled engine profile: `jill1`, `jill2`, `jill3` |
| `data` | no | asset directory; defaults to the profile's (`jillt`) |
| `shapes` | no | `.sha` shape bank; defaults to the profile's |
| `tiles` | no | `.dma` tile table; defaults to the profile's |
| `pages` | no | `.vcl` sampled-sound bank that also carries the 40 text pages; defaults to the profile's sound bank |
| `board_ext` | no | board suffix, e.g. `.jn1` or `.ONE`; defaults to the profile's |
| `first_board` | no | board START GAME loads; extension optional |
| `map_board` | no | hub/world-map board used by `!` and map-mode checks; defaults to `first_board`. Onesimus uses `map_board = MAP` while starting on `1` |
| `intro_board` | no | board used by the menu THEME backdrop |
| `menu_block` | no | `.dma` tile name used for the theme menu frame ring (default: the board's most common solid tile) |
| `palette` | no | 768-byte raw 8-bit RGB palette file, replaces the built-in default |
| `suit` | no | palette index for the SUIT HUD background, `-1` = derive from the player art |
| `hud_icon_bank` | no | shape bank for item icons and health pips; default `14` |
| `boards` | no | comma/space separated level-select list, no extensions; defaults to the profile's board list |

Parsing rules `[verified]` (port/src/game.cpp):
* keys are case-insensitive, surrounding whitespace is trimmed;
* `#` or `;` at the start of a line is a comment; `[section]` lines are ignored;
* an unknown key prints a warning and is otherwise ignored;
* a game without `profile`, or with a profile id not in the compiled table, is
  skipped and reported; it does not abort the list;
* `boards` entries are stored verbatim, so a mod can list names the base
  profile never shipped.

## Discovery

* `--games DIR` reads every `*.game` in `DIR` (sorted by file name).
* Without the flag the port reads `games` relative to the working directory;
  `run.cmd` changes to the repository root first.
* `jill.ini` `[paths]` can override the `games` and `profiles` directories and
  give each installed game its own asset directory (`<id> = DIR`); an explicit
  `--games`/`--profiles`/`--data` wins for that run.  A missing `<id>` key
  keeps the manifest's local default (`jillt` for the three episodes, `ONESIM`
  for Onesimus). **[verified]**
* If the directory is missing or holds no usable manifest, the three compiled
  episodes are exposed as built-in games, so a bare executable still runs
  `[verified]`.

The game list drives the START GAME menu, the F3 level select (Left/Right
cycles games) and the MENUS THEME list (`PORT` plus one row per game).  The
theme index is clamped to the installed count, so nothing assumes three games.

## Profile files (engine data)

`profiles/<id>.profile` holds the compiled engine data a manifest names:
kind table, handler tables, item icons, EPIC score and the object limits.  The
port loads every `*.profile` in `profiles\` at start-up (override with
`--profiles DIR`), so a total conversion does **not** need a rebuild.  The
three shipped episodes also have their profile written out as a reference
example; it is byte-for-byte equivalent to the compiled table `[verified]`
(headless object dumps are identical with and without the profile files).

Format, same `key = value` grammar as the game manifest:

| key | meaning |
|--|--|
| `id` | profile id (defaults to the file stem) |
| `base` | inherit every key from another profile, then apply this file |
| `kind_count` | number of kinds (64 in Jill 1, 69 in Jill 2/3) |
| `kind` | `k, NAME, w, h, flags, tileset, count`; only needed when it differs from the base |
| `item_icons` | 16 item-id -> HUD icon frames |
| `epic_score` | `_addscore` value for kind 49 |
| `object_max`, `object_stride` | object pool sizing (192 / 31) |
| `fly_frame`, `fly_y`, `phoenix_frame`, `phoenix_mirror`, `demon_frame`, `fatso_frame`, `expl2_frame`, `bat_frame`, `bat_y` | per-handler animation tables |
| `handler_offsets`, `dgroup_base`, `board_ext`, `episode` | provenance; ignored by the runtime |

So a same-engine total conversion is typically four lines:

```
id = onesimus
base = jill3
kind_count = 69
```

Anything absent is inherited, and the runtime prints a warning for a key it
does not understand instead of rejecting the file.

## Profiling a new game: `tools/mkprofile.py`

```
py -3 tools/mkprofile.py --exe PATH\GAME.EXE --id mytc --data PATH\ASSETS
```

It discovers, without any manual RE `[verified]` on jill1.exe, jill3.exe and
ONESIMUS.EXE:

* MZ header size and load image;
* the contiguous kind-name block (`PLAYER` ... `ROMAN`);
* the kind-table initialiser (the `mov word [addr], imm` run, 621 stores in
  all three builds);
* the DGROUP image base and stored segment, by solving the name-string offsets
  against the name offset/segment table (Jill 1 `0x1cff0/0x1cff`, Jill 3
  `0x1d750/0x1d75`, Onesimus `0x1d850/0x1d85`);
* the width/height/flags/tileset/default-count tables (matched against the
  base profile);
* the nine handler tables (the whole block shifts by one uniform delta);
* the item-icon table and the 768-byte palette at `DGROUP:0x0194`;
* the base profile, by nearest DGROUP base plus name and handler-block match;
* with `--data`: board extension, board list, `.sha`/`.dma`/`.vcl` names, first
  board (`MAP`/`0`/`1`) and intro board (`INTRO`), the 36 entry level-title
  table, and a `.pal` next to the profile (expanded from the 6-bit DAC values to
  8-bit RGB).

It writes `profiles/<id>.profile` and (with `--data`) `games/<id>.game`.
Use `--force` to overwrite, `--board-ext` / `--boards` / `--first-board` /
`--intro-board` to override a heuristic, and `--no-game` for the profile only.

Onesimus, profiled and played end to end `[verified]`:

```
py -3 tools/mkprofile.py --exe D:\Emu\DOS\C\ONESIM\ONESIMUS.EXE --id onesimus \
    --title ONESIMUS --data D:\Emu\DOS\C\ONESIM
# -> base = jill3, count = 69 kinds, 0 kind overrides needed,
#    DGROUP 0x1d850, palette found, 16 .ONE boards, first MAP, intro INTRO
jill --game onesimus --board MAP --ticks 4
# -> onesimus MAP.ONE  hp=6 ...   (boards and art load and render)
```

The five Onesimus-only kinds (64..68) still fall back to the static draw until
their handlers are ported; the *profile* is no longer the blocker.

## Adding a game (one-command flow)

For a same-engine build (the normal total conversion):

1. `py -3 tools/mkprofile.py --exe GAME.EXE --id mytc --data ASSETS`
2. `jill --game mytc`

That is the whole procedure: no `docs/profiles.json` edit, no `gamedata.inc`
regeneration, no rebuild, no menu edit.  The tool reports anything it could not
prove so you know what to verify by hand.

For a build whose kind data or palette differs from every shipped base, the
generated profile falls back to inheritance and lists the fields it could not
solve; those need the `docs/profiles.json` route (the reference values from the
decompilation are the only trustworthy source for a rewritten kind table).

## Board format compatibility

`*.ONE` (Onesimus) parses with the port's `Board::load` unchanged: 0x4000 tile
map, `u16` object count, 31-byte objects, 0x46 header, trailing length-prefixed
strings, exact-EOF check.  All 16 shipped `.ONE` files pass `[verified]`, so
`board_ext` is the only thing a manifest needs for those assets.

## Open

* Kind *handlers* are code.  Kinds 64..68 (EYE, VINECLIMB, FLAG, MAPDEMO,
  ROMAN) profile and load, but until their handlers are ported they draw
  statically; a mod that changes handler *behaviour* (not data) still needs C++.
* The tool recognises a derivative when its kind values and handler tables still
  match a shipped base (`[verified]` for Onesimus).  A build with rewritten
  animation tables or kind dimensions inherits the base values and says so; a
  value-by-value override then has to be written by hand in the `.profile`.
* `hud_icon_bank` only moves the item-icon/health-pip bank; other absolute
  shape ids in the HUD paths (`0x0E05`, `0x0E10`, `0x0E18`, `0x0E21`, font
  banks) are still the documented Jill values.  A total HUD art replacement
  needs those exposed too `[open]`.
* Manifest/profile/palette paths are resolved against the working directory,
  not the executable; `jill.ini` can relocate them without a rebuild, but a
  portable build still wants an exe-relative base `[open]`.
* Key bindings are still global; save names are now per engine profile
  (`save_prefix`/`save_slots`), so each episode writes its own prefix.
  **[verified]**
