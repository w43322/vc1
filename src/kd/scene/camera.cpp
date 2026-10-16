#include "kd/scene/camera.h"

#include "bindings/component.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "bindings/rect.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

float RoundToNearest(float value);

// Also the scene object name GameLevel "GetObjectAt" looks up.
// GLOBAL: VC1 0x004d0dd8
extern const char kCameraName[] = "Camera";
// GLOBAL: VC1 0x004d0684
const char* g_kdNameCamera = kCameraName;

// STRING: VC1 0x004d524c
static const char kScriptWorldToScreen[] = "WorldToScreen";
// STRING: VC1 0x004d525c
static const char kScriptScreenToWorld[] = "ScreenToWorld";
// STRING: VC1 0x004d526c
static const char kScriptWorldToUI[] = "WorldToUI";
// STRING: VC1 0x004d5278
static const char kScriptUIToWorld[] = "UIToWorld";
// STRING: VC1 0x004d5284
static const char kScriptScreenToUI[] = "ScreenToUI";
// STRING: VC1 0x004d5290
static const char kScriptUIToScreen[] = "UIToScreen";
// STRING: VC1 0x004d529c
static const char kScriptGetWorldToScreen[] = "GetWorldToScreen";
// STRING: VC1 0x004d52b0
static const char kScriptGetScreenToWorld[] = "GetScreenToWorld";
// STRING: VC1 0x004d52c4
static const char kScriptGetWorldToUI[] = "GetWorldToUI";
// STRING: VC1 0x004d52d4
static const char kScriptGetUIToWorld[] = "GetUIToWorld";
// STRING: VC1 0x004d52e4
static const char kScriptGetScreenToUI[] = "GetScreenToUI";
// STRING: VC1 0x004d52f4
static const char kScriptGetUIToScreen[] = "GetUIToScreen";
// STRING: VC1 0x004d5304
static const char kScriptToScreen[] = "ToScreen";

// Row-vector point transform, products in the original's operand order.
static inline Vector2 MapPoint(const Matrix3& m, const Vector2& point)
{
    return Vector2(m._21 * point.y + m._11 * point.x + m._31,
        m._22 * point.y + m._12 * point.x + m._32);
}

// MOV EAX, "Camera"; RET. IScriptObject+0x14.
// FUNCTION: VC1 0x00416820
const char* KdCamera::GetTypeName()
{
    return kCameraName;
}

// IPositionable+0x10.
// FUNCTION: VC1 0x00470140
const Matrix3& KdCamera::GetPosition()
{
    return position;
}

// RET 4. IPositionable+0x0c.
// FUNCTION: VC1 0x00470150
void KdCamera::SetPosition(const Matrix3& value)
{
    position = value;
}

// FUNCTION: VC1 0x00470170
const Matrix3& KdCamera::GetUIToScreen()
{
    return uiToScreen;
}

// FUNCTION: VC1 0x00470180
const Matrix3& KdCamera::GetScreenToUI()
{
    return screenToUI;
}

// FUNCTION: VC1 0x00470190
const Matrix3& KdCamera::GetUIToWorld()
{
    return uiToWorld;
}

// FUNCTION: VC1 0x004701a0
const Matrix3& KdCamera::GetWorldToUI()
{
    return worldToUI;
}

// FUNCTION: VC1 0x004701b0
const Matrix3& KdCamera::GetScreenToWorld()
{
    return screenToWorld;
}

// FUNCTION: VC1 0x004701c0
const Matrix3& KdCamera::GetWorldToScreen()
{
    return worldToScreen;
}

// IScriptObject+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004701d0
unsigned char KdCamera::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// ADD ECX, 0xc; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x00470230
unsigned KdCamera::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (value == 0) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x00470250
unsigned KdCamera::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 8 (hidden result). Script "UIToScreen".
// FUNCTION: VC1 0x00470260
Vector2 KdCamera::UIToScreen(const Vector2& point)
{
    return MapPoint(uiToScreen, point);
}

// RET 8. Script "ScreenToUI".
// FUNCTION: VC1 0x004702b0
Vector2 KdCamera::ScreenToUI(const Vector2& point)
{
    return MapPoint(screenToUI, point);
}

// RET 8. Script "UIToWorld".
// FUNCTION: VC1 0x00470300
Vector2 KdCamera::UIToWorld(const Vector2& point)
{
    return MapPoint(uiToWorld, point);
}

// RET 8. Script "WorldToUI".
// FUNCTION: VC1 0x00470350
Vector2 KdCamera::WorldToUI(const Vector2& point)
{
    return MapPoint(worldToUI, point);
}

// RET 8. Script "ScreenToWorld".
// FUNCTION: VC1 0x004703a0
Vector2 KdCamera::ScreenToWorld(const Vector2& point)
{
    return MapPoint(screenToWorld, point);
}

// RET 8. Script "WorldToScreen".
// FUNCTION: VC1 0x004703e0
Vector2 KdCamera::WorldToScreen(const Vector2& point)
{
    return MapPoint(worldToScreen, point);
}

// RET 8. ICamera+0x14, called by KdGraphics::SetView (0x0045ce30). The
// camera position's origin is projected to the screen, inverted into the
// screen to UI transform and centred in the view on whole pixels (0.5 is the
// double at 0x004d8a08). The graphics transform becomes ScreenToUI; the
// IGraphics QI result is used without a null check.
// FUNCTION: VC1 0x00470410
void KdCamera::BindView(const KdPtr<IAbstract>& graphics, const Rect& view)
{
    screenToUI = position;
    const SQFloat x = worldToScreen._21 * screenToUI._32 + screenToUI._31 * worldToScreen._11
        + worldToScreen._31;
    const SQFloat y = worldToScreen._22 * screenToUI._32 + worldToScreen._12 * screenToUI._31
        + worldToScreen._32;
    screenToUI._31 = x;
    screenToUI._32 = y;
    screenToUI = Matrix3::Inverse(screenToUI);
    screenToUI._31 = RoundToNearest(static_cast<float>(view.width * 0.5 + screenToUI._31));
    screenToUI._32 = RoundToNearest(static_cast<float>(view.height * 0.5 + screenToUI._32));
    uiToScreen = Matrix3::Inverse(screenToUI);
    worldToUI = screenToUI * worldToScreen;
    uiToWorld = Matrix3::Inverse(worldToUI);

    KdPtr<IGraphics> target(graphics);
    target.ptr->SetTransform(screenToUI);
}

// ESI=this. Resets the script instance before the members go.
// FUNCTION: VC1 0x00470590
KdCamera::~KdCamera() throw()
{
    IScriptInstanceReset* reset = &script;
    reset->Reset();
}

// EAX=this. All seven matrices start as identity.
// FUNCTION: VC1 0x00470600
KdCamera::KdCamera() throw()
{
}

// RET 4. Returns the new camera as IAbstract.
// FUNCTION: VC1 0x004707c0
void** CreateKdCamera(void** out)
{
    void* const memory = malloc(sizeof(KdCamera));
    KdCamera* const object = memory ? new (memory) KdCamera() : 0;
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

// RET 4. IScriptLoadable+0x0c: loads the script instance (owner = the
// IScriptObject face), then the projection from the table member "ToScreen"
// (0x00416930 is GetScriptValue<Matrix3>). Before the first BindView the UI
// matrices equal the screen ones.
// FUNCTION: VC1 0x00470820
bool KdCamera::LoadFromScript(SquirrelObject& table)
{
    if (!script.Load(table, static_cast<IScriptObject*>(this))) {
        return false;
    }
    {
        IScriptInstance* instance = &script;
        SquirrelObject member;
        instance->GetMember(kScriptToScreen, member);
        worldToScreen = GetScriptValue<Matrix3>(member);
    }
    screenToWorld = Matrix3::Inverse(worldToScreen);
    worldToUI = worldToScreen;
    uiToWorld = screenToWorld;
    return true;
}

// RET 8. The script instance's members first, then the method table (table
// 0x004ee664, guard bit 0x004ee674): the six point mappings and the six
// matrix getters (ICamera slots through vcall thunks).
// FUNCTION: VC1 0x00470a50
unsigned char KdCamera::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptWorldToScreen] = CreateScriptMethod<KdCamera>(&KdCamera::WorldToScreen);
        methods[kScriptScreenToWorld] = CreateScriptMethod<KdCamera>(&KdCamera::ScreenToWorld);
        methods[kScriptWorldToUI] = CreateScriptMethod<KdCamera>(&KdCamera::WorldToUI);
        methods[kScriptUIToWorld] = CreateScriptMethod<KdCamera>(&KdCamera::UIToWorld);
        methods[kScriptScreenToUI] = CreateScriptMethod<KdCamera>(&KdCamera::ScreenToUI);
        methods[kScriptUIToScreen] = CreateScriptMethod<KdCamera>(&KdCamera::UIToScreen);
        methods[kScriptGetWorldToScreen] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetWorldToScreen));
        methods[kScriptGetScreenToWorld] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetScreenToWorld));
        methods[kScriptGetWorldToUI] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetWorldToUI));
        methods[kScriptGetUIToWorld] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetUIToWorld));
        methods[kScriptGetScreenToUI] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetScreenToUI));
        methods[kScriptGetUIToScreen] = CreateScriptMethod<KdCamera>(
            static_cast<const Matrix3& (KdCamera::*)()>(&ICamera::GetUIToScreen));
    }
    return methods.Find(name, out);
}

}
