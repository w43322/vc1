# Virtual City decomp

A playable decompilation of `VirtualCity.Unwrapped.exe`, rebuilt with VC9 SP1 x86 `/GL /LTCG` and compared against the original with [reccmp](https://github.com/isledecomp/reccmp). Campaign and sandbox modes are playable; assembly matching is still in progress (5282 functions, 3058 at 100%, 75.5% match-weighted).

The game is not in the repo. Place it so this file exists (SHA-256 `bdc3bc9f075a813d12078fd2eaaaa1131ba74616b11aa1444cdd5a5b83e08695`):

`Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe`

Agent rules (evidence, markers, ABI, workflow) are in `AGENTS.md`.

## Setup

1. `git clone --recursive https://github.com/w43322/vc1.git` (or `git submodule update --init`).
2. Install Visual Studio 2008 SP1 (VC9) x86 (`VC9_ROOT` if not in the default path), Python 3.10+, CMake 3.13+ and Ghidra 12.x.
3. Set machine paths once (or pass `-GhidraHome` / `-GhidraMcpDir` to the scripts):

```powershell
setx VC1_GHIDRA_HOME "C:\tools\ghidra_12.1.2_PUBLIC"
setx VC1_GHIDRA_MCP_DIR "C:\tools\ghidra-mcp"
```

4. Create the venv, reccmp config, MCP config and Ghidra project:

```powershell
.\tools\setup-reccmp.ps1
.\tools\setup-mcp.ps1        # writes the local .mcp.json from config/mcp.json.template
.\tools\init-ghidra.ps1      # first time only
```

## Build and compare

```powershell
.\tools\build-vc9.cmd
.\tools\compare-reccmp.ps1            # -ResetBaseline at the start of a round
.\tools\diff-reccmp.ps1 0x00486b00    # one function's asm diff
.\tools\stackcmp-reccmp.ps1 0x00486b00
```

Reports go to `build/vc9/reports/`. Ghidra: `open-ghidra.ps1`, then `close-ghidra.ps1` and `import-ghidra.ps1` (the only write path) after a build.

## Run

From `Virtual-City_Win_EN_Full/VirtualCity`, or with the helpers:

```powershell
.\.venv\Scripts\python.exe tools\run-debug.py --timeout 20
.\.venv\Scripts\python.exe tools\run-screenshot.py build\shot.png 8
.\tools\decompile-nut.ps1     # game scripts -> Virtual-City_Win_EN_Full/nut-src/ (vendor/NutCracker)
```

## SDL3 build

```powershell
.\tools\build-sdl3.cmd        # modern MSVC x86, Ninja; output build/sdl3/bin/VirtualCity.SDL3.exe
```

It runs the same recovered code over SDL3 implementations of the Windows imports (`src/imports/sdl3/`) and imports only SDL3.dll and KERNEL32.dll. SDL3 3.4.16 / SDL3_mixer 3.2.4 are downloaded into `build/` (or set `VC1_SDL3_SOURCE_DIR`, `VC1_SDL3_MIXER_SOURCE_DIR`).

## Addons

`VC1_LOG` (KdLog to the console) and `VC1_MODS` (PNG tile replacements) are on for the SDL3 build and off for the reference build. A mod is a folder in the git-ignored `mods/` with a `mod.json` such as `{"id":"advisor","version":"4.0","tiles":{"SCORING_FULL_GIRL_01":"textures/advisor.png"}}`; `mods/enabled.txt` lists the active ones. `tools/mod-tiles.py NAME` shows a tile's geometry.

## Do not commit

`ghidra/`, `build/`, `.venv/`, `.mcp.json`, `reccmp-user.yml`, `reccmp-build.yml`, game files. `Virtual-City_Win_EN_Full/` is not git-ignored (so editors can search the decompiled scripts): stage by name, never add anything under it.
