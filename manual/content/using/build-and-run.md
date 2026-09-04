---
title: Build and run
summary: Builds the Debug or Release executable for Windows or macOS and runs it against a directory of game data.
category: getting-started
source_files:
  - docs/BUILDING.md
  - CMakeLists.txt
  - code/CMakeLists.txt
  - code/language/CMakeLists.txt
related:
  - type: using
    id: game-data
  - type: using
    id: developer-build-troubleshooting
---

OpenTS supports Windows Win32 and macOS 15 or newer on arm64 and x86_64. The repository's `docs/BUILDING.md` owns the exact toolchain matrix.

The renderer and the audio layer are vendored dependencies, so a clone that did not fetch submodules has to fetch them before configuring. Configuration stops with instructions if a submodule is missing.

```bash
git submodule update --init --recursive
```

## Windows

Install Visual Studio 2022 with the **Desktop development with C++** workload, a Windows SDK, CMake 3.23 or newer, and Git for Windows.

```powershell title="PowerShell"
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The Debug build writes `GameD.exe`, its symbols, map file, and the matching `Language.dll` to `build/bin/Debug/`. Use `--config Release` to write `Game.exe` to `build/bin/Release/` instead. Nothing is copied out of the build directory, so the two configurations never overwrite each other.

`-A x64` builds the 64-bit executable. A build directory holds one platform, so give the 64-bit build its own, such as `-B build/x64`. A saved game belongs to the platform that wrote it, and a network game needs every player on the same platform.

Supply the required game data in `Run/`, then launch the built executable and name that data directory:

```powershell title="PowerShell"
.\build\bin\Debug\GameD.exe -DATADIR="$PWD\Run"
```

The game changes to the directory holding the executable before it reads the command line. A relative `-DATADIR` path is resolved from that directory, not from the one the command runs in.

The engine reads its strings and dialogs from `Language.dll`, and loads it from the directory holding the executable. The freshly built copy is therefore the one that runs, and a localized or edited library sitting in the game data directory is not read.

## macOS

Install the Xcode command-line tools, CMake 3.23 or newer, and Ninja. Configure
an arm64 Debug build on Apple Silicon like this:

```bash title="Terminal"
cmake -S . -B build/macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
cmake --build build/macos
ctest --test-dir build/macos --output-on-failure
```

Add `-DCMAKE_OSX_ARCHITECTURES=x86_64` and use a separate build directory for
an Intel build. Use `-DCMAKE_BUILD_TYPE=Release` for a Release build.

Debug produces `GameD`; Release produces `Game` in `build/macos/bin/`. The build
leaves the installation's `Language.dll` unchanged. After supplying the game
data in `Run/`, launch the executable from that directory so relative asset paths
and `Language.dll` resolve there:

```bash title="Terminal"
cd Run
../build/macos/bin/GameD
```
