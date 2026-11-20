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

### Demo studio

The external `.mac` viewer/editor is built separately for each Windows target:

```
cmd /c tools\build_demo_studio.cmd       REM x64  -> tools\demo_studio\build\jill_demo_studio.exe
cmd /c tools\build_demo_studio32.cmd     REM Win32 -> tools\demo_studio\build32\jill_demo_studio.exe
```

`make_release.cmd` and `make_release_32.cmd` build and copy the matching studio
executable into `Jill-EX-release-x64.zip` / `Jill-EX-release-x86.zip`.  It uses
the same `SDL2.dll`, `games/`, `profiles/` and `jill.ini` as the game.  See
`docs\demo-studio.md`.

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
* `port\ps4\build\JillEX-PS4-Data.zip` (companion data: `jill.ini`, `games/`,
  `profiles/` and a README; extract its contents into `/data/JillEX`)

For convenience the `.pkg` and `JillEX-PS4-Data.zip` are also copied to the
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

### Android build (Android SDK/NDK + SDL2)

The Android target is a separate Gradle project in `port\android` and does not
touch `port\build` or `port\build32`.  Prerequisites (see `TOOL_LOCATIONS.md`):
Android SDK/NDK, JDK 17 and the SDL2 source tree (2.32.10).

```
cmd /c build_android.cmd          :: assembleDebug + copy the APK to the root
cmd /c build_android.cmd install  :: ... and adb install -r
cmd /c build_android.cmd clean    :: remove app/build and app/.cxx
```

Outputs:

* `port\android\app\build\outputs\apk\debug\app-debug.apk` (installable debug APK)
* `JillEX-android.apk` in the repository root (git-ignored)

The APK is universal: `arm64-v8a`, `armeabi-v7a`, `x86_64` and `x86` (~6 MB).
Native libraries are linked with 16 KiB page alignment for Android 15+ devices;
without it installation fails with "16kb load segment alignment".  Build one ABI
for faster iteration with `gradlew.bat assembleDebug -PjillAbis=arm64-v8a`.

The APK contains only configuration (`jill.ini`, `games\`, `profiles\`); copy
your own `jillt\` and `ONESIM\` folders onto the device after installing:

```
adb push jillt  /sdcard/JillEX/
adb push ONESIM /sdcard/JillEX/
```

The app uses `/sdcard/JillEX` directly, so it requests "All files access"
(`MANAGE_EXTERNAL_STORAGE`) on Android 11+; the legacy storage permission is
used on Android 10 and older.

`port\android\local.properties` (git-ignored) points at the SDK and SDL2:

```properties
sdk.dir=D\:\\Apps\\AndroidSDK
sdl2.dir=D\:\\Apps\\AndroidSDK\\SDL2\\SDL2-2.32.10
```

See `port/android/README.md` for the project layout, data root, touch controls
and the remaining on-device work.

### Xbox build (NXDK + SDL2)

The Xbox target is a separate CMake project in `port\xbox` and does not touch
`port\build`, `port\build32`, `port\ps4` or `port\vita`.  Requirements (see
`TOOL_LOCATIONS.md`):

* NXDK at `D:\Apps\NXDK` (or set `NXDK_DIR`), including `lib\sdl\SDL2`,
  `lib\libcxx` and the prebuilt `tools\cxbe\cxbe.exe`.
* CMake 3.20+ and Ninja.

```
cmd /c build_xbox.cmd          :: compile + link + default.xbe + data zip
cmd /c build_xbox.cmd clean    :: remove port\xbox\build
```

Outputs:

* `port\xbox\build\default.xbe` (the game executable)
* `port\xbox\build\data\` (`default.xbe`, `jill.ini`, `games\`, `profiles\`,
  `external\`)
* `JillEX-Xbox.zip` in the repository root (the complete install bundle)

NXDK's prebuilt `libc++.lib`/`libSDL2.lib` are not required: both libraries are
compiled from the SDK's checked-out sources by the project.  The first build
takes ~1-2 minutes; later builds are incremental.

Deploy by extracting `JillEX-Xbox.zip` into one folder on the console (for
example `E:\Apps\Jill-EX\`), adding your own `jillt\` / `ONESIM\` folders, and
running `default.xbe`; burning the same layout as an XISO works too.  The data
root is the folder the XBE was launched from; if it has no `jill.ini`, the
first of these that does is used:

```
D:\  E:\JillEX  E:\Apps\Jill-EX  E:\Apps\JillEX  E:\Games\Jill-EX  E:\Games\JillEX
```

`D:\` is the disc root; a writable root is needed for saves, high scores and
`jill.log`.  See `port\xbox\README.md` for the port layout, the compile-time
decisions (no C++ unwinder, `jill_fs` filesystem facade, excluded libc++
units) and the runtime notes.  **Verified on hardware (2026-11-19): playable at
a stable frame rate in 480i/480p and 720p.**






