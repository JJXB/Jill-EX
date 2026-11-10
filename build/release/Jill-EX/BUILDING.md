\## Building



\### Requirements

\* Windows 10/11 x64.

\* Visual Studio 2022 with the \*\*Desktop development with C++\*\* workload

&#x20; (MSVC v143 and a Windows SDK).

\* CMake 3.18 or newer (bundled with Visual Studio, or a standalone install).

\* vcpkg, by default at `C:\\vcpkg` (or set `VCPKG\_ROOT`), with SDL2:

&#x20; `vcpkg install sdl2:x64-windows`.

\* SDL2 is optional for a headless-only build. A normal game build needs it for

&#x20; the window, input and presentation.

\* Python 3 is optional and only needed to regenerate data or run the tools:

&#x20; `py -3 -m pip install -r requirements.txt` (Pillow).

\* Game data, configured in `jill.ini` `\[paths]`. The shipped file uses

&#x20; `D:/Emu/DOS/C/jillt` and `D:/Emu/DOS/C/ONESIM`; a fresh checkout can instead

&#x20; place local `jillt` and `ONESIM` folders next to `run.cmd`.



\### Build steps

1\. Install the Visual Studio C++ workload and CMake.

2\. Install vcpkg and SDL2:

&#x20;  `C:\\vcpkg\\vcpkg install sdl2:x64-windows`

&#x20;  or run `build.cmd install`.

3\. Check the asset paths in `jill.ini`:

&#x20;  ```

&#x20;  \[paths]

&#x20;  jill1 = D:/Emu/DOS/C/jillt

&#x20;  jill2 = D:/Emu/DOS/C/jillt

&#x20;  jill3 = D:/Emu/DOS/C/jillt

&#x20;  onesimus = D:/Emu/DOS/C/ONESIM

&#x20;  ```

4\. Build:

&#x20;  ```

&#x20;  cmd /c build.cmd

&#x20;  ```

&#x20;  The executable is written to `port\\build\\Release\\jill.exe` (with

&#x20;  `SDL2.dll` beside it).

5\. Run:

&#x20;  ```

&#x20;  run.cmd

&#x20;  ```

&#x20;  or `port\\build\\Release\\jill.exe --game jill1`.

6\. Headless smoke test:

&#x20;  ```

&#x20;  port\\build\\Release\\jill.exe --game jill1 --board 1 --ticks 40

&#x20;  ```



If Visual Studio is not in the default location, edit the `VCVARS` path in

`build.cmd`. If vcpkg is elsewhere, set `VCPKG\_ROOT`. To rebuild the generated

tables after editing the analysis data, run `py -3 re\\gen\_port\_data.py` and

`py -3 re\\gen\_games.py`.





