#include "squirrel_object.h"

namespace vc1 {

// GLOBAL: VC1 0x004cfa98
const char g_sqplusErrorFormat[] = "error: %s\n";

// DWORD loop then leftover bytes. Mismatch sign is (diff SAR 31) | 1.
// FUNCTION: VC1 0x00499d00
int MemCompare(const void* left, const void* right, unsigned size)
{
    const unsigned char* a = static_cast<const unsigned char*>(left);
    const unsigned char* b = static_cast<const unsigned char*>(right);
    int diff;

    while (size >= 4) {
        if (*reinterpret_cast<const unsigned*>(a)
            != *reinterpret_cast<const unsigned*>(b)) {
            break;
        }
        a += 4;
        b += 4;
        size -= 4;
    }
    if (size == 0) {
        return 0;
    }

    diff = static_cast<int>(a[0]) - static_cast<int>(b[0]);
    if (diff == 0 && size > 1) {
        diff = static_cast<int>(a[1]) - static_cast<int>(b[1]);
    }
    if (diff == 0 && size > 2) {
        diff = static_cast<int>(a[2]) - static_cast<int>(b[2]);
    }
    if (diff == 0 && size > 3) {
        diff = static_cast<int>(a[3]) - static_cast<int>(b[3]);
    }
    if (diff == 0) {
        return 0;
    }
    return (diff >> 31) | 1;
}

}
