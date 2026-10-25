#ifndef VC1_KD_GROUP_PTR_SET_H
#define VC1_KD_GROUP_PTR_SET_H

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdUpdateGroup;

// Inferred names. The groups keep their members in KdRefList buffers: plain
// lists (appended) or sets sorted by the IAbstract identity of each member
// (KdPtr operator<, a null pointer sorts first).

// One slice of KdUpdateGroup: the members whose GetLayers() has bit key.
// SIZE 0x14
struct KdSliceEntry
{
    int key;           // 0x00
    KdRefList objects; // 0x04
};

// Sorted by key; entries are 0x14 bytes in a KdFactory buffer.
// SIZE 0x10
struct KdSliceMap
{
    KdFactory items; // 0x00

    ~KdSliceMap() throw();
    KdSliceEntry* begin() const { return reinterpret_cast<KdSliceEntry*>(items.begin); }
    KdSliceEntry* end() const
    {
        return reinterpret_cast<KdSliceEntry*>(
            reinterpret_cast<unsigned char*>(items.begin) + items.size);
    }
};

// Result of PtrSetInsert: the slot and whether it was added.
struct KdPtrInsert
{
    IUnknown** slot;
    unsigned char inserted;
};

inline IUnknown** RefBegin(const KdRefList* list)
{
    return reinterpret_cast<IUnknown**>(list->items.begin);
}

inline IUnknown** RefEnd(const KdRefList* list)
{
    return reinterpret_cast<IUnknown**>(
        reinterpret_cast<unsigned char*>(list->items.begin) + list->items.size);
}

// Empties a list without shrinking it (the slots keep their refs until they
// are overwritten or the list dies).
inline void RefClear(KdRefList* list)
{
    if (list->items.capacity < list->items.Count()) {
        GrowCountedBuffer(&list->items, 0, 1);
    }
    list->items.size = 0;
}

extern KdPtr<IUnknown> g_kdNullRef;

// The object identity: its IAbstract face.
__forceinline IUnknown* Identity(IUnknown* object)
{
    return static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract));
}

// PtrIsNull's body. The original calls PtrIsNull (0x00416f40) from
// KdUpdateGroup::RemoveMember / AddMember and inlines it into Remove / Add.
__forceinline bool PtrIsNullInline(IUnknown* const* object)
{
    if (*object) {
        if (g_kdNullRef.ptr) {
            return Identity(*object) == Identity(g_kdNullRef.ptr);
        }
        return false;
    }
    return !g_kdNullRef.ptr;
}

// KdPtr operator==: same IAbstract identity, or both null.
inline bool PtrEqual(IUnknown* a, IUnknown* b)
{
    if (a && b) {
        return a->QueryInterface(&g_iidAbstract) == b->QueryInterface(&g_iidAbstract);
    }
    return !a && !b;
}

IUnknown** PtrRemove(IUnknown** first, IUnknown** last, IUnknown* const* value);
void ReserveRefsLoose(unsigned count, KdRefList* list);
void SortByRenderOrder(IUnknown** first, IUnknown** last);
void PtrListAppend(KdRefList* list, IUnknown* const* object);
void ReserveRefs(unsigned count, KdRefList* list, unsigned char exact);
IUnknown** CopyRefs(IUnknown** first, IUnknown** last, IUnknown** dest);
void PtrListPush(KdRefList* list, IUnknown* const* object);
IUnknown** PtrListFind(IUnknown** first, IUnknown** last, IUnknown* const* key);
KdRefList* SliceMapAt(KdSliceMap* map, const int* key);
KdPtrInsert* PtrSetInsert(KdRefList* set, IUnknown* const* key, KdPtrInsert* out);
void PtrSetErase(KdRefList* set, IUnknown* const* key);
void AddEach(IUnknown** first, IUnknown** last, KdUpdateGroup* group);
void RemoveEach(IUnknown** first, IUnknown** last, KdUpdateGroup* group);
void UpdateEach(IUnknown** first, IUnknown** last, int slice, int dt);
bool PtrIsNull(IUnknown* const* object);
IUnknown** PtrSetInsertAt(KdRefList* set, IUnknown** pos, IUnknown* const* key);
KdSliceEntry* SliceMapLowerBound(KdSliceMap* map, const int* key);
IUnknown** PtrLowerBound(IUnknown** first, IUnknown** last, IUnknown* const* key);
bool PtrLess(IUnknown* const* a, IUnknown* const* b);

// PtrLess's body. The original calls PtrLess (0x004170c0) on the erase paths
// but inlines it into the lower bound and the insert paths.
__forceinline bool PtrLessInline(IUnknown* const* a, IUnknown* const* b)
{
    if (*a) {
        if (*b) {
            return Identity(*a) < Identity(*b);
        }
        return false;
    }
    return *b != 0;
}

// PtrSetInsert's body (0x00416dc0), inlined where the original inlines it
// (KdUpdateGroup::AddMember's member set).
__forceinline KdPtrInsert* PtrSetInsertInline(KdRefList* set, IUnknown* const* key,
    KdPtrInsert* out)
{
    IUnknown** const last = RefEnd(set);
    IUnknown** const found = PtrLowerBound(RefBegin(set), last, key);

    if (found != last && !PtrLessInline(key, found)) {
        out->slot = found;
        out->inserted = 0;
        return out;
    }
    out->slot = PtrSetInsertAt(set, found, key);
    out->inserted = 1;
    return out;
}

// PtrSetErase's body. The original inlines it into KdUpdateGroup::RemoveMember
// and AddMember but calls PtrSetErase (0x00416e30) from Remove and Add.
__forceinline void PtrSetEraseInline(KdRefList* set, IUnknown* const* key)
{
    IUnknown** const last = RefEnd(set);
    IUnknown** found = PtrLowerBound(RefBegin(set), last, key);

    if (found == last || PtrLess(key, found)) {
        found = last;
    }
    if (found != RefEnd(set)) {
        EraseCountedBytes(found, found + 1, RefEnd(set));
        set->items.size -= 4;
    }
}

// Inlined at every caller in the original.
__forceinline bool PtrSetContains(const KdRefList* set, IUnknown* const* key)
{
    IUnknown** const last = RefEnd(set);
    IUnknown** const found = PtrLowerBound(RefBegin(set), last, key);
    return found != last && !PtrLess(key, found);
}

}

#endif
