#ifndef VC1_KD_PYRO_H
#define VC1_KD_PYRO_H

#include "kd/iface/ipyro_device.h"
#include "pyro/pyro_file.h"

namespace vc1 {

// Inferred name. One particle corner as uploaded to the device.
// SIZE 0x18
struct PyroVertex
{
    VC1_PYRO_HEAP

    float x;        // 0x00
    float y;        // 0x04
    float z;        // 0x08
    unsigned color; // 0x0c
    float u;        // 0x10
    float v;        // 0x14
};

// Inferred name. Device vertex buffer owned by PyroLibrary+0x08 (made by
// IPyroDevice::Pyro0c); deleted through its scalar deleting destructor at
// vtable+0x0c.
struct PyroVertexBuffer
{
    virtual void Buffer00() = 0;                                  // vtable+0x00
    virtual void Buffer04() = 0;                                  // vtable+0x04
    virtual int Upload(const void* data, int offset, int size, int discard) = 0; // vtable+0x08
    virtual ~PyroVertexBuffer() {}                                // vtable+0x0c
};

// Inferred name. Pyro library object owned by g_displayPyro (0x004ed628).
// Built by KdDisplayDevice::SetSize; Done then free in KdDisplayDevice::Shutdown.
// Layers batch quads into vertices and upload them into the device buffer as
// a ring of capacity quads.
// SIZE 0x18
// VTABLE: VC1 0x004d7cd0
class PyroLibrary
{
public:
    IPyroDevice* device;            // 0x04  Init stores its argument here
    PyroVertexBuffer* vertexBuffer; // 0x08
    PyroVertex* vertices;           // 0x0c  scratch, capacity * 4
    int capacity;                   // 0x10  quads; Init sets 0x200
    int vertexCursor;               // 0x14  next free vertex in vertexBuffer

    PyroLibrary()
    {
        device = 0;
        vertexBuffer = 0;
        capacity = 0;
        vertexCursor = 0;
        vertices = 0;
    }

    virtual PyroFile* LoadFile(IPyroStream* stream); // vtable+0x00
    virtual int Pyro04(void* a);              // vtable+0x04
    virtual void Init(IPyroDevice* device);   // vtable+0x08
    virtual void Done();                      // vtable+0x0c
    virtual ~PyroLibrary();                   // vtable+0x10
};

// SYNTHETIC: VC1 0x0042ed40
// vc1::PyroLibrary::`scalar deleting destructor'

typedef char Vc1PyroLibrarySizeCheck[sizeof(PyroLibrary) == 0x18 ? 1 : -1];
typedef char Vc1PyroVertexSizeCheck[sizeof(PyroVertex) == 0x18 ? 1 : -1];

}

#endif
