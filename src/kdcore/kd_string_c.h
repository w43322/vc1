#ifndef VC1_KDCORE_KD_STRING_C_H
#define VC1_KDCORE_KD_STRING_C_H

/* Force-included into every libpng source (cmake/libpng.cmake). The game's
 * libpng compares through MemCompare (0x00499d00, src/bindings/squirrel_object.cpp)
 * instead of the CRT: png_sig_cmp (0x00448530) tail-calls it and
 * png_read_info (0x004b9be0) inlines it for the 4-byte chunk names. */

#include <string.h>

int MemCompare(const void* left, const void* right, unsigned size);

#define memcmp MemCompare

#endif
