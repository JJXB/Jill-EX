# Jill-EX PS4 port plan (OpenOrbis + Piglet OpenGL ES)

Status: **planning + first build slice** (2026-11-12)
Target: PS4 homebrew (`eboot.bin`, optionally a fake-signed `.pkg`)
Toolchain: `D:\Apps\OpenOrbis\PS4Toolchain` (`%OO_PS4_TOOLCHAIN%`)
Host LLVM: `C:\Program Files\LLVM\bin` (clang 23.1.4)

## 0. Delivered in this slice

Already in the tree and verified:

* `port/src/platform_paths.{h,cpp}` — `/data/JillEX` root, directory creation,
  and PS4 fallback for stale Windows paths in a copied `jill.ini`.
* `port/src/screen.h`, `port/src/screen_sdl.cpp` — display seam; the PC SDL
  window/renderer/texture code moved out of `app.cpp` unchanged.  PC build
  re-verified with `build.cmd`.
* `port/ps4/screen_gl.cpp` — EGL/Piglet OpenGL ES 2.0 renderer using the
  firmware `texmap` shaders, 320x200 RGBA streaming texture, 1080p integer fit.
* `[video] renderer = auto|gl|sdl` (and `--renderer`): `auto` uses GLES on
  hardware and falls back to the SDL software screen under shadPS4 (detected
  through its `/hostapp` mount), where Piglet init hangs.  Startup milestones
  are traced to `jill.log` so a hang is locatable.
* `port/ps4/jill.ini` — PS4 defaults with local `jillt`/`ONESIM` paths.
* `build_ps4.cmd` — full compile + link + `create-fself`; produces
  `port\ps4\build\eboot.bin` (verified: 4.9 MB eboot from a clean build).
* Fake-signed `.pkg` packaging is part of the same build:
  `port\ps4\build\IV0000-BREW00199_00-JILLEXPS40000000.pkg` (verified:
  7.3 MB, includes eboot, param.sfo, icon0.png, libc.prx, libSceFios2.prx,
  `sce_sys/about/right.sprx`).  `build_ps4.cmd nopkg` skips it.
* Toolchain fixes: SDL 2.0.9 hint guard, `std::fabs` type fix, `gnu++17`.
* `main.cpp` — PS4 data-root creation, absolute `/data/JillEX/...` resolution
  for `jill.ini`, `games/`, `profiles/`, `external/`, per-game folders,
  palettes, saves and high scores, plus `jill.log` mirroring.

Not done yet (Phase 1+ in §9): booting on hardware, pad-only Noisemaker and
hiscore entry, hiding keyboard/VIDEO/MIDI-only rows, packaging, and the
on-device test pass.

## 1. Goal

One source tree that builds:

| Target | Build system | Renderer | Status |
| --- | --- | --- | --- |
| Windows x64/x86 (existing) | `build.cmd` / `build32.cmd` (CMake + MSVC + vcpkg SDL2) | SDL2 SDL_Renderer | unchanged, must keep building |
| PS4 (new) | `build_ps4.cmd` (clang + ld.lld + create-fself) | **OpenGL ES 2.0 via Piglet/EGL** | first slice in this change |

The PS4 build must not change PC behaviour. All new platform code is either
compiled only for PS4 or is a no-op on Windows.

## 2. Verified environment facts

These were checked against the installed toolkit, not assumed:

* `%OO_PS4_TOOLCHAIN%` exists and contains `libSDL2.a`, `libSDL2main.a`,
  `libSDL2_image.a`, `include/SDL2`, `include/EGL`, `include/GLES2`,
  `include/orbis`, the PS4 libc/libc++ sysroot and the OpenOrbis samples.
* The bundled SDL2 is **2.0.9**, not 2.32.  It contains
  `SDL_ps4video.c`, `SDL_ps4audio.c`, the PS4 joystick driver and the PS4
  software renderer.  There is **no SDL OpenGL driver**, so the OGL renderer
  must talk to EGL/Piglet directly.
* Piglet is OpenGL ES 2.0 + EGL 1.4.  The working sample uses
  `libScePigletv2VSH` plus `libScePrecompiledShaders`, and loads shaders as
  **PSSL binaries** with `glShaderBinary` (`texmap/v_2.vert`,
  `texmap/f_2.frag`).  Uniforms/attributes: `u_modelViewMatrix`,
  `u_projectionMatrix`, `u_textureSpaceMatrix`, `s_sampler`, `u_opacity`,
  `a_vertex` (`vec4` = xy position + uv).
* `std::filesystem` is present in the bundled libc++ 11 headers and links
  against `libc++.a` (verified with a test ELF using
  `fs::directory_iterator`).
* The port must be compiled with **`-std=gnu++17`**.  With `-std=c++17`
  clang defines `__STRICT_ANSI__`, which makes the PS4 musl-style headers hide
  `nanosleep`, `locale_t`, etc., and libc++ fails to compile.  (`-std=gnu++17`
  or `-D_DEFAULT_SOURCE` both work; `gnu++17` was verified.)
* Current source survey (PS4 target, `-DJILL_HAVE_SDL`): every port source and
  every `third_party` C file compiles except three known issues:
  1. `SDL_HINT_DIRECTINPUT_ENABLED` does not exist in SDL 2.0.9.
  2. `std::fabs(float)` returns `double` on this libc, so
     `std::max(float, std::fabs(float))` fails in `sfx.cpp`/`audio.cpp`.
  3. `<cmath>` assumes `::abs` exists in the PS4 headers; include
     `<cstdlib>`/`<stdlib.h>` first (the current translation units already do).
* `mediafoundation`/WinMM must be Windows-only (already guarded or trivial to
  guard).

## 3. Data and paths: `/data/JillEX`

Everything the game reads or writes lives under one root:

```
/data/JillEX/
  jill.ini              settings + bindings (written on exit)
  games/                *.game manifests (project data)
  profiles/             *.profile + *.pal
  external/             installed asset packs / autoload
  jillt/                Jill 1/2/3 extracted game data (user supplied)
  ONESIM/               Onesimus extracted game data (user supplied)
  jn1save.* / jn2save.* / jn3save.* / onesave.*   save slots
  hiscore_<id>.txt      high-score tables
  jill.log              diagnostic log (PS4 only)
```

Implementation:

* New `port/src/platform_paths.{h,cpp}`:
  * `jill_paths::data_root()` returns `"/data/JillEX"` when `JILL_PS4` is
    defined and `""` on desktop.
  * `jill_paths::resolve(path)` joins a relative path onto the root and leaves
    absolute paths alone.
  * `jill_paths::ensure_data_root()` creates the folder on PS4 (no-op on PC).
* `main.cpp` uses that root instead of the exe/argv search on PS4, creates it,
  `chdir`s into it, and mirrors `stdout`/`stderr` to `jill.log`.
* Because the process working directory becomes `/data/JillEX`, every existing
  relative path (`jill.ini`, `games`, `profiles`, `external`, `jillt`,
  `ONESIM`, save slots, high scores) already lands in the right place.  No
  engine or app-level path rewrite is needed.
* The PS4 build does **not** rely on `chdir` alone: shadPS4 stubs
  `posix_chdir`, so the working directory is whatever the loader set.  All
  configured directories, per-game asset folders, palettes, save slots, high
  scores, `jill.ini` and `jill.log` are resolved through
  `jill_paths::resolve()` to absolute `/data/JillEX/...` paths before use.
  On desktop `resolve()` is an identity function.
* `port/ps4/jill.ini` is the PS4 default file.  It uses local relative asset
  folders (`jill1 = jillt`, `onesimus = ONESIM`) instead of the Windows
  `D:/Emu/...` paths.  If a PC `jill.ini` is copied to the console, the startup
  code drops absolute asset paths that do not exist and falls back to the
  local `games`/`profiles`/manifest defaults, so the first run still finds the
  game data.
* Writable-root note: `/data` is the usual homebrew writable mount.  If a
  loader/sandbox denies it, `SDATA` can be added as a fallback later.  The root
  is created with `std::filesystem::create_directories`.

## 4. Renderer: OpenGL ES 2.0 (Piglet)

Do **not** route PS4 video through SDL.  SDL 2.0.9's PS4 video driver has no GL
context, and mixing a software window surface with Piglet is not supported.

Architecture: a small `Screen` interface in `port/src/screen.h`:

```cpp
class Screen {
    virtual bool init(int px_w, int px_h, int mode, std::string& err);
    virtual void apply(int px_w, int px_h, int mode, int disp_w, int disp_h);
    virtual void present(const uint32_t* argb, int w, int h);
    virtual void set_title(const char* title);
    virtual void shutdown();
};
```

* `port/src/screen_sdl.cpp` — PC implementation.  It is the current
  `SDL_CreateWindow` / `SDL_CreateRenderer` / streaming-texture code moved
  behind `present()`, so PC rendering is pixel-identical.
* `port/ps4/screen_gl.cpp` — PS4 implementation:
  * Load `libScePigletv2VSH.sprx` + `libScePrecompiledShaders.sprx` through
    `sceKernelLoadStartModule` using the sandbox prefix (same as the piglet
    sample).
  * `scePigletSetConfigurationVSH` → `eglGetDisplay` → `eglInitialize` →
    `eglBindAPI(EGL_OPENGL_ES_API)` → window surface (`1920x1080`) →
    ES2 context → `eglMakeCurrent`.
  * Build one program from the precompiled `texmap` vertex/fragment shaders
    (`glShaderBinary`, format 0 = PSSL).  This avoids needing Sony's shader
    compiler or runtime GLSL compilation.
  * Create one 320x200 `GL_RGBA`/`GL_UNSIGNED_BYTE` streaming texture.  The
    game framebuffer stays 8-bit indexed; `present()` converts it to a
    swizzled RGBA byte buffer (320*200*4 = 256 KiB/frame) and calls
    `glTexSubImage2D`.
  * Draw one textured quad.  Output rect is centred and integer-scaled to the
    largest fit in 1920x1080 (5x = 1600x1000 with pillarboxing).  A later
    option can stretch to 16:9 or use an arbitrary fit.
  * `eglSwapBuffers` each frame.  Vsync can be 0 (the app already paces
    presentation) or 1 (then remove the software pacer for PS4).
  * `present()` keeps the CPU `filter_smooth`/`filter_crt` path that already
    exists in `app.cpp`, so "smooth" and "CRT" keep working without a shader.
* `app.cpp` changes are limited to the existing window/present call sites
  (`SDL_CreateWindow` block, `apply_video`, `SDL_UpdateTexture` block, title,
  cleanup).  All keyboard/gamepad event handling stays as-is.

Shader/texture caveat: the precompiled `texmap` shaders are Sony firmware
content and are loaded at runtime, not redistributed.  A custom PSSL shader
would require the official Sony toolchain; it is not needed for the port.

## 5. Input, audio, MIDI

* **Input:** keep SDL for controller events (`SDL_INIT_GAMECONTROLLER |
  SDL_INIT_JOYSTICK`) and read the existing `poll_input`.  On PS4, do not init
  SDL video; `SDL_GetKeyboardState` returns an all-zero array and no keyboard
  events arrive.
* **Audio:** use the bundled SDL 2.0.9 PS4 audio driver (SceAudioOut).  The
  existing `MusicPlayer` callback path is platform-neutral, and the OPL2
  emulator, SFX mixer and external WAV/OGG/MP3 decoding are all software.
* **MIDI OUT:** WinMM does not exist on PS4.  `midi.cpp` already stubs out to
  an empty device list; the `MIDI OUT` option must be hidden/disabled so the
  menu does not offer a dead device.  No system MIDI on PS4.
* **Fullscreen/window menu:** keep the rows for now, but make them no-ops on
  PS4 (or hide `VIDEO > WINDOW TYPE/SIZE/RESOLUTION`).  The GL screen always
  outputs 1080p.

## 6. FluidSynth for MIDI: licence and feasibility

**Licence:** Fluidsynth is **LGPL-2.1-or-later** (the `LICENSE` file is plain
LGPL-2.1; sources say "version 2.1 or any later version").  It also depends on
**GLib (LGPL-2.1+)**, and optionally libsndfile (LGPL-2.1+) and others.

**Compatibility with this project:**

* Jill-EX is PolyForm Noncommercial 1.0.0, i.e. source-available but not a
  permissive/OSI licence.  LGPL §6 allows linking a "work that uses the
  Library" only if the whole can be modified/relinked by the recipient.  A
  static PS4 `eboot.bin` with FluidSynth inside therefore requires shipping
  the LGPL source *and* a relinkable form of Jill-EX (object files or full
  source plus build scripts), plus prominent notices.  That is possible in
  practice because the project ships source, but it adds real obligations and
  a licence-review step to every release.
* The bigger practical blocker is **GLib**.  There is no official PS4 port of
  GLib; it expects POSIX threads, `gmodule`, `gthread`, iconv/locale and a
  build system that does not target FreeBSD/Orbis.  Porting GLib is far more
  work than the synth itself.  FluidSynth also wants a writable config dir,
  threads, file I/O and an audio driver backend.
* FluidSynth needs a **SoundFont (.sf2)**.  That is a separate asset with its
  own licence (e.g. GeneralUser GS, FluidR3, MuseScore_General).  It cannot be
  assumed to be redistributable, and it is 10-100+ MB of RAM/disk.

**Verdict:** technically *possible* but not worth it for this port.  Do not
make FluidSynth a PS4 dependency.

**Recommended alternatives:**

1. Keep the existing **OPL2/CMF** path as the PS4 default.  It is pure
   software, already accurate to the DOS originals, and adds no dependencies.
2. If a General-MIDI-sounding option is wanted later, use **TinySoundFont**
   (MIT, single header, no GLib) with a user-supplied `.sf2`, or a small
   built-in GM sample set.  This keeps the licence and build simple.
3. If FluidSynth is ever added for PC only, keep it behind a desktop-only
   CMake option and never link it into the PS4 eboot.

## 7. Build system

New `build_ps4.cmd` (does not touch `build.cmd`/`build32.cmd`):

1. Require `%OO_PS4_TOOLCHAIN%` and clang/`ld.lld` on `PATH` (fallback to
   `C:\Program Files\LLVM\bin`).
2. Compile every `port/src/*.cpp` plus `port/ps4/screen_gl.cpp` with:
   `--target=x86_64-pc-freebsd12-elf -std=gnu++17 -fPIC -funwind-tables
   -fexceptions -fcxx-exceptions -DJILL_PS4=1 -DJILL_HAVE_SDL=1
   -I%OO_PS4_TOOLCHAIN%/include -I.../include/c++/v1 -I.../include/SDL2`
3. Compile `port/src/third_party/*.c` as C (`-std=gnu11`).
4. Link with
   `ld.lld -m elf_x86_64 -pie --script %OO_PS4_TOOLCHAIN%/link.x
   --eh-frame-hdr -L%OO_PS4_TOOLCHAIN%/lib
   -lc -lkernel -lc++ -lSDL2 -lScePigletv2VSH -lScePrecompiledShaders
   -lSceSysmodule -lSceSystemService -lSceUserService -lSceAudioOut -lScePad
   -lSceVideoOut -lSceShellCoreUtil -lSceRtc
   %OO_PS4_TOOLCHAIN%/lib/crt1.o ...objects...`
5. `create-fself` → `eboot.bin` (with the OpenOrbis authinfo/paid values from
   the samples).
6. Fake-signed package, built by the same script:
   * stage `eboot.bin`, `sce_sys/icon0.png` (repo), `sce_sys/about/right.sprx`
     and `sce_module/libc.prx` + `libSceFios2.prx` (copied from the installed
     OpenOrbis sample at build time; not committed).
   * `PkgTool.Core sfo_new/sfo_setentry` creates `sce_sys/param.sfo`
     (`TITLE_ID=BREW00199`, `CATEGORY=gd`, `TITLE=Jill-EX`, version 1.00).
   * `create-gp4` + `PkgTool.Core pkg_build` produce
     `port\ps4\build\IV0000-BREW00199_00-JILLEXPS40000000.pkg`.
   * The content id must stay exactly 36 characters.  `PkgTool.Core` targets
     .NET Core 3.0, so the script exports `DOTNET_ROLL_FORWARD=Major`.
   * `build_ps4.cmd nopkg` stops after `eboot.bin` for quick iterations.
   * Game data, `games/` and `profiles/` are intentionally **not** in the
     package; the user copies them to `/data/JillEX` after installing.

CMake is intentionally not used for the PS4 target in the first slice: the
OpenOrbis flow is compiler-driver + script driven and a hand-written script is
easier to verify.  If the project later wants one entry point, add a
`toolchain/ps4.cmake` plus external-project packaging; the source list and
flags above are the contract.

## 8. Other issues and risks

Severity: H = blocks boot/play, M = breaks a feature, L = cosmetic/polish.

| # | Sev | Issue | Evidence / impact | Mitigation |
| --- | --- | --- | --- | --- |
| 1 | H | **Case-sensitive filesystem.** PS4 `/data` is case-sensitive; Windows is not. | `games/*.game` reference `ONESIMUS.SHA`, `jill1.sha`, `SONG5.ARK` etc.  They match today, but a mod or user copy with different case will fail. | Validate every manifest path at load, report the exact missing file, and document exact case.  Add a `--check-data` mode later. |
| 2 | H | **No keyboard on PS4.** Menus, hiscore name entry, Noisemaker and the Controls screen assume one. | Pad navigation exists for most menus, but `NOISEMAKER` needs QWERTY, hiscore entry needs letters, `CONTROLS` rebinding asks for keys, F1-F5/F11 are debug keys. | Pad-only pass: Noisemaker D-pad/A/B, on-screen hiscore letter picker (or default `JILL`), hide keyboard rows in Controls, hide F-key hints, add missing actions to Options. |
| 3 | H | **SDL2 2.0.9 API drift.** | `SDL_HINT_DIRECTINPUT_ENABLED` (2.0.18+) is used unconditionally; newer hints/APIs may appear later. | Guard with `SDL_VERSION_ATLEAST`, keep the renderer behind `Screen`, test the PS4 build in CI. |
| 4 | H | **No runtime GLSL on Piglet.** | Piglet has no GLSL compiler; shaders are PSSL binaries. | Use the firmware `texmap` precompiled shaders (proven in the sample).  Any custom effect (CRT) needs another precompiled shader or CPU filter. |
| 5 | H | **PS4 process CWD is `/` (read-only) and some loaders stub `chdir`.** | Every relative read/write would fail before the path fix; shadPS4 logs `posix_chdir: (STUBBED)` and then cannot open `jill.ini`/`jillt/...`. | Never rely on cwd: resolve all config dirs, per-game data dirs, palettes, saves, high scores, `jill.ini` and `jill.log` to absolute `/data/JillEX/...` paths; keep the chdir as a convenience only. |
| 6 | H | **Toolchain `std=gnu++17` requirement.** | `-std=c++17` hides POSIX declarations; libc++ headers fail (`nanosleep`, `locale_t`). | Set `gnu++17` in the PS4 build; leave MSVC/CMake alone. |
| 7 | M | **`std::fabs` type mismatch.** | `std::fabs(float)` returns `double`; `std::max(float, double)` fails. | Cast to `float` in `sfx.cpp`/`audio.cpp` (done). |
| 8 | M | **No Media Foundation on PS4.** | `extaudio.cpp` M4A/AAC path is Windows-only; pack music in `.m4a`/`.aac` will not decode. | Document supported pack audio (WAV/OGG/MP3).  Optional later: libSceAudiodec. |
| 9 | M | **MIDI OUT menu offers nothing useful.** | WinMM absent; `midi.cpp` stubs. | Hide/disable `MIDI OUT` on PS4. |
| 10 | M | **Window/video options are meaningless on PS4.** | `VIDEO` submenu changes an SDL window that does not exist. | Hide the submenu or wire it to the GL scale mode. |
| 11 | M | **Controller mapping needs verification.** | SDL 2.0.9's PS4 joystick driver maps Cross/Circle to A/B; Start/Options mapping and reconnect behaviour must be tested on hardware. | On-device test; keep the existing raw-joystick fallback.  Consider ScePad directly if 2.0.9 mapping is poor. |
| 12 | M | **Piglet memory config.** | The sample asks for 250 MB system + 512 MB video + 170 MB flexible memory.  It works but is oversized for a 2D game and may conflict with constrained homebrew. | Start from the sample values (known good); tune down after boot testing. |
| 13 | M | **No console output.** | `printf`/`stderr` are invisible; crashes are silent. | Mirror stdio to `/data/JillEX/jill.log` on PS4; keep log flushing.  Optionally `sceKernelDebugOutText`. |
| 14 | M | **Distribution/licensing.** | Original game data cannot be in the PKG.  SDL2/Piglet notices must ship. | PKG contains only code/project data; users copy their own `jillt`/`ONESIM` to `/data/JillEX`.  Update `THIRD_PARTY_NOTICES.md` for SDL2 2.0.9/Piglet. |
| 15 | M | **Save slots and high scores are written to CWD.** | They will land in `/data/JillEX` after the chdir, which is correct, but only if the folder is writable. | Check `create_directories` + a write test at startup; show a clear error. |
| 16 | L | `SDL_SetWindowTitle` every 500 ms. | No-op on PS4; harmless. | Keep, or skip under `JILL_PS4`. |
| 17 | L | The CRT filter is CPU-side. | Works, costs a full 320x200 pass per frame; fine. | Keep; revisit as a shader later. |
| 18 | L | `stb_image`/miniz/stb_vorbis/dr_mp3 compile fine. | Verified with the PS4 target. | None. |
| 19 | L | `libScePrecompiledShaders` availability is firmware-dependent. | If a firmware lacks the module, the renderer cannot start. | Fail with a readable log line; consider shipping a PSSL shader built with the official toolchain later. |
| 20 | L | PC/PS4 binary asset parity. | `jill.ini` differs (Windows absolute paths vs local PS4 folders). | Ship `port/ps4/jill.ini`; document the difference. |
| 21 | M | `PkgTool.Core` targets .NET Core 3.0. | Fails on machines with only .NET 6/8/9/10 unless roll-forward is allowed. | `build_ps4.cmd` sets `DOTNET_ROLL_FORWARD=Major` (verified with .NET 6/8/9/10). |
| 22 | M | Homebrew title-ID collision. | `BREW00199` is a placeholder; IDs are not globally allocated. | Change `PKG_TITLE_ID`/`PKG_CONTENT_ID` near the top of `build_ps4.cmd` before a public release. |
| 23 | L | System modules are copied from the local toolkit at build time. | `libc.prx`, `libSceFios2.prx` and `right.sprx` must be present in the package. | The script checks for them and prints a clear error; they are not committed to this repository. |
| 24 | H | **Piglet/EGL and the SDL presentation path can both show a black screen.** | The OpenOrbis SDL2 PS4 driver copies the window surface using `window->x/y` as the destination offset; shadPS4 reports 720p, so a forced 1080p window was centred at (-320,-180) and only a black corner was copied. | The PS4 SDL backend sizes the window from `SDL_GetDesktopDisplayMode(0)` and creates it at (0,0); direct window-surface writes are kept. `sdl screen: video mode` + `presented #n` confirm. |
| 25 | M | `/hostapp` is present on the test console too. | It cannot be used as a shadPS4-only marker; `auto` picks SDL everywhere on that setup. | Force `renderer = gl` on hardware; a better probe is still needed. |
| 26 | H | `eglGetDisplay` fails on hardware with `0x12288`. | Piglet never produces a display even with the sample's exact config/flags. | Deferred: hardware uses the fixed SDL backend for now.  Next step is to log `scePigletGetConfigurationVSH` and try a smaller shared-memory config. |
| 27 | H | **No controller input.** | SDL2's PS4 joystick driver builds a malformed GUID, so `SDL_IsGameController` is false and no `SDL_CONTROLLERBUTTONDOWN` events are ever sent; the menu code only listens for those and keyboard events. | Native libScePad polling in `port/ps4/ps4_pad.cpp`; pad edges are converted to SDL controller events for all menu paths and fed directly to the engine during play. |
| 28 | M | Occasional music slowdown. | SDL2's PS4 audio driver calls `sceAudioOutOutput(handle, NULL)` to wait for the previous buffer *before* mixing the next one, so mixing time is added to every playback period (worse on the slower console CPU). | PS4 uses a native `sceAudioOut` thread (`MusicPlayer::ps4_open`) that keeps two buffers: one plays while the other is mixed. `audio: mix ... ms / 42.7 ms budget` reports the loader. |
| 29 | H | **Frame rate capped at 30 Hz on hardware.** Perf overlay: present ~25 ms, frame ~33 ms. | SDL2's PS4 video driver waits for the previous flip to finish before building the next frame (one frame per ~1.5 vblanks). | Native sceVideoOut backend with 3 buffers and two flips in flight (screen_videoout.cpp); enderer = auto prefers ideoout, then gl, then sdl. |
| 30 | H | **CE-34878-0 when quitting.** Log reached startup: main loop exited, so the fault was in CRT/SDL teardown. | PS4 devices are already closed at that point and texit(SDL_Quit)/static destructors run afterwards. | PS4 main calls _exit(rc) after un_app(); videoout shutdown drains flip events before unregistering buffers. |
| 31 | L | Companion data had to be collected by hand. | The pkg deliberately contains no game data. | build_ps4.cmd builds JillEX-data.zip (jill.ini, games/, profiles/, README). |

## 9. Phases

**Phase 0 — portability + build (this change)**
* `PS4_PORT_PLAN.md`.
* `platform_paths` root + `chdir("/data/JillEX")` + PS4 `jill.ini`.
* `Screen` seam; PC implementation moved out of `app.cpp` unchanged.
* PS4 GLES2/EGL `Screen` using precompiled `texmap` shaders.
* SDL 2.0.9 hint guard and float fixes.
* `build_ps4.cmd` that compiles, links and makes `eboot.bin`.
* PC builds re-verified.

**Phase 1 — boot on hardware**
* First boot: EGL/Piglet init, 320x200 quad, audio device, pad input.
* Logging to `/data/JillEX/jill.log`; data-root creation and write test.
* Confirm the pad drives the menu, gameplay, text pages and pause menu.

**Phase 2 — pad-only UI**
* Noisemaker pad controls, hiscore letter picker, hide keyboard-only rows,
  hide VIDEO/MIDI rows, hide F-key hints.
* Verify all screens can be reached and left with the pad.

**Phase 3 — polish**
* Optional on-screen scale mode (stretch vs integer fit) and a CRT/smooth
  decision (CPU filter vs precompiled shader).
* Reduce the Piglet memory reservation from the sample's known-good values.
* Tighten load times and add a `--check-data` validation mode.
* Tune frame pacing after the first on-device run (vsync on vs software pacer).

**Phase 4 — optional**
* TinySoundFont GM music option (user-supplied SF2), if wanted.
* PC GL renderer reuse behind the same `Screen` interface.

## 10. On-device test checklist

1. `eboot.bin` boots to the splash/menu; log file exists.
2. `START GAME` loads `/data/JillEX/games/jill1.game`, `profiles/`, `jillt/`.
3. Board 1 renders with correct palette and sprites at 5x integer scale.
4. Pad: D-pad/stick moves, Cross jumps, Square/whatever throws, Options pauses.
5. Save to slot 0, quit, relaunch, `RESTORE GAME` reads the save.
6. High score is written to `/data/JillEX/hiscore_jill1.txt`.
7. Text pages (F1 equivalent) scroll and close with the pad.
8. Music (OPL2/CMF) and SFX both audible; volume options work.
9. Onesimus loads (`ONESIM/`, uppercase names) and renders.
10. External pack mount from `external/` works (including a `.zip`).
11. No crash when `jill.ini` contains a stale Windows `D:/...` path.
12. 30 minutes of play: no memory growth, no audio underruns.

## 11. Decisions needed from the owner

1. **PKG or loose eboot?** Both are now produced by `build_ps4.cmd`.  Confirm
   `TITLE_ID=BREW00199` / content id are acceptable (change them near the top of
   the script before a public release if they collide).
2. **Output scaling:** integer-fit with pillarboxing (crisp, black bars) or
   stretch to 16:9 (fills the screen, distorts 8:5 art)?
3. **Music on PS4:** OPL2 only (recommended) or also a GM/SoundFont option?
4. **Bedrock data layout:** ship empty `jillt/`/`ONESIM/` folders with a
   README, or have the game prompt the user to copy data on first run?
5. **Legal review:** confirm the PolyForm + LGPL/Piglet notices text before
   distributing any PKG.
