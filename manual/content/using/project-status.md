---
title: Project status
summary: OpenTS supports Windows and macOS, with broader runtime coverage on Windows.
category: getting-started
source_files:
  - README.md
  - docs/BUILDING.md
related:
  - type: using
    id: build-and-run
  - type: using
    id: game-data
---

OpenTS continues the reconstructed Tiberian Sun engine as an actively developed project. Each release provides a standalone `Game.exe` and its `Language.dll` for 32-bit and 64-bit Windows, published with the engine source and this manual.

Release 0.1.0 runs the full Tiberian Sun 2.03 Firestorm game, with the fixes and changes listed in its release notes. The GDI, Nod and Firestorm campaigns, skirmish, and saving and loading have had full play-through testing. LAN multiplayer is playable and has had more limited testing. No user-visible regression from the original game is currently known. CnCNet play is not yet supported.

On macOS, arm64 and x86_64 builds have loaded the first GDI campaign map, rendered its terrain and radar, advanced the simulation, and accepted unit selection and movement. The x86_64 runtime check used Rosetta 2. These checks predate the current window and interface layers; the current arm64 build has been checked only through startup to the main menu. Campaign completion, save and load, native Intel hardware, and interface screens beyond the tested path have not received runtime verification. Mixed Windows/macOS multiplayer is unsupported pending determinism evidence. macOS-to-macOS LAN play has not been tested.

## Releases and developer builds

Stable releases are published on the project's GitHub releases page. Nightly developer builds carry the latest merged changes without release validation, and their downloads expire after 90 days.

OpenTS does not distribute the original game assets. They come from an existing Tiberian Sun installation; [Game data](/using/game-data/) covers where a developer build finds them.

## Toolchain and targets

- CMake with Visual Studio 2022
- 32-bit and 64-bit Windows
- macOS 15 or newer on arm64 or x86_64 with Apple clang and Ninja
- C++20
- Debug and Release configurations

Both platforms build in both configurations with the documented toolchain. [Build and run](/using/build-and-run/) gives the commands.
