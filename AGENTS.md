# Virtual City decomp — agent guide

This is an early decomp of `VirtualCity.Unwrapped.exe` (SHA-256 in README). The build is a VC9 SP1 x86 `/GL /LTCG` reimplementation compared against the original with reccmp. It is not the full game.

Every recovered function must be backed by **Ghidra evidence from the original** and a **reccmp diff of the recompiled bytes**. Do not mark a function done if either side is missing. Put evidence in source comments; do not add extra docs.

Do not spawn sub-agents. Do not upload game assets. Do not commit Ghidra databases, `build/`, local tool copies, `reccmp-build.yml`, or `reccmp-user.yml`.

## Before you touch anything

1. `git status --short`. Keep other people's and the user's uncommitted files. Do not reset or clean the tree.
2. Confirm the original binary hash matches README. Every Ghidra read must name program `VirtualCity.Unwrapped.exe`.
3. Pick one addressed function or a tight cluster. Continue from `// FUNCTION: VC1 0x...` markers in `src/` and from original addresses that are not mapped yet. Game entry is stdcall `WinMain` at `0x00486b00` (`src/host/win_main.cpp`); the PE entry is stock VC9 `WinMainCRTStartup`. Do not add a fake `main`.

## One round (do this in order)

Target about **1000 new lines under `src/`** per round (guideline, no upper bound). If you land short, **finish the round** — do not reopen Ghidra to pad the count.

1. **Open Ghidra** — `tools/open-ghidra.ps1` (project `ghidra/VirtualCity.gpr`).
2. **Analyze, read-only** — connect MCP, recover the chosen functions (details below). Never write to Ghidra from MCP.
3. **Write C++ into `src/`** — VC9-era C++, `FUNCTION` / `GLOBAL` / `VTABLE` / `STRING` markers as in the Write source section. `FUNCTION` markers **in address order per file**.
4. **Close Ghidra** — `tools/close-ghidra.ps1`. Headless import fails if the GUI is still open.
5. **Build and compare** — `tools/build-vc9.cmd`, then `tools/compare-reccmp.ps1`. Read the asmcmp diff for every function you added or changed. If you added `GLOBAL` or `VTABLE` markers, also read `build/vc9/reports/VC1-data.txt` and `VC1-vtable.txt`.
6. **Import names into Ghidra** — `tools/import-ghidra.ps1` (official `reccmp-ghidra-import`). This is the only allowed Ghidra write path.
7. **Commit** — only when the user asked for the loop that includes commit, or they explicitly ask to commit. Do not commit `.gitignore` edits or generated `reccmp-*.yml` copies unless the user asked for those.

Then stop, or start a **new** round from step 1 if the user wants another loop.

## Analyze (Ghidra MCP)

MCP namespace is `user-ghidra`.

1. `list_instances`.
2. `connect_instance` with project `VirtualCity`.
3. On every read, pass program `VirtualCity.Unwrapped.exe`.

**Decompiled pseudocode is the main evidence.** Use disassembly, xrefs, callers/callees, and strings/constants as supplements — especially for ABI.

Before writing C++:

- Confirm function bounds (start/end).
- Read the decompiler output for control flow and calls.
- Check assembly for parameter location, return register, stack balance, callee-saved registers, and `RET N`.
- Confirm ABI from **at least one caller**. VC9 LTCG often passes values in EAX, ESI, EDI, EBX instead of the stack. `WinMain` is stdcall (`RET 0x10`).

**Never write Ghidra through MCP.** Do not rename, set comments, set prototypes, create structs, or save the program. Names and types go back through `tools/import-ghidra.ps1` after the GUI is closed.

Inferred names are not original symbols; say so in the source comment. The Ghidra project under `ghidra/` is local-only.

## Write source

- Toolchain: VC9 SP1 x86. No modern C++ (`auto`, `nullptr`, range-for, `override`, lambdas, etc.). `/GR-`, `/EHsc`. Keep `/GL /LTCG`.
- Linker is already `/SUBSYSTEM:WINDOWS` so the CRT entry is `WinMainCRTStartup`, matching the original WinMain path. Do not switch back to a console `main`.
- Game functions: `// FUNCTION: VC1 0x00......` immediately above the definition, **sorted by address within that file**. Reccmp decomplint fails if they are out of order.
- Empty or dishonest bodies: `// STUB: VC1 0x00......` so asmcmp skips them. Do not mass-convert incomplete-but-honest `FUNCTION`s to stubs to inflate accuracy.
- Globals with a known original VA: `// GLOBAL: VC1 0x00......` on the **definition** in a `.cpp` (not only a header comment).
- Classes/structs with a known original vftable: `// VTABLE: VC1 0x00......` immediately above the type.
- String literals with a known original VA: `// STRING: VC1 0x00......` immediately above the line that contains the text. The same STRING address may repeat if the text matches.
- `LINE` pins a recompiled instruction to a source line when the asm diff mis-aligns; use it on near-matches, not as a first annotation.
- Stock third-party / CRT / Squirrel: `config/verified.csv` (`address|type|name|symbol`). Do **not** hide mismatches with `report: ignore-functions:` in `reccmp-project.yml`.
- Do not edit `third_party/squirrel223`. Do not use inline asm. Do not invent a mapping for a function you did not recover.
- Unmapped callees of a mapped function may live as stubs **without** `FUNCTION` markers (see `src/host/stubs.cpp`) so the TU links.
- You may change recovered source layout and compile flags when original evidence supports it; do not drop `/GL /LTCG`.

A function is not recovered if it only exists as a Ghidra name, or only as uncommented C++ in `src/`.

## reccmp

After every mapping change:

```text
tools/build-vc9.cmd
tools/compare-reccmp.ps1
```

`tools/compare-reccmp.ps1` is the full pass: decomplint, asmcmp (JSON/HTML/SVG), aggregate diff against `reports/VC1.prev.json`, datacmp, vtable, and roadmap. Only decomplint and asmcmp fail the script. Data/vtable/roadmap mismatches are expected this early; still read those reports when you added `GLOBAL` or `VTABLE` markers.

Compare reports per-function similarity. Report those numbers. Do not call them overall game completion.

For a function that is close but not matching, check stack slots with `tools/stackcmp-reccmp.ps1 <original-offset>`. For a single asm diff: from `build/vc9`, `reccmp-reccmp --target VC1 --verbose 0x...`.

`reccmp-project.yml` target `VC1` points at `VirtualCity.Unwrapped.exe` and the README SHA-256. Do not use `reccmp-verexp` (this target is an EXE, not a DLL).

## What “next” looks like

Follow the original call graph from `WinMain` (`0x00486b00`) and from existing `FUNCTION` markers. Prefer a cluster of callers/callees over scattered 100% matches. Low match on a newly mapped function is acceptable if the control flow and calls are honest; raise match in a later round with more ABI evidence.

## Invariants

- Original binary identity: README SHA-256, program name `VirtualCity.Unwrapped.exe`.
- Read Ghidra, write `src/`, import with the script. Never the reverse.
- No fake host `main`, no `ignore-functions` in the reccmp YAML.
- Default: no sub-agents, no asset upload, no commits of local databases or build output.
