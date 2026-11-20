RIZIN - "C:\\Apps\\rizin-win-installer-vs2019\_static-64\\bin\\rizin.exe"
GHIDRA - "D:\\Apps\\ghidra\_12.2\_PUBLIC"
MKPROFILE - "tools\\mkprofile.py" (Python; profiles a Jill-engine EXE into profiles\<id>.profile + games\<id>.game)

HUDLAYOUT - "tools\\hud_layout.py" (Python/Tk + Pillow; HUD alignment editor with PNG or Jill .dma/.sha tilesets, writes .hudlayout files; see docs\hud-layout-tool.md)

JILLEDITOR - "jill_editor.py" (Python/Tk + Pillow; GUI editor for .sha sprites, .dma tiles, .jn1 boards/levels, VCL text pages and VCL SFX, built on jill_extractor.py + jill_import.py; see EDITOR_README.md)

DEMOSTUDIO - "tools\\demo_studio" (C++/SDL2; external .mac playback viewer/editor that links the port/src engine core; x64 build tools\build_demo_studio.cmd + tools\run_demo_studio.cmd, Win32 build tools\build_demo_studio32.cmd + tools\run_demo_studio32.cmd, capture with tools\capture_demo_studio.cmd; the matching jill_demo_studio.exe is included in the x64/x86 release zips; see docs\demo-studio.md)

FFMPEG - "C:\\Apps\\ffmpeg-master-latest-win64-gpl-shared\\bin"

VCPKG - "C:\\vcpkg"

OpenXRSDK - "C:\\Users\\jjxb\_\\GIT\\OpenXR-SDK"

VS2022VARS - C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat

VS2022VARS32 - "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat"

VS2017VARS - C:\\Program Files\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat

VS2017VARS32 - "C:\\Program Files\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat"

OXR-Template - "C:\\Users\\jjxb\_\\GIT\\OpenXR-Template"

DXCAP - "C:\\Users\\jjxb\_\\GIT\\DirectX-Capture"

\--ANDROID--

ANDROIDSDK - "D:\Apps\AndroidSDK"

ANDROIDJAVA - "D:\Apps\AndroidSDK\jdk-17" (JAVA_HOME for Android builds; system Java 25 remains on PATH)

ANDROIDGRADLE - "D:\Apps\AndroidSDK\gradle\gradle-8.14.7\bin"

ANDROIDNDK - "D:\Apps\AndroidSDK\ndk\27.3.13750724"

SDL2ANDROID - "D:\Apps\AndroidSDK\SDL2\SDL2-2.32.10\android-project"

\--PS4--

LibOrbisPkg - "D:\\Apps\\LibOrbisPkg"

opengnm - "C:\\Users\\jjxb\_\\GIT\\opengnm"

opengnm-psbc - "C:\\Users\\jjxb\_\\GIT\\opengnm-psbc"

vulkanps4 - "C:\\Users\\jjxb\_\\GIT\\vulkan-ps4"

PS4Toolchain - "%OO\_PS4\_TOOLCHAIN%"

\--PSP--

PSPSDK - "D:\Apps\PSPSDK"

\--XBOX--

NXDK - "D:\Apps\NXDK" (NXDK_DIR; toolchain + SDL2/libc++ sources; cxbe at tools\cxbe\cxbe.exe)

