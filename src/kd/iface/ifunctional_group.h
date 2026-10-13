#ifndef VC1_KD_IFUNCTIONAL_GROUP_H
#define VC1_KD_IFUNCTIONAL_GROUP_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Slot names inferred from KdSystemBase::Unmapped10 / Unmapped14 / Shutdown.
// SIZE 0x04
struct IFunctionalGroup : IUnknown
{
    virtual void Add(void** object) = 0;    // vtable+0x0c
    virtual void Remove(void** object) = 0; // vtable+0x10
    virtual void Clear() = 0;               // vtable+0x14
};

typedef char Vc1IFunctionalGroupSizeCheck[sizeof(IFunctionalGroup) == 0x04 ? 1 : -1];

}

#endif
