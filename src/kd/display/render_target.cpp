#include "kd/display/render_target.h"

#include "kd/iface/iid.h"

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
        return &abstract;
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

    face = reinterpret_cast<IUnknown*>(&abstract);
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

// EAX=malloc(0x20) block, stdcall extra = texture node. EDI=self for RenderTargetInit. RET 4.
// FUNCTION: VC1 0x0045bf00
KdRenderTarget* ConstructKdRenderTarget(KdRenderTarget* self, void* node)
{
    unsigned* object = reinterpret_cast<unsigned*>(self);

    RenderTargetInit(self);
    object[3] = 0x004d1034;
    object[4] = 1;
    object[0] = 0x004d68b4;
    object[3] = 0x004d68d4;
    object[7] = 0;
    object[1] = reinterpret_cast<unsigned>(node);
    return self;
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
        object = ConstructKdRenderTarget(object, node);
    }
    if (!object) {
        *out = 0;
        return out;
    }
    object->width = width;
    object->height = height;
    iface = reinterpret_cast<IUnknown*>(&object->abstract);
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
