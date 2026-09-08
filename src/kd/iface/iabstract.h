#ifndef VC1_KD_IABSTRACT_H
#define VC1_KD_IABSTRACT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IAbstract : IUnknown
{
    virtual ~IAbstract() = 0; // vtable+0x0c
};

typedef char Vc1IAbstractSizeCheck[sizeof(IAbstract) == 0x04 ? 1 : -1];

// SIZE 0x04
struct Abstract : IAbstract
{
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1AbstractSizeCheck[sizeof(Abstract) == 0x04 ? 1 : -1];

}

#endif
