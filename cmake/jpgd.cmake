# Upstream Small JPEG Decoder Library v0.93b by Rich Geldreich
# (http://subquantumtech.com/jpeg/jpgd093b.zip; inc/ and jpgdlib/ flattened).
# jpegdecoder.h carries one marked VC1 patch: the game was built without
# SUPPORT_X86ASM / SUPPORT_MMX, so jidctfst.cpp (MMX IDCT only) is not built.
set(JPGD_ROOT "${PROJECT_SOURCE_DIR}/third_party/jpgd093b")
set(JPGD_SOURCES
  ${JPGD_ROOT}/jpegdecoder.cpp
  ${JPGD_ROOT}/H2v2.cpp
  ${JPGD_ROOT}/idct.cpp)
# The library relies on the VC6 for-loop scope rule.
set_source_files_properties(${JPGD_SOURCES} PROPERTIES
  INCLUDE_DIRECTORIES "${JPGD_ROOT}"
  COMPILE_FLAGS "/Zc:forScope-")
