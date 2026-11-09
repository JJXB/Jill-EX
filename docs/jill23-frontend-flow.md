# Jill 2 / Jill 3 - front end and level flow

This note covers what happens between starting `jill2.exe`/`jill3.exe` and
entering a board, the menu, the attract demos, the level graph and the
level-title table. Jill 1 is described in `jill1-engine.md`; only differences and
Jill 2/3 specifics are repeated here.

Confidence markers follow the other docs: `[verified]` from disassembly or file
bytes, `[inferred]` from consistent evidence, `[open]` unresolved.

## 1. Boot assets

Both episodes use the same asset family as Jill 1 with substituted names:

| asset | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| sampled sound bank | `jill1.vcl` | `jill2.vcl` | `jill3.vcl` |
| shape file | `jill1.sha` | `jill2.sha` | `jill3.sha` |
| tile definitions | `jill.dma` | `jill.dma` | `jill.dma` |
| config | `jill1.cfg` | `jill2.cfg` | `jill3.cfg` |
| intro board | `intro.jn1` | `intro.jn2` | `intro.jn3` |
| title music | `funky.ddt` | `peaks.ddt` | `wild.ddt` |
| save prefix | `jn1save` | `jn2save` | `jn3save` |

The name strings sit together in the EXE data. In Jill 2 they appear as
`jill2.vcl`, `jill2.cfg`, `jill.dma`, `0.jn2`, `intro.jn2`, `peaks.ddt`; the
start of a new game is `0.jn2` (`mov ax, 0x180B` at file `0x159A1`). In Jill 3 the
new game starts at `map.jn3` (`mov ax, 0x18B9` at file `0x15C65`). **[verified]**

Boot order matches Jill 1: load VCL, load/read CFG, install keyboard and joystick,
run the graphics/driver setup, load shapes, load title music, draw the menu, and
show the intro board. Jill 3 adds the Sound Blaster detection and CPU-speed check
before the sound questions (`jill3-differences.md` section 7). **[verified]**

## 2. Main menu

The menu prompt is byte-identical in all three episodes and is a single string
with `\r`-separated items. The first byte of each item is a colour/attribute
digit; the visible text follows it.

```
7PICK A CHOICE:
2PLAY
2RESTORE
5STORY
5INSTRUCTIONS
5ORDERING INFO
5CREDITS
3DEMO
3NOISEMAKER
4QUIT
```

DGROUP offsets of the '7' prefix:

| episode | prompt | code that pushes it (file offset) |
|--|--|--|
| Jill 1 | `0x14A8` | `0x156BC` |
| Jill 2 | `0x1544` | `0x15927` |
| Jill 3 | `0x195C` | `0x15BEB` |

The menu renderer is a shared text-window routine; the per-episode difference is
only the location of the prompt and of the auxiliary strings (title caption,
"PICK A CHOICE" window, etc.). **[verified]**

### 2.1 Item keys and actions

The menu dispatch compares the last key against ASCII codes. The letters match
the visible items in all three episodes: **[verified]**

| key | item | action |
|--|--|--|
| `P` | PLAY | start a new game: Jill 2 loads `0.jn2`, Jill 3 loads `map.jn3` |
| `R` | RESTORE | open the LOAD GAME window, then load the saved board |
| `S` | STORY | show the story text screen |
| `I` | INSTRUCTIONS | show the instructions screen |
| `O` / `H` | ORDERING INFO | show ordering information |
| `C` | CREDITS | show the credits screen |
| `D` | DEMO | play the next attract demo (section 3) |
| `N` | NOISEMAKER | enter the sound-test screen |
| `Q` / `ESC` | QUIT | leave the menu / ask to quit |
| `E` | (extra) | calls a text-page helper; purpose not identified |

`P` is also accepted as the numeric code `0x10` and there is a second numeric
branch (`0x05`) that runs the same "leave menu" sequence. These are not ASCII
letters; they are most likely translated joystick/alternate input codes. **[open]**

The text items use two different viewers.  STORY, ORDERING INFO and CREDITS
call `FUN_22ab_266f(page)` (`S` -> 0, `O` -> 8, `C` -> 0xc): that scans the
current front-end board for CHECKPT objects whose `p13` equals the page number,
scrolls the viewport to the object's position and waits for a key; the object's
`p1b` links to the next page.  INSTRUCTIONS (`I`) calls
`FUN_22ab_0f0a(1)`, the `.vcl` text-page viewer (page 1 = "How the Heck to
Play"), which is also what page apples use (see `jill1-engine.md` #10.1).
NOISEMAKER is the special page 99. **[verified]**

After a demo ends, control returns to the intro board (`0x5D84` in Jill 3 loads
`intro.jn3`), not directly to the menu.

### 2.2 Save / restore

RESTORE opens a window labelled `LOAD GAME`; the in-game save function uses
`SAVE GAME`. Slots are numbered and the file name is built from the episode
prefix plus the slot number, i.e. `jn2save.N` / `jn3save.N`. The file is the same
board serialisation used by Jill 1 (`jill1-engine.md` section 7), so a port can
share one reader/writer. **[verified for the prefix, inferred for the `.N` suffix
from the Jill 1 behaviour and the 6-slot CFG block]**

High scores use the `HI SCORES` window and the same 10-entry table layout in
`jillN.cfg` as Jill 1.

## 3. Attract / demo mode

DEMO plays a recorded input macro on a fixed board. The table is stored per
episode and has one entry per demo:

| episode | demo boards | macros | level ids |
|--|--|--|--|
| Jill 1 | `0.dem`, `2.dem`, `1.dem` (from intro checkpoints) | `jn1demo.mac` | - |
| Jill 2 | `3.jn2`, `9.jn2`, `17.jn2` | `jn2dem1.mac` .. `jn2dem3.mac` | 3, 9, 17 |
| Jill 3 | `1.jn3`, `5.jn3`, `12.jn3` | `jn3dem1.mac` .. `jn3dem3.mac` | 1, 5, 12 |

Jill 3's demo code is at runtime `2000:59F9`. It indexes a far-pointer table at
DGROUP:`0x19CF` (board names), reads the level id from DGROUP:`0x19F7`, sets the
HUD level word `DGROUP:0x7E42`, loads the board, then starts macro playback from
the pointer table at DGROUP:`0x1A01`. Jill 2 uses the same structure
(`2000:5AE5`): board-name pointers at DGROUP:`0x188A`, level bytes at
DGROUP:`0x18B2`, macro pointers at DGROUP:`0x18BC`, HUD level word
`DGROUP:0x80E0`. **[verified]**

The intro board itself is inherited from Jill 1. `intro.jn2`/`intro.jn3` still
contain CHECKPT strings `0.dem`, `2.dem`, `1.dem` from the Jill 1 build, but the
menu DEMO path uses the episode-specific boards above. The intro board is also
where the "A NEW RELEASE FROM / PRODUCED BY" and ordering text lives.

## 4. Level graph

Progression is not a table in the EXE: it is encoded in CHECKPT object strings
inside the boards (prefix rules in `jill1-engine.md` section 7). A bare filename
is a warp; `!` is the end-of-level sequence.

### 4.1 Jill 2

Scanner output for the shipped `.jn2` boards: **[verified strings]**

```
0.jn2   -> 1.jn2
1.jn2   -> 3.jn2, 17.jn2
3.jn2   -> 9.jn2
9.jn2   -> 10.jn2
10.jn2  -> 11.jn2, 15.jn2
11.jn2  -> 13.jn2
13.jn2  -> 10.jn2
15.jn2  -> 1.jn2
17.jn2  -> 20.jn2
20.jn2  -> &end2.mac, ending text
```

Read as a graph, boards `1` and `10` are hubs: `1` branches to `3` and `17`,
`10` branches to `11` and `15`. `13 -> 10` and `15 -> 1` return to the hubs after
the branch is completed. `20` is the ending board.

Jill 2 ships no `map.jn2`; the hub screens are ordinary levels.

### 4.2 Jill 3

`map.jn3` is the hub and the new-game board. Its CHECKPT strings warp to the
numbered levels (string order in the file): **[verified]**

```
map.jn3 -> 1.jn3 2.jn3 3.jn3 4.jn3 5.jn3 6.jn3 8.jn3 7.jn3
           9.jn3 10.jn3 11.jn3 12.jn3 13.jn3
map.jn3 -> *neoclass.ddt
```

The `8`/`7` order is the order the objects appear on the map, not a numbering
error. Levels `1`-`13` each contain `!` and no warp filename; level `13` instead
warps to `14.jn3`. Level `14` is the ending (prince text, `*neoclass.ddt`).

The natural reading is: complete a level -> `!` returns to the map -> choose
another exit; after level 13 -> castle ending `14.jn3`. The `!`-returns-to-map
step is inferred, because the J3 `!` handler was not traced; the map-only set of
incoming warps and the absence of any other route back make it the only
consistent route. **[inferred]**

### 4.3 Where `!` goes

The `!` branch of `22ab:1531` loads a fixed board name from DGROUP:`0x137A`
(`"temp"`, the save/temp board slot), keeps the score, places the player on the
CHECKPT whose `p13` equals the current level id (`26F4:04C7`), and then kills
that checkpoint unless it is the state-1 case with gems. In a fresh game the
temp/save board is a copy of the hub board. The port therefore loads
`profile.first_board` for a new game and `game.map_board` for the `!`/hub path:

* Jill 1: starts on `map.jn1` (port decision; the original new game is inferred
  to start at level `1`, and the map is only reached through the `!` path).
  **[inferred]**
* Jill 2: `0.jn2` - no map exists; `!` restarts the first level.
* Jill 3: `map.jn3` - verified hub flow above.
* Onesimus: starts on `1.ONE` (the executable's start string is `1.one`), and
  its world map is `MAP.ONE` (`map_board = MAP`). `1.ONE`'s exit CHECKPT warps
  to `map.one`. **[verified]**

After the load, `place_at_checkpoint(level)` puts the player on the entrance
with the matching `p13`, and `!` kills that entrance so the hub shows the level
as finished. Hub boards are the exception: the port restores the stashed map
copy and keeps the player where it stood (the map's `p13 = 0` CHECKPT is the
music trigger, not an entrance), so returning does not teleport Jill to `(0,0)`.
A bare-filename warp instead loads the named board and places the player on its
own entrance CHECKPT (`p13 == level id`, `x & ~7`, `y - 0x10` for normal Jill;
TINY keeps `y`).

## 5. Level-title table

Both episodes have a 36-entry table of far pointers to `\r`-formatted title
strings. Index = level id + 4; entries 0-3 are the four demo captions, entries
4.. are one per level. The extra offset of 4 is confirmed by Jill 3's level 12
title, which literally says "IF YOU THINK THE NEXT LEVEL IS NUMBER TWELVE,
YOU'RE RIGHT!". **[verified]**

| episode | table file offset | stored segment | first title string |
|--|--|--|--|
| Jill 2 | `0x1F549` | `0x1D32` | `0x1F61E` |
| Jill 3 | `0x1FA65` | `0x1D75` | `0x1FB3A` |

Pointers are `(offset, stored_dgroup_segment)`; the file offset of a title is
`segment*16 + offset`. All 36 entries point inside the DGROUP block.

### 5.1 Jill 2 titles

| level | title |
|--|--|
| 0 | (empty) |
| 1 | JILL GOES / UNDERGROUND |
| 3 | JILL ENTERS / MONTEZUMA'S / CASTLE |
| 9 | JILL DESCENDS / INTO THE / DEPTHS OF / HECK |
| 10 | JILL RUNS INTO / THE RED PUZZLE |
| 11 | JILL SWIMS TO / THE WATERWORLD |
| 13 | JILL DRIFTS / INTO THE / DEMONIC MAZE |
| 14 | JILL BOUNDS / INTO / THE BONUS LEVEL (no `14.jn2` ships) |
| 15 | JILL WANDERS / INTO THE LAND / OF ETERNAL / WEIRDNESS |
| 17 | JILL BETTER / THINK FAST! |
| 20 | JILL ESCAPES / THE UNDERGROUND |
| 21..29 | "21".."29" placeholders |

Unused ids (2, 4-8, 12, 16, 18, 19) hold empty or blank titles; ids 21-29 are
numeric placeholders for cut levels.

### 5.2 Jill 3 titles

| level | title |
|--|--|
| 0 (`map.jn3`) | JILL ENTERS / THE / JUNGLE MAP |
| 1 | JILL ENTERS / THE VALLEY |
| 2 | JILL ROAMS / THROUGH THE / VILLAGE |
| 3 | JILL JOURNEYS / TO THE DAM |
| 4 | JILL DISCOVERS / THE SECRET FOREST |
| 5 | JILL BOUNDS INTO / THE AERIE |
| 6 | JILL EXPLORES / THE AQUEDUCT |
| 7 | JILL BOARDS / THE SHIP OF / THE GIANT GREEN / LIZARD MEN! |
| 8 | JILL VENTURES / INTO THE / MEGA PUZZLE |
| 9 | JILL JOURNEYS / INTO THE JAIL |
| 10 | JILL TRYS HER / LUCK IN THE / PYRAMID PUZZLE. |
| 11 | JILL LEAPS INTO / LEVEL ELEVEN |
| 12 | IF YOU THINK THE / NEXT LEVEL IS / NUMBER TWELVE, / YOU'RE RIGHT! |
| 13 | JILL FINALLY / DISCOVERS / THE CASTLE |
| 14 | (empty - ending board) |
| 21..29 | "21".."29" placeholders |

The four demo captions (indices 0-3) are the same in both episodes:
`Look, an airplane!`, `Are you just gonna sit there?`, `Have you seen Jill
anywhere?`, `Hey,  your shoes are untied.` (two spaces in the last one).

The title is displayed by `_putlevelmsg`/`_leveltxt` when a board loads; the HUD
`LEVEL` field shows the level id itself.

**Port status:** the table is read into `ProfileDef::level_titles`
(`re/gen_port_data.py` -> `kTitles1..3`) and the port draws it as a centred
framed caption box when a board loads (`index = level id + 4`), revealed at the
dialog speed.  A direction still held from the previous level cannot dismiss it
for the first `0x28` ticks, so walking through an exit still shows the popup;
after that any input closes it.  Runtime profiles can override it with
`level_title = N, text` rows.

Jill 2/3 board headers usually store `level_id = 0`, so the port derives the id
from a numeric board file name (`3.jn2` -> 3, `10.jn2` -> 10) before indexing
the table; non-numeric names (`map`, `intro`) keep the header value.  A warp then
recomputes the caption after restoring the id carried by the CHECKPT, which is
what makes "JILL GOES / UNDERGROUND" and the other Jill 2 popups appear.
**[verified by shipped headers; inferred for the warp recompute]**

## 6. Text screens and the intro

`STORY`, `ORDERING INFO` and `CREDITS` are board-page screens: the front-end
board carries CHECKPT objects with `p13` = page number and `p1b` = next page,
and `FUN_22ab_266f(page)` scrolls the viewport to each one.  `INSTRUCTIONS`
instead calls `FUN_22ab_0f0a(1)`, which reads text page 1 from the `.VCL` bank
("How the Heck to Play").  The intro board is a normal board file that
combines:

* the publisher/producer captions,
* the game blurb and ordering information,
* CHECKPT warps used by the attract mode.

Jill 3's intro ends with the "save the prince" story text; Jill 2's with the
"your adventure begins in . . ." text. The intro music is `funky.ddt` in both.

Separately, the `.VCL` page table supplies the help/hint/news pages that appear
in game: a kind 1 APPLE with `state > 0` shows page `state` when touched (see
`jill1-engine.md` #10.1).  Jill 1 labels these pages "News bulletin"; Onesimus
uses the same mechanism for its "Book of Philemon" verses, with the page apples
placed across its 16 boards.

## 7. Port checklist

* One menu implementation, episode table for prompt offset, items and key map.
* Episode table for start board (`0.jn2` / `map.jn3`), demo boards/macros/levels,
  title-table location and save prefix.
* Keep CHECKPT string semantics: bare filename = warp, `!` = level end, `*` =
  music, `&` = macro, `#` = conditional load.
* Save files are board files; reuse the Jill 1 serialiser with a new prefix.
* Do not hardcode the Jill 1 menu addresses; all three prompts are the same
  bytes but at different DGROUP offsets.

## 8. Open questions

* Exact `!` handling in Jill 3 (return to map vs. implicit level+1). **[open]**
* Meaning of numeric menu codes `0x05` and `0x10`. **[open]**
* Whether the inherited `0.dem`/`1.dem`/`2.dem` intro warps are reachable in
  Jill 2/3 or only dead strings. **[open]**
