# Jill-EX — Android target

Gradle + Android NDK build of the same `port/src` engine used by the desktop,
PS4 and Vita ports.  SDL2 is compiled from the installed source tree and owns
the window, GLES renderer, audio, lifecycle and gamepad support; the engine
adds an on-screen touch pad in `port/src/touch.cpp`.

```
port/android/
  build.gradle, settings.gradle, gradle.properties
  local.properties            machine paths (git-ignored)
  gradlew(.bat), gradle/      wrapper (Gradle 8.14.7)
  jill.ini                    defaults installed into the app data folder
  ANDROID-DATA-README.txt     user data instructions (installed as README.txt)
  app/
    build.gradle              applicationId, SDK/NDK, CMake args
    src/main/AndroidManifest.xml
    src/main/java/org/jillex/JillEXActivity.java
    src/main/java/org/libsdl/app/     SDL2 Java glue (2.32.10)
    src/main/jni/CMakeLists.txt       engine + SDL2 build
    src/main/res/             launcher icon, strings, fullscreen theme
    src/main/assets/          generated at build time (git-ignored)
```

## Prerequisites

From `TOOL_LOCATIONS.md`:

| Component | Path |
| --- | --- |
| Android SDK | `D:\Apps\AndroidSDK` |
| JDK 17 (`JAVA_HOME`) | `D:\Apps\AndroidSDK\jdk-17` |
| NDK | `D:\Apps\AndroidSDK\ndk\27.3.13750724` |
| SDL2 source | `D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10` |

`local.properties` is machine-specific:

```properties
sdk.dir=D\:\\Apps\\AndroidSDK
sdl2.dir=D\:\\Apps\\AndroidSDK\\SDL2\\SDL2-2.32.10
```

Alternatively set `ANDROID_HOME` and `SDL2_ANDROID` in the environment.  The
Gradle wrapper is cached locally (`~/.gradle/wrapper/dists`); the build does
not need network access once AGP 8.7.3 and the wrapper are present.

## Build

```
cmd /c build_android.cmd          :: assembleDebug + copy the APK
cmd /c build_android.cmd install  :: ... and adb install -r
cmd /c build_android.cmd clean    :: remove app/build and app/.cxx
```

Outputs:

| File | What it is |
| --- | --- |
| `port/android/app/build/outputs/apk/debug/app-debug.apk` | installable debug APK |
| `Releases/JillEX-android.apk` | the same APK copied to the git-root `Releases/` (git-ignored) |

The script stages `jill.ini`, `games/*.game`, `profiles/*.profile|*.pal`,
`README.txt` and `DISCLAIMER.txt` into `app/src/main/assets/` before Gradle
runs.  That folder is generated and ignored by git.

Equivalent manual commands:

```
set JAVA_HOME=D:\Apps\AndroidSDK\jdk-17
cd port\android
gradlew.bat assembleDebug
```

First build compiles SDL2 and the engine for four ABIs (`arm64-v8a`,
`armeabi-v7a`, `x86_64`, `x86`), which covers modern phones, older Fire
tablets, Intel Fire tablets and emulators.  Expect 10–15 minutes the first
time and ~6–20 s incrementally.  For a quick single-ABI build:

```
cd port\android
gradlew.bat assembleDebug -PjillAbis=arm64-v8a
```

The native libraries are linked with 16 KiB LOAD-segment alignment
(`-Wl,-z,max-page-size=16384`), which Android 15+ / 16 KB-page devices require.
Without it, installation fails with "16kb load segment alignment".

## Data and paths

`JILL_ANDROID` selects the Android data root in `platform_paths.cpp`:

```
/sdcard/JillEX        (shared storage; visible to adb and file managers)
```

`JillEXActivity` copies the bundled configuration there on first launch and
never overwrites `jill.ini` afterwards (the engine rewrites it on exit).  The
folder is outside the app sandbox, so on first launch the app opens the
"All files access" settings page (`MANAGE_EXTERNAL_STORAGE`, API 30+).  On
Android 10 and older it requests the legacy storage permission instead; the
screen blocks SDL until the grant is present.  An adb alternative:

```
adb shell appops set org.jillex MANAGE_EXTERNAL_STORAGE allow
```

Game data is not bundled:

```
adb push jillt  /sdcard/JillEX/
adb push ONESIM /sdcard/JillEX/
```

Saves, high scores and `jill.log` are written to the same folder.  If startup
fails (missing game files, bad data root), the app keeps the screen up with the
error, the `adb push` commands and the tail of `jill.log`, plus a RETRY button.
Back exits normally.

`trace()` also goes to logcat under the `JillEX` tag:

```
adb logcat -d -s JillEX SDL AndroidRuntime libc
```

The first lines name the data folder, the installed games, the SDL video
driver and any native signal (`fatal: native signal N`).

## Input

* On-screen overlay: D-pad (left), A = jump, X = throw (right), BACK and START
  at the top corners.  It is drawn into the 320x200 ARGB frame and hit-tested
  through `Screen::touch_to_frame()`, so it scales and letterboxes exactly with
  the game.
* The positions are adjustable: **OPTIONS > TOUCH CONTROLS** puts the overlay
  into drag mode (grab any button, move it; RESET restores the defaults; DONE
  or pad A/B saves).  Positions are stored in `jill.ini` under `[touch]`, as
  centres in 320x200 frame pixels.
* SDL's renderer already maps finger/mouse events into the logical 320x200
  space once `SDL_RenderSetLogicalSize` is active; `touch_to_frame()` must not
  letterbox them a second time.  It previously did, which pushed the hit zones
  away from the drawn buttons on phones (tall 20:9 screens) while 16:10
  tablets happened to line up.
* Touch buttons feed `poll_input()` directly for gameplay and synthesise
  `SDL_CONTROLLERBUTTONDOWN/UP` events for the existing menu, pause, text-page
  and demo code.
* Bluetooth/USB pads keep working through SDL's controller layer (A jump,
  X throw, Start pause, D-pad/stick move).

Known gaps (plan Milestone C): hi-score name entry only accepts the default
name on touch, the Noisemaker QWERTY page needs a keyboard, and the text pages
use the D-pad zones for scrolling.

## Platform notes

| Piece | Behaviour |
| --- | --- |
| `screen_sdl.cpp` | SDL window + GLES renderer, `SDL_RenderSetLogicalSize(320,200)` with integer scaling and black letterbox |
| `touch.cpp` | overlay drawing, finger tracking, hit zones, synthetic controller events |
| `platform_paths.cpp` | external/internal app storage root, logcat tracing |
| `main.cpp` | console-style bootstrap (create root, `chdir`, `jill.log`), no PS4 `_exit` |
| SDL2 | built from source as `libSDL2.so`; `main.cpp` is compiled with `main=SDL_main` so `SDLActivity` finds `SDL_main` in `libmain.so` |
| Java | `JillEXActivity extends SDLActivity`: install assets, keep screen on, immersive sticky fullscreen |

Default ABIs: `arm64-v8a`, `armeabi-v7a`, `x86_64`, `x86`.  `minSdk` is 24,
`targetSdk` 35.  Override with `-PjillAbis=arm64-v8a` for a smaller/faster
build.
