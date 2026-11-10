@echo off
setlocal
cd /d "%~dp0"
set "EXE=%~dp0port\build32\Release32\jill.exe"
if not exist "%EXE%" (
    echo %EXE% not found - run build32.cmd first.
    exit /b 1
)
"%EXE%" %*
