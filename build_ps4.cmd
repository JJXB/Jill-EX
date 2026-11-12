@echo off
setlocal enabledelayedexpansion

rem Jill-EX PS4 build (OpenOrbis + Piglet OpenGL ES 2.0).
rem
rem This script is independent of build.cmd/build32.cmd: it never touches the
rem desktop CMake build or its outputs.  It produces
rem   port\ps4\build\eboot.bin
rem   port\ps4\build\<CONTENT_ID>.pkg   (fake-signed installable package)
rem and copies the default jill.ini template beside them.
rem
rem Usage:  build_ps4.cmd            compile + link + eboot + pkg
rem         build_ps4.cmd nopkg      compile + link + eboot only
rem         build_ps4.cmd clean      remove port\ps4\build

set ROOT=%~dp0
set PORT=%ROOT%port
set OUT=%PORT%\ps4\build
set BUILD_PKG=1
if /I "%~1"=="nopkg" set BUILD_PKG=0

if /I "%~1"=="clean" (
    if exist "%OUT%" rmdir /s /q "%OUT%"
    echo Cleaned %OUT%
    exit /b 0
)

rem --- package metadata -----------------------------------------------------
set PKG_TITLE=Jill-EX
set PKG_VERSION=1.00
set PKG_TITLE_ID=BREW00199
rem The content id must be exactly 36 characters (IV0000-<9>_00-<16>).
set PKG_CONTENT_ID=IV0000-BREW00199_00-JILLEXPS40000000

rem --- toolchain ------------------------------------------------------------
set TC=%OO_PS4_TOOLCHAIN%
if "%TC%"=="" set TC=D:\Apps\OpenOrbis\PS4Toolchain
if not exist "%TC%\lib\link.x" if not exist "%TC%\link.x" (
    echo Cannot find the OpenOrbis toolchain at "%TC%".
    echo Set OO_PS4_TOOLCHAIN to the PS4Toolchain folder.
    exit /b 1
)
if not exist "%TC%\lib\libSDL2.a" (
    echo Missing %TC%\lib\libSDL2.a
    exit /b 1
)

where clang >nul 2>nul
if errorlevel 1 set PATH=C:\Program Files\LLVM\bin;%PATH%
where clang >nul 2>nul
if errorlevel 1 (
    echo clang was not found. Install LLVM and put its bin folder on PATH.
    exit /b 1
)
where ld.lld >nul 2>nul
if errorlevel 1 (
    echo ld.lld was not found. Install LLVM and put its bin folder on PATH.
    exit /b 1
)

if not exist "%OUT%" mkdir "%OUT%"

set CXXFLAGS=--target=x86_64-pc-freebsd12-elf -std=gnu++17 -fPIC -funwind-tables -fexceptions -fcxx-exceptions -DJILL_PS4=1 -DJILL_HAVE_SDL=1 -I"%TC%\include" -I"%TC%\include\c++\v1" -I"%TC%\include\SDL2"
set CFLAGS=--target=x86_64-pc-freebsd12-elf -std=gnu11 -fPIC -funwind-tables -I"%TC%\include"

rem --- compile -------------------------------------------------------------
for %%f in ("%PORT%\src\*.cpp") do (
    echo [cxx] %%~nxf
    clang++ %CXXFLAGS% -c -o "%OUT%\%%~nf.o" "%%f"
    if errorlevel 1 goto :fail
)
echo [cxx] screen_gl.cpp
clang++ %CXXFLAGS% -c -o "%OUT%\screen_gl.o" "%PORT%\ps4\screen_gl.cpp"
if errorlevel 1 goto :fail
echo [cxx] ps4_pad.cpp
clang++ %CXXFLAGS% -c -o "%OUT%\ps4_pad.o" "%PORT%\ps4\ps4_pad.cpp"
if errorlevel 1 goto :fail
echo [cxx] screen_videoout.cpp
clang++ %CXXFLAGS% -c -o "%OUT%\screen_videoout.o" "%PORT%\ps4\screen_videoout.cpp"
if errorlevel 1 goto :fail

for %%f in ("%PORT%\src\third_party\*.c") do (
    echo [cc ] %%~nxf
    clang %CFLAGS% -c -o "%OUT%\%%~nf.o" "%%f"
    if errorlevel 1 goto :fail
)

rem --- link ----------------------------------------------------------------
set OBJS=
for %%f in ("%OUT%\*.o") do set OBJS=!OBJS! "%%~ff"

set LIBS=-lc -lkernel -lc++ -lSDL2 -lScePigletv2VSH -lScePrecompiledShaders -lSceSysmodule -lSceSystemService -lSceUserService -lSceAudioOut -lScePad -lSceVideoOut -lSceShellCoreUtil -lSceRtc

echo [ld ] jill.elf
ld.lld -m elf_x86_64 -pie --script "%TC%\link.x" --eh-frame-hdr -o "%OUT%\jill.elf" "-L%TC%\lib" %LIBS% "%TC%\lib\crt1.o" %OBJS%
if errorlevel 1 goto :fail

echo [fself] eboot.bin
"%TC%\bin\windows\create-fself.exe" -in "%OUT%\jill.elf" --out "%OUT%\jill.oelf" --eboot "%OUT%\eboot.bin" --paid 0x3800000000000011
if errorlevel 1 goto :fail

if exist "%PORT%\ps4\jill.ini" copy /y "%PORT%\ps4\jill.ini" "%OUT%\jill.ini" >nul
if exist "%ROOT%DISCLAIMER.txt" copy /y "%ROOT%DISCLAIMER.txt" "%OUT%\" >nul

rem --- companion data zip (games/, profiles/, jill.ini) ---------------------
echo [data] staging games/profiles/jill.ini
set DATADIR=%OUT%\data
if exist "%DATADIR%" rmdir /s /q "%DATADIR%"
mkdir "%DATADIR%\games"
mkdir "%DATADIR%\profiles"
mkdir "%DATADIR%\external"
copy /y "%PORT%\ps4\jill.ini" "%DATADIR%\jill.ini" >nul
copy /y "%ROOT%games\*.game" "%DATADIR%\games\" >nul
copy /y "%ROOT%profiles\*.profile" "%DATADIR%\profiles\" >nul
copy /y "%ROOT%profiles\*.pal" "%DATADIR%\profiles\" >nul
copy /y "%PORT%\ps4\PS4-DATA-README.txt" "%DATADIR%\README.txt" >nul
copy /y "%ROOT%DISCLAIMER.txt" "%DATADIR%\" >nul
echo [data] JillEX-data.zip
if exist "%OUT%\JillEX-data.zip" del /q "%OUT%\JillEX-data.zip"
powershell -NoProfile -ExecutionPolicy Bypass -Command "Compress-Archive -Path '%DATADIR%\*' -DestinationPath '%OUT%\JillEX-data.zip' -Force"
if errorlevel 1 goto :fail
rmdir /s /q "%DATADIR%"
copy /y "%OUT%\JillEX-data.zip" "%ROOT%" >nul
if errorlevel 1 goto :fail
echo Built %OUT%\JillEX-data.zip (copy at %ROOT%JillEX-data.zip)

rem --- package (fake-signed .pkg) -------------------------------------------
if "%BUILD_PKG%"=="0" (
    echo [pkg] skipped ^(nopkg^)
    goto :done
)

set PKGTOOL=%TC%\bin\windows\PkgTool.Core.exe
set GP4TOOL=%TC%\bin\windows\create-gp4.exe
if not exist "%PKGTOOL%" (
    echo Missing %PKGTOOL%
    exit /b 1
)
if not exist "%GP4TOOL%" (
    echo Missing %GP4TOOL%
    exit /b 1
)
if not exist "%TC%\samples\SDL2\sce_module\libc.prx" (
    echo Missing the PS4 system modules under "%TC%\samples\SDL2\sce_module".
    echo They are required inside the package ^(libc.prx and libSceFios2.prx^).
    exit /b 1
)
if not exist "%PORT%\ps4\sce_sys\icon0.png" (
    echo Missing "%PORT%\ps4\sce_sys\icon0.png"
    exit /b 1
)
rem PkgTool.Core targets .NET Core 3.0; allow newer runtimes to run it.
set DOTNET_ROLL_FORWARD=Major

set PKGDIR=%OUT%\pkg
if exist "%PKGDIR%" rmdir /s /q "%PKGDIR%"
mkdir "%PKGDIR%\sce_sys\about"
mkdir "%PKGDIR%\sce_module"

echo [pkg] staging
copy /y "%OUT%\eboot.bin" "%PKGDIR%\" >nul
copy /y "%PORT%\ps4\sce_sys\icon0.png" "%PKGDIR%\sce_sys\" >nul
set PKG_FILES=eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png sce_module/libc.prx sce_module/libSceFios2.prx
rem Optional home-screen artwork (1920x1080 PNG) and startup sound.
if exist "%PORT%\ps4\sce_sys\pic0.png" (
    echo [pkg] adding pic0.png
    copy /y "%PORT%\ps4\sce_sys\pic0.png" "%PKGDIR%\sce_sys\" >nul
    set PKG_FILES=!PKG_FILES! sce_sys/pic0.png
)
if exist "%PORT%\ps4\sce_sys\pic1.png" (
    echo [pkg] adding pic1.png
    copy /y "%PORT%\ps4\sce_sys\pic1.png" "%PKGDIR%\sce_sys\" >nul
    set PKG_FILES=!PKG_FILES! sce_sys/pic1.png
)
if exist "%PORT%\ps4\sce_sys\snd0.at9" (
    echo [pkg] adding snd0.at9
    copy /y "%PORT%\ps4\sce_sys\snd0.at9" "%PKGDIR%\sce_sys\" >nul
    set PKG_FILES=!PKG_FILES! sce_sys/snd0.at9
)
copy /y "%TC%\samples\SDL2\sce_sys\about\right.sprx" "%PKGDIR%\sce_sys\about\" >nul
copy /y "%TC%\samples\SDL2\sce_module\libc.prx" "%PKGDIR%\sce_module\" >nul
copy /y "%TC%\samples\SDL2\sce_module\libSceFios2.prx" "%PKGDIR%\sce_module\" >nul

pushd "%PKGDIR%"
"%PKGTOOL%" sfo_new sce_sys\param.sfo >nul
if errorlevel 1 goto :pkgfail
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo APP_TYPE --type Integer --maxsize 4 --value 1 >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo APP_VER --type Utf8 --maxsize 8 --value %PKG_VERSION% >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo ATTRIBUTE --type Integer --maxsize 4 --value 0 >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo CATEGORY --type Utf8 --maxsize 4 --value "gd" >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo CONTENT_ID --type Utf8 --maxsize 48 --value %PKG_CONTENT_ID% >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0 >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo SYSTEM_VER --type Integer --maxsize 4 --value 0 >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo TITLE --type Utf8 --maxsize 128 --value "%PKG_TITLE%" >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo TITLE_ID --type Utf8 --maxsize 12 --value %PKG_TITLE_ID% >nul
"%PKGTOOL%" sfo_setentry sce_sys\param.sfo VERSION --type Utf8 --maxsize 8 --value %PKG_VERSION% >nul
if errorlevel 1 goto :pkgfail

echo [pkg] create-gp4
"%GP4TOOL%" -out pkg.gp4 --content-id=%PKG_CONTENT_ID% --files "%PKG_FILES%"
if errorlevel 1 goto :pkgfail

echo [pkg] pkg_build
"%PKGTOOL%" pkg_build pkg.gp4 .
if errorlevel 1 goto :pkgfail
popd

copy /y "%PKGDIR%\%PKG_CONTENT_ID%.pkg" "%OUT%\" >nul
if errorlevel 1 goto :fail
copy /y "%PKGDIR%\%PKG_CONTENT_ID%.pkg" "%ROOT%" >nul
if errorlevel 1 goto :fail

:done
echo.
echo Built %OUT%\eboot.bin
if "%BUILD_PKG%"=="1" echo Built %OUT%\%PKG_CONTENT_ID%.pkg (copy at %ROOT%%PKG_CONTENT_ID%.pkg)
echo.
echo Installing:
echo   - PKG: install %PKG_CONTENT_ID%.pkg on the console (GoldHEN/debug settings).
echo   - Loose eboot: copy eboot.bin to your loader folder.
echo   - Game data is NOT packaged: copy jill.ini, games\, profiles\, external\
echo     and your extracted jillt\/ONESIM\ to /data/JillEX on the console.
exit /b 0

:pkgfail
popd
:fail
echo.
echo PS4 build failed.
exit /b 1
