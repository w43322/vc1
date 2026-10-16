#ifndef VC1_PYRO_DEVICE_H
#define VC1_PYRO_DEVICE_H

#include "pyro/pyro_string.h"

namespace vc1 {

// Inferred names. Objects the Pyro device creates for the library. The
// library deletes them through the virtual destructor (heap free).

// Decoded 32-bit image handed to the device (IPyroDevice::Pyro18). The
// library frees the pixels itself before deleting the image.
// SIZE 0x14
// VTABLE: VC1 0x004d6240
struct PyroDeviceImage
{
    VC1_PYRO_HEAP

    void* pixels; // 0x04
    int width;    // 0x08
    int height;   // 0x0c
    int flags;    // 0x10

    PyroDeviceImage(void* data, int w, int h, int f)
        : pixels(data), width(w), height(h), flags(f)
    {
    }
    virtual ~PyroDeviceImage() {} // vtable+0x00
};

// SYNTHETIC: VC1 0x0041aba0
// vc1::PyroDeviceImage::`scalar deleting destructor'

// Device texture made from an image (IPyroDevice::Pyro14).
// SIZE 0x04
// VTABLE: VC1 0x004d6244
struct PyroDeviceObject
{
    VC1_PYRO_HEAP

    virtual int Object00() { return 0; } // vtable+0x00
    virtual int Object04() { return 0; } // vtable+0x04
    virtual ~PyroDeviceObject() {}       // vtable+0x08
};

// SYNTHETIC: VC1 0x0041abc0
// vc1::PyroDeviceObject::`scalar deleting destructor'

// Vertex buffer as the device draws it (IPyroDevice::Pyro08, Pyro24).
// SIZE 0x04
// VTABLE: VC1 0x004d6250
struct PyroDeviceBuffer
{
    VC1_PYRO_HEAP

    virtual int Buffer00() { return 0; } // vtable+0x00
    virtual int Buffer04() { return 0; } // vtable+0x04
    virtual int Buffer08() = 0;          // vtable+0x08
    virtual int Buffer0c() = 0;          // vtable+0x0c
    virtual void* GetData() = 0;         // vtable+0x10
    virtual ~PyroDeviceBuffer() {}       // vtable+0x14
};

// SYNTHETIC: VC1 0x0041abe0
// vc1::PyroDeviceBuffer::`scalar deleting destructor'

// Vertex buffer as the library fills it (IPyroDevice::Pyro0c, Pyro28).
// SIZE 0x04
// VTABLE: VC1 0x004d6268
struct PyroVertexBuffer
{
    VC1_PYRO_HEAP

    virtual int Buffer00() { return 0; }                                  // vtable+0x00
    virtual int Buffer04() { return 0; }                                  // vtable+0x04
    virtual int Upload(const void* data, int offset, int size, int discard) = 0; // vtable+0x08
    virtual ~PyroVertexBuffer() {}                                        // vtable+0x0c
};

// SYNTHETIC: VC1 0x0041ac00
// vc1::PyroVertexBuffer::`scalar deleting destructor'

typedef char Vc1PyroDeviceImageSizeCheck[sizeof(PyroDeviceImage) == 0x14 ? 1 : -1];

}

#endif
