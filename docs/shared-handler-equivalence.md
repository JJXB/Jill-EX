# Shared object-kind handlers across Jill 1, 2 and 3

All three episodes ship the same 64 base object kinds. Jill 2 and Jill 3 add five
more (64-68, see `jill23-new-kinds.md`). This note answers a narrower question:
**are the 64 shared handler bodies the same code, or were they rewritten?**

Short answer: Jill 1 and Jill 3 share one source of the handler module with three
small gameplay edits; Jill 2 is a separate rebuild of the same source with
code-generation differences but no behavioural differences found. A port can
implement one handler table and apply a per-episode option for the three Jill 3
edits.

## 1. Method

The kind table is in BSS, so its handler pointers cannot be read from the image.
They are parsed from the kind-table initializer (the function that writes each
entry with `c7 06 <dgroup_addr> <value>`). Initializer file offsets:

| episode | image | initializer | kind-table seg array | kind-table off array |
|--|--|--|--|--|
| Jill 1 | `re/img.bin` | `0x6442` | DGROUP:`0xB850` | DGROUP:`0xB84E` |
| Jill 2 | `re/img2.bin` | `0x6717` | DGROUP:`0xB58A` | DGROUP:`0xB588` |
| Jill 3 | `re/img3.bin` | `0x6500` | DGROUP:`0xB2EC` | DGROUP:`0xB2EA` |

Each entry is a far pointer. Its stored segment gets `+0x1000` to become the
runtime segment; the file offset is `(seg-0x1000)*16 + off`.

For every kind the handler body is then recursively disassembled from its entry
point: intra-module jumps are followed, `call`/`lcall` are not, and decoding stops
at `retf`. Bodies are sorted by address so they can be compared as a sequence.

Two tolerances are measured, because a recompile changes operands but not intent:

* **mnemonic stream** - the ordered list of mnemonics only; absolute offsets,
  register allocation and segment numbers are discarded. A ratio of 1.0 means
  every instruction matches.
* **small-constant multiset** - every immediate `< 0x100` that is not a branch or
  call target, counted. This catches score values, sound ids and state
  thresholds independent of where they load from.

Tools: `re/hcmp.py` (table parser, recursive sweep, per-kind comparison) and
`re/hsig.py` (constant / call-target / mnemonic signatures). Run with `py -3`,
which has Capstone 5.0.7 installed.

## 2. Results

| pair | mnemonic stream identical | ratio >= 0.99 | same constant multiset |
|--|--|--|--|
| Jill 1 vs Jill 2 | 3 / 64 | 36 / 64 | 43 / 64 |
| Jill 2 vs Jill 3 | 3 / 64 | 33 / 64 | 41 / 64 |
| Jill 1 vs Jill 3 | 62 / 64 | 62 / 64 | 61 / 64 |

The Jill 1 - Jill 3 row is the interesting one: 62 of 64 handlers are an exact
mnemonic-for-mnemonic match, and only three kinds differ in any constant. The
Jill 1 - Jill 2 and Jill 2 - Jill 3 rows are dominated by compilation noise, so a
low number is **not** evidence of a gameplay change.

Only three kinds survive both tests as real deltas, all in Jill 3:

| kind | name | delta | confidence |
|--|--|--|--|
| 7 | DEMON | new `obj[0x1B]` guards around a spawn/trigger block | verified instructions, intent inferred |
| 28 | TOKEN | 134 -> 215 instructions; new ROMAN/DEMON and `obj[0x13] == 6` paths | verified |
| 49 | EPIC | `_addscore` id `2` -> `0x19` (25) | verified |

Everything else can use the Jill 1 behaviour in `jill1-kind-handlers.md` for all
three episodes, provided the per-episode DGROUP and segment bases are used.

## 3. Why Jill 2 looks so different

Jill 2 was rebuilt with a different compiler state: the same C bodies produce
different but equivalent code. Recurring patterns:

* redundant `jmp` after a short block (`xor ax, ax; jmp L; L: pop bp`) is dropped;
* `or ax, ax / je / cmp ax, 1 / je` is folded to `cmp ax, 1 / je`;
* branch polarity and block order are inverted (`je` + fall-through vs `jne` +
  jump), which changes the mnemonic sequence without changing the CFG;
* a duplicated block is merged, e.g. BUTTON (52) has two identical
  `_sound(3, 0x2C)` call sites in Jill 1/Jill 3 but one in Jill 2;
* stack-frame sizes differ by a byte or two, so `sub sp, N` constants shift;
* helper call offsets move with their module, and segment numbers move with the
  load address.

Example, kind 3 (a handler that always returns 0):

```
Jill 1                          Jill 2
push bp                         push bp
mov  bp, sp                     mov  bp, sp
xor  ax, ax                     xor  ax, ax
jmp  0x7252                     pop  bp
pop  bp                         retf
retf
```

Example, kind 6 (returns 1 only for action 1):

```
Jill 1                                      Jill 2
mov  ax, [bp+8]                             mov  ax, [bp+8]
or   ax, ax                                 cmp  ax, 1
je   L1                                     je   L2
cmp  ax, 1                                  jmp  L3
je   L2                                     L2: mov ax, 1
jmp  L3                                     L3: pop bp
L1: jmp L3                                       retf
L2: mov ax, 1
L3: pop bp
    retf
```

Neither rewrite changes a return value. This is why the port should not be
written by translating instruction streams; use the documented behaviour.

## 4. The three real Jill 3 deltas

### 4.1 DEMON (kind 7)

Jill 3 adds two guards on `obj[0x1B]` (`obj` = `DGROUP:0x9148 + i*31`). At the
first one (`2000:8FE7` runtime, file `0x8FE7`):

```
cmp  word ptr es:[bx+0x1B], 0
je   continue
jmp  skip
continue:
  ... _173B:2634(0, si, &obj[5], &obj[7], 4) ...
skip:
```

The second occurrence is later in the same handler and guards a second block.
In Jill 1/Jill 2 those blocks run unconditionally. Behaviourally the demon now
does something only once per object, or suppresses a spawn after a linked event.
The exact meaning of field `0x1B` is still open (it is also set by TOKEN, below).

### 4.2 TOKEN (kind 28)

Jill 3's handler is 215 instructions against 134 in Jill 1/Jill 2. Two additions:

* **action 1 (hit by another object)**: if the other object's kind is `0x44`
  (ROMAN, 68) or `7` (DEMON), kill this token with `173B:185D(si)` and set
  `other->0x1B = 1`.
* **action 2 (kill/collect)**: if `obj[0x13] == 6`, call `173B:1A83(6)` first.
  If that returns non-zero the normal kill path is skipped. The helper inspects a
  state byte, selects a value 5 (state 1) or 3, and calls `173B:20E9` with
  `(si, value, obj[0x13])` before returning 1.

So in Jill 3 a token can be consumed by a demon or by ROMAN, and the
`obj[0x13] == 6` link selects an alternate award path. This is the puzzle hook
that pairs with the DEMON change; neither exists in Jill 2.

### 4.3 EPIC (kind 49)

One immediate changes in the `_addscore` call:

```
Jill 1 / Jill 2:  mov ax, 2       ; lcall _addscore
Jill 3:           mov ax, 0x19    ; 25
```

The epic pickup therefore awards a different score id in Jill 3. `_addscore`
itself is `173B:1FAC` in Jill 3 (`16F4:1FB2` in Jill 1); the constant tables
behind the id are per-episode data.

## 5. Port guidance

* Implement the 64 base handlers once, with `obj` base, current module segment
  and sound/music entry points supplied per episode.
* Read behaviour from `jill1-kind-handlers.md`; it is valid for Jill 2 and for
  61 of 64 Jill 3 kinds without change.
* Gate only these on the episode:
  * Jill 3 DEMON: one-shot guard on `obj[0x1B]`;
  * Jill 3 TOKEN: ROMAN/DEMON consumption and the `obj[0x13] == 6` award path;
  * Jill 3 EPIC: score id 25 instead of 2.
* Do not copy numeric constants out of Jill 2 disassembly. Its `sub sp, N`,
  `mov dx, 0x1F`, DGROUP addresses and helper offsets differ from Jill 1 but the
  gameplay values (scores, sounds, thresholds) are the same.
* `obj[0x1B]` remains an open semantic; treat it as an opaque per-object flag and
  keep the TOKEN/DEMON interaction intact.

## 6. Regenerating the comparison

```
py -3 re/hcmp.py
py -3 re/hsig.py
```

`hcmp.py` prints per-pair identical counts, per-kind first difference and total
instruction counts. `hsig.py` prints constant and call-target differences; its
call lists are noisy for Jill 2 because helper module segments move, so use the
constant list and `hcmp.py` ratios for decisions.

## 7. Open questions

* Meaning of `obj[0x1B]` and of the `173B:2634` spawn call in DEMON. **[open]**
* Exact score values behind the TOKEN `173B:1A83` path (5 vs 3). **[open]**
* Helper modules were not compared beyond call targets. A helper-by-helper
  comparison could still reveal an episode-specific edit that never shows up in a
  handler body. **[open]**
