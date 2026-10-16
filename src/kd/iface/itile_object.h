#ifndef VC1_KD_ITILE_OBJECT_H
#define VC1_KD_ITILE_OBJECT_H

#include "kd/iface/irenderable.h"

namespace vc1 {

// Inferred names. An IRenderable that lives in a TileLayer: TileLayer::Add
// (0x004a2bb0) keeps it when GetRenderOrder() equals the layer's own, and
// GameLevel's Add/RemoveObject (0x004b0f30 / 0x004b0e30) look the same value
// up in the SetDynamicLayer table. TileLayer::Render (0x004a3690) and its
// depth sort (0x00409ee0) call vtable+0x30 with the graphics camera and
// interleave the object before every tile whose y is larger, so +0x30 is
// the object's depth (screen y). Slots +0x18..+0x2c are not analyzed yet.
// SIZE 0x04
struct ITileObject : IRenderable
{
    virtual void UnanalyzedSlot18() = 0;           // vtable+0x18
    virtual void UnanalyzedSlot1c() = 0;           // vtable+0x1c
    virtual void UnanalyzedSlot20() = 0;           // vtable+0x20
    virtual void UnanalyzedSlot24() = 0;           // vtable+0x24
    virtual void UnanalyzedSlot28() = 0;           // vtable+0x28
    virtual void UnanalyzedSlot2c() = 0;           // vtable+0x2c
    virtual float GetDepth(IUnknown** camera) = 0; // vtable+0x30
};

typedef char Vc1ITileObjectSizeCheck[sizeof(ITileObject) == 0x04 ? 1 : -1];

}

#endif
