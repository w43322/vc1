#ifndef VC1_KD_ICAMERA_H
#define VC1_KD_ICAMERA_H

#include "kd/iface/iunknown.h"

namespace vc1 {

struct Matrix3;

// Slots from KdObjectWatcher::Update (0x004adb90): it moves the camera with
// +0x0c/+0x10 like IPositionable and maps a point through the +0x18 matrix.
// SIZE 0x04
struct ICamera : IUnknown
{
    virtual void SetPosition(const Matrix3& value) = 0; // vtable+0x0c
    virtual const Matrix3& GetPosition() = 0;           // vtable+0x10
    // KdGraphics::SetView passes (&graphics IAbstract, &view rect).
    virtual void BindView() = 0;                        // vtable+0x14
    virtual const Matrix3& GetViewMatrix() = 0;         // vtable+0x18
};

typedef char Vc1ICameraSizeCheck[sizeof(ICamera) == 0x04 ? 1 : -1];

}

#endif
