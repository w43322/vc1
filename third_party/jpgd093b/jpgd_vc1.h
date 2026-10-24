// VC1: force-included into the jpgd sources (cmake/jpgd.cmake). The game's
// jpgd calls the CRT memcpy (transform_row 0x00457750, load_next_row
// 0x00457d80) but keeps memset as an intrinsic (init 0x00458060,
// decode_scan 0x00458f60).
#include <string.h>
#pragma function(memcpy)
