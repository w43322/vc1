# Virtual City decomp — agent guide

This is an early decomp of `VirtualCity.Unwrapped.exe` (SHA-256 in README). The build is a VC9 SP1 x86 `/GL /LTCG` reimplementation compared against the original with reccmp. It is not the full game.

Every recovered function must be backed by **Ghidra evidence from the original** and a **reccmp diff of the recompiled bytes**. Do not mark a function done if either side is missing. Put evidence in source comments; do not add extra docs.

Do not spawn sub-agents. Do not upload game assets. Do not commit Ghidra databases, `build/`, local tool copies, `reccmp-build.yml`, or `reccmp-user.yml`.

## Before you touch anything

1. `git status --short`. Keep other people's and the user's uncommitted files. Do not reset or clean the tree.
2. Confirm the original binary hash matches README. Every Ghidra read must name program `VirtualCity.Unwrapped.exe`.
3. Pick one addressed function or a tight cluster. Continue from `// FUNCTION: VC1 0x...` markers in `src/` and `third_party/squirrel223`, and from original addresses that are not mapped yet. Game entry is stdcall `WinMain` at `0x00486b00` (`src/kdcore/win_main.cpp`); the PE entry is stock VC9 `WinMainCRTStartup`. Do not add a fake `main`.

For documentation/tool maintenance without recovery or mapping changes, inspect the tracked files and their callers; the recovery round below is not required. Leave untracked and ignored files untouched unless explicitly requested. Do not delete a standalone diagnostic merely because no build script calls it. Initial Ghidra analysis uses `tools/init-ghidra.ps1`; recovered names and types use `tools/import-ghidra.ps1`, with no hardcoded annotation replay.

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

Do not wait until every callee, string, and sibling in the cluster is fully dumped. Recover the next address after the current body is in `src/` (`STUB` at that VA is fine for unmapped callees). Follow-up reads that refine the same body are expected; wholesale “analyze the cluster, then write” is not.

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
- The VA uses `g_vm`, `g_sizeClassHeap`, Win32, or sits in the host run after `sq_compilebuffer` (`0x004999c0+`). That is host or CRT, not stock `sq_*`. `0x00497490` is the host size-class heap (`sq_vm_malloc` in `src/memory/fixed_pool.cpp`; stock `sqmem.cpp` is not built), not `sq_malloc`.
- Generic `sqvector` push/pop helpers (`FUN_00415790`, `FUN_0040f8c0`) are not unique source symbols. A unique instantiation uses a `TEMPLATE` name comment on `sqvector<T>::…` in a header (see reccmp annotations), not a recovered host wrapper.

A **0%** asmcmp hit on a newly named function usually means the wrong overload or the wrong source symbol (`default_delegate_tostring` vs `base_type`), not “LTCG noise”. Check the `Ambiguous match` warning. Two overloads of the same name need the **mangled symbol** as the name comment (see reccmp annotations). Do not give two comments the same source name. Do not put two non-`FOLDED` `FUNCTION` comments on one VA.

```c++
// FUNCTION: VC1 0x0047eaa0
// ?AddInstruction@SQFuncState@@QAEXAAUSQInstruction@@@Z
```

## Source layout: recovered code vs. imports

One rule decides where code goes: everything that was inside the original
`.exe` is shared recovered code; everything it imported from a DLL is provided
per target.

- `src/kd/`, `src/app/`, `src/bindings/`, `src/pyro/`, ...: engine and game.
- `src/kdcore/`: the recovered G5 OpenKODE core (events, files and timers, log,
  window procedure and input, threads, network, crash handler, `WinMain`,
  globals). `kdcore/platform.h` is the engine-facing contract and must not
  include Win32 or renderer headers; `kdcore/runtime.h` / `window.h` hold the
  native records; `kdcore/native.h` includes the Windows SDK headers.
  `cmake/kdcore.cmake` lists these sources in the reference translation-unit
  order; keep it.
- `src/imports/win32/`: the reference target's imports are the real DLLs
  (`cmake/imports-win32.cmake`, BASS/d3d8 import-library stubs). `crt.cpp` holds
  the two file driver slots that depend on the VC9 CRT's `FILE` layout.
- `src/imports/sdl3/`: the SDL3 target (`tools/build-sdl3.cmd`, modern MSVC x86,
  C++17). `include/` replaces `<windows.h>`, `<winsock2.h>`, `<mmsystem.h>`,
  `<shellapi.h>`, `<shlobj.h>`, `<shlwapi.h>` with only what the game uses; each
  DLL is one file implemented directly over SDL3 (`kernel32.cpp`, `user32.cpp`,
  `gdi32.cpp`, `shell32.cpp`, `shlwapi.cpp`, `winmm.cpp`, `ws2_32.cpp`,
  `d3d8.cpp`, `bass.cpp` over SDL3_mixer) plus `crt.cpp` and `sdl3.h/.cpp`
  helpers. There is no SDL `main`: the CRT entry point calls the recovered
  `WinMain`. `user32.cpp` turns SDL events into Win32 messages for the recovered
  window procedure; a HWND is a small record with an SDL window for captioned
  windows; mouse positions are back buffer pixels. `d3d8.cpp` transforms FVF
  0x142 vertices on the CPU and draws with SDL geometry; unused COM slots abort
  with their index.

Recovered sources never branch on the target: there is no `VC1_USE_SDL3` define
in code. When the SDL3 build needs something new, add the missing SDK
declaration to `imports/sdl3/include/windows.h` and implement the call in its
DLL's file instead of forking a recovered function. Write the imports in the
project's normal style. Shim semantics are as simple as the game allows (kernel
events and semaphores are dummies and waits never block; threads made by
pthread_create stay suspended, which only matters for the console's stdin
reader under KD_ENABLE_STDIO; `LoadLibraryA` only knows kernel32, so
dbghelp crash dumps and QUSEREX stay off; `CreateMutexA` reports no other
instance; WM_CHAR is Latin-1). Only `KdFileMap`/`KdFileUnmap` (`crt.cpp`) differ
per target, because they need CRT internals.

The event queue block at 0x004ed144 (cursor, 16-event pool, head, tail) is one
`KdEventQueue` object because `AllocEvent` scans the pool up to the queue head;
separate globals let the scan run into unrelated data.

The SDL executable imports only SDL3.dll and the CRT's KERNEL32.dll. The
reference target keeps its source order, VC9 flags and `/GL /LTCG`; SDL code has
no original mapping markers and keeps modern C++ inside `src/imports/sdl3/`.
SDL dependencies and extracted cursor/icon assets stay under `build/`, never Git.
`tools/msvc-showincludes.py` normalizes localized cl output for Ninja dependencies;
do not remove it without verifying header rebuilds. Engine x86 ABI/layout
assumptions still prevent a 64-bit port. For mechanical moves, preserve
recovered bodies and markers and compare every mapped entity against the
pre-move baseline; matching scores alone are not a runtime test.

## Addons (not original code)

Two CMake options, on by default for the SDL3 build and off for the reference
build (a reference build with them on is not comparable):

- `VC1_LOG`: `KdLogV` also passes the text to `OutputDebugStringA` (SDL3: the
  console). Without it KdLog text is dropped, as in the original release build.
- `VC1_MODS`: texture mods in `src/mod/` (VC9-compatible). The one hook is in
  `LoadTextureFile`; `texture_hook.cpp` locks the new texture through the D3D8
  table, `mods.cpp` patches atlas pixels, `mod_io.cpp` reads files with the CRT,
  PNG with the game's libpng 1.0.2 and tile names from the live Squirrel
  `Tiles`/`TileImages` tables. Mods only replace images, with no script hooks;
  never overwrite game files. The root `mods/` folder is local and git-ignored;
  the build copies its folders beside the executable and seeds
  `mods/enabled.txt` once. Exported images and temporary verification stay
  under `build/`. Keep addon code free of original mapping markers.

## Write source

- Matching/shared-source toolchain: VC9 SP1 x86. No modern C++ (`auto`, `nullptr`, range-for, `override`, lambdas, etc.). `/GR-`, no `/EH` (the original game code has no C++ EH frames; only the CRT sets up `fs:[0]`). Keep `/GL /LTCG`.
- Linker is already `/SUBSYSTEM:WINDOWS` so the CRT entry is `WinMainCRTStartup`, matching the original WinMain path. Do not switch back to a console `main`.
- Game functions: `// FUNCTION: VC1 0x00......` immediately above the definition, **sorted by address within that file**. Reccmp decomplint fails if they are out of order.
- Empty or dishonest bodies: `// STUB: VC1 0x00......` so asmcmp skips them. Do not mass-convert incomplete-but-honest `FUNCTION`s to stubs to inflate accuracy.
- Globals with a known original VA: `// GLOBAL: VC1 0x00......` on the **definition** in a `.cpp` (not only a header comment). Give the definition external linkage (no `static`): a `static` symbol shows up without the `vc1::` prefix, so every access counts as an operand mismatch in asmcmp.
- Classes/structs with a known original vftable: `// VTABLE: VC1 0x00......` immediately above the type. Multiple inheritance needs **one `VTABLE` per original table**, with the `{for `Base'}` name after the address so same-module markers do not collide (`DUPLICATE_MODULE`). Example:

```c++
// VTABLE: VC1 0x004d0b78 vc1::IScriptObject
// VTABLE: VC1 0x004d0b94 vc1::IRenderTarget
// VTABLE: VC1 0x004d0be8 vc1::IDisplay
// VTABLE: VC1 0x004d0c28 vc1::IShutdownable
// VTABLE: VC1 0x004d0c38 vc1::IAbstract
class KdDisplay : public IScriptObject, public IRenderTarget, public IDisplay,
                  public IShutdownable, public IAbstract
```

  The extra field must match the PDB `{for `...`}` string (include `vc1::` when the demangler does). Do not use a free-form “extra original vftables” comment. Reccmp decomplint rejects two `VTABLE: VC1` lines on one class unless those extras differ. See `vendor/reccmp/docs/annotations.md`.
- Compiler-generated `scalar deleting destructor` (and similar) use `SYNTHETIC` name comments in a header, not `FUNCTION`. Do not put the marker on `~T()` — that binds the real destructor. Include the `vc1::` prefix so asmcmp binds the MSVC symbol:

```c++
// SYNTHETIC: VC1 0x00402230
// vc1::KdDisplay::`scalar deleting destructor'
```

- Recovered constructors are `T::T(...)` with a `FUNCTION` marker on that definition. Do not add a separate `ConstructT` that takes a `malloc` block and pokes it. `SYNTHETIC` is only for symbols the compiler emits that you must not write: `scalar deleting destructor`, compiler default copy/assign, and MI adjustor thunks (`W3A` / `WBA` / `?AddRef@T@vc1@@W3AEIXZ`). A user `T::T()` or `T::AddRef` is `FUNCTION`.
- Do not store vptrs or subobjects as `unsigned*` integer writes (`object[0] = 0x004d...`). Give the type a constructor and `VTABLE` markers; the compiler writes vptrs. Put fields on the interface or class that owns that offset (`ITexture::native`, `IRenderTarget::extra` / `graphics`). `IAbstract` is always inherited (`refs` at IAbstract+4, SIZE 0x08), never composed as a nested `Abstract` / `void* abstract`. After `malloc`, placement-`new` the constructor.
- Owning interface-pointer members whose destructor the original inlines as `if (p) p->Release(); p = 0;` in reverse member order are `KdRef<T>` (`kd/iface/iunknown.h`), not raw pointers released by hand in the destructor body; the member order then reproduces the original teardown order.
- Do not give `IUnknown` a default `AddRef` / `Release` / `QueryInterface`. Those stay pure. Write one implementation per class (`FUNCTION`). Other bases are compiler adjustor thunks (`SYNTHETIC` in the header). A second full copy at its own VA (e.g. `TextureAddRef` with ITexture `this`) is also `FUNCTION`, not a thunk and not a copy-paste onto every class.
- String literals with a known original VA: `// STRING: VC1 0x00......` immediately above the line that contains the text. Each STRING address may appear once: reccmp drops a repeated address with `[ERROR] Dropped duplicate address`. When several functions use the same original literal, define it once as an `extern const char` array marked `GLOBAL` (not `STRING`; a named array is a data symbol, so a `STRING` marker leaves asmcmp comparing STRING vs DATA) and reference that symbol elsewhere (`kKdParentWindow`).
- `LINE` pins a recompiled instruction to a source line when the asm diff mis-aligns; use it on near-matches, not as a first annotation.
- Stock Squirrel 2.2.3 (`third_party/squirrel223`): mapping work is **comments only**, do not rewrite the bodies. Preserve the existing evidence-backed `// VC1:` patches: `SQVM::IsEqual` (`0x00493230`) compares native-backed instances by userpointer, the float `%` in `ARITH_OP` calls `fmodf` (`0x004999c0`), and `squirrel.h` maps `scstrstr`/`scstrcmp` to the host's `vc1::KdStrstr`/`vc1::KdStrcmp` (`0x00482220`/`0x00499c90`); see `PROVENANCE.md`. `reccmp-project.yml` already has this tree in `source-root`. Prefer `// FUNCTION: VC1 0x...` on the implementation when that `.cpp` is already in original address order. Static functions are fine that way. If source order does not match the original (typical for sqstdlib), use the **name-comment** form from reccmp's annotations docs, and put it in a **header** (`.h` / `.hpp`). Decomplint allows lookup-by-name only in headers (`byname_function_in_cpp` fails `--warnfail` in `.cpp`). Example that matches `_file__typeof`:

```c++
// FUNCTION: VC1 0x0048f600
// _file__typeof
```

  Do not also list that VA in `config/verified.csv` or as a `FUNCTION` in `src/`. Host forks of squirrel (KD `getenv` / `OpenKdFile` / `dofile`, extra math, aux printerror strings) stay as recovered bodies in `src/`. Overloads of one C++ name (`AddInstruction`, `Raise_Error`, `GetNumericConstant`) need the mangled PDB symbol as the name comment, or asmcmp binds the wrong one (often 0%).
- klibc's scanf family (`third_party/klibc`, upstream files plus one marked VC9 patch, built by `cmake/klibc.cmake`) replaces the CRT `sscanf`. Same comments-only rule as squirrel; name comments live in `third_party/klibc/klibc.h`. Before rewriting a library-looking function by hand, check whether it is a known open-source body and vendor that instead.
- The JPEG decoder is Rich Geldreich's jpgd **0.93b** (`third_party/jpgd093b`, `cmake/jpgd.cmake`, built without `SUPPORT_X86ASM`/`SUPPORT_MMX` and with `/Zc:forScope-`); name comments in `third_party/jpgd093b/jpgd093b.h`. Its header-inline `jpeg_decoder_file_stream` runs on KD files through `src/kd/display/jpeg_stream.h`.
- PNG uses libpng **1.0.2** and zlib **1.1.3** inflate (`third_party/libpng102`, `third_party/zlib113`, `cmake/libpng.cmake`); name comments live in `libpng102.h` and `zlib113.h`. Keep the pinned PNG feature definitions: `src/pyro/pyro_image.cpp` checks `png_struct` size 0x168 and `png_info` size 0x48. Library mappings are comments only.
- LZMA SDK **4.57** C decoder subset (`third_party/lzma457`, `cmake/lzma.cmake`) preserves upstream layout and bodies; mappings/evidence live in `lzma457.h` and `lzma457_audit.h`. The original uses **`_LZMA_IN_CB` and `_LZMA_PROB32`**, default 64-bit `CFileSize`, no `_LZMA_OUT_READ`. `SzExtract` is at `0x004bc6e0`, outside the main library cluster. The allocator aliases are folded at `0x004b70f0` / `0x004b7100`; labels without functions were bootstrapped as STUB candidates through the official importer, then decompiled before FUNCTION mapping. Do not treat missing Ghidra functions as absent code. CRC uses a precomputed original table; do not map `CrcGenerateTable` to an unrelated initializer. The host `bar` driver over it (`KdConstructNamed` `0x0047cb00`, `src/kdcore/startup.cpp`) is game code.
- Boost **1.41.0** (`third_party/boost_1_41_0`, `cmake/boost.cmake`, `BOOST_DISABLE_THREADS`) is the unchanged Boost.Pool header subset plus what VC9 includes for it. The size-class heap is 33 `boost::pool<default_user_allocator_malloc_free>` (`FixedPool` in `src/memory/fixed_pool.h`); `malloc_need_resize` `0x0040f1a0` is the one out-of-line pool body, name comments in `boost_vc1.h`. Pool headers are identical in 1.35-1.45; 1.41.0 is the last release before the exe's link date. Vendored SqPlus names the engine string `SqPlusStdString` (`vc1::KdString`) instead of redefining `std::string`, so CRT headers such as `<functional>` coexist with it.
- pthreads-win32 **2.8.0** (`third_party/pthreads-w32-2-8-0`, `cmake/pthreads.cmake`) is unchanged upstream source built like its `nmake VC-static` target (the inlined `pthread.c`, `PTW32_STATIC_LIB`, `__CLEANUP_C`; game code sees the same two defines). Mappings live in `ptw32_vc1.h`, the library globals are `global` rows in `config/verified.csv`. Not 2.9.x: `pthread_mutex_lock` `0x0047a840` has no robust-mutex branch. KD's own thread code is `src/kdcore/thread.cpp` (`AllocPrimaryTls`, `KdThreadStart`, the key destructor); call the pthread API, do not re-implement it. The SDL3 build compiles `pthread.c` as C++ (`/TP`, the SDK subset is C++) with `_CRT_NO_TIME_T` and `/FIeh.h`.
- Game and library code is `/GS-` (only the prebuilt CRT calls `__security_check_cookie`). Without cookies LTCG inlines more; where the original keeps a small helper out of line at every call site (`HostLogV`), `__declspec(noinline)` with a comment is acceptable.
- CRT and other libraries **without** source in this tree: `config/verified.csv` (`address|type|name|symbol`). Do **not** hide mismatches with `report: ignore-functions:` in `reccmp-project.yml`.
- Do not use inline asm. Do not invent a mapping for a function you did not recover.
- Name virtual slots, helpers and typedefs after what they do (script method names, callers, the body), not after their vtable offset (`Script0c`, `Display1c`, `Fn14`). Keep the offset in the trailing `// vtable+0x..` comment. When the evidence is thin, still pick a descriptive name and say it is a guess in the comment; a slot nobody calls and whose bodies are trivial is `Unused`.
- Every original callee has a VA. Put it in the owning `.cpp` with `FUNCTION` or `STUB` at that address, or call an already-mapped / Win32 / CRT symbol. Do not invent unmarked `Unmapped*` helpers. Stock squirrel stays comments-only; templated instantiations use `TEMPLATE` (see reccmp annotations).
- Prefer improving existing mapped functions over leaving a known-wrong body in place. Layout, names, ABI, call targets, markers, `verified.csv`, flags, and workflow scripts are all in scope when the original supports the change. Do not drop `/GL /LTCG`.

A function is not recovered if it only exists as a Ghidra name, or only as uncommented C++. A `FUNCTION` comment on a stock squirrel implementation or a header name-comment that asmcmp diffs counts.

## reccmp

After every mapping change:

```text
tools/build-vc9.cmd
tools/compare-reccmp.ps1
```

`tools/compare-reccmp.ps1` is the full pass: decomplint, asmcmp (JSON/HTML/SVG), aggregate diffs, datacmp, vtable, roadmap, and `tools/report-progress.py`. Only decomplint and asmcmp fail the script. Full tool output goes to `build/vc9/reports/` (`VC1-asmcmp.txt`, `VC1-data.txt`, `VC1-vtable.txt`, ...); the console shows only errors/warnings, the diff **since the previous run** (`VC1.diff.txt`) and **since the round baseline** (`VC1.base-diff.txt`), and progress. Run it with `-ResetBaseline` at the start of a round (or right after a commit) so the baseline diff lists every regression of the round, not just the last iteration. `-Full` echoes everything. Search the report files with Grep instead of re-running with filters. Data/vtable/roadmap mismatches are expected this early; still read those reports when you added `GLOBAL` or `VTABLE` markers.

The link uses `/OPT:NOICF`, so the copies of a `FOLDED` original stay separate in the recompiled exe and asmcmp binds only one of them; `Match (...) collides with previous staged match` on a `FOLDED` address is expected. On any other address it means two markers claim one VA.

Compare reports per-function similarity. Report those numbers. Do not call them overall game completion. If asmcmp prints `Ambiguous match` on a name you just added, fix the name comment in the same round (mangled symbol) — do not leave a 0% overlay as “mapped”.

For a function that is close but not matching, check stack slots with `tools/stackcmp-reccmp.ps1 <original-offset>`. For asm diffs use `tools/diff-reccmp.ps1 0x... [0x...]` (runs `reccmp-reccmp --verbose` from `build/vc9` without the PDB-parsing noise).

Supplementary raw views of the original (Ghidra stays primary evidence): `.venv/Scripts/python.exe tools/disasm-orig.py 0x...` disassembles up to the function's final `RET` (or pass a hex length), and `--dwords 0x... [count]` dumps a vtable or data block.

Shell notes (Windows PowerShell): do not pass Python with embedded double quotes through `python -c` (PowerShell mangles the quoting); write a temp `.py` and run it. Do not write sources with `Set-Content -Encoding utf8` (adds a BOM) or through PowerShell pipes (CRLF); sources are LF without BOM.

`reccmp-project.yml` target `VC1` points at `VirtualCity.Unwrapped.exe` and the README SHA-256. Do not use `reccmp-verexp` (this target is an EXE, not a DLL).

## What “next” looks like

Follow the original call graph from `WinMain` (`0x00486b00`) and from existing `FUNCTION` markers. Prefer a cluster of callers/callees over scattered 100% matches. Low match on a newly mapped function is acceptable if the control flow and calls are honest; raise match in a later round with more ABI evidence.

## Run the game (runtime goal)

The current goal beyond per-function matching is a recompiled exe that runs: first a window that renders the loading screen, then the game. Run it from the game data directory `Virtual-City_Win_EN_Full/VirtualCity` (scripts and `res/` are relative to it; the original exe and `bass.dll` sit one level up).

- `.venv/Scripts/python.exe tools/run-debug.py --timeout 20 [--nolog] [--bp HEX ...]`: Win32 debugger over the recompiled exe. Prints `KdLog`/script errors, and on a fatal exception the registers, code bytes and a symbolized stack scan (symbols from `build/vc9/bin/VirtualCity.Unwrapped.map`). `--bp` takes **recompiled** VAs (look them up in the `.map`); each hit prints regs + stack. `--bp-args N` switches to one compact line per hit: the first four raw stack arguments plus N dwords dumped at stack argument `--bp-arg-index K` (or at a register with `--bp-reg ecx`), `--bp-floats` prints them as floats. Diffing such dumps between the original and the recompiled exe (same `--click`, same timeout) is the fastest way to find which stage of a pipeline diverges. Output redirected with PowerShell `>` is UTF-16. `run-debug` stops the process at timeout by default; `--keep` leaves it running. Close a kept instance before the next run because it holds the `VirtualCity` mutex (new runs exit with code 1). For interactive validation, let the user click; do not automate clicks unless explicitly requested. `--click FX,FY@T` moves the real cursor and clicks at client fractions after T seconds (the WM_TIMER handler re-reads `GetCursorPos`, so posted messages do not work); `--click 0.84,0.53@14` presses PLAY GAME. `--exe Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe --map <one-line map naming KdLog at 0x00496a40>` traces the original for comparison. Blank `<empty fmt ... %s%s\n>` lines are empty prints.
- `.venv/Scripts/python.exe tools/run-screenshot.py [out.png] [seconds] [exe]`: run for N seconds, screenshot, kill. Pass `Virtual-City_Win_EN_Full/VirtualCity.Unwrapped.exe` as `exe` to capture the original for comparison (at 8 s it shows the "Loading... Please wait" screen).
- Loop: run, read the first error or crash, find the responsible function in the original (Ghidra), recover it (usually a `STUB` or an unrecovered factory), rebuild, rerun. Each fix is still a normal recovered function with markers and a reccmp diff.

### Game scripts

The `.nut` files are **compiled** Squirrel bytecode, not text. `tools/decompile-nut.ps1` clones and builds the decompiler ([w43322/NutCracker](https://github.com/w43322/NutCracker), branch `sq2-vc1`) into `build/NutCracker` and writes readable sources to `Virtual-City_Win_EN_Full/nut-src/` (same tree as `VirtualCity/`). Use them to see which engine components, methods and natives a script needs (`getComponent("X")`, `createFromScript`, `System.RegisterObject`, method names looked up through `IScriptObject::GetMember`). Script call stacks in the log name the file and line.

`Virtual-City_Win_EN_Full/` is deliberately **not** in `.gitignore` so Cursor Grep/Read can see `nut-src/`. Grep only accepts it with an uppercase drive path (`D:\vc1\Virtual-City_Win_EN_Full\nut-src`). Never stage anything under it: add files by name, not `git add -A` / `git add .`.

### Runtime status (update when it changes)

This section records prior runtime observations, not full-game validation. Later library/template mappings alone do not establish new gameplay coverage.

- SDL3 build: modern MSVC x86; it runs the recovered kdcore (including `WinMain`, the window procedure and input) over the SDL3 imports in `src/imports/sdl3/`. Before the move to imports a manual run loaded `Level_01_05` and the user confirmed controls, music and effects; the imports-based build has not been run yet. This is not full campaign validation.

- Recent source/build changes: `src/kdcore/sqstd_system.cpp` implements the game's system-library fork (ShellExecute, QPC clock and KD paths); SqPlus strings use `KdString`, global new/delete use malloc/free, and the build no longer needs the C++ standard-library/EH runtime. D3D8 uses a generated import library (`cmake/imports-win32.cmake`) like BASS. Additional SqPlus/template, CRT, libpng and zlib mappings are in source/header annotations and `config/verified.csv` as appropriate.
- Works: CRT/WinMain, script VM and natives, `Main.nut` `initApp`, window class + `KdWindowProc`, D3D8 device, main loop with Begin/EndScene/Present per frame, System/UpdateGroup/RenderGroup/InputGroup, Display script methods, System `GetMember` (RegisterObject/GetObject/RenderFrame/...), CompoundObject, ScriptedObject (`src/kd/scene/scripted_object.cpp`), `KdScriptInstance` (`src/kd/script_instance.cpp`), SquirrelVM `BeginCall`/`PushParam`/`EndCall`. `Game.nut` loads as a ScriptedObject; its `Update`/`Render`/`OnKeyPressed` run every frame (SplashScene).
- The main menu renders (background, zeppelin scene, menu buttons and font text). Recovered for that: graphics `GetMember` (`0x0045dc20`), TGA (`libtga`) and PVR (`LoadPvrTexture` `0x00465990`, 32-bit only) textures, TileManager (`src/kd/tile/tile_manager.cpp`: engine `KdTileManager` + game `VcTileManager`, factory `0x004a1aa0`, tile command interpreter in `Render`), D3D8 COM calls through `D3dRelease` / stdcall typedefs. The game starts fullscreen like the original (`run-screenshot.py` captures black in fullscreen; it works if the window is not minimized in windowed mode). Menu clicks work; PLAY GAME starts the intro comic (`scripts/comics/*`) and its JPEG backgrounds load (`LoadJpgTexture` `0x00465f10`). The comic runs to the end of the run without script errors.
- Pyro effects: `.pyro` files load (`[Effect] Loaded ... (emitters: n)`). The statically linked Fenomen Pyro library (proprietary, do not vendor its SDK) is recovered under `src/pyro/` (archive strings, stream adapter, file/header/resource/shape/frame readers, emitter and layer deserializers). The game side is in `src/kd/display/effect.*`: `KdEffect` (CreateEffect), `KdPyroStream` and `KdEffectEmitter` (the scene object that `CreateEmitter` returns). Particles render: the intro comic's car smoke and emergency signal (about 45 s with `--click 0.84,0.53@14`) match the original apart from noise. Pipeline: `PyroLayer::Update`/`Prepare` (simulation), `Render` `0x00452140` → `BuildQuad` `0x004513d0` → `PyroVertexBuffer::Upload` → `KdDisplayDevice::DrawQuads`/`DrawQuadBuffer` → `KdGraphics::DrawQuads` `0x0045c910` → `BeginQuadBatch` `0x00464530`; textures come from `CreateImage`/`CreateImageTexture` (a `KdTexture` locked to the image size). Turbulence is `PyroNoise` `0x0044d5c0` (`src/pyro/pyro_random.cpp`). `PyroLibrary::LoadFromPath` `0x004566b0` (filename loader, no callers) is recovered with Pyro's own `CFile` layer (`src/pyro/pyro_cfile.cpp`).
- Sound: BASS 2.4 (`BASS.dll`, linked statically by the original through import thunks `0x0044467a+`, `library` rows in `config/verified.csv`). Declarations are our own (`src/kd/sound/bass_api.h`; do not vendor un4seen's `bass.h`). `cmake/bass.cmake` builds the import library from `cmake/bass.def` + `cmake/bass_stub.c` (a throwaway DLL in `build/vc9/bass-import/`, never next to the exe) and copies the game's `bass.dll` into `build/vc9/bin/`. `KdSoundDriver::SetEnabled` initializes BASS; music streams (`KdMusic`, `src/kd/sound/music.cpp`, `[Music] Loaded ...`) and sound effects (`KdSound`, 8-channel samples, `src/kd/sound/sound.cpp`, `[Sound] Loaded ...`) play.
- Resources: the game loads its cursors (`setCursor` ids 100..105) and icon from its own module. CMake runs `tools/extract-rsrc.py` at configure time to dump the original's `.rsrc` (minus the manifest) into `build/vc9/original-rsrc/original.res` and links it; without it the cursor is invisible.
- The http KD driver (`KdConstructHttp` `0x0047d650`, `src/kdcore/startup.cpp`) is recovered over wininet; it has not been run since (earlier runs logged `can't mount (null)()` while it was a stub). The SDL3 `wininet.cpp` fails every call, so http opens nothing there. The blank `<empty fmt ...>` log lines label the bytes after the format string, not the log level.
- `scripts/menus/Loading.nut` (`res/MenuLoading.nut`, `ProgressFG` control) is the loading screen; `scripts/Game.nut` is the `Game` object.
- Levels: `Scene` (`src/kd/scene/scene.cpp`) and its subclass `GameLevel` (`src/kd/world/game_level.cpp`: dynamic tile layers, `FindObjectsByMask`, `GetObjectAt`) are recovered. Clicking through the comic (`--click 0.84,0.53@14` then `0.5,0.5` every 4 s from 22 s) reaches `Loading level: Level_01_01`; `File` (`src/kd/file/file.cpp`) opens the level files and `TileLayer` (`src/kd/tile/tile_layer.cpp`: tiles, 128 px spatial grid, parallax back tile, tile objects drawn between tiles by depth) loads the three tile layers. `PassMap` (`src/kd/road/pass_map.cpp`: cell byte grid, `IGridMap`, `OnCellChange`, `IGridRenderer` list) loads too, and the host math lib fork (`src/kdcore/sqstd_math.cpp`) adds `round`/`fmod`/`DBPI`/`HALFPI`/`INVDBPI`. `GridRouter` (`src/kd/road/grid_router.cpp`), `Blob` (`src/kd/blob/blob.cpp`) and `Camera` (`src/kd/scene/camera.cpp`: world/screen/UI matrices, `BindView` sets the graphics transform) load as well. `ClassificatedRenderer` (`src/kd/render/classificated_renderer.cpp`) and `RoadRenderer` (`src/kd/road/road_renderer.cpp`: one malloc'd `RoadCell` per grid cell, link bits index 16-entry road tile tables, preview cells) load. `TerrainBuilder` (`src/kd/build/terrain_builder.cpp`: build-road / build-forest / destroy modes over the grid map, per-cell tint as an `IGridRenderer`, road drafts through `IGridRouter`, script `CanBuildRoad`/`OnBuild*`/`OnDestroy*` callbacks) loads. `GridRectRenderer` (`src/kd/road/grid_rect_renderer.cpp`: cell rectangles outlined with a 16-entry tile table indexed by inner sides) loads for the activity radius and grid margin. `TileObject` (`src/kd/tile/tile_object.cpp`: one tile of the TileManager drawn at an `IPositionable`, joint sub-objects, hit test, depth) and `BuildingBuilder` (`src/kd/build/building_builder.cpp`: building placement with a `TileObject` child, footprint snapped to the grid, margin rectangles, `OnBoundsChange`/`OnBuild`/`OnRefuseBuild`) load. `IGridRenderer`'s slot is `RenderGrid` because BuildingBuilder also implements `IRenderable::Render`. `TransportMover` (`src/kd/route/transport_mover.cpp`) loads with all 21 script bindings, the transport records and the `IRoute` interface (`src/kd/iface/iroute.h`); it is fully recovered: movement (`Update`/`UpdateRecord`: blocker check, corner arcs, `OnCellChange`/`OnMovingFinish`), roadside parking (`MoveToRoadsite`), the debug overlay `Render`, path finders into cell rects and stream read/write. The optimal-route planners order areas with `TSPSolver` (`src/kd/route/tsp_solver.cpp`: farthest insertion over a `KdObjArray<KdArray<int> >` distance matrix, a singleton). `GroupRenderer` (`src/kd/render/group_renderer.cpp`: a ScriptedObject with its own `RenderGroup`, script `RegisterObject`) and `RoadCellsRenderer` (`src/kd/road/road_cells_renderer.cpp`: a copy of RoadRenderer cells drawn as route marks) make `RoutingRenderer.nut` load. `GameObject` (`src/kd/world/game_object.cpp`: a ScriptedObject whose `SetPosition` moves its positionable children) loads every plant, `CameraMover6` and `Garage8`; `TileAnimation` (`src/kd/tile/tile_animation.cpp`: picks a `TileAnimationData` channel by direction and drives a slave `TileObject`'s tile) animates them. The menus' `Layer` component (`src/kd/scene/layer.cpp`: an `IGraphics` that records every call as an inline `LayerCommand` and replays them in `Render`) draws the hints. `Route` (`src/kd/route/route.cpp`: a polyline with per-point distances, filled from an `IGridRouter` path, sampled by distance) is recovered. Level_01_01 then runs with no script errors and reacts to clicks; transports have not been exercised at runtime yet.

### Pitfalls seen at runtime

- Globals the original keeps adjacent and indexes as one block (a `RECT`, an array) must be **one** C++ object. Separate `int` globals get reordered by the linker, and writing `rect[2]` silently overwrites an unrelated global (this corrupted `g_kdDisplay`).
- A wrong comparison direction in a sorted-table insert/lookup (`RegisterFactory`) makes components "missing" with no crash; check the `cmp`/`jb` in asm.
- Vendored SqPlus differs slightly from the game's revision; patches are marked `// VC1:` (e.g. `GetInstance` ignores the `sq_getinstanceup` result because the Component class typetag never matches).
- Always rebuild after reverting an experiment before measuring again; a stale exe once hid the particle texture branch and cost a long hunt.
- A stub that returns 0 for a factory or a vtable slot fails late and far away (null deref in a constructor, "index does not exist" in a script). When a script says a component or method is missing, check the factory and `GetMember` first.

## Invariants

- Original binary identity: README SHA-256, program name `VirtualCity.Unwrapped.exe`.
- Read Ghidra, write `src/` or squirrel comments, import with the script. Never the reverse.
- No fake host `main`, no `ignore-functions` in the reccmp YAML.
- Default: no sub-agents, no asset upload, no commits of local databases or build output, nothing under `Virtual-City_Win_EN_Full/`.
