# Building OpenTS

> [!IMPORTANT]
> OpenTS supports Visual Studio 2022 `Win32` and `x64` builds and macOS 15 or
> newer on arm64 and x86_64, each in Debug and Release. All were verified from
> fresh CMake configurations. A successful build does not verify runtime
> behavior.

## Supported targets

### Windows

| Component | Requirement |
| --- | --- |
| Host | Windows |
| Target platforms | 32-bit (`Win32`) and 64-bit (`x64`) |
| Processor | SSE2, so a Pentium 4 or Athlon 64 onward; the `x64` build needs a 64-bit processor and Windows |
| Generator and compiler | Visual Studio 2022 MSVC 19.30 or newer |
| Windows SDK | A Visual Studio-installed Windows SDK |
| CMake | 3.23 or newer |
| C++ language level | C++20 |
| Configurations | Debug and Release, on both platforms |

### macOS

| Component | Requirement |
| --- | --- |
| Host and architecture | macOS 15 or newer; arm64 or x86_64 |
| Generator and compiler | Ninja with Apple clang from Xcode 16 or newer |
| CMake | 3.23 or newer |
| C++ language level | C++20 |
| Configurations | Debug and Release |

Other generators, compilers, operating systems, architectures, and
configurations are unsupported unless a later section marks them as an
experiment.

Install Visual Studio 2022 with the **Desktop development with C++** workload,
a Windows SDK, and CMake 3.23 or newer. Git for Windows is needed to clone the
repository and initialize its dependencies, but not to compile a complete
source tree.

### Save and network compatibility between the platforms

A save records pointer identities at a fixed width, but the members and raw
structures around them travel at the build's own widths, so a `Win32` build and
an `x64` build do not read each other's saves. Their network packets differ for
the same reason.

Nothing detects this. The packed version stamp that saves and network packets
carry records the version, not the pointer width, so a build of either platform
accepts the other's save and admits it to a network game, and the result is a
failed load or a desync rather than a refusal. Until the stamp distinguishes
them, keep a saved game with the platform that wrote it, and play a network
game with peers running the same platform.

## Dependencies

The renderer uses [bgfx](https://github.com/bkaradzic/bgfx), vendored through
`thirdparty/bgfx.cmake` at a tested tag. That submodule contains bgfx, bx, and
bimg as nested submodules, so initialize it recursively:

```powershell
git submodule update --init --recursive
```

The audio layer uses [miniaudio](https://github.com/mackron/miniaudio),
vendored through `thirdparty/miniaudio` at a tested tag and compiled as one
translation unit from `thirdparty/miniaudio-impl.c`.

For a fresh clone, use `git clone --recurse-submodules`. Configuration stops
with instructions if a submodule is missing. Update a pinned tag in a
separate change.

Compression uses [LZO](https://www.oberhumer.com/opensource/lzo/) 2.10,
vendored under `thirdparty/lzo` and built by `thirdparty/CMakeLists.txt`.
Upstream publishes releases as a tarball rather than through a repository, so
this copy is checked in instead of pinned as a submodule. It holds only the
LZO1X-1 sources the engine calls; take a later release by extracting it over
the files already there, in a separate change.

## Configure and build

Run these commands from the repository root in PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Debug
cmake --build build --config Release
```

`-A` selects the platform, and a build directory holds one of them. Configure
`x64` beside the 32-bit build rather than over it:

```powershell
cmake -S . -B build/x64 -G "Visual Studio 17 2022" -A x64
cmake --build build/x64 --config Debug
cmake --build build/x64 --config Release
```

CMake normally finds Visual Studio through the Visual Studio Installer. For an
unregistered installation, set `CMAKE_GENERATOR_INSTANCE` to its directory and
product version.

The solution contains only Debug and Release. Each writes its runtime files to
`<build directory>/bin/<configuration>/` and copies nothing anywhere else. The
test harnesses build into `<build directory>/test-bin/<configuration>/`, so
`bin/` holds only what the game runs. Compiler and linker intermediates stay in
the selected build directory.

| Configuration | Runtime files |
| --- | --- |
| Debug | `GameD.exe`, `GameD.pdb`, `GameD.map`, `Language.dll` |
| Release | `Game.exe`, `Game.pdb`, `Game.map`, `Language.dll` |

Run a build from its output directory, naming the game data with `-DATADIR=`:

```powershell
build\bin\Debug\GameD.exe -DATADIR=Run
```

`OPENTS_GAME_DIR` names that data directory for the generated Visual Studio
debugger settings and defaults to `Run/`. The data directory is only read from.
Saved games, logs, and crash reports go to the user directory, which defaults to
the executable's own directory, so a build writes beside itself unless
`-USERDIR=` says otherwise.

## Experimental clang-cl cross-build

An unsupported Linux cross-build is available for compiler-portability work. It
uses native `clang-cl`, LLD, and LLVM library and resource tools with the
MSVC headers and libraries. It does not expand the supported build matrix or
establish runtime behavior.

The reconstructed codebase may still contain undefined behavior that the
supported MSVC build happens not to expose. A successful clang-cl build may
therefore run incorrectly or fail at runtime; validate any result separately.

Provide a directory containing a Visual Studio layout and Windows SDK. The
cross-build uses the layout's default MSVC toolset and newest complete SDK.
Configure a single-configuration Ninja build:

```bash
cmake -S . -B build/clang-cl -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/clang-cl-msvc.cmake \
  -DOPENTS_MSVC_ROOT=/path/to/msvc
cmake --build build/clang-cl
```

The toolchain requires `clang-cl`, `lld-link`, `llvm-lib`, `llvm-mt`, and
`llvm-rc` on `PATH`. It exports `compile_commands.json`; one configuration in
`.vscode/c_cpp_properties.clang.example.json` reads that file for IntelliSense.

Set `-DOPENTS_WINDOWS_ARCH=x64` to cross-build Windows x64. The default is `x86`.

## macOS build

Install the Xcode command-line tools, CMake 3.23 or newer, and Ninja. The
default deployment target is macOS 15.0. Run these commands from the repository
root:

```bash
cmake -S . -B build/macos -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
cmake --build build/macos
ctest --test-dir build/macos
```

Use `-DCMAKE_BUILD_TYPE=Release` in a separate build directory for a Release
build. On Apple Silicon, configure a separate x86_64 tree to test the Rosetta
path:

```bash
cmake -S . -B build/macos-x86_64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
cmake --build build/macos-x86_64
ctest --test-dir build/macos-x86_64
```

Each build produces a plain arm64 or x86_64 executable in its `bin` directory
and copies it next to the game data, mirroring the Windows post-build step.
Debug builds define the engine's `_DEBUG` branches and use the `GameD` name;
Release builds omit them and use `Game`.
It links AppKit, Metal, and the audio and video frameworks; a Windows
compatibility layer under `code/platform/macos` supplies the Win32, COM
storage, DirectSound, and resource-loading interfaces the engine still calls.
The `code/language` resource DLL is Windows-only and is skipped.

The campaign-launch and basic-play checks recorded in the port plan have passed
on native arm64 and on x86_64 under Rosetta 2. These checks do not establish
native Intel runtime coverage or Windows/macOS multiplayer determinism.

## Build from Visual Studio Code

With the recommended extensions installed, the repository provides:

- CMake Tools settings;
- a configure task, a configuration picker, and hidden per-configuration tasks
  used by the launch configurations;
- launch and attach configurations;
- Test Explorer integration.

Standard VS Code shortcuts such as `Ctrl+Shift+B`, `F5`, and `Ctrl+F5` work as
usual.

## Build identity

The top-level `CMakeLists.txt` declares the project version in
`project(OpenTS VERSION ...)`. Since `project()` accepts only numbers, any
SemVer prerelease label goes in `OPENTS_VERSION_PRERELEASE`. Both values must
match the development entry in the manual's release registry;
`python manual/tools/manage.py check` verifies this.

Each build writes two generated headers from that version and the repository
state:

| Header | Contents |
| --- | --- |
| `opents_version.h` | The version components, the version string, a prerelease flag, and the packed version number |
| `opents_build.h` | The commit, branch, commit date, whether tracked files were modified, and the version as it is displayed |

The packed version stores the major, minor, and patch components in one byte
each. Saves and network peers reject a different number. Builds within one
release cycle, including prereleases, share it, but their saves, replays, and
network sessions may still be incompatible. The stamp does not record the
target platform; see
[Save and network compatibility between the platforms](#save-and-network-compatibility-between-the-platforms).

The version resources in `Game.exe` and `Language.dll`, the title screen,
version dialog, crash report, and debug log banner all read these headers. A
normal build shows the version and commit, such as `0.1.0 (ab12cd3)`, plus a
marker when tracked files are modified. The commit identifies the build for
diagnostics; it is not a save or network compatibility stamp. An official
build configured with `-DOPENTS_OFFICIAL_BUILD=ON` shows only its declared
version.

`opents_version.h` changes only with the version, so an ordinary commit does not
rebuild code that reads only that header. `opents_build.h` is checked on every
build, so a new commit appears without reconfiguring; an unchanged header is
not rewritten.

A tag or pull-request build uses a detached checkout with no branch. Its stamp
uses a ref that points to the commit, preferring a tag, so a pull-request CI
build names the pull request instead of `HEAD`.

Git is optional at build time once the complete source tree is present. Without
Git or repository metadata, the build records the commit as `unknown` and shows
the version without one.

## Continuous integration

The `Engine` workflow runs for ready pull requests and pushes to `main` when
their changed paths match its engine and build filters. Draft pull requests do
not build until marked ready; the workflow then builds their current commit.

`Engine nightly` runs daily. A scheduled run cancels itself when the newest
commit is at least 25 hours old; manually started runs always build. This keeps
the latest successful scheduled run attached to downloadable artifacts.

Both use the reusable `Engine build` workflow. It runs one job per Windows
platform and configuration, four by default, each on its own Windows runner
with Visual Studio 2022. A job configures and builds its platform with the
commands above, runs CTest, and uploads the executable, language library,
symbol file, and license notices. Artifact names contain the platform,
configuration, and short commit, as in `opents-x64-Release-ab12cd3`. Linker
maps are omitted because the symbol files are sufficient. The same workflow
builds and tests macOS arm64 and x86_64 Debug and Release, verifies each
executable's architecture and macOS 15.0 deployment target, and uploads the
runtime files in permission-preserving archives. A failure on any platform
fails the workflow.
After a successful pull-request build, `Engine build comment` maintains one
pull-request comment with direct nightly.link downloads.

Publishing a GitHub release runs `Engine release`. It builds the release commit
for every platform with `-DOPENTS_OFFICIAL_BUILD=ON`. Each Windows zip holds
`Game.exe`, `Language.dll`, `Game.pdb`, and the project and third-party license
notices and is named after the release tag and the platform, such as
`OpenTS-v0.2.0-x64.zip`. The macOS arm64 and x86_64 zips hold `Game` and the
Win32 build's `Language.dll`. It attaches all of them to the release, and
appends notes generated from the manual's change records by
`python manual/tools/manage.py release-notes`. See
[Maintaining](../manual/MAINTAINING.md) for the full release procedure.

CI collects the uploaded artifacts from `build/bin/<configuration>/`.

## Verification boundary

The Windows matrix was verified on September 11, 2026 with CMake 4.3.3,
Visual Studio 2022 Community 17.14.37614.0, MSVC 19.44.35228, and Windows SDK
10.0.26100. Fresh Win32 and x64 builds completed successfully in both
configurations, and CTest passed all 40 tests in each of the four. The builds
retain inherited MSVC warnings; warnings are not treated as errors, but
contributions should not add new warnings.

The macOS matrix was verified on September 4, 2026 with macOS 26.6.2, Xcode
26.6, Apple clang, CMake 3.31.7, and Ninja 1.12.1. Fresh arm64 and x86_64 Debug
and Release builds targeting macOS 15.0 completed successfully, and their CTest
suites passed with the hardware audio playback test skipped. CI also
builds and tests both architectures and configurations on macOS 15 runners.

This verifies only that the supported toolchains compile, link, pass the
tests, and produce the listed files. Runtime behavior requires separate play
testing, and the x64 build has none of that history: only the Win32 build has
been played on Windows. Asset-backed play testing on macOS 26.6.2 covered arm64
and an x86_64 build under Rosetta 2 through campaign launch and a basic
movement order. It did not cover macOS 15, native Intel hardware, save and
load, mission completion, or mixed Windows/macOS multiplayer.

The repository contains no maps, movies, audio, or other original game assets.
Keep legally obtained runtime data local and outside version control. The
repository safety rules are in [CONTRIBUTING.md](../CONTRIBUTING.md).
