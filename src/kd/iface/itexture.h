#ifndef VC1_KD_ITEXTURE_H
#define VC1_KD_ITEXTURE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x08. native is the D3D texture at ITexture+0x04 (KdTexture+0x10).
struct ITexture : IUnknown
{
    IUnknown* native; // 0x04
};

typedef char Vc1ITextureSizeCheck[sizeof(ITexture) == 0x08 ? 1 : -1];

}

#endif
