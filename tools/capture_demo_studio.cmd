@echo off
rem ---------------------------------------------------------------------------
rem  Jill-EX demo studio - window capture helper
rem  Records the desktop for a fixed 30 s while the studio plays, so the
rem  interpolation modes can be compared frame by frame afterwards.
rem
rem    tools\capture_demo_studio.cmd
rem    tools\capture_demo_studio.cmd --interp original --mac jn1demo.mac
rem    tools\capture_demo_studio.cmd --interp off --game jill3 --mac jn3dem2.mac
rem
rem  Change DUR below for a longer clip.  Keep the studio window in front;
rem  the tool is started first and closed by you when the clip is done.
rem ---------------------------------------------------------------------------
setlocal
set ROOT=%~dp0..
set FFMPEG=C:\Apps\ffmpeg-master-latest-win64-gpl-shared\bin\ffmpeg.exe
set OUTDIR=%ROOT%\captures
set DUR=30
set EXE=%ROOT%\tools\demo_studio\build\jill_demo_studio.exe

if not exist "%EXE%" (
    echo Not built yet: run tools\build_demo_studio.cmd first.
    exit /b 1
)
if not exist "%OUTDIR%" mkdir "%OUTDIR%"
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value') do set LDT=%%I
set STAMP=%LDT:~0,8%_%LDT:~8,6%

echo Opening the demo studio - press SPACE to play, F5 to cycle interpolation.
start "" "%EXE%" %*
timeout /t 3 >nul
echo Recording the desktop for %DUR% s...
"%FFMPEG%" -y -f gdigrab -framerate 30 -i desktop ^
  -c:v libx264 -preset veryfast -crf 18 -pix_fmt yuv420p ^
  -t %DUR% "%OUTDIR%\demo_studio_%STAMP%.mp4"
echo.
echo   video: %OUTDIR%\demo_studio_%STAMP%.mp4   (%DUR% s)
echo   still: press F9 in the studio for a PPM frame in tools\demo_studio\shots
endlocal
