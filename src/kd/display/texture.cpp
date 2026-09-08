#include "kd/display/texture.h"

#include "kd/iface/iid.h"

namespace vc1 {

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
    unsigned* slot = reinterpret_cast<unsigned*>(reinterpret_cast<char*>(self) + 0xc);
    unsigned value = *slot;
    if (value != 0) {
        ++value;
        *slot = value;
    }
    return value;
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

// EAX=malloc(0x24) block. Sets vptrs; returns this.
// FUNCTION: VC1 0x0045c0f0
KdTexture* ConstructKdTexture(KdTexture* self)
{
    unsigned* object = reinterpret_cast<unsigned*>(self);

    object[1] = 0x004d1d30;
    object[2] = 0x004d1e68;
    object[3] = 0x004d681c;
    object[4] = 0;
    object[5] = 0x004d1034;
    object[6] = 1;
    object[0] = 0x004d683c;
    object[1] = 0x004d684c;
    object[2] = 0x004d685c;
    object[3] = 0x004d6874;
    object[5] = 0x004d6894;
    object[7] = 0;
    object[8] = 0;
    return self;
}

}
