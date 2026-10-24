# Upstream libtga 1.0.1 (https://tgalib.sourceforge.net/) src/{tga.c,tgaread.c}.
# Compiled as C++ so TGA_ERROR can log through vc1::KdLog; file I/O runs on KD
# files through src/kdcore/kd_tga_stdio.h. tgaconfig.h is a VC1 replacement.
set(LIBTGA_ROOT "${PROJECT_SOURCE_DIR}/third_party/libtga")
set(LIBTGA_SOURCES
  ${LIBTGA_ROOT}/tga.c
  ${LIBTGA_ROOT}/tgaread.c)
set_source_files_properties(${LIBTGA_SOURCES} PROPERTIES
  INCLUDE_DIRECTORIES "${LIBTGA_ROOT}"
  COMPILE_FLAGS "/TP /FIkdcore/kd_tga_stdio.h"
  OBJECT_DEPENDS "${PROJECT_SOURCE_DIR}/src/kdcore/kd_tga_stdio.h;${PROJECT_SOURCE_DIR}/src/kdcore/kd_stdio.h")
