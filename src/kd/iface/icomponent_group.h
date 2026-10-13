#ifndef VC1_KD_ICOMPONENT_GROUP_H
#define VC1_KD_ICOMPONENT_GROUP_H

#include "kd/iface/ishutdownable.h"

namespace vc1 {

class KdFactory;

// SIZE 0x04
struct IComponentGroup : IShutdownable
{
    virtual int Unmapped10(void* key, void** out) = 0;   // vtable+0x10
    virtual void** Unmapped14(void** out, void* key) = 0; // vtable+0x14
    virtual void** Unmapped18(void** out, void* key) = 0; // vtable+0x18
    virtual KdFactory* Unmapped1c(const char** iid) = 0;  // vtable+0x1c
    virtual void* Unmapped20() = 0;                      // vtable+0x20
    virtual int Unmapped24() = 0;                        // vtable+0x24
};

typedef char Vc1IComponentGroupSizeCheck[sizeof(IComponentGroup) == 0x04 ? 1 : -1];

}

#endif
