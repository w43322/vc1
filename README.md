# Virtual City decomp

Early VC9 SP1 x86 `/GL /LTCG` reimplementation of `VirtualCity.Unwrapped.exe`, compared against the original with [reccmp](https://github.com/isledecomp/reccmp). The build is not the full game.

Original binary (keep it next to the repo, not in Git):

`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`

SHA-256: `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`

Game entry is stdcall `WinMain` at `0x00486b00` (`src/host/win_main.cpp`). The PE entry is stock VC9 `WinMainCRTStartup`. Subsystem is WINDOWS.

Every recovered function needs Ghidra evidence from that original and a reccmp asm diff of the recompiled bytes. Put evidence in source comments.

## First-time setup

1. Place the original install so the EXE path above exists.
2. Install Visual Studio 2008 SP1 (VC9) x86. Optional: set `VC9_ROOT` if it is not under `%ProgramFiles(x86)%\Microsoft Visual Studio 9.0`.
3. Install Python 3.10+ and Ghidra 12.x. `tools/open-ghidra.ps1` / `init-ghidra.ps1` / `import-ghidra.ps1` default to `D:\ghidra_12.1.2_PUBLIC`.
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

## Agent round

Work one addressed function or a tight cluster per round. Typical loop:

```powershell
.\tools\open-ghidra.ps1
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
| `reccmp-datacmp` | `reports/VC1-data.txt` | no |
| `reccmp-vtable` | `reports/VC1-vtable.txt` | no |
| `reccmp-roadmap` | `reports/VC1-roadmap.csv` / `.txt` | no |

After a successful asm compare, the JSON is copied to `reports/VC1.prev.json` so the next round can show what changed.

Data, vtable, and roadmap mismatches are expected while layouts are still incomplete. Report **per-function** match rates from asmcmp; do not treat them as whole-game progress.

Near-match stack layout:

```powershell
.\tools\stackcmp-reccmp.ps1 0x00486b00
```

Single-function asm diff (from `build/vc9`):

```powershell
..\..\.venv\Scripts\reccmp-reccmp.exe --target VC1 --verbose 0x00486b00
```

`reccmp-verexp` is for DLL exports; this target is an EXE, so it is unused. CRT and Squirrel are listed in `config/verified.csv`, not hidden with `report: ignore-functions:`.

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

`FUNCTION` markers must be in address order **within that file**. `STUB` is for bodies that should not be compared yet; do not convert incomplete-but-honest functions to stubs just to raise the score. Stock third-party / CRT symbols go in `config/verified.csv` (`address|type|name|symbol`).

## Toolchain

- MSVC 1500 (VC9 SP1), 32-bit, RelWithDebInfo.
- `/MT /GS /GR- /O2 /Ob2 /Z7 /GL` (no `/EH`) and linker `/LTCG`. Do not drop `/GL /LTCG`.
- VC9-era C++ only in `src/` (no `auto`, `nullptr`, range-for, `override`, lambdas).
- Do not edit `third_party/squirrel223`. Do not add inline asm. Do not invent mappings.

## Do not commit

`ghidra/`, `build/`, `.venv/`, `reccmp-user.yml`, `reccmp-build.yml`, and the original game tree / zip.
