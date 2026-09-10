#ifndef VC1_KD_ISYSTEM_H
#define VC1_KD_ISYSTEM_H

#include "kd/iface/icomponent_group.h"

namespace vc1 {

// SIZE 0x04
struct ISystem : IComponentGroup
{
    virtual int Start(IUnknown** object) = 0; // vtable+0x28
};

typedef char Vc1ISystemSizeCheck[sizeof(ISystem) == 0x04 ? 1 : -1];

}

#endif
