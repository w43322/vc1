#ifndef VC1_KD_WORLD_GAME_OBJECT_H
#define VC1_KD_WORLD_GAME_OBJECT_H

#include "kd/scene/scripted_object.h"

namespace vc1 {

// Inferred name. Factory "GameObject" (Level.nut CreateObjectFromRecord: the
// plants, CameraMover, Garage, ...): a ScriptedObject whose SetPosition also
// moves every positionable child component by the same transform. QI
// 0x004b0ae0 asks the ScriptedObject first, then answers IAbstract.
// SIZE 0xd8
// VTABLE: VC1 0x004d0ec0 vc1::IUpdateable
// VTABLE: VC1 0x004d0ed8 vc1::IRenderable
// VTABLE: VC1 0x004d0ef0 vc1::IInputHandler
// VTABLE: VC1 0x004d0f04 vc1::IStateProvider
// VTABLE: VC1 0x004d0f18 vc1::IScriptObject
// VTABLE: VC1 0x004d0f30 vc1::IScriptLoadable
// VTABLE: VC1 0x004d0f40 vc1::IPositionable
// VTABLE: VC1 0x004d0f54 vc1::IIdentifiable
// VTABLE: VC1 0x004d0f6c vc1::IClassifiable
// VTABLE: VC1 0x004d0f88 vc1::KdComponentGroup
// VTABLE: VC1 0x004d0fb0 vc1::IAbstract
class KdGameObject : public KdScriptedObject
{
public:
    KdGameObject() throw();
    ~KdGameObject() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    void SetPosition(const Matrix3& value);
    int Shutdown();
};

typedef char Vc1KdGameObjectSizeCheck[sizeof(KdGameObject) == 0xd8 ? 1 : -1];

void** CreateKdGameObject(void** out);

// SYNTHETIC: VC1 0x00404350
// vc1::KdGameObject::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00404370
// ?Release@KdGameObject@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00404380
// ?QueryInterface@KdGameObject@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404390
// ?AddRef@KdGameObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x004043a0
// ?Release@KdGameObject@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x004043b0
// ?QueryInterface@KdGameObject@vc1@@WDA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004043c0
// ??_EKdGameObject@vc1@@WDA@AEPAXI@Z
// SYNTHETIC: VC1 0x004043d0
// ?Release@KdGameObject@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004043e0
// ?QueryInterface@KdGameObject@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004043f0
// ?AddRef@KdGameObject@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404400
// ?Release@KdGameObject@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00404410
// ?QueryInterface@KdGameObject@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404420
// ?AddRef@KdGameObject@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00404430
// ?Release@KdGameObject@vc1@@WDI@AEIXZ
// SYNTHETIC: VC1 0x00404440
// ?QueryInterface@KdGameObject@vc1@@WDI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404450
// ?AddRef@KdGameObject@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x00404460
// ?Release@KdGameObject@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00404470
// ?QueryInterface@KdGameObject@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404480
// ?AddRef@KdGameObject@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00404490
// ?Release@KdGameObject@vc1@@WFM@AEIXZ
// SYNTHETIC: VC1 0x004044a0
// ?QueryInterface@KdGameObject@vc1@@WFM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004044b0
// ??_EKdGameObject@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x004044c0
// ?AddRef@KdGameObject@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x004044d0
// ??_EKdGameObject@vc1@@WFM@AEPAXI@Z
// SYNTHETIC: VC1 0x004044e0
// ?AddRef@KdGameObject@vc1@@WDI@AEIXZ
// SYNTHETIC: VC1 0x004044f0
// ?Release@KdGameObject@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00404500
// ?QueryInterface@KdGameObject@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404510
// ?AddRef@KdGameObject@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00404520
// ?AddRef@KdGameObject@vc1@@WFM@AEIXZ
// SYNTHETIC: VC1 0x00404530
// ?Release@KdGameObject@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00404540
// ?QueryInterface@KdGameObject@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404550
// ?Release@KdGameObject@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404560
// ?QueryInterface@KdGameObject@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404570
// ?AddRef@KdGameObject@vc1@@W3AEIXZ

}

#endif
