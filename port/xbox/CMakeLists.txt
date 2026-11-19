# Jill-EX for the original Xbox (NXDK + its SDL2 port).
#
# Standalone CMake project with its own build directory; it shares only
# port/src with the desktop, PS4, Vita and Android targets and never touches
# their outputs.
#
#   set NXDK_DIR=D:\Apps\NXDK
#   cmake -S port/xbox -B port/xbox/build -G Ninja ^
#         -DCMAKE_TOOLCHAIN_FILE=port/xbox/nxdk.toolchain.cmake
#   cmake --build port/xbox/build
#
# build_xbox.cmd does all of that and turns the linked PE into default.xbe.
#
# Why libc++ and SDL2 are built here: an NXDK source checkout ships their
# sources but no prebuilt lib/libc++.lib or lib/libSDL2.lib (those come from
# the release tarball), and NXDK's Makefiles refuse to run from a plain MSYS
# shell.  Building both from the checked-out sources keeps this target
# self-contained and independent of a prebuilt SDK layout.

cmake_minimum_required(VERSION 3.20)

project(jill_xbox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT NXDK_DIR)
    if(DEFINED ENV{NXDK_DIR})
        set(NXDK_DIR "$ENV{NXDK_DIR}")
    else()
        message(FATAL_ERROR "NXDK_DIR is not set; run build_xbox.cmd or pass "
                            "-DNXDK_DIR=<nxdk checkout>.")
    endif()
endif()

set(JILL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(NXDK_SDL_DIR "${NXDK_DIR}/lib/sdl/SDL2")
set(NXDK_LIBCXX_DIR "${NXDK_DIR}/lib/libcxx")

## libc++ --------------------------------------------------------------------
# The source list is read from NXDK's own Makefile.nxdk so it stays in sync
# with the SDK version rather than being copied here.
file(READ "${NXDK_LIBCXX_DIR}/Makefile.nxdk" _libcxx_makefile)
string(REGEX MATCHALL
       "\\$\\(LIBCXX_DIR\\)/src/[A-Za-z0-9_/]+\\.cpp"
       _libcxx_relative "${_libcxx_makefile}")
if(NOT _libcxx_relative)
    message(FATAL_ERROR
        "Could not read the libc++ source list from ${NXDK_LIBCXX_DIR}/Makefile.nxdk")
endif()
set(_libcxx_sources "")
foreach(_rel IN LISTS _libcxx_relative)
    string(REPLACE "$(LIBCXX_DIR)" "${NXDK_LIBCXX_DIR}" _abs "${_rel}")
    list(APPEND _libcxx_sources "${_abs}")
endforeach()
list(REMOVE_DUPLICATES _libcxx_sources)
# <random> selects the Win32 rand_s() backend whenever _WIN32 is defined, and
# NXDK's C library has no rand_s().  The engine never uses std::random_device,
# so the translation unit is left out instead of pulling in a dead dependency.
list(FILTER _libcxx_sources EXCLUDE REGEX "/random\\.cpp$")
# exception.cpp selects the MSVC <exception> implementation (_MSC_VER is set
# for the win32 target), which references __uncaught_exceptions() from NXDK's
# libxboxrt eh.obj; that object is built with MSVC exception machinery
# (___CxxFrameHandler3/___std_terminate) that a -fno-exceptions freestanding
# build cannot link.  Nothing in the engine uses std::exception.
list(FILTER _libcxx_sources EXCLUDE REGEX "/exception\\.cpp$")
# std::future/std::thread/condition_variable are unused by the engine and need
# std::exception_ptr, which only the excluded MSVC exception implementation
# provides.  port/xbox/xbox_runtime.cpp supplies std::terminate() and the
# uncaught_exception() queries that <ios>/<ostream> still reference.
foreach(_unused
        "src/future\\.cpp$"
        "src/thread\\.cpp$"
        "src/condition_variable\\.cpp$"
        "src/condition_variable_destructor\\.cpp$"
        "src/mutex\\.cpp$"
        "src/mutex_destructor\\.cpp$"
        "src/shared_mutex\\.cpp$")
    list(FILTER _libcxx_sources EXCLUDE REGEX "${_unused}")
endforeach()

add_library(nxdk_cxx STATIC ${_libcxx_sources})
target_include_directories(nxdk_cxx PUBLIC "${NXDK_LIBCXX_DIR}/include")
target_compile_options(nxdk_cxx PRIVATE
    -O2
    # The Xbox CRT has no C++ unwinder (no __CxxFrameHandler3 /
    # ___std_terminate); libc++'s <__config> already sets
    # _LIBCPP_NO_EXCEPTIONS for NXDK.
    -fno-exceptions
    "-include" "${CMAKE_CURRENT_SOURCE_DIR}/xbox_compat.h")
list(LENGTH _libcxx_sources _libcxx_count)
message(STATUS "libc++: ${_libcxx_count} sources from ${NXDK_LIBCXX_DIR}")

## SDL2 ----------------------------------------------------------------------
# Same source groups as NXDK's lib/sdl/SDL2/Makefile.xbox.
set(_sdl_sources "")
foreach(_pattern
        "${NXDK_SDL_DIR}/src/*.c"
        "${NXDK_SDL_DIR}/src/audio/*.c"
        "${NXDK_SDL_DIR}/src/audio/xbox/*.c"
        "${NXDK_SDL_DIR}/src/cpuinfo/*.c"
        "${NXDK_SDL_DIR}/src/events/*.c"
        "${NXDK_SDL_DIR}/src/file/*.c"
        "${NXDK_SDL_DIR}/src/haptic/*.c"
        "${NXDK_SDL_DIR}/src/haptic/dummy/*.c"
        "${NXDK_SDL_DIR}/src/joystick/*.c"
        "${NXDK_SDL_DIR}/src/joystick/xbox/*.c"
        "${NXDK_SDL_DIR}/src/loadso/dummy/*.c"
        "${NXDK_SDL_DIR}/src/power/*.c"
        "${NXDK_SDL_DIR}/src/filesystem/dummy/*.c"
        "${NXDK_SDL_DIR}/src/render/*.c"
        "${NXDK_SDL_DIR}/src/render/software/*.c"
        "${NXDK_SDL_DIR}/src/sensor/*.c"
        "${NXDK_SDL_DIR}/src/sensor/dummy/*.c"
        "${NXDK_SDL_DIR}/src/stdlib/*.c"
        "${NXDK_SDL_DIR}/src/thread/*.c"
        "${NXDK_SDL_DIR}/src/thread/generic/SDL_syscond.c"
        "${NXDK_SDL_DIR}/src/thread/windows/*.c"
        "${NXDK_SDL_DIR}/src/timer/*.c"
        "${NXDK_SDL_DIR}/src/timer/windows/*.c"
        "${NXDK_SDL_DIR}/src/video/*.c"
        "${NXDK_SDL_DIR}/src/video/xbox/*.c"
        "${NXDK_SDL_DIR}/src/video/yuv2rgb/*.c"
        "${NXDK_SDL_DIR}/src/libm/*.c"
        "${NXDK_SDL_DIR}/src/atomic/*.c")
    file(GLOB _matched "${_pattern}")
    list(APPEND _sdl_sources ${_matched})
endforeach()

if(NOT _sdl_sources)
    message(FATAL_ERROR "No SDL2 sources found under ${NXDK_SDL_DIR}")
endif()

add_library(nxdk_sdl STATIC ${_sdl_sources})
target_include_directories(nxdk_sdl PUBLIC
    "${NXDK_SDL_DIR}/include"
    # SDL's Xbox joystick driver talks to the OHCI USB stack directly; the
    # stack's config comes from NXDK's Xbox-specific header (see NXDK's
    # lib/usb/Makefile).
    "${NXDK_DIR}/lib/usb/libusbohci/inc"
    "${NXDK_DIR}/lib/usb/libusbohci_xbox"
)
target_compile_definitions(nxdk_sdl PRIVATE
    XBOX=1
    USBH_USE_EXTERNAL_CONFIG="usbh_config_xbox.h"
)
target_compile_options(nxdk_sdl PRIVATE -O2 -Wno-unused-parameter)

## Jill ----------------------------------------------------------------------
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
    ${JILL_SRC}/platform_fs.cpp
    ${JILL_SRC}/screen_sdl.cpp
    ${JILL_SRC}/cmf.cpp
    ${JILL_SRC}/opl2.cpp
    ${JILL_SRC}/nuked_opl.cpp
    ${JILL_SRC}/fmopl_opl.cpp
    ${JILL_SRC}/music.cpp
    ${JILL_SRC}/midi.cpp
    ${JILL_SRC}/sfx.cpp
    ${JILL_SRC}/audio.cpp
    ${JILL_SRC}/extaudio.cpp
    ${JILL_SRC}/external.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xbox_runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xbox_fs_glue.cpp
    ${JILL_SRC}/third_party/stb_vorbis.c
    ${JILL_SRC}/third_party/opl3.c
    ${JILL_SRC}/third_party/fmopl.c
    ${JILL_SRC}/third_party/miniz.c
    ${JILL_SRC}/third_party/miniz_tinfl.c
    ${JILL_SRC}/third_party/miniz_tdef.c
    ${JILL_SRC}/third_party/miniz_zip.c
)

target_include_directories(jill PRIVATE
    "${JILL_SRC}"
    "${NXDK_LIBCXX_DIR}/include"
)

target_compile_definitions(jill PRIVATE
    JILL_XBOX=1
    JILL_HAVE_SDL=1
    SDL_MAIN_HANDLED=1
    XBOX=1
    # miniz only needs its in-memory API here (zip packs are read from RAM);
    # its stdio path pulls in <sys/stat.h>/<sys/utime.h>, which the Xbox SDK
    # does not have, and its timestamps use localtime_s().
    MINIZ_NO_STDIO=1
    MINIZ_NO_TIME=1
    # The Xbox CPU profile (Pentium III) has no guaranteed SSE2, which
    # stb_image's SIMD IDCT assumes.
    STBI_NO_SIMD=1
)

# -include applies the libc++ compatibility shim to every translation unit
# (see port/xbox/xbox_compat.h).  The Xbox CRT has no C++ unwinder, so the
# engine is built without exceptions; the few guarded blocks use
# port/src/platform_except.h.
target_compile_options(jill PRIVATE
    -O2
    -fno-exceptions
    -Wall
    -Wextra
    -Wno-unused-parameter
    # C++ only: the shim includes C++ headers (libc++), and the third_party C
    # sources must not see it.
    "$<$<COMPILE_LANGUAGE:CXX>:-include>"
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/xbox_compat.h>"
)

target_link_libraries(jill PRIVATE nxdk_sdl nxdk_cxx)

# NXDK's CRT runs main() on a thread whose stack size comes from the XBE header
# (SizeOfStack) and only reserves 64 KiB by default.  The engine, SDL and
# libc++ need considerably more; without this the console returns to the
# dashboard before the first log line.
target_link_options(jill PRIVATE "-stack:1048576")

## default.xbe ---------------------------------------------------------------
find_program(CXBE_TOOL
    NAMES cxbe cxbe.exe
    PATHS "${NXDK_DIR}/tools/cxbe"
    NO_DEFAULT_PATH)
if(NOT CXBE_TOOL)
    message(FATAL_ERROR "cxbe was not found in ${NXDK_DIR}/tools/cxbe")
endif()

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/default.xbe" XBE_FILE)
add_custom_command(TARGET jill POST_BUILD
    COMMAND "${CXBE_TOOL}"
            "-OUT:${XBE_FILE}"
            "-TITLE:Jill-EX"
            "$<SHELL_PATH:$<TARGET_FILE:jill>>"
    COMMENT "Packing ${XBE_FILE}"
    VERBATIM)
