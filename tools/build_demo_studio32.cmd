@echo off
rem Build the 32-bit (Win32/x86) demo studio into tools\demo_studio\build32.
rem
rem   tools\build_demo_studio32.cmd           configure and build
rem   tools\build_demo_studio32.cmd install   install sdl2:x86-windows first
rem
rem The standard 64-bit studio stays in tools\demo_studio\build
rem (tools\build_demo_studio.cmd), and the game's own 32-bit build is
rem unrelated (build32.cmd).
setlocal
set ROOT=%~dp0..
set JILL_VCPKG=%VCPKG_ROOT%
if "%JILL_VCPKG%"=="" set JILL_VCPKG=C:\vcpkg

set VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat
if not exist "%VCVARS%" (
    echo Cannot find vcvars32.bat. Edit tools\build_demo_studio32.cmd.
    exit /b 1
)
call "%VCVARS%" >nul

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
    echo        Run: tools\build_demo_studio32.cmd install
    exit /b 1
)

set TOOLCHAIN=
if exist "%JILL_VCPKG%\scripts\buildsystems\vcpkg.cmake" (
    set TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=%JILL_VCPKG%/scripts/buildsystems/vcpkg.cmake"
)

cmake -S "%ROOT%\tools\demo_studio" -B "%ROOT%\tools\demo_studio\build32" -A Win32 ^
      %TOOLCHAIN% "-DVCPKG_TARGET_TRIPLET=x86-windows"
if errorlevel 1 exit /b 1
cmake --build "%ROOT%\tools\demo_studio\build32" --config Release
if errorlevel 1 exit /b 1

if not exist "%ROOT%\tools\demo_studio\build32\jill_demo_studio.exe" (
    echo error: build finished but the 32-bit exe was not created.
    exit /b 1
)

echo.
echo Built %ROOT%\tools\demo_studio\build32\jill_demo_studio.exe
echo Run it with tools\run_demo_studio32.cmd (asset paths come from jill.ini).
