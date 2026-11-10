# Jill of the Jungle - `jill1.exe` engine notes

Reverse-engineering notes for the 16-bit DOS original (`JILL1.EXE`, Epic MegaGames, 1992),
written as groundwork for the native port described in `TODO.md`.

Everything below was derived from static analysis only:

* Ghidra 12.2 headless (`x86:LE:16:Real Mode`) -> decompiled C in `re/export/decomp/`
  (428 functions, 422 decompiled), index in `re/export/index.txt`.
* `rizin -a x86 -b 16 -m 0x10000` on the raw load module `re/img.bin` to verify
  addresses the decompiler garbles (DGROUP-relative accesses, relocation values).
* Turbo C debug symbol names recovered from the EXE overlay (`re/exe_symbols.txt`,
  1012 names including source module, function and variable names).
* Direct inspection of the shipped data files in `jillt/`.

Confidence markers used below: **[verified]** = read from disassembly or reproduced
against the data files; **[inferred]** = consistent behaviour but not proven;
**[open]** = unknown.

For the **Jill 2** delta (`jill2.exe`, 69 kinds, `.jn2` boards, shifted DGROUP) see
`jill2-differences.md`; for **Jill 3** (`jill3.exe`, `.jn3`, new sound path) see
`jill3-differences.md`. Cross-episode material lives in
`shared-handler-equivalence.md` (which handler bodies really changed),
`jill23-frontend-flow.md` (menu, demos, level graph, title table),
`jill23-sounds.md` (`_sound` ABI, id remap, every call site) and
`episode-profiles.md` (all per-episode constants in one table, mirrored by
`profiles.json`). The frame order, object update pass and player physics are in
`jill1-player-physics.md`.

---

## 1. The executable

### 1.1 File layout **[verified]**

| Offset | Size | Contents |
|--|--|--|
| `0x00000` | `0x01C00` | MZ header + relocation table (1767 entries) |
| `0x01C00` | `0x2D486` (185 478) | load module (`jill1.exe` image, starts at `1000:0000`) |
| `0x2F086` | `0x3AE2` (15 028) | 3757 x 4-byte Turbo Debugger symbol records |
| `0x32B3A` | `0x23AA` (9 130) | 1012 NUL-terminated symbol names, ends exactly at EOF |

The overlay is not executed; it is the linker's debug/symbol block. Its name list is
alphabetically ordered per module group and contains **all source-level identifiers**,
which is what makes the function names (`_play`, `_loadboard`, `_addobj`, `_pixaddr_vga`,
`_shm_do`, `_sbfm_...`, ...) recoverable.

Relocation: `img file offset = 0x1C00 + (linear - 0x10000)`, where
`linear = segment*16 + offset`. Segment values stored in the image are pre-relocation,
i.e. the runtime segment is `value + 0x1000` (e.g. a stored `0x0FAE` is the runtime
segment `0x1FAE`). The decompiler already applied this, so **decompiled segment halves
must not be incremented again**.

The MZ relocation table is partly malformed for naive parsers (1326 of 1767 targets fall
outside the image if you assume a single header size); don't depend on it.

### 1.2 Segment map **[inferred from code roles]**

| Runtime seg | Role |
|--|--|
| `0x1000` | Turbo C 2.0 startup (`__c0crtinit`, `__crtinit`, `_exit`, `__REALCVT`) |
| `0x1069` | Shape/bitmap resource loader (`_shm_init`, `_readin`, `_latch_*`, `.SHA` loading) |
| `0x1123` | Graphics driver: mode set, palette, page flip, clearing (GR) |
| `0x11F4` | Planar/E0 sprite blitter, viewport scroll (`_drawshape`, `_ldrawsh_*`, `_scrollvp`) |
| `0x1415` | Keyboard/joystick/macro input (KEYBOARD) |
| `0x150F` | Level-warp helper used by the `W`+digit cheat |
| `0x1530` | PC-speaker driver (PIT ch2, int 1Ch ISR, `_noisemaker`) |
| `0x1581` | Sound/music front end + `.VCL`/`.DDT` loading (`_dolevelsong`, `_play`) |
| `0x15ED` | Sound Blaster DSP detection/control (6 base ports x 4 IRQ/DMA combos) |
| `0x1644` | Object handlers, kinds 1-27 (APPLE ... SCORE) |
| `0x1A41` | Object handlers, kinds 28-63 + drawing/text helpers |
| `0x1FAE` | Player and player-forms (kinds 0, 23, 54-57) |
| `0x22AB` | Game flow: board load/save, main loop, HUD, menus (JUNGLE) |
| `0x25EB` | Design/editor object inspector printing |
| `0x26F4` | Object engine: spawn, update, collide, draw, cull |
| `0x2B31`-`0x2CE7` | Turbo C runtime (`fopen`/`fread` wrappers, `sprintf`, `atoi`, far malloc) |

### 1.3 DGROUP **[verified]**

`DGROUP` = segment `0x2CFF`, linear `0x2CFF0`, i.e. file offset
`0x1EBF0 + dgroup_offset` (e.g. the string `"PLAYER"` at DGROUP:`0xC76` -> file `0x1F866`).

Important globals (all DGROUP-relative):

| Offset | Meaning |
|--|--|
| `0x0194` | 768-byte palette buffer (`vga_setpal` source); pre-filled from the executable image with the default 256-colour palette |
| `0x0494` / `0x0498` | CRTC index/data ports (3D4/3D5 or 3B4/3B5), chosen per video card |
| `0x0AAE` | saved int 1Ch vector (speaker driver) |
| `0x0BF0` | far pointer to loaded `.DDT` music buffer |
| `0x0BF6` | Sound Blaster base port |
| `0x120A` | "no-clip / invincible" cheat toggle (Zx3) |
| `0x1210` | game-speed toggle (T) and HUD x-offset |
| `0x2FAE` | 64 x u16 "resource needed" flags (set from board tiles + object tilesets) |
| `0x3A32` | viewport / clipping rectangle (`x, y, w=0x140, h=200, ...`) |
| `0x3A4D` | video backend id: `0` = CGA/EGA-ish, `2` = EGA 16-colour, `4` = VGA 320x200x256 |
| `0x3C76`/`0x3C78` | shift/alt key-down flags |
| `0x3C7A`/`0x3C7E` | ctrl / "any key" flags |
| `0x3C86` | last key code (ASCII-ish, high bit stripped semantics) |
| `0x3DC2` | macro **recording** active |
| `0x3DC8` | macro **replay** active |
| `0x3E26` | sound-enable flag (toggled by `N`) |
| `0x3E28` | software tick counter (18.2 Hz timer ISR) |
| `0x3E40` | time of last music command |
| `0x3F10`/`0x3F12` | 50-entry digital-SFX table (far offset/segment) |
| `0x3FDA`/`0x3FDC` | 50-entry "song/pattern" table (far offset/segment) |
| `0x41DA`, `0x42F6` | `.VCL` 40-entry text-page sizes / offsets |
| `0x43A6` | **tile map**, 0x4000 bytes = 128x64 x u16 |
| `0x83A6` | **board header** (0x46 bytes) - also the live HUD/state variables |
| `0x96AC` | **object array**, up to `0x100` x 31 bytes (`0x96AC..0xB5AC`); board files cap the count at 192 |
| `0xB6B6` | kind table: width (64 x u16) |
| `0xB740` | kind table: height (64 x u16) |
| `0xB7F2` | camera struct (`+6` view w, `+8` x, `+10` y) |
| `0xB84E` | kind table: handler far pointer (64 x 4) |
| `0xB99C` | kind table: name far pointer (64 x 4) |
| `0xBB1C` | object count |
| `0xBB1E` | pending level-transition string (filled by object handlers) |
| `0xBB3E` | kind table: tileset id (64 x u16) |
| `0xBBC8` | kind table: default "count" (64 x u16) |
| `0xBCAC` | active/visible object index list (u16), count at `0xBECC` (cap 0xC0) |
| `0xBE3C` | kind table: flags (64 x u16) |

---

## 2. Boot sequence

Entry `1000:0000` is stock Turbo C startup; it ends in the game's `main`/`_play`
chain in segment `0x22AB`.

`FUN_22ab_2ed8` (game start) does, in order: **[verified]**

1. clear screen / reset graphics (`FUN_22ab_095b`, `FUN_2c48_0006`), open the
   text window used for messages (`0x184a` string, `0xB800` video base).
2. `FUN_1581_0009("jill1.vcl")` - load the sampled-sound bank (see #10).
3. `FUN_1415_0a94` - install keyboard ISR, calibrate/read joystick, load `jill1.cfg`.
4. `FUN_14cc_0004` - config/driver menu (`Video mode: C)ga E)ga V)ga?`,
   `Game controller: K)eyboard, J)oystick?`, joystick calibration).
5. `FUN_1123_0a9b` + `FUN_1123_08cd` - set video mode, allocate off-screen page,
   clear the 256-entry palette.
6. `FUN_1069_000d("jill1.sha")`, set "needed shape" flags and `FUN_1069_094d`
   to load them (see #9).
7. `FUN_1581_069e("funky.ddt")` - load the title music.
8. `FUN_22ab_1aed` - reset kind tables / draw HUD frames.
9. `FUN_22ab_001f` - load `.VCL` music tables for the level songs.
10. Set camera struct, viewport and HUD globals; install the 18.2 Hz tick handler.
11. Enter the menu (`FUN_22ab_2aed`, `PICK A CHOICE`, `PLAY`, `RESTORE`, ...) and then
    `FUN_22ab_1531` = the play loop.

---

## 3. Main loop (`FUN_22ab_1531`, symbol `_play`) **[verified]**

One iteration, in order:

1. `FUN_22ab_05b4("PRESS F1 FOR HELP", 4)` - one-shot intro message.
2. Init object engine (`FUN_26f4_1ab9`, `FUN_26f4_215f`), load HUD (`FUN_22ab_0319`,
   `FUN_22ab_05dd`), enable page flipping.
3. **Level-transition check**: if `DGROUP:0xBB1E` is a non-empty string, dispatch on its
   first character (see #7).
4. Read timer (`tick = DGROUP:0x3E28`), service music (`FUN_1581_063e`), bump frame
   counter, poll keyboard (`FUN_1415_026f(1)`).
5. Cheat / special-key handling (below).
6. Object engine:
   `FUN_26f4_11d7` (tile pre-pass: tiles whose attribute has bit `0x20` run the
   `1F28:0006` hook and may get `0x4000` OR-ed into the map cell), `FUN_26f4_12b9(1)` (update+collide),
   `FUN_26f4_259b` (sort/visibility), `FUN_26f4_0d52(page)` (draw),
   `FUN_26f4_2506` (HUD/overlays).
7. Jump-table dispatch on the key (`piVar5 = 0x19DF`, 5 entries) for menu-ish keys.
8. **Frame limiter**: busy-wait `while (DGROUP:0x3E28 - tick < speed + 1)` where
   `speed = DGROUP:0x1210` (0 or 1).

With a BIOS-rate 18.2 Hz tick this yields ~18 or ~9 fps; the `T` key toggles it.
**[inferred]**

### Special keys **[verified]**

| Key | Action |
|--|--|
| `X` x3 | health = 8, grants item 10 and item 1 (extra life/key) |
| `Z` x3 | toggles `DGROUP:0x120A` (invincibility / no-clip) |
| `W` + digit | warp to level N (`FUN_150f_0008`) |
| `N` | toggles `0x3E26` (sound on/off; also shifts HUD sprite index by 0x60A) |
| `T` | toggles `0x1210` (game speed) |
| `S` | SAVE GAME prompt (`FUN_22ab_23c2`) |
| `R` | RESTORE GAME prompt (`FUN_22ab_22b2`) |
| `P` | pause: inner loop that redraws and waits for a key |
| `/` + `R` | start macro **record** to `temp.mac` (linear `0x147F4`) |
| `/` + `C` | play `temp.mac` back (linear `0x1474D`) |
| `/` + `0` | stop recording and write the buffer to disk (linear `0x1483B`) |

Repeated-key cheat detection: the loop counts consecutive identical key codes in
`local_6`; the third hit fires the cheat.

The S/R actions live in the main loop's five-entry jump table, which is in the
code segment (not DGROUP): `FUN_22ab_1531` sets `piVar5 = (int *)0x19DF` and
compares with `CS:[bx]`. The keys are `1B, 51, 52, 53, BB` and the near handlers
are `1A66, 1A66, 1A20, 19F3, 1A5B` (`S` -> `FUN_22ab_23c2`, `R` ->
`FUN_22ab_22b2`). See `port-save-splash-cheats.md`.

---

## 4. Video

### 4.1 Backends **[verified]**

`DGROUP:0x3A4D` selects the driver. `FUN_1123_0a9b` configures each:

* `0` - 16-colour mode via BIOS `int 10h, AX=0x1000 | 0x04` (CGA/EGA path), port `0x3D9` bit 0x11.
* `2` - EGA: CRTC `0x1200/0x31`, plane masks via `0x3C4`/`0x3CE`, map size `0x2000`,
  `FUN_1123_04eb(1,9,0)`.
* `4` - **VGA**: mode `0x13` (320x200x256, base `A000:0000`), CRTC "byte mode"
  (`0x3D4/0x17 = 0xE3`), page stride `0x4000`, `0x3C4 = 0x0604`, `0x3CE = 0x4005`.
  Palette is programmed by `FUN_1123_0791` (DAC index at `0x3C8`, data at `0x3C9`)
  from the 768-byte buffer at DGROUP:`0x0194`; `FUN_1123_09b6` implements
  `_fadein`/`_fadeout` by scaling that buffer 64 -> 0.

The buffer at DGROUP:`0x0194` is **static data in the load module**: startup
does not parse a palette file, and no code writes to `0x0194` except the 64x12
shape case in 9.4. The initial contents are therefore the default game palette.
It is 6-bit RGB, identical in `jill1.exe`, `jill2.exe` and `jill3.exe`, and has
the shape `16 EGA colours, 8-step grey ramp, 6x6x6-ish colour cube` (no standard
VGA ramp). `re/gen_port_data.py` copies it verbatim to `port/src/palette.inc`.
**[verified]**

Page flipping (`_pageflip`, `FUN_1123_06c5`) waits for vertical retrace with
`in(0x3DA)` and reprograms the CRTC start address (`_wait_vbi`, `_setpages`).

### 4.2 Sprite blitter **[verified]**

`FUN_11f4_027c` is the generic masked sprite routine: it clips a rectangle against
the viewport (`param_1` points at `{x, y, w, h}` in DGROUP), handles negative clipping
by skipping rows and advancing the source pointer, and writes through the VGA
planar registers (`0x3C4` map mask, `0x3CE` bit mask) for the 16-colour path, or
linear bytes for mode 13h (`param_7 == 3` = VGA, and `(param_2 & 7) == 0` triggers
the byte-wide path where width is divided by 8). The mask table lives at
DGROUP:`0x3232`; `FUN_11f4_0e77` sets the viewport used by it.

Named helpers from the symbol table: `_drawshape`, `_ldrawsh_cga/_ega/_vga/_mcga`,
`_scroll`, `_scrollvp`, `_clrvp`, `_lcopypage`, `_plot_cga/_ega/_vga`.

### 4.3 Text

Character output is `FUN_1353_074e(window, x, y, font, string)`; the `font` argument is
`1` -> 8-pixel-wide glyphs, `2` -> 6-pixel-wide (6x7) glyphs. Colour/bit-mask setup for
the current video backend is `FUN_1353_0b64`. `FUN_1353_000d` creates windows and
`FUN_2cf8_0005` sets window geometry (called with `(1,1,0x50,0x19)` for the 80x25
error screen). HUD text uses these directly (`HEALTH`, `SCORE`, `LEVEL`).

---

## 5. Board (tile map)

* `128 x 64` cells, `u16` each, **column-major**: `cell = map[(x * 64) + y]`
  (the engine always indexes `[x*0x40 + y]`, so each column is 64 contiguous
  words). Confirmed both in the disassembly and by measuring horizontal
  continuity in the shipped maps. **[verified]**
* Cell = tile id in the low bits + two flag bits in `0xC000`.
  A fresh/unused area is filled with `0xC000` (blocking, tile 0). **[verified]**
* World size `2048 x 1024` px (16 px tiles).
* `FUN_22ab_1202` fills the map with `0xC000` and resets the camera.
* During the object update pass, every cell covered by a *blocking* object gets
  `0xC000` OR-ed in, so background collision tests see objects as walls. **[verified]**

Collision queries (`FUN_26f4_...`) use the low bits for tile identity and
`cell & 0xC000` for "solid".

Tile id -> graphics mapping and per-tile attributes come from `jill.dma` (#8), loaded
into the per-tile table described in #5.1.

### 5.1 Per-tile table (600 entries) **[verified]**

600 records of 8 bytes at DGROUP:`0x83EC`..`0x96AB`, immediately after the 0x46-byte
board header at `0x83A6`:

```
+0  u16  shape id            default 0x4700 (blank tile)
+2  u16  flags               default 0x4006
+4  u16  name pointer, offset default DGROUP:0x1688
+6  u16  name pointer, segment
```

`jill.dma` (#8) overwrites `+0`, replaces `+2` with `0x4006 ^ attr`, and stores a
malloc'd copy of the tile name behind a fresh far pointer in `+4`. Tiles with no
`.dma` record keep the defaults.

The low two bits of `+2` are the collision class; BRIDGER (kind 26) treats
`(& 3) == 3` as solid terrain **[verified]**. The other attribute bits seen in
`jill.dma` are `0x0001 0x0008 0x0010 0x0020 0x0200 0x6000` plus combinations; they are
tile-shape/behaviour hints whose exact consumers are not mapped. **[open]**

The board loader walks the map, marks which tilesets are needed, and loads only those
shape groups from `jill1.sha`. **[verified]**

---

## 6. Objects

### 6.1 Record layout - 31 bytes (`0x1F`) **[verified]**

| Off | Type | Meaning |
|--|--|--|
| `0x00` | u8 | kind (0-63) - index into the kind table |
| `0x01` | i16 | x (world pixels) |
| `0x03` | i16 | y |
| `0x05` | i16 | x delta / velocity |
| `0x07` | i16 | y delta / velocity |
| `0x09` | u16 | collision width (defaults from kind table) |
| `0x0B` | u16 | collision height (defaults from kind table) |
| `0x0D` | u8 | state/status byte (heavily used, 140 handlers) |
| `0x0E` | - | kind-specific (5 bytes, `0x0E`-`0x12`; `0x0F` and `0x11` are u16s used by ~45 handlers) |
| `0x13` | u16 | primary state/parameter (196 handlers); CHECKPT uses it as the link id |
| `0x15` | u16 | flags word; engine ORs `0xC000` into it while the object is "active/blocked" |
| `0x17` | u16 | attached string offset (runtime far pointer) |
| `0x19` | u16 | attached string segment |
| `0x1B` | u16 | kind-specific (22 handlers) |
| `0x1D` | u16 | countdown / time-to-live (decremented each update, 13 handlers) |

Object 0 in a shipped board is the PLAYER; the remaining objects follow in file order.
The board loader caps the file count at **192 objects**, but the runtime array has
room for `0x100` (256) records (`0x96AC..0xB5AC`) and `FUN_26f4_1892` (`_addobj`)
appends up to that count.  After every draw pass `FUN_26f4_2506` compacts the
array: each record whose kind became `3` (KILLME, set by `FUN_26f4_1863`) is
dropped, the surviving records move down and the count shrinks.  A port that
never compacts keeps corpses forever and eventually cannot spawn weapons,
particles or score popups. **[verified]**

### 6.2 Kind table (64 entries, built by `FUN_1644_0002`) **[verified]**

Nine parallel arrays at the DGROUP addresses listed in #1.3. The handler is stored as a
far pointer `[offset @ 0xB84E+4k, segment @ 0xB850+4k]` and called with

```
call far [DGROUP:0xB84E + kind*4]
arguments: (ptr to a "this is not a real string" dummy, obj_index, action, other_index)
action 1 = "you hit obj other_index", 2 = "update yourself"
```

`func` returns non-zero to mean "blocked/hit" and the engine then ORs `0xC000` into the
object's `0x15` word.

| kind | name | handler | w | h | flags | tileset | cnt |
|--|--|--|--|--|--|--|--|
| 0 | PLAYER | 1fae:01d8 | 16 | 32 | 0x0008 | 0x08 | 0 |
| 1 | APPLE | 1644:0e14 | 12 | 12 | 0x0000 | 0x09 | 12 |
| 2 | KNIFE | 1644:0fff | 10 | 10 | 0x4008 | 0x0d | 0 |
| 3 | KILLME | 1644:0e0b | 0 | 0 | 0x0000 | 0x00 | 0 |
| 4 | BIGANT | 1644:13cf | 24 | 16 | 0x0480 | 0x3b | 8 |
| 5 | FLY | 1644:1789 | 16 | 14 | 0x0080 | 0x3c | 3 |
| 6 | MACROTRIG | 1a41:4c75 | 0 | 0 | 0x0008 | 0x00 | 0 |
| 7 | DEMON | 1644:2974 | 32 | 32 | 0x0480 | 0x2b | 20 |
| 8 | BUNNY | 1644:36ee | 8 | 8 | 0x0480 | 0x3a | 2 |
| 9 | INCHWORM | 1644:1c33 | 16 | 8 | 0x0480 | 0x16 | 3 |
| 10 | ZAPPER | 1644:1d91 | 32 | 16 | 0x0000 | 0x1c | 0 |
| 11 | BOBSLUG | 1644:1e50 | 24 | 24 | 0x0400 | 0x34 | 5 |
| 12 | CHECKPT | 1644:1fe2 | 16 | 16 | 0x0040 | 0x00 | 0 |
| 13 | PAUL | 1644:22d0 | 24 | 32 | 0x0000 | 0x39 | 0 |
| 14 | KEY | 1644:2790 | 16 | 8 | 0x0000 | 0x0e | 0 |
| 15 | PAD | 1644:28a4 | 16 | 16 | 0x0000 | 0x00 | 0 |
| 16 | WISEMAN | 1644:234b | 16 | 24 | 0x0040 | 0x0b | 0 |
| 17 | FATSO | 1644:2d43 | 20 | 28 | 0x0480 | 0x2c | 12 |
| 18 | FIREBALL | 1644:3105 | 16 | 16 | 0x0000 | 0x1a | 0 |
| 19 | CLOUD | 1644:324e | 16 | 16 | 0x0000 | 0x0a | 0 |
| 20 | TEXT6 | 1644:3371 | 6 | 7 | 0x0040 | 0x00 | 0 |
| 21 | TEXT8 | 1644:35df | 8 | 8 | 0x0040 | 0x00 | 0 |
| 22 | FROG | 1644:36ee | 14 | 10 | 0x0480 | 0x3f | 15 |
| 23 | TINY | 1fae:1d15 | 4 | 10 | 0x0008 | 0x10 | 0 |
| 24 | DOOR | 1644:3a00 | 16 | 24 | 0x0100 | 0x00 | 0 |
| 25 | FALLDOOR | 1644:3da3 | 16 | 16 | 0x0100 | 0x0e | 0 |
| 26 | BRIDGER | 1644:2503 | 0 | 0 | 0x0100 | 0x00 | 0 |
| 27 | SCORE | 1644:3482 | 4 | 5 | 0x0000 | 0x00 | 0 |
| 28 | TOKEN | 1a41:000f | 16 | 16 | 0x0000 | 0x00 | 0 |
| 29 | ANT | 1644:15a8 | 32 | 16 | 0x0480 | 0x0a | 6 |
| 30 | PHOENIX | 1644:198a | 16 | 16 | 0x0480 | 0x0b | 4 |
| 31 | FIRE | 1a41:0160 | 16 | 32 | 0x0000 | 0x0c | 0 |
| 32 | SWITCH | 1a41:02ab | 16 | 16 | 0x0008 | 0x3c | 0 |
| 33 | GEM | 1a41:04b2 | 16 | 16 | 0x0000 | 0x09 | 23 |
| 34 | TXTMSG | 1a41:4c8f | 16 | 16 | 0x0008 | 0x0e | 0 |
| 35 | BOULDER | 1a41:05f6 | 16 | 16 | 0x0000 | 0x00 | 0 |
| 36 | EXPL1 | 1a41:0a3b | 16 | 32 | 0x0000 | 0x2e | 0 |
| 37 | EXPL2 | 1a41:0b91 | 16 | 32 | 0x0000 | 0x0e | 0 |
| 38 | STALAG | 1a41:0c54 | 16 | 16 | 0x0100 | 0x00 | 0 |
| 39 | SNAKE | 1a41:0dcb | 56 | 16 | 0x0400 | 0x0f | 35 |
| 40 | SEAROCK | 1a41:46d5 | 16 | 16 | 0x0000 | 0x0e | 0 |
| 41 | BOLL | 1a41:1261 | 14 | 14 | 0x0000 | 0x1f | 0 |
| 42 | MEGA | 1a41:163c | 20 | 24 | 0x0100 | 0x21 | 0 |
| 43 | BAT | 1a41:17ee | 26 | 32 | 0x0480 | 0x23 | 4 |
| 44 | KNIGHT | 1a41:1bb4 | 32 | 32 | 0x0100 | 0x24 | 0 |
| 45 | BEENEST | 1a41:1e13 | 16 | 16 | 0x0000 | 0x25 | 11 |
| 46 | BEESWARM | 1a41:2069 | 16 | 16 | 0x0000 | 0x25 | 0 |
| 47 | CRAB | 1a41:22f3 | 16 | 16 | 0x0480 | 0x26 | 2 |
| 48 | CROC | 1a41:2682 | 64 | 8 | 0x0480 | 0x27 | 3 |
| 49 | EPIC | 1a41:29fa | 32 | 16 | 0x0000 | 0x28 | 35 |
| 50 | SPINBLAD | 1a41:2b1a | 16 | 16 | 0x4008 | 0x2d | 0 |
| 51 | SKULL | 1a41:3029 | 22 | 26 | 0x0100 | 0x2f | 0 |
| 52 | BUTTON | 1a41:3261 | 16 | 16 | 0x0008 | 0x31 | 0 |
| 53 | PAC | 1a41:3402 | 16 | 16 | 0x0400 | 0x32 | 0 |
| 54 | JILLFISH | 1fae:202a | 24 | 16 | 0x0008 | 0x33 | 0 |
| 55 | JILLSPIDER | 1fae:262f | 16 | 16 | 0x0008 | 0x00 | 0 |
| 56 | JILLBIRD | 1fae:29801 | 16 | 16 | 0x0008 | 0x0b | 0 |
| 57 | JILLFROG | 1fae:2648 | 14 | 10 | 0x0008 | 0x3f | 0 |
| 58 | BUBBLE | 1a41:3834 | 8 | 8 | 0x0000 | 0x33 | 0 |
| 59 | JELLYFISH | 1a41:3957 | 16 | 24 | 0x0000 | 0x33 | 0 |
| 60 | BADFISH | 1a41:3d03 | 28 | 16 | 0x0000 | 0x33 | 7 |
| 61 | ELEV | 1a41:3fe8 | 16 | 16 | 0x0100 | 0x00 | 0 |
| 62 | FIREBULLET | 1a41:436d | 16 | 16 | 0x4008 | 0x1a | 0 |
| 63 | FISHBULLET | 1a41:45ed | 12 | 5 | 0x4008 | 0x33 | 0 |

1 nearest exported function is `FUN_1fae_298b` (the pointer arithmetic in the
initialiser resolves a few bytes off).

**Per-kind behaviour is documented separately in [`jill1-kind-handlers.md`](jill1-kind-handlers.md)**
(the 3-call protocol, the shared object API with addresses, and notes for all 64 kinds).
Three entries here are a few bytes early: MEGA's prologue is `1a41:165a`, CROC's is
`1a41:269f` and JILLBIRD's is `1fae:298b`. [verified]

Kind names are consecutive NUL-terminated strings starting at DGROUP:`0x0C76` ("PLAYER"),
in the order listed above.

**Flag bits** (word at DGROUP:`0xBE3C + kind*2`). Only six bits occur anywhere in the
64-entry table; every one is now identified: **[verified]**

| bit | meaning | tested at |
|---|---|---|
| `0x0008` | takes part in the per-frame hit scan: the object is added to the on-screen list and tested against every other listed object; on AABB overlap **both** sides receive action 1 | `FUN_26f4_12b9` 0x281F9, test 0x28448 |
| `0x0040` | carries an attached board string (`s17`/`s19`); the editor labels it "Text Inside" | 0x260EE, 0x266B5 |
| `0x0080` | killable by KNIFE (2) and FIREBULLET (62) | 0x1E9CE, 0x1778C |
| `0x0100` | receives triggers (`_sendtrig`) | 0x29050, 0x266B7 |
| `0x0400` | valid target/victim for SPINBLAD (50) | 0x1CFE9, 0x1D0E3 |
| `0x4000` | counts as an attack/projectile: enemies react to it (and it never triggers other projectiles) | 0x190E7, 0x1B5EF, 0x1C1F7, 0x1C2F0, 0x1E0CE, 0x1E3E4 |

`cnt` is the value pre-loaded into the record's `0x13`/`0x1D` fields at spawn (e.g. the
number of apples left in a cluster, snake length, animation frames).

### 6.3 Update pass (`FUN_26f4_12b9`) **[verified]**

See `jill1-player-physics.md` #2 for the per-state player consequences, the
handler ABI and the `0xC000` semantics.

```
visible_count = 1
active[0]     = 0                            ; object 0 = player, always first
for i in 1..numobjs-1:
    if obj.x + obj.w >= cam.x - 0x60 and obj.x <= cam.x + cam.w + 0x60 and
       obj.y + obj.h >= cam.y - 0x30 and obj.y <= cam.y + cam.h + 0x30:
        active[visible_count++] = i
        obj.flags &= ~0xC000                 ; clear "blocked"
    else:
        obj.flags |= 0xC000                  ; off-screen => blocked

if mode == 0:  (draw/mark pass)
    for each active obj: obj.flags |= 0xC000
else:          (update pass)
    for each active obj:
        if obj.ttl > 0: obj.ttl--
        if handler(obj, 2, 0) != 0: obj.flags |= 0xC000

    for each active obj with kindflags & 8:
        for each other active obj:
            if AABB overlap: handler(a, 1, b); handler(b, 1, a)
                             both get 0xC000

    for each active obj whose flags are set:
        OR 0xC000 into every map cell covering obj's AABB

    ; then a second sweep re-checks the map under each object and marks
    ; objects that are entirely inside blocking cells
```

Notes:
* Culling uses a `0x60`/`0x30` margin (6/3 tiles) so objects just off-screen keep animating.
* The active list is capped at `0xC0` (192) entries.
* Drawing (`FUN_26f4_0d52`) walks the active list sorted by y (`FUN_26f4_259b`
  prepares the order) so that lower objects overlap higher ones.

---

## 7. Board file format (`.jn1`, `.dem`, and save files) **[verified]**

Verified by fully parsing all 9 shipped boards - every file ends exactly at EOF with
this layout:

```
0000  u16 tile[128*64]        0x4000 bytes, column major: tile[x*64 + y]
4000  u16 objcount            (1..192; object 0 = PLAYER)
4002  object[objcount]        31 bytes each (see 6.1)
      board_header            0x46 bytes (70)
      strings                 1 per object whose (word@0x17 | word@0x19) != 0,
                              in object order, each: u16 len, len bytes, NUL
```

There is **no separate string table**: the strings belong to objects. Which objects
carry them:

* kind 20/21 (`TEXT6`/`TEXT8`) - screen text overlays, e.g. `" GO UP!"`,
  `"GONNA GET"`, `" HOW YA"`; the 50.jn1 ending is ~50 consecutive text objects.
* kind 12 (`CHECKPT`) - the interactive trigger strings: a board name (`"2.jn1"`),
  music (`"*stealth.ddt"`), macro (`"&end1.mac"`), or `"!"`.

String command prefixes **[verified by their use in the main loop]**:

| Prefix | Meaning |
|--|--|
| `' '` or plain text | display message (handled by the TEXT handler) |
| `*name.ddt` | load/play a music track (CMF, #11) |
| `&name.mac` | start recorded-input macro playback (+ optional music) |
| `#name` | conditional load (checked with `FUN_1581_0660` first) |
| `!` | end-of-level sequence: gem check, "YOU LEFT WITHOUT FINDING A GEM!" |
| bare filename | warp to that board, preserving score/health |

`!` handling (`FUN_22ab_1531`): find the CHECKPT whose `0x13` word equals the current
level number, read its state byte `0x0D`, and either restart the player at the
checkpoint's `0x05/0x07` fields or advance.

`FUN_22ab_0c9a` writes the same structure back (used by the editor and by save games);
`JN1SAVE.n` files are just board files.

### Board header (70 bytes at DGROUP:`0x83A6`) **[verified]**

Read/written as one 0x46-byte block by `FUN_22ab_0aa5` (load) and `FUN_22ab_0c9a`
(save); it *is* the live HUD state, so the fields are proven by the HUD/death code.

| Off | Addr | Type | Meaning |
|--|--|--|--|
| `0x00` | `0x83A6` | u16 | level / checkpoint id shown by the HUD; `0x7F` blanks it |
| `0x02` | `0x83A8` | u16 | health, in pips (new board = 6; apple pickup caps at 8; `FUN_26f4_05e7` subtracts damage and ends the game at 0) |
| `0x04` | `0x83AA` | u16 | key count, 0..15 |
| `0x06` | `0x83AC` | u16[16] | key ids (32 bytes). `FUN_26f4_1a13` appends, `FUN_26f4_1a38` removes the first match and shifts the rest down; both set bit 14/15 of `0xBE38` to mark the HUD dirty |
| `0x26` | `0x83CC` | u32 | score (`0x83CC` low word, `0x83CE` high) |
| `0x2A` | `0x83D0` | u16 | damage/HUD flash flag: `FUN_26f4_05e7` sets 1, `FUN_26f4_0d52` clears it after drawing the red flash |
| `0x2C` | `0x83D2` | u32 | score copy used by `FUN_26f4_04c7` when restoring state |
| `0x30` | `0x83D6` | 22 bytes | dead block: `FUN_26f4_0008` memsets it to 0 through `FUN_2ba4_0007(0x83D6, 0x16, ...)`; saved and restored verbatim but no code ever reads it (only occurrence of the address in the whole image is that memset), so it is reserved/editor scratch |

Confirmed against `JN1SAVE.0` (level `0x7F`, health 6, key count 0, key[0] = 3,
score `0x17`) and the three `*.dem` demos. For boards 1-4 and `map.jn1` the 22
trailing bytes hold repeated tile ids matching very common tiles in that same board;
`map.jn1` (the editor map) is the only file that fills them consistently.

---

## 8. `jill.dma` - tile definitions **[verified]**

```
record[] until EOF - 320 records, no file header:
       u16 tile_id      tile index in the 600-entry table (0 .. 534)
       u16 shape_id     first shape of the tile (0x5B00 for terrain, +3 each)
       u16 attr         attribute word, XORed into the tile-table flags
       u8  name_len
       char name[name_len]      no NUL terminator
```

Record 0 is the default entry (`id 0, shape 0x4700, attr 0x0201, name "0"`), so the
table slot 0 gets the same value it is initialised with. There is no separate header:
the first 8 bytes are exactly one such record.

The loader stores each record into the 600-entry tile table (#5.1) as
`+0 = shape_id`, `+2 = 0x4006 ^ attr`, and a malloc'd copy of `name` behind the far
pointer at `+4`. **[verified]**

Shape numbering is per tileset group: terrain `0x5B00` +3 per tile, bricks/wood
`0x7000` +1, sun `0x5100`, shade overlays `0x5200`/`0x5300`, stalk `0x6000`,
forest `0x5E00`, castle `0x5900`, water `0x5C00`, rock `0x5400`, caves `0x5700`,
numbers `0x6918`..`0x6921`, etc.

Names: `BASETREE`, `BASESHORE`, `BASECLIFF`, `BASEWATER`, `SMALLBRICKS`, `BIGBRICKS`,
`WOODPLACK`, `WOODTORCH`, `WOODPANEL`, `HINTPANEL`, `SHORE`, `SUN`, `SUNB`, `SUNT`,
`SUNTL`, `SUNTR`, `SUNBL`, `SUNBR`, `LASHADE0..7`, `BLSHADE0..9`, `STALK0..9`,
`MUSH1/2/T`, `CL0..CL20`, `T1..T20`, ... `WALL1..7`.

---

## 9. `jill1.sha` - shapes **[verified]**

```
0000  128 x u32  absolute file offsets (entries 0 and 64..127 are unused = 0)
0200  128 x u16  sizes
0300  data       sum(sizes) == filesize - 0x300  (holds exactly)
```

So `resource[i].data = file[offsets[i] .. +sizes[i]]`, loaded on demand. Two flag
arrays drive it: `DGROUP:0x2FAE[i] = 1` means "shape i needed" and
`0x302E[i]` caches the entry header flags (used as the blit mode).
`FUN_1069_000d` reads the directory, `FUN_1069_094d` (`_shm_do`) loads every flagged
entry into the 4 KB scratch from `malloc(0x1000)`, and `FUN_1069_0271` decodes one
entry. Symbol names in the overlay: `_uncrunch`, `_xlate_table`, `_latch_*`,
`_shm_rotate`, `_shm_tbllen`, `_shm_tbladdr`.

In `jill1.sha` only entries 1..63 are populated: 49 shapes are 4bpp, 10 are 3bpp,
3 are 2bpp and 1 is 8bpp. Entry 1 is the 8x8 text font (128 glyphs), entry 4 the
HUD/arrow glyphs, and most others are animation strips.

### 9.1 Shape entry format **[verified]**

All multi-byte fields little-endian. The parse below lands exactly on `sizes[i]`
for all 63 entries.

```
+0   u8   frame count N        runtime decodes exactly N frames
+1   u16  always 1             (63/63 entries)
+3   u16  decoded block size, 2bpp  (video modes 0/1)
+5   u16  decoded block size, 4bpp  (video modes 2/3)
+7   u16  decoded block size, 8bpp  (video mode 4)
+9   u8   source bits per pixel: 2, 3, 4 or 8
+10  u16  flags
```

`flags` takes only 0, 1 and 4 in `jill1.sha`:

| Bit | Meaning |
|--|--|
| 0 | colour table **absent** (`1` = identity index mapping, `0` = table follows). Also used as the blitter's transparency mode (`[0x302E + shape*2] & 1` in `FUN_1123_00eb`) |
| 2 | destination uses planar packing (4 bit-planes) instead of packed pixels |

Colour table, present only when `flags&1 == 0` and `bpp < 8`: `1<<bpp` entries of
**4 bytes** each. The low 16 bits are unused; the useful byte depends on the active
video mode:

| Mode | Shift | Mask |
|--|--|--|
| 0 (CGA 2bpp) | 0 | 3 |
| 1 (MCGA 2bpp) | 4 | 3 |
| 2 (EGA 4bpp) | 8 | 0x0F |
| 3 (EGA 4bpp) | 12 | 0x0F |
| 4 (VGA 8bpp) | 16 | 0xFF |

i.e. `palette_index = (entry >> shift) & mask`, which `_xlate_table` bakes into the
256-byte table at `DGROUP:0x2E5C` (`FUN_1069_004D`).

### 9.2 Frame records **[verified]**

```
u8 width, u8 height, u8 opcode, then payload
  opcode 0   width*height raw 8-bit pixel bytes, byte aligned
  opcode 1   width*height pixels of `bpp` bits, LSB first; the bit pointer is
             reset to 0 at the start of the payload
  opcode 2   empty frame (no payload)
  opcode >2  treated like 2 (no payload)
```

All 1107 records in the bank use opcode 0 in practice. The bit reader
(`FUN_1069_0233`) keeps a 16-bit window at the source pointer with the bit position
in `DGROUP:0x2FAC`; it only advances the pointer once the position reaches 8, so
payloads must end byte aligned (the encoder guarantees it, otherwise the next frame
header would misalign).

**Trailing record.** Every entry carries exactly one more record after the N decoded
frames (same encoding, almost always a stand-alone image such as 16x16 or 16x32).
The runtime ignores it: it allocates only `N*(4 + packed frame bytes)` bytes
(`malloc([bp-0x22])` in `FUN_1069_0271`), which is exactly the `+3/+5/+7` word for
the active mode. The extra record is presumably editor data (preview/icon).

### 9.3 Destination block **[verified]**

`FUN_1069_0271` builds a frame directory at the block start, 4 bytes per frame:

```
u16 offset   byte offset of the frame's pixels inside the block
u8  width_bytes = ceil(width * pxbits / 8)  (2bpp: (w+3)/4)
u8  height
```

Packing per video mode: 4 px/byte (2bpp), 2 px/byte (4bpp), 1 byte/px (8bpp); with
`flags&4` the 4bpp path writes bit planes instead (bit masks 8/4/2/1). Allocation size
words verified for all 63 entries:

* `+3` (2bpp) = `sum(4 + ceil(w/4)*h)` -> exact in 63/63
* `+7` (8bpp) = `sum(4 + pad4(w)*h)` where `pad4(w) = (w+3)/4*4` -> exact in 63/63
* `+5` (4bpp) = same with 2 px/byte -> matches in 47/63; the 16 exceptions store an
  unrelated (larger) value. Since only the word for the active mode is used this is
  harmless; treat `+5` as advisory. **[open]**

### 9.4 Palette special case **[verified]**

Before dispatching on the opcode, `FUN_1069_0271` checks
`width == 0x40 && height == 0x0C && video_mode == 4`. Such a record is not drawn:
its 768 payload bytes are copied to `DGROUP:0x0194` and `FUN_1123_0791` programs the
VGA palette. No 64x12 shape exists in any shipped `*.sha`, so this path is unused
data; the active palette is the default one already in the image at
`DGROUP:0x0194` (see 4.1).

`re/shadec.py` implements all of the above (self-test: 0 mismatches over all 63
entries) and can export any entry as a PNG contact sheet for visual checks.

## 10. `jill1.vcl` - sampled sounds and text pages **[verified]**

```
0000  50 x u32  offsets      first real data at 0x67B
00C8  50 x u16  sizes
012C  50 x u16  sample rates (0x1770 = 6000 Hz for every used entry)
0190  40 x u32  text-page offsets
0230  40 x u16  text-page sizes
```

Loaded by `FUN_1581_0009` into `DGROUP:0x3F10/0x3F12` (50 digital sounds),
`DGROUP:0x3FDA/0x3FDC` (song pointers), plus the tables at `0x41DA`, `0x422A`, `0x42F6`.
`FUN_1581_0178(sound_id, ...)` is "play sound N"; it dispatches either to the
PC-speaker path (`FUN_1530_00fe`) or to the Sound Blaster DSP
(`FUN_15ed_042f`/`FUN_15ed_038c`).

### 10.1 Text pages **[verified]**

The 40-entry table at `0x190`/`0x230` is **not** music: `FUN_1581_03ef(page)`
seeks to `offset[page]`, allocates `size[page]` bytes and reads the page into
the far-heap buffer `DGROUP:0x3F0A/0x3F0C`.  Page numbers are 0-based indexes;
the menu passes 1 for INSTRUCTIONS (Jill 1 page 1 = "How the Heck to Play").

A page is a `\r`-separated block: the first line is the title, then a blank line,
then the body.  A leading ASCII `'0'`-`'7'` on any line is its colour code
(`22AB:0dc6` skips control bytes, takes the digit, and defaults to 7);
`22AB:0e86` draws each line centred with the 6-pixel font (bank 2).

The viewer is `FUN_22ab_0f0a(page)`: it draws a bordered window, shows 6-px
lines, scrolls with up/down, and exits on Enter/Esc.

**How pages are triggered.**  Kind 1 APPLE doubles as a page pickup: when an
apple with `state > 0` is touched (by the player or anything else) the handler
plays sound `(6,0x19)`, calls `FUN_22ab_0f0a(state)`, sets `ttl = 4` and kills
the object.  A normal apple (`state == 0`) gives health instead.  The boards
place these page apples directly: Jill 1 `1.jn1` has state 3 and 5, its hub
`map.jn1` has the news bulletins 10-14 plus page 4, and Onesimus places the
Philemon verses at states 10-29 across its 16 boards.  This is the "News
bulletin" screen and the Onesimus bible verses.

`F1` in game shows page 1 (the "How the Heck to Play" page; the intro message is
`PRESS F1 FOR HELP!`).

**Port status:** `port/src/textpages.cpp` reads the 40-entry table into
`TextPages`; `GameDef::pages_file` (manifest key `pages`, defaulting to the
profile's `.vcl`) names the bank.  The port's viewer (`Engine::show_text_page`)
is a framed full-screen page with the title, coloured centred lines and
up/down scrolling; page apples open it and freeze the world.  F1 opens page 1.
`--text-page N` opens page N headlessly.  Gamepad D-pad/left stick scrolls,
left/right steps pages and A/B/Start/Back closes.

## 11. `*.ddt` - music (Creative Music File) **[verified]**

`funky.ddt` / `action.ddt` start with `"CTMF"` followed by the standard CMF header
(`version 0x0101`, instrument block at `0x28`, music data at `0xF8`, 0x60 ticks/beat...).
Loaded by `FUN_1581_069e` into a far-heap buffer (`DGROUP:0x0BF0`), then driven by the
`_sbfm_*` routines (SB FM, OPL2) - `MUSDATA.ASM`, `SBCDATA.ASM`, `CMFASM.ASM`,
`CMFDRV.ASM` appear in the symbol table.

## 12. Sound engine

Three independent paths, all fed by `FUN_1581_0178` / `FUN_1581_063e`:

1. **PC speaker** (segment `0x1530`): `FUN_1530_0440` installs a custom int 1Ch handler
   and programs PIT channel 0; `FUN_1530_03a0(channel=2, mode, divisor)` writes
   `0x43`/`0x42`; `FUN_1530_0512` masks the speaker gate via `in/out(0x61)`.
   This is the `_noisemaker` sound-effect engine.
2. **Sound Blaster DSP** (segment `0x15ED`): detection in `FUN_15ed_04da` writes reset
   `0xE1`-style commands to `base+6`/`base+0xC` across 6 base ports (`0x210..0x260`
   step 0x10) and 4 IRQ/DMA configurations (`DGROUP:0x0BD2`, `0x0BDE`); single-cycle
   DMA playback via `FUN_15ed_038c` (uses the 8237 at `0x0A`-`0x0F`).
3. **SB FM music**: the CMF driver (`_sbfm_*`) plays `.ddt` files.

`DGROUP:0x3E26` = master sound enable (toggled with `N`); `DGROUP:0x0BF9`
disables music when the DSP is missing.

## 13. Input

* Custom **int 9 handler** installed by `FUN_1415_0a94`/`_installhandler`
  (`_oldint9`, `_keybuf`, `_k_pressed`, `_k_read`, `_k_status`, `_getkey`).
  It maintains `DGROUP:0x3C86` (last ASCII key), `0x3C76`/`0x3C78` (x/y movement
  direction, -1 / 0 / +1), `0x3C7A`/`0x3C7E` (action buttons, from bytes `0x3C65`
  and `0x3A63`), and a ring buffer.
* **Joystick**: `FUN_1415_0004`/`001a`/`0030` read `in(0x201)`; calibration
  (`_calibratejoy`, `_caldir`, `_joypresent`) is done from the setup menu with the
  "UPPER LEFT / LOWER RIGHT corner" prompts.
* **Macros**: see #13.1. Cheat keys in the `FUN_22ab_...` handler: `R` starts
  recording to `temp.mac`, `C` plays `temp.mac` back, `0` stops recording and
  flushes the buffer.

### 13.1 `*.mac` - recorded input macros / demos **[verified]**

Shipped files: `jn1demo.mac` (2091 B), `end1.mac`, `end2.mac` (120 B),
`jn2dem1-3.mac`, `jn3dem1-3.mac`. All nine parse cleanly with the layout below.
There is **no file header** - byte 0 is already the first record.

```
rec[0]    : u8 mask, then one byte per set bit of mask
rec[n>0]  : time, u8 mask, then one byte per set bit of mask
```

`time` is an **absolute** timestamp in ticks of DGROUP:`0xBE3A`, counted from the
start of playback (rec[0] is implicitly 0). It is a base-128 varint:

```
v < 0x80 : one byte v
else     : ((v & 0x7F) | 0x80), (v >> 7)
```

Mask bits, in the order their values appear in the stream:

| bit | DGROUP | contents |
|---|---|---|
| 0 | `0x3C76` | horizontal direction word (-1 / 0 / +1) |
| 1 | `0x3C78` | vertical direction word (-1 / 0 / +1) |
| 2 | `0x3C7A` | action button 1 (copied from byte `0x3C65`) |
| 3 | `0x3C7E` | action button 2 (copied from byte `0x3A63`) |
| 4 | `0x3C86` | last ASCII key code (special keys, menus) |

Only bits 0-4 ever occur in the shipped files, which is a good parser self-check.
A record is only emitted when the change mask is non-zero, so every record changes at
least one input. `jn1demo.mac` is 519 records and ends at tick 4295 (~4 min at
18.2 Hz); `jn2dem1.mac` 102 records / 984 ticks.

**Playback**: linear `0x1474D` (symbol map: `_recordmac`, `0415:05fd`) opens the file,
gets its length, `malloc`s that many bytes, reads the whole file, sets playback flag
`0x3DC8 = 1` and clears `0xBE3A`. The input poller (`FUN_1415_0a8b`, linear
`0x14A8B`) then waits until `0xBE3A - [0x3C72] >= next_time` and applies the values.

**Recording**: linear `0x147F4` (symbol map: `_playmac`, `0415:06a4`) `malloc`s an
8000-byte buffer, sets record flag `0x3DC2 = 1`, opens the target file and clears
`0xBE3A`. Each input poll (linear `0x14881`) emits a record when the mask is non-zero; the buffer
is written out by linear `0x1483B`, which is called when the write index reaches
`0x7530` and by the `/`+`0` key. Linear `0x14702` is the reset/teardown used at the
start of both operations: it clears `0x3DC8`/`0x3DC2` and frees the buffer.

The names in the exported symbol list are the opposite of what the code does, so use
the linear addresses above rather than the symbol names.

## 14. Menus, HUD, save/restore

* **HUD** (`FUN_22ab_05dd`, linear `0x23A05`): **[verified]** the panel is
  cleared first (`FUN_1353_0b4a`), then:
  * `HEALTH` at `(2, 2)`, health pips `0x0E2A` for `i < health-1` at
    `(i*3 + 0x2A, 2)` and the end cap `0x0E2B` at `((health-1)*3 + 0x28, 2)`;
  * `SCORE` at `(0x21, 10)`, the score digits right-aligned at
    `x = 0x40 - (digits*6 + 1)`, `y = 0x10`;
  * `LEVEL` at `(1, 10)`, the level value at `(1, 0x10)` (blank when `0x7F`);
  * inventory icons at `((i/3)*0x0E + 1, (i%3)*0x0E + 0x1A)` with the shape
    `0x0E00 + item_icons[item_id]`; the 16-word table is at DGROUP:`0x0ED4`
    (Jill 2 `0x0F70`, Jill 3 `0x1080`);
  * text uses shape entry 2 (6x7, ASCII glyph index, 6 px advance); `0xFFF9`
    etc. passed to `FUN_1353_0b64` are negated colour indices (7, 5, 4, 2) and
    the third argument (8) is the panel fill colour; the damage flash switches
    the fill to 4.
  * The port's CLASSIC style draws the original screen presentation from
    shape entry 3: frame 0x0A is the noise fill, 0x00/0x03 the vertical
    edges, 0x06/0x08 the horizontal edges and 0x04 the corners (a 9-slice).
    The game title sits in a top strip, and a 64-px status column carries a
    health bar, LEVEL/SCORE, a 3-column inventory and the INVENTORY caption.
    The board renders into the framed viewport (x 74..312, y 20..186) and the
    camera clamps to that viewport, so Jill stays visible at the board edges
    while CLASSIC is active.  MODERN remains a separate top strip and keeps
    the full 320x200 camera.  The layout is reconstructed from the original
    menu/HUD screenshots **[inferred]**; the field positions follow the
    documented draw calls above.  A pack `hud/classic.<ext>` still replaces
    the panel using the legacy 64x84 layout.
  * `PRESS F1 FOR HELP` is the one-shot intro message from `FUN_22ab_05b4`, not
    part of the per-frame HUD.
* **Menus** (`FUN_22ab_2aed`): `PICK A CHOICE  PLAY / RESTORE / ...`; the high-score
  table is `FUN_22ab_1c54` (`HI SCORES`), with a `PRESS ... TO ABORT` prompt
  (`FUN_22ab_1f69`).
* **Save/restore** (`FUN_22ab_23c2`): prompts for a name, echoes it to the screen,
  and writes `JN1SAVE.n` = the current board serialised with `FUN_22ab_0c9a`
  (tile map + objects + header + strings). Restore = `FUN_22ab_0aa5` on that file.
* **`jill1.cfg`**: see #14.1.

### 14.1 `jill1.cfg` **[verified]**

`_loadcfg` (`22ab:095b`, linear `0x2340B`) opens the file and reads four blocks in
this order; `_savecfg` (`22ab:0a39`, linear `0x234E9`) writes the same four blocks
back. 254 bytes total:

| offset | size | target | contents |
|---|---|---|---|
| `0x000` | 120 | DGROUP:`0xB5EA` | 10 x 10-byte high-score names, NUL padded |
| `0x078` | 40 | DGROUP:`0xB7CA` | 10 x u32 high scores (low word `0xB7CA+4i`, high word `0xB7CC+4i`) |
| `0x0A0` | 72 | DGROUP:`0xBAB4` | 6 x 12-byte restore-slot labels |
| `0x0E8` | 22 | DGROUP:`0x3DCE` | option block, 11 words |

Loading rules (from the disassembly, not the decompiler output):

* `fopen` failure **or** file length 0: the first byte of each of the 10 names, all
  10 scores, and the first byte of each of the 6 slot labels are zeroed, then
  `0x3DCE = 1` (sound on) and the file is closed.
* Otherwise the three big blocks are read verbatim and then 22 bytes are read into
  `0x3DCE`. If that last read returns a negative status, `0x3DCE` is set to 1;
  otherwise the bytes from the file are kept as-is.

The distributed file is 254 bytes, i.e. the option block is **truncated to 6 bytes**
(`0xE8..0xED`), so only words `0x3DCE..0x3DD2` come from disk - the shipped values are
`4, 1, 1` (the rest of the block stays zero). The high-score area contains filler:
eight scores of `100`, then `23` and `74`; name slot 8 is the single character `]`
and slot 9 is empty.

Score comparison treats the player's score as the 32-bit pair
`0x83CE:0x83CC` and each table entry as `0xB7CC[i*2]:0xB7CA[i*2]`
(`FUN_22ab_1c54`). The eight tags `"Look" "out" "world" "here" "comes" "epic"
"mega" "games"` are simply the first eight name slots and have no special role.

## 15. Built-in level editor **[verified in outline]**

The overlay symbol list contains `_design`, `_objdesign`, `_printobjinfo`, `_drawboard`,
`_drawcell`, `_saveboard`, `_loadboard`. The editor is reachable from the main menu and
reuses the same board format; `_saveboard` = `FUN_22ab_0c9a`. `FUN_25eb_0058` prints the
inspector fields (`kind`, `stat`, ...) using the same record offsets as #6.1, which
confirms the object layout independently of the runtime.

## 16. Open questions

1. **Object record bytes `0x0E`-`0x12` and `0x1B`** - kind-specific; each handler
   interprets them, so a port can copy them verbatim.
2. **Tile attribute bits** (`attr` in `jill.dma`, copied to table `+2` as `0x4006 ^
   attr`): only the low two (collision class) and the BRIDGER test are identified.
   Which bits mean foreground / shading / water animation is unmapped.
3. **`jill1.cfg` option block**: the 22-byte block at `0x3DCE` loads/saves correctly
   but the meaning of its 11 words is unknown; `0x3DCE` itself doubles as a
   sound/overlay flag elsewhere in the code.
4. **Shape `+5` (4bpp) size word** deviates from the frame math in 16/63 entries
   (#9.3), and the purpose of the trailing shape record is unconfirmed (it is never
   decoded by the runtime).
5. **Exact tick rate**: code divides deltas by 18 (BIOS 18.2 Hz) but the speaker driver
   reprograms PIT channel 0; confirm the divisor on real hardware.

## 17. Porting notes

* The whole game is a 320x200x256 framebuffer with 16x16 tiles and 2048x1024 boards;
  a modern port can keep the logic 1:1 (object update is already frame-based, driven by
  a tick counter rather than by elapsed time).
* Object logic is fully table-driven: the 64 kind handlers are the only code that needs
  porting per object; everything else (spawn, cull, collide, draw order, HUD) is generic.
* Collision is AABB against other objects plus a tile-cell test, with objects OR-ing
  their footprint into the map - reproduce that exactly or the level geometry changes.
* Level data needs no conversion: `.jn1` can be read directly (documented layout above).
* Tiles need `jill.dma` (id -> shape + attributes) and `jill1.sha` (shape data); sounds
  need `jill1.vcl`; music is standard CMF and can be fed to libADLMIDI or similar.
* The `W`+digit cheat, `X`x3, `Z`x3, `T`, `P`, `N` and the `/` macro keys should be
  kept as debug bindings in the port.
* For verification runs, capture the original in DOSBox and compare frames; the port
  must match `jill1.exe` object-for-object at the same tick schedule.

---

### Appendix: how these notes were produced

```
# dump disassembly
rizin -a x86 -b 16 -m 0x10000 -q -e scr.color=0 -c "s 0x22ab0+0xaa5; pd 150" re/img.bin

# re-run the Ghidra export
analyzeHeadless re/proj Jill1 -process jill1.exe -noanalysis \
  -scriptPath re/scripts -postScript ExportAll.java

# regenerate this document's kind table
python re/kindtable.py
# re-verify every board file parses exactly to EOF
python re/check_jn1.py
# re-verify the shape decoder against every .sha entry, then export entry 5
python re/shadec.py 5
```
