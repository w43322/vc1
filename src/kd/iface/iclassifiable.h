#ifndef VC1_KD_ICLASSIFIABLE_H
#define VC1_KD_ICLASSIFIABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Classification bit mask at +4. The default vftable implements every slot:
// add (0x004060c0), remove (0x004060d0), get (0x0040f130, shared with other
// "return [this+4]" bodies) and the deleting destructor (0x004060e0). Slot
// names from the script natives addClassificator / removeClassificator /
// getClassificators / checkMask.
// SIZE 0x08
// VTABLE: VC1 0x004d15d4
struct IClassifiable : IUnknown
{
    unsigned value;

    IClassifiable() : value(0) {}

    virtual void AddClassificator(unsigned mask) { value |= mask; }     // vtable+0x0c
    virtual void RemoveClassificator(unsigned mask) { value &= ~mask; } // vtable+0x10
    virtual unsigned GetClassificators() { return value; }            // vtable+0x14
    virtual ~IClassifiable() {}                                       // vtable+0x18
};

typedef char Vc1IClassifiableSizeCheck[sizeof(IClassifiable) == 0x08 ? 1 : -1];

// SYNTHETIC: VC1 0x004060e0
// vc1::IClassifiable::`scalar deleting destructor'

}

#endif
