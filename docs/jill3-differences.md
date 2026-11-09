# Jill 3 (`jill3.exe`) - differences from Jill 2

Third companion to `jill1-engine.md` (Jill 1) and `jill2-differences.md` (Jill 2).
This file only lists deltas against Jill 2; anything not mentioned is unchanged.
Markers: **[verified]** = read from the disassembly or reproduced against the shipped
data files; **[inferred]** = consistent with the data but not proven; **[open]** =
unknown.

Jill 3 is *Jill Saves The Prince*, the third commercial volume. The engine is the same
code base as Jill 2, with the sound subsystem extended and a new graphics set.

Shared-handler evidence is in `shared-handler-equivalence.md`; the Jill 2/3 menu,
demo and level graph in `jill23-frontend-flow.md`; `_sound` ids and call sites in
`jill23-sounds.md`. Every per-episode constant is tabulated in
`episode-profiles.md` (machine-readable copy: `profiles.json`). The engine
frame order, update pass and player state machine (the same code in all three
episodes) are documented in `jill1-player-physics.md`.

---

## 1. File set

| File | Jill 2 | Jill 3 |
|--|--|--|
| executable | `jill2.exe` 185,574 B | `jill3.exe` 204,174 B |
| shapes | `jill2.sha` 260,553 B | `jill3.sha` 263,884 B |
| sampled sound | `jill2.vcl` 85,515 B | `jill3.vcl` 75,528 B |
| config | `jill2.cfg` 254 B | `jill3.cfg` 254 B |
| tile definitions | `jill.dma` | `jill.dma` (same shared file) |
| boards | 11 x `.jn2` | 16 x `.jn3` (`1`-`14`, `intro`, `map`) |
| input macros | `jn2dem1-3.mac` | `jn3dem1-3.mac` |
| title music | `peaks.ddt` | `wild.ddt` |
| save file | `jn2save` | `jn3save` |

There is no Jill 3 manual in this copy either; `jillt/jill.doc` is still the Jill 1
text. [verified]

---

## 2. Executable

### 2.1 MZ header / image **[verified]**

| | Jill 2 | Jill 3 |
|--|--|--|
| file size | 185,574 | 204,174 |
| MZ header (`e_cparhdr` x 16) | 7,168 | 7,680 |
| declared image | 178,406 | 179,638 |
| bytes after the image | 0 (stripped) | **16,856 (debug block)** |
| `e_cp` / `e_cblp` | `0x16B` / `0xE6` | `0x16E` / `0x1B6` |
| relocations (`e_crlc`) | 1,779 | 1,858 |
| `e_minalloc` | `0x136` | 0 |
| `e_ss` / `e_sp` | `0x28FB` / `0xE6` | `0x2948` / `0xE6` |
| entry (`e_cs`:`e_ip`) | `0:0` | `0:0` |

Unlike Jill 2, **Jill 3 ships with its Turbo Debugger block**: the 16,856 bytes after
the declared image contain the same kind of symbol records/names as Jill 1
(`_kindtable`, `_shm_do`, `__turboCrt`, module names, ...). That block is why
`jill3.exe` is larger than `jill2.exe` even though the two images are almost the same
size (`179,638` vs `178,406`, `+1,232` bytes).

Extraction is the same as for Jill 1: slice at `e_cparhdr` (word at `0x08`), not at
`e_cp*16`, and addresses are relative to a load at segment `0x1000`
(`img offset = linear - 0x10000` in `re/img3.bin`).

### 2.2 Segment map

Stored handler segments in the kind table (add `0x1000` for the runtime segment):

| Module | Jill 2 (stored) | Jill 3 (stored) | Jill 3 runtime | Delta from Jill 2 |
|--|--|--|--|--|
| main kind module (APPLE..SCORE, ROMAN) | `0x0671` | `0x0650` | `0x1650` | `-0x21` |
| second kind module (TOKEN..FISHBULLET, new 64-67) | `0x0A6C` | `0x0A51` | `0x1A51` | `-0x1B` |
| player forms (kinds 0, 23, 54-57) | `0x0FC1` | `0x0FCA` | `0x1FCA` | `+0x09` |

The kind-table initializer sits at linear `0x16500` (`0x1650:0000`, the first byte of
the main kind module) and stores 621 words = **69 kinds x 9 fields**, exactly as in
Jill 2.

---

## 3. DGROUP layout

The runtime DGROUP segment is `0x2D75` (Jill 2: `0x2D32`). The internal layout is
**not** a uniform shift from Jill 2 - it was re-derived from the initializer and the
instruction stream:

| Variable | Jill 2 | Jill 3 | Evidence |
|--|--|--|--|
| tile map (128x64 u16) | `0x40E0` | `0x3E42` | board loader |
| board header (0x46 B) | `0x80E0` | `0x7E42` | 12 refs (same count as Jill 1/2) |
| tile table (600 x 8) | `0x8126` | `0x7E88` | 26 refs (`add bx,0x7E88`) |
| object array (192 x 31) | `0x93E6` | `0x9148` | 15 refs (`add ax,0x9148`) |
| object count | `0xB856` | `0xB5F8` | 43 refs, used as `dec ax; mov dx,0x1F; mul` |
| frame counter | `0xBB74` | `0xBC16` | 24 refs |
| kind table: flags | `0xBB76` | `0xBC18` | 13 refs (`test word [0xBC18],imm`) |
| visible/on-screen count | `0xBC06` | `0xBCA8` | `cmp word [0xBCA8],0xC0` |

As in both earlier games the five map/header/tile/object values are contiguous:
`map + 0x4000 = header`, `header + 0x46 = tile table`,
`tile table + 600*8 = object array`.

Kind-table bases in Jill 3 (per-kind stride 4 for far pointers, 2 for u16 fields):

| Field | Jill 3 base | Jill 2 base |
|--|--|--|
| handler segment | `0xB2EC` | `0xB58A` |
| handler offset | `0xB2EA` | `0xB588` |
| width | `0xB152` | `0xB3F0` |
| height | `0xB1DC` | `0xB47A` |
| name offset | `0xB438` | `0xB6D6` |
| name segment (`ds`) | `0xB43A` | `0xB6D8` |
| flags | `0xBC18` | `0xBB76` |
| tileset | `0xB61A` | `0xB878` |
| default count | `0xB6A4` | `0xB902` |

The DGROUP base is validated by the string block: kind 0's name offset is `0x0E22`
and `PLAYER` sits at linear `0x2E572`, so DGROUP = `0x2E572 - 0x0E22 = 0x2D750`.

---

## 4. Kind table: still 69 kinds

Names, widths, heights, tilesets and default counts are **identical to Jill 2 for all
69 kinds**. Only handler addresses moved, plus **two gameplay flag changes**:

| Kind | Name | Jill 2 flags | Jill 3 flags | Effect |
|--|--|--|--|--|
| 7 | `DEMON` | `0x0480` | `0x0000` | no longer killable / no longer a SPINBLAD target |
| 28 | `TOKEN` | `0x0000` | `0x0008` | is now a hit-scan (AABB) participant |

Everything else (including the five new kinds 64-68: `EYE`, `VINECLIMB`, `FLAG`,
`MAPDEMO`, `ROMAN`) is unchanged from Jill 2. [verified]

Shipped `.jn3` boards use kind 64 (3 objects), 65 (21) and 66 (21); `MAPDEMO` and
`ROMAN` still appear in no shipped board. Jill 3 is the first game to actually place
`VINECLIMB` (kind 65) in levels. [verified]

The handler bodies for kinds 64-68 are documented in `jill23-new-kinds.md`; their
behaviour is identical between Jill 2 and Jill 3, only addresses and DGROUP
constants differ.

---

## 5. Board files: `.jn3`

Format is still the Jill 1 format - 0x4000-byte tile map, u16 object count, 31-byte
object records, 0x46-byte header, length-prefixed strings. All 16 boards parse exactly
to EOF with `re/check_jn1.py`'s logic, and named-object count equals string count in
every file. [verified]

| Board | Objects | Named | File size |
|--|--|--|--|
| `1.jn3` | 27 | 2 | 17,310 |
| `2.jn3` | 76 | 24 | 19,199 |
| `3.jn3` | 55 | 5 | 18,189 |
| `4.jn3` | 40 | 3 | 17,723 |
| `5.jn3` | 86 | 12 | 19,222 |
| `6.jn3` | 47 | 4 | 17,951 |
| `7.jn3` | 67 | 2 | 18,550 |
| `8.jn3` | 48 | 4 | 17,972 |
| `9.jn3` | 78 | 2 | 18,891 |
| `10.jn3` | 35 | 2 | 17,561 |
| `11.jn3` | 86 | 2 | 19,142 |
| `12.jn3` | 111 | 5 | 19,942 |
| `13.jn3` | 91 | 4 | 19,328 |
| `14.jn3` | 142 | 34 | 21,601 |
| `intro.jn3` | 106 | 99 | 22,782 |
| `map.jn3` | 33 | 14 | 17,603 |

Notes:

* **`map.jn3` is back.** Jill 2 shipped no overworld board; Jill 3 does, with 14 named
  objects (the level nodes). [verified]
* Level ids are contiguous `1..14`, plus `intro` and `map`.
* `5.jn3` stores `0xC156` in header word 0 (the level/checkpoint field); every other
  board stores 0, and `intro` stores 1. This looks like an editor leftover, not a
  format change. [open]
* Header word 1 is health 6 on every new board; word 2 is the key count (0 on
  `12.jn3`/`14.jn3`/`intro.jn3`, 1 elsewhere).

---

## 6. Assets

### 6.1 `jill3.sha` **[verified]**

Same directory layout as Jill 1/2 (128 offsets, 128 sizes, first data at `0x300`,
63 populated entries, `sum(sizes) == filesize - 0x300`), and the size table is
identical to Jill 1/2 **except two entries**:

| Entry | Tileset | Jill 2 size | Jill 3 size | Change |
|--|--|--|--|--|
| 57 | `0x39` PAUL | 3,053 | 6,128 | 10 -> **11** frames; planar -> packed (`flags` 4 -> 0); 4bpp decoded size 1,320 -> 2,860 |
| 58 | `0x3A` BUNNY | 737 | 993 | 6 frames; same decoded sizes; colour table/re-encoding changed |

All other entries keep their Jill 1/2 sizes, but several were **redrawn** (byte diffs
against Jill 1):

| Entry | Tileset / owner | Bytes changed (of size) |
|--|--|--|
| 7 | terrain / default tile sheet | 480 / 1,005 |
| 8 | PLAYER | 6 / 27,828 |
| 40 | `0x28` EPIC | 1,428 / 4,455 |
| 47 | `0x2F` SKULL | 898 / 2,355 |
| 3, 25, 48 | MAPDEMO sheet / unused tilesets `0x19`, `0x30` | 12, 11, 11 |

The remaining entries differ only in 1-6 bytes (probably header/palette fields, not
redraws): the art is effectively the same. So Jill 3 ships its own terrain, EPIC,
SKULL, PAUL and BUNNY artwork, and reuses the rest. **[inferred for the small diffs]**

### 6.2 `jill3.vcl`

Same header layout (50 u32 offsets / 50 u16 sizes / 50 u16 rates, 40-entry
text-page table at `0x190`/`0x230`): [verified]

| | Jill 2 | Jill 3 |
|--|--|--|
| file size | 85,515 | 75,528 |
| non-empty SFX (bank 1) | 24 | 23 |
| bank-1 sample bytes | 82,300 | 71,965 |
| sample rates | 6000, 6024 | 6000 |
| non-empty text pages | 1 | **3** (sizes 1,581 / 180 / 137) |

So Jill 3's sound bank is smaller than Jill 2's but ships three text pages
("How the Heck to Play", "No!", "Castle").

### 6.3 `jill3.cfg` **[verified]**

Identical to `jill2.cfg` (a fresh default state: same three bytes differ from
`jill1.cfg`), same 254-byte four-block layout. No format change.

### 6.4 `jill.dma`, demos, music

* `jill.dma` is the same shared file; no parser change. [verified]
* Demos use the same record format and all three parse exactly to EOF:
  `jn3dem1.mac` 139 records / last tick 1,079; `jn3dem2.mac` 138 / 949;
  `jn3dem3.mac` 159 / 933. [verified]
* Title music is `wild.ddt` (Jill 2: `peaks.ddt`), standard CMF/`CTMF`.

---

## 7. Runtime/feature changes

Jill 3 is *Jill Saves The Prince*. New/changed text strings (vs Jill 2):
[verified]

* title and end text: `Jill Saves The Prince`,
  `Thank you for playing Jill Saves the Prince!`,
  `This is the third volume of the Jill series from Epic`.
* new level-name fragments: `JILL BOARDS`, `JILL DISCOVERS`, `JILL FINALLY`,
  `JILL LEAPS INTO`, `JILL ROAMS`, `JILL TRYS HER`, `THE AERIE`, `THE AQUEDUCT`,
  `THE CASTLE`, `THE GIANT GREEN`, `THE SECRET FOREST`, `THE SHIP OF`, `THE VALLEY`,
  `THROUGH THE`, `TO THE DAM`, `INTO THE JAIL`, `LUCK IN THE`, `LEVEL ELEVEN`,
  `NUMBER TWELVE,`, `NEXT LEVEL IS`, `MEGA PUZZLE`, `PYRAMID PUZZLE.`, `LIZARD MEN!`,
  `JUNGLE MAP`. Jill 3 also carries some complete Jill 1 phrases again
  (`JILL BOUNDS INTO`, `JILL EXPLORES`, `JILL JOURNEYS`, `JILL VENTURES`), so its
  title table is a superset of both earlier schemes. **[inferred]**
* new in-game messages: `A BAG OF COINS!`, `YOU FIND A THROWING STAR`,
  `YOU'RE RIGHT!`, `IF YOU THINK THE`.
* command line: `/NOSND`, `/SB`, `/DEMO` added (Jill 2's `/TEST` and `/NOSB` are
  still present), plus the help lines ` /SB    (With a Sound Blaster)`,
  ` /NOSB  (No Sound Blaster card)`, ` /NOSND (If all else fails)` and
  `If your system locks, reboot and type:`.
* sound diagnostics: `Detecting your hardward...` (typo is in the original) and
  `The problem may be due to not enough free RAM or disk space.`

The debug block also lists source modules that do not exist in Jill 1:
`DIGI.ASM`, `SBCRWIO.ASM`, `SCANCARD.ASM`, `CARDHERE.ASM`, `DSPRESET.ASM`,
`VECTOR.ASM` and `JVOL3.C`, and variables `_vocptr/_voclen/_vocposn/_vocrate/_vocpri/
_vocflag/_vocfilehandle` plus `_DacRead/_DacWrite/_DacData`. This is a **new
digitized-sound / VOC playback path** on top of the Jill 2 Sound Blaster code.
**[inferred from names; handler code not decompiled]**

The `W`+digit warp, `X`x3, `Z`x3, `T`, `P`, `N` and macro keys are presumably
unchanged from Jill 2. **[open]**

---

## 8. Port parameterisation

Extend the per-episode profile from `jill2-differences.md` to three entries:

| Item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| shape / sound / config | `jill1.*` | `jill2.*` | `jill3.*` |
| board extension | `.jn1` | `.jn2` | `.jn3` |
| save name | `jn1save` | `jn2save` | `jn3save` |
| demos | `jn1demo.mac` | `jn2dem1-3.mac` | `jn3dem1-3.mac` |
| title music | `funky.ddt` | `peaks.ddt` | `wild.ddt` |
| boards | `1,2,3,4,6,9,50,intro,map` | `0,1,3,9,10,11,13,15,17,20,intro` | `1..14,intro,map` |
| kind count | 64 | 69 | 69 |
| DGROUP base | `0x2CFF` | `0x2D32` | `0x2D75` |
| DGROUP offsets | `jill1-engine.md` #3 | `-0x2C6` | section 3 above |
| title | Jill of the Jungle | Jill goes Underground | Jill Saves The Prince |

The board/save/config/macro formats and the handler ABI stay shared. Jill 3 adds a
sound path, new artwork and two flag tweaks; the five extra kinds already exist in
Jill 2. The `DEMON`/`TOKEN` flag differences mean the flag table must be per-episode
data, not a constant shared by Jill 2 and Jill 3.

---

## 9. Open questions

1. Behaviour of `MAPDEMO` (67) and `ROMAN` (68), still unused by any shipped board.
2. The `0xC156` value in `5.jn3` header word 0.
3. Whether the VOC/DIGI path is required for correct Jill 3 audio, or only an
   alternate sample source for the `.VCL` bank.
4. Which of the redrawn shape sheets (terrain, EPIC, SKULL, PAUL, BUNNY) are actually
   visible in the shipped 14 levels.
5. Jill 3's debug block makes full symbol recovery possible (as was done for Jill 1);
   mapping its records to addresses would name the new sound routines and the five
   new kind handlers.
