#include "squirrel_object.h"

#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004cfa98
const char g_sqplusErrorFormat[] = "error: %s\n";

}

// The compiler's memcmp intrinsic (DWORD loop, then bytes; a mismatch is
// (diff SAR 31) | 1) as an LTCG function.
// FUNCTION: VC1 0x00499d00
extern "C" int MemCompare(const void* left, const void* right, unsigned size)
{
    return memcmp(left, right, size);
}
