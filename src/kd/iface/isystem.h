#ifndef VC1_KD_ISYSTEM_H
#define VC1_KD_ISYSTEM_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ISystem : IUnknown
{
    virtual int Unmapped0c() = 0; // vtable+0x0c
    virtual int Unmapped10() = 0; // vtable+0x10
    virtual int Unmapped14() = 0; // vtable+0x14
    virtual int Unmapped18() = 0; // vtable+0x18
    virtual int Unmapped1c() = 0; // vtable+0x1c
    virtual int Unmapped20() = 0; // vtable+0x20
    virtual int Unmapped24() = 0; // vtable+0x24
    virtual int Start() = 0;      // vtable+0x28
};

typedef char Vc1ISystemSizeCheck[sizeof(ISystem) == 0x04 ? 1 : -1];

}

#endif
