# Sound triggers per episode

All object handlers play samples through one helper. This note documents the
helper's ABI, the logical-id remap table, every call site in the 64 shared kinds,
and the per-episode differences. The `jill1.vcl` container format itself is in
`jill1-engine.md` section 10.

## 1. `_sound(chan, id)`

```
runtime far address          stored far address
Jill 1  1581:0178            0581:0178
Jill 2  15EB:01DA            05EB:01DA
Jill 3  1576:01E8            0576:01E8
```

The stored segment gets `+0x1000` at load time. Call sites push the arguments
right-to-left, so `push id; push chan; lcall _sound` is `_sound(chan, id)`.
**[verified from the prologue]**

Prologue (Jill 1, file `0x5988`):

```
mov  si, [bp+8]        ; id
mov  di, [bp+6]        ; chan / priority
cmp  word [0x3F0E], 0  ; sound system present?
jz   return
cmp  byte [si+0x1212], 0   ; remap table
jz   no_remap
mov  al, [si+0x1212]
cbw
mov  si, ax            ; id = remap[id]
no_remap:
cmp  word [0x0BF9], 0
jz   play
cmp  di, [0x3E40]      ; currently playing priority
jl   drop              ; lower priority than what is playing -> ignore
play:
...
```

So `chan` is a priority (1 = background ambience, 6 = player voice), and a new
sound is dropped while a strictly higher-priority one is active. The helper also
supports the PC-speaker path and the Sound Blaster DSP path; the caller only sees
`_sound`.

## 2. Logical id -> sample index

Every call site passes a **logical** id. The helper replaces it with
`remap[id]` when `remap[id] != 0`, otherwise it uses `id` as the sample index.
The 50-byte table is identical in all three episodes:

| episode | DGROUP offset |
|--|--|
| Jill 1 | `0x1212` |
| Jill 2 | `0x12AE` |
| Jill 3 | `0x13AC` |

Non-zero entries (logical -> actual):

```
07->17   09->1C   0D->18   0E->1C   14->30   1A->05   1B->30
1D->17   1E->18   1F->12   20->10   22->03   24->0C   26->08
28->29   2A->20   2B->08   2C->18   2D->0A   2F->23   31->30
```

ids `0x00`-`0x06`, `0x08`, `0x0A`-`0x0C`, `0x0F`-`0x13`, `0x15`-`0x19`,
`0x1C`, `0x21`, `0x23`, `0x25`, `0x27`, `0x29`, `0x2E`, `0x30` map to
themselves. This folding is why the 45 Jill 1 call sites resolve to only 19
distinct sample indices (17 audible plus the two silent ones): several logical
effects reuse one recorded sample.

## 3. Per-kind call sites

`(chan, logical)` and, when remapped, `-> actual`. `*` marks a logical id whose
actual sample is empty (silent) in every episode.

| kind | name | sound sites |
|--|--|--|
| 0 | PLAYER | (2,22->03), (2,03), (2,08), (2,08) |
| 1 | APPLE | (6,19), (2,0B) |
| 2 | KNIFE | (2,07->17), (3,0A) |
| 5 | FLY | (1,0F) |
| 8 | BUNNY | (1,12) |
| 12 | CHECKPT | (4,05) |
| 14 | KEY | (3,06) |
| 17 | FATSO | (1,11*) |
| 22 | FROG | (1,12) |
| 24 | DOOR | (3,24->0C), (3,0C) |
| 25 | FALLDOOR | (3,0E->1C), (3,0D->18) |
| 28 | TOKEN | (3,19) |
| 30 | PHOENIX | (1,0F) |
| 32 | SWITCH | (2,17), (2,18) |
| 33 | GEM | (3,10) |
| 34 | TXTMSG | (3,14->30) |
| 35 | BOULDER | (2,2D->0A) |
| 39 | SNAKE | (3,1F->12) |
| 41 | BOLL | (1,09->1C) |
| 42 | MEGA | (2,1C) |
| 45 | BEENEST | (3,20->10) |
| 49 | EPIC | (3,30), (2,20->10) |
| 50 | SPINBLAD | (3,0A), (1,23) |
| 51 | SKULL | (4,21) |
| 52 | BUTTON | (3,2C->18) - twice in Jill 1/3, once in Jill 2 |
| 54 | JILLFISH | (1,25*), (1,26->08) |
| 56 | JILLBIRD | (1,0F), (1,22->03) |
| 57 | JILLFROG | (1,2B->08), (1,12) |
| 61 | ELEV | (2,1D->17), (2,1E->18) |
| 62 | FIREBULLET | (3,0A) |
| 66 | FLAG | (2,20->10) - Jill 2/3 only |
| 68 | ROMAN | (1,11*) - Jill 2/3 only |

Site counts: 45 in Jill 1, 46 in Jill 2, 47 in Jill 3. **[verified by scanning
each handler body for `lcall` to the episode's `_sound` offset]**

Kinds not listed play no sound directly. Explosion kinds (36/37) and projectile
kinds (56/57/62/63) are handled by their parent or by helper code, not by a
`_sound` call inside the kind.

## 4. Per-episode differences

| item | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| `_sound` site count | 45 | 46 | 47 |
| FLAG (66) | kind absent | `_sound(2, 0x20)` | `_sound(2, 0x20)` |
| ROMAN (68) | kind absent | `_sound(1, 0x11)` | `_sound(1, 0x11)` |
| BUTTON (52) | two `(3,0x2C)` sites | one site (branches merged) | two sites |

The FLAG and ROMAN sounds are not gameplay edits to shared kinds; they are the
new kinds' own hit sounds (`jill23-new-kinds.md`). The BUTTON difference is a
code-generation merge and is inaudible: in Jill 1/Jill 3 both branches of the
button press play the same sample, so collapsing them in Jill 2 changes nothing.
**[verified]**

No shared kind gained, lost or changed a `(chan, id)` pair in any episode.

## 5. VCL sample bank

The `.vcl` starts with a 50-entry sampled bank (u32 offsets, u16 sizes, u16
rates) followed by a 40-entry text-page table (u32 offsets, u16 sizes).  All
three files use the same header; only the payload differs.

| bank | Jill 1 | Jill 2 | Jill 3 |
|--|--|--|--|
| file size | 94,954 | 85,515 | 75,528 |
| non-empty SFX entries | 23 | 24 | 23 |
| non-empty text-page entries | 14 | 1 | 3 |
| sample rate | 6000 Hz | 6000 Hz (one at 6024) | 6000 Hz |

Non-empty SFX ids:

```
Jill 1: 01 02 03 04 05 06 08 0A 0B 0C 0F 10 12 13 17 18 19 1C 21 23 27 29 30
Jill 2: same plus 1B
Jill 3: same as Jill 1
```

The two silent logical ids are explained by this list: `0x11` and `0x25` are not
present in any of the three banks. They are used by FATSO (17), ROMAN (68) and
JILLFISH (54). Those calls are harmless and should be treated as no-ops in a
port rather than errors. **[verified]**

Sample `0x30` (used by EPIC's `(3,0x30)`) is 11,518 bytes in Jill 1 and Jill 3
but 3,617 bytes in Jill 2, so the epic fanfare is a different recording in Jill 2.
Sample `0x29` is 7,551 bytes in Jill 1/2 and 3,340 in Jill 3. **[verified]**

## 6. Music path

Level and title music is separate from `_sound`: CHECKPT strings beginning with
`*` load a CMF `.ddt` and hand it to the `_sbfm_*` / OPL2 driver
(`jill1-engine.md` section 11). Title tracks are `funky.ddt` (Jill 1),
`peaks.ddt` (Jill 2) and `wild.ddt` (Jill 3).  The 40-entry table is not sound
at all: it is the text-page table (`jill1-engine.md` #10.1).  Jill 1 ships 14
pages (instructions, hints, five news bulletins, mega hint); Jill 2 has only
page 1 and Jill 3 pages 1-3. **[verified]**

## 7. Port guidance

* Implement `_sound(priority, logical_id)` once. Resolve `actual = remap[id] ?
  remap[id] : id`, then index the loaded sample bank.
* Keep the remap table as data; it is identical across episodes but belongs in
  the episode profile alongside the bank.
* Implement priority masking: drop a call when a strictly higher priority sound
  is already playing.
* Treat samples with zero size as silent. Do not log them as missing assets;
  `0x11` and `0x25` are intentionally absent.
* Drive all shared-kind sounds from the table in section 3; only FLAG/ROMAN add
  sites, and only BUTTON differs in site count (no audible change).
* Music is not routed through `_sound`; keep the `.ddt`/OPL path separate.

## 8. Open questions

* Whether the PC-speaker fallback uses the same remap table (the helper remaps
  before it branches, so it should). **[inferred]**
* Why sample `0x29` was shortened in Jill 3 and `0x30` in Jill 2; no handler
  depends on the length. **[open]**
* The second VCL bank's exact role in Jill 1 versus the `.ddt` path. **[open]**
