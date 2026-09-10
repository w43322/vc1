#ifndef VC1_KD_ICLASSIFIABLE_H
#define VC1_KD_ICLASSIFIABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Inferred from KdCompoundObject::QueryInterface. Value at +4 makes SIZE 0x08.
// Default vftable 0x004d15d4 has four operations after IUnknown.
// SIZE 0x08
// VTABLE: VC1 0x004d15d4
struct IClassifiable : IUnknown
{
    unsigned value;

    IClassifiable() : value(0) {}

    virtual int Classify0c(void* arg) = 0;
    virtual int Classify10(void* arg) = 0;
    virtual int Classify14(void* arg) = 0;
    virtual int Classify18() = 0;
};

typedef char Vc1IClassifiableSizeCheck[sizeof(IClassifiable) == 0x08 ? 1 : -1];

}

#endif
