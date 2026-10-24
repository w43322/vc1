# Upstream libpng 1.0.2 (libpng-1.0.2.tar.gz, read side only) and the zlib
# 1.1.3 inflate it needs (zlib-1.1.3.tar.gz; the binary carries
# " inflate 1.1.3 Copyright 1995-1998 Mark Adler " and "1.0.2"). Unmodified apart from one
# marked pngconf.h switch (PNG_NO_READ_INTERLACING); name comments live in
# third_party/libpng102/libpng102.h.
set(ZLIB_ROOT "${PROJECT_SOURCE_DIR}/third_party/zlib113")
set(LIBPNG_ROOT "${PROJECT_SOURCE_DIR}/third_party/libpng102")
set(ZLIB_SOURCES
  ${ZLIB_ROOT}/adler32.c
  ${ZLIB_ROOT}/crc32.c
  ${ZLIB_ROOT}/infblock.c
  ${ZLIB_ROOT}/infcodes.c
  ${ZLIB_ROOT}/inffast.c
  ${ZLIB_ROOT}/inflate.c
  ${ZLIB_ROOT}/inftrees.c
  ${ZLIB_ROOT}/infutil.c
  ${ZLIB_ROOT}/zutil.c)
set(LIBPNG_SOURCES
  ${LIBPNG_ROOT}/png.c
  ${LIBPNG_ROOT}/pngerror.c
  ${LIBPNG_ROOT}/pngget.c
  ${LIBPNG_ROOT}/pngmem.c
  ${LIBPNG_ROOT}/pngread.c
  ${LIBPNG_ROOT}/pngrio.c
  ${LIBPNG_ROOT}/pngrtran.c
  ${LIBPNG_ROOT}/pngrutil.c
  ${LIBPNG_ROOT}/pngset.c
  ${LIBPNG_ROOT}/pngtrans.c)
# The game's pngconf.h options (PNG_NO_READ_INTERLACING needs the marked
# pngconf.h patch), from the original's struct layouts
# (png_struct 0x168 bytes: no user-transform pointers, gamma at 0x124, trans at
# 0x14c, time_buffer last; png_info 0x48 bytes), from the chunks png_read_info
# dispatches (IHDR PLTE IEND IDAT gAMA tRNS zTXt) and from the transform
# branches of png_read_transform_info / png_do_read_transformations (expand,
# strip alpha, gray to RGB, gamma, BGR, filler, swap alpha, swap).
set(LIBPNG_DEFINITIONS
  PNG_NO_READ_BACKGROUND PNG_NO_READ_16_TO_8 PNG_NO_READ_DITHER
  PNG_NO_READ_INVERT PNG_NO_READ_SHIFT PNG_NO_READ_PACK PNG_NO_READ_PACKSWAP
  PNG_NO_READ_INVERT_ALPHA PNG_NO_READ_USER_TRANSFORM
  PNG_NO_PROGRESSIVE_READ PNG_NO_READ_INTERLACING
  PNG_NO_READ_bKGD PNG_NO_READ_cHRM PNG_NO_READ_hIST PNG_NO_READ_oFFs
  PNG_NO_READ_pCAL PNG_NO_READ_pHYs PNG_NO_READ_sBIT PNG_NO_READ_sRGB
  PNG_NO_READ_tEXt PNG_NO_READ_tIME
  PNG_NO_WRITE_TRANSFORMS PNG_NO_WRITE_ANCILLARY_CHUNKS PNG_NO_WRITE_FLUSH
  PNG_NO_WRITE_WEIGHTED_FILTER)
# MY_ZCALLOC: the game's zcalloc/zcfree are in src/memory/global_new.cpp.
set_source_files_properties(${ZLIB_SOURCES} PROPERTIES
  INCLUDE_DIRECTORIES "${ZLIB_ROOT}"
  COMPILE_DEFINITIONS "MY_ZCALLOC"
  COMPILE_FLAGS "/wd4131 /wd4244 /wd4996")
set_source_files_properties(${LIBPNG_SOURCES} PROPERTIES
  INCLUDE_DIRECTORIES "${LIBPNG_ROOT};${ZLIB_ROOT}"
  COMPILE_DEFINITIONS "${LIBPNG_DEFINITIONS}"
  COMPILE_FLAGS "/wd4131 /wd4244 /wd4996")
# The game's libpng compares memory through MemCompare (src/kdcore/kd_string_c.h).
set_property(SOURCE ${LIBPNG_SOURCES} APPEND PROPERTY COMPILE_OPTIONS
  "/FI${PROJECT_SOURCE_DIR}/src/kdcore/kd_string_c.h")
set_property(SOURCE ${LIBPNG_SOURCES} APPEND PROPERTY OBJECT_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/kdcore/kd_string_c.h")
# The game's libpng reads files through the KD layer (src/kdcore/kd_stdio_c.h).
set_property(SOURCE ${LIBPNG_ROOT}/pngrio.c APPEND PROPERTY COMPILE_OPTIONS
  "/FI${PROJECT_SOURCE_DIR}/src/kdcore/kd_stdio_c.h")
set_source_files_properties(${PROJECT_SOURCE_DIR}/src/pyro/pyro_image.cpp PROPERTIES
  COMPILE_DEFINITIONS "${LIBPNG_DEFINITIONS}")
