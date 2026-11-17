@echo off
rem Run the 32-bit demo studio from the repository root.  The 64-bit exe is
rem tools\demo_studio\build\jill_demo_studio.exe (tools\run_demo_studio.cmd);
rem this script uses the Win32 build in tools\demo_studio\build32.
setlocal
set ROOT=%~dp0..
cd /d "%ROOT%"
if not exist "%ROOT%\tools\demo_studio\build32\jill_demo_studio.exe" (
    echo Not built yet: run tools\build_demo_studio32.cmd first.
    exit /b 1
)
"%ROOT%\tools\demo_studio\build32\jill_demo_studio.exe" %*
