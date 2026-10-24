#ifndef VC1_KD_IIDENTIFIABLE_H
#define VC1_KD_IIDENTIFIABLE_H

#include "kd/iface/iunknown.h"
#include "kd/factory.h"

namespace vc1 {

// The shared empty name every IIdentifiable starts from (object_watcher.cpp).
extern KdString g_kdEmptyIdentifiableName;

// The +4 payload is the object's name, assigned from the shared empty string.
// Default vftable: GetName (0x00401000, shared with other "return this+4"
// bodies), SetName (0x00403ab0), then the deleting destructor. Slot names
// from getIdentificator / setIdentificator (0x004b59f0, 0x004b5970).
// SIZE 0x14
// VTABLE: VC1 0x004d124c
struct IIdentifiable : IUnknown
{
    KdString name;

    IIdentifiable()
        : name(g_kdEmptyIdentifiableName.c_str(), g_kdEmptyIdentifiableName.size) {}
    virtual const KdString& GetName() { return name; }                // vtable+0x0c
    virtual void SetName(const KdString& value) { name.Assign(value); } // vtable+0x10
    virtual ~IIdentifiable() {}                                       // vtable+0x14
};

typedef char Vc1IIdentifiableSizeCheck[sizeof(IIdentifiable) == 0x14 ? 1 : -1];

// SYNTHETIC: VC1 0x004053b0
// vc1::IIdentifiable::`scalar deleting destructor'

}

#endif
