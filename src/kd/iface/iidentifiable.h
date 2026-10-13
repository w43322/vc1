#ifndef VC1_KD_IIDENTIFIABLE_H
#define VC1_KD_IIDENTIFIABLE_H

#include "kd/iface/iunknown.h"
#include "kd/factory.h"

namespace vc1 {

extern const char* g_emptyCountedBegin;
extern unsigned g_emptyCountedSize;

// Inferred from KdCompoundObject::QueryInterface and ctor vptr at +0x0c.
// The +4 payload is the object's name, assigned from the shared empty string.
// SIZE 0x14
// VTABLE: VC1 0x004d124c
struct IIdentifiable : IUnknown
{
    KdString name;

    IIdentifiable() : name(g_emptyCountedBegin, g_emptyCountedSize) {}
    virtual ~IIdentifiable() {}
    virtual const char* GetName() = 0;
    virtual void SetName(const char* value) = 0;
};

typedef char Vc1IIdentifiableSizeCheck[sizeof(IIdentifiable) == 0x14 ? 1 : -1];

}

#endif
