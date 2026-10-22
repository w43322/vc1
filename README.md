# Virtual City decomp

A playable decompilation and reimplementation of `VirtualCity.Unwrapped.exe`, built with early VC9 SP1 x86 `/GL /LTCG` and compared against the original with [reccmp](https://github.com/isledecomp/reccmp).

**Both campaign and sandbox modes are playable.** Function mapping and assembly matching are still in progress; the project has not yet reached 100% function coverage or a 100% binary match.

Original binary (keep it next to the repo, not in Git):

`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`

SHA-256: `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`

Game entry is stdcall `WinMain` at `0x00486b00` (`src/host/win_main.cpp`). The PE entry is stock VC9 `WinMainCRTStartup`. Subsystem is WINDOWS.

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

The recorded runtime reaches the main menu, intro comic and Level_01_01, with graphics, particles and BASS music/sound. This is not a full-game validation; transport gameplay still needs runtime exercise. `run-debug.py` leaves the process running on timeout: stop that instance before another run or the game mutex will reject it. Screenshots can be black in fullscreen.

## Included libraries and tools

The build includes Squirrel 2.2.3, SqPlus, klibc's scanf family, libtga, jpgd 0.93b, libpng 1.0.2, zlib 1.1.3 inflate and the LZMA SDK 4.57 C decoder subset (7z/LZMA/BCJ/BCJ2). Integration and compile options live in `cmake/`. The LZMA library is built for comparison; the host `bar` driver is still a stub. BASS and D3D8 use generated import libraries; the original resources and BASS DLL are copied locally into the build, not committed.

Keep the build/compare, Ghidra setup/import and runtime helpers in `tools/`. `disasm-orig.py` provides supplementary raw assembly/data views; `crt-ident.py` finds candidate CRT symbols by byte patterns and still requires Ghidra evidence and reccmp verification. `extract-rsrc.py` is called by CMake, and `report-progress.py` by the compare script.

## Toolchain

- MSVC 1500 (VC9 SP1), 32-bit, RelWithDebInfo.
- `/MT /GS- /GR- /O2 /Ob2 /Z7 /GL` (no `/EH`) and linker `/LTCG /OPT:NOREF /OPT:NOICF`. Do not drop `/GL /LTCG`.
- VC9-era C++ only in `src/` (no `auto`, `nullptr`, range-for, `override`, lambdas).
- Vendored library bodies stay upstream except documented, evidence-backed game/VC9 patches; mapping additions are comments only. Do not add inline asm. Do not invent mappings.

## Do not commit

`ghidra/`, `build/`, `.venv/`, `reccmp-user.yml`, `reccmp-build.yml`, and the original game tree / zip. `Virtual-City_Win_EN_Full/` is intentionally not git-ignored (so editor search sees the decompiled scripts); stage files by name and never add anything under it.
