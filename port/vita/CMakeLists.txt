# Jill-EX for PS Vita (VitaSDK + SDL2).
#
# This is a standalone CMake project with its own build directory; it shares
# only port/src with the desktop target.  Configure it with the Vita toolchain:
#
#   set VITASDK=C:\vitasdk
#   cmake -S port/vita -B port/vita/build -G Ninja
#   cmake --build port/vita/build
#
# build_vita.cmd does all of that, stages the companion data and prints the
# FTP instructions.

cmake_minimum_required(VERSION 3.18)

# The Vita toolchain file has to be in place before project(), and it is what
# makes cmake call arm-vita-eabi-gcc instead of the host compiler.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake"
            CACHE PATH "VitaSDK toolchain")
    else()
        message(FATAL_ERROR
            "VITASDK is not set. Run build_vita.cmd, or point VITASDK at the "
            "SDK root (for example C:/vitasdk).")
    endif()
endif()

project(jill_vita LANGUAGES C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## LiveArea metadata ---------------------------------------------------------
set(VITA_APP_NAME "Jill-EX")
set(VITA_TITLEID  "JILLEX001")   # exactly 9 characters
set(VITA_VERSION  "01.00")

## Sources -------------------------------------------------------------------
# The same translation units as the desktop target, cross-compiled.  The only
# platform-specific source is screen_vita2d.cpp, which presents through
# libvita2d; JILL_VITA selects the console data root (ux0:data/JillEX) and
# leaves the PS4-only videoout/pad/exit paths out.
set(JILL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src")

add_executable(jill
    ${JILL_SRC}/main.cpp
    ${JILL_SRC}/app.cpp
    ${JILL_SRC}/engine.cpp
    ${JILL_SRC}/macro.cpp
    ${JILL_SRC}/player.cpp
    ${JILL_SRC}/board.cpp
    ${JILL_SRC}/tiles.cpp
    ${JILL_SRC}/shapes.cpp
    ${JILL_SRC}/framebuffer.cpp
    ${JILL_SRC}/profile.cpp
    ${JILL_SRC}/game.cpp
    ${JILL_SRC}/textpages.cpp
    ${JILL_SRC}/config.cpp
    ${JILL_SRC}/platform_paths.cpp
    ${JILL_SRC}/screen_sdl.cpp
    ${JILL_SRC}/cmf.cpp
    ${JILL_SRC}/nuked_opl.cpp
    ${JILL_SRC}/fmopl_opl.cpp
    ${JILL_SRC}/music.cpp
    ${JILL_SRC}/midi.cpp
    ${JILL_SRC}/sfx.cpp
    ${JILL_SRC}/audio.cpp
    ${JILL_SRC}/extaudio.cpp
    ${JILL_SRC}/external.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/screen_vita2d.cpp
    ${JILL_SRC}/third_party/stb_vorbis.c
    ${JILL_SRC}/third_party/opl3.c
    ${JILL_SRC}/third_party/fmopl.c
    ${JILL_SRC}/third_party/miniz.c
    ${JILL_SRC}/third_party/miniz_tinfl.c
    ${JILL_SRC}/third_party/miniz_tdef.c
    ${JILL_SRC}/third_party/miniz_zip.c
)

target_include_directories(jill PRIVATE ${JILL_SRC})

# JILL_HAVE_SDL pulls in the SDL2 screen and audio backends (the same ones the
# desktop build uses).  SDL_MAIN_HANDLED keeps our own main().
target_compile_definitions(jill PRIVATE
    JILL_VITA=1
    JILL_HAVE_SDL=1
    SDL_MAIN_HANDLED
)

# The Vita has no floating-point unit, so -O2 matters; unused-parameter stays
# quiet because the desktop backends are compiled with console stubs.
target_compile_options(jill PRIVATE
    -O2
    -Wall
    -Wextra
    -Wno-unused-parameter
)

find_package(SDL2 CONFIG REQUIRED)
# SDL2 is used for events, the pad and audio; libvita2d owns presentation.
target_link_libraries(jill PRIVATE SDL2::SDL2 vita2d)

## Vita artefacts ------------------------------------------------------------
vita_create_self(jill.self jill)

set(JILL_VITA_ASSETS "${CMAKE_CURRENT_SOURCE_DIR}/sce_sys")
vita_create_vpk(jill.vpk ${VITA_TITLEID} jill.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${JILL_VITA_ASSETS}/icon0.png                      sce_sys/icon0.png
    FILE ${JILL_VITA_ASSETS}/livearea/contents/bg.png        sce_sys/livearea/contents/bg.png
    FILE ${JILL_VITA_ASSETS}/livearea/contents/startup.png   sce_sys/livearea/contents/startup.png
    FILE ${JILL_VITA_ASSETS}/livearea/contents/template.xml  sce_sys/livearea/contents/template.xml
)
