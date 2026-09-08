#ifndef VC1_KD_TEXTURE_H
#define VC1_KD_TEXTURE_H

#include "kd/iface/iloadable.h"
#include "kd/iface/icloneable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itexture.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

// Inferred name. Factory string "Texture" (0x004d4c38). QI at 0x0045c020:
// ILoadable this, ICloneable+4, IScriptObject+8, ITexture+0x0c, IAbstract+0x14.
// Created by IDisplay::CreateTexture.
// VTABLE: VC1 0x004d683c vc1::ILoadable
// VTABLE: VC1 0x004d684c vc1::ICloneable
// VTABLE: VC1 0x004d685c vc1::IScriptObject
// VTABLE: VC1 0x004d6874 vc1::ITexture
// VTABLE: VC1 0x004d6894 vc1::IAbstract
// SIZE 0x24
class KdTexture : public ILoadable,
                  public ICloneable,
                  public IScriptObject,
                  public ITexture,
                  public IAbstract
{
public:
    unsigned refs;   // 0x18
    unsigned width;  // 0x1c
    unsigned height; // 0x20

    void* QueryInterface(const char** iid); // ILoadable vtable+0x00
    unsigned AddRef();                      // ILoadable vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1KdTextureSizeCheck[sizeof(KdTexture) == 0x24 ? 1 : -1];

void* TextureQueryInterface(ITexture* self, const char** iid);
unsigned TextureAddRef(ITexture* self);
KdTexture* ConstructKdTexture(KdTexture* self);

}

#endif
