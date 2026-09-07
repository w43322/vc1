#ifndef VC1_KD_ISHUTDOWNABLE_H
#define VC1_KD_ISHUTDOWNABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IShutdownable : IUnknown
{
    virtual int Shutdown() = 0; // vtable+0x0c
};

typedef char Vc1IShutdownableSizeCheck[sizeof(IShutdownable) == 0x04 ? 1 : -1];

// SIZE 0x04
struct Shutdownable : IShutdownable
{
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int Shutdown();                         // vtable+0x0c
};

typedef char Vc1ShutdownableSizeCheck[sizeof(Shutdownable) == 0x04 ? 1 : -1];

}

#endif
