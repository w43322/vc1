# Virtual City decomp — agent guide

This is an early decomp of `VirtualCity.Unwrapped.exe` (SHA-256 in README). The build is a VC9 SP1 x86 `/GL /LTCG` reimplementation compared against the original with reccmp. It is not the full game.

Every recovered function must be backed by **Ghidra evidence from the original** and a **reccmp diff of the recompiled bytes**. Do not mark a function done if either side is missing. Put evidence in source comments; do not add extra docs.

Do not spawn sub-agents. Do not upload game assets. Do not commit Ghidra databases, `build/`, local tool copies, `reccmp-build.yml`, or `reccmp-user.yml`.

## Before you touch anything

1. `git status --short`. Keep other people's and the user's uncommitted files. Do not reset or clean the tree.
2. Confirm the original binary hash matches README. Every Ghidra read must name program `VirtualCity.Unwrapped.exe`.
3. Pick one addressed function or a tight cluster. Continue from `// FUNCTION: VC1 0x...` markers in `src/` and `third_party/squirrel223`, and from original addresses that are not mapped yet. Game entry is stdcall `WinMain` at `0x00486b00` (`src/host/win_main.cpp`); the PE entry is stock VC9 `WinMainCRTStartup`. Do not add a fake `main`.

## One round

Target about **1000 new lines under `src/`** per round (guideline, no upper bound). If you land short, **finish the round** — do not reopen Ghidra to pad the count.

Keep Ghidra open through analyze, write, build, and compare. Close it only immediately before import. Never write to Ghidra from MCP.

1. **Open Ghidra** — `tools/open-ghidra.ps1` (project `ghidra/VirtualCity.gpr`).
2. **Connect MCP** — then **interleave analyze and write**. For each chosen address: read enough to recover that function (see Analyze), and **write or update its C++ in `src/` (or a squirrel header `FUNCTION` name comment) before you pile up the next function's dumps**. Keep Ghidra reads going in parallel with edits. Do not finish a long read-only pass and only then start writing.
3. **Build and compare** — `tools/build-vc9.cmd`, then `tools/compare-reccmp.ps1`. Read the asmcmp diff for every function you added or changed. If you added `GLOBAL` or `VTABLE` markers, also read `build/vc9/reports/VC1-data.txt` and `VC1-vtable.txt`. Leave Ghidra up so you can re-read a function if the diff needs it.
4. **Close Ghidra** — `tools/close-ghidra.ps1`. Headless import fails if the GUI is still open.
5. **Import names into Ghidra** — `tools/import-ghidra.ps1` (official `reccmp-ghidra-import`). This is the only allowed Ghidra write path.
6. **Commit** — only when the user asked for the loop that includes commit, or they explicitly ask to commit. Do not commit `.gitignore` edits or generated `reccmp-*.yml` copies unless the user asked for those.

Then stop, or start a **new** round from step 1 if the user wants another loop.

If a read shows that already-mapped source, a callee, a marker, a compile flag, a tool script, or this workflow is wrong or weaker than the original, **fix it in the same round**. That includes existing `FUNCTION` bodies, `config/verified.csv`, CMake/VC9 flags (keep `/GL /LTCG`), and `AGENTS.md` / `tools/` when the loop itself is the problem.

## Analyze (Ghidra MCP)

MCP namespace is `user-ghidra`.

1. `list_instances`.
2. `connect_instance` with project `VirtualCity`.
3. On every read, pass program `VirtualCity.Unwrapped.exe`.

**Decompiled pseudocode is the main evidence.** Use disassembly, xrefs, callers/callees, and strings/constants as supplements — especially for ABI.

Write the C++ for a function as soon as you have that function's evidence. Enough to write:

- Function bounds (start/end).
- Decompiler output for control flow and calls.
- Assembly for parameter location, return register, stack balance, callee-saved registers, and `RET N`.
- ABI from **at least one caller**. VC9 LTCG often passes values in EAX, ESI, EDI, EBX instead of the stack. `WinMain` is stdcall (`RET 0x10`).

Do not wait until every callee, string, and sibling in the cluster is fully dumped. Recover the next address after the current body is in `src/` (stubs without `FUNCTION` markers are fine for unmapped callees). Follow-up reads that refine the same body are expected; wholesale “analyze the cluster, then write” is not.

**Never write Ghidra through MCP.** Do not rename, set comments, set prototypes, create structs, or save the program. Names and types go back through `tools/import-ghidra.ps1` after the GUI is closed.

Inferred names are not original symbols; say so in the source comment. The Ghidra project under `ghidra/` is local-only.

## Identify (strings, xrefs, gaps)

Start from a unique literal in the stock source, not from a guessed address. `search_strings` then `get_xrefs_to` on that VA. Error / assert / `Raise_Error` / `sq_throwerror` text is the usual fingerprint. Keep original typos; they are better IDs than the corrected English (`delagate cycle`, `comparsion between`, `ivalid type`). A string with **no code xref** is often dead table data, not a missing function.

When there is no unique string, work from neighbors:

- **Address gaps** in existing `FUNCTION` comments in that header or `.cpp`. Unnamed `FUN_00......` in a mapped cluster are the next candidates. `get_function_by_address` on the byte after a mapped `RET` if Ghidra did not create a function.
- **Callers of a mapped helper** (`sq_aux_gettypedarg`, `sq_throwerror`, `sq_aux_invalidtype`, `GetConstant`, `PrintObjVal`, `IdType2Name`). Tiny wrappers often have no string of their own.
- **Registration / vtable / `SQRegFunction` data.** A DATA xref to a `FUN_` from a table whose next/prev slot is a name string (`type`, `dummy`, `collectgarbage`) is the native at that pointer. Confirm `nparamscheck` / typemask if the record layout is visible. Do not treat a pointer that is not a function (ICF-folded `return 0`, or a VA with no Ghidra function) as recovered.
- **Type-tag immediates** in the body. Squirrel `OT_*` shows up as hex compares: `OT_NULL=0x1000001`, `OT_INTEGER=0x5000002`, `OT_FLOAT=0x5000004`, `OT_BOOL=0x1000008`, `OT_STRING=0x8000010`, `OT_TABLE=0xa000020`, `OT_ARRAY=0x8000040`, `OT_INSTANCE=0xa008000`, `OT_CLASS=0x8004000`. Refcounted objects test `0x8000000` (`SQOBJECT_REF_COUNTED`).

Always confirm with **one caller** and the assembly (`RET` vs `RET N`, this in ECX/EAX). Ghidra decompiler frequently **inverts** `SQTable::Get` / similar bool returns; trust the `TEST`/`JZ` in asm over the pseudocode.

Leave it unmapped when the original does not have a unique body:

- LTCG inlined the stock function into every caller (`sq_gettop`, `OptionalSemicolon`, `SQLexer::Error`, `sqstd_rex_match` into `_regexp_match`).
- LTCG merged several *different* source functions into one blob (`LogicalAndExp` with later binops). Do not pin a single source name to that blob.
- Identical functions that `/OPT:ICF` combined at one VA: annotate **every** source name with `// FUNCTION: VC1 0x... FOLDED` on that address (see reccmp annotations). All copies must be `FOLDED`; a mix of folded and non-folded on the same VA is a duplicate.
- The VA uses `g_vm`, `g_sizeClassHeap`, Win32, or sits in the host run after `sq_compilebuffer` (`0x004999c0+`). That is host or CRT, not stock `sq_*`. `0x00497490` is the host size-class heap, not `sq_malloc`.
- Generic `sqvector` push/pop helpers (`FUN_00415790`, `FUN_0040f8c0`) are not unique source symbols.

A **0%** asmcmp hit on a newly named function usually means the wrong overload or the wrong source symbol (`default_delegate_tostring` vs `base_type`), not “LTCG noise”. Check the `Ambiguous match` warning. Two overloads of the same name need the **mangled symbol** as the name comment (see reccmp annotations). Do not give two comments the same source name. Do not put two non-`FOLDED` `FUNCTION` comments on one VA.

```c++
// FUNCTION: VC1 0x0047eaa0
// ?AddInstruction@SQFuncState@@QAEXAAUSQInstruction@@@Z
```

## Write source

- Toolchain: VC9 SP1 x86. No modern C++ (`auto`, `nullptr`, range-for, `override`, lambdas, etc.). `/GR-`, `/EHsc`. Keep `/GL /LTCG`.
- Linker is already `/SUBSYSTEM:WINDOWS` so the CRT entry is `WinMainCRTStartup`, matching the original WinMain path. Do not switch back to a console `main`.
- Game functions: `// FUNCTION: VC1 0x00......` immediately above the definition, **sorted by address within that file**. Reccmp decomplint fails if they are out of order.
- Empty or dishonest bodies: `// STUB: VC1 0x00......` so asmcmp skips them. Do not mass-convert incomplete-but-honest `FUNCTION`s to stubs to inflate accuracy.
- Globals with a known original VA: `// GLOBAL: VC1 0x00......` on the **definition** in a `.cpp` (not only a header comment).
- Classes/structs with a known original vftable: `// VTABLE: VC1 0x00......` immediately above the type.
- String literals with a known original VA: `// STRING: VC1 0x00......` immediately above the line that contains the text. The same STRING address may repeat if the text matches.
- `LINE` pins a recompiled instruction to a source line when the asm diff mis-aligns; use it on near-matches, not as a first annotation.
- Stock Squirrel 2.2.3 (`third_party/squirrel223`): **comments only**, do not rewrite the bodies. `reccmp-project.yml` already has this tree in `source-root`. Prefer `// FUNCTION: VC1 0x...` on the implementation when that `.cpp` is already in original address order. Static functions are fine that way. If source order does not match the original (typical for sqstdlib), use the **name-comment** form from reccmp's annotations docs, and put it in a **header** (`.h` / `.hpp`). Decomplint allows lookup-by-name only in headers (`byname_function_in_cpp` fails `--warnfail` in `.cpp`). Example that matches `_file__typeof`:

```c++
// FUNCTION: VC1 0x0048f600
// _file__typeof
```

  Do not also list that VA in `config/verified.csv` or as a `FUNCTION` in `src/`. Host forks of squirrel (KD `getenv` / `OpenKdFile` / `dofile`, extra math, aux printerror strings) stay as recovered bodies in `src/`. Overloads of one C++ name (`AddInstruction`, `Raise_Error`, `GetNumericConstant`) need the mangled PDB symbol as the name comment, or asmcmp binds the wrong one (often 0%).
- CRT and other libraries **without** source in this tree: `config/verified.csv` (`address|type|name|symbol`). Do **not** hide mismatches with `report: ignore-functions:` in `reccmp-project.yml`.
- Do not use inline asm. Do not invent a mapping for a function you did not recover.
- Unmapped callees of a mapped function may live as stubs **without** `FUNCTION` markers (see `src/host/stubs.cpp`) so the TU links.
- Prefer improving existing mapped functions over leaving a known-wrong body in place. Layout, names, ABI, call targets, markers, `verified.csv`, flags, and workflow scripts are all in scope when the original supports the change. Do not drop `/GL /LTCG`.

A function is not recovered if it only exists as a Ghidra name, or only as uncommented C++. A `FUNCTION` comment on a stock squirrel implementation or a header name-comment that asmcmp diffs counts.

## reccmp

After every mapping change:

```text
tools/build-vc9.cmd
tools/compare-reccmp.ps1
```

`tools/compare-reccmp.ps1` is the full pass: decomplint, asmcmp (JSON/HTML/SVG), aggregate diff against `reports/VC1.prev.json`, datacmp, vtable, and roadmap. Only decomplint and asmcmp fail the script. Data/vtable/roadmap mismatches are expected this early; still read those reports when you added `GLOBAL` or `VTABLE` markers.

Compare reports per-function similarity. Report those numbers. Do not call them overall game completion. If asmcmp prints `Ambiguous match` on a name you just added, fix the name comment in the same round (mangled symbol) — do not leave a 0% overlay as “mapped”.

For a function that is close but not matching, check stack slots with `tools/stackcmp-reccmp.ps1 <original-offset>`. For a single asm diff: from `build/vc9`, `reccmp-reccmp --target VC1 --verbose 0x...`.

`reccmp-project.yml` target `VC1` points at `VirtualCity.Unwrapped.exe` and the README SHA-256. Do not use `reccmp-verexp` (this target is an EXE, not a DLL).

## What “next” looks like

Follow the original call graph from `WinMain` (`0x00486b00`) and from existing `FUNCTION` markers. Prefer a cluster of callers/callees over scattered 100% matches. Low match on a newly mapped function is acceptable if the control flow and calls are honest; raise match in a later round with more ABI evidence.

## Invariants

- Original binary identity: README SHA-256, program name `VirtualCity.Unwrapped.exe`.
- Read Ghidra, write `src/` or squirrel comments, import with the script. Never the reverse.
- No fake host `main`, no `ignore-functions` in the reccmp YAML.
- Default: no sub-agents, no asset upload, no commits of local databases or build output.
