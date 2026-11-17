@echo off
rem Build the external .mac demo studio (tools/demo_studio).
rem Reuses port/src engine sources and needs SDL2 from vcpkg, like build.cmd.
rem
rem This is the 64-bit build (tools\demo_studio\build).  The Win32 build is
rem tools\build_demo_studio32.cmd (tools\demo_studio\build32).
setlocal
set ROOT=%~dp0..
set JILL_VCPKG=%VCPKG_ROOT%
if "%JILL_VCPKG%"=="" set JILL_VCPKG=C:\vcpkg

set VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%VCVARS%" (
    echo Cannot find vcvars64.bat. Edit tools\build_demo_studio.cmd.
    exit /b 1
)
call "%VCVARS%" >nul

if /I "%~1"=="install" (
    if not exist "%JILL_VCPKG%\vcpkg.exe" (
        echo vcpkg.exe not found under %JILL_VCPKG%
        exit /b 1
    )
    "%JILL_VCPKG%\vcpkg.exe" install sdl2:x64-windows
    if errorlevel 1 exit /b 1
)

set TOOLCHAIN=
if exist "%JILL_VCPKG%\scripts\buildsystems\vcpkg.cmake" (
    set TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=%JILL_VCPKG%/scripts/buildsystems/vcpkg.cmake"
)

cmake -S "%ROOT%\tools\demo_studio" -B "%ROOT%\tools\demo_studio\build" -A x64 %TOOLCHAIN%
if errorlevel 1 exit /b 1
cmake --build "%ROOT%\tools\demo_studio\build" --config Release
if errorlevel 1 exit /b 1

echo.
echo Built %ROOT%\tools\demo_studio\build\jill_demo_studio.exe
echo Run it with tools\run_demo_studio.cmd (asset paths come from jill.ini).
