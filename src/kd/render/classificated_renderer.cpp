#include "kd/render/classificated_renderer.h"

#include "kdcore/platform.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004ed650
KdRef<IUnknown> g_kdClassificatedNullObject;
// GLOBAL: VC1 0x004e95fc
KdString g_kdClassificatedEmptyName = KdEmptyText();
// GLOBAL: VC1 0x004ed654
float g_kdClassificatedDefaultStateWeight = g_kdInfinity.value;
// GLOBAL: VC1 0x004ed658
float g_kdClassificatedDefaultStatePhase = g_kdInfinity.value;

// STRING: VC1 0x004cfcdc
static const char kClassificatedRenderer[] = "ClassificatedRenderer";
// GLOBAL: VC1 0x004d0b28
const char* g_kdNameClassificatedRenderer = kClassificatedRenderer;

// MOV EAX, "ClassificatedRenderer"; RET. IScriptObject+0x14.
// FUNCTION: VC1 0x00401fe0
const char* KdClassificatedRenderer::GetTypeName()
{
    return kClassificatedRenderer;
}

// EAX=this. KdScriptedObject's constructor (ECX), then the vptrs.
// FUNCTION: VC1 0x00401ff0
KdClassificatedRenderer::KdClassificatedRenderer() throw()
{
}

// IScriptObject+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004b2300
unsigned char KdClassificatedRenderer::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// ADD ECX, 0x5c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004b2310
unsigned KdClassificatedRenderer::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (value == 0) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004b2330
unsigned KdClassificatedRenderer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. Returns the new renderer as IAbstract.
// FUNCTION: VC1 0x004b2370
void** CreateKdClassificatedRenderer(void** out)
{
    void* const memory = malloc(sizeof(KdClassificatedRenderer));
    KdClassificatedRenderer* const object = memory ? new (memory) KdClassificatedRenderer() : 0;
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

// RET 8. IRenderable+0x0c. Renders (KdScriptedObject::Render, direct) only
// when every bit of mask.lo and no bit of mask.hi is among the object's
// classificators.
// FUNCTION: VC1 0x004b23d0
int KdClassificatedRenderer::RenderMasked(void* graphics, void* mask)
{
    const RenderMask* filter = static_cast<const RenderMask*>(mask);
    const unsigned classes = GetClassificators();
    if ((filter->lo & classes) == filter->lo && (filter->hi & classes) == 0) {
        return KdScriptedObject::Render(graphics);
    }
    return 0;
}

// RET 8. The script instance's members, then KdScriptedObject::GetMember
// (inlined, so the instance is asked twice before the compound's table).
// FUNCTION: VC1 0x004b2410
unsigned char KdClassificatedRenderer::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    return KdScriptedObject::GetMember(name, out) != 0;
}

}
