#ifndef VC1_KD_ILOADABLE_H
#define VC1_KD_ILOADABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ILoadable : IUnknown
{
};

typedef char Vc1ILoadableSizeCheck[sizeof(ILoadable) == 0x04 ? 1 : -1];

}

#endif
