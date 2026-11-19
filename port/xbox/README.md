# Jill-EX for the original Xbox (NXDK)

The Xbox target builds the shared `port/src` engine with NXDK's toolchain and
its SDL2 port, and produces a plain `default.xbe`.

**Status: builds, not yet run on hardware.**  The engine, SDL2 2.0.9 (Xbox
video/audio/joystick drivers) and libc++ all compile and link; the linked PE is
packed into a valid XBE.  Nothing below has been verified on a console or in
Cxbx/XQEMU yet.

## Requirements

* NXDK checkout with sources at `D:\Apps\NXDK` (or set `NXDK_DIR`).  It must
  include `lib/sdl/SDL2` and `lib/libcxx`, and the prebuilt build tools
  (`tools/cxbe/cxbe.exe`).
* CMake 3.20+ and Ninja.  NXDK has no CMake generator of its own and the
  Visual Studio generator cannot cross-compile with `nxdk-cxx`.

NXDK's prebuilt `lib/libc++.lib` and `lib/libSDL2.lib` are **not** needed: this
project compiles both from the SDK's checked-out sources, because a source
checkout does not ship them and NXDK's Makefiles refuse to run outside a
MinGW shell.

## Build

```
cmd /c build_xbox.cmd
```

Outputs:

```
port\xbox\build\default.xbe        the game executable
port\xbox\build\data\              default.xbe + jill.ini, games\, profiles\, external\
JillEX-Xbox.zip                    the same folder, zipped (repo root)
```

`build_xbox.cmd clean` removes `port\xbox\build`.

The first build is ~1-2 minutes (SDL2 and libc++ are compiled from source);
incremental builds are a few seconds.

## Deploy

`JillEX-Xbox.zip` is the whole install: extract it into one folder (for
example `E:\Apps\Jill-EX\`), add your own extracted `jillt\` / `ONESIM\`
folders, and run `default.xbe`.  Burning the same layout as an XISO works too.
See `XBOX-DATA-README.txt`.

The data root is the folder the XBE is launched from (`XeImageFileName`), so
the game works from a disc, an HDD folder or an FTP deployment.  If that
folder has no `jill.ini`, the first of these that does is used:

```
D:\  E:\JillEX  E:\Apps\Jill-EX  E:\Apps\JillEX  E:\Games\Jill-EX  E:\Games\JillEX
```

(`D:\` is the disc root.)  A writable root is needed for save games, high
scores and `jill.log`; a burned disc is read-only, so use the hard disk for
those.

## What the port changes

* `port/xbox/CMakeLists.txt` — standalone CMake project: the shared engine
  sources plus `xbox_runtime.cpp`, a libc++ static library built from
  `lib/libcxx/Makefile.nxdk`'s source list, and an SDL2 static library built
  from `lib/sdl/SDL2/Makefile.xbox`'s source groups.  A post-build step runs
  `cxbe` to write `default.xbe`.
* `port/xbox/nxdk.toolchain.cmake` — fixes NXDK's shipped CMake toolchain for
  Windows (`.cmd` launchers) and removes the non-existent prebuilt
  `libc++.lib` from the link line.
* `port/xbox/xbox_compat.h` — force-included into every C++ unit: restores
  `std::printf` (NXDK's libc++ sets `_LIBCPP_HAS_NO_STDOUT`), declares the
  `wcsnrtombs`/`mbsnrtowcs` helpers libc++'s `<locale>` needs, and pins the
  pdclib Annex K guard macro.
* `port/xbox/xbox_runtime.cpp` — `std::terminate`, `uncaught_exception(s)` and
  `__call_once`, because NXDK's `libxboxrt` exception object needs MSVC EH
  machinery a `-fno-exceptions` build cannot link.  The engine never throws.
* `port/xbox/xbox_fs_glue.cpp` — replaces pdclib's `_PDCLIB_open` (the funnel
  behind `fopen`/`ifstream`/`ofstream`) so `'/'` becomes `'\'`.  NXDK's
  `CreateFileA` goes straight to `NtCreateFile`, which only accepts
  backslashes, while `jill.ini`, pack manifests and the engine's `join()`
  produce `'/'`.
* `port/src/platform_fs.{h,cpp}` — small `jill_fs` facade.  Desktop, PS4, Vita
  and Android alias `std::filesystem`; Xbox uses a Win32-backed implementation
  (`FindFirstFileA` / `GetFileAttributesA` / `CreateDirectoryA`), because
  libc++'s `<filesystem>` implementation is POSIX-only and NXDK does not
  compile it.
* `port/src/platform_except.h` — `JILL_TRY`/`JILL_CATCH` compile the three
  defensive try/catch blocks as plain blocks on Xbox.
* `port/src/platform_paths.cpp` — Xbox data root detection (`XeImageFileName`
  first, then the known HDD roots), and `trace()` writes to the debug channel
  (`debugPrint`) plus `jill.log` instead of stdio.
* `port/src/main.cpp` — console paths, no `chdir`, local `--music-seconds`
  parser (pdclib declares but does not implement `atof`/`strtod`).  On Xbox the
  log is opened before anything else and stdout/stderr are not re-opened:
  pdclib's stdout/stderr are pseudo streams with `INVALID_HANDLE_VALUE`.
* Link step: `-stack:1048576`.  NXDK's CRT runs `main()` on a thread whose
  stack size comes from the XBE header and defaults to 64 KiB, which overflows
  immediately (the console drops back to the dashboard before the first log
  line).
* `port/src/screen_sdl.cpp` — Xbox joins the PS4 "write the scaled frame into
  the window surface" path; the SDL2 Xbox port has no accelerated renderer.
* `port/src/app.cpp` — Xbox sets the video mode before SDL init, because SDL's
  Xbox driver reads `XVideoGetMode()` (still 0x0 until `XVideoSetMode()` runs),
  and initialises SDL one subsystem at a time so failures are visible in the
  log.
* `port/src/midi.cpp`, `port/src/extaudio.cpp` — the Win32 MIDI and Media
  Foundation backends are disabled on Xbox (no `winmm`/MF).
* `port/src/external.cpp`, `port/src/extaudio.cpp` — `STBI_NO_STDIO` /
  `DR_MP3_NO_STDIO` (memory decoding only; NXDK has no `fopen_s`), and
  `ifstream` takes the path as a string.

## Compile-time decisions worth knowing

* The Xbox CRT has **no C++ unwinder**, so the build uses `-fno-exceptions`
  and `port/src/platform_except.h`.
* libc++'s `random`, `exception`, `future`, `thread`, `condition_variable`,
  `mutex` and `shared_mutex` units are excluded: they select MSVC/Win32
  backends (`rand_s`, `exception_ptr`, `_MSC_VER` EH) that NXDK does not
  implement, and the engine uses none of them.
* `MINIZ_NO_STDIO`/`MINIZ_NO_TIME` and `STBI_NO_SIMD` avoid POSIX headers and
  SSE2 (the target profile is Pentium III).
* SDL2 and libc++ are compiled with the SDK's own flags; only their source
  lists come from NXDK's makefiles.

## Next steps (runtime bring-up)

1. Boot in Cxbx-Reloaded or on hardware and read `jill.log` next to the XBE
   (it is opened first; the same lines go to the debug output).  Expected
   order: `startup: data root ...`, `startup: log ready`, `startup: games=...`,
   then the SDL lines (`sdl screen: video mode 640x480`).
2. Confirm the 320x200 frame is presented (letterboxed at 2x in 640x480).
3. Verify pad input (`pad:` trace lines) and the X/A mappings.
4. Verify audio: SDL2's Xbox audio driver is the `xbox` backend (direct sound
   through `libxboxrt`); check music (OPL2) and effects.
5. Wire saves/high scores to a writable volume if the data root is the disc.
6. Watch for missing libc++ pieces on first run (iostream/fstream paths are
   only exercised once jill.ini and packs are read).
