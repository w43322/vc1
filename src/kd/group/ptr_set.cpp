#include "kd/group/ptr_set.h"

#include "kd/group/update_group.h"
#include "kd/iface/iid.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iupdateable.h"

#include <new>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004e987c
KdString g_kdPtrSetEmptyName = KdEmptyText();

// The null KdPtr the groups compare new members against (destroyed at exit
// by 0x004ca850). Inferred name.
// GLOBAL: VC1 0x004ed6d0
KdPtr<IUnknown> g_kdNullRef;


// ECX=first, EAX=dest (first), EBP=value, cdecl last. std::remove of every
// slot equal to value (KdPtr operator==); returns the new end.
// FUNCTION: VC1 0x00405c20
IUnknown** PtrRemove(IUnknown** first, IUnknown** last, IUnknown* const* value)
{
    IUnknown** dest = first;

    for (; first != last; ++first) {
        if (PtrEqual(*first, *value)) {
            continue;
        }
        if (*first) {
            (*first)->AddRef();
        }
        if (*dest) {
            (*dest)->Release();
        }
        *dest = *first;
        ++dest;
    }
    return dest;
}

// EAX=count, ECX=list. ReserveRefs with exact 0, for the KdPtr<IRenderable>
// and KdPtr<IInputHandler> lists.
// FUNCTION: VC1 0x00405cb0
void ReserveRefsLoose(unsigned count, KdRefList* list)
{
    const unsigned old = list->items.capacity >> 2;
    IUnknown** slot;
    IUnknown** end;

    GrowCountedBuffer(&list->items, static_cast<int>(count * 4), 0);
    if ((list->items.capacity >> 2) > old) {
        slot = reinterpret_cast<IUnknown**>(list->items.begin) + old;
        end = reinterpret_cast<IUnknown**>(list->items.begin) + (list->items.capacity >> 2);
        for (; slot < end; ++slot) {
            if (slot) {
                *slot = 0;
            }
        }
    }
}

// EAX=list, EBX=object. Appends one counted ref.
// FUNCTION: VC1 0x00416c10
void PtrListPush(KdRefList* list, IUnknown* const* object)
{
    const unsigned needed = ((list->items.size >> 2) + 1) * 4;
    IUnknown** slot;

    if (list->items.capacity < needed) {
        ReserveRefs(needed >> 2, list, 0);
    }
    list->items.size = needed;
    slot = reinterpret_cast<IUnknown**>(
        reinterpret_cast<unsigned char*>(list->items.begin) + needed - 4);
    if (*object) {
        (*object)->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = *object;
}

// EAX=first, EBX=key, cdecl last. Linear search by identity.
// FUNCTION: VC1 0x00416c70
IUnknown** PtrListFind(IUnknown** first, IUnknown** last, IUnknown* const* key)
{
    for (; first != last; ++first) {
        if (*first && *key) {
            if (Identity(*key) == Identity(*first)) {
                return first;
            }
        }
        else if (!*first && !*key) {
            return first;
        }
    }
    return first;
}

// Releases every slice list up to capacity, then the entry buffer.
// FUNCTION: VC1 0x00416cd0
KdSliceMap::~KdSliceMap() throw()
{
    if (items.capacity) {
        KdSliceEntry* entry = begin();
        KdSliceEntry* const last = entry + items.capacity / sizeof(KdSliceEntry);
        for (; entry < last; ++entry) {
            entry->objects.~KdRefList();
        }
    }
}

static void ConstructSliceEntries(KdSliceEntry* first, KdSliceEntry* last);
static KdSliceEntry* SliceMapInsertSlot(KdSliceMap* map, KdSliceEntry* pos);
static void CopySliceEntry(KdSliceEntry* dest, const int* key, const KdRefList* source);
static KdSliceEntry* AssignSliceEntry(KdSliceEntry* dest, const KdSliceEntry* source);

// ECX=map, EAX=key. The slice list for key, inserting an empty one.
// FUNCTION: VC1 0x00416d40
KdRefList* SliceMapAt(KdSliceMap* map, const int* key)
{
    KdSliceEntry* entry = SliceMapLowerBound(map, key);

    if (entry == map->end() || *key < entry->key) {
        KdRefList empty;
        KdSliceEntry fresh;
        CopySliceEntry(&fresh, key, &empty);
        entry = SliceMapInsertSlot(map, entry);
        AssignSliceEntry(entry, &fresh);
    }
    return &entry->objects;
}

// ESI=set, EAX=key, stdcall out. Sorted insert unless already present.
// FUNCTION: VC1 0x00416dc0
KdPtrInsert* PtrSetInsert(KdRefList* set, IUnknown* const* key, KdPtrInsert* out)
{
    return PtrSetInsertInline(set, key, out);
}

// EDI=set, stdcall key. Removes the key's slot if present.
// FUNCTION: VC1 0x00416e30
void PtrSetErase(KdRefList* set, IUnknown* const* key)
{
    PtrSetEraseInline(set, key);
}

// for_each over the pending adds (EAX=first, EDI=last, EBX=out functor).
// FUNCTION: VC1 0x00416e80
void AddEach(IUnknown** first, IUnknown** last, KdUpdateGroup* group)
{
    for (; first != last; ++first) {
        group->AddMember(first);
    }
}

// for_each over the pending removes (EAX=first, EBX=last).
// FUNCTION: VC1 0x00416ec0
void RemoveEach(IUnknown** first, IUnknown** last, KdUpdateGroup* group)
{
    for (; first != last; ++first) {
        group->RemoveMember(first);
    }
}

// for_each over one slice (EAX=first, EDI=last): IUpdateable::Update.
// FUNCTION: VC1 0x00416f00
void UpdateEach(IUnknown** first, IUnknown** last, int slice, int dt)
{
    for (; first != last; ++first) {
        static_cast<IUpdateable*>(*first)->Update(static_cast<unsigned>(slice), dt);
    }
}

// EAX=object. KdPtr operator== against g_kdNullRef. Out of line at both
// callers (KdUpdateGroup::RemoveMember / AddMember).
// FUNCTION: VC1 0x00416f40
__declspec(noinline) bool PtrIsNull(IUnknown* const* object)
{
    return PtrIsNullInline(object);
}

// EAX=set, EBX=key, stdcall pos. Opens a slot at pos and stores the key.
// FUNCTION: VC1 0x00416f90
IUnknown** PtrSetInsertAt(KdRefList* set, IUnknown** pos, IUnknown* const* key)
{
    unsigned char* const oldBegin = reinterpret_cast<unsigned char*>(set->items.begin);
    IUnknown** slot;

    ReserveRefs((set->items.size >> 2) + 1, set, 0);
    slot = static_cast<IUnknown**>(InsertCountedBytes(&set->items,
        reinterpret_cast<unsigned char*>(set->items.begin)
            + (reinterpret_cast<unsigned char*>(pos) - oldBegin),
        4));
    if (*key) {
        (*key)->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = *key;
    return slot;
}

// EAX=map, stdcall key. Binary search of the 0x14-byte slice entries.
// FUNCTION: VC1 0x00416ff0
KdSliceEntry* SliceMapLowerBound(KdSliceMap* map, const int* key)
{
    KdSliceEntry* low = map->begin();
    KdSliceEntry* high = map->end();

    while (low != high) {
        KdSliceEntry* const mid = low + (high - low) / 2;
        if (mid->key < *key) {
            low = mid + 1;
        }
        else {
            high = mid;
        }
    }
    return low;
}

// EAX=first, cdecl last and key. Binary search by identity.
// FUNCTION: VC1 0x00417040
IUnknown** PtrLowerBound(IUnknown** first, IUnknown** last, IUnknown* const* key)
{
    while (first != last) {
        IUnknown** const mid = first + (last - first) / 2;
        if (PtrLessInline(mid, key)) {
            first = mid + 1;
        }
        else {
            last = mid;
        }
    }
    return first;
}

// ECX=a, EAX=b. KdPtr operator<: identities compared, null first.
// FUNCTION: VC1 0x004170c0
__declspec(noinline) bool PtrLess(IUnknown* const* a, IUnknown* const* b)
{
    if (*a) {
        if (*b) {
            return Identity(*a) < Identity(*b);
        }
        return false;
    }
    return *b != 0;
}

// EDI=dest, EAX=key, cdecl source list. Copy-constructs a slice entry.
// FUNCTION: VC1 0x00417110
static void CopySliceEntry(KdSliceEntry* dest, const int* key, const KdRefList* source)
{
    const unsigned count = source->items.size >> 2;

    dest->key = *key;
    new (&dest->objects) KdRefList();
    if (count) {
        ReserveRefs(count, &dest->objects, 1);
    }
    dest->objects.items.size = count * 4;
    CopyRefs(RefBegin(source), RefEnd(source), RefBegin(&dest->objects));
}

// EAX=source, cdecl dest. Slice entry assignment.
// FUNCTION: VC1 0x00417170
static KdSliceEntry* AssignSliceEntry(KdSliceEntry* dest, const KdSliceEntry* source)
{
    const unsigned bytes = (source->objects.items.size >> 2) * 4;

    dest->key = source->key;
    if (dest->objects.items.capacity < bytes) {
        ReserveRefs(source->objects.items.size >> 2, &dest->objects, 1);
    }
    dest->objects.items.size = bytes;
    CopyRefs(RefBegin(&source->objects), RefEnd(&source->objects), RefBegin(&dest->objects));
    return dest;
}

// ECX=map, EAX=pos. Grows by one entry (new slots constructed empty) and
// opens a 0x14-byte slot at pos.
// FUNCTION: VC1 0x004171d0
static KdSliceEntry* SliceMapInsertSlot(KdSliceMap* map, KdSliceEntry* pos)
{
    unsigned char* const oldBegin = reinterpret_cast<unsigned char*>(map->items.begin);
    const unsigned oldCap = map->items.capacity / sizeof(KdSliceEntry);

    GrowCountedBuffer(&map->items,
        static_cast<int>((map->items.size / sizeof(KdSliceEntry) + 1) * sizeof(KdSliceEntry)), 0);
    if (oldCap < map->items.capacity / sizeof(KdSliceEntry)) {
        ConstructSliceEntries(map->begin() + oldCap,
            map->begin() + map->items.capacity / sizeof(KdSliceEntry));
    }
    return static_cast<KdSliceEntry*>(InsertCountedBytes(&map->items,
        reinterpret_cast<unsigned char*>(map->items.begin)
            + (reinterpret_cast<unsigned char*>(pos) - oldBegin),
        sizeof(KdSliceEntry)));
}

// ECX=first, ESI=last. Placement-constructs empty slice entries.
// FUNCTION: VC1 0x00417240
static void ConstructSliceEntries(KdSliceEntry* first, KdSliceEntry* last)
{
    for (; first < last; ++first) {
        if (first) {
            first->key = 0;
            new (&first->objects) KdRefList();
        }
    }
}

// cdecl first, last. Stable insertion sort of IRenderables by their
// GetRenderOrder() order; each element is rotated back past every greater one.
// FUNCTION: VC1 0x00417c00
void SortByRenderOrder(IUnknown** first, IUnknown** last)
{
    IUnknown** current;

    for (current = first + 1; current < last; ++current) {
        IUnknown** pos = current;
        while (pos > first
            && static_cast<IRenderable*>(pos[-1])->GetRenderOrder()
                > static_cast<IRenderable*>(*current)->GetRenderOrder()) {
            --pos;
        }
        EraseCountedBytes(pos, current, current + 1);
    }
}

// EAX=list, EBX=object. PtrListPush through ReserveRefsLoose.
// FUNCTION: VC1 0x0041a360
void PtrListAppend(KdRefList* list, IUnknown* const* object)
{
    const unsigned needed = ((list->items.size >> 2) + 1) * 4;
    IUnknown** slot;

    if (list->items.capacity < needed) {
        ReserveRefsLoose(needed >> 2, list);
    }
    list->items.size = needed;
    slot = reinterpret_cast<IUnknown**>(
        reinterpret_cast<unsigned char*>(list->items.begin) + needed - 4);
    if (*object) {
        (*object)->AddRef();
    }
    if (*slot) {
        (*slot)->Release();
    }
    *slot = *object;
}

}
