#ifndef VC1_KD_RENDER_TARGET_H
#define VC1_KD_RENDER_TARGET_H

#include "kd/iface/irender_target.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/itexture.h"

namespace vc1 {

// Inferred name. Factory string "RenderTarget" (0x004d4c28), created by
// IDisplay::CreateRenderTarget. IAbstract is a base at +0x0c (QI ADD 0xc).
// SIZE 0x20
// VTABLE: VC1 0x004d68b4 vc1::IRenderTarget
// VTABLE: VC1 0x004d68d4 vc1::IAbstract
class KdRenderTarget : public IRenderTarget,
                       public IAbstract
{
public:
    unsigned width;          // 0x14
    unsigned height;         // 0x18
    ITexture* cachedTexture; // 0x1c

    KdRenderTarget(void* node);
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int GetWidth();
    int GetHeight();
    void* GetRenderTexture();
};

// The D3D texture of a render-target node (LookupRenderTargetTexture).
struct KdRenderTargetNode
{
    unsigned width;      // 0x00
    unsigned height;     // 0x04
    IUnknown* texture;   // 0x08
    KdRenderTargetNode* next; // 0x0c
};

typedef char Vc1KdRenderTargetSizeCheck[sizeof(KdRenderTarget) == 0x20 ? 1 : -1];

void* LookupRenderTargetTexture(unsigned width, unsigned height);
void** CreateRenderTargetBody(void** out, unsigned width, unsigned height);

}

#endif
