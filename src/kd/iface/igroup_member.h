#ifndef VC1_KD_IGROUP_MEMBER_H
#define VC1_KD_IGROUP_MEMBER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

template <class T>
class KdPtr;

// IID "IGroupMember" (g_iidGroupMember 0x004d05d4). KdComponentGroup asks
// every object it registers, unregisters or shuts down for it and hands it
// the group (RegisterObject 0x00472db0, UnregisterObject 0x00472bf0, Shutdown
// 0x004727a0: push the slot, ECX=member, call [vtbl+0x10]). Nothing in the
// game answers it: the IID string 0x004d4d90 is referenced only by
// g_iidGroupMember, and no QueryInterface compares it, so these calls never
// run and slot 3 is never seen.
// SIZE 0x04
struct IGroupMember : IUnknown
{
    // Never called; its name and signature are unknown.
    virtual void Reserved() = 0; // vtable+0x0c
    // The group's IAbstract face, or null when leaving it. Guessed name.
    virtual void SetGroup(const KdPtr<IUnknown>& group) = 0; // vtable+0x10
};

typedef char Vc1IGroupMemberSizeCheck[sizeof(IGroupMember) == 0x04 ? 1 : -1];

}

#endif
