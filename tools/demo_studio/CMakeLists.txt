cmake_minimum_required(VERSION 3.18)
project(jill_demo_studio LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Reuse the engine core from port/src.  Audio/display/screen translation units
# are deliberately excluded: the studio owns its own window and has no sound.
set(JILL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../../port/src")

add_executable(jill_demo_studio
    src/main.cpp
    src/studio.cpp
    src/ui.cpp
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
    ${JILL_SRC}/external.cpp
    ${JILL_SRC}/third_party/miniz.c
    ${JILL_SRC}/third_party/miniz_tinfl.c
    ${JILL_SRC}/third_party/miniz_tdef.c
    ${JILL_SRC}/third_party/miniz_zip.c
)

target_include_directories(jill_demo_studio PRIVATE
    ${JILL_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(jill_demo_studio PRIVATE SDL2::SDL2)

if(WIN32)
    target_link_libraries(jill_demo_studio PRIVATE winmm)
endif()

target_compile_definitions(jill_demo_studio PRIVATE SDL_MAIN_HANDLED)
if(MSVC)
    target_compile_definitions(jill_demo_studio PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(jill_demo_studio PRIVATE /W3)
else()
    target_compile_options(jill_demo_studio PRIVATE -Wall -Wextra)
endif()

# Put the exe in the build directory that was configured: x64 builds land in
# tools/demo_studio/build, the 32-bit build in tools/demo_studio/build32, so the
# two never overwrite each other.
set_target_properties(jill_demo_studio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
)
