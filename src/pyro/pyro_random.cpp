#include "host/clock.h"
#include "pyro/pyro_random.h"

#include "host/platform.h"

#include <math.h>
#include <stdlib.h>

namespace vc1 {

PyroRandom::PyroRandom()
{
    PlatformClockSample qpc;

    first = state;
    second = state + 1;
    PlatformReadCounter(&qpc);
    Seed(static_cast<unsigned>(static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick)));
}

// Function-local static (guard bit 0x004ed874, object 0x004ed878).
// FUNCTION: VC1 0x0042e790
PyroRandom* PyroGetRandom()
{
    static PyroRandom random;
    return &random;
}

// Value noise lattice: a shuffled permutation of 0..511 and random values in
// [-1, 1), each followed by a copy of its first 0x202 entries.
// GLOBAL: VC1 0x004eb0f8
int g_pyroNoisePerm[0x200 + 0x202];
// GLOBAL: VC1 0x004ec100
float g_pyroNoiseValue[0x200 + 0x202];
// GLOBAL: VC1 0x004ed8c4
bool g_pyroNoiseReady;

// cdecl. Inferred name. Integral of the quintic-faded segment between lattice
// values a and b over [from, to], both inside one cell.
// FUNCTION: VC1 0x0044cf60
float PyroNoiseSpan(float a, float b, float from, float to)
{
    const float e2 = to * to;
    const float e3 = e2 * to;
    const float s2 = from * from;
    const float ae = a * to;
    const float s3 = s2 * from;
    const float k = from * e3 * 8.0f;
    const float head = ((e2 * from * 15.0f * a +
        ((e2 * 20.0f * b +
        ((e3 * 15.0f * a + ((b * e2 * e2 * 8.0f - e2 * e2 * a * 8.0f) - e3 * b * 25.0f) + b * k) -
        a * k)) - e2 * from * 25.0f * b) + s2 * e2 * 8.0f * b) - s2 * e2 * 8.0f * a);
    const float tail = ((a * 15.0f * s3 +
        ((s2 * 20.0f * b +
        (((to * 8.0f * b * s3 + ae * 15.0f * s2 +
        ((to * 20.0f * from * b - ae * 10.0f) - to * 25.0f * s2 * b)) - ae * 8.0f * s3) -
        from * 10.0f * a)) - b * 25.0f * s3) + s2 * s2 * b * 8.0f) - s2 * s2 * a * 8.0f);
    return ((head + tail) * (to - from)) / -20.0f;
}

// cdecl. Inferred name. The same integral over [from, 1].
// FUNCTION: VC1 0x0044d150
float PyroNoiseHead(float a, float b, float from)
{
    const float s2 = from * from;
    return ((((((((s2 * from * b * 8.0f - s2 * from * a * 8.0f) - s2 * a) - s2 * 9.0f * b) -
        b * from * 6.0f) + a * from * 6.0f) - b * 3.0f) + a * 3.0f) *
        (from - 1.0f) * (from - 1.0f)) / 20.0f;
}

// cdecl. Inferred name. The same integral over [0, to].
// FUNCTION: VC1 0x0044d1f0
float PyroNoiseTail(float a, float b, float to)
{
    const float s2 = to * to;
    const float k = s2 * to * 8.0f;
    return (-s2 * ((a * 15.0f * s2 + ((b * k - a * k) - s2 * 25.0f * b) + to * 20.0f * b) -
        a * 10.0f)) / 20.0f;
}

// Inferred name. Fills the lattice once from rand().
// FUNCTION: VC1 0x0044d430
void PyroNoiseInit()
{
    int i;

    if (g_pyroNoiseReady) {
        return;
    }
    g_pyroNoiseReady = true;
    for (i = 0; i < 0x200; ++i) {
        g_pyroNoisePerm[i] = i;
        g_pyroNoiseValue[i] = static_cast<float>(rand() % 0x400 - 0x200) * (1.0f / 512.0f);
    }
    for (i = 0x1ff; i > 0; --i) {
        const int value = g_pyroNoisePerm[i];
        const int j = rand() % 0x200;
        g_pyroNoisePerm[i] = g_pyroNoisePerm[j];
        g_pyroNoisePerm[j] = value;
    }
    for (i = 0; i < 0x202; ++i) {
        g_pyroNoiseValue[0x200 + i] = g_pyroNoiseValue[i];
        g_pyroNoisePerm[0x200 + i] = g_pyroNoisePerm[i];
    }
}

static inline float PyroNoiseAt(int cell)
{
    return g_pyroNoiseValue[g_pyroNoisePerm[cell & 0x1ff]];
}

// cdecl. Integral of the value noise over [from, to] (swapped if reversed),
// biased by 10000 so the cells stay positive; whole cells add 0.15 * (v0 - v1).
// FUNCTION: VC1 0x0044d5c0
float PyroNoise(float from, float to)
{
    if (from > to) {
        const float swap = to;
        to = from;
        from = swap;
    }
    PyroNoiseInit();
    const float biasFrom = static_cast<float>(from + 10000.0);
    const float floorFrom = static_cast<float>(floor(biasFrom));
    const int first = static_cast<int>(floorFrom);
    const float fracFrom = biasFrom - floorFrom;
    const float biasTo = static_cast<float>(to + 10000.0);
    const float floorTo = static_cast<float>(floor(biasTo));
    const int last = static_cast<int>(floorTo);
    const float fracTo = biasTo - floorTo;
    float sum = 0.0f;

    for (int cell = first + 1; cell < last; ++cell) {
        sum += (PyroNoiseAt(cell) - PyroNoiseAt(cell + 1)) * 0.15f;
    }
    if (first == last) {
        return PyroNoiseSpan(PyroNoiseAt(first), PyroNoiseAt(first + 1), fracFrom, fracTo) + sum;
    }
    const float head = PyroNoiseHead(PyroNoiseAt(first), PyroNoiseAt(first + 1), fracFrom);
    return PyroNoiseTail(PyroNoiseAt(last), PyroNoiseAt(last + 1), fracTo) + (head + sum);
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
