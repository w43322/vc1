#ifndef VC1_KD_ICLONEABLE_H
#define VC1_KD_ICLONEABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Default vftable written by ICloneable(); KdTexture replaces it.
// SIZE 0x04
// VTABLE: VC1 0x004d1d30
struct ICloneable : IUnknown
{
    virtual void** Clone(void** out) = 0; // vtable+0x0c
};

typedef char Vc1ICloneableSizeCheck[sizeof(ICloneable) == 0x04 ? 1 : -1];

}

#endif
