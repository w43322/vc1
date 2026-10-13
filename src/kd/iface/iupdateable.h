#ifndef VC1_KD_IUPDATEABLE_H
#define VC1_KD_IUPDATEABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IUpdateable : IUnknown
{
    virtual int Unmapped0c() = 0;              // vtable+0x0c
    virtual int SetLayer(unsigned layer) = 0; // vtable+0x10
    virtual void Update(unsigned sliceBit, int dt) = 0; // vtable+0x14
};

typedef char Vc1IUpdateableSizeCheck[sizeof(IUpdateable) == 0x04 ? 1 : -1];

}

#endif
