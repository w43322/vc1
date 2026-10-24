#ifndef VC1_KD_IABSTRACT_H
#define VC1_KD_IABSTRACT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Default vftable written by IAbstract(); most-derived classes replace it.
// SIZE 0x08. Always a C++ base, never a nested member.
// VTABLE: VC1 0x004d1034
struct IAbstract : IUnknown
{
    unsigned refs; // 0x04

    IAbstract();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    virtual ~IAbstract();                   // vtable+0x0c
};

typedef char Vc1IAbstractSizeCheck[sizeof(IAbstract) == 0x08 ? 1 : -1];

// Inferred name. The object behind an IAbstract face as a counted IUnknown
// (the face QI'd for IAbstract twice).
KdPtr<IUnknown> SelfObject(IAbstract* face);

// SYNTHETIC: VC1 0x00404bf0
// vc1::IAbstract::`scalar deleting destructor'

}

#endif
