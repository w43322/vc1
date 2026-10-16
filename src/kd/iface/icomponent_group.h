#ifndef VC1_KD_ICOMPONENT_GROUP_H
#define VC1_KD_ICOMPONENT_GROUP_H

#include "kd/iface/ishutdownable.h"

namespace vc1 {

class KdFactory;
class KdString;
struct IAbstract;

// Slots 0x10..0x18 are the script methods "RegisterObject",
// "UnregisterObject" and "GetObject" (GetMember tables 0x00463470 and
// 0x00473a90 bind them through vcall thunks); FindObject is renamed to stay
// clear of the Win32 GetObject macro.
// SIZE 0x04
struct IComponentGroup : IShutdownable
{
    virtual void RegisterObject(const KdString& name, const KdPtr<IAbstract>& object) = 0; // vtable+0x10
    virtual KdPtr<IUnknown> UnregisterObject(const KdString& name) = 0; // vtable+0x14
    virtual KdPtr<IUnknown> FindObject(const KdString& name) = 0;       // vtable+0x18
    virtual KdFactory* CollectObjects(const char** iid) = 0;  // vtable+0x1c
    virtual void* GetRecords() = 0;                      // vtable+0x20
    virtual int ClearCollected() = 0;                        // vtable+0x24
};

typedef char Vc1IComponentGroupSizeCheck[sizeof(IComponentGroup) == 0x04 ? 1 : -1];

}

#endif
