#include "kd/world/game_object.h"

#include "kdcore/platform.h"
#include "kd/iface/iid.h"
#include "bindings/matrix3.h"

#include <new>
#include <stdlib.h>
#include <string.h>

// SetPosition calls the CRT memcpy for the old matrix, not an inline copy.
#pragma function(memcpy)

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004ed668
KdRef<IUnknown> g_kdGameObjectNullObject;
// GLOBAL: VC1 0x004e9640
KdString g_kdGameObjectEmptyName = KdEmptyText();
// GLOBAL: VC1 0x004ed66c
float g_kdGameObjectDefaultStateWeight = g_kdInfinity.value;
// GLOBAL: VC1 0x004ed670
float g_kdGameObjectDefaultStatePhase = g_kdInfinity.value;

// STRING: VC1 0x004cfdb0
static const char kGameObject[] = "GameObject";
// GLOBAL: VC1 0x004d0af0
const char* g_kdNameGameObject = kGameObject;

// MOV EAX, "GameObject"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00404340
const char* KdGameObject::GetTypeName()
{
    return kGameObject;
}

// RET 8. IScriptObject vtable+0x10: the script instance's SetMember (tail
// call).
// FUNCTION: VC1 0x004b08f0
unsigned char KdGameObject::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// ADD ECX, 0x5c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004b0900
unsigned KdGameObject::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004b0920
unsigned KdGameObject::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. IPositionable vtable+0x0c (ECX=this+0x18). Stores the position
// like KdScriptedObject::SetPosition, then moves every positionable child
// component except the position provider by the same transform:
// child = Inverse(old) * new * child (Matrix3 operator order).
// FUNCTION: VC1 0x004b0930
void KdGameObject::SetPosition(const Matrix3& value)
{
    SQFloat old[9];
    KdFactory* records;
    NamedRecord* record;

    memcpy(old, &GetPosition(), sizeof(old));
    if (positionProvider.ptr) {
        positionProvider->SetPosition(value);
    }
    else {
        position = value;
    }
    records = static_cast<KdFactory*>(static_cast<KdComponentGroup&>(*this).GetRecords());
    for (record = reinterpret_cast<NamedRecord*>(records->begin);
         record != reinterpret_cast<NamedRecord*>(
             reinterpret_cast<unsigned char*>(records->begin) + records->size);
         ++record) {
        IPositionable* child;

        if (!record->object.ptr) {
            continue;
        }
        child = static_cast<IPositionable*>(record->object->QueryInterface(&g_iidPositionable));
        if (!child) {
            continue;
        }
        child->AddRef();
        bool other = true;
        if (positionProvider.ptr) {
            void* const provider = positionProvider->QueryInterface(&g_iidAbstract);
            other = provider != child->QueryInterface(&g_iidAbstract);
        }
        if (other) {
            const Matrix3& current = child->GetPosition();
            child->SetPosition(Matrix3::Inverse(*reinterpret_cast<const Matrix3*>(old))
                * GetPosition() * current);
        }
        child->Release();
    }
}

// ECX=this. Only the vftables are rewritten before the ScriptedObject
// teardown.
// FUNCTION: VC1 0x004b0a80
KdGameObject::~KdGameObject() throw()
{
}

// EAX=this.
// FUNCTION: VC1 0x004b0b10
KdGameObject::KdGameObject() throw()
{
}

// FUNCTION: VC1 0x004b0b70
void** CreateKdGameObject(void** out)
{
    void* const memory = malloc(sizeof(KdGameObject));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdGameObject();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// KdComponentGroup vtable+0x0c: JMP to the ScriptedObject's.
// FUNCTION: VC1 0x004b0bd0
int KdGameObject::Shutdown()
{
    return KdScriptedObject::Shutdown();
}

// RET 4. IScriptLoadable vtable+0x0c.
// FUNCTION: VC1 0x004b0be0
bool KdGameObject::LoadFromScript(SquirrelObject& table)
{
    return KdScriptedObject::LoadFromScript(table) != 0;
}

// RET 8. The script instance's members, then KdScriptedObject::GetMember
// (inlined: the instance again, then the CompoundObject methods).
// FUNCTION: VC1 0x004b0c00
unsigned char KdGameObject::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    return KdScriptedObject::GetMember(name, out) != 0;
}

}
