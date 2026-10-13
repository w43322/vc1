#ifndef VC1_KD_PYRO_H
#define VC1_KD_PYRO_H

#include "kd/iface/ipyro_device.h"

namespace vc1 {

// Inferred name. Object owned by PyroLibrary+0x08; deleted through its
// scalar deleting destructor at vtable+0x0c.
struct PyroResource
{
    virtual void Resource00() = 0; // vtable+0x00
    virtual void Resource04() = 0; // vtable+0x04
    virtual void Resource08() = 0; // vtable+0x08
    virtual ~PyroResource() {}     // vtable+0x0c
};

// Inferred name. Pyro library object owned by g_displayPyro (0x004ed628).
// Built by KdDisplayDevice::SetSize; Done then free in KdDisplayDevice::Shutdown.
// SIZE 0x18
// VTABLE: VC1 0x004d7cd0
class PyroLibrary
{
public:
    IPyroDevice* device;    // 0x04  Init stores its argument here
    PyroResource* resource; // 0x08
    void* buffer;           // 0x0c  freed by Done
    unsigned field10;       // 0x10  Init sets 0x200
    unsigned field14;       // 0x14

    PyroLibrary()
    {
        device = 0;
        resource = 0;
        field10 = 0;
        field14 = 0;
        buffer = 0;
    }

    virtual int Pyro00(void* a);              // vtable+0x00
    virtual int Pyro04(void* a);              // vtable+0x04
    virtual int Init(IPyroDevice* device);    // vtable+0x08
    virtual void Done();                      // vtable+0x0c
    virtual ~PyroLibrary();                   // vtable+0x10
};

// SYNTHETIC: VC1 0x0042ed40
// vc1::PyroLibrary::`scalar deleting destructor'

typedef char Vc1PyroLibrarySizeCheck[sizeof(PyroLibrary) == 0x18 ? 1 : -1];

}

#endif
