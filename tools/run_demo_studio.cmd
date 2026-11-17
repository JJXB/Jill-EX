@echo off
rem Run the .mac demo studio from the repository root so relative paths and
rem jill.ini [paths] behave like the game.
setlocal
set ROOT=%~dp0..
cd /d "%ROOT%"
if not exist "%ROOT%\tools\demo_studio\build\jill_demo_studio.exe" (
    echo Not built yet: run tools\build_demo_studio.cmd first.
    exit /b 1
)
"%ROOT%\tools\demo_studio\build\jill_demo_studio.exe" %*
