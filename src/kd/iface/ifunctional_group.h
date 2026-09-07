#ifndef VC1_KD_IFUNCTIONAL_GROUP_H
#define VC1_KD_IFUNCTIONAL_GROUP_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IFunctionalGroup : IUnknown
{
};

typedef char Vc1IFunctionalGroupSizeCheck[sizeof(IFunctionalGroup) == 0x04 ? 1 : -1];

}

#endif
