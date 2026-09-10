#ifndef VC1_KD_ICOMPONENT_GROUP_H
#define VC1_KD_ICOMPONENT_GROUP_H

#include "kd/iface/ishutdownable.h"

namespace vc1 {

// SIZE 0x04
struct IComponentGroup : IShutdownable
{
    virtual int Unmapped10(void* key, void** out) = 0;
    virtual void** Unmapped14(void** out, void* key) = 0;
    virtual void** Unmapped18(void** out, void* key) = 0;
    virtual int Unmapped1c(void* key) = 0;
    virtual void* Unmapped20() = 0;
    virtual int Unmapped24() = 0;
};

typedef char Vc1IComponentGroupSizeCheck[sizeof(IComponentGroup) == 0x04 ? 1 : -1];

}

#endif
