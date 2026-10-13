#ifndef VC1_KD_IFONT_H
#define VC1_KD_IFONT_H

#include "kd/iface/iunknown.h"
#include "kd/factory.h"

namespace vc1 {

// Slots from the KdFont vftable (0x004d5cd0). Names are inferred.
// SIZE 0x04
struct IFont : IUnknown
{
    virtual unsigned char HasGlyph(unsigned code) = 0;                      // vtable+0x0c
    virtual int GetTextWidth(const char* text) = 0;                         // vtable+0x10
    virtual int GetHeight() = 0;                                            // vtable+0x14
    virtual KdStringList WrapText(const char* text, int width) = 0;         // vtable+0x18
    virtual void DrawText(IUnknown* const* graphics, const char* text) = 0; // vtable+0x1c
};

typedef char Vc1IFontSizeCheck[sizeof(IFont) == 0x04 ? 1 : -1];

KdRef<IFont>* AssignFont(KdRef<IFont>* self, IUnknown* const* source);

}

#endif
