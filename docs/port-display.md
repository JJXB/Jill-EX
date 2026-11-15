# Port display options: filters and limits

All post-processing happens on the 320x200 ARGB frame just before the texture
upload, so it works with SDL's accelerated and software renderers alike.  It is
not a GLSL/HLSL shader pipeline.

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

## FPS counter
`OPTIONS > VIDEO > FPS COUNTER` toggles a small `NN FPS` overlay (default
**OFF**), stored as `[video] fps_counter = 0/1`.  It shows the measured
presentation rate (the loop's start-of-frame interval, smoothed with a 0.9/0.1
EMA), so with HIGH FRAMERATE off it reads about 18, and with 60/90/120/144 it
tracks the selected rate.  In the classic HUD it sits at the left end of the
title strip; in the modern HUD it sits just under the top bar.  It is drawn
into the 320x200 frame before the palette conversion, so the filters and the
Vita/PS4 backends see it like any other pixel.

## Menu and HUD scaling

Menus and the in-game HUD scale independently; the rendered game and the menu
backdrop keep their original pixel size.  All values are percentages of the
original 6x8 cell, and **100% is the default**, which leaves every existing
layout exactly as before.

| Option | Ini key | Effect |
| --- | --- | --- |
| `OPTIONS > MENUS > SCALE` | `[menus] scale` | Zooms the whole menu: window, padding, row spacing and text |
| `OPTIONS > MENUS > TEXT SIZE` | `[menus] font_scale` | Text size relative to that zoom (100% = the zoomed size) |
| `OPTIONS > HUD > FONT SIZE` | `[hud] font_scale` | Text size of the modern top bar |

Each offers `50 / 75 / 100 / 125 / 150 / 200%`.  Glyphs are drawn with a
nearest-neighbour pixel scale, so 200% gives doubled 12x16 cells.  The menu
recomputes its box and row height from the scale and shrinks long labels back
to fit the 320x200 frame; the 14-row CHEATS screen still fits at 200%.  The
modern HUD grows its bar to fit the font and drops the `HEALTH` word when the
scaled layout leaves no room (the pips still show the value).  The classic
64-px HUD panel keeps the original cells because doubled text cannot fit, and
its `FONT SIZE` setting therefore has no visible effect while `STYLE: CLASSIC`
is selected.

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
