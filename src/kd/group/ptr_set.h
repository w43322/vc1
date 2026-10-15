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

inline bool PtrSetContains(const KdRefList* set, IUnknown* const* key)
{
    IUnknown** const last = RefEnd(set);
    IUnknown** const found = PtrLowerBound(RefBegin(set), last, key);
    return found != last && !PtrLess(key, found);
}

}

#endif
