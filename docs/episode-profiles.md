# Episode profiles - Jill 1 / Jill 2 / Jill 3

This is the single lookup table a multi-episode port should read at start-up. It
collects every per-episode constant that the engine docs describe, in one place,
and it is mirrored by `docs/profiles.json` for code to load directly.

Nothing here should be hardcoded in the port: the source of truth is the episode
profile. The engine code is shared (see `shared-handler-equivalence.md`); only
these values change.

## 0. Conventions

| Term | Meaning |
|--|--|
| `DGROUP:xxxx` | offset from DS at run time (DS = runtime DGROUP segment) |
| `image` | offset inside `re/img.bin` / `re/img2.bin` / `re/img3.bin` |
| `stored seg` | segment word as it appears in the EXE/TD image, before relocation |
| `runtime seg` | `stored seg + 0x1000` |

Relations used throughout:

```
linear      = 0x10000 + stored_seg * 16 + offset
image off   = linear - 0x10000
image off   = dgroup_image_base + dgroup_offset
```

The DGROUP image base is the stored DGROUP segment times 16:
`0x1CFF0`, `0x1D320`, `0x1D750`.

Confidence markers follow the other docs. Values marked `[verified]` were checked
against the EXE bytes or the disassembly; `[inferred]` values follow from a proven
layout rule and still have a cross-check.

## 1. Executables and analysis images

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| EXE | `jillt/jill1.exe` | `jillt/jill2.exe` | `jillt/jill3.exe` |
| EXE size | 216804 (`0x34EE4`) | 185574 (`0x2D4E6`) | 204174 (`0x31D8E`) |
| MZ header | `0x1C00` | `0x1C00` | `0x1C00` |
| load module size | 185478 (`0x2D486`) | 178406 (`0x2B8E6`) | 179638 (`0x2BDB6`) |
| debug/symbol tail | 24158 (`0x5E5E`) | 0 | 16856 (`0x41D8`) |
| analysis image | `re/img.bin` (209636 B) | `re/img2.bin` (178406 B) | `re/img3.bin` (196494 B) |
| runtime DGROUP segment | `0x2CFF` | `0x2D32` | `0x2D75` |
| stored DGROUP segment | `0x1CFF` | `0x1D32` | `0x1D75` |
| DGROUP image base | `0x1CFF0` | `0x1D320` | `0x1D750` |
| kind count | 69 (0..68) | 69 (0..68) | 69 (0..68) |
| board extension | `.jn1` | `.jn2` | `.jn3` |

`re/img.bin` and `re/img3.bin` include the appended TD debug block; `re/img2.bin`
does not. All image offsets used by the engine are below the load-module size, so
the tail does not affect addressing. **[verified]**

## 2. Board / object anchors (DGROUP offsets)

These are the values the board loader, object pass and renderer read every frame.

| Variable | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| tile map, 128x64 x u16 (`0x4000` B) | `0x43A6` | `0x40E0` | `0x3E42` |
| board header / live HUD, `0x46` B | `0x83A6` | `0x80E0` | `0x7E42` |
| per-tile table, 600 x 8 B | `0x83EC` | `0x8126` | `0x7E88` |
| object array, 192 x 31 B | `0x96AC` | `0x93E6` | `0x9148` |
| object count (u16) | `0xBB1C` | `0xB856` | `0xB5F8` |
| frame counter (u16) | `0xBE3A` | `0xBB74` | `0xBC16` |
| kind flags array (kind_count x u16) | `0xBE3C` | `0xBB76` | `0xBC18` |
| visible / on-screen count (u16) | `0xBECC` | `0xBC06` | `0xBCA8` |
| active object index list (u16[192]) | `0xBCAC` | n/a | n/a |
| camera / render-rect far pointer | `0xB7F2` | `0xB52C` | `0xB28E` |
| pending level-transition string | `0xBB1E` | `0xB858` | `0xB5FA` |
| HUD item-id -> icon frame table (u16[16]) | `0x0ED4` | `0x0F70` | `0x1080` |

The item-icon table is byte-identical in all three episodes; only its DGROUP
offset differs. `re/gen_port_data.py` emits it as `kItemIconsN`. The EPIC
`_addscore` value (2 in Jill 1/2, 25 in Jill 3) is likewise generated as
`ProfileDef::epic_score`. **[verified]**

The four board-structure addresses satisfy the same invariants in every episode:

```
map + 0x4000            = header
header + 0x46           = tile table
tile table + 600 * 8    = object array
```

`[verified for all three]`

Notes:

* The object count is followed immediately by the pending-transition string
  (`count + 2`). The string is tested with exactly one
  `cmp byte ptr [addr], 0` in the main loop in all three images. **[verified]**
* The camera slot is a **far pointer** (loaded with `les bx, ptr [slot]`), and the
  struct is read at `+6` (view width), `+8` (x) and `+10` (y). The
  `les bx, ptr [slot]` encoding occurs 108 / 60 / 95 times in Jill 1 / 2 / 3.
  **[verified]**
* Jill 1's active list starts at `0xBCAC` with the count at `0xBECC`
  (`0xBCAC + 192*2`). The Jill 2/3 active-list base has not been tracked down; the
  visible count is still known. **[open for J2/J3 base]**

## 3. Kind table

The table lives in BSS and is filled by a per-episode initializer. All three
executables initialise the same 69 kinds (0..68); Jill 1 omits the immediate
stores for fields whose value is zero, which is why the store count is lower
there. Kinds 64..68 (`EYE`, `VINECLIMB`, `FLAG`, `MAPDEMO`, `ROMAN`) therefore
exist in Jill 1 too, and its boards reference them. **[verified]**

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| initializer image offset | `0x6442` | `0x6717` | `0x6500` |
| initializer stored far addr | `0644:0002` | `0671:0007` | `0650:0000` |
| initializer runtime far addr | `1644:0002` | `1671:0007` | `1650:0000` |

Field arrays (DGROUP offset of element 0; stride 4 for far pointers, 2 for words):

| Field | Jill 1 base | Jill 2 base | Jill 3 base |
|--|--|--|--|
| handler offset (u16) | `0xB84E` | `0xB588` | `0xB2EA` |
| handler segment (u16) | `0xB850` | `0xB58A` | `0xB2EC` |
| width (u16) | `0xB6B6` | `0xB3F0` | `0xB152` |
| height (u16) | `0xB740` | `0xB47A` | `0xB1DC` |
| name offset (u16) | `0xB99C` | `0xB6D6` | `0xB438` |
| name segment (u16) | `0xB99E` | `0xB6D8` | `0xB43A` |
| flags (u16) | `0xBE3C` | `0xBB76` | `0xBC18` |
| tileset id (u16) | `0xBB3E` | `0xB878` | `0xB61A` |
| default count (u16) | `0xBBC8` | `0xB902` | `0xB6A4` |

The handler/name entries are far-pointer pairs: at base `+0` the offset, at base
`+2` the segment. Reading kind `k` therefore uses `base + 4*k` for these two
fields and `base + 2*k` for the rest. All bases **[verified]**; the name-segment
array in Jill 1/2 is written with `mov [addr], ds` rather than with an immediate,
but the slot exists in all three games.

## 4. Sound

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| `_sound(chan, id)` runtime far addr | `1581:0178` | `15EB:01DA` | `1576:01E8` |
| stored far addr | `0581:0178` | `05EB:01DA` | `0576:01E8` |
| image offset of prologue | `0x5988` | `0x608A` | `0x5948` |
| id remap table (DGROUP) | `0x1212` | `0x12AE` | `0x13AC` |
| sampled-sound bank | `jill1.vcl` | `jill2.vcl` | `jill3.vcl` |
| SFX used count | 23 | 24 | 23 |

The remap table is a 50-byte array of logical id -> sample index (0 = identity).
Its contents are identical in all three games; the call sites and the two
per-episode sample-size exceptions are in `jill23-sounds.md`. **[verified]**

## 5. Front end and assets

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| shape file | `jill1.sha` | `jill2.sha` | `jill3.sha` |
| tile definitions | `jill.dma` | `jill.dma` | `jill.dma` |
| config file | `jill1.cfg` | `jill2.cfg` | `jill3.cfg` |
| intro board | `intro.jn1` | `intro.jn2` | `intro.jn3` |
| first board of a new game | `map.jn1` (port; the original uses `1.jn1`) | `0.jn2` | `map.jn3` |
| title music | `funky.ddt` | `peaks.ddt` | `wild.ddt` |
| save prefix | `jn1save` | `jn2save` | `jn3save` |
| save slots | 6 | 6 | 6 |
| menu prompt (DGROUP) | `0x14A8` | `0x1544` | `0x195C` |
| menu prompt (code, image) | `0x156BC` | `0x15927` | `0x15BEB` |

Board list shipped with the game:

| Episode | boards |
|--|--|
| Jill 1 | `1, 2, 3, 4, 6, 9, 50, map, intro` + demo boards `0.dem, 1.dem, 2.dem` |
| Jill 2 | `0, 1, 3, 9, 10, 11, 13, 15, 17, 20, intro` (no map board) |
| Jill 3 | `1..14, map, intro` |

Level flow is data-driven (CHECKPT strings), not a table; the graphs are in
`jill23-frontend-flow.md` section 4.

### 5.1 Attract demos

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| demo code runtime far addr | (main loop) | `2000:5AE5` | `2000:59F9` |
| board-name ptr table (DGROUP) | none | `0x188A` | `0x19CF` |
| level-id bytes (DGROUP) | none | `0x18B2` | `0x19F7` |
| macro ptr table (DGROUP) | none | `0x18BC` | `0x1A01` |
| HUD level word (DGROUP) | `0x83A6` | `0x80E0` | `0x7E42` |
| demo boards | `0.dem, 2.dem, 1.dem` | `3.jn2, 9.jn2, 17.jn2` | `1.jn3, 5.jn3, 12.jn3` |
| demo macros | `jn1demo.mac` | `jn2dem1-3.mac` | `jn3dem1-3.mac` |
| demo level ids | from intro CHECKPT | 3, 9, 17 | 1, 5, 12 |

Jill 1 has no separate demo table: the intro board's CHECKPT strings warp to
`0.dem` / `2.dem` / `1.dem` and the same `jn1demo.mac` drives playback. Jill 2/3
use a real board/macro pointer pair per demo. **[verified]**

### 5.2 Level-title table

36 entries, each a stored far pointer to a NUL-terminated `\r`-formatted title
string in DGROUP. Index = `level_id + 4`; entries 0..3 are the four demo captions
(identical in all three games) and the high indices hold fallback `"NN"`
placeholders or empty strings where a level has no phrase.

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| table image offset | `0x1F481` | `0x1F549` | `0x1FA65` |
| table DGROUP offset | `0x2491` | `0x2229` | `0x2315` |
| stored table segment | `0x1CFF` | `0x1D32` | `0x1D75` |
| entry count | 36 | 36 | 36 |
| first title string (DGROUP) | `0x2549` | `0x22FE` | `0x23EA` |

Example: Jill 1 index 4 is `JILL ENTERS\rTHE\rJUNGLE MAP.\r`, Jill 3 index 12
(the level-8 joke slot) follows the same layout. Full title lists are in
`jill23-frontend-flow.md` section 5; the Jill 1 strings use the same encoding.
**[verified]**

## 6. Values still missing

1. Jill 2/3 base of the active object index list (Jill 1 `0xBCAC`). Only the
   visible count is known. **[open]**
2. Jill 2/3 in-game editor/map board handling; Jill 2 ships no map board.
3. Exact meaning of the two numeric menu codes `0x05` / `0x10`.
4. Whether the `!` handler in Jill 3 returns to `map.jn3` (inferred, not traced).

## 7. `profiles.json`

`docs/profiles.json` mirrors sections 1-5 with one object per episode. It is
emitted by `re/gen_profiles.py`; change the constants there (not in the JSON) and
re-run the script. Keys are stable and snake_case; offsets are integers (decimal)
because JSON has no hex literals. Fields whose value is not known are `null`.

`profile_id` (optional, default `jill<episode>`) is the stable name a game
manifest uses to select this compiled dataset; see `game-manifests.md`.

## See also

* `game-manifests.md` - runtime game/mod manifests and the fourth-game checklist
* `games.json` - installed games merged with these profiles by `re/gen_games.py`
* `jill1-engine.md` - engine, board format, DGROUP map
* `jill2-differences.md`, `jill3-differences.md` - per-episode deltas
* `shared-handler-equivalence.md` - handler reuse proof
* `jill23-frontend-flow.md` - menu, demos, level graph, titles
* `jill23-sounds.md` - `_sound` ABI, remap and call sites
