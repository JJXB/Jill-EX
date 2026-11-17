# Jill-EX Android port plan (Android SDK/NDK + SDL2)

Status: **planning + toolchain provisioning** (2026-11-16)
Target: Android phone/tablet, API 21+ (recommended API 24+), arm64-v8a (optional armeabi-v7a/x86_64)
Engine: the same `port/src` C++17 sources used by desktop, PS4 and Vita.
Toolchain root: `D:\Apps\AndroidSDK`

## 0. Provisioned toolchain (this step)

| Component | Version | Installed under |
| --- | --- | --- |
| Temurin JDK | 17.0.21+9 | `D:\Apps\AndroidSDK\jdk-17` |
| Android command-line tools | 9862592 | `D:\Apps\AndroidSDK\cmdline-tools\latest` |
| Android platform-tools | latest | `D:\Apps\AndroidSDK\platform-tools` |
| Android platform | `android-35` | `D:\Apps\AndroidSDK\platforms\android-35` |
| Android build-tools | `35.0.1` | `D:\Apps\AndroidSDK\build-tools\35.0.1` |
| Android NDK | `27.3.13750724` | `D:\Apps\AndroidSDK\ndk\27.3.13750724` |
| CMake (SDK) | `3.22.1` | `D:\Apps\AndroidSDK\cmake\3.22.1` |
| Gradle | `8.14.7` | `D:\Apps\AndroidSDK\gradle\gradle-8.14.7` |
| SDL2 source (Android project) | `2.32.10` | `D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10` |

Environment variables set at user scope:
`ANDROID_HOME` / `ANDROID_SDK_ROOT` = `D:\Apps\AndroidSDK`,
`JAVA_HOME` = `D:\Apps\AndroidSDK\jdk-17`, and PATH entries for the JDK,
sdkmanager, platform-tools (adb), Gradle and NDK binaries.

The machine's pre-existing Java 25 stays installed; the Android build pins JDK 17
because Android Gradle Plugin 8.x does not support Java 25.

## 1. Goal and constraints

One source tree (`port/src`, `port/games`, `port/profiles` via the repo roots)
builds for Android without changing gameplay logic. New platform code is either
compiled only for Android (`JILL_ANDROID`) or is a no-op elsewhere. PC, PS4 and
Vita builds must keep working.

| Target | Build system | Display | Status |
| --- | --- | --- | --- |
| Windows x64/x86 | `build.cmd`, CMake + MSVC | SDL2 SDL_Renderer | unchanged |
| PS4 / Vita | `build_ps4.cmd`, `build_vita.cmd` | GLES / libvita2d | unchanged |
| Android (new) | Gradle + CMake via NDK | SDL2 video (EGL/ANGLE) | this plan |

## 2. Why this is straightforward

* The engine renders into a fixed **320x200 8-bit indexed framebuffer**, converted
  to an ARGB texture. `port/src/screen.h` already defines a `Screen` seam with
  `create_screen_sdl()`, `create_screen_gl()`, `create_screen_videoout()` and
  `create_screen_vita2d()`. SDL2's Android video driver can reuse
  `screen_sdl.cpp` directly; a `create_screen_android()` is only needed if the
  desktop code needs guarding.
* SDL2 2.32.10 ships an official `android-project/` Gradle template and handles
  the Android window, EGL context, audio (AAudio/OpenSL ES), lifecycle events,
  gamepads and touch events.
* `port/src/platform_paths.{h,cpp}` already switches a writable data root per
  target. Adding a `JILL_ANDROID` branch is a few lines.
* `main.cpp` already has the console data-root bootstrap, config resolution and
  diagnostic logging pattern; Android needs an entry-point shim and logcat.

## 3. Project layout to create

```
port/android/
  build.gradle              root Gradle build (AGP 8.x, SDK 35, NDK 27)
  settings.gradle           include :app, :SDL2
  gradle.properties
  gradle/wrapper/           gradle-wrapper.jar + properties (Gradle 8.14.7)
  app/
    build.gradle            applicationId, ABIs, CMake args, SDL2 dependency
    proguard-rules.pro
    src/main/
      AndroidManifest.xml   orientation, features, native activity
      java/.../JillEXActivity.java   extends SDLActivity (from SDL2 template)
      res/                  icon + values (strings, styles)
      jni/
        CMakeLists.txt      reuses ../../../src/*.cpp + third_party/*.c
        src/main.cpp        thin SDL_main wrapper -> existing run_app()
      assets/               (optional) initial jill.ini / games / profiles
  SDL2/                     symlink/copy of D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10
                            (Android library module from SDL2's android-project)
```

The `jni/CMakeLists.txt` mirrors `port/vita/CMakeLists.txt`: every `port/src`
translation unit, `JILL_ANDROID=1`, `JILL_HAVE_SDL=1`, and the third-party C
files. Do **not** define `SDL_MAIN_HANDLED` for Android — SDL's native activity
calls `SDL_main`. If keeping the existing `main()` is preferred, add a small
`SDL_main` shim that forwards to it.

## 4. Display and presentation

* Start with `screen_sdl.cpp` unchanged. On Android SDL creates a full-screen
  EGL window; `SDL_RenderSetLogicalSize(320,200)` plus integer scale and the
  existing ARGB streaming texture give the same output as desktop.
* Guard desktop-only calls that are meaningless on a phone:
  `--windowed/--fullscreen/--borderless`, `WINDOW TYPE/SIZE/RESOLUTION` rows,
  window title, and `SDL_GetDesktopDisplayMode` assumptions. Make them no-ops
  or hide the rows, exactly as the Vita/PS4 builds do.
* Respect Android display cutouts/rotation: lock landscape and letterbox the
  8:5 frame. Offer integer-fit vs stretch-to-fill as a video option later.
* `Engine` is ~72 KiB per object; allocate on the heap, not the native thread
  stack (the Vita port learned this the hard way).

## 5. Data and paths

* `jill_paths::data_root()` returns an Android-writable directory, e.g. the
  native activity's `internalDataPath` or `SDL_GetPrefPath("JillEX")`. Mirror
  the console layout under it: `jill.ini`, `games/`, `profiles/`, `external/`,
  `jillt/`, `ONESIM/`, save slots, high scores and `jill.log` (or logcat).
* No game data is shipped (project licence / Epic trademarks). First run must
  explain how to get legally obtained `jillt/` and `ONESIM/` onto the device:
  ADB push, or an in-app Android Storage Access Framework importer. Ship a
  companion README and a default `jill.ini` with relative asset paths.
* `jill.ini` is rewritten on exit. On Android also flush it on
  `SDL_APP_WILLENTERBACKGROUND` / `SDL_APP_TERMINATING` so a backgrounded app
  does not lose settings or saves.

## 6. Input — the main work item

The game currently listens only to SDL keyboard events, game-controller
buttons, joystick and mouse buttons. There is **no touch handling**.

* **Milestone A — gamepad only.** SDL 2.32 handles USB/Bluetooth controllers and
  the existing menu navigation already listens to controller buttons. A pad is
  enough for a fully playable build with almost no new input code. Verify pad
  mapping (A=jump, X=throw, Start=pause, D-pad=move) on a real device.
* **Milestone B — touch controls.** Add an on-screen overlay (SDL touch events
  or a Java-side view) that maps zones/buttons to the existing actions, either
  by pushing SDL events or by feeding `poll_input()` in `app.cpp`. Minimum set:
  D-pad, jump, throw, Start/pause, Back.
* **Milestone C — touch-only UI.** Pad/keyboard assumptions to fix:
  hi-score name entry, Noisemaker (QWERTY grid), CONTROLS rebinding capture,
  F1-F5/F11 debug hints, text-page scrolling. Use the PS4 plan's Phase 2 list
  as the checklist.

## 7. Audio, MIDI and lifecycle

* Audio: reuse the existing `audio.cpp` SDL_OpenAudioDevice callback. SDL's
  Android driver selects AAudio/OpenSL ES; set a small buffer in `SDL_AudioSpec`
  for low latency. Keep OPL2/CMF as the default music path (pure software).
* MIDI OUT: WinMM does not exist; `midi.cpp` already stubs an empty device list.
  Hide the `MIDI OUT` row. FluidSynth is deliberately **not** a dependency (see
  PS4 plan §6); TinySoundFont remains the optional future GM option.
* Lifecycle: handle SDL app events — save/flush on background, reopen/resume
  audio and presentation on foreground, survive low-memory events, keep the
  native thread alive across activity restarts.
* External decoder gap: Media Foundation M4A/AAC (`extaudio.cpp`) is Windows
  only; document WAV/OGG/MP3 as the supported external-pack audio formats.

## 8. Build and packaging

1. Gradle wrapper uses the installed Gradle 8.14.7; `local.properties` points
   `sdk.dir=D\:\\Apps\\AndroidSDK`.
2. Build a debug APK first (`gradlew :app:assembleDebug`), install with
   `adb install -r`, watch `adb logcat` for the `jill_paths::trace()` output.
3. Produce a signed release APK/AAB only after the pad/touch milestones pass.
4. Licence: bundle PolyForm Noncommercial notice, SDL2 (zlib) and the existing
   `THIRD_PARTY_NOTICES.md`. Do not package original game art/data; the app
   icon is the only artwork, kept separate from the game data licence.

## 9. Phases

**Phase 0 — toolchain (this step).** SDK/NDK/JDK/Gradle/SDL2 installed under
`D:\Apps\AndroidSDK`; PATH and env vars configured; `sdkmanager --version` and
`adb version` verified.

**Phase 1 — boot to a frame.** Android Gradle project compiles `port/src`;
SDL2 native activity starts; `jill.ini` and data root resolve; a 320x200 board
renders and OPL2 music plays. `adb logcat` shows startup milestones.

**Phase 2 — gamepad playable.** Pad drives menus, gameplay, text pages, pause
and save/restore. Desktop/PS4/Vita builds re-verified unchanged.

**Phase 3 — touch.** On-screen controls and touch-friendly menus; hi-score and
Noisemaker input solved; landscape/cutout/letterbox verified on real devices.

**Phase 4 — polish/distribution.** Audio latency and suspend/resume, scale vs
stretch option, `--check-data`-style validation, signed release packaging.

## 10. Risks and mitigations

| # | Issue | Mitigation |
| --- | --- | --- |
| 1 | No touch input in the engine | Milestones B/C; keep pad path as fallback |
| 2 | Java 25 too new for AGP 8.x | Pin JDK 17 in `JAVA_HOME`, set Gradle JVM |
| 3 | Case-sensitive Linux filesystem in APK assets | Validate manifest paths, document exact case (same as PS4) |
| 4 | SDL video on Android differs from desktop SDL | Reuse `screen_sdl.cpp`; only add guards if needed |
| 5 | Background/kill loses progress | Flush saves/config on SDL app background events |
| 6 | Game data acquisition on a phone | ADB instructions + optional SAF importer |
| 7 | Native library size / ABI choice | Start arm64-v8a only; add v7a/x86_64 if needed |
| 8 | Noncommercial licence vs app stores | Sideload first; review store terms before publishing |

## 11. Decisions needed from the owner

1. Touch controls in the first playable build, or gamepad-only first?
2. Minimum Android version / ABIs (API 24+ arm64-v8a recommended).
3. How users supply game data: ADB only, or an in-app SAF importer?
4. Sideloaded APK only, or also target a store listing?
