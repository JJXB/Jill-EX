# Jill-EX for Android (SDL2 + NDK).
#
# This project compiles the same port/src translation units as the desktop,
# PS4 and Vita builds.  SDL2 is built from the installed source tree so the
# native library and the Java glue copied into app/src/main/java/org/libsdl/app
# always match.
#
# Gradle passes -DJILL_SDL2_DIR=<SDL2 source root>; see app/build.gradle.

cmake_minimum_required(VERSION 3.18)
project(jillex LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## SDL2 ----------------------------------------------------------------------
if(NOT JILL_SDL2_DIR)
    set(JILL_SDL2_DIR "$ENV{SDL2_ANDROID}" CACHE PATH "SDL2 source tree")
endif()
if(NOT JILL_SDL2_DIR OR NOT EXISTS "${JILL_SDL2_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "SDL2 source not found. Set sdl2.dir in port/android/local.properties "
        "or the SDL2_ANDROID environment variable (currently '${JILL_SDL2_DIR}').")
endif()

# Shared libSDL2.so is loaded by SDLActivity; the app module supplies SDL_main.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
set(SDL_TESTS  OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

# Android 15+ devices use 16 KiB pages by default and refuse to install APKs
# whose native libraries have 4 KiB-aligned LOAD segments (Play/ADB reports
# "16kb load segment alignment").  This applies to libSDL2.so and libmain.so.
add_link_options("-Wl,-z,max-page-size=16384" "-Wl,-z,common-page-size=16384")

message(STATUS "SDL2 source: ${JILL_SDL2_DIR}")
add_subdirectory("${JILL_SDL2_DIR}" "${CMAKE_BINARY_DIR}/SDL2")

## Jill engine ---------------------------------------------------------------
get_filename_component(JILL_REPO_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../.." ABSOLUTE)
set(JILL_SRC "${JILL_REPO_ROOT}/port/src")

add_library(main SHARED
    ${JILL_SRC}/main.cpp
    ${JILL_SRC}/app.cpp
    ${JILL_SRC}/engine.cpp
    ${JILL_SRC}/editor.cpp
    ${JILL_SRC}/textkey.cpp
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
    ${JILL_SRC}/touch.cpp
    ${JILL_SRC}/cmf.cpp
    ${JILL_SRC}/nuked_opl.cpp
    ${JILL_SRC}/fmopl_opl.cpp
    ${JILL_SRC}/music.cpp
    ${JILL_SRC}/midi.cpp
    ${JILL_SRC}/sfx.cpp
    ${JILL_SRC}/audio.cpp
    ${JILL_SRC}/extaudio.cpp
    ${JILL_SRC}/external.cpp
    ${JILL_SRC}/third_party/stb_vorbis.c
    ${JILL_SRC}/third_party/opl3.c
    ${JILL_SRC}/third_party/fmopl.c
    ${JILL_SRC}/third_party/miniz.c
    ${JILL_SRC}/third_party/miniz_tinfl.c
    ${JILL_SRC}/third_party/miniz_tdef.c
    ${JILL_SRC}/third_party/miniz_zip.c
)

target_include_directories(main PRIVATE ${JILL_SRC})

# main.cpp includes SDL_main.h on Android; that header renames main() to
# SDL_main with C linkage, which is the symbol SDLActivity dlsym()s.

target_compile_definitions(main PRIVATE
    JILL_ANDROID=1
    JILL_HAVE_SDL=1
)

target_compile_options(main PRIVATE
    -O2
    -Wall
    -Wextra
    -Wno-unused-parameter
)

target_link_libraries(main PRIVATE
    SDL2::SDL2
    log
    android
)

# Keep libmain.so named exactly "main" so SDLActivity's getMainSharedObject()
# finds it without an override.
set_target_properties(main PROPERTIES OUTPUT_NAME "main")
