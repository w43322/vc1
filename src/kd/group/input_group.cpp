#include "kd/factory.h"
#include "kd/group/ptr_set.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iid.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iinput_handler.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Inferred name. Factory "InputGroup": offers key and pointer events to its
// IInputHandler members in insertion order until one consumes it.
// SIZE 0x34
// VTABLE: VC1 0x004d5d28 vc1::IFunctionalGroup
// VTABLE: VC1 0x004d5d40 vc1::IInputHandler
// VTABLE: VC1 0x004d5d54 vc1::IIdentifiable
// VTABLE: VC1 0x004d5d6c vc1::IAbstract
class KdInputGroup : public IFunctionalGroup,
                     public IInputHandler,
                     public IIdentifiable,
                     public IAbstract
{
public:
    KdRefList members; // 0x24

    KdInputGroup() throw();
    ~KdInputGroup() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char Add(void** object);
    void Remove(void** object);
    void Clear();
    unsigned char OnKey(const unsigned* data);
    unsigned char OnPointer(const unsigned* data);
};

typedef char Vc1KdInputGroupSizeCheck[sizeof(KdInputGroup) == 0x34 ? 1 : -1];

// STRING: VC1 0x004d4bb0
static const char kInputGroup[] = "InputGroup";
// GLOBAL: VC1 0x004d066c
const char* g_kdNameInputGroup = kInputGroup;

// The null KdPtr<IInputHandler> Add/Remove compare against. Inferred name.
// GLOBAL: VC1 0x004ed6e8
KdPtr<IUnknown> g_kdNullInputHandler;

// FUNCTION: VC1 0x004b68c0
void* KdInputGroup::QueryInterface(const char** iid)
{
    const char* name = *iid;

    if (name == g_iidInputHandler) {
        return static_cast<IInputHandler*>(this);
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
// FUNCTION: VC1 0x004b6910
unsigned KdInputGroup::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004b6930
unsigned KdInputGroup::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IInputHandler+0x10.
// FUNCTION: VC1 0x004b6940
unsigned char KdInputGroup::OnPointer(const unsigned* data)
{
    unsigned i;

    for (i = 0; i < (members.items.size >> 2); ++i) {
        if (static_cast<IInputHandler*>(RefBegin(&members)[i])->OnPointer(data)) {
            return 1;
        }
    }
    return 0;
}

// IInputHandler+0x0c.
// FUNCTION: VC1 0x004b6990
unsigned char KdInputGroup::OnKey(const unsigned* data)
{
    unsigned i;

    for (i = 0; i < (members.items.size >> 2); ++i) {
        if (static_cast<IInputHandler*>(RefBegin(&members)[i])->OnKey(data)) {
            return 1;
        }
    }
    return 0;
}

// FUNCTION: VC1 0x004b69e0
void KdInputGroup::Clear()
{
    RefClear(&members);
}

// Appends the object's IInputHandler face; 0 for an object without one.
// FUNCTION: VC1 0x004b6a10
unsigned char KdInputGroup::Add(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    unsigned char added = 0;
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidInputHandler)) : 0);

    if (!PtrEqual(g_kdNullInputHandler.ptr, face.ptr)) {
        PtrListAppend(&members, &face.ptr);
        added = 1;
    }
    return added;
}

// EAX=this.
// FUNCTION: VC1 0x004b6ab0
KdInputGroup::~KdInputGroup() throw()
{
}

// Drops every slot holding the object's IInputHandler face.
// FUNCTION: VC1 0x004b6b20
void KdInputGroup::Remove(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidInputHandler)) : 0);

    if (!PtrEqual(g_kdNullInputHandler.ptr, face.ptr)) {
        IUnknown** const first = RefBegin(&members);
        IUnknown** const last = RefEnd(&members);
        IUnknown** const kept = PtrRemove(first, last, &face.ptr);
        const unsigned removed = static_cast<unsigned>(last - kept) * 4;
        EraseCountedBytes(kept, kept + (last - kept), RefEnd(&members));
        members.items.size -= removed;
    }
}

// ESI=this.
// FUNCTION: VC1 0x004b6bf0
KdInputGroup::KdInputGroup() throw()
{
    name.Assign(KdString(kInputGroup));
}

// RET 4. Returns the new group as IAbstract.
// FUNCTION: VC1 0x004b6cd0
void** CreateKdInputGroup(void** out)
{
    void* const memory = malloc(sizeof(KdInputGroup));
    KdInputGroup* const object = memory ? new (memory) KdInputGroup() : 0;
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
