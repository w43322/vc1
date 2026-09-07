#ifndef VC1_KD_ICOMPONENT_GROUP_H
#define VC1_KD_ICOMPONENT_GROUP_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IComponentGroup : IUnknown
{
};

typedef char Vc1IComponentGroupSizeCheck[sizeof(IComponentGroup) == 0x04 ? 1 : -1];

}

#endif
