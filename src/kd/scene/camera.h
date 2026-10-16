#ifndef VC1_KD_SCENE_CAMERA_H
#define VC1_KD_SCENE_CAMERA_H

#include "bindings/matrix3.h"
#include "bindings/vector2.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icamera.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"
#include "kd/script_instance.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "Camera": the view of the level. Its position is
// the scroll offset; the script table member "ToScreen" is the world to
// screen projection. BindView (called by KdGraphics::SetView) centres the
// position in the view and rebuilds the screen/UI matrices, then installs
// ScreenToUI as the graphics transform. Matrix names come from the script
// getters in GetMember (0x00470a50). QI 0x004701e0 gives the face offsets.
// SIZE 0x128
// VTABLE: VC1 0x004d5310 vc1::ICamera
// VTABLE: VC1 0x004d5340 vc1::IScriptLoadable
// VTABLE: VC1 0x004d5350 vc1::IScriptObject
// VTABLE: VC1 0x004d5368 vc1::IAbstract
class KdCamera : public ICamera,
                 public IScriptLoadable,
                 public IScriptObject,
                 public IAbstract
{
public:
    KdScriptInstance script; // 0x14
    Matrix3 position;        // 0x2c
    Matrix3 worldToScreen;   // 0x50 script "ToScreen"
    Matrix3 screenToWorld;   // 0x74
    Matrix3 worldToUI;       // 0x98
    Matrix3 uiToWorld;       // 0xbc
    Matrix3 screenToUI;      // 0xe0 the graphics transform
    Matrix3 uiToScreen;      // 0x104

    KdCamera() throw();
    ~KdCamera() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    void BindView(const KdPtr<IAbstract>& graphics, const Rect& view);
    const Matrix3& GetWorldToScreen();
    const Matrix3& GetScreenToWorld();
    const Matrix3& GetWorldToUI();
    const Matrix3& GetUIToWorld();
    const Matrix3& GetScreenToUI();
    const Matrix3& GetUIToScreen();

    bool LoadFromScript(SquirrelObject& table);

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    // Script point mappings through the matrices above.
    Vector2 UIToScreen(const Vector2& point);
    Vector2 ScreenToUI(const Vector2& point);
    Vector2 UIToWorld(const Vector2& point);
    Vector2 WorldToUI(const Vector2& point);
    Vector2 ScreenToWorld(const Vector2& point);
    Vector2 WorldToScreen(const Vector2& point);
};

typedef char Vc1KdCameraSizeCheck[sizeof(KdCamera) == 0x128 ? 1 : -1];

void** CreateKdCamera(void** out);

// SYNTHETIC: VC1 0x00416830
// vc1::KdCamera::`scalar deleting destructor'

// TEMPLATE: VC1 0x00416850
// ??$CreateScriptMethod@VKdCamera@vc1@@P812@AE?AUVector2@2@ABU32@@Z@vc1@@YA?AVSquirrelObject@@P8KdCamera@0@AE?AUVector2@0@ABU30@@Z@Z

// TEMPLATE: VC1 0x004168c0
// ??$CreateScriptMethod@VKdCamera@vc1@@P812@AEABUMatrix3@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdCamera@0@AEABUMatrix3@0@XZ@Z

// TEMPLATE: VC1 0x00416930
// ??$GetScriptValue@UMatrix3@vc1@@@vc1@@YA?AUMatrix3@0@ABVSquirrelObject@@@Z

// TEMPLATE: VC1 0x00470910
// ?Dispatch@?$ScriptMethodCall@VKdCamera@vc1@@P812@AEABUMatrix3@2@XZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004709b0
// ?Dispatch@?$ScriptMethodCall@VKdCamera@vc1@@P812@AE?AUVector2@2@ABU32@@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00416a90
// ?QueryInterface@KdCamera@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00416aa0
// ?Release@KdCamera@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00416ab0
// ?Release@KdCamera@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00416ac0
// ?AddRef@KdCamera@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00416ad0
// ??_EKdCamera@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x00416ae0
// ?AddRef@KdCamera@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00416af0
// ?AddRef@KdCamera@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00416b00
// ?QueryInterface@KdCamera@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00416b10
// ?QueryInterface@KdCamera@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00416b20
// ?Release@KdCamera@vc1@@W3AEIXZ

}

#endif
