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
// SIZE 0x24
// VTABLE: VC1 0x004d683c vc1::ILoadable
// VTABLE: VC1 0x004d684c vc1::ICloneable
// VTABLE: VC1 0x004d685c vc1::IScriptObject
// VTABLE: VC1 0x004d6874 vc1::ITexture
// VTABLE: VC1 0x004d6894 vc1::IAbstract
class KdTexture : public ILoadable,
                  public ICloneable,
                  public IScriptObject,
                  public ITexture,
                  public IAbstract
{
public:
    unsigned width;  // 0x1c
    unsigned height; // 0x20

    KdTexture();
    void* QueryInterface(const char** iid); // ILoadable vtable+0x00
    unsigned AddRef();                      // ILoadable vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char Load(const char* name);   // ILoadable vtable+0x0c
    void** Clone(void** out);               // ICloneable vtable+0x0c
    unsigned char Script0c(void* a, void* b);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
    unsigned char Texture0c(void* arg);
    int GetWidth();
    int GetHeight();
    void* Lock(void* region, int mode);
    void Unlock();
};

typedef char Vc1KdTextureSizeCheck[sizeof(KdTexture) == 0x24 ? 1 : -1];

void* TextureQueryInterface(ITexture* self, const char** iid);
unsigned TextureAddRef(ITexture* self);

}

#endif
