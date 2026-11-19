# NXDK toolchain for Jill-EX.
#
# NXDK's own share/toolchain-nxdk.cmake is the base, but it points CMake at
# "$NXDK_DIR/bin/nxdk-cc" and "$NXDK_DIR/bin/nxdk-link", which only exist with
# the .cmd suffix on Windows (this SDK ships no extension-less launchers).
# It also assumes NXDK's prebuilt lib/libc++.lib, which is not part of a source
# checkout; the project builds libc++ itself, so it is removed from the
# default link line.
#
# Use through build_xbox.cmd, or:
#   set NXDK_DIR=D:\Apps\NXDK
#   cmake -S port/xbox -B port/xbox/build -G Ninja ^
#         -DCMAKE_TOOLCHAIN_FILE=port/xbox/nxdk.toolchain.cmake

if(DEFINED ENV{NXDK_DIR})
    set(NXDK_DIR "$ENV{NXDK_DIR}")
endif()
if(NOT NXDK_DIR)
    message(FATAL_ERROR "NXDK_DIR is not set; point it at the nxdk checkout.")
endif()

include("${NXDK_DIR}/share/toolchain-nxdk.cmake")

if(CMAKE_HOST_WIN32)
    set(CMAKE_C_COMPILER "${NXDK_DIR}/bin/nxdk-cc.cmd")
    set(CMAKE_CXX_COMPILER "${NXDK_DIR}/bin/nxdk-cxx.cmd")
    set(CMAKE_C_LINK_EXECUTABLE
        "${NXDK_DIR}/bin/nxdk-link.cmd <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -out:<TARGET> <LINK_LIBRARIES>")
    set(CMAKE_CXX_LINK_EXECUTABLE
        "${NXDK_DIR}/bin/nxdk-link.cmd <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -out:<TARGET> <LINK_LIBRARIES>")
endif()

# libc++ comes from the nxdk_cxx target, not from $NXDK_DIR/lib.
set(CMAKE_CXX_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES}")
