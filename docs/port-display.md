# Port display options: filters and limits

All post-processing happens on the 320x200 ARGB frame just before the texture
upload, so it works with SDL's accelerated and software renderers alike.  It is
not a GLSL/HLSL shader pipeline.

## Stereo 3D

`OPTIONS > VIDEO > STEREO 3D` cycles `OFF / INTERLACED`.  The game is rendered
twice with a horizontal eye separation: a static board backdrop, tiles at half
the separation and objects at the full separation.  HUD text, dialog boxes, the
message line and menus stay at zero parallax.  `INTERLACED` takes even output
rows from the left eye and odd rows from the right.

SBS output is disabled for now and its depth/convergence rows are hidden from
the menu.  The settings remain available to the interlaced path as
`[video] stereo = 0/1`, `[video] stereo_depth` (1..8, default 2) and
`[video] stereo_convergence` (0..8, default 0), with `--stereo off|interlaced`,
`--stereo-depth N` and `--stereo-convergence N`.  Headless `--dump` applies the
same interlaced composition.

## Filters

`OPTIONS > VIDEO > FILTER` cycles `NONE / SMOOTH / CRT` (default **NONE**).
Stored as `[video] filter = 0/1/2`, with `--filter 0|1|2` (names `none`,
`smooth`, `crt` are also accepted).

`SMOOTH` runs two separable 3-tap passes over the frame:

* horizontal `(left + 2*centre + right) / 4`
* vertical `(up + 2*centre + down) / 4`, mixed 75% blurred / 25% original

The result keeps the pixel-art edges but takes the hard stair-steps off.

`CRT` runs a horizontal 1-2-1 bleed, darkens every other source row to 72%
(scanlines), adds a slight phosphor glow from the horizontal filter and applies
a radial vignette (up to 38% darkening at the corners).  At integer window
scales the source rows become 2-6 pixel scanlines, which is what gives the
effect its CRT look.

`--dump FILE` renders the same filter path headlessly, so the output can be
compared without opening a window:

```bat
port\build\Release\jill.exe --game jill1 --board 1 --ticks 5 --dump plain.ppm
port\build\Release\jill.exe --game jill1 --board 1 --ticks 5 --filter 1 --dump smooth.ppm
port\build\Release\jill.exe --game jill1 --board 1 --ticks 5 --filter 2 --dump crt.ppm
```

## Object and kind limits

The display work also raised two total-conversion limits:

* `ProfileDef::kind_limit()` is at least **138** (double Jill 1's 69 kinds).
  Ids above the compiled table use safe dummy metadata (zero flags), so they
  do not participate in hit/trigger passes but can still be carried by a board
  or spawned.
* `Engine::kActiveLimit` is **384** (double the original 0xC0 visible/active
  cap), matching the 384-record object ceiling used for large mods.

## Level select

`F3` and `OPTIONS > CHEATS > LEVEL SELECT` both open the same board picker.
The panel scrolls after 16 rows and shows `first-last / total`, so a 21-board
total conversion fits on the 320x200 screen.
