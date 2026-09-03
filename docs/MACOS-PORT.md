# macOS port plan

This page records the high-level route to a natively running macOS build. It
orders the work and names exit criteria; it does not schedule it. Each phase
decomposes into small, individually reviewable changes. [Building
OpenTS](BUILDING.md) stays the authority on supported targets: macOS becomes
supported only by an explicit decision once the port reaches verified runtime
behavior.

## Ground rules

- Preserve the supported Visual Studio 2022 Win32 Debug and Release
  configurations and behavior. Report Windows checks that were not run; they
  are not phase exit criteria for the macOS port.
- Saves, replays, network packets, deterministic simulation, and
  layout-sensitive structures are compatibility boundaries. An intentional
  break needs an explicit decision, tests, and a migration path.
- Replace platform code behind existing interfaces instead of rewriting
  callers, in line with [Project direction](DIRECTION.md).

## Phase 1 — 64-bit readiness

macOS runs only 64-bit code, so this phase gates everything else and carries
the most risk. Inventory pointer-size assumptions: pointer-in-integer casts,
layout-sensitive structures, and the `IStream` save path with pointer
swizzling (`code/saveload.cpp`, `code/swizzle.h`). Decide and record what
happens to the save format on a 64-bit build. Add a Windows x64 configuration
as the proving ground.

Exit: the engine compiles and links for a 64-bit ABI, the pointer-width
inventory is closed, and the save-format decision is documented and tested.

### Phase 1 pointer-width inventory

| Boundary | Finding | Phase 1 treatment |
| --- | --- | --- |
| Save streams and swizzling | `CONTENTS` is written field by field, but it includes native-width pointer values used as swizzle IDs and other layout-sensitive members. The pointer width therefore changes the stream, even though COM storage itself is architecture-neutral. | Save headers now record the writer's pointer width. Load admission rejects a different width before reading `CONTENTS`. |
| Network packets and replays | `EventClass` is a packed wire record and replays write the same record. | Its 46-byte legacy layout remains the contract. `NetContract` checks the size and the compressed and uncompressed decoders on both build architectures. |
| TMP tile data | The tile table stores 32-bit offsets from the start of the file, not native pointers. | The in-memory view keeps those entries as `uint32_t` and resolves each offset when accessed instead of rewriting the mapped data as pointers. |
| Window procedures, timers, and VQA callbacks | Several Win32 callbacks and user-data slots carried pointers through `int`, `long`, or `DWORD`. | Callback signatures and storage use `INT_PTR`, `LONG_PTR`, `DWORD_PTR`, `WPARAM`, `LPARAM`, `intptr_t`, or `uintptr_t` as required by the API. |
| Rendering scratch buffers | Z, alpha, and isometric drawing paths used 32-bit integers for addresses and pointer arithmetic. | Address-bearing values use pointers or `uintptr_t`; sizes and pixel offsets remain fixed-width values. |
| Crash reports | The stack walker and register report assumed an x86 `CONTEXT`. | The reporter selects the x86 or AMD64 context fields and machine type at compile time. |
| x86 assembly | MASM objects and several inline assembly fast paths cannot be built for x64. | The x64 proving build excludes MASM, uses the templated C++ blitters, replaces the timestamp reads with the compiler intrinsic, and supplies C++ LCW and VQA paths. The Win32 build still selects the inherited assembly where it remains. Phase 2 removes that dependency from every configuration. |

### Phase 1 save-format decision

Saves are architecture-specific. Save-header format version 2 adds a `Pointer
Size` property. New Win32 saves record `4`; Windows x64 saves record `8`.
Headers from format version 1 have no property and are treated as Win32 saves.
The load dialog, direct load path, and spawner resume path require both the
engine version and pointer width to match.

This preserves existing Win32 saves on Win32 and prevents a 64-bit process
from interpreting 32-bit object layouts or truncated swizzle IDs. Win32 and
x64 saves do not load across architectures. There is no safe automatic
converter because `CONTENTS` is an architecture-dependent member stream, not
a portable versioned schema. Players migrating an existing campaign must
finish it with a matching Win32 build; the x64 build starts a separate save
history. A future cross-architecture format requires an explicit field schema
and converter.

`SaveCompat` pins the admission matrix without COM or game assets. `LCWRoundTrip`
checks the x64 compressor fallback, and `VqaDecode` pins the 4×2 row layout of
the x64 VQA decoder:

| Reader | Legacy or version 2 Win32 save | Version 2 x64 save |
| --- | --- | --- |
| Win32 | Accept | Reject |
| x64 | Reject | Accept |

### Phase 1 proving configuration and status

Visual Studio can configure the proving build with `-A x64`. The experimental
clang-cl toolchain accepts `-DOPENTS_WINDOWS_ARCH=x64`; x86 remains its
default. [Building OpenTS](BUILDING.md#phase-1-x64-proving-build)
contains the commands and validation boundary.

Phase 1 is complete. Debug and Release compile and link as x64 PEs with the
clang-cl proving configuration. Strict pointer-cast diagnostics pass, and the
native `SaveCompat`, `LCWRoundTrip`, and `VqaDecode` tests cover the portable
logic without requiring Windows or proprietary game assets. These results are
compile, link, and focused test evidence; they make no Windows runtime claim.

## Phase 2 — Portable replacements for 32-bit assembly

Replace `code/winasm.asm`, `code/unvq_asm.asm`, and the `__asm` blocks in
`code/lcw.cpp`, `code/blitblit.h`, `code/rlerle.h`, and `code/keyboard.cpp`
with C++ verified against the current output.

Exit: no configuration requires MASM or inline assembly.

Status: the C++ replacements are in place and no build configuration
references MASM objects or `__asm` blocks. `LCWRoundTrip` and `VqaDecode`
cover the compression and video paths. The Visual Studio Win32 builds have
not been re-verified against this state.

## Phase 3 — Platform abstraction

Move the remaining Windows dependencies behind a thin platform layer:

- Window, message loop, input, cursor, timers, and the Win32 dialogs
  (`code/windlg.cpp`); a cross-platform windowing library such as SDL is the
  expected backend.
- Audio: put `code/dsaudio.cpp` behind an audio interface and stop leaking
  DirectSound types through `code/sound.h` into gameplay code.
- Networking: port `code/wspudp.cpp` to BSD sockets.
- COM: a minimal `IUnknown`/`IStream` shim so save and load work without
  Windows COM.
- Strings: replace the `Language.dll` resource loading with a portable
  loader.
- Video: presentation already goes through bgfx, which has a Metal backend;
  retire or wrap the remaining DirectDraw surface types (`code/dsurface.cpp`,
  `code/ownrdraw.cpp`, `code/srfcache.cpp`).

Exit: platform-dependent callers use the new interfaces, portable backends
cover each interface, and the Windows configurations still compile against
them.

Status: instead of rewriting callers onto new interfaces, the bring-up keeps
the Win32 API surface and supplies a compatibility layer under
`code/platform/macos`: an AppKit window, message queue, and input path; an
AudioQueue backend behind the DirectSound interface; BSD sockets behind the
winsock header; a COM `IStorage`/`IStream` implementation for saves
(`MacOSStorage`); and a PE resource reader for `Language.dll` strings and
dialogs (`PEResource`). Whether the thin-interface extraction still happens
per subsystem remains open; the compatibility layer is the current working
boundary. The Windows configurations have not been re-verified against this
state.

## Phase 4 — Non-MSVC toolchain

Extend the CMake build to a clang toolchain targeting non-Windows systems.
The experimental clang-cl cross build is groundwork but still uses the MSVC
ABI and Windows headers; this phase removes those dependencies.

Exit: the tree compiles and links with Apple clang.

Status: met. The tree configures, compiles, and links with Apple clang on
macOS arm64; [Building OpenTS](BUILDING.md#experimental-macos-build) records
the commands.

## Phase 5 — macOS bring-up

Bring the game up on macOS x86_64 first, which also runs under Rosetta 2 on
Apple Silicon. Follow with native arm64, replacing the SSE2 assumption with
NEON mappings or scalar fallbacks. Verify at runtime with game assets;
automated tests must stay free of proprietary assets.

Exit: the game plays natively on macOS.

Status: in progress on native arm64. A build from `build/macos` starts,
initializes the Metal renderer and AudioQueue output, decrypts the bootstrap
mixfiles, loads fonts and `Language.dll` resources, and reaches a rendered,
interactive main menu with music. Bring-up so far fixed several width- and
platform-dependent bugs the Win32 build never exercised: the SHA-1 digest
union used `unsigned long` (`SHADigest` now uses `unsigned int`, pinned by
`SHADigest`); the Base64 decoder keyed off `BIG_ENDIAN`, which BSD headers
define unconditionally (now `__BIG_ENDIAN__`); and `MixFileClass::Offset`
uppercased a string literal in place. The macOS window now accepts key
status so the game receives its first focus event, and the main menu responds
to real mouse and keyboard input.

Known limitation past the menu: the graphic menu and the game's own
owner-drawn dialogs run their own input loops and work, but the generic
`DialogBoxParam`/`CreateDialogIndirectParam` shim in
`code/platform/macos/wincontrols.cpp` is not modal. It creates the dialog,
reads a result, and destroys it without pumping messages or laying out the
child controls from the PE dialog template, so screens that depend on a Win32
modal dialog are not yet interactive. A modal message loop and template-driven
control layout are the next bring-up step. Interactive gameplay past the menu
and the arm64 SIMD work are also unverified; the SSE2 paths still need NEON or
scalar fallbacks. Driving the menu from an automated tool needs Accessibility
permission for the controlling process; real keyboard and mouse input reach
the game normally.

## Phase 6 — Support decision

Decide whether macOS becomes a supported target: update
[Building OpenTS](BUILDING.md) and continuous integration, and record the
stance on cross-platform multiplayer, where floating-point determinism
between compilers and architectures is unresolved.

## Execution notes for agents

- Work the phases in order, one small reviewable change at a time.
- Spawn subagents where parallel work helps: inventories, independent
  replacements, reviews, and verification runs.
- `codex` with computer use may drive the running game or other GUI tools
  when a verification step needs it.
- Original game assets live in the local `Run/` directory and are gitignored;
  never commit them.
