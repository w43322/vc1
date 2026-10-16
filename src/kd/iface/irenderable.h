#ifndef VC1_KD_IRENDERABLE_H
#define VC1_KD_IRENDERABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Inferred name. 64-bit render flags (the script FLAG_* bits); renderers
// receive it by pointer as the second Render0c argument.
// SIZE 0x08
struct RenderMask
{
    unsigned lo; // 0x00
    unsigned hi; // 0x04
};

// Never written; the mask passed when no renderer-specific one applies.
extern RenderMask g_defaultRenderMask;

// SIZE 0x04
struct IRenderable : IUnknown
{
    virtual int Render0c(void* graphics, void* unused); // vtable+0x0c
    virtual int Render(void* graphics) = 0;            // vtable+0x10
    virtual int Render14() = 0;                        // vtable+0x14
};

typedef char Vc1IRenderableSizeCheck[sizeof(IRenderable) == 0x04 ? 1 : -1];

}

#endif
