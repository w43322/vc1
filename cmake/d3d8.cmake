# Import library for d3d8.dll. The original imports Direct3DCreate8 directly
# (thunk 0x00442e0e); the Windows SDK that ships with VC9 has no d3d8.lib, so
# a throwaway DLL built from a placeholder body emits it, as for BASS. The DLL
# goes to its own directory so the game never loads it instead of the system one.
add_library(d3d8_import SHARED cmake/d3d8_stub.c cmake/d3d8.def)
set_source_files_properties(cmake/d3d8_stub.c PROPERTIES COMPILE_FLAGS "/GL-")
set_target_properties(d3d8_import PROPERTIES
  OUTPUT_NAME "d3d8"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/d3d8-import"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/d3d8-import"
  LINK_FLAGS "/NOENTRY /INCREMENTAL:NO")
