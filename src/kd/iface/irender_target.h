#ifndef VC1_KD_IRENDER_TARGET_H
#define VC1_KD_IRENDER_TARGET_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x10. GetGraphics..GetFont share bodies with IDisplay; IDisplay vtable
// uses SUB ECX, 0x10 thunks to this this-pointer (IRenderTarget at KdDisplay+0x04).
// Default vftable written by RenderTargetInit; KdDisplay/KdRenderTarget replace it.
// VTABLE: VC1 0x004d5de0
struct IRenderTarget : IUnknown
{
    void* extra;         // 0x04  texture node on KdRenderTarget
    IUnknown* graphics;   // 0x08
    void* abstract;       // 0x0c  IAbstract vptr on KdRenderTarget

    virtual void* Display0c(void* a, void* b) = 0; // vtable+0x0c
    virtual void* Display10() = 0;                 // vtable+0x10
    virtual int GetWidth() = 0;                    // vtable+0x14
    virtual int GetHeight() = 0;                   // vtable+0x18
    virtual void* Display1c() = 0;                // vtable+0x1c
};

typedef char Vc1IRenderTargetSizeCheck[sizeof(IRenderTarget) == 0x10 ? 1 : -1];

}

#endif
