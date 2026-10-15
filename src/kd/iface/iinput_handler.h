#ifndef VC1_KD_IINPUT_HANDLER_H
#define VC1_KD_IINPUT_HANDLER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// The system's input group face. The KD input callbacks (0x00462110 for event
// 0x38, 0x00461f00 / 0x004a8c40 for event 0x39) pass the event data union
// (KdEvent+0x10). Slot names are inferred.
// SIZE 0x04
struct IInputHandler : IUnknown
{
    // Non-zero when the event was consumed (the input group stops there).
    virtual unsigned char OnKey(const unsigned* data) = 0;     // vtable+0x0c
    virtual unsigned char OnPointer(const unsigned* data) = 0; // vtable+0x10
};

typedef char Vc1IInputHandlerSizeCheck[sizeof(IInputHandler) == 0x04 ? 1 : -1];

}

#endif
