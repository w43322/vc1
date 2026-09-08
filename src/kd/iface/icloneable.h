#ifndef VC1_KD_ICLONEABLE_H
#define VC1_KD_ICLONEABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ICloneable : IUnknown
{
};

typedef char Vc1ICloneableSizeCheck[sizeof(ICloneable) == 0x04 ? 1 : -1];

}

#endif
