#ifndef VC1_KD_IRENDERABLE_H
#define VC1_KD_IRENDERABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

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
