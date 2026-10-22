// SDL3 native texture storage; engine interface behavior follows kd/display/texture.cpp.
#include "kd/display/texture.h"
#include "platform/sdl3/render.h"
#include "kd/display/display.h"

#include "bindings/sqplus.h"

#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// XOR AL,AL; RET 4. ITexture vtable+0x0c.
unsigned char KdTexture::Reload(void* arg)
{
    (void)arg;
    return 0;
}

unsigned char KdRenderTexture::Reload(void* arg)
{
    (void)arg;
    return 0;
}

// ESI=this, EAX=D3D texture (borrowed, not AddRef'd). Width/height from
// GetLevelDesc(0) (texture vtable+0x38), 0 when it fails.
KdRenderTexture::KdRenderTexture(IUnknown* texture) { native = texture; width = SdlTextureWidth(texture); height = SdlTextureHeight(texture); readback = 0; }


int KdRenderTexture::GetWidth()
{
    return static_cast<int>(width);
}

int KdRenderTexture::GetHeight()
{
    return static_cast<int>(height);
}

// Drops the readback surface of Lock (vtable+0x28 unlock, then Release).
void KdRenderTexture::Unlock() { SdlUnlockTexture(native); readback = 0; }


// this = ITexture. ITexture or IAbstract at +8. RET 4.
void* KdRenderTexture::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidTexture) {
        return static_cast<ITexture*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 8; deleting destructor on zero refs.
unsigned KdRenderTexture::Release()
{
    IAbstract* face = static_cast<IAbstract*>(this);

    if (--face->refs == 0) {
        delete face;
        return 0;
    }
    return face->refs;
}

// refs at ITexture+0x0c. RET.
unsigned KdRenderTexture::AddRef()
{
    IAbstract* face = static_cast<IAbstract*>(this);
    unsigned value = face->refs;
    if (value != 0) {
        ++value;
        face->refs = value;
    }
    return value;
}

// D3DLOCKED_RECT.
struct D3dLockedRect
{
    int pitch;   // 0x00
    void* bits;  // 0x04
};

static const char kAlreadyLocked[] = "already locked";

// RET 8. region is {x, y, width, height}. Copies that part of the render
// target into a system-memory surface in the display format
// (CreateImageSurface, CopyRects) and locks it; only one lock at a time.
void* KdRenderTexture::Lock(void* region, bool readOnly) { const int* r = static_cast<const int*>(region); return LockTextureRect(native,r[0],r[1],readOnly,r[2],r[3]); }


// MOV EAX,[ECX+0x14]; RET. this=ITexture (KdTexture+0x0c).
int KdTexture::GetHeight()
{
    return static_cast<int>(height);
}

// MOV EAX,[ECX+0x10]; RET. this=ITexture (KdTexture+0x0c).
int KdTexture::GetWidth()
{
    return static_cast<int>(width);
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
unsigned char KdTexture::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Calls native texture vtable+0x44(native,0). this=ITexture.
void KdTexture::Unlock() { SdlUnlockTexture(native); }


// this = ILoadable. RET 4.
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
unsigned KdTexture::Release()
{
    IUnknown* face;
    unsigned result;
    typedef void (__thiscall *FnDeletingDestructor)(void* object, int arg);

    face = static_cast<IUnknown*>(static_cast<IAbstract*>(this));
    result = --refs;
    if (result == 0) {
        reinterpret_cast<FnDeletingDestructor>((*reinterpret_cast<void***>(face))[3])(face, 1);
        return 0;
    }
    return result;
}

// MOV EAX, [ECX+0x18]; TEST; INC; store. RET.
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
KdTexture::KdTexture()
{
    width = 0;
    height = 0;
}

const char* KdTexture::GetTypeName()
{
    return 0;
}

// RET 4. this = ICloneable. A new KdTexture whose first writable lock
// creates its native texture at this texture's size; the pixels are copied
// from a read-only lock of this one. Returns the copy's IAbstract.
void** KdTexture::Clone(void** out)
{
    void* const memory = malloc(sizeof(KdTexture));
    KdTexture* copy;
    IUnknown* face;
    IUnknown* queried;

    if (memory) {
        copy = new (memory) KdTexture();
    }
    else {
        copy = 0;
    }
    int source[4];
    source[2] = static_cast<int>(width);
    source[3] = static_cast<int>(height);
    source[0] = 0;
    source[1] = 0;
    const void* const pixels = Lock(source, true);
    int target[4];
    target[0] = 0;
    target[1] = 0;
    target[2] = static_cast<int>(width);
    target[3] = static_cast<int>(height);
    void* const destination = copy->Lock(target, false);
    memcpy(destination, pixels, height * width * 4);
    copy->Unlock();
    Unlock();
    face = &static_cast<IAbstract&>(*copy);
    if (!face) {
        queried = 0;
    }
    else {
        queried = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 8. this = ITexture. region is {x, y, width, height}. The first
// writable lock of an empty texture creates the native texture at the
// region's size (the old native is not released).
void* KdTexture::Lock(void* region, bool readOnly)
{
    const int* const rect = static_cast<const int*>(region);

    if (!readOnly && !width && !height) {
        width = rect[2];
        height = rect[3];
        native = CreateManagedTexture(width, height);
    }
    return LockTextureRect(native, rect[0], rect[1], readOnly, rect[2], rect[3]);
}

// RET 8. Script method lookup over GetWidth/GetHeight (table 0x004ee4bc,
// guard bit 0x004ee4cc, atexit destructor 0x004cb030).
unsigned char KdTexture::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptGetWidth] = CreateScriptMethod<KdTexture>(&KdTexture::GetWidth);
        methods[kScriptGetHeight] = CreateScriptMethod<KdTexture>(&KdTexture::GetHeight);
    }
    return methods.Find(name, out);
}

// RET 4. Replaces native with the file's texture; width/height come from
// GetLevelDesc(0) (texture vtable+0x38), so they are the padded sizes.
unsigned char KdTexture::Load(const char* name) { if(native) ReleaseTrackedTexture(native); native=LoadTextureFile(name); if(!native) return 0; width=SdlTextureWidth(native); height=SdlTextureHeight(native); HostLog(&g_kdNameTexture,"Loaded %s (size: %ix%i)",name,width,height); return 1; }


}
