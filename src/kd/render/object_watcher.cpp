#include "kd/render/object_watcher.h"

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/isystem.h"
#include "kd/iface/idisplay.h"
#include "kd/scene/compound_object.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

float RoundToNearest(float value); // host_services.cpp
extern const char kScriptGetTexture[]; // resource_manager.cpp
extern const char kScriptSetCamera[];  // system.cpp

// STRING: VC1 0x004cfcf4
static const char kObjectWatcher[] = "ObjectWatcher";
// GLOBAL: VC1 0x004d0b24
const char* g_kdNameObjectWatcher = kObjectWatcher;
// GLOBAL: VC1 0x004d11f4
extern const char kScriptInitRender[] = "InitRender";
// GLOBAL: VC1 0x004d1200
extern const char kScriptSetPositionable[] = "SetPositionable";
// GLOBAL: VC1 0x004d1210
extern const char kScriptSetRenderable[] = "SetRenderable";
// GLOBAL: VC1 0x004d1220
extern const char kScriptSetRenderMask[] = "SetRenderMask";
// GLOBAL: VC1 0x004cf8d4
extern const char kScriptSetEnabled[] = "SetEnabled";
// GLOBAL: VC1 0x004d123c
extern const char kScriptGetCorrection[] = "GetCorrection";

// Release the slot, then take the iid face of source (AddRef'd), or null.
template <class T>
static inline void AssignFace(KdRef<T>& slot, IUnknown* source, const char** iid)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(source->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// AddRef the new pointer before releasing the old one.
template <class T>
static inline void AssignCounted(KdRef<T>& slot, T* value)
{
    if (value) {
        value->AddRef();
    }
    if (slot.ptr) {
        slot.ptr->Release();
    }
    slot.ptr = value;
}

// MOV EAX, "ObjectWatcher"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00405400
const char* KdObjectWatcher::GetTypeName()
{
    return kObjectWatcher;
}

// Script "GetCorrection": the sub-pixel offset of the last update.
// FUNCTION: VC1 0x004ad870
const Vector2& KdObjectWatcher::GetCorrection()
{
    return correction;
}

// RET 4. Script "SetEnabled".
// FUNCTION: VC1 0x004ad880
void KdObjectWatcher::SetEnabled(bool value)
{
    enabled = value;
}

// RET 4. Script "SetRenderable".
// FUNCTION: VC1 0x004ad890
void KdObjectWatcher::SetRenderable(const KdPtr<IRenderable>& value)
{
    AssignCounted(renderable, value.ptr);
}

// RET 4. Script "SetPositionable": the object the camera follows.
// FUNCTION: VC1 0x004ad8d0
void KdObjectWatcher::SetPositionable(const KdPtr<IPositionable>& value)
{
    AssignCounted(positionable, value.ptr);
}

// MOV EAX, 0x40; RET. IUpdateable vtable+0x0c.
// FUNCTION: VC1 0x004ad910
int KdObjectWatcher::GetUpdateLayer()
{
    return 0x40;
}

// RET 4. IUpdateable vtable+0x10; the layer is ignored.
// FUNCTION: VC1 0x004ad920
int KdObjectWatcher::SetLayer(unsigned layer)
{
    (void)layer;
    return 0;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x004ad930
unsigned char KdObjectWatcher::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// RET 4 (hidden return). Script "GetTexture": the render target's texture
// (IRenderTarget vtable+0x1c), or the null object's ITexture face.
// FUNCTION: VC1 0x004ad940
KdPtr<ITexture> KdObjectWatcher::GetTexture()
{
    if (!target) {
        return KdPtr<ITexture>(g_kdNullObject.ptr
            ? static_cast<ITexture*>(g_kdNullObject.ptr->QueryInterface(&g_iidTexture)) : 0);
    }
    return *static_cast<KdPtr<ITexture>*>(target->GetRenderTexture());
}

// RET 8. Script "SetCamera": the camera and the offset added to the watched
// position.
// FUNCTION: VC1 0x004ad9b0
void KdObjectWatcher::SetCamera(const KdPtr<ICamera>& value, const Vector2& cameraOffset)
{
    AssignCounted(camera, value.ptr);
    offset = cameraOffset;
}

// RET 8. Script "SetRenderMask": clears the mask, then ORs in both halves.
// FUNCTION: VC1 0x004ad9f0
void KdObjectWatcher::SetRenderMask(int lo, int hi)
{
    renderMask.lo = 0;
    renderMask.hi = 0;
    renderMask.lo |= lo;
    renderMask.hi |= hi;
}

// ECX=IShutdownable (this+4). Points every reference at the null object.
// FUNCTION: VC1 0x004ada10
int KdObjectWatcher::Shutdown()
{
    AssignFace(positionable, g_kdNullObject.ptr, &g_iidPositionable);
    AssignFace(renderable, g_kdNullObject.ptr, &g_iidRenderable);
    AssignFace(target, g_kdNullObject.ptr, &g_iidRenderTarget);
    AssignFace(camera, g_kdNullObject.ptr, &g_iidCamera);
    return 0;
}

// ECX=this; the count lives in the IAbstract face (this+0x24).
// FUNCTION: VC1 0x004adb60
unsigned KdObjectWatcher::Release()
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

// FUNCTION: VC1 0x004adb80
unsigned KdObjectWatcher::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// RET 8. IUpdateable vtable+0x14 (ECX=this+8). Moves the camera onto the
// watched position plus offset, records how far that point lands from a whole
// pixel in camera space, and renders the renderable into the target under
// renderMask.
// FUNCTION: VC1 0x004adb90
void KdObjectWatcher::Update(unsigned sliceBit, int dt)
{
    (void)sliceBit;
    (void)dt;
    if (!enabled || !positionable || !renderable || !target) {
        return;
    }
    const Matrix3& watched = positionable->GetPosition();
    Vector2 point(watched._31 + offset.x, watched._32 + offset.y);
    Matrix3 view = camera->GetPosition();
    view._31 = point.x;
    view._32 = point.y;
    camera->SetPosition(view);
    Vector2 screen = camera->GetViewMatrix().MultVrt(point);
    correction.x = RoundToNearest(screen.x) - screen.x;
    correction.y = RoundToNearest(screen.y) - screen.y;
    KdPtr<IGraphics> graphics;
    target->BeginFrame(&graphics.ptr, &camera);
    if (graphics.ptr) {
        renderable->RenderMasked(&graphics, &renderMask);
        target->EndFrame();
    }
}

// EAX=this.
// FUNCTION: VC1 0x004adcb0
KdObjectWatcher::~KdObjectWatcher()
{
    Shutdown();
}

// ESI=this. refs=1, the name from the shared empty string, the mask from
// g_defaultRenderMask; enabled is left as allocated.
// FUNCTION: VC1 0x004ae1a0
KdObjectWatcher::KdObjectWatcher()
    : renderMask(), offset(0.0f, 0.0f), correction(0.0f, 0.0f)
{
    renderMask = g_defaultRenderMask;
}

// FUNCTION: VC1 0x004ae250
void** CreateKdObjectWatcher(void** out)
{
    void* const memory = malloc(sizeof(KdObjectWatcher));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdObjectWatcher();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 8. Script "InitRender": a width x height render target from the
// system's display (ISystem vtable+0x40, IDisplay vtable+0x38).
// FUNCTION: VC1 0x004ae340
void KdObjectWatcher::InitRender(int width, int height)
{
    KdPtr<ISystem> system(GetKdSystem());
    KdPtr<IUnknown> display;
    IDisplay* face = *reinterpret_cast<IDisplay**>(
        system->GetDisplay(reinterpret_cast<void**>(&display.ptr)));
    KdPtr<IRenderTarget> created(face->CreateRenderTarget(width, height));
    AssignFace(target, created.ptr, &g_iidRenderTarget);
}

// RET 8. Script method lookup (table 0x004ee7a0, guard bit 0x004ee7b0,
// atexit destructor 0x004c9cd0).
// FUNCTION: VC1 0x004ae400
unsigned char KdObjectWatcher::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptInitRender] = CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::InitRender);
        methods[kScriptSetPositionable] =
            CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::SetPositionable);
        methods[kScriptSetRenderable] =
            CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::SetRenderable);
        methods[kScriptSetRenderMask] =
            CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::SetRenderMask);
        methods[kScriptSetCamera] = CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::SetCamera);
        methods[kScriptSetEnabled] = CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::SetEnabled);
        methods[kScriptGetTexture] = CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::GetTexture);
        methods[kScriptGetCorrection] =
            CreateScriptMethod<KdObjectWatcher>(&KdObjectWatcher::GetCorrection);
    }
    return methods.Find(name, out);
}

}
