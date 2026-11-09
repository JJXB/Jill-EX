# Brief: object kind handler notes

Analyse Jill-of-the-Jungle object kind handlers and write terse notes.

Read `C:\Users\jjxb_\GIT\Jill-EX\docs\jill1-kind-handlers.md` FIRST: it has the
handler call protocol and a table of shared helper routines (address -> meaning).
Use it as the dictionary.

For each kind you are given, open its decompiled function at
`re\export\decomp\<seg>_<off>_FUN_<seg>_<off>.c`. Handy tool:

    python re\show.py FUN_<seg>_<off>

which prints a condensed listing (object fields appear as `O(i).x`, `O(i).state`,
`O(i).p13`, ... and DGROUP globals lose the `DAT_2cff_` prefix).

Object record: base DGROUP:0x96AC, stride 31 bytes.

    +0x01 x   +0x03 y   +0x05 dx   +0x07 dy   +0x09 w   +0x0B h
    +0x0D state   +0x0F p0f   +0x11 p11   +0x13 p13   +0x15 flags
    +0x17/+0x19 string ptr   +0x1B p1b   +0x1D ttl

Handler params: `(dummy, obj, action, other)`; action 0 = draw, 1 = hit,
2 = update. Action 1 is called with `other = 0` when the player touched it.

## Output format

One block per kind, exactly:

    ### 13. PAUL (1644:22d0) [verified]
    - draw: ...
    - hit: ...
    - update: ...
    - notes: ...

Rules:

* Only state what the code shows. If a call is not in the API table, write its
  address, e.g. "calls 26F4:263a".
* 1-4 bullets per kind; be concrete (numbers, sound ids, tile ids, kinds spawned).
* `[verified]` = read directly; `[inferred]` = interpretation; `[open]` = guess.
* Do NOT edit anything under `docs\` or any existing file except the output file
  named in your brief. Never run the game.
