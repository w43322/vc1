# Upstream https://git.kernel.org/pub/scm/libs/klibc/klibc.git usr/klibc/.
# vsscanf.c carries one marked VC9 patch; vc9/ supplies <stdint.h> and
# <inttypes.h> for these files only, force-included because sscanf.c expects
# klibc's <stdio.h> to declare vsscanf and va_list.
set(KLIBC_ROOT "${PROJECT_SOURCE_DIR}/third_party/klibc")
set(KLIBC_SOURCES
  ${KLIBC_ROOT}/sscanf.c
  ${KLIBC_ROOT}/strntoumax.c
  ${KLIBC_ROOT}/vsscanf.c)
set_source_files_properties(${KLIBC_SOURCES} PROPERTIES
  INCLUDE_DIRECTORIES "${KLIBC_ROOT}/vc9"
  COMPILE_DEFINITIONS "inline=__inline"
  COMPILE_FLAGS "/FIinttypes.h")
