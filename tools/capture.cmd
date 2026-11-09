@echo off
rem ---------------------------------------------------------------------------
rem  Jill of the Jungle - capture helper
rem  Records the whole desktop while DOSBox runs the game, so nothing has to be
rem  hooked into the emulator.  Trim the result afterwards.
rem
rem    tools\capture.cmd           -> 60 s capture
rem    tools\capture.cmd 120       -> 120 s capture
rem
rem  You drive the game; this script only records.  Close DOSBox when done.
rem  Simpler alternative: DOSBox Staging's own Ctrl+F5 (screenshot) and
rem  Ctrl+Alt+F5 (video capture, saved next to dosbox.exe).
rem ---------------------------------------------------------------------------
setlocal
set DOSBOX=D:\Emu\dosbox-staging-v0.83.0\dosbox.exe
set FFMPEG=C:\Apps\ffmpeg-master-latest-win64-gpl-shared\bin\ffmpeg.exe
set GAMEDIR=C:\Users\jjxb_\GIT\Jill-EX\jillt
set OUTDIR=%~dp0..\captures
set DUR=%~1
if "%DUR%"=="" set DUR=60

if not exist "%OUTDIR%" mkdir "%OUTDIR%"
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set LDT=%%I
set STAMP=%LDT:~0,8%_%LDT:~8,6%

echo Recording desktop for %DUR% s - start the game in the DOSBox window now.
start "" /b "%FFMPEG%" -y -f gdigrab -framerate 30 -i desktop ^
  -c:v libx264 -preset veryfast -crf 18 -pix_fmt yuv420p ^
  -t %DUR% "%OUTDIR%\jill_%STAMP%.mp4"

start "" "%DOSBOX%" -c "mount c: %GAMEDIR%" -c "c:" -c "jill1.exe"

echo.
echo   video  : %OUTDIR%\jill_%STAMP%.mp4   (%DUR% s)
echo   screens: DOSBox Ctrl+F5, or ffmpeg -i the mp4
echo.
endlocal
