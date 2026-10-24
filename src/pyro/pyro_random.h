#ifndef VC1_PYRO_RANDOM_H
#define VC1_PYRO_RANDOM_H

namespace vc1 {

// Inferred names. The Pyro library's random generator: 17 state words with two
// moving cursors. One function-local instance (PyroGetRandom), seeded from
// the QPC clock in nanoseconds.
// SIZE 0x4c
struct PyroRandom
{
    unsigned state[17]; // 0x00
    unsigned* first;    // 0x44
    unsigned* second;   // 0x48

    PyroRandom();

    unsigned Next();
    void Seed(unsigned seed);
    void Randomize();
};

PyroRandom* PyroGetRandom();

// Integrated 1-D noise over [from, to] (turbulence).
float PyroNoise(float from, float to);

// [0, 1], in double as in the original.
inline double PyroRandomUnit()
{
    return PyroGetRandom()->Next() / 4294967295.0;
}

// [-1, 1].
inline double PyroRandomSigned()
{
    return PyroRandomUnit() * 2.0 - 1.0;
}

}

#endif
