\## Building



\### Requirements

\* Windows 10/11 x64.

\* Visual Studio 2022 with the \*\*Desktop development with C++\*\* workload

&#x20; (MSVC v143 and a Windows SDK).

\* CMake 3.18 or newer (bundled with Visual Studio, or a standalone install).

\* vcpkg, by default at `C:\\vcpkg` (or set `VCPKG\_ROOT`), with SDL2:

&#x20; `vcpkg install sdl2:x64-windows`.

\* SDL2 is optional for a headless-only build. A normal game build needs it for

&#x20; the window, input and presentation.

\* Python 3 is optional and only needed to regenerate data or run the tools:

&#x20; `py -3 -m pip install -r requirements.txt` (Pillow).

\* Game data, configured in `jill.ini` `\[paths]`. The shipped file uses

&#x20; `D:/Emu/DOS/C/jillt` and `D:/Emu/DOS/C/ONESIM`; a fresh checkout can instead

&#x20; place local `jillt` and `ONESIM` folders next to `run.cmd`.



\### Build steps

1\. Install the Visual Studio C++ workload and CMake.

2\. Install vcpkg and SDL2:

&#x20;  `C:\\vcpkg\\vcpkg install sdl2:x64-windows`

&#x20;  or run `build.cmd install`.

3\. Check the asset paths in `jill.ini`:

&#x20;  ```

&#x20;  \[paths]

&#x20;  jill1 = D:/Emu/DOS/C/jillt

&#x20;  jill2 = D:/Emu/DOS/C/jillt

&#x20;  jill3 = D:/Emu/DOS/C/jillt

&#x20;  onesimus = D:/Emu/DOS/C/ONESIM

&#x20;  ```

4\. Build:

&#x20;  ```

&#x20;  cmd /c build.cmd

&#x20;  ```

&#x20;  The executable is written to `port\\build\\Release\\jill.exe` (with

&#x20;  `SDL2.dll` beside it).

5\. Run:

&#x20;  ```

&#x20;  run.cmd

&#x20;  ```

&#x20;  or `port\\build\\Release\\jill.exe --game jill1`.

6\. Headless smoke test:

&#x20;  ```

&#x20;  port\\build\\Release\\jill.exe --game jill1 --board 1 --ticks 40

&#x20;  ```



\### 32-bit build

The 64-bit build stays the default.  A separate Win32/x86 build is available:

1. Install the 32-bit SDL2 package once:

&#x20;  ```
&#x20;  cmd /c build32.cmd install
&#x20;  ```

2. Build:

&#x20;  ```
&#x20;  cmd /c build32.cmd
&#x20;  ```

The executable and `SDL2.dll` are written to
`port\build32\Release32\`.  Run it with `run32.cmd` or
`port\build32\Release32\jill.exe --game jill1`.  The 32-bit build uses
`vcvars32.bat`, `-A Win32` and the `x86-windows` vcpkg triplet; it does not
touch `port\build\Release`.

If Visual Studio is not in the default location, edit the `VCVARS` path in

`build.cmd`. If vcpkg is elsewhere, set `VCPKG\_ROOT`. To rebuild the generated

tables after editing the analysis data, run `py -3 re\\gen\_port\_data.py` and

`py -3 re\\gen\_games.py`.

### PS4 build (OpenOrbis)

The PS4 target is a separate script and does not touch `port\build` or
`port\build32`.  Requirements:

* OpenOrbis PS4Toolchain, with `OO_PS4_TOOLCHAIN` set (default
  `D:\Apps\OpenOrbis\PS4Toolchain`).
* LLVM/clang and `ld.lld` on `PATH` (default fallback
  `C:\Program Files\LLVM\bin`).

Build:

```
cmd /c build_ps4.cmd
```

This produces both:

* `port\ps4\build\eboot.bin` (loose homebrew executable)
* `port\ps4\build\IV0000-BREW00199_00-JILLEXPS40000000.pkg` (fake-signed
  installable package)
* `port\ps4\build\JillEX-data.zip` (companion data: `jill.ini`, `games/`,
  `profiles/` and a README; extract its contents into `/data/JillEX`)

For convenience the `.pkg` and `JillEX-data.zip` are also copied to the
repository root after a successful build (and are listed in `.gitignore`).

Packaging artwork (all optional except `icon0.png`):

```text
port\ps4\sce_sys\icon0.png   512x512   the home-screen icon (required)
port\ps4\sce_sys\pic0.png    1920x1080 large background art on the home screen
port\ps4\sce_sys\pic1.png    1920x1080 additional background image
port\ps4\sce_sys\snd0.at9              startup sound (AT9)
```

Replace the file and re-run `build_ps4.cmd`; the pkg picks up any `pic0`,
`pic1` or `snd0.at9` that exists in that folder, and includes them in the
`create-gp4` file list automatically.  Reinstall the pkg to refresh the tile
(the console caches the icon; delete the old installation if it does not
change).

Use `build_ps4.cmd nopkg` for a quick eboot-only iteration, and
`build_ps4.cmd clean` to remove the output folder.  Packaging needs the
OpenOrbis `PkgTool.Core` (.NET Core 3.0+; the script sets
`DOTNET_ROLL_FORWARD=Major` so newer runtimes work).  The `sce_module/*.prx`
and `sce_sys/about/right.sprx` files are copied from the installed toolkit at
build time and are not stored in this repository.

The player supplies their own extracted game data; on the console everything
lives under `/data/JillEX`:

```
jill.ini  games\  profiles\  external\  jillt\  ONESIM\
```

Display backend selection (PS4 only):

```
[video]
renderer = auto     ; auto | videoout | gl | sdl
```

`auto` prefers the native `sceVideoOut` backend (triple buffered, 60 Hz), then
OpenGL ES (Piglet), then the SDL software path.  Piglet/EGL fails on hardware
with `eglGetDisplay` and the SDL PS4 driver stalls on the flip (30 Hz), so the
native path is the default.  Force `videoout`, `gl` or `sdl` to override.
Startup diagnostics are written to
`/data/JillEX/jill.log`.  The package deliberately does **not** contain game
data or the project's `games/`/`profiles/` folders; copy them next to
`jill.ini` in `/data/JillEX`.

Debug overlay (PS4 only):

```
[debug]
overlay = 1          ; or pass --debug
```

Shows FPS, frame time, engine tick / frame build / present times and the audio
mixer cost (`A mix/budget ms`).  Every 5 seconds the same values are written to
`jill.log` as `perf: fps=... frame=... tick=... draw=... present=... audio=...`.

See `PS4_PORT_PLAN.md` for the renderer design, verified toolchain constraints,
the FluidSynth licence verdict and the remaining on-device work.





