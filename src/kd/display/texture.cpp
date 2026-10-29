#include "kd/display/texture.h"
#include "kdcore/native.h"
#include "kd/display/display.h"

#include "bindings/sqplus.h"

#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// XOR AL,AL; RET 4. ITexture vtable+0x0c.
// FUNCTION: VC1 0x00418000 FOLDED
unsigned char KdTexture::Reload(void* arg)
{
    (void)arg;
    return 0;
}

// FUNCTION: VC1 0x00418000 FOLDED
unsigned char KdRenderTexture::Reload(void* arg)
{
    (void)arg;
    return 0;
}

// ESI=this, EAX=D3D texture (borrowed, not AddRef'd). Width/height from
// GetLevelDesc(0) (texture vtable+0x38), 0 when it fails.
// FUNCTION: VC1 0x0041d4e0
KdRenderTexture::KdRenderTexture(IDirect3DTexture8* texture)
{
    D3dSurfaceDesc desc;

    native = texture;
    width = native->GetLevelDesc(0, &desc) < 0 ? 0 : desc.width;
    readback = 0;
    height = native->GetLevelDesc(0, &desc) < 0 ? 0 : desc.height;
}

// FUNCTION: VC1 0x0041d560
int KdRenderTexture::GetWidth()
{
    return static_cast<int>(width);
}

// FUNCTION: VC1 0x0041d570
int KdRenderTexture::GetHeight()
{
    return static_cast<int>(height);
}

// Drops the readback surface of Lock (vtable+0x28 unlock, then Release).
// FUNCTION: VC1 0x0045bc00
void KdRenderTexture::Unlock()
{

    if (readback) {
        readback->UnlockRect();
        readback->Release();
        readback = 0;
    }
}

// this = ITexture. ITexture or IAbstract at +8. RET 4.
// FUNCTION: VC1 0x0045bc30
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
// FUNCTION: VC1 0x0045bc60
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
// FUNCTION: VC1 0x0045bc80
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

// STRING: VC1 0x004d68a4
static const char kAlreadyLocked[] = "already locked";

// RET 8. region is {x, y, width, height}. Copies that part of the render
// target into a system-memory surface in the display format
// (CreateImageSurface, CopyRects) and locks it; only one lock at a time.
// FUNCTION: VC1 0x0045be10
void* KdRenderTexture::Lock(void* region, bool readOnly)
{
    const int* const rect = static_cast<const int*>(region);
    IDirect3DSurface8* surface;
    RECT source;
    POINT origin;
    D3dLockedRect locked;

    if (readback) {
        HostLogError(&g_kdNameRenderTarget, kAlreadyLocked);
        return 0;
    }
    surface = 0;
    native->GetSurfaceLevel(0, &surface);
    g_d3dDevice->CreateImageSurface(rect[2], rect[3], g_displayMode[2], &readback);
    source.left = rect[0];
    source.top = rect[1];
    source.right = rect[0] + rect[2];
    source.bottom = rect[1] + rect[3];
    origin.x = 0;
    origin.y = 0;
    g_d3dDevice->CopyRects(surface, &source, 1, readback, &origin);
    surface->Release();
    locked.pitch = 0;
    locked.bits = 0;
    readback->LockRect(&locked, 0, (readOnly ? 0x10 : 0) | 0x8800);
    return locked.bits;
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
unsigned char KdTexture::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// IDirect3DTexture8::UnlockRect (vtable+0x44) of level 0. this=ITexture.
// FUNCTION: VC1 0x0045c010
void KdTexture::Unlock()
{
    native->UnlockRect(0);
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

// ADD ECX, 0x14, then IAbstract::Release (0x00401ee0) inlined by LTCG: the deleting
// destructor (vtable+0x0c) on zero refs.
// FUNCTION: VC1 0x0045c080
unsigned KdTexture::Release()
{
    return IAbstract::Release();
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

const char* KdTexture::GetTypeName()
{
    return 0;
}

// RET 4. this = ICloneable. A new KdTexture whose first writable lock
// creates its native texture at this texture's size; the pixels are copied
// from a read-only lock of this one. Returns the copy's IAbstract.
// FUNCTION: VC1 0x0045c1a0
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
// FUNCTION: VC1 0x0045c290
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
// FUNCTION: VC1 0x0045c3c0
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
// FUNCTION: VC1 0x0045c570
unsigned char KdTexture::Load(const char* name)
{
    D3dSurfaceDesc desc;

    if (native) {
        ReleaseTrackedTexture(native);
    }
    native = LoadTextureFile(name);
    if (!native) {
        return 0;
    }
    width = native->GetLevelDesc(0, &desc) < 0 ? 0 : desc.width;
    height = native->GetLevelDesc(0, &desc) < 0 ? 0 : desc.height;
    // STRING: VC1 0x004d6804
    HostLog(&g_kdNameTexture, "Loaded %s (size: %ix%i)", name, width, height);
    return 1;
}

}
