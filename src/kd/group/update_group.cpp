#include "kd/group/update_group.h"

#include "kd/factory.h"
#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004d4bc8
static const char kUpdateGroup[] = "UpdateGroup";
// GLOBAL: VC1 0x004d0664
const char* g_kdNameUpdateGroup = kUpdateGroup;

// OR EAX, -1; RET. IUpdateable+0x0c: the group itself runs in every slice.
// FUNCTION: VC1 0x0046f610
int KdUpdateGroup::GetUpdateLayer()
{
    return -1;
}

// FUNCTION: VC1 0x0046f620
void* KdUpdateGroup::QueryInterface(const char** iid)
{
    const char* name = *iid;

    if (name == g_iidUpdateable) {
        return static_cast<IUpdateable*>(this);
    }
    if (name == g_iidFunctionalGroup) {
        return static_cast<IFunctionalGroup*>(this);
    }
    if (name == g_iidIdentifiable) {
        return static_cast<IIdentifiable*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 0x1c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x0046f670
unsigned KdUpdateGroup::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x0046f690
unsigned KdUpdateGroup::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IFunctionalGroup+0x14. Empties the slices, members and fresh lists; the
// pending adds and removes stay.
// FUNCTION: VC1 0x0046f6a0
void KdUpdateGroup::Clear()
{
    if (slices.items.capacity < slices.items.Count()) {
        GrowCountedBuffer(&slices.items, 0, 1);
    }
    slices.items.size = 0;
    RefClear(&members);
    RefClear(&fresh);
}

// EAX=this. SetLayer(layer) on every member that joined since the last
// flush, then empties the list. Inferred name.
// FUNCTION: VC1 0x0046f710
void KdUpdateGroup::FlushFresh()
{
    if (fresh.items.size) {
        IUnknown** it;
        const unsigned value = layer;
        for (it = RefBegin(&fresh); it != RefEnd(&fresh); ++it) {
            static_cast<IUpdateable*>(*it)->SetLayer(value);
        }
        RefClear(&fresh);
    }
}

// EDI=this, cdecl object. Drops a member from every slice, the member set
// and the fresh set. Inferred name.
// FUNCTION: VC1 0x0046f770
void KdUpdateGroup::RemoveMember(IUnknown* const* object)
{
    KdSliceEntry* entry;

    if (PtrIsNull(object)) {
        return;
    }
    if (!PtrSetContains(&members, object)) {
        return;
    }
    for (entry = slices.begin(); entry != slices.end(); ++entry) {
        IUnknown** const last = RefEnd(&entry->objects);
        IUnknown** const found = PtrListFind(RefBegin(&entry->objects), last, object);
        if (found != last) {
            EraseCountedBytes(found, found + 1, last);
            entry->objects.items.size -= 4;
        }
    }
    PtrSetEraseInline(&members, object);
    PtrSetEraseInline(&fresh, object);
}

// cdecl this, object. (Re)files a member into the slices its GetLayers()
// asks for; a new member also goes to the fresh set. Inferred name.
// FUNCTION: VC1 0x0046f920
void KdUpdateGroup::AddMember(IUnknown* const* object)
{
    KdPtrInsert inserted;
    bool isNew;
    unsigned layers;
    int bit;

    if (PtrIsNull(object)) {
        return;
    }
    isNew = true;
    if (PtrSetContains(&members, object)) {
        RemoveMember(object);
        isNew = false;
    }
    layers = static_cast<unsigned>(static_cast<IUpdateable*>(*object)->GetUpdateLayer());
    for (bit = 1; bit != 0x100; bit *= 2) {
        if (layers & static_cast<unsigned>(bit)) {
            PtrListPush(SliceMapAt(&slices, &bit), object);
        }
    }
    PtrSetInsertInline(&members, object, &inserted);
    if (isNew) {
        PtrSetInsert(&fresh, object, &inserted);
    }
}

// IFunctionalGroup+0x10. Queues a remove (and cancels a queued add).
// FUNCTION: VC1 0x0046fb10
void KdUpdateGroup::Remove(const KdPtr<IAbstract>& object)
{
    IUnknown* const source = object.ptr;
    KdPtrInsert inserted;
    bool isNull;

    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        isNull = PtrIsNullInline(&face.ptr);
    }
    if (isNull) {
        return;
    }
    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        PtrSetInsert(&toRemove, &face.ptr, &inserted);
    }
    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        PtrSetErase(&toAdd, &face.ptr);
    }
}

// IFunctionalGroup+0x0c. Queues an add (and cancels a queued remove); 0 for
// an object without IUpdateable.
// FUNCTION: VC1 0x0046fc40
unsigned char KdUpdateGroup::Add(const KdPtr<IAbstract>& object)
{
    IUnknown* const source = object.ptr;
    KdPtrInsert inserted;
    bool isNull;

    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        isNull = PtrIsNullInline(&face.ptr);
    }
    if (isNull) {
        return 0;
    }
    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        PtrSetInsert(&toAdd, &face.ptr, &inserted);
    }
    {
        KdPtr<IUnknown> face(source
            ? static_cast<IUnknown*>(source->QueryInterface(&g_iidUpdateable)) : 0);
        PtrSetErase(&toRemove, &face.ptr);
    }
    return 1;
}

// EAX=this. Member destructors in reverse order, then the IIdentifiable name.
// FUNCTION: VC1 0x0046fd70
KdUpdateGroup::~KdUpdateGroup() throw()
{
}

// IUpdateable+0x14. Applies the queued adds/removes, flushes the fresh set,
// counts slice-1 frames, then updates the slice's members and those filed
// under -1.
// FUNCTION: VC1 0x0046fe00
void KdUpdateGroup::Update(unsigned slice, int dt)
{
    const int key = static_cast<int>(slice);
    int all;
    KdSliceEntry* entry;

    AddEach(RefBegin(&toAdd), RefEnd(&toAdd), this);
    RefClear(&toAdd);
    RemoveEach(RefBegin(&toRemove), RefEnd(&toRemove), this);
    RefClear(&toRemove);
    FlushFresh();
    if (key == 1) {
        ++frames;
    }
    entry = SliceMapLowerBound(&slices, &key);
    if (entry != slices.end() && !(key < entry->key)) {
        UpdateEach(RefBegin(&entry->objects), RefEnd(&entry->objects), key, dt);
    }
    all = -1;
    entry = SliceMapLowerBound(&slices, &all);
    if (entry != slices.end() && !(-1 < entry->key)) {
        UpdateEach(RefBegin(&entry->objects), RefEnd(&entry->objects), key, dt);
    }
}

// IUpdateable+0x10. Applies the queued adds/removes and flushes the fresh
// set; the value is not kept.
// FUNCTION: VC1 0x0046ff30
int KdUpdateGroup::SetLayer(unsigned value)
{
    (void)value;
    AddEach(RefBegin(&toAdd), RefEnd(&toAdd), this);
    RefClear(&toAdd);
    RemoveEach(RefBegin(&toRemove), RefEnd(&toRemove), this);
    RefClear(&toRemove);
    FlushFresh();
    return 0;
}

// ESI=this.
// FUNCTION: VC1 0x0046ffc0
KdUpdateGroup::KdUpdateGroup() throw()
{
    name.Assign(KdString(kUpdateGroup));
    frames = 0;
}

// RET 4. Returns the new group as IAbstract.
// FUNCTION: VC1 0x004700e0
void** CreateKdUpdateGroup(void** out)
{
    void* const memory = malloc(sizeof(KdUpdateGroup));
    KdUpdateGroup* const object = memory ? new (memory) KdUpdateGroup() : 0;
    IUnknown* queried = 0;

    if (object) {
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

}
