#include "kd/render/group_renderer.h"

#include "kdcore/platform.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004e9650
KdString g_kdGroupRendererEmptyName = KdEmptyText();
// GLOBAL: VC1 0x004ed680
float g_kdGroupRendererDefaultStateWeight = g_kdInfinity.value;
// GLOBAL: VC1 0x004ed684
float g_kdGroupRendererDefaultStatePhase = g_kdInfinity.value;

// STRING: VC1 0x004cfcc0
static const char kGroupRenderer[] = "GroupRenderer";
// GLOBAL: VC1 0x004d0b30
const char* g_kdNameGroupRenderer = kGroupRenderer;

// Inferred name. This file's own never-assigned null object.
// GLOBAL: VC1 0x004ed67c
KdRef<IUnknown> g_kdGroupRendererNullObject;

// The render group back to the null object's IRenderable face (counted), or 0.
static __forceinline void ResetRenderGroup(KdRef<IRenderable>& slot)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!g_kdGroupRendererNullObject.ptr) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<IRenderable*>(
        g_kdGroupRendererNullObject->QueryInterface(&g_iidRenderable));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// MOV EAX, "GroupRenderer"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00405020
const char* KdGroupRenderer::GetTypeName()
{
    return kGroupRenderer;
}

// IVisible vtable+0x10.
// FUNCTION: VC1 0x004ae7f0
bool KdGroupRenderer::IsVisible()
{
    return visible;
}

// RET 4. IVisible vtable+0x0c.
// FUNCTION: VC1 0x004ae800
void KdGroupRenderer::SetVisible(bool value)
{
    visible = value;
}

// KdComponentGroup vtable+0x0c (ECX=this+0x3c): drops the render group.
// FUNCTION: VC1 0x004ae810
int KdGroupRenderer::Shutdown()
{
    ResetRenderGroup(renderGroup);
    return 0;
}

// RET 8. IScriptObject vtable+0x10: the script instance's SetMember (tail
// call).
// FUNCTION: VC1 0x004ae860
unsigned char KdGroupRenderer::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// RET 4. Script "RegisterObject": adds object to the render group unless the
// group is the null object.
// FUNCTION: VC1 0x004ae870
bool KdGroupRenderer::RegisterObject(const KdPtr<IAbstract>& object)
{
    KdPtr<IFunctionalGroup> group(renderGroup.AsPtr());
    IUnknown* const null = g_kdGroupRendererNullObject.ptr;

    if (!null) {
        if (!group.ptr) {
            return false;
        }
    } else if (group.ptr
        && group->QueryInterface(&g_iidAbstract) == null->QueryInterface(&g_iidAbstract)) {
        return false;
    }
    return group->Add(object) != 0;
}

// IAbstract-face release.
// FUNCTION: VC1 0x004ae910
unsigned KdGroupRenderer::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x004ae930
unsigned KdGroupRenderer::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// ESI=this. The render group is reset and released before the
// ScriptedObject teardown.
// FUNCTION: VC1 0x004ae9e0
KdGroupRenderer::~KdGroupRenderer() throw()
{
    ResetRenderGroup(renderGroup);
}

// ESI=this. A fresh "RenderGroup" from the factory, kept as IRenderable.
// FUNCTION: VC1 0x004aeae0
KdGroupRenderer::KdGroupRenderer() throw()
{
    void* created = 0;
    IUnknown* group;

    GetKdFactory()->QueryNamed(&created, &g_kdNameRenderGroup);
    group = static_cast<IUnknown*>(created);
    if (renderGroup.ptr) {
        renderGroup.ptr->Release();
    }
    if (!group) {
        renderGroup.ptr = 0;
        return;
    }
    renderGroup.ptr = static_cast<IRenderable*>(group->QueryInterface(&g_iidRenderable));
    if (renderGroup.ptr) {
        renderGroup.ptr->AddRef();
    }
    group->Release();
}

// FUNCTION: VC1 0x004aebd0
void** CreateKdGroupRenderer(void** out)
{
    void* const memory = malloc(sizeof(KdGroupRenderer));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdGroupRenderer();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 8. IRenderable vtable+0x0c (ECX=this+8): while visible and when the
// classificators pass the mask, the object (KdScriptedObject::Render,
// direct) and then its render group.
// FUNCTION: VC1 0x004aec30
int KdGroupRenderer::RenderMasked(void* graphics, void* mask)
{
    if (!visible) {
        return 0;
    }
    const RenderMask* filter = static_cast<const RenderMask*>(mask);
    const unsigned classes = GetClassificators();
    if ((filter->lo & classes) == filter->lo && (filter->hi & classes) == 0) {
        KdScriptedObject::Render(graphics);
        return renderGroup->Render(graphics);
    }
    return 0;
}

// RET 8. The script instance's members, the own table, then
// KdScriptedObject::GetMember.
// FUNCTION: VC1 0x004aec80
unsigned char KdGroupRenderer::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptRegisterObject] =
            CreateScriptMethod<KdGroupRenderer>(&KdGroupRenderer::RegisterObject);
    }
    if (methods.Find(name, out)) {
        return 1;
    }
    return KdScriptedObject::GetMember(name, out) != 0;
}

}
