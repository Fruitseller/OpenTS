---
title: Developer-build troubleshooting
summary: Checks the supported toolchain, target architecture, output location, and local game-data tree.
category: troubleshooting
source_files:
  - docs/BUILDING.md
  - CMakeLists.txt
  - code/CMakeLists.txt
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

## Configuration fails before compilation

A message that `thirdparty/bgfx.cmake` is empty means the clone did not fetch the vendored renderer. Run `git submodule update --init --recursive` and configure again.

On Windows, use the Visual Studio 2022 generator with `-A Win32` or `-A x64`. On macOS 15 or newer, use Ninja and Apple clang with an arm64 or x86_64 target. The build supports no other compilers, Visual Studio versions, or target architectures. Configuring a platform over a build directory that already holds the other one fails; give each its own directory.

For a Visual Studio installation that CMake cannot discover through the Visual Studio Installer, pass its installation path and product version as described in the repository's `docs/BUILDING.md`.

## The executable is not in the run directory

Builds write their runnable files to their build directory's `bin` folder and copy nothing into `Run/`.

On Windows:
- Debug: `GameD.exe`, `GameD.pdb`, `GameD.map`, and `Language.dll` in `build/bin/Debug/`
- Release: `Game.exe`, `Game.pdb`, `Game.map`, and `Language.dll` in `build/bin/Release/`

On macOS, Debug builds `GameD` and Release builds `Game` in `build/macos/bin/`. The macOS build does not replace `Language.dll`.

## The executable cannot initialize game data

Name the game data directory with [`-DATADIR=<path>`](/using/command-line/data-directory/). The path is a legitimate Tiberian Sun installation or a copy of its data. The repository and CMake build directory do not supply proprietary game assets.
