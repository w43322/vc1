#ifndef VC1_KD_IFUNCTIONAL_GROUP_H
#define VC1_KD_IFUNCTIONAL_GROUP_H

#include "kd/iface/iid.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

// Slot names inferred from KdSystemBase::RegisterObject / UnregisterObject / Shutdown.
// Add returns 0 for a null object or one the group refuses.
// SIZE 0x04
struct IFunctionalGroup : IUnknown
{
    virtual unsigned char Add(void** object) = 0; // vtable+0x0c
    virtual void Remove(void** object) = 0; // vtable+0x10
    virtual void Clear() = 0;               // vtable+0x14
};

typedef char Vc1IFunctionalGroupSizeCheck[sizeof(IFunctionalGroup) == 0x04 ? 1 : -1];

template <>
struct KdIid<IFunctionalGroup>
{
    static const char** Get() { return &g_iidFunctionalGroup; }
};

}

#endif
