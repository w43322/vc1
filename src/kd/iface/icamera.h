#ifndef VC1_KD_ICAMERA_H
#define VC1_KD_ICAMERA_H

#include "kd/iface/ipositionable.h"

namespace vc1 {

struct IAbstract;
struct Rect;

// Matrix getter names from the Camera script table (KdCamera::GetMember
// 0x00470a50). KdCamera answers IPositionable with this face.
// SIZE 0x04
struct ICamera : IPositionable
{
    // KdGraphics::SetView passes (graphics IAbstract, view rect).
    virtual void BindView(const KdPtr<IAbstract>& graphics, const Rect& view) = 0; // vtable+0x14
    virtual const Matrix3& GetWorldToScreen() = 0; // vtable+0x18
    virtual const Matrix3& GetScreenToWorld() = 0; // vtable+0x1c
    virtual const Matrix3& GetWorldToUI() = 0;     // vtable+0x20
    virtual const Matrix3& GetUIToWorld() = 0;     // vtable+0x24
    virtual const Matrix3& GetScreenToUI() = 0;    // vtable+0x28
    // TileLayer::Render (0x004a3690) maps the graphics clip rect through it
    // to find the visible tiles, and GameLevel "GetObjectAt" (0x004b1680)
    // maps its script point through it.
    virtual const Matrix3& GetUIToScreen() = 0;    // vtable+0x2c
};

typedef char Vc1ICameraSizeCheck[sizeof(ICamera) == 0x04 ? 1 : -1];

}

#endif
