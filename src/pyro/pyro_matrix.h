#ifndef VC1_PYRO_MATRIX_H
#define VC1_PYRO_MATRIX_H

namespace vc1 {

// Inferred names. Small math types of the Pyro particle library.

// SIZE 0x10
struct PyroAxisAngle
{
    float angle;   // 0x00  radians
    float axis[3]; // 0x04
};

// SIZE 0x10
struct PyroQuat
{
    float w; // 0x00
    float x; // 0x04
    float y; // 0x08
    float z; // 0x0c

    PyroQuat* FromAxisAngle(const PyroAxisAngle* source);
};

// Row-major 3x4 affine matrix; the fourth row is implicitly (0, 0, 0, 1).
// SIZE 0x30
struct PyroMatrix34
{
    float m[3][4];

    void Scale(float x, float y, float z, float w);
    void SetRotation(const PyroQuat* q);
    PyroMatrix34* Multiply(PyroMatrix34* out, const PyroMatrix34* b) const;
};

typedef char Vc1PyroMatrix34SizeCheck[sizeof(PyroMatrix34) == 0x30 ? 1 : -1];

}

#endif
