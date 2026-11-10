@echo off
rem Build a distributable Jill-EX package from the current Release build.
rem
rem Included: jill.exe, SDL2.dll, the extractor/importer tools, profiles/,
rem games/*.game, external/ and the user documentation.
rem Excluded: all original game data (jillt/, ONESIM/ and extracted assets).
rem
rem Usage: make_release.cmd [zip-name]
rem Default zip name: Jill-EX-release-x86.zip

setlocal EnableExtensions
cd /d "%~dp0"

set "STAGE=build32\Release32\Jill-EX"
set "ZIP=%~1"
if "%ZIP%"=="" set "ZIP=Jill-EX-release-x86.zip"

if not exist "port\build32\Release32\jill.exe" (
    echo error: port\build32\Release32\jill.exe not found.
    echo        Run build.cmd first.
    exit /b 1
)
if not exist "port\build32\Release32\SDL2.dll" (
    echo error: SDL2.dll not found beside jill.exe.
    exit /b 1
)

if exist "build32\release32" rmdir /s /q "build32\release32"
mkdir "%STAGE%" || goto :error

rem --- game binary -----------------------------------------------------------
copy /y "port\build32\Release32\jill.exe" "%STAGE%\" >nul || goto :error
copy /y "port\build32\Release32\SDL2.dll" "%STAGE%\" >nul || goto :error

rem --- extractor / importer tools --------------------------------------------
copy /y "jill_extractor.py" "%STAGE%\" >nul || goto :error
copy /y "jill_import.py" "%STAGE%\" >nul || goto :error
copy /y "extract_all.py" "%STAGE%\" >nul || goto :error
copy /y "requirements.txt" "%STAGE%\" >nul || goto :error

rem --- runtime data ----------------------------------------------------------
robocopy "profiles" "%STAGE%\profiles" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :error
robocopy "games" "%STAGE%\games" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :error
robocopy "external" "%STAGE%\external" /E /NFL /NDL /NJH /NJS /NP >nul
if errorlevel 8 goto :error

rem --- documentation and notices ---------------------------------------------
for %%F in (README.md ENGINE.md MODS.md EXTASSETS.md EXTRACTOR_README.md
            BUILDING.md TOOLS_DATA.md LICENSE.md THIRD_PARTY_NOTICES.md) do (
    if exist "%%F" copy /y "%%F" "%STAGE%\" >nul
)

rem --- zip -------------------------------------------------------------------
if exist "%ZIP%" del /q "%ZIP%"
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "Compress-Archive -LiteralPath '%STAGE%' -DestinationPath '%ZIP%' -Force"
if errorlevel 1 goto :error

echo.
echo Staged in:  %STAGE%
for %%F in ("%ZIP%") do echo Release zip: %%~fF
echo.
dir /b "%STAGE%"
exit /b 0

:error
echo.
echo error: release packaging failed.
exit /b 1
