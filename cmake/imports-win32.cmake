# Reference target: the original's imports are the real Windows DLLs.
set(VC1_IMPORTS_DIR src/imports/win32)
set(VC1_IMPORT_SOURCES)
set(VC1_IMPORT_LIBRARIES user32 ws2_32 shell32 shlwapi winmm bass_import d3d8_import)

# Import library for BASS.dll (BASS 2.4, linked statically by the original:
# the BASS_* calls go through import thunks at 0x0044467a+). VC9 cannot make a
# stdcall import library with undecorated import names from a .def alone, so a
# throwaway DLL built from placeholder bodies emits it. The DLL goes to its own
# directory so the game never loads it instead of the real one.
add_library(bass_import SHARED src/imports/win32/bass_stub.c src/imports/win32/bass.def)
set_source_files_properties(src/imports/win32/bass_stub.c PROPERTIES COMPILE_FLAGS "/GL-")
set_target_properties(bass_import PROPERTIES
  OUTPUT_NAME "BASS"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bass-import"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bass-import"
  LINK_FLAGS "/NOENTRY /INCREMENTAL:NO")

# The recompiled exe runs from bin/ with the game data as the working
# directory, so it needs the game's own BASS.dll beside it (local copy only).
set(VC1_GAME_BASS_DLL "${CMAKE_SOURCE_DIR}/Virtual-City_Win_EN_Full/bass.dll")

# Import library for d3d8.dll. The original imports Direct3DCreate8 directly
# (thunk 0x00442e0e); the Windows SDK that ships with VC9 has no d3d8.lib, so
# a throwaway DLL built from a placeholder body emits it, as for BASS. The DLL
# goes to its own directory so the game never loads it instead of the system one.
add_library(d3d8_import SHARED src/imports/win32/d3d8_stub.c src/imports/win32/d3d8.def)
set_source_files_properties(src/imports/win32/d3d8_stub.c PROPERTIES COMPILE_FLAGS "/GL-")
set_target_properties(d3d8_import PROPERTIES
  OUTPUT_NAME "d3d8"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/d3d8-import"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/d3d8-import"
  LINK_FLAGS "/NOENTRY /INCREMENTAL:NO")
