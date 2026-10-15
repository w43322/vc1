#include "kd/scene/scripted_object.h"

#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "bindings/component.h"
#include "sqplus/SquirrelVM.h"

#include <limits>
#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004d4b68
static const char kScriptedObject[] = "ScriptedObject";
// GLOBAL: VC1 0x004d0688
const char* g_kdNameScriptedObject = kScriptedObject;

// GLOBAL: VC1 0x004d5034
extern const char kScriptUpdate[] = "Update";
// GLOBAL: VC1 0x004d503c
extern const char kScriptRender[] = "Render";
// GLOBAL: VC1 0x004d5044
extern const char kScriptOnInputChar[] = "OnInputChar";
// GLOBAL: VC1 0x004d5050
extern const char kScriptOnKeyPressed[] = "OnKeyPressed";
// GLOBAL: VC1 0x004d5060
extern const char kScriptOnKeyReleased[] = "OnKeyReleased";
// GLOBAL: VC1 0x004d5070
extern const char kScriptGetStateWeight[] = "GetStateWeight";
// Also read by 0x004a4222.
// GLOBAL: VC1 0x004d1e18
extern const char kScriptGetRenderLayer[] = "GetRenderLayer";

// Inferred names. Dynamic initializers 0x004c9430 / 0x004c9440 copy +Inf
// (the CRT's _FInf at 0x004e95cc): what a script without the member reports.
// GLOBAL: VC1 0x004ed6c8
float g_kdDefaultStateWeight = std::numeric_limits<float>::infinity();
// GLOBAL: VC1 0x004ed6cc
float g_kdDefaultStatePhase = std::numeric_limits<float>::infinity();

// The Component object a script member runs with as `this`: the counted
// IAbstract face (QI'd twice, as every caller inlines it), pushed through
// SquirrelObject::Set<KdPtr<IUnknown> > (0x004014a0).
static __forceinline SquirrelObject ScriptSelf(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return SquirrelObject((KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0)));
}

// MOV EAX, "ScriptedObject"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00416520
const char* KdScriptedObject::Script14()
{
    return kScriptedObject;
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call
// through its vftable), as in KdCompoundObject::Script10.
// FUNCTION: VC1 0x00470e40
unsigned char KdScriptedObject::Script10(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// IPositionable vtable+0x10. The provider's position, or the own matrix.
// FUNCTION: VC1 0x00470e50
const Matrix3& KdScriptedObject::GetPosition()
{
    if (positionProvider.ptr) {
        return positionProvider->GetPosition();
    }
    return position;
}

// RET 4. IPositionable vtable+0x0c. The provider, or the own matrix.
// FUNCTION: VC1 0x00470e70
void KdScriptedObject::SetPosition(const Matrix3& value)
{
    if (positionProvider.ptr) {
        positionProvider->SetPosition(value);
        return;
    }
    position = value;
}

// FUNCTION: VC1 0x00470ea0
unsigned KdScriptedObject::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x00470ec0
unsigned KdScriptedObject::AddRef()
{
    return IAbstract::AddRef();
}

// EDI=this. The matrix starts as identity, the handlers as null.
// FUNCTION: VC1 0x00471320
KdScriptedObject::KdScriptedObject() throw()
    : pointerHandlersEx(false)
{
}

// FUNCTION: VC1 0x00471430
void** CreateKdScriptedObject(void** out)
{
    KdScriptedObject* memory = static_cast<KdScriptedObject*>(malloc(sizeof(KdScriptedObject)));
    IUnknown* queried = 0;

    if (memory) {
        new (memory) KdScriptedObject();
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// STRING: VC1 0x004d513c
static const char kScriptGetStatePhase[] = "GetStatePhase";

// RET 4. IStateProvider vtable+0x10: script "GetStatePhase"(state).
// FUNCTION: VC1 0x00471490
float KdScriptedObject::GetStatePhase(const char* state)
{
    SquirrelObject member = script.GetValue(kScriptGetStatePhase);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return g_kdDefaultStatePhase;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(state);
    return SquirrelVM::EndCall().ToFloat();
}

// RET 4. IStateProvider vtable+0x0c: script "GetStateWeight"(state).
// FUNCTION: VC1 0x00471660
float KdScriptedObject::GetStateWeight(const char* state)
{
    SquirrelObject member = script.GetValue(kScriptGetStateWeight);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return g_kdDefaultStateWeight;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(state);
    return SquirrelVM::EndCall().ToFloat();
}

// RET 4. IInputHandler vtable+0x10, data = {code, button/wheel, x, y}.
// Pointer n's codes are 8 * n above pointer 0's; only pointer 0 without Ex
// handlers. 0x4001/0x4002 move, 0x4003 press (data[1] != 0) or release,
// 0x4004 wheel (data[1] = delta:16, value:16). The Ex handlers take the
// pointer index first. Non-zero when the handler returned non-zero.
// FUNCTION: VC1 0x00471830
unsigned char KdScriptedObject::OnPointer(const unsigned* data)
{
    int count = (pointerHandlersEx ? 3 : 0) + 1;
    unsigned code = data[0];
    int index;

    for (index = 0; index < count; ++index, code -= 8) {
        SquirrelObject* handler;

        switch (code) {
        case 0x4001:
        case 0x4002:
            handler = &pointerHandlers[2];
            break;
        case 0x4003:
            handler = data[1] ? &pointerHandlers[0] : &pointerHandlers[1];
            break;
        case 0x4004:
            handler = &pointerHandlers[3];
            break;
        default:
            handler = 0;
            break;
        }
        if (!handler) {
            continue;
        }
        if (handler->GetObjectHandle()._type == OT_NULL) {
            return 0;
        }
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(*handler, self);
        if (pointerHandlersEx) {
            SquirrelVM::PushParam(index);
        }
        if (code == 0x4003) {
            SquirrelVM::PushParam(static_cast<SQInteger>(data[1]));
        }
        else if (code == 0x4004) {
            SquirrelVM::PushParam(static_cast<SQInteger>(static_cast<short>(data[1] >> 16)));
            SquirrelVM::PushParam(static_cast<SQInteger>(data[1] & 0xffff));
        }
        SquirrelVM::PushParam(static_cast<SQInteger>(data[2]));
        SquirrelVM::PushParam(static_cast<SQInteger>(data[3]));
        return SquirrelVM::EndCall().ToBool();
    }
    return 0;
}

// RET 4. IInputHandler vtable+0x0c, data = {key, ?, kind, ?}: kind 0
// "OnKeyReleased", 1 "OnKeyPressed", 2 "OnInputChar". The second argument
// is the 64-bit data[2..3] shifted left by 32 (_allshl) and truncated.
// FUNCTION: VC1 0x00471a20
unsigned char KdScriptedObject::OnKey(const unsigned* data)
{
    const char* name;

    switch (data[2]) {
    case 0:
        name = kScriptOnKeyReleased;
        break;
    case 1:
        name = kScriptOnKeyPressed;
        break;
    case 2:
        name = kScriptOnInputChar;
        break;
    default:
        return 0;
    }
    SquirrelObject member = script.GetValue(name);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(static_cast<SQInteger>(data[0]));
    SquirrelVM::PushParam(static_cast<SQInteger>(
        *reinterpret_cast<const unsigned __int64*>(data + 2) << 32));
    return SquirrelVM::EndCall().ToBool();
}

// RET 4. IRenderable vtable+0x10: script "Render"(graphics), graphics as
// KdPtr<IGraphics> (SquirrelObject::Set, 0x00416550).
// FUNCTION: VC1 0x00471bd0
int KdScriptedObject::Render(void* graphics)
{
    SquirrelObject member = script.GetValue(kScriptRender);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(SquirrelObject(*static_cast<KdPtr<IGraphics>*>(graphics)));
    SquirrelVM::EndCall();
    return 0;
}

// IRenderable vtable+0x14: script "GetRenderLayer"(), or 0.
// FUNCTION: VC1 0x00471d60
int KdScriptedObject::Render14()
{
    SquirrelObject member = script.GetValue(kScriptGetRenderLayer);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    SquirrelObject self = ScriptSelf(this);
    return SquirrelVM::RunScript(member, &self).ToInteger();
}

// RET 8. IUpdateable vtable+0x14: script "Update"(sliceBit, dt).
// FUNCTION: VC1 0x00471ee0
void KdScriptedObject::Update(unsigned sliceBit, int dt)
{
    SquirrelObject member = script.GetValue(kScriptUpdate);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(static_cast<SQInteger>(sliceBit));
    SquirrelVM::PushParam(dt);
    SquirrelVM::EndCall();
}

// STRING: VC1 0x004d5134
static const char kScriptInit[] = "Init";

// RET 4. IUpdateable vtable+0x10: script "Init"(layer).
// FUNCTION: VC1 0x00472040
int KdScriptedObject::SetLayer(unsigned layer)
{
    SquirrelObject member = script.GetValue(kScriptInit);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    SquirrelObject self = ScriptSelf(this);
    SquirrelVM::BeginCall(member, self);
    SquirrelVM::PushParam(static_cast<SQInteger>(layer));
    SquirrelVM::EndCall();
    return 0;
}

// STRING: VC1 0x004d5124
static const char kScriptGetUpdateLayer[] = "GetUpdateLayer";

// IUpdateable vtable+0x0c: script "GetUpdateLayer"(), or 1.
// FUNCTION: VC1 0x00472190
int KdScriptedObject::Unmapped0c()
{
    SquirrelObject member = script.GetValue(kScriptGetUpdateLayer);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return 1;
    }
    SquirrelObject self = ScriptSelf(this);
    return SquirrelVM::RunScript(member, &self).ToInteger();
}

// STRING: VC1 0x004d5118
static const char kScriptShutdown[] = "Shutdown";

// KdComponentGroup vtable+0x0c: script "Shutdown"(), then the compound's.
// FUNCTION: VC1 0x00472310
int KdScriptedObject::Shutdown()
{
    SquirrelObject member = script.GetValue(kScriptShutdown);
    if (member.GetObjectHandle()._type != OT_NULL) {
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::EndCall();
    }
    KdCompoundObject::Shutdown();
    return 0;
}

// STRING: VC1 0x004d5080
static const char kScriptOnPointerPressedEx[] = "OnPointerPressedEx";
// STRING: VC1 0x004d5094
static const char kScriptOnPointerReleasedEx[] = "OnPointerReleasedEx";
// STRING: VC1 0x004d50a8
static const char kScriptOnPointerMovedEx[] = "OnPointerMovedEx";
// STRING: VC1 0x004d50bc
static const char kScriptOnPointerWheelEx[] = "OnPointerWheelEx";
// STRING: VC1 0x004d50d0
static const char kScriptOnPointerPressed[] = "OnPointerPressed";
// STRING: VC1 0x004d50e4
static const char kScriptOnPointerReleased[] = "OnPointerReleased";
// STRING: VC1 0x004d50f8
static const char kScriptOnPointerMoved[] = "OnPointerMoved";
// STRING: VC1 0x004d5108
static const char kScriptOnPointerWheel[] = "OnPointerWheel";

// RET 4. IScriptLoadable vtable+0x0c: the compound's load (owner = the
// IScriptObject face), then the pointer handlers, Ex if any is defined.
// FUNCTION: VC1 0x00472460
bool KdScriptedObject::LoadFromScript(SquirrelObject& table)
{
    if (!script.Load(table, static_cast<IScriptObject*>(this))) {
        return false;
    }
    pointerHandlers[0] = script.GetValue(kScriptOnPointerPressedEx);
    pointerHandlers[1] = script.GetValue(kScriptOnPointerReleasedEx);
    pointerHandlers[2] = script.GetValue(kScriptOnPointerMovedEx);
    pointerHandlers[3] = script.GetValue(kScriptOnPointerWheelEx);
    pointerHandlersEx = pointerHandlers[0].GetObjectHandle()._type != OT_NULL
        || pointerHandlers[1].GetObjectHandle()._type != OT_NULL
        || pointerHandlers[2].GetObjectHandle()._type != OT_NULL
        || pointerHandlers[3].GetObjectHandle()._type != OT_NULL;
    if (!pointerHandlersEx) {
        pointerHandlers[0] = script.GetValue(kScriptOnPointerPressed);
        pointerHandlers[1] = script.GetValue(kScriptOnPointerReleased);
        pointerHandlers[2] = script.GetValue(kScriptOnPointerMoved);
        pointerHandlers[3] = script.GetValue(kScriptOnPointerWheel);
    }
    return true;
}

// RET 8. The script instance's members, then the compound's table.
// FUNCTION: VC1 0x00472750
unsigned char KdScriptedObject::Script0c(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    return KdCompoundObject::Script0c(name, out) != 0;
}

}
