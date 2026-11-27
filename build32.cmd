@echo off
rem Build the 32-bit (Win32/x86) Release port into port\build32\Release32 and
rem package it as Releases\Jill-EX-release-x86.zip.
rem
rem   build32.cmd           configure, build and package
rem   build32.cmd install   install sdl2:x86-windows first
rem
rem The 64-bit build is build.cmd.

setlocal EnableExtensions
set "ROOT=%~dp0"
set "JILL_VCPKG=%VCPKG_ROOT%"
if "%JILL_VCPKG%"=="" set "JILL_VCPKG=C:\vcpkg"

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
if not exist "%VCVARS%" (
    echo Cannot find vcvars32.bat. Edit build32.cmd.
    exit /b 1
)
call "%VCVARS%" >nul

rem vcvars sets VCPKG_ROOT to Visual Studio's bundled vcpkg.  Put the user's
rem (or the default C:\vcpkg) back so the demo-studio build links the same SDL2
rem that the game just used.
set "VCPKG_ROOT=%JILL_VCPKG%"

if /I "%~1"=="install" (
    if not exist "%JILL_VCPKG%\vcpkg.exe" (
        echo vcpkg.exe not found under %JILL_VCPKG%
        exit /b 1
    )
    "%JILL_VCPKG%\vcpkg.exe" install sdl2:x86-windows
    if errorlevel 1 exit /b 1
)

if not exist "%JILL_VCPKG%\installed\x86-windows\bin\SDL2.dll" (
    echo error: sdl2:x86-windows is not installed.
    echo        Run: build32.cmd install
    exit /b 1
)

set "TOOLCHAIN="
if exist "%JILL_VCPKG%\scripts\buildsystems\vcpkg.cmake" (
    set TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=%JILL_VCPKG%/scripts/buildsystems/vcpkg.cmake"
)

set "OUT=%ROOT%port/build32/Release32"

cmake -S "%ROOT%port" -B "%ROOT%port\build32" -A Win32 %TOOLCHAIN% ^
      "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=%OUT%"
if errorlevel 1 exit /b 1
cmake --build "%ROOT%port\build32" --config Release
if errorlevel 1 exit /b 1

if not exist "%OUT%\jill.exe" (
    echo error: build finished but %OUT%\jill.exe was not created.
    exit /b 1
)
copy /y "%JILL_VCPKG%\installed\x86-windows\bin\SDL2.dll" "%OUT%\" >nul

echo.
echo Built %OUT%\jill.exe

rem --- release package (x86) -------------------------------------------------
rem Merged from make_release_32.cmd: every successful build leaves a
rem distributable archive in Releases\.  Included: jill.exe, the Win32 demo
rem studio, SDL2.dll, the extractor/importer tools, profiles/, games/*.game,
rem external/ and the user documentation.  Excluded: all original game data
rem (jillt/, ONESIM/ and extracted assets).
set "STAGE=%ROOT%build32\Release32\Jill-EX"
set "RELEASES=%ROOT%Releases"
set "ZIP=%RELEASES%\Jill-EX-release-x86.zip"

if not exist "%RELEASES%" mkdir "%RELEASES%" || goto :fail
if not exist "%OUT%\SDL2.dll" (
    echo error: SDL2.dll not found beside jill.exe.
    goto :fail
)

if exist "%ROOT%build32" rmdir /s /q "%ROOT%build32"
mkdir "%STAGE%" || goto :fail

rem --- game binary -----------------------------------------------------------
copy /y "%OUT%\jill.exe" "%STAGE%\" >nul || goto :fail
copy /y "%OUT%\SDL2.dll" "%STAGE%\" >nul || goto :fail

rem --- demo studio -----------------------------------------------------------
rem Always rebuild (incremental) so the bundled studio matches port/src.
call "%ROOT%tools\build_demo_studio32.cmd"
if errorlevel 1 goto :fail
copy /y "%ROOT%tools\demo_studio\build32\jill_demo_studio.exe" "%STAGE%\" >nul || goto :fail

rem --- extractor / importer tools --------------------------------------------
for %%F in (jill_extractor.py jill_import.py jill_editor.py extract_all.py
            requirements.txt) do (
    copy /y "%ROOT%%%F" "%STAGE%\" >nul || goto :fail
)
copy /y "%ROOT%tools\hud_layout.py" "%STAGE%\hud_layout.py" >nul || goto :fail

rem --- runtime data ----------------------------------------------------------
robocopy "%ROOT%profiles" "%STAGE%\profiles" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :fail
robocopy "%ROOT%games" "%STAGE%\games" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :fail
robocopy "%ROOT%external" "%STAGE%\external" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :fail

rem --- documentation and notices ---------------------------------------------
for %%F in (README.md ENGINE.md MODS.md EXTASSETS.md EXTRACTOR_README.md
            IMPORT_README.md EDITOR_README.md BUILDING.md TOOLS_DATA.md
            LICENSE.md THIRD_PARTY_NOTICES.md) do (
    if exist "%ROOT%%%F" copy /y "%ROOT%%%F" "%STAGE%\" >nul
)
if exist "%ROOT%docs\hud-layout-tool.md" copy /y "%ROOT%docs\hud-layout-tool.md" "%STAGE%\" >nul
if exist "%ROOT%docs\demo-studio.md" copy /y "%ROOT%docs\demo-studio.md" "%STAGE%\" >nul

rem --- zip -------------------------------------------------------------------
if exist "%ZIP%" del /q "%ZIP%"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Compress-Archive -LiteralPath '%STAGE%' -DestinationPath '%ZIP%' -Force"
if errorlevel 1 goto :fail

echo Staged in: %STAGE%
for %%F in ("%ZIP%") do echo Release zip: %%~fF
echo Run it with run32.cmd (asset paths are in jill.ini [paths]).
exit /b 0

:fail
echo.
echo error: build or release packaging failed.
exit /b 1
