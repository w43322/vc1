# Virtual City decomp

A playable decompilation and reimplementation of `VirtualCity.Unwrapped.exe`, built with early VC9 SP1 x86 `/GL /LTCG` and compared against the original with [reccmp](https://github.com/isledecomp/reccmp).

**Both campaign and sandbox modes are playable.** Function mapping and assembly matching are still in progress; the project has not yet reached 100% function coverage or a 100% binary match.

Original binary (keep it next to the repo, not in Git):

`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`

SHA-256: `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`

Game entry is stdcall `WinMain` at `0x00486b00` (`src/platform/win32/win_main.cpp`). The PE entry is stock VC9 `WinMainCRTStartup`. Subsystem is WINDOWS.

Every recovered function needs Ghidra evidence from that original and a reccmp asm diff of the recompiled bytes. Put evidence in source comments.

## First-time setup

1. Place the original install so the EXE path above exists.
2. Install Visual Studio 2008 SP1 (VC9) x86. Optional: set `VC9_ROOT` if it is not under `%ProgramFiles(x86)%\Microsoft Visual Studio 9.0`.
3. Install Python compatible with `tools/requirements-reccmp.txt`, CMake 3.13+ and Ghidra 12.x. `tools/open-ghidra.ps1` / `init-ghidra.ps1` / `import-ghidra.ps1` default to `D:\ghidra_12.1.2_PUBLIC`.
4. Create the venv and reccmp configs:

```powershell
.\tools\setup-reccmp.ps1
```

5. If `ghidra/VirtualCity.gpr` does not exist yet:

```powershell
.\tools\init-ghidra.ps1
```

6. Build with VC9 and compare:

```powershell
.\tools\build-vc9.cmd
.\tools\compare-reccmp.ps1
```

`setup-reccmp.ps1` writes local `reccmp-user.yml` (original EXE path). CMake writes `build/vc9/reccmp-build.yml` (recompiled EXE + PDB). Do not commit either file.

The pinned `vendor/reccmp` submodule uses the [w43322 fork](https://github.com/w43322/reccmp). Setup installs that checkout in editable mode. `init-ghidra.ps1` checks the original hash and performs initial analysis only; recovered names are imported after building through `import-ghidra.ps1`.

## Agent round

Work one addressed function or a tight cluster per round. Typical loop:

```powershell
.\tools\open-ghidra.ps1
.\tools\compare-reccmp.ps1 -ResetBaseline # with an existing build, before edits
# MCP is read-only. For each address: recover it, then write src/ before
# dumping the next function. Keep reading and writing in the same stretch.
# Fix already-mapped source, flags, or workflow if a read shows they are wrong.
.\tools\build-vc9.cmd
.\tools\compare-reccmp.ps1
# leave Ghidra open through compare; close only for import
.\tools\close-ghidra.ps1
.\tools\import-ghidra.ps1
```

`import-ghidra.ps1` is the only allowed Ghidra write path: it runs official `reccmp-ghidra-import` and copies names, prototypes, and types from the recompiled PDB. Close the Ghidra GUI immediately before that script; headless import fails if it is still open.

See `AGENTS.md` for ABI rules, marker order, and what “done” means.

## reccmp

Target id is `VC1` (`reccmp-project.yml`). `tools/compare-reccmp.ps1` runs, from `build/vc9`:

| Tool | Output | Fails the script? |
| --- | --- | --- |
| `reccmp-decomplint` | marker syntax / order | yes |
| `reccmp-reccmp` | `reports/VC1.json`, `.html`, `.svg` | yes |
| `reccmp-aggregate --diff` | `reports/VC1.diff.txt` when a previous JSON exists | no |
| `reccmp-aggregate --diff` | `reports/VC1.base-diff.txt` against the round baseline | no |
| `reccmp-datacmp` | `reports/VC1-data.txt` | no |
| `reccmp-vtable` | `reports/VC1-vtable.txt` | no |
| `reccmp-roadmap` | `reports/VC1-roadmap.csv` / `.txt` | no |
| `report-progress.py` | console summary from the generated reports | no explicit exit-code check |

After a successful asm compare, the JSON is copied to `reports/VC1.prev.json` so the next run can show what changed. Use `-ResetBaseline` at the start of a round (with an existing build) or after a commit; `-Full` echoes the full tool output.

Data, vtable, and roadmap mismatches are expected while layouts are still incomplete. Report **per-function** match rates from asmcmp; do not treat them as whole-game progress.

Near-match stack layout:

```powershell
.\tools\stackcmp-reccmp.ps1 0x00486b00
```

Single-function asm diff (from the repository root):

```powershell
.\tools\diff-reccmp.ps1 0x00486b00
```

`reccmp-verexp` is for DLL exports; this target is an EXE, so it is unused. CRT and libraries without source use `config/verified.csv`; vendored libraries use source annotations. Do not hide mismatches with `report: ignore-functions:`.

### Markers

Place these immediately above the definition (or, for `STRING`, above the line that contains the literal):

```cpp
// FUNCTION: VC1 0x00486b00
// STUB: VC1 0x00......          // empty / dishonest body; excluded from asm compare
// GLOBAL: VC1 0x004ed13c
// VTABLE: VC1 0x004cfb98
// STRING: VC1 0x004d0c48
// LINE: VC1 0x00......          // pin a specific instruction to a source line
```

`FUNCTION` markers must be in address order **within that file**. `STUB` is for bodies that should not be compared yet; do not convert incomplete-but-honest functions to stubs just to raise the score. Libraries without source use `config/verified.csv` (`address|type|name|symbol`). Vendored library mappings belong on implementations or as header name comments; see `AGENTS.md` for overloads, templates and folded bodies.

## Running the recompiled exe

The working directory must be `Virtual-City_Win_EN_Full/VirtualCity`. Helpers (Python from `.venv`):

```powershell
.\.venv\Scripts\python.exe tools\run-debug.py --timeout 20          # log + crash stack
.\.venv\Scripts\python.exe tools\run-debug.py --bp 414020 415400   # breakpoints at recompiled VAs (.map)
.\.venv\Scripts\python.exe tools\run-screenshot.py build\shot.png 8
.\tools\decompile-nut.ps1   # compiled .nut scripts -> Virtual-City_Win_EN_Full/nut-src/
```

`decompile-nut.ps1` uses [w43322/NutCracker](https://github.com/w43322/NutCracker) (branch `sq2-vc1`), cloned and built into `build/NutCracker`. Current runtime status and the next blockers are in `AGENTS.md` ("Run the game").

The recorded runtime reaches the main menu, intro comic and Level_01_01, with graphics, particles and BASS music/sound. This is not a full-game validation; transport gameplay still needs runtime exercise. `run-debug.py` stops the process on timeout unless `--keep` is supplied; close a kept instance before another run or the game mutex will reject it. Screenshots can be black in fullscreen.

## Included libraries and tools

The build includes Squirrel 2.2.3, SqPlus, klibc's scanf family, libtga, jpgd 0.93b, libpng 1.0.2, zlib 1.1.3 inflate and the LZMA SDK 4.57 C decoder subset (7z/LZMA/BCJ/BCJ2). Integration and compile options live in `cmake/`. The LZMA library is built for comparison; the host `bar` driver is still a stub. BASS and D3D8 use generated import libraries; the original resources and BASS DLL are copied locally into the build, not committed.

Keep the build/compare, Ghidra setup/import and runtime helpers in `tools/`. `disasm-orig.py` provides supplementary raw assembly/data views; `crt-ident.py` finds candidate CRT symbols by byte patterns and still requires Ghidra evidence and reccmp verification. `extract-rsrc.py` is called by CMake, and `report-progress.py` by the compare script.

## Platform boundary

`src/host/platform.h` declares the shared KD services and event/file types without
including Windows headers. The matching host implementation and its native state
live in `src/platform/win32/`, selected by `cmake/platform-win32.cmake` in the
original translation-unit order. `host/runtime.h` is a compatibility umbrella for
remaining consumers of the Win32 runtime; shared code should use `host/platform.h`.

`host/window.h` exposes opaque KD window creation. Win32 window state, class
registration, message processing and resource cursors live in
`src/platform/win32/window.{h,cpp}`; native key/character/pointer translation lives
in `src/platform/win32/input.cpp`. `app/script.h` declares script initialization
and bindings without native types, so `app/script.cpp` no longer includes the
Windows runtime. Both current targets still select the Win32 window/input backend.

An optional SDL3 transition build uses SDL performance counters and gamepad input.
`host/clock.h` selects the counter implementation at compile time; the VC9 build
keeps its original QPC calls. SDL gamepad edges enter the existing KD event queue,
including dpad and disconnect releases. Modern CRT file mappings keep their own
handles instead of accessing VC9's private `FILE` layout.

Build with modern Visual Studio C++ tools (2017 or newer), Ninja and CMake 3.16+:

```powershell
.\tools\build-sdl3.cmd
cd build\sdl3
ctest --output-on-failure
```

The script selects x86 and prefers Visual Studio's bundled CMake. `VC1_CMAKE` can
override it. CMake uses an installed SDL3 3.4.16 SDK or downloads the pinned
official release into the build tree; set `VC1_SDL3_SOURCE_DIR` to an extracted
source directory for an offline build. The game output is
`build/sdl3/bin/VirtualCity.SDL3.exe`. Run from the same game data directory, or use
`tools/run-debug.py --exe build/sdl3/bin/VirtualCity.SDL3.exe --map build/sdl3/bin/VirtualCity.SDL3.map --timeout 60 --keep`
from the repository root. Interact manually; `--keep` leaves the window open.

This is still a Windows/x86 build using the existing window, D3D8 renderer and
BASS audio. Those backends and the remaining native calls need further separation
before a cross-platform port. The default `tools/build-vc9.cmd` explicitly disables
SDL3 and preserves `/GL /LTCG` and reccmp; the modern target has separate flags and
is not used for binary matching. SDL counter and virtual-gamepad integration tests
pass. Startup reaches the main-menu music; a subsequent manual run loads
`Level_01_05` and exits with code 0 without logged script errors or fatal exceptions.
This does not establish full campaign gameplay coverage.

## Toolchain

- MSVC 1500 (VC9 SP1), 32-bit, RelWithDebInfo.
- `/MT /GS- /GR- /O2 /Ob2 /Z7 /GL` (no `/EH`) and linker `/LTCG /OPT:NOREF /OPT:NOICF`. Do not drop `/GL /LTCG`.
- VC9-era C++ only in `src/` (no `auto`, `nullptr`, range-for, `override`, lambdas).
- Vendored library bodies stay upstream except documented, evidence-backed game/VC9 patches; mapping additions are comments only. Do not add inline asm. Do not invent mappings.

## Do not commit

`ghidra/`, `build/`, `.venv/`, `reccmp-user.yml`, `reccmp-build.yml`, and the original game tree / zip. `Virtual-City_Win_EN_Full/` is intentionally not git-ignored (so editor search sees the decompiled scripts); stage files by name and never add anything under it.
