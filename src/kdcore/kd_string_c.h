#ifndef VC1_KDCORE_KD_STRING_C_H
#define VC1_KDCORE_KD_STRING_C_H

/* Force-included into every libpng source (cmake/libpng.cmake). The game's
 * libpng compares through MemCompare (0x00499d00, src/bindings/squirrel_object.cpp)
 * instead of the CRT: png_sig_cmp (0x00448530) tail-calls it and
 * png_read_info (0x004b9be0) inlines it for the 4-byte chunk names. */

#include <stdio.h>
#include <string.h>

int MemCompare(const void* left, const void* right, unsigned size);
/* The game's sprintf (0x004969f0, src/kdcore/heap.cpp): png_handle_zTXt and
 * png_do_read_transformations format their messages through it. */
int FormatBufferOpen(char* buf, const char* fmt, ...);

/* The game's libpng logs through KdLog (src/kdcore/heap.cpp) instead of
 * stderr: png_default_error (0x00448320) and png_default_warning. */
void KdLog(const char* fmt, ...);

#define memcmp MemCompare
#define fprintf(stream, ...) KdLog(__VA_ARGS__)
#define sprintf FormatBufferOpen

#endif
