cmake_minimum_required(VERSION 3.18)
project(jill_port LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(jill
    src/main.cpp
    src/app.cpp
    src/engine.cpp
    src/player.cpp
    src/board.cpp
    src/tiles.cpp
    src/shapes.cpp
    src/framebuffer.cpp
    src/profile.cpp
    src/game.cpp
    src/textpages.cpp
    src/config.cpp
    src/cmf.cpp
    src/opl2.cpp
    src/music.cpp
    src/sfx.cpp
    src/audio.cpp
)

target_compile_definitions(jill PRIVATE SDL_MAIN_HANDLED)
if(MSVC)
    target_compile_definitions(jill PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(jill PRIVATE /W3)
else()
    target_compile_options(jill PRIVATE -Wall -Wextra)
endif()

# SDL2 is optional only so the core can be built and exercised headless on a
# machine that does not have it installed yet.
find_package(SDL2 CONFIG QUIET)
if(SDL2_FOUND)
    target_compile_definitions(jill PRIVATE JILL_HAVE_SDL=1)
    target_link_libraries(jill PRIVATE SDL2::SDL2)
    message(STATUS "SDL2: ${SDL2_DIR}")
else()
    message(WARNING "SDL2 not found - building the headless core only")
endif()
