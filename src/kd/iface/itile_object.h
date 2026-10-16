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
// the object's depth (screen y). Slots +0x18..+0x2c and +0x34 are the
// script methods of KdTileObject (GetMember 0x004a1410 binds them through
// vcall thunks).
// SIZE 0x04
struct ITileObject : IRenderable
{
    virtual int GetTile() = 0;                     // vtable+0x18 "GetTile"
    virtual void SetTile(int tile) = 0;            // vtable+0x1c "SetTile"
    virtual void SetRenderLayer(int layer) = 0;    // vtable+0x20 "SetRenderLayer"
    virtual void SetColor(int color) = 0;          // vtable+0x24 "SetColor"
    virtual void SetHighlighted(bool value) = 0;   // vtable+0x28 "SetHighlighted"
    virtual void EnableFloatCoords(bool value) = 0; // vtable+0x2c "EnableFloatCoords"
    virtual float GetDepth(IUnknown** camera) = 0; // vtable+0x30
    virtual KdPtr<IUnknown> GetJoint(int joint) = 0; // vtable+0x34 "GetJoint"
};

typedef char Vc1ITileObjectSizeCheck[sizeof(ITileObject) == 0x04 ? 1 : -1];

}

#endif
