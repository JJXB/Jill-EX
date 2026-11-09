@echo off
setlocal
set ROOT=%~dp0
set JILL_VCPKG=%VCPKG_ROOT%
if "%JILL_VCPKG%"=="" set JILL_VCPKG=C:\vcpkg

set VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat
if not exist "%VCVARS%" (
    echo Cannot find vcvars64.bat. Edit build.cmd.
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

cmake -S "%ROOT%port" -B "%ROOT%port\build" -A x64 %TOOLCHAIN%
if errorlevel 1 exit /b 1
cmake --build "%ROOT%port\build" --config Release
if errorlevel 1 exit /b 1

echo.
echo Built %ROOT%port\build\Release\jill.exe
echo Run it with run.cmd (asset paths are in jill.ini [paths]).
