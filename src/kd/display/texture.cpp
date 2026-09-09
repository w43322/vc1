#include "kd/display/texture.h"

#include "kd/iface/iid.h"

namespace vc1 {

// XOR AL,AL; RET 4. ITexture vtable+0x0c.
// FUNCTION: VC1 0x00418000
unsigned char KdTexture::Texture0c(void* arg)
{
    (void)arg;
    return 0;
}

// this = ITexture. ITexture or IAbstract at +8. RET 4.
// FUNCTION: VC1 0x0045bc30
void* TextureQueryInterface(ITexture* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidTexture) {
        return self;
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(self) + 8;
    }
    return 0;
}

// this = ITexture. refs at ITexture+0x0c (KdTexture+0x18). RET.
// FUNCTION: VC1 0x0045bc80
unsigned TextureAddRef(ITexture* self)
{
    IAbstract* face = reinterpret_cast<IAbstract*>(reinterpret_cast<char*>(self) + 8);
    unsigned value = face->refs;
    if (value != 0) {
        ++value;
        face->refs = value;
    }
    return value;
}

// MOV EAX,[ECX+0x14]; RET. this=ITexture (KdTexture+0x0c).
// FUNCTION: VC1 0x0045bfe0
int KdTexture::GetHeight()
{
    return static_cast<int>(height);
}

// MOV EAX,[ECX+0x10]; RET. this=ITexture (KdTexture+0x0c).
// FUNCTION: VC1 0x0045bff0
int KdTexture::GetWidth()
{
    return static_cast<int>(width);
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x0045c000
unsigned char KdTexture::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Calls native texture vtable+0x44(native,0). this=ITexture.
// FUNCTION: VC1 0x0045c010
void KdTexture::Unlock()
{
    typedef void (__thiscall *Fn)(IUnknown* object, int arg);
    void** vtbl = *reinterpret_cast<void***>(native);
    reinterpret_cast<Fn>(vtbl[0x11])(native, 0);
}

// this = ILoadable. RET 4.
// FUNCTION: VC1 0x0045c020
void* KdTexture::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidTexture) {
        if (!this) {
            return 0;
        }
        return static_cast<ITexture*>(this);
    }
    if (name == g_iidLoadable) {
        return this;
    }
    if (name == g_iidCloneable) {
        if (!this) {
            return 0;
        }
        return static_cast<ICloneable*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 0x14; decrement refs at IAbstract+4. If 0, vtbl+0x0c(this, 1).
// FUNCTION: VC1 0x0045c080
unsigned KdTexture::Release()
{
    IUnknown* face;
    unsigned result;
    typedef void (__thiscall *Fn0c)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<Fn0c>((*reinterpret_cast<void***>(face))[3])(face, 1);
        return 0;
    }
    return result;
}

// MOV EAX, [ECX+0x18]; TEST; INC; store. RET.
// FUNCTION: VC1 0x0045c0a0
unsigned KdTexture::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// EAX=this. native from ITexture(), refs=1, width/height=0. Base vptrs then KdTexture tables.
// FUNCTION: VC1 0x0045c0f0
KdTexture::KdTexture()
{
    width = 0;
    height = 0;
}

const char* KdTexture::Script14()
{
    return 0;
}

// Allocates and constructs a KdTexture, copies its locked pixels, returns IAbstract.
// STUB: VC1 0x0045c1a0
void** KdTexture::Clone(void** out)
{
    *out = 0;
    return out;
}

// Locks native storage and lazily creates it from region width/height.
// STUB: VC1 0x0045c290
void* KdTexture::Lock(void* region, int mode)
{
    (void)region;
    (void)mode;
    return 0;
}

// Script binding implementation is recovered separately; keep the correct slot.
// STUB: VC1 0x0045c3c0
unsigned char KdTexture::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Loads native texture data from a named resource and records dimensions.
// STUB: VC1 0x0045c570
unsigned char KdTexture::Load(const char* name)
{
    (void)name;
    return 0;
}

}
