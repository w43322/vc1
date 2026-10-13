#include "kd/component_group.h"

#include "kd/iface/iid.h"
#include "kd/iface/ishutdownable.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed6c0
void* g_groupNotify;

// Inlined into ~KdSystemBase (0x00462900): extra then records.
KdComponentGroup::~KdComponentGroup()
{
    FactoryReleasePointers(&extra);
    FactoryDtorTail(&records);
}

// LEA EAX, [ECX+4]; RET.
// FUNCTION: VC1 0x00472790
void* KdComponentGroup::Unmapped20()
{
    return &records;
}

// Unmapped24 (virtual), then every record: notify IGroupMember with
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

    Unmapped24();

    it = reinterpret_cast<unsigned char*>(records.begin);
    end = it + records.size;
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
    if (records.capacity < records.Count()) {
        GrowCountedBuffer(&records, 0, 1);
    }
    records.size = 0;
    return 0;
}

// Every record's IComponentGroup gets Unmapped24; then empties extra.
// FUNCTION: VC1 0x00472870
int KdComponentGroup::Unmapped24()
{
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IComponentGroup* group;

    it = reinterpret_cast<unsigned char*>(records.begin);
    end = it + records.size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (!object) {
            continue;
        }
        group = static_cast<IComponentGroup*>(object->QueryInterface(&g_iidComponentGroup));
        if (group) {
            group->AddRef();
            group->Unmapped24();
            group->Release();
        }
    }
    if (extra.capacity < extra.Count()) {
        GrowCountedBuffer(&extra, 0, 1);
    }
    extra.size = 0;
    return 0;
}

// RET 4. Collects into extra every record (and nested group member) that
// answers iid; returns &extra.
// FUNCTION: VC1 0x004728f0
KdFactory* KdComponentGroup::Unmapped1c(const char** iid)
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

    if (extra.capacity < extra.Count()) {
        GrowCountedBuffer(&extra, 0, 1);
    }
    extra.size = 0;

    for (it = reinterpret_cast<unsigned char*>(records.begin);
        it != reinterpret_cast<unsigned char*>(records.begin) + records.size;
        it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        group = object
            ? static_cast<IComponentGroup*>(object->QueryInterface(&g_iidComponentGroup))
            : 0;
        if (group) {
            group->AddRef();
            nested = group->Unmapped1c(iid);
            AppendCountedRange(&appended, nested->begin,
                reinterpret_cast<unsigned char*>(nested->begin) + nested->size, &extra);
        }
        name = *iid;
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (object && object->QueryInterface(&name)) {
            needed = ((extra.size >> 2) + 1) * 4;
            if (extra.capacity < needed) {
                oldCount = extra.capacity >> 2;
                GrowCountedBuffer(&extra, static_cast<int>(needed), 0);
                if ((extra.capacity >> 2) > oldCount) {
                    fill = reinterpret_cast<IUnknown**>(extra.begin) + oldCount;
                    fillEnd = reinterpret_cast<IUnknown**>(extra.begin) + (extra.capacity >> 2);
                    for (; fill < fillEnd; fill++) {
                        if (fill) {
                            *fill = 0;
                        }
                    }
                }
            }
            extra.size = needed;
            slot = reinterpret_cast<IUnknown**>(
                reinterpret_cast<unsigned char*>(extra.begin) + needed - 4);
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
    return &extra;
}

// RET 8. Lookup key; on hit return the record's IUnknown*. On a miss ask each
// nested IComponentGroup (vtable+0x18) and keep the first result whose
// IAbstract differs from g_groupNotify.
// FUNCTION: VC1 0x00472a70
void** KdComponentGroup::Unmapped18(void** out, void* key)
{
    unsigned char* pos;
    unsigned char* end;
    unsigned char* it;
    IUnknown* object;
    IComponentGroup* group;
    IUnknown* found;
    void* absFound;
    void* absNotify;
    unsigned char take;

    pos = LookupCountedRecord(&records, key);
    end = reinterpret_cast<unsigned char*>(records.begin) + records.size;
    if (pos < end && RecordEq(key, pos)) {
        pos = end;
    }
    if (pos != end) {
        object = *reinterpret_cast<IUnknown**>(pos + 0x10);
        *out = object;
        if (object) {
            object->AddRef();
        }
        return out;
    }

    it = reinterpret_cast<unsigned char*>(records.begin);
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
        found = 0;
        group->Unmapped18(reinterpret_cast<void**>(&found), key);
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
                    found->Release();
                    found = 0;
                }
            }
        }
        else if (g_groupNotify) {
            take = 1;
        }
        if (take) {
            *out = found;
            if (found) {
                found->AddRef();
                found->Release();
            }
            group->Release();
            return out;
        }
        group->Release();
    }
    object = static_cast<IUnknown*>(g_groupNotify);
    *out = object;
    if (object) {
        object->AddRef();
    }
    return out;
}

// RET 8. Removes key from the table; the object (IGroupMember notified with
// g_groupNotify) is returned AddRef'd.
// FUNCTION: VC1 0x00472bf0
void** KdComponentGroup::Unmapped14(void** out, void* key)
{
    unsigned char* pos;
    unsigned char* end;
    IUnknown* object;
    IUnknown* member;
    typedef void (__thiscall *FnSetGroup)(IUnknown* object, void* group);

    pos = LookupCountedRecord(&records, key);
    end = reinterpret_cast<unsigned char*>(records.begin) + records.size;
    if (pos < end && RecordEq(key, pos)) {
        pos = end;
    }
    *out = 0;
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
        if (*out) {
            static_cast<IUnknown*>(*out)->Release();
        }
        *out = object;
        EraseCountedBytes(pos, pos + 0x14,
            reinterpret_cast<unsigned char*>(records.begin) + records.size);
        records.size -= 0x14;
        if (member) {
            member->Release();
        }
    }
    return out;
}

// RET 8. Insert-or-replace key -> *out; an IGroupMember is told its new group.
// FUNCTION: VC1 0x00472db0
int KdComponentGroup::Unmapped10(void* key, void** out)
{
    IUnknown** slot;
    IUnknown* object;
    IUnknown* member;
    void* ident;
    typedef void (__thiscall *FnSetGroup)(IUnknown* object, void* group);

    slot = InsertCountedSlot(key, &records);
    object = out ? static_cast<IUnknown*>(*out) : 0;
    if (object) {
        object->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = object;
    if (!object) {
        return 0;
    }
    member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
    if (!member) {
        return 0;
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
    return 0;
}

}
