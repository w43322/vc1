#ifndef VC1_KD_SCRIPTED_OBJECT_H
#define VC1_KD_SCRIPTED_OBJECT_H

#include "kd/iface/iinput_handler.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/istate_provider.h"
#include "kd/iface/iupdateable.h"
#include "kd/scene/compound_object.h"
#include "bindings/matrix3.h"

namespace vc1 {

// Script member names that also gate the QueryInterface faces (0x00471040).
extern const char kScriptUpdate[];
extern const char kScriptRender[];
extern const char kScriptOnInputChar[];
extern const char kScriptOnKeyPressed[];
extern const char kScriptOnKeyReleased[];
extern const char kScriptGetStateWeight[];

// Factory "ScriptedObject": a CompoundObject whose update, render, input and
// state faces call members of its script instance. QI only hands out a face
// when the script defines the member behind it. The pointer handlers are the
// "OnPointer*Ex" members (with a pointer index) when any exists, else the
// plain "OnPointer*" ones, cached by LoadFromScript.
// SIZE 0xd8
// VTABLE: VC1 0x004d514c vc1::IUpdateable
// VTABLE: VC1 0x004d5164 vc1::IRenderable
// VTABLE: VC1 0x004d517c vc1::IInputHandler
// VTABLE: VC1 0x004d5190 vc1::IStateProvider
// VTABLE: VC1 0x004d51a4 vc1::IScriptObject
// VTABLE: VC1 0x004d51bc vc1::IScriptLoadable
// VTABLE: VC1 0x004d51cc vc1::IPositionable
// VTABLE: VC1 0x004d51e0 vc1::IIdentifiable
// VTABLE: VC1 0x004d51f8 vc1::IClassifiable
// VTABLE: VC1 0x004d5214 vc1::KdComponentGroup
// VTABLE: VC1 0x004d523c vc1::IAbstract
class KdScriptedObject : public IUpdateable,
                         public IRenderable,
                         public IInputHandler,
                         public IStateProvider,
                         public KdCompoundObject
{
public:
    Matrix3 position;                 // 0x80, used without a provider
    SquirrelObject pointerHandlers[4]; // 0xa4: pressed, released, moved, wheel
    bool pointerHandlersEx;           // 0xd4

    KdScriptedObject() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    int Shutdown();

    int GetUpdateLayer();
    int SetLayer(unsigned layer);
    void Update(unsigned sliceBit, int dt);
    int Render(void* graphics);
    int GetRenderOrder();
    unsigned char OnKey(const unsigned* data);
    unsigned char OnPointer(const unsigned* data);
    float GetStateWeight(const char* state);
    float GetStatePhase(const char* state);
};

typedef char Vc1KdScriptedObjectSizeCheck[sizeof(KdScriptedObject) == 0xd8 ? 1 : -1];

// SYNTHETIC: VC1 0x00416530
// vc1::KdScriptedObject::`scalar deleting destructor'

}

#endif
