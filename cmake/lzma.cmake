# LZMA SDK 4.57 C decoder subset, preserving the upstream directory layout.
# No encoder or SDK executable is built. Keep 64-bit CFileSize (default)
# and callback input: original LzmaReadImp 0x004764c0 and SzDecodeCopy
# 0x00476520 pull chunks from ISzInStream. Do not enable _LZMA_OUT_READ.
# _LZMA_PROB32: original 0x00475350 initializes dword probabilities, and
# 0x004765c0 allocates (1846 + (768 << (lc+lp))) * 4 bytes.
set(LZMA457_ROOT "${PROJECT_SOURCE_DIR}/third_party/lzma457")
set(LZMA457_SOURCES
  ${LZMA457_ROOT}/C/7zCrc.c
  ${LZMA457_ROOT}/C/Archive/7z/7zAlloc.c
  ${LZMA457_ROOT}/C/Archive/7z/7zBuffer.c
  ${LZMA457_ROOT}/C/Archive/7z/7zDecode.c
  ${LZMA457_ROOT}/C/Archive/7z/7zExtract.c
  ${LZMA457_ROOT}/C/Archive/7z/7zHeader.c
  ${LZMA457_ROOT}/C/Archive/7z/7zIn.c
  ${LZMA457_ROOT}/C/Archive/7z/7zItem.c
  ${LZMA457_ROOT}/C/Archive/7z/7zMethodID.c
  ${LZMA457_ROOT}/C/Compress/Branch/BranchX86.c
  ${LZMA457_ROOT}/C/Compress/Branch/BranchX86_2.c
  ${LZMA457_ROOT}/C/Compress/Lzma/LzmaDecode.c)
set_source_files_properties(${LZMA457_SOURCES} PROPERTIES
  COMPILE_DEFINITIONS "_LZMA_IN_CB;_LZMA_PROB32")
