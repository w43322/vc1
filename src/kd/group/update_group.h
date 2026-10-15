#ifndef VC1_KD_GROUP_UPDATE_GROUP_H
#define VC1_KD_GROUP_UPDATE_GROUP_H

#include "kd/group/ptr_set.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iupdateable.h"

namespace vc1 {

// Inferred name. Factory "UpdateGroup": runs IUpdateable::Update on its
// members once per slice they ask for (GetLayers bits 1..0x80). Adds and
// removes are deferred to the next Update/SetLayer; new members then get
// SetLayer(layer).
// SIZE 0x7c
// VTABLE: VC1 0x004d5378 vc1::IUpdateable
// VTABLE: VC1 0x004d5390 vc1::IFunctionalGroup
// VTABLE: VC1 0x004d53a8 vc1::IIdentifiable
// VTABLE: VC1 0x004d53c0 vc1::IAbstract
class KdUpdateGroup : public IUpdateable,
                      public IFunctionalGroup,
                      public IIdentifiable,
                      public IAbstract
{
public:
    unsigned layer;     // 0x24 never written by the group
    int frames;         // 0x28 Update calls with slice 1
    KdSliceMap slices;  // 0x2c
    KdRefList members;  // 0x3c sorted
    KdRefList fresh;    // 0x4c joined since the last flush
    KdRefList toAdd;    // 0x5c sorted
    KdRefList toRemove; // 0x6c sorted

    KdUpdateGroup() throw();
    ~KdUpdateGroup() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    int Unmapped0c();
    int SetLayer(unsigned value);
    void Update(unsigned slice, int dt);
    unsigned char Add(void** object);
    void Remove(void** object);
    void Clear();

    void FlushFresh();
    void RemoveMember(IUnknown* const* object);
    void AddMember(IUnknown* const* object);
};

typedef char Vc1KdUpdateGroupSizeCheck[sizeof(KdUpdateGroup) == 0x7c ? 1 : -1];

void** CreateKdUpdateGroup(void** out);

}

#endif
