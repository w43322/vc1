#ifndef VC1_KD_RENDER_TARGET_H
#define VC1_KD_RENDER_TARGET_H

#include "kd/iface/irender_target.h"

namespace vc1 {

// Inferred name. Factory string "RenderTarget" (0x004d4c28), created by
// IDisplay::CreateRenderTarget. IAbstract vptr is stored in IRenderTarget::abstract.
// VTABLE: VC1 0x004d68b4 vc1::IRenderTarget
// SIZE 0x20
class KdRenderTarget : public IRenderTarget
{
public:
    unsigned refs;            // 0x10
    unsigned width;          // 0x14
    unsigned height;         // 0x18
    IUnknown* cachedTexture; // 0x1c

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int GetWidth();
    int GetHeight();
};

typedef char Vc1KdRenderTargetSizeCheck[sizeof(KdRenderTarget) == 0x20 ? 1 : -1];

void RenderTargetInit(IRenderTarget* self);
void* LookupRenderTargetTexture(unsigned width, unsigned height);
KdRenderTarget* ConstructKdRenderTarget(KdRenderTarget* self, void* node);
void** CreateRenderTargetBody(void** out, unsigned width, unsigned height);

}

#endif
