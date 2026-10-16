#ifndef VC1_KD_ITEXTURE_H
#define VC1_KD_ITEXTURE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Default vftable written by ITexture(); KdTexture replaces it.
// SIZE 0x08. native is the D3D texture at ITexture+0x04 (KdTexture+0x10).
// VTABLE: VC1 0x004d681c
struct ITexture : IUnknown
{
    IUnknown* native; // 0x04

    ITexture()
        : native(0)
    {
    }

    virtual unsigned char Texture0c(void* arg) = 0;   // vtable+0x0c
    virtual int GetWidth() = 0;                       // vtable+0x10
    virtual int GetHeight() = 0;                      // vtable+0x14
    virtual void* Lock(void* region, bool readOnly) = 0;   // vtable+0x18
    virtual void Unlock() = 0;                        // vtable+0x1c
};

typedef char Vc1ITextureSizeCheck[sizeof(ITexture) == 0x08 ? 1 : -1];

}

#endif
