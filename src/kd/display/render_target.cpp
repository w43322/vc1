#include "kd/display/render_target.h"

#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed704
static IUnknown* g_renderTargetDefault;

// MOV EAX, [ECX+0x18]; RET. IRenderTarget vtable+0x18.
// FUNCTION: VC1 0x0045bbe0
int KdRenderTarget::GetHeight()
{
    return static_cast<int>(height);
}

// MOV EAX, [ECX+0x14]; RET. IRenderTarget vtable+0x14.
// FUNCTION: VC1 0x0045bbf0
int KdRenderTarget::GetWidth()
{
    return static_cast<int>(width);
}

// this = IRenderTarget. RET 4.
// FUNCTION: VC1 0x0045bc90
void* KdRenderTarget::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidRenderTarget) {
        return this;
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 0x0c; decrement refs at IAbstract+4. If 0, vtbl+0x0c(this, 1).
// FUNCTION: VC1 0x0045bcc0
unsigned KdRenderTarget::Release()
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

// MOV EAX, [ECX+0x10]; TEST; INC; store. RET.
// FUNCTION: VC1 0x0045bce0
unsigned KdRenderTarget::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// EAX=malloc(0x20) block, stdcall extra = texture node. IRenderTarget() then extra/refs.
// FUNCTION: VC1 0x0045bf00
KdRenderTarget::KdRenderTarget(void* node)
{
    extra = node;
    cachedTexture = 0;
}

void* KdRenderTarget::Display1c()
{
    return 0;
}

// cdecl out. LTCG extras width/height in EBX/EDI. Lookup miss AddRefs g_renderTargetDefault.
// FUNCTION: VC1 0x0045bf40
void** CreateRenderTargetBody(void** out, unsigned width, unsigned height)
{
    void* node;
    KdRenderTarget* object;
    IUnknown* ident;
    IUnknown* iface;

    node = LookupRenderTargetTexture(width, height);
    if (!node) {
        ident = g_renderTargetDefault;
        *out = ident;
        if (ident) {
            ident->AddRef();
        }
        return out;
    }
    object = static_cast<KdRenderTarget*>(malloc(0x20));
    if (object) {
        new (object) KdRenderTarget(node);
    }
    if (!object) {
        *out = 0;
        return out;
    }
    object->width = width;
    object->height = height;
    iface = static_cast<IUnknown*>(static_cast<IAbstract*>(object));
    ident = static_cast<IUnknown*>(iface->QueryInterface(&g_iidAbstract));
    if (ident) {
        ident->AddRef();
    }
    ident->Release();
    *out = ident;
    ident->AddRef();
    ident->Release();
    return out;
}

}
