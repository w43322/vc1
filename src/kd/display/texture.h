#ifndef VC1_KD_TEXTURE_H
#define VC1_KD_TEXTURE_H

#include "kd/iface/iloadable.h"
#include "kd/iface/icloneable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itexture.h"
#include "kd/iface/iabstract.h"
#include "kd/display/d3d8_api.h"

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
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* a, void* b);
    const char* GetTypeName();
    unsigned char Reload(void* arg);
    int GetWidth();
    int GetHeight();
    void* Lock(void* region, bool readOnly);
    void Unlock();
};

typedef char Vc1KdTextureSizeCheck[sizeof(KdTexture) == 0x24 ? 1 : -1];

// Inferred name. Texture face of a render target (KdRenderTarget::GetRenderTexture):
// borrows the target's D3D texture; native is refreshed on every GetRenderTexture.
// SIZE 0x1c
// VTABLE: VC1 0x004d68e4 vc1::ITexture
// VTABLE: VC1 0x004d6904 vc1::IAbstract
class KdRenderTexture : public ITexture,
                        public IAbstract
{
public:
    unsigned width;     // 0x10
    unsigned height;    // 0x14
    IDirect3DSurface8* readback; // 0x18  surface held by Lock

    explicit KdRenderTexture(IDirect3DTexture8* texture);
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char Reload(void* arg);
    int GetWidth();
    int GetHeight();
    void* Lock(void* region, bool readOnly);
    void Unlock();
};

typedef char Vc1KdRenderTextureSizeCheck[sizeof(KdRenderTexture) == 0x1c ? 1 : -1];

// SYNTHETIC: VC1 0x0041d2f0
// vc1::KdTexture::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041d580
// vc1::KdRenderTexture::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041d5f0
// ?Release@KdRenderTexture@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041d600
// ?QueryInterface@KdRenderTexture@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041d610
// ??_EKdRenderTexture@vc1@@W7AEPAXI@Z
// SYNTHETIC: VC1 0x0041d620
// ?AddRef@KdRenderTexture@vc1@@W7AEIXZ

}

#endif
