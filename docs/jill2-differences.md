# Jill 2 (`jill2.exe`) - differences from Jill 1

Companion to `jill1-engine.md`. Everything here is a delta against Jill 1; anything
not mentioned is unchanged from that document. Confidence markers: **[verified]** =
read from the disassembly or reproduced against the shipped data files; **[inferred]** =
consistent with the data but not proven; **[open]** = unknown.

Method: static analysis of `jillt/jill2.exe`, extraction of the load module to
`re/img2.bin`, a byte-level diff against `re/img.bin`, and parsers run over every
`.jn2` / `.mac` / `.cfg` / `.sha` / `.vcl` file in `jillt/`.

For Jill 3 (`jill3.exe`, `.jn3`, 69 kinds, new sound path) see
`jill3-differences.md`. Shared/ported behaviour is collected in
`shared-handler-equivalence.md`, `jill23-frontend-flow.md` and
`jill23-sounds.md`. The consolidated per-episode constant list is
`episode-profiles.md` / `profiles.json`. Frame order, the update pass and the
player state machine (identical code in all episodes) are in
`jill1-player-physics.md`; #10 there lists the per-episode offsets that matter
to the player.

---

## 1. File set

| File | Jill 1 | Jill 2 | Notes |
|--|--|--|--|
| executable | `jill1.exe` 216,804 B | `jill2.exe` 185,574 B | Jill 2 has no debug block (see #2) |
| shapes | `jill1.sha` 260,553 B | `jill2.sha` 260,553 B | same directory + same sizes, 4 entries differ |
| sampled sound | `jill1.vcl` 94,954 B | `jill2.vcl` 85,515 B | same header layout, different contents |
| config | `jill1.cfg` 254 B | `jill2.cfg` 254 B | same format |
| tile definitions | `jill.dma` | `jill.dma` | **shared file**, identical |
| boards | `*.jn1` (9) | `*.jn2` (11) | same format, different level set |
| input macros | `jn1demo.mac`, `end1.mac`, `end2.mac` | `jn2dem1.mac`-`jn2dem3.mac` | 3 demos instead of 1 |
| music | `funky.ddt` | `peaks.ddt` | same CMF/CTMF format |
| save file | `jn1save` | `jn2save` | name is a string in the EXE |

`jillt/jill.doc` is still the **Jill 1, Volume I** manual text; Jill 2 has no separate
manual in this copy. [verified]

---

## 2. Executable layout

### 2.1 MZ header / image **[verified]**

| | Jill 1 | Jill 2 | Jill 3 (reference) |
|--|--|--|--|
| file size | 216,804 | 185,574 | 204,174 |
| MZ header (`e_cparhdr` x 16) | 7,168 | 7,168 | 7,680 |
| declared image | 185,478 | 178,406 | 179,638 |
| bytes after the image | 24,158 (debug block) | **0** | 16,856 (debug block) |
| `e_cp` / `e_cblp` | `0x16B` / `0x86` | `0x16B` / `0xE6` | `0x16E` / `0x1B6` |
| `e_minalloc` | 0 | **`0x136`** | 0 |
| `e_ss` / `e_sp` | `0x28F5` / `0xE6` | `0x28FB` / `0xE6` | `0x2948` / `0xE6` |
| entry (`e_cs`:`e_ip`) | `0:0` | `0:0` | `0:0` |

Jill 1's trailing 24,158 bytes are the Turbo Debugger symbol block (15,028 + 9,130
bytes, see `jill1-engine.md` #1.1). **Jill 2 was shipped stripped:** its file ends
exactly at the end of the declared image, and no Turbo C symbol/module names are
present. Its load image is also 7,072 bytes smaller than Jill 1's (178,406 vs
185,478), even though the kind table grew by five entries.

Jill 2 additionally asks DOS for at least `0x136` paragraphs (`~5 KB`) of heap beyond
the image; Jill 1 asks for none.

### 2.2 Extraction caveat

Use `e_cparhdr` (word at file offset `0x08`), **not** `e_cp` (offset `0x04`), as the
header size when slicing the image. Using `e_cp * 16` shifts the extracted image by
`0x550` bytes and produces plausible-looking but wrong linear addresses. The corrected
module is `re/img2.bin` (178,406 B); `re/img3.bin` (196,494 B) was re-extracted the same
way. Addresses below are relative to a load at segment `0x1000`, so
`img offset = 0x1C00 + (linear - 0x10000)` in the EXE file and
`img offset = linear - 0x10000` in `re/img2.bin`.

### 2.3 Segment map **[verified by the kind table]**

The three object-handler modules kept their roles but moved:

| Module | Jill 1 (stored) | Jill 1 runtime | Jill 2 (stored) | Jill 2 runtime | Delta |
|--|--|--|--|--|--|
| main kind module (APPLE..SCORE, plus new kind 68) | `0x0644` | `0x1644` | `0x0671` | `0x1671` | `+0x2D` |
| second kind module (TOKEN..FISHBULLET, MACROTRIG/TXTMSG, new 64-67) | `0x0A41` | `0x1A41` | `0x0A6C` | `0x1A6C` | `+0x2B` |
| player forms (kinds 0, 23, 54-57) | `0x0FAE` | `0x1FAE` | `0x0FC1` | `0x1FC1` | `+0x13` |

The table initializer itself is at linear `0x16717` (Jill 1: `0x16442`) and has the
same 9-stores-per-kind layout. A port must treat these as relocatable symbols - never
copy the segment words.

---

## 3. DGROUP shift

All static addresses below the kind table moved by **-0x2C6**, i.e. 0x2C6 bytes less
data now precedes them (the 69-entry tables themselves are larger and sit after the
map). The runtime DGROUP segment is `0x2D32` (Jill 1: `0x2CFF`).

| Variable | Jill 1 | Jill 2 | Evidence |
|--|--|--|--|
| tile map (128x64 u16) | `0x43A6` | `0x40E0` | loader/save references |
| board header (0x46 B) | `0x83A6` | `0x80E0` | 12 refs at the new offset, 0 at the old |
| tile table (600 x 8) | `0x83EC` | `0x8126` | 24 refs (`add bx,0x8126`) |
| object array (192 x 31) | `0x96AC` | `0x93E6` | 15 refs (`add ax,0x93E6`) |
| object count | `0xBB1C` | `0xB856` | 37 refs at the new offset, 0 at the old |
| frame counter | `0xBE3A` | `0xBB74` | 24 refs |
| kind table: flags | `0xBE3C` | `0xBB76` | 14 refs (`test word [0xBB76],imm`) |

**Verification:** for each address above the EXE contains the same number of
references at the shifted offset and none at the Jill 1 offset. Representative
encodings: `add ax, 0x93E6` (object stride), `add bx, 0x8126` (tile table),
`test word [0xBB76], imm` (kind flags).

Kind-table bases in Jill 2 (all offsets, each +4 or +2 per kind):

| Field | Jill 2 base | Jill 1 base |
|--|--|--|
| handler segment | `0xB58A` | `0xB850` |
| handler offset | `0xB588` | `0xB84E` |
| width | `0xB3F0` | `0xB6B6` |
| height | `0xB47A` | `0xB740` |
| name offset | `0xB6D6` | `0xB99C` |
| flags | `0xBB76` | `0xBE3C` |
| tileset | `0xB878` | `0xBB3E` |
| default count | `0xB902` | `0xBBC8` |

(The handler/name *segment* half is `ds`, written with `mov [addr],ds`.)

---

## 4. Kind table: 64 -> 69

The table now has **69 entries (0..68)**. Entries 0..63 are identical to Jill 1 in
every parsed field - width, height, flags, tileset, default count and name; only the
handler far pointer changed (code was relinked, see #2.3). Five kinds were appended:

| Kind | Name | Handler (stored) | w | h | Flags | TS | Cnt | Notes |
|--|--|--|--|--|--|--|--|--|
| 64 | `EYE` | `0A6C:46A9` | 16 | 12 | `0x0180` | `0x3E` | 3 | killable + receives `_sendtrig` |
| 65 | `VINECLIMB` | `0A6C:481E` | 16 | 8 | `0x0000` | `0x3D` | 0 | climbable decoration |
| 66 | `FLAG` | `0A6C:4A18` | 36 | 16 | `0x0008` | `0x05` | 0 | hit-scan trigger (3-frame strip) |
| 67 | `MAPDEMO` | `0A6C:4C4D` | 64 | 16 | `0x0000` | `0x03` | 0 | map/demo marker |
| 68 | `ROMAN` | `0671:2F36` | 16 | 32 | `0x0480` | `0x2C` | 12 | killable + SPINBLAD target |

Flag meanings are the ones proven in `jill1-engine.md` #6.2: `0x0008` hit-scan AABB,
`0x0080` killable by KNIFE/FIREBULLET, `0x0100` receives `_sendtrig`, `0x0400`
SPINBLAD target, `0x4000` projectile. The new-kind roles are **[inferred]** from the
name and flag combination; handler bodies were not decompiled.

Shipped boards use kind 64 (10 objects) and kind 66 (6 objects); 65, 67 and 68 are
defined but do not appear in any shipped `.jn2`. [verified]

Only two new tilesets are referenced: `0x3D` (VINECLIMB) and `0x3E` (EYE). Both shape
entries exist unchanged in `jill1.sha` (entries 61 and 62), so the art was shipped in
Jill 1 but unused there. [inferred]

---

## 5. Board files: `.jn2`

The format is **unchanged** from `.jn1`: [verified]

```
0000  0x4000  tile map, 128 x 64 x u16 (low 14 bits tile id, top 2 bits flags)
4000  u16     object count
4002  N x 31  object records (kind byte first, name offset at +23)
....  0x46    board header (same layout as jill1-engine.md #7)
....  ...     NUL-assisted length-prefixed strings (u16 len + bytes + NUL)
```

The existing `re/check_jn1.py` parser lands exactly on EOF for all 11 boards, and the
number of non-empty object names equals the number of strings in every file
(`EXACT` for all). The 0x46-byte header has the same field layout and the same kind of
values as `.jn1`: level/checkpoint id at `+0`, health 6 at `+2`, key count at `+4`,
and the same repeated-value block from `+6` onward (Jill 1 `1.jn1` holds `0x0032`
there, Jill 2 `11.jn2` holds `0x01A6`). **[verified]**

| Board | Objects | Named | File size |
|--|--|--|--|
| `0.jn2` | 48 | 2 | 17,965 |
| `1.jn2` | 91 | 7 | 19,354 |
| `3.jn2` | 133 | 13 | 20,708 |
| `9.jn2` | 77 | 3 | 18,874 |
| `10.jn2` | 91 | 7 | 19,345 |
| `11.jn2` | 124 | 23 | 20,436 |
| `13.jn2` | 113 | 18 | 20,179 |
| `15.jn2` | 82 | 8 | 19,093 |
| `17.jn2` | 130 | 21 | 20,726 |
| `20.jn2` | 32 | 28 | 18,239 |
| `intro.jn2` | 149 | 112 | 24,250 (credits text) |

Differences from Jill 1's board set:

* extension `.jn2` instead of `.jn1`;
* **no `map.jn1` equivalent** - Jill 1 ships `map.jn1`, Jill 2 ships none, so the
  overworld/map screen is likely drawn from code or a different resource. **[open]**
* level ids are `0,1,3,9,10,11,13,15,17,20,intro` (gaps; Jill 1 ships
  `1,2,3,4,6,9,50,intro,map`).
* object names still come from the same string pool; kinds 0..68 all fit in the kind
  byte.

---

## 6. Other assets

### 6.1 `jill2.sha` **[verified]**

Same layout and **identical 128-entry offset table and 128-entry size table** as
`jill1.sha`; 63 populated entries. Only four entries differ in content:

| Entry | Bytes changed | Shape size | Likely owner |
|--|--|--|--|
| 3 | 17 | 5,246 | MAPDEMO sheet (kind 67; kind is not placed in any shipped board) |
| 7 | 448 | 1,005 | sheet holding the default/blank map tile `0x4700` -> terrain/background art |
| 8 | 6 | 27,828 | PLAYER sheet (tileset `0x08`) - tiny palette/transparency fix |
| 25 | 53 | 5,774 | unused tileset `0x19` (no kind uses it) |

Entries 3 and 25 are the only asset diffs with no shipped-level impact. **[open]**
They may belong to the intro or to a map screen that is gone from this board set.

### 6.2 `jill2.vcl`

Same header layout as `jill1.vcl` (50 u32 offsets / 50 u16 sizes / 50 u16 rates at
`0`, `0xC8`, `0x12C`; 40-entry text-page table at `0x190`/`0x230`). [verified]

| | Jill 1 | Jill 2 |
|--|--|--|
| file size | 94,954 | 85,515 |
| non-empty SFX (bank 1) | 23 | 24 |
| bank-1 sample bytes | 88,748 | 82,300 |
| sample rates | 6000 | 6000 and 6024 |
| non-empty text-page entries | 14 | **1** |

Jill 2 keeps only one text page ("How the Heck to Play"); the hint and news
pages were Jill 1 extras.  Music is not in the `.VCL` at all for either episode;
the `.ddt` files hold it.  Treat the `.VCL` as a per-episode table, not a fixed
set of sounds.

### 6.3 `jill2.cfg` **[verified]**

Identical 254-byte layout (120 B names / 40 B u32 scores / 72 B slot labels / 22 B
options). The shipped file differs from `jill1.cfg` in only 3 bytes: one character in
name slot 6, one score word (slot 8, 0 vs 23) and the first restore-slot label.
So the shipped `jill2.cfg` is simply a fresh default state; the loader/saver needs no
changes beyond the file name.

### 6.4 `jill.dma` **[verified]**

The same file is used by both games (`jill.dma` appears in both EXEs) and is not
versioned. There are no `EYE`/`VINECLIMB`/`MAPDEMO`/`ROMAN` records: `.dma` only maps
**map tiles** to shapes, not object sprites (object art comes from the kind's tileset
byte). No parser change needed.

### 6.5 `*.mac` demos **[verified]**

Same record format. Jill 2 ships three short demos instead of Jill 1's single
`jn1demo.mac`:

| File | Records | Last tick |
|--|--|--|
| `jn1demo.mac` (Jill 1) | 519 | 4295 |
| `jn2dem1.mac` | 102 | 984 |
| `jn2dem2.mac` | 130 | 875 |
| `jn2dem3.mac` | 125 | 1012 |

The demo names are hardcoded strings in the EXE, so a port needs a per-episode list.

### 6.6 Music

Jill 1 loads `funky.ddt`; Jill 2 loads `peaks.ddt` (both standard CMF/`CTMF`). Level
songs are still selected by the board string command `*name.ddt`, so only the title
track name changes. [verified by strings]

---

## 7. Strings and feature changes

New strings in `jill2.exe` (none present in `jill1.exe`): [verified]

* level-name **fragments** instead of whole phrases:
  `JILL GOES`, `JILL DESCENDS`, `JILL DRIFTS`, `JILL ESCAPES`, `JILL JUMPS`,
  `JILL RUNS INTO`, `JILL SWIMS TO`, `JILL WANDERS`, `JILL BETTER`,
  `UNDERGROUND`, `INTO THE LAND`, `OF ETERNAL`, `DEPTHS OF`, `WEIRDNESS`,
  `MONTEZUMA'S`, `THE RED PUZZLE`, `DEMONIC MAZE`, `THE WATERWORLD`,
  `INTO A STICKY`, `SITUATION`, `CASTLE`.
  Jill 1 instead stores complete titles (`JILL BOUNDS INTO`, `JILL CREEPS INTO`,
  `JILL DASHES INTO`, `JILL EXPLORES`, ...). The new scheme appears to concatenate
  two fragments per level. **[inferred]**
* `Stack overflow!`
* command-line / test switches `/TEST` and `/NOSB` (Jill 1 has neither).
* new title/credits text: `Epic MegaGames Presents`, `A Commercial Production`,
  `Jill goes Underground`, `Copyright 1992 Epic MegaGames`, credits for
  `Joe Hitchens`, `John Pallett-Plowright`, `Dan Froelich`.
* new registration wording: `Only volume one is shareware.`,
  `This is the second volume of the Jill series from Epic`,
  `Thank you for playing Jill goes Underground!` (Jill 1's text is volume-one
  shareware wording).
* `jn2save`, `jill2.cfg/.sha/.vcl`, `jn2dem1-3.mac`, `intro.jn2`,
  `0.jn2`, `3.jn2`, `9.jn2`, `17.jn2`, `peaks.ddt`.

No new cheat keys are implied by the strings; the `W`+digit warp, `X`x3, `Z`x3,
`T`, `P`, `N` bindings from Jill 1 are presumably unchanged. **[open]**

---

## 8. What a port must parameterise

One engine binary can run both episodes if the following are per-episode data, not
constants:

| Item | Jill 1 | Jill 2 |
|--|--|--|
| shape file | `jill1.sha` | `jill2.sha` |
| sound file | `jill1.vcl` | `jill2.vcl` |
| config file | `jill1.cfg` | `jill2.cfg` |
| board extension | `.jn1` | `.jn2` |
| save name | `jn1save` | `jn2save` |
| demo macros | `jn1demo.mac` | `jn2dem1-3.mac` |
| title music | `funky.ddt` | `peaks.ddt` |
| board list | `1,2,3,4,6,9,50,intro,map` | `0,1,3,9,10,11,13,15,17,20,intro` |
| kind count | 64 | 69 |
| DGROUP base | `0x2CFF` | `0x2D32` |
| DGROUP variable block | Jill 1 offsets | all Jill 1 offsets `-0x2C6` |
| level-title table | full phrases | fragment pairs |

Everything else is shared: the `.sha`/`.vcl`/`.cfg`/`.dma` directory layouts, the
board format, the 31-byte object record, the 0x46-byte header, the `.mac` format and
the handler ABI (`handler(obj, action, other)`). The five new kinds are additive; the
64 Jill 1 handlers can be reused unchanged once the offsets are parameterised.

---

## 9. Open questions

1. Which objects/levels use `jill2.sha` entries 3 and 25 (only changed art not
   referenced by the kind table or the shipped boards).
2. Whether Jill 2 has a map/overworld board at all, and how it is drawn.
3. Precise behaviour of EYE / VINECLIMB / MAPDEMO / ROMAN - now decompiled, see
   `jill23-new-kinds.md`. Only the `DGROUP:0x13A8` map-demo flag writer and the
   ROMAN frame-count mismatch remain open.
4. ~~Why the `.VCL` second bank is nearly empty in Jill 2 (music moved to `.DDT`?)~~
   Resolved: the second bank is the text-page table, and Jill 2 ships only page 1.
5. Jill 3 (`jill3.exe`, `re/img3.bin`, 16,856 B debug tail) was not analysed; it may
   add more kinds and boards. A three-episode profile table should be the target.
