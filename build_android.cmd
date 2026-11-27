@echo off
setlocal enabledelayedexpansion

rem Jill-EX Android build (Gradle + AGP + Android NDK + SDL2 source).
rem
rem Outputs:
rem   port\android\app\build\outputs\apk\debug\app-debug.apk
rem   Releases\JillEX-android.apk              copy in the git root
rem
rem Usage:  build_android.cmd           assemble the debug APK + copy it
rem         build_android.cmd install   ... and adb install -r it
rem         build_android.cmd clean     remove the Android build outputs
rem
rem Requirements: Android SDK + NDK + JDK 17 + Gradle (see TOOL_LOCATIONS.md)
rem and the SDL2 source tree (default D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10).

set ROOT=%~dp0
set ANDROID=%ROOT%port\android
set OUT=%ANDROID%\app\build\outputs\apk\debug\app-debug.apk
set "RELEASES=%ROOT%Releases"
set "APK=%RELEASES%\JillEX-android.apk"

if /I "%~1"=="clean" (
    if exist "%ANDROID%\app\build" rmdir /s /q "%ANDROID%\app\build"
    if exist "%ANDROID%\app\.cxx" rmdir /s /q "%ANDROID%\app\.cxx"
    if exist "%ANDROID%\build" rmdir /s /q "%ANDROID%\build"
    echo Cleaned %ANDROID%
    exit /b 0
)

rem --- toolchain ------------------------------------------------------------
if "%ANDROID_HOME%"=="" set ANDROID_HOME=D:\Apps\AndroidSDK
if not exist "%ANDROID_HOME%\platform-tools\adb.exe" (
    echo Cannot find the Android SDK at "%ANDROID_HOME%".
    echo Set ANDROID_HOME, or install the SDK - see TOOL_LOCATIONS.md.
    exit /b 1
)
if "%JAVA_HOME%"=="" (
    if exist "%ANDROID_HOME%\jdk-17\bin\java.exe" (
        set JAVA_HOME=%ANDROID_HOME%\jdk-17
    )
)
if "%JAVA_HOME%"=="" (
    echo JAVA_HOME is not set and %ANDROID_HOME%\jdk-17 is missing.
    echo AGP 8.x needs JDK 17.
    exit /b 1
)
set PATH=%JAVA_HOME%\bin;%PATH%

if not exist "%ANDROID_HOME%\ndk\27.3.13750724" (
    echo Android NDK 27.3.13750724 is missing from "%ANDROID_HOME%\ndk".
    exit /b 1
)

if "%SDL2_ANDROID%"=="" (
    if exist "D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10\CMakeLists.txt" (
        set SDL2_ANDROID=D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10
    )
)
if "%SDL2_ANDROID%"=="" (
    echo SDL2 source not found. Set SDL2_ANDROID to the SDL2-2.x.y folder
    echo that contains CMakeLists.txt, or add sdl2.dir to
    echo port\android\local.properties.
    exit /b 1
)

rem --- companion assets (configuration only; never game data) ---------------
echo [assets] staging jill.ini, games and profiles
set ASSETS=%ANDROID%\app\src\main\assets
if exist "%ASSETS%" rmdir /s /q "%ASSETS%"
mkdir "%ASSETS%\games"
mkdir "%ASSETS%\profiles"
copy /y "%ANDROID%\jill.ini" "%ASSETS%\jill.ini" >nul
copy /y "%ANDROID%\ANDROID-DATA-README.txt" "%ASSETS%\README.txt" >nul
copy /y "%ROOT%games\*.game" "%ASSETS%\games\" >nul
copy /y "%ROOT%profiles\*.profile" "%ASSETS%\profiles\" >nul
copy /y "%ROOT%profiles\*.pal" "%ASSETS%\profiles\" >nul
if exist "%ROOT%DISCLAIMER.txt" copy /y "%ROOT%DISCLAIMER.txt" "%ASSETS%\" >nul

rem --- Gradle ---------------------------------------------------------------
echo [gradle] assembling debug APK
call "%ANDROID%\gradlew.bat" -p "%ANDROID%" assembleDebug
if errorlevel 1 goto :fail

if not exist "%OUT%" (
    echo Build finished but %OUT% is missing.
    exit /b 1
)
if not exist "%RELEASES%" mkdir "%RELEASES%"
if errorlevel 1 goto :fail
copy /y "%OUT%" "%APK%" >nul
if errorlevel 1 goto :fail

echo.
echo Built  %OUT%
echo Copied %APK%

if /I "%~1"=="install" (
    "%ANDROID_HOME%\platform-tools\adb.exe" install -r "%APK%"
    if errorlevel 1 goto :fail
)

echo.
echo Installing:
echo   - Universal APK: arm64-v8a, armeabi-v7a, x86_64, x86.
echo   - adb install -r JillEX-android.apk, or copy the APK to the device.
echo   - Grant "All files access" on first launch (the app opens the settings page).
echo   - Game data is not bundled.  With the app installed once, run:
echo       adb push jillt  /sdcard/JillEX/
echo       adb push ONESIM /sdcard/JillEX/
echo   - Start with a gamepad, or use the on-screen D-pad / A / X / START.
exit /b 0

:fail
echo.
echo Build failed.
exit /b 1
