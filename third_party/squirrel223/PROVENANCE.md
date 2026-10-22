# Squirrel 2.2.3 stable

Selected source files from the upstream `include`, `squirrel` and
`sqstdlib` directories, plus COPYRIGHT. Upstream IDE/project files, command-line
driver, documentation, samples and prebuilt binaries are omitted because the
root CMake target compiles the library sources directly into the reccmp harness.

- URL: https://downloads.sourceforge.net/project/squirrel/squirrel2/squirrel%202.2.3%20stable/squirrel_2.2.3_stable.tar.gz?download=1
- Archive size: 419005 bytes
- SHA-256: a822def76b177024e45a013017df43e390cbbc7fac970a131b50855b54eded37
- License: see COPYRIGHT and the notice in include/squirrel.h.

Build integration is maintained outside this source tree in cmake/squirrel223.cmake.

Local additions include reccmp mapping comments. The marked `// VC1:` patch in
`squirrel/sqvm.cpp`, `SQVM::IsEqual` (original `0x00493230`), compares native-backed
instances by userpointer to reproduce the game's VM. Other recovery work should
add mapping comments without rewriting upstream bodies; host forks live in `src/`.
