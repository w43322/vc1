#include "kd/script_instance.h"

#include "bindings/component.h"
#include "kd/iface/iid.h"
#include "kd/iface/iscript_object.h"

namespace vc1 {

// XOR EAX,EAX; RET 4. ICF-folded "answer no IID".
// FUNCTION: VC1 0x004059e0 FOLDED
void* KdScriptInstance::QueryInterface(const char** iid)
{
    (void)iid;
    return 0;
}

// XOR EAX,EAX; RET. ICF-folded return-0.
// FUNCTION: VC1 0x0041adb0 FOLDED
unsigned KdScriptInstance::AddRef()
{
    return 0;
}

// FUNCTION: VC1 0x0041adb0 FOLDED
unsigned KdScriptInstance::Release()
{
    return 0;
}

// ECX=this, EDX=name, ESI=hidden return. The member, or null.
// FUNCTION: VC1 0x004b6d40
SquirrelObject IScriptInstance::GetValue(const char* name)
{
    SquirrelObject value;
    GetMember(name, value);
    return value;
}

// RET 8. SquirrelObject::GetSlot with sq_get inlined, then the slot value.
// FUNCTION: VC1 0x004b6d60
unsigned char KdScriptInstance::GetMember(const char* name, SquirrelObject& out)
{
    HSQUIRRELVM v;

    if (instance.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    v = SquirrelVM::_VM;
    sq_pushobject(v, instance.GetObjectHandle());
    sq_pushstring(v, name, -1);
    if (SQ_FAILED(sq_get(v, -2))) {
        sq_pop(v, 1);
        return 0;
    }
    out.AttachToStackObject(-1);
    sq_pop(v, 2);
    return 1;
}

// IScriptInstanceReset vtable+0x0c (this = the +4 face). Drops the instance
// and the owner.
// FUNCTION: VC1 0x004b6e20
void KdScriptInstance::Reset()
{
    instance.Reset();
    owner = 0;
}

// RET 8. SquirrelObject::SetValue(key, value) on the instance.
// FUNCTION: VC1 0x004b6e60
unsigned char KdScriptInstance::SetMember(const char* name, const SquirrelObject& value)
{
    if (instance.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    return instance.SetValue(name, value) != 0;
}

// FUNCTION: VC1 0x004b6e90
const char* KdScriptInstance::GetTypeName()
{
    if (instance.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    return instance.GetTypeName();
}

// STRING: VC1 0x004d2f10
static const char kScriptConstructor[] = "constructor";

// EAX=script, RET 4. A class is instantiated (SquirrelVM::CreateInstance),
// anything else is used as is. The instance's "constructor" then runs with
// the owner's Component object as `this`, not the instance: script code
// reaches its own members through the owner's _get / _set.
// FUNCTION: VC1 0x004b6eb0
unsigned char KdScriptInstance::Load(const SquirrelObject& script, IUnknown* object)
{
    if (const_cast<SquirrelObject&>(script).GetObjectHandle()._type == OT_CLASS) {
        instance = SquirrelVM::CreateInstance(const_cast<SquirrelObject&>(script));
    }
    else {
        instance = script;
    }
    if (instance.GetObjectHandle()._type == OT_NULL) {
        return 0;
    }
    owner = object;
    SquirrelObject constructor;
    GetMember(kScriptConstructor, constructor);
    if (constructor.GetObjectHandle()._type != OT_NULL) {
        IScriptObject* face = owner
            ? static_cast<IScriptObject*>(owner->QueryInterface(&g_iidScriptObject))
            : 0;
        SquirrelObject self((KdPtr<IScriptObject>(face)));
        SquirrelVM::RunScript(constructor, &self);
    }
    return 1;
}

}
