#include "kd/factory.h"
#include "kd/group/ptr_set.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iid.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/irenderable.h"

#include <new>
#include <stdlib.h>

namespace vc1 {


// Inferred name. Factory "RenderGroup": renders its IRenderable members in
// GetRenderOrder() order while visible.
// SIZE 0x3c
// VTABLE: VC1 0x004d5654 vc1::IFunctionalGroup
// VTABLE: VC1 0x004d566c vc1::IRenderable
// VTABLE: VC1 0x004d5684 vc1::IVisible
// VTABLE: VC1 0x004d5698 vc1::IIdentifiable
// VTABLE: VC1 0x004d56b0 vc1::IAbstract
class KdRenderGroup : public IFunctionalGroup,
                      public IRenderable,
                      public IVisible,
                      public IIdentifiable,
                      public IAbstract
{
public:
    KdRefList members;     // 0x28
    unsigned char visible; // 0x38

    KdRenderGroup() throw();
    ~KdRenderGroup() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char Add(void** object);
    void Remove(void** object);
    void Clear();
    int RenderMasked(void* graphics, void* space);
    int Render(void* graphics);
    int GetRenderOrder();
    void SetVisible(bool value);
    bool IsVisible();
};

typedef char Vc1KdRenderGroupSizeCheck[sizeof(KdRenderGroup) == 0x3c ? 1 : -1];

// STRING: VC1 0x004d4bbc
static const char kRenderGroup[] = "RenderGroup";
// GLOBAL: VC1 0x004d0668
const char* g_kdNameRenderGroup = kRenderGroup;

// The null KdPtr<IRenderable> Add/Remove compare against. Inferred name.
// GLOBAL: VC1 0x004ed6e0
KdPtr<IUnknown> g_kdNullRenderable;

// IRenderable+0x14: render order 0.
// FUNCTION: VC1 0x0041adb0 FOLDED
int KdRenderGroup::GetRenderOrder()
{
    return 0;
}

// IVisible+0x10.
// FUNCTION: VC1 0x0046d090
bool KdRenderGroup::IsVisible()
{
    return visible != 0;
}

// IVisible+0x0c.
// FUNCTION: VC1 0x0046d0a0
void KdRenderGroup::SetVisible(bool value)
{
    visible = value;
}

// IRenderable+0x10: RenderMasked under g_defaultRenderMask.
// FUNCTION: VC1 0x0046d0b0
int KdRenderGroup::Render(void* graphics)
{
    return RenderMasked(graphics, &g_defaultRenderMask);
}

// FUNCTION: VC1 0x0046d0d0
void* KdRenderGroup::QueryInterface(const char** iid)
{
    const char* name = *iid;

    if (name == g_iidVisible) {
        return static_cast<IVisible*>(this);
    }
    if (name == g_iidRenderable) {
        return static_cast<IRenderable*>(this);
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

// ADD ECX, 0x20; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x0046d130
unsigned KdRenderGroup::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x0046d150
unsigned KdRenderGroup::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// FUNCTION: VC1 0x0046d160
void KdRenderGroup::Clear()
{
    RefClear(&members);
}

// IRenderable+0x0c. Every member's RenderMasked while visible.
// FUNCTION: VC1 0x0046d190
int KdRenderGroup::RenderMasked(void* graphics, void* space)
{
    if (visible) {
        IUnknown** it;
        IUnknown** const last = RefEnd(&members);
        for (it = RefBegin(&members); it != last; ++it) {
            static_cast<IRenderable*>(*it)->RenderMasked(graphics, space);
        }
    }
    return 0;
}

// Appends the object's IRenderable face and re-sorts by render order; 0 for
// an object without one.
// FUNCTION: VC1 0x0046d1d0
unsigned char KdRenderGroup::Add(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    unsigned char added = 0;
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidRenderable)) : 0);

    if (!PtrEqual(g_kdNullRenderable.ptr, face.ptr)) {
        PtrListAppend(&members, &face.ptr);
        SortByRenderOrder(RefBegin(&members), RefEnd(&members));
        added = 1;
    }
    return added;
}

// EAX=this.
// FUNCTION: VC1 0x0046d280
KdRenderGroup::~KdRenderGroup() throw()
{
}

// Drops every slot holding the object's IRenderable face.
// FUNCTION: VC1 0x0046d2f0
void KdRenderGroup::Remove(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidRenderable)) : 0);

    if (!PtrEqual(g_kdNullRenderable.ptr, face.ptr)) {
        IUnknown** const first = RefBegin(&members);
        IUnknown** const last = RefEnd(&members);
        IUnknown** const kept = PtrRemove(first, last, &face.ptr);
        const unsigned removed = static_cast<unsigned>(last - kept) * 4;
        EraseCountedBytes(kept, kept + (last - kept), RefEnd(&members));
        members.items.size -= removed;
    }
}

// ESI=this.
// FUNCTION: VC1 0x0046d3c0
KdRenderGroup::KdRenderGroup() throw()
{
    name.Assign(KdString(kRenderGroup));
    visible = 1;
}

// RET 4. Returns the new group as IAbstract.
// FUNCTION: VC1 0x0046d4b0
void** CreateKdRenderGroup(void** out)
{
    void* const memory = malloc(sizeof(KdRenderGroup));
    KdRenderGroup* const object = memory ? new (memory) KdRenderGroup() : 0;
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
