#ifndef VC1_KD_ITILE_OBJECT_H
#define VC1_KD_ITILE_OBJECT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Inferred names. Only vtable+0x14 is read so far: GameLevel's Add/
// RemoveObject (0x004b0f30 / 0x004b0e30) look its result up in the
// SetDynamicLayer table, so it is the tile object's layer (guess). The two
// slots before it are not analyzed yet.
// SIZE 0x04
struct ITileObject : IUnknown
{
    virtual void UnanalyzedSlot0c() = 0; // vtable+0x0c
    virtual void UnanalyzedSlot10() = 0; // vtable+0x10
    virtual int GetLayer() = 0;          // vtable+0x14
};

typedef char Vc1ITileObjectSizeCheck[sizeof(ITileObject) == 0x04 ? 1 : -1];

}

#endif
