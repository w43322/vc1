# SDL3 target: the original's imports, implemented over SDL3 in
# src/imports/sdl3/ (one file per DLL). Its include/ directory replaces the
# Windows SDK headers for every source in the target.
include(cmake/sdl3.cmake)
include(cmake/sdl3-mixer.cmake)
set(VC1_IMPORTS_DIR src/imports/sdl3)
set(VC1_IMPORT_SOURCES
  src/imports/sdl3/sdl3.cpp
  src/imports/sdl3/kernel32.cpp
  src/imports/sdl3/user32.cpp
  src/imports/sdl3/gdi32.cpp
  src/imports/sdl3/shell32.cpp
  src/imports/sdl3/shlwapi.cpp
  src/imports/sdl3/winmm.cpp
  src/imports/sdl3/ws2_32.cpp
  src/imports/sdl3/d3d8.cpp
  src/imports/sdl3/bass.cpp)
set(VC1_IMPORT_LIBRARIES SDL3::SDL3 SDL3_mixer::SDL3_mixer legacy_stdio_definitions)
