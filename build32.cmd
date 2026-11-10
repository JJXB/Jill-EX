@echo off
rem Build the 32-bit (Win32/x86) Release port into port\build32\Release32.
rem
rem   build32.cmd           configure and build
rem   build32.cmd install   install sdl2:x86-windows first
rem
rem The 64-bit build stays in port\build\Release (build.cmd).

setlocal
set "ROOT=%~dp0"
set "JILL_VCPKG=%VCPKG_ROOT%"
if "%JILL_VCPKG%"=="" set "JILL_VCPKG=C:\vcpkg"

set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
if not exist "%VCVARS%" (
    echo Cannot find vcvars32.bat. Edit build32.cmd.
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
echo Run it with run32.cmd (asset paths are in jill.ini [paths]).
