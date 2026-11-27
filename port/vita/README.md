# Jill-EX — PS Vita target

VitaSDK + SDL2 (audio/input) + libvita2d (display) build of the same engine as
the desktop and PS4 ports. Only `port/src` is shared; this folder owns the Vita
packaging and the libvita2d screen.

```
port/vita/
  CMakeLists.txt        cross-compiled target, SELF/VPK rules
  screen_vita2d.cpp     libvita2d (GXM) display backend
  jill.ini              default settings for ux0:data/JillEX
  sce_sys/              LiveArea art (placeholder, from the PS4 icon)
  build/                generated (git-ignored)
  VITA-DATA-README.txt  copied into the companion data zip as README.txt
```

## Prerequisites

1. VitaSDK installed with `vdpm` (see the repo's `PROGRESS.md`, 2026-11-14):

   ```
   git clone https://github.com/vitasdk/vdpm
   cd vdpm
   powershell -ExecutionPolicy Bypass -File .\bootstrap-vitasdk.ps1 -InstallDirectory C:\vitasdk
   vdpm install sdl2 --noconfirm
   ```

   `VITASDK` must point at the SDK root (`C:\vitasdk` by default).
   Install the display library too:

   ```powershell
   vdpm install libvita2d --noconfirm
   ```
2. CMake 3.18+ and Ninja. CMake's default Visual Studio generator cannot
   cross-compile with `arm-vita-eabi-g++`, so Ninja is required; Visual Studio
   ships a copy if `ninja` is not on PATH.

## Build

```
cmd /c build_vita.cmd          :: configure + build + vpk + data zip
cmd /c build_vita.cmd clean    :: remove port\vita\build
```

Outputs:

| File | What it is |
| --- | --- |
| `port/vita/build/jill.vpk` | installable bubble (title id `JILLEX001`) |
| `port/vita/build/jill.self` | the SELF inside the VPK |
| `port/vita/build/JillEX-Vita-data.zip` | `jill.ini`, `games/`, `profiles/` |
| `Releases/jill.vpk` | the same VPK, copied to the git-root `Releases/` |
| `Releases/JillEX-Vita-data.zip` | the same companion data zip |

Equivalent manual commands:

```
set VITASDK=C:\vitasdk
cmake -S port/vita -B port/vita/build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build port/vita/build --parallel
```

## Install

1. Copy `jill.vpk` to the console and install it with VitaShell.
2. Unpack `JillEX-Vita-data.zip` **and** your own extracted `jillt\` and
   `ONESIM\` folders into `ux0:data/JillEX\`:

   ```
   ux0:data/JillEX/jill.ini
   ux0:data/JillEX/games/       *.game manifests
   ux0:data/JillEX/profiles/    *.profile + *.pal
   ux0:data/JillEX/external/    optional asset packs
   ux0:data/JillEX/jillt/       Jill 1/2/3 data
   ux0:data/JillEX/ONESIM/      Onesimus data
   ```

Saves, high scores and `jill.log` are written next to those files, so
`ux0:data/JillEX` must stay writable.

## How the platform is selected

| Macro | Effect |
| --- | --- |
| `JILL_VITA` | data root `ux0:data/JillEX`, console log file, absolute path resolution; PS4-only videoout/pad/`_exit` paths stay compiled out |
| `JILL_HAVE_SDL` | SDL2 events, pad and audio; **SDL video is not initialised** |
| `screen_vita2d.cpp` | libvita2d (GXM) presentation, linked as `vita2d` |
| `SDL_MAIN_HANDLED` | the port keeps its own `main()` |

The screen is borderless fullscreen and always uses the 960x544 panel.
`screen_vita2d.cpp` uploads the 320x200 frame into an `A8B8G8R8` texture every
present, then draws it with libvita2d at the largest integer scale that fits
(2x = 640x400) and centres it; the black clear provides the letterbox.  SDL
video is not initialised, so libvita2d owns GXM/display while SDL handles
events, the pad and audio.  The window/drawable is still clamped to 960x544 even
if an old desktop `scale` or `resolution` setting asks for more; the Vita
`RESOLUTION` list only offers 960x544, 640x480 and 480x272.

## LiveArea artwork

SceShell only accepts **8-bit indexed** PNGs here, otherwise the install
fails at the very end with `0x8010113D` (VitaShell shows `0xFFFFFFFF`):

| File | Size | Limit |
| --- | --- | --- |
| `sce_sys/icon0.png` | 128x128 | 128 KiB |
| `sce_sys/livearea/contents/startup.png` | 280x158 | 64 KiB |
| `sce_sys/livearea/contents/bg.png` | 840x500 | 128 KiB |

`make_livearea.ps1` regenerates all three from the PS4 icon through
ffmpeg's `palettegen`/`paletteuse` (`pal8`); System.Drawing writes 32-bit RGBA
and would break the install.  Nothing may be interlaced.

## Verified on hardware (2026-11-14)

* Install, boot, front end, Jill 1 hub, CMF music, SFX, external asset packs and
  pad input all work on a retail Vita.
* Two platform details are load-bearing and easy to break again:
  * `main.cpp` defines `sceUserMainThreadStackSize = 1 MiB` and
    `sceLibcHeapSize = 32 MiB`; `vita-elf-create` reads those symbols.  The
    default 256 KiB stack silently kills the process during `Engine`.
  * `Engine` must stay off the stack (~72 KiB per object).
* The VitaSDK newlib has no C99-wide printf formats: never use `%zu`/`%jd` in
  `trace()`/`fprintf` here, it aborts rather than misprints.
* SDL exposes the pad as a game controller named `PSVita Controller`
  (`a:b2,b:b1,x:b3,y:b0,start:b11,back:b10`, D-pad on `b8/b6/b7/b9`), so
  `[controls]` in `jill.ini` binds cross to jump and square to throw.  A key
  missing from the ini keeps its built-in default; an empty value unbinds.

## Still to verify

* Jill 2/3 and Onesimus content, save/restore, high scores, text pages,
  endings, minimap and the NOISEMAKER screens.
* Screen fit/letterboxing at 960x544, audio latency and behaviour after
  suspend/resume.  The window is clamped and presentation now goes through
  libvita2d; this needs a visual check on hardware.
* `sce_sys/` art is a resized copy of the PS4 icon — replace it when there is
  real artwork (keep it 8-bit indexed).
