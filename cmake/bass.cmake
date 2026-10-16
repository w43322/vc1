# Import library for BASS.dll (BASS 2.4, linked statically by the original:
# the BASS_* calls go through import thunks at 0x0044467a+). VC9 cannot make a
# stdcall import library with undecorated import names from a .def alone, so a
# throwaway DLL built from placeholder bodies emits it. The DLL goes to its own
# directory so the game never loads it instead of the real one.
add_library(bass_import SHARED cmake/bass_stub.c cmake/bass.def)
set_source_files_properties(cmake/bass_stub.c PROPERTIES COMPILE_FLAGS "/GL-")
set_target_properties(bass_import PROPERTIES
  OUTPUT_NAME "BASS"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bass-import"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bass-import"
  LINK_FLAGS "/NOENTRY /INCREMENTAL:NO")

# The recompiled exe runs from bin/ with the game data as the working
# directory, so it needs the game's own BASS.dll beside it (local copy only).
set(VC1_GAME_BASS_DLL "${CMAKE_SOURCE_DIR}/Virtual-City_Win_EN_Full/bass.dll")
