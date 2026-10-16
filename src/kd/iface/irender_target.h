#ifndef VC1_KD_IRENDER_TARGET_H
#define VC1_KD_IRENDER_TARGET_H

#include "kd/iface/igraphics.h"

namespace vc1 {

// SIZE 0x0c. GetGraphics..GetFont share bodies with IDisplay; IDisplay vtable
// uses SUB ECX, 0x10 thunks to this this-pointer (IRenderTarget at KdDisplay+0x04).
// Default vftable written by IRenderTarget(); KdDisplay/KdRenderTarget replace it.
// IAbstract is a following base (KdRenderTarget+0x0c), not a member.
// VTABLE: VC1 0x004d5de0
struct IRenderTarget : IUnknown
{
    void* extra;         // 0x04  texture node on KdRenderTarget
    IGraphics* graphics; // 0x08  KdGraphics built by IRenderTarget()

    IRenderTarget();
    ~IRenderTarget()
    {
        if (graphics) {
            graphics->Release();
        }
        graphics = 0;
    }

    virtual void* BeginFrame(void* a, void* b);     // vtable+0x0c
    virtual void* EndFrame();                     // vtable+0x10
    virtual int GetWidth() = 0;                    // vtable+0x14
    virtual int GetHeight() = 0;                   // vtable+0x18
    virtual void* GetRenderTexture() = 0;                // vtable+0x1c
};

typedef char Vc1IRenderTargetSizeCheck[sizeof(IRenderTarget) == 0x0c ? 1 : -1];

}

#endif
