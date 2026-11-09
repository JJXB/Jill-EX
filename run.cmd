@echo off
setlocal
cd /d "%~dp0"
set EXE=%~dp0port\build\Release\jill.exe
if not exist "%EXE%" (
    echo %EXE% not found - run build.cmd first.
    exit /b 1
)
"%EXE%" %*
