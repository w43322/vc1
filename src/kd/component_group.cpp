#include "kd/component_group.h"

#include "kd/iface/iabstract.h"
#include "kd/iface/iid.h"
#include "kd/iface/ishutdownable.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed6c0
void* g_groupNotify;

// Inlined into ~KdSystemBase (0x00462900): extra then records.
KdComponentGroup::~KdComponentGroup()
{
}

// LEA EAX, [ECX+4]; RET.
// FUNCTION: VC1 0x00472790
void* KdComponentGroup::GetRecords()
{
    return &records.items;
}

// ClearCollected (virtual), then every record: notify IGroupMember with
// g_groupNotify and Shutdown its IShutdownable face; empties the table.
// FUNCTION: VC1 0x004727a0
int KdComponentGroup::Shutdown()
{
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IUnknown* member;
    IShutdownable* face;
    typedef void (__thiscall *FnSetGroup)(IUnknown* object, void* group);

    ClearCollected();

    it = reinterpret_cast<unsigned char*>(records.items.begin);
    end = it + records.items.size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        member = 0;
        if (object) {
            member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
            if (member) {
                member->AddRef();
                reinterpret_cast<FnSetGroup>((*reinterpret_cast<void***>(member))[4])(
                    member, &g_groupNotify);
            }
        }
        if (object) {
            face = static_cast<IShutdownable*>(object->QueryInterface(&g_iidShutdownable));
            if (face) {
                face->AddRef();
                face->Shutdown();
                face->Release();
            }
        }
        if (member) {
            member->Release();
        }
    }
    if (records.items.capacity < records.items.Count()) {
        GrowCountedBuffer(&records.items, 0, 1);
    }
    records.items.size = 0;
    return 0;
}

// Every record's IComponentGroup gets ClearCollected; then empties extra.
// FUNCTION: VC1 0x00472870
int KdComponentGroup::ClearCollected()
{
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IComponentGroup* group;

    it = reinterpret_cast<unsigned char*>(records.items.begin);
    end = it + records.items.size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (!object) {
            continue;
        }
        group = static_cast<IComponentGroup*>(object->QueryInterface(&g_iidComponentGroup));
        if (group) {
            group->AddRef();
            group->ClearCollected();
            group->Release();
        }
    }
    if (extra.items.capacity < extra.items.Count()) {
        GrowCountedBuffer(&extra.items, 0, 1);
    }
    extra.items.size = 0;
    return 0;
}

// RET 4. Collects into extra every record (and nested group member) that
// answers iid; returns &extra.items (the KdFactory).
// FUNCTION: VC1 0x004728f0
KdFactory* KdComponentGroup::CollectObjects(const char** iid)
{
    unsigned char* it;
    IUnknown* object;
    IComponentGroup* group;
    KdFactory* nested;
    const char* name;
    void* appended;
    IUnknown** slot;
    unsigned needed;
    unsigned oldCount;
    IUnknown** fill;
    IUnknown** fillEnd;

    if (extra.items.capacity < extra.items.Count()) {
        GrowCountedBuffer(&extra.items, 0, 1);
    }
    extra.items.size = 0;

    for (it = reinterpret_cast<unsigned char*>(records.items.begin);
        it != reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size;
        it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        group = object
            ? static_cast<IComponentGroup*>(object->QueryInterface(&g_iidComponentGroup))
            : 0;
        if (group) {
            group->AddRef();
            nested = group->CollectObjects(iid);
            AppendCountedRange(&appended, nested->begin,
                reinterpret_cast<unsigned char*>(nested->begin) + nested->size, &extra.items);
        }
        name = *iid;
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (object && object->QueryInterface(&name)) {
            needed = ((extra.items.size >> 2) + 1) * 4;
            if (extra.items.capacity < needed) {
                oldCount = extra.items.capacity >> 2;
                GrowCountedBuffer(&extra.items, static_cast<int>(needed), 0);
                if ((extra.items.capacity >> 2) > oldCount) {
                    fill = reinterpret_cast<IUnknown**>(extra.items.begin) + oldCount;
                    fillEnd = reinterpret_cast<IUnknown**>(extra.items.begin) + (extra.items.capacity >> 2);
                    for (; fill < fillEnd; fill++) {
                        if (fill) {
                            *fill = 0;
                        }
                    }
                }
            }
            extra.items.size = needed;
            slot = reinterpret_cast<IUnknown**>(
                reinterpret_cast<unsigned char*>(extra.items.begin) + needed - 4);
            object = *reinterpret_cast<IUnknown**>(it + 0x10);
            if (object) {
                object->AddRef();
            }
            if (*slot) {
                (*slot)->Release();
            }
            *slot = *reinterpret_cast<IUnknown**>(it + 0x10);
        }
        if (group) {
            group->Release();
        }
    }
    return &extra.items;
}

// RET 8. Lookup key; on hit return the record's IUnknown*. On a miss ask each
// nested IComponentGroup (vtable+0x18) and keep the first result whose
// IAbstract differs from g_groupNotify.
// FUNCTION: VC1 0x00472a70
KdPtr<IUnknown> KdComponentGroup::FindObject(const KdString& name)
{
    void* key = const_cast<KdString*>(&name);
    unsigned char* pos;
    unsigned char* end;
    unsigned char* it;
    IUnknown* object;
    IComponentGroup* group;
    IUnknown* found;
    void* absFound;
    void* absNotify;
    unsigned char take;

    pos = LookupCountedRecord(&records.items, key);
    end = reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size;
    if (pos < end && RecordEq(key, pos)) {
        pos = end;
    }
    if (pos != end) {
        return KdPtr<IUnknown>(*reinterpret_cast<IUnknown**>(pos + 0x10));
    }

    it = reinterpret_cast<unsigned char*>(records.items.begin);
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (!object) {
            continue;
        }
        group = static_cast<IComponentGroup*>(object->QueryInterface(&g_iidComponentGroup));
        if (!group) {
            continue;
        }
        group->AddRef();
        KdPtr<IUnknown> nested = group->FindObject(name);
        found = nested.ptr;
        take = 0;
        if (found) {
            if (!g_groupNotify) {
                take = 1;
            }
            else {
                absFound = found->QueryInterface(&g_iidAbstract);
                absNotify = static_cast<IUnknown*>(g_groupNotify)->QueryInterface(
                    &g_iidAbstract);
                if (absFound != absNotify) {
                    take = 1;
                }
                else {
                    found = 0;
                }
            }
        }
        else if (g_groupNotify) {
            take = 1;
        }
        if (take) {
            group->Release();
            return nested;
        }
        group->Release();
    }
    return KdPtr<IUnknown>(static_cast<IUnknown*>(g_groupNotify));
}

// RET 8. Removes key from the table; the object (IGroupMember notified with
// g_groupNotify) is returned AddRef'd.
// FUNCTION: VC1 0x00472bf0
KdPtr<IUnknown> KdComponentGroup::UnregisterObject(const KdString& name)
{
    void* key = const_cast<KdString*>(&name);
    KdPtr<IUnknown> result;
    unsigned char* pos;
    unsigned char* end;
    IUnknown* object;
    IUnknown* member;
    typedef void (__thiscall *FnSetGroup)(IUnknown* object, void* group);

    pos = LookupCountedRecord(&records.items, key);
    end = reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size;
    if (pos < end && RecordEq(key, pos)) {
        pos = end;
    }
    if (pos != end) {
        object = *reinterpret_cast<IUnknown**>(pos + 0x10);
        member = 0;
        if (object) {
            member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
            if (member) {
                member->AddRef();
                reinterpret_cast<FnSetGroup>((*reinterpret_cast<void***>(member))[4])(
                    member, &g_groupNotify);
            }
            object->AddRef();
        }
        if (result.ptr) {
            result.ptr->Release();
        }
        result.ptr = object;
        EraseCountedBytes(pos, pos + 0x14,
            reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size);
        records.items.size -= 0x14;
        if (member) {
            member->Release();
        }
    }
    return result;
}

// RET 8. Insert-or-replace key -> *out; an IGroupMember is told its new group.
// FUNCTION: VC1 0x00472db0
void KdComponentGroup::RegisterObject(const KdString& name, const KdPtr<IAbstract>& value)
{
    void* key = const_cast<KdString*>(&name);
    IUnknown** slot;
    IUnknown* object;
    IUnknown* member;
    void* ident;
    typedef void (__thiscall *FnSetGroup)(IUnknown* object, void* group);

    slot = InsertCountedSlot(key, &records.items);
    object = value.ptr;
    if (object) {
        object->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = object;
    if (!object) {
        return;
    }
    member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
    if (!member) {
        return;
    }
    member->AddRef();
    ident = QueryInterface(&g_iidAbstract);
    if (ident) {
        ident = static_cast<IUnknown*>(ident)->QueryInterface(&g_iidAbstract);
        if (ident) {
            static_cast<IUnknown*>(ident)->AddRef();
        }
    }
    reinterpret_cast<FnSetGroup>((*reinterpret_cast<void***>(member))[4])(member, &ident);
    if (ident) {
        static_cast<IUnknown*>(ident)->Release();
    }
    member->Release();
}

}
