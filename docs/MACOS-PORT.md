# macOS port plan

This page records the high-level route to a natively running macOS build. It
orders the work and names exit criteria; it does not schedule it. Each phase
decomposes into small, individually reviewable changes. [Building
OpenTS](BUILDING.md) stays the authority on supported targets. Phase 6 records
the explicit decision that made macOS supported after the port reached verified
runtime behavior.

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
| Save streams and swizzling | The game state is written field by field, and pointer members were written as native-width swizzle IDs. | Superseded upstream: the save container no longer uses COM storage, and swizzle identities travel as four bytes numbered in save order ([The saved game format](SAVE-FORMAT.md)). |
| Network packets and replays | `EventClass` is a packed wire record and replays write the same record. | Its 46-byte legacy layout remains the contract. `NetContract` checks the size and the compressed and uncompressed decoders on both build architectures. |
| TMP tile data | The tile table stores 32-bit offsets from the start of the file, not native pointers. Its records use the 52-byte Microsoft bitfield layout. | The in-memory view keeps table entries as `uint32_t`, resolves each offset when accessed, and preserves the record layout on Apple clang. |
| Window procedures, timers, and VQA callbacks | Several Win32 callbacks and user-data slots carried pointers through `int`, `long`, or `DWORD`. | Callback signatures and storage use `INT_PTR`, `LONG_PTR`, `DWORD_PTR`, `WPARAM`, `LPARAM`, `intptr_t`, or `uintptr_t` as required by the API. |
| Rendering scratch buffers | Z, alpha, and isometric drawing paths used 32-bit integers for addresses and pointer arithmetic. | Address-bearing values use pointers or `uintptr_t`; sizes and pixel offsets remain fixed-width values. |
| Crash reports | The stack walker and register report assumed an x86 `CONTEXT`. | The reporter selects the x86 or AMD64 context fields and machine type at compile time. |
| x86 assembly | MASM objects and several inline assembly fast paths cannot be built for x64. | The x64 proving build excludes MASM, uses the templated C++ blitters, replaces the timestamp reads with the compiler intrinsic, and supplies C++ LCW and VQA paths. The Win32 build still selects the inherited assembly where it remains. Phase 2 removes that dependency from every configuration. |

### Phase 1 save-format decision

Phase 1 first recorded the writer's pointer width in the save header and
refused saves from the other width. The upstream save container that replaced
COM structured storage writes every swizzle identity as four bytes, so the
game state no longer depends on pointer width, and that header field was
dropped. [The saved game format](SAVE-FORMAT.md) owns the current layout.
`LCWRoundTrip` checks the x64 compressor fallback, and `VqaDecode` pins the 4×2
row layout of the x64 VQA decoder.

### Phase 1 proving configuration and status

Windows x64 has since become a supported upstream target. The experimental
clang-cl toolchain accepts `-DOPENTS_WINDOWS_ARCH=x64`; x86 remains its
default. [Building OpenTS](BUILDING.md) contains the commands.

Phase 1 is complete. Debug and Release compile and link as x64 PEs with the
clang-cl proving configuration. Strict pointer-cast diagnostics pass, and the
native `LCWRoundTrip` and `VqaDecode` tests cover the portable
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
winsock header; and a PE resource reader for `Language.dll` strings and
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
macOS arm64; [Building OpenTS](BUILDING.md#macos-build) records
the commands.

## Phase 5 — macOS bring-up

Bring the game up on macOS x86_64 first, which also runs under Rosetta 2 on
Apple Silicon. Follow with native arm64, replacing the SSE2 assumption with
NEON mappings or scalar fallbacks. Verify at runtime with game assets;
automated tests must stay free of proprietary assets.

Exit: the game plays natively on macOS.

Status: runtime exit met on native arm64. A build from `build/macos` starts,
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

The dialog shim in `code/platform/macos/wincontrols.cpp` now creates child
controls from the PE dialog template — classic and extended, with class,
identifier, text, style, and rectangles scaled at the 6-by-13 dialog-unit base
the engine's layout reference expects — and `DialogBoxParam` runs a modal
message loop that returns the `EndDialog` result. Template lookup searches
every loaded resource library because the executable is not a PE module on
macOS; the measuring dialog the executable would provide is mirrored in the
shim. Shim control classes now use the Win32 mixed-case names the engine
compares against. The control layer tracks the dialog hierarchy (`GetTopWindow`,
`GetWindow`, `BringWindowToTop`), screen-space coordinate translation
(`GetWindowRect`, `MapWindowPoints`), mouse-event routing to child controls,
button and checkbox state changes, and keyboard navigation in `IsDialogMessage`
(`VK_RETURN`, `VK_ESCAPE`, `VK_TAB`). The `WinControls` test pins the template
layout, dialog-unit scaling, hierarchy traversal, button and checkbox clicks,
keyboard commands, and the modal loop without game assets.

The macOS window layer forces activation only until AppKit reports that the
window became key. Later application switches do not pull the game back into
focus, and a borderless full-screen window stays at the normal window level so
the selected application can appear above it. Closing the main window or
choosing Quit requests an exit from the AppKit callback; the event pump runs
the atexit-registered `Prog_End` teardown after the callback returns. The engine
ignores `WM_CLOSE` by design, so this still quits immediately without the
engine's own in-mission prompts. The `MacOSWindow` test pins the activation
latch, full-screen window level, and deferred quit boundary without game
assets.

Asset-backed Debug arm64 and x86_64 `-SPAWN` runs load `GDI1A.MAP`, render its
terrain and radar, and advance the simulation. The arm64 session remained live
for more than three minutes; the corrected x86_64 session also remained live
under Rosetta 2 for more than three minutes. Direct mouse input selected an
infantry unit and issued a movement order in both architectures. This verifies
the campaign launch and basic-play path, not mission completion, save and load, or
every dialog-based screen.

Campaign loading exposed three terrain compatibility faults. The macOS
`_makepath` shim omitted the separator before theater suffixes, so it searched
for names such as `CLEAR01TEM` instead of `CLEAR01.TEM`; `MacOSCompat` pins the
correct separator behavior. Apple clang also packed `IsoTileRecord` bitfields
differently from the 52-byte TMP file record, so the macOS view now selects the
Microsoft layout and checks the record size and offsets at compile time.
Finally, empty tile sets and absent sub-tile records now avoid invalid
arithmetic and indexing, while `Cell_Render_Rect` keeps the cell's base bounds
when the optional record is absent. The x86_64 run also exposed a Windows-to-
macOS `long` width mismatch in the credits format; the readout now formats its
32-bit value correctly on both architectures.

No enabled first-party SSE or MMX path remains in the arm64 build; bgfx selects
its NEON path, and `SosParity` checks the portable audio decoder against
recorded assembly output in Windows and macOS test builds. The Phase 5 exit is
met for the tested campaign path. The support decision does not broaden this
runtime evidence.

## Phase 6 — Support decision

Decide whether macOS becomes a supported target: update
[Building OpenTS](BUILDING.md) and continuous integration, and record the
stance on cross-platform multiplayer, where floating-point determinism
between compilers and architectures is unresolved.

Status: complete. macOS 15 or newer is supported on arm64 and x86_64 with
Apple clang, CMake, and Ninja. Continuous integration builds and tests Debug
and Release for both architectures and verifies their executable architecture
and deployment target. Mixed Windows/macOS multiplayer remains unsupported
until cross-compiler and cross-architecture simulation determinism is
established. macOS-to-macOS multiplayer has not received runtime testing.

## Execution notes for agents

- Work the phases in order, one small reviewable change at a time.
- Spawn subagents where parallel work helps: inventories, independent
  replacements, reviews, and verification runs.
- `codex` with computer use may drive the running game or other GUI tools
  when a verification step needs it.
- Original game assets live in the local `Run/` directory and are gitignored;
  never commit them.
