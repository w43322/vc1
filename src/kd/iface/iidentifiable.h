#ifndef VC1_KD_IIDENTIFIABLE_H
#define VC1_KD_IIDENTIFIABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// The +4 payload is a polymorphic 0x10-byte counted-name object.
// SIZE 0x10
// VTABLE: VC1 0x004d0d9c
class IdentifiableName
{
public:
    IdentifiableName();
    virtual ~IdentifiableName() {}

    const void* begin;
    unsigned size;
    unsigned capacity;
};

typedef char Vc1IdentifiableNameSizeCheck[sizeof(IdentifiableName) == 0x10 ? 1 : -1];

// Inferred from KdCompoundObject::QueryInterface and ctor vptr at +0x0c.
// SIZE 0x14
// VTABLE: VC1 0x004d124c
struct IIdentifiable : IUnknown
{
    IdentifiableName name;

    virtual ~IIdentifiable() {}
    virtual const char* GetName() = 0;
    virtual void SetName(const char* value) = 0;
};

typedef char Vc1IIdentifiableSizeCheck[sizeof(IIdentifiable) == 0x14 ? 1 : -1];

}

#endif
