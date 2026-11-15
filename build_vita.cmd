@echo off
setlocal enabledelayedexpansion

rem Jill-EX PS Vita build (VitaSDK + SDL2 + libvita2d).
rem
rem This script is independent of build.cmd/build32.cmd and build_ps4.cmd: it
rem never touches the desktop or PS4 outputs.  It produces
rem   port\vita\build\jill.vpk     installable bubble
rem   port\vita\build\jill.self    the SELF inside it
rem   port\vita\build\data\        companion data (jill.ini, games, profiles)
rem   JillEX-Vita-data.zip         the same data, zipped (repo root)
rem
rem Usage:  build_vita.cmd            compile + link + vpk + data
rem         build_vita.cmd clean      remove port\vita\build
rem
rem Requirements: VitaSDK (vdpm bootstrap), the sdl2 package, CMake and Ninja.

set ROOT=%~dp0
set PORT=%ROOT%port
set VITA=%PORT%\vita
set OUT=%VITA%\build

if /I "%~1"=="clean" (
    if exist "%OUT%" rmdir /s /q "%OUT%"
    echo Cleaned %OUT%
    exit /b 0
)

rem --- toolchain ------------------------------------------------------------
if "%VITASDK%"=="" set VITASDK=C:\vitasdk
if not exist "%VITASDK%\bin\arm-vita-eabi-gcc.exe" (
    echo Cannot find VitaSDK at "%VITASDK%".
    echo Install it with the vdpm bootstrap, or set VITASDK to the SDK root.
    exit /b 1
)
if not exist "%VITASDK%\share\vita.toolchain.cmake" (
    echo "%VITASDK%\share\vita.toolchain.cmake" is missing; the VitaSDK install looks incomplete.
    exit /b 1
)
if not exist "%VITASDK%\arm-vita-eabi\lib\libSDL2.a" (
    echo SDL2 is not installed in the SDK.  Run:  vdpm install sdl2
    exit /b 1
)
if not exist "%VITASDK%\arm-vita-eabi\lib\libvita2d.a" (
    echo libvita2d is not installed in the SDK.  Run:  vdpm install libvita2d
    exit /b 1
)

where cmake >nul 2>nul
if errorlevel 1 set PATH=C:\Program Files\CMake\bin;%PATH%
where cmake >nul 2>nul
if errorlevel 1 (
    echo cmake was not found. Install CMake and put it on PATH.
    exit /b 1
)

rem Ninja is a host tool, not a VitaSDK package: CMake's default Visual Studio
rem generator cannot cross-compile with arm-vita-eabi-g++, so it is required.
rem Visual Studio ships a copy under its CMake folder.
set MAKE_PROGRAM=
where ninja >nul 2>nul
if not errorlevel 1 set MAKE_PROGRAM=ninja
if "%MAKE_PROGRAM%"=="" (
    for %%d in (
        "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
    ) do (
        if exist "%%~d\ninja.exe" set MAKE_PROGRAM=%%~d\ninja.exe
    )
)
if "%MAKE_PROGRAM%"=="" (
    echo Ninja was not found.  Install it ^(winget install Ninja-build.Ninja^)
    echo or add the Ninja folder from Visual Studio to PATH.
    exit /b 1
)

rem --- configure + build ----------------------------------------------------
if not exist "%OUT%" mkdir "%OUT%"
echo [cmake] configuring
cmake -S "%VITA%" -B "%OUT%" -G Ninja -DCMAKE_MAKE_PROGRAM="%MAKE_PROGRAM%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :fail

echo [cmake] building
cmake --build "%OUT%" --parallel
if errorlevel 1 goto :fail

rem --- companion data -------------------------------------------------------
echo [data] staging games/profiles/jill.ini
set DATADIR=%OUT%\data
if exist "%DATADIR%" rmdir /s /q "%DATADIR%"
mkdir "%DATADIR%\games"
mkdir "%DATADIR%\profiles"
mkdir "%DATADIR%\external"
copy /y "%VITA%\jill.ini" "%DATADIR%\jill.ini" >nul
copy /y "%ROOT%games\*.game" "%DATADIR%\games\" >nul
copy /y "%ROOT%profiles\*.profile" "%DATADIR%\profiles\" >nul
copy /y "%ROOT%profiles\*.pal" "%DATADIR%\profiles\" >nul
if exist "%VITA%\VITA-DATA-README.txt" copy /y "%VITA%\VITA-DATA-README.txt" "%DATADIR%\README.txt" >nul
if exist "%ROOT%DISCLAIMER.txt" copy /y "%ROOT%DISCLAIMER.txt" "%DATADIR%\" >nul

echo [data] JillEX-Vita-data.zip
if exist "%OUT%\JillEX-Vita-data.zip" del /q "%OUT%\JillEX-Vita-data.zip"
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%DATADIR%\*' -DestinationPath '%OUT%\JillEX-Vita-data.zip' -Force"
if errorlevel 1 goto :fail
echo Made %OUT%\JillEX-Vita-data.zip

echo.
echo Built %OUT%\jill.vpk
echo Built %OUT%\JillEX-Vita-data.zip
echo.
echo Installing:
echo   - VPK: copy jill.vpk to the console ^(VitaShell/FTP^) and install it.
echo   - Data: copy the contents of JillEX-Vita-data.zip AND your own extracted
echo     jillt\ and ONESIM\ folders into ux0:data/JillEX\.
echo     Expected layout: ux0:data/JillEX/jill.ini, games\, profiles\, external\,
echo     jillt\, ONESIM\  (saves, high scores and jill.log are written there).
exit /b 0

:fail
echo.
echo Build failed.
exit /b 1
