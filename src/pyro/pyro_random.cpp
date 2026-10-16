#include "pyro/pyro_random.h"

#include "host/runtime.h"

#include <stdlib.h>

namespace vc1 {

PyroRandom::PyroRandom()
{
    LARGE_INTEGER qpc;

    first = state;
    second = state + 1;
    QueryPerformanceCounter(&qpc);
    Seed(static_cast<unsigned>(static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick)));
}

// Function-local static (guard bit 0x004ed874, object 0x004ed878).
// FUNCTION: VC1 0x0042e790
PyroRandom* PyroGetRandom()
{
    static PyroRandom random;
    return &random;
}

// cdecl. Body not recovered yet: permutation/gradient tables at 0x004eb0f8 /
// 0x004ec100 (lazy init 0x0044d430) and segment helpers 0x0044cf60,
// 0x0044d150, 0x0044d1f0.
// STUB: VC1 0x0044d5c0
float PyroNoise(float from, float to)
{
    (void)from;
    (void)to;
    return 0.0f;
}

// EDI=this. When the second cursor wraps the first one stays put.
// FUNCTION: VC1 0x0044d8c0
unsigned PyroRandom::Next()
{
    unsigned a = *second;
    const unsigned b = *first;
    const unsigned mix = ((b + a) << 7) ^ (a >> 7);
    unsigned* cursor;

    a += mix * 0x173 + 0xb4e50197;
    *first = b ^ a;
    *second += a * a * 0x13d + mix + 0x23bf9c;
    cursor = second;
    do {
        if (++cursor == state + 17) {
            cursor = state;
        }
        else if (++first == state + 17) {
            first = state;
        }
    } while (first == cursor);
    second = cursor;
    return a;
}

// EAX=seed; this is the static instance.
// FUNCTION: VC1 0x0044d930
void PyroRandom::Seed(unsigned seed)
{
    for (int i = 0; i < 17; ++i) {
        state[i] = (seed - 0x5d4cf43) ^ (seed + 0x14593bf1);
        seed = _rotr(seed, 7) + 0x6768eb15;
    }
}

}
