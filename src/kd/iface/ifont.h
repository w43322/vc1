#ifndef VC1_KD_IFONT_H
#define VC1_KD_IFONT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Inferred from KdSystemBase::Render (0x00463980), which sizes the console
// backdrop with these two slots. Slot 0x0c is not recovered.
// SIZE 0x04
struct IFont : IUnknown
{
    virtual void Font0c() = 0;                         // vtable+0x0c
    virtual int GetTextWidth(const char* text) = 0;    // vtable+0x10
    virtual int GetHeight() = 0;                       // vtable+0x14
};

typedef char Vc1IFontSizeCheck[sizeof(IFont) == 0x04 ? 1 : -1];

KdRef<IFont>* AssignFont(KdRef<IFont>* self, IUnknown* const* source);

}

#endif
