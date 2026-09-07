#ifndef VC1_KD_IRENDER_TARGET_H
#define VC1_KD_IRENDER_TARGET_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x10
struct IRenderTarget : IUnknown
{
    unsigned unk04; // 0x04
    unsigned unk08; // 0x08
    void* unk0c;    // 0x0c
};

typedef char Vc1IRenderTargetSizeCheck[sizeof(IRenderTarget) == 0x10 ? 1 : -1];

}

#endif
