#ifndef VC1_KD_ISYSTEM_H
#define VC1_KD_ISYSTEM_H

#include "kd/iface/ishutdownable.h"

namespace vc1 {

// SIZE 0x04
struct ISystem : IShutdownable
{
    virtual int Unmapped10() = 0; // vtable+0x10
    virtual void** Unmapped14(void** out, void* key) = 0; // vtable+0x14
    virtual void** Unmapped18(void** out, void* a) = 0; // vtable+0x18
    virtual int Unmapped1c(void* arg) = 0; // vtable+0x1c
    virtual void* Unmapped20() = 0; // vtable+0x20  returns IInputStream+0x04
    virtual int Unmapped24() = 0; // vtable+0x24
    virtual int Start(IUnknown** object) = 0; // vtable+0x28
};

typedef char Vc1ISystemSizeCheck[sizeof(ISystem) == 0x04 ? 1 : -1];

}

#endif
