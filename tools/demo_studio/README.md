# Demo studio

External playback viewer/editor for the engine's recorded `*.mac` input
scripts.  It builds against the real engine core in `port/src`, so the game
view is the game's own 320x200 frame.

```bat
tools\build_demo_studio.cmd      REM 64-bit -> tools\demo_studio\build
tools\build_demo_studio32.cmd    REM 32-bit -> tools\demo_studio\build32
tools\run_demo_studio.cmd --mac jn1demo.mac
tools\run_demo_studio32.cmd --mac jn1demo.mac
tools\run_demo_studio.cmd --help
```

`F5` cycles the interpolation modes and `F4` opens the options page (UI text
scale, game view scale, interpolation and classic HUD).  `CTRL+N` starts a new
`.mac` from scratch; `INS` adds records at the playhead.  The full key list,
timeline columns and headless/selftest modes are documented in
[`docs/demo-studio.md`](../../docs/demo-studio.md).

Playback reads the same `[gameplay]` keys as the game, including `respawn_mode`,
`phoenix_water_death` and `throw_from_vines`, so a preview uses the options you
are currently testing.  The matching CLI switches are `--respawn N`,
`--no-phoenix-water-death` and `--throw-from-vines`.

Generated files (`build/`, `shots/`) are ignored by git.
