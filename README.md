# Virtual City decomp

A playable decompilation and reimplementation of `VirtualCity.Unwrapped.exe`, built with early VC9 SP1 x86 `/GL /LTCG` and compared against the original with [reccmp](https://github.com/isledecomp/reccmp).

**Both campaign and sandbox modes are playable.** Function mapping and assembly matching are still in progress; the project has not yet reached 100% function coverage or a 100% binary match.

Original binary (keep it next to the repo, not in Git):

`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`

SHA-256: `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`

Game entry is stdcall `WinMain` at `0x00486b00` (`src/kdcore/win_main.cpp`). The PE entry is stock VC9 `WinMainCRTStartup`. Subsystem is WINDOWS.

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

## Source layout

Everything that was inside the original executable is shared recovered code;
everything it imported from a DLL is provided per target.

- `src/kdcore/`: the recovered G5 OpenKODE core: events, files and timers, log,
  window procedure and input, threads, network, crash handler, `WinMain` and
  globals. `kdcore/platform.h` is the engine's view of it, free of Windows
  headers. `cmake/kdcore.cmake` keeps the reference translation-unit order.
- `src/imports/win32/`: the reference build links the real Windows DLLs, plus
  generated BASS/d3d8 import libraries (`cmake/imports-win32.cmake`).
- `src/imports/sdl3/`: the SDL3 build. `include/` stands in for the Windows SDK
  headers with just the declarations the game uses, and each imported DLL is one
  file implemented over SDL3 (`kernel32.cpp`, `user32.cpp`, `gdi32.cpp`,
  `shell32.cpp`, `shlwapi.cpp`, `winmm.cpp`, `ws2_32.cpp`, `d3d8.cpp`, and
  `bass.cpp` over SDL3_mixer). The CRT entry point calls the recovered `WinMain`;
  SDL events reach the recovered window procedure as Win32 messages, and the
  recovered display code draws through the D3D8 implementation.

Recovered sources do not branch on the target. `crt.cpp` in each imports folder
holds the two file driver slots that need the VC9 CRT's `FILE` internals.

## SDL3 build

Build with modern Visual Studio C++ tools (2017 or newer), Ninja, Python 3 and
CMake 3.16+:

```powershell
.\tools\build-sdl3.cmd
```

The script selects x86 and prefers Visual Studio's bundled CMake. `VC1_CMAKE`
overrides it. CMake uses installed SDL3 3.4.16 / SDL3_mixer 3.2.4 SDKs or downloads
the pinned official sources into `build/`. For offline builds, set
`VC1_SDL3_SOURCE_DIR` and `VC1_SDL3_MIXER_SOURCE_DIR` to extracted source trees.
`tools/msvc-showincludes.py` normalizes localized compiler include messages so
Ninja tracks header changes; `build-sdl3.cmd --clean-first` forces a full rebuild.

Output: `build/sdl3/bin/VirtualCity.SDL3.exe`. Run from the same game data directory,
or use this from the repository root:

```powershell
.\.venv\Scripts\python.exe tools/run-debug.py --exe build/sdl3/bin/VirtualCity.SDL3.exe --map build/sdl3/bin/VirtualCity.SDL3.map --timeout 60 --keep
```

Interact manually; `--keep` leaves the window open. The game starts full screen
like the original; switch to windowed mode in its options. At configure time,
`tools/extract-sdl-resources.py` converts the original's cursor/icon resources into
`build/sdl3/bin/platform-resources/`. These generated game assets stay local.
SDL3_mixer is built statically by default; BASS.dll is not required by this target.
The executable imports only SDL3.dll and KERNEL32.dll.

The SDL build is currently supported with Windows MSVC x86. Engine pointer
handles, layout checks and the remaining VC9-specific code still need work for
other compilers or 64-bit builds. The default `tools/build-vc9.cmd` disables SDL3
and preserves `/GL /LTCG` and reccmp.

## Addons

`VC1_LOG` and `VC1_MODS` are CMake options, on by default for the SDL3 build and
off for the reference build (turning them on there makes it non-comparable).
`VC1_LOG` sends KdLog text to `OutputDebugStringA` (the console in the SDL3
build); the original release build drops it.

## Mods (`VC1_MODS`)

The game can replace atlas artwork with PNGs. Mods are local content in the
git-ignored `mods/` folder at the repository root; the build copies every folder
in it beside the executable and seeds `mods/enabled.txt` once (one mod folder per
line, `#` comments; later entries win). Edit `build/sdl3/bin/mods/enabled.txt` and
restart to switch mods; an empty or missing list restores the original artwork.
The `VC1_MODS` environment variable selects a different mods directory.

A mod is a folder with a `mod.json` that maps tile names to PNGs:

```json
{"id":"advisor","version":"4.0","tiles":{"SCORING_FULL_GIRL_01":"textures/advisor.png"}}
```

`id` must equal the folder name. Tile names resolve through the live
`Tiles`/`TileImages` tables and geometry comes from `res/Tiles.bin`; static and
composite tiles (commands 0, 3, 4) are supported. Each PNG is scaled to the tile's
logical canvas and written into the matching atlas rectangles when the texture loads,
so every tile that shares a region (flipped variants, composites) changes too.
Use PNGs at the logical tile size for the sharpest result. Original files are
never written. `tools/mod-tiles.py NAME [--export out.png]` prints a tile's
geometry or exports the original locally; keep exported game artwork out of Git.

The advisor portraits use three regions: `INTERFACE_FACE_01` (217x339 main-menu
half portrait), `SCORING_FULL_GIRL_01` (192x484 full portrait for briefing and
level results; its upper 159x183 fragment is also the tutorial face, including the
mirrored `INTERFACE_FACE_02_H`) and `INTERFACE_FACE_04` (56x60 head icon, also
inside the `BUTTONS_HINT_*` hint button).

## Toolchain

- MSVC 1500 (VC9 SP1), 32-bit, RelWithDebInfo.
- `/MT /GS- /GR- /O2 /Ob2 /Z7 /GL` (no `/EH`) and linker `/LTCG /OPT:NOREF /OPT:NOICF`. Do not drop `/GL /LTCG`.
- Matching/shared source stays VC9-compatible. Only `src/imports/sdl3/` uses C++17.
- Vendored library bodies stay upstream except documented, evidence-backed game/VC9 patches; mapping additions are comments only. Do not add inline asm. Do not invent mappings.

## Do not commit

`ghidra/`, `build/`, `.venv/`, `reccmp-user.yml`, `reccmp-build.yml`, and the original game tree / zip. `Virtual-City_Win_EN_Full/` is intentionally not git-ignored (so editor search sees the decompiled scripts); stage files by name and never add anything under it.
