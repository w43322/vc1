#ifndef VC1_KD_IPOSITIONABLE_H
#define VC1_KD_IPOSITIONABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

struct Matrix3;

// Slot names from the script natives setPosition (0x004b56f0, vtable+0x0c)
// and getPosition (0x004b5770, vtable+0x10, Matrix3::Null on a failed cast).
// SIZE 0x04
// VTABLE: VC1 0x004d10cc
struct IPositionable : IUnknown
{
    virtual void SetPosition(const Matrix3& value) = 0; // vtable+0x0c
    virtual const Matrix3& GetPosition() = 0;           // vtable+0x10
};

typedef char Vc1IPositionableSizeCheck[sizeof(IPositionable) == 0x04 ? 1 : -1];

}

#endif
