@echo off
setlocal enabledelayedexpansion

rem Jill-EX original Xbox build (NXDK + NXDK's SDL2 port).
rem
rem Independent of build.cmd / build32.cmd / build_ps4.cmd / build_vita.cmd: it
rem never touches their outputs.  It produces
rem   port\xbox\build\default.xbe        the game executable
rem   port\xbox\build\data\             jill.ini, games/, profiles/ (for the disc)
rem   JillEX-Xbox-data.zip               the same data, zipped (repo root)
rem
rem Usage:  build_xbox.cmd            compile + link + default.xbe + data
rem         build_xbox.cmd clean      remove port\xbox\build
rem
rem Requirements: NXDK (NXDK_DIR, default D:\Apps\NXDK) with its SDL2 sources,
rem CMake and Ninja.  NXDK's prebuilt lib\libc++.lib / lib\libSDL2.lib are NOT
rem required: this project builds both from the SDK's checked-out sources.

set ROOT=%~dp0
set PORT=%ROOT%port
set XBOX=%PORT%\xbox
set OUT=%XBOX%\build

if /I "%~1"=="clean" (
    if exist "%OUT%" rmdir /s /q "%OUT%"
    echo Cleaned %OUT%
    exit /b 0
)

rem --- toolchain ------------------------------------------------------------
if "%NXDK_DIR%"=="" set NXDK_DIR=D:\Apps\NXDK
if not exist "%NXDK_DIR%\share\toolchain-nxdk.cmake" (
    echo Cannot find NXDK at "%NXDK_DIR%".
    echo Set NXDK_DIR to the nxdk checkout ^(share\toolchain-nxdk.cmake is missing^).
    exit /b 1
)
if not exist "%NXDK_DIR%\bin\nxdk-cxx.cmd" (
    echo "%NXDK_DIR%\bin\nxdk-cxx.cmd" is missing; the NXDK install looks incomplete.
    exit /b 1
)
if not exist "%NXDK_DIR%\lib\sdl\SDL2\Makefile.xbox" (
    echo The NXDK checkout has no SDL2 sources at lib\sdl\SDL2.
    exit /b 1
)
if not exist "%NXDK_DIR%\lib\libcxx\Makefile.nxdk" (
    echo The NXDK checkout has no libc++ sources at lib\libcxx.
    exit /b 1
)
if not exist "%NXDK_DIR%\tools\cxbe\cxbe.exe" (
    echo "%NXDK_DIR%\tools\cxbe\cxbe.exe" is missing; build NXDK's tools first.
    exit /b 1
)

where cmake >nul 2>nul
if errorlevel 1 set PATH=C:\Program Files\CMake\bin;%PATH%
where cmake >nul 2>nul
if errorlevel 1 (
    echo cmake was not found. Install CMake and put it on PATH.
    exit /b 1
)

rem Ninja is a host tool.  NXDK has no CMake generator of its own and the
rem default Visual Studio generator cannot cross-compile with nxdk-cxx, so a
rem single-config generator is required.
set MAKE_PROGRAM=
where ninja >nul 2>nul
if not errorlevel 1 set MAKE_PROGRAM=ninja
if "%MAKE_PROGRAM%"=="" (
    for %%d in (
        "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
        "%ProgramFiles%\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
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
echo [cmake] configuring (NXDK_DIR=%NXDK_DIR%)
rem CMake only reads CMAKE_TOOLCHAIN_FILE on the first configure; passing it
rem again would print an "unused variable" warning on every rebuild.
if exist "%OUT%\CMakeCache.txt" goto :configure_cached
cmake -S "%XBOX%" -B "%OUT%" -G Ninja -DCMAKE_MAKE_PROGRAM="%MAKE_PROGRAM%" -DCMAKE_TOOLCHAIN_FILE="%XBOX%\nxdk.toolchain.cmake" -DCMAKE_BUILD_TYPE=Release -DNXDK_DIR="%NXDK_DIR%"
if errorlevel 1 goto :fail
goto :configured

:configure_cached
cmake -S "%XBOX%" -B "%OUT%" -DCMAKE_BUILD_TYPE=Release -DNXDK_DIR="%NXDK_DIR%"
if errorlevel 1 goto :fail

:configured

echo [cmake] building
cmake --build "%OUT%" --parallel
if errorlevel 1 goto :fail

if not exist "%OUT%\default.xbe" (
    echo default.xbe was not produced.
    goto :fail
)

rem --- install bundle --------------------------------------------------------
rem The Xbox build reads default.xbe, jill.ini, games/, profiles/ and the
rem extracted game folders from the folder the executable is launched from.
echo [bundle] staging default.xbe, jill.ini, games, profiles, external
set DATADIR=%OUT%\data
if exist "%DATADIR%" rmdir /s /q "%DATADIR%"
mkdir "%DATADIR%\games"
mkdir "%DATADIR%\profiles"
mkdir "%DATADIR%\external"
copy /y "%OUT%\default.xbe" "%DATADIR%\default.xbe" >nul
copy /y "%XBOX%\jill.ini" "%DATADIR%\jill.ini" >nul
copy /y "%ROOT%games\*.game" "%DATADIR%\games\" >nul
copy /y "%ROOT%profiles\*.profile" "%DATADIR%\profiles\" >nul
copy /y "%ROOT%profiles\*.pal" "%DATADIR%\profiles\" >nul
if exist "%XBOX%\XBOX-DATA-README.txt" copy /y "%XBOX%\XBOX-DATA-README.txt" "%DATADIR%\README.txt" >nul
if exist "%ROOT%DISCLAIMER.txt" copy /y "%ROOT%DISCLAIMER.txt" "%DATADIR%\" >nul

echo [bundle] JillEX-Xbox.zip
if exist "%OUT%\JillEX-Xbox.zip" del /q "%OUT%\JillEX-Xbox.zip"
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%DATADIR%\*' -DestinationPath '%OUT%\JillEX-Xbox.zip' -Force"
if errorlevel 1 goto :fail
copy /y "%OUT%\JillEX-Xbox.zip" "%ROOT%" >nul

echo.
echo Built %OUT%\default.xbe
echo Built %OUT%\JillEX-Xbox.zip (install bundle, copy at %ROOT%JillEX-Xbox.zip)
echo.
echo Deploying:
echo   - Extract JillEX-Xbox.zip into one folder on the console ^(for example
echo     E:\Apps\Jill-EX\^) and add your own extracted jillt\ / ONESIM\
echo     folders there, or burn the same layout as an XISO.
echo   - The zip already contains default.xbe, jill.ini, games\, profiles\,
echo     external\ and README.txt.
echo   - The data root is the folder the XBE is launched from; if it has no
echo     jill.ini, the first of D:\, E:\JillEX, E:\Apps\Jill-EX,
echo     E:\Apps\JillEX, E:\Games\Jill-EX, E:\Games\JillEX that does is used
echo     ^(D:\ is the disc root^).  See port\xbox\README.md.
exit /b 0

:fail
echo.
echo Xbox build failed.
exit /b 1
