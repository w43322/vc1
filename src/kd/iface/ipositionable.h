#ifndef VC1_KD_IPOSITIONABLE_H
#define VC1_KD_IPOSITIONABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Two position operations after IUnknown; default vftable 0x004d10cc.
// SIZE 0x04
// VTABLE: VC1 0x004d10cc
struct IPositionable : IUnknown
{
    virtual void* Position0c(void* out) = 0;
    virtual void Position10(const void* value) = 0;
};

typedef char Vc1IPositionableSizeCheck[sizeof(IPositionable) == 0x04 ? 1 : -1];

}

#endif
