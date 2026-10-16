#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "kd/component_group.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_group.h"

namespace vc1 {

// GLOBAL: VC1 0x004ee3d8
unsigned char* g_componentCursor;

// STRING: VC1 0x004d69b8
static const char kComponentFormat[] = "(component : %p)";

// cdecl (up, size). Release hook that Push (0x0040c970) attaches to an
// engine object's instance.
// FUNCTION: VC1 0x00401c10
SQInteger ReleaseComponent(SQUserPointer p, SQInteger size)
{
    (void)size;
    if (p) {
        static_cast<IUnknown*>(p)->Release();
    }
    return 0;
}

}

namespace SqPlus {

// EAX=&value, EDI=vm. A null pointer pushes null. Otherwise a native
// "Component" instance holds the object's IAbstract face, AddRef'd after the
// instance exists. On failure SQPLUS_THROW logs "error: %s\n" (0x004cfa98)
// with 0x004cfac4.
// The same body for the IScriptObject face (KdScriptInstance::Load's `this`).
// FUNCTION: VC1 0x00403de0
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IScriptObject>& value)
{
    vc1::IAbstract* face;

    if (!value.ptr) {
        sq_pushnull(v);
        return;
    }
    face = static_cast<vc1::IAbstract*>(value->QueryInterface(&vc1::g_iidAbstract));
    if (!CreateNativeClassInstance(v, GetTypeName(*face), face, vc1::ReleaseComponent)) {
        SQPLUS_THROW(_SC("Push(): could not create INSTANCE (check registration name)"));
        return;
    }
    face->AddRef();
}

}

namespace vc1 {

// ESI=vm, stack &value. A script array of the list's objects, each pushed
// like Push(KdPtr<IUnknown>&) (null slots as null). A failed instance still
// reaches the sq_set. Returned by GameLevel "FindObjectsByMask".
// FUNCTION: VC1 0x00404070
void Push(HSQUIRRELVM v, KdRefList& value)
{
    unsigned i;

    sq_newarray(v, value.items.size >> 2);
    for (i = 0; i < value.items.size >> 2; ++i) {
        IUnknown* const object = reinterpret_cast<IUnknown**>(value.items.begin)[i];

        sq_pushinteger(v, static_cast<SQInteger>(i));
        if (object) {
            IAbstract* const face = static_cast<IAbstract*>(object->QueryInterface(&g_iidAbstract));
            if (!CreateNativeClassInstance(v, SqPlus::GetTypeName(*face), face,
                    ReleaseComponent)) {
                SQPLUS_THROW(_SC("Push(): could not create INSTANCE (check registration name)"));
            }
            else {
                face->AddRef();
            }
        }
        else {
            sq_pushnull(v);
        }
        sq_set(v, -3);
    }
}

}

namespace SqPlus {

// FUNCTION: VC1 0x0040c970
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IUnknown>& value)
{
    vc1::IAbstract* face;

    if (!value.ptr) {
        sq_pushnull(v);
        return;
    }
    face = static_cast<vc1::IAbstract*>(value->QueryInterface(&vc1::g_iidAbstract));
    if (!CreateNativeClassInstance(v, GetTypeName(*face), face, vc1::ReleaseComponent)) {
        SQPLUS_THROW(_SC("Push(): could not create INSTANCE (check registration name)"));
        return;
    }
    face->AddRef();
}

// EAX=&value. The same body for a graphics argument (ScriptedObject's
// "Render").
// FUNCTION: VC1 0x004165b0
template<> void Push(HSQUIRRELVM v, vc1::KdPtr<vc1::IGraphics>& value)
{
    vc1::IAbstract* face;

    if (!value.ptr) {
        sq_pushnull(v);
        return;
    }
    face = static_cast<vc1::IAbstract*>(value->QueryInterface(&vc1::g_iidAbstract));
    if (!CreateNativeClassInstance(v, GetTypeName(*face), face, vc1::ReleaseComponent)) {
        SQPLUS_THROW(_SC("Push(): could not create INSTANCE (check registration name)"));
        return;
    }
    face->AddRef();
}

}

namespace vc1 {

// ECX=key (null reads as ""). Whether the _nexti cursor's record name equals
// key; length first, then MemCompare (0x00499d00).
// FUNCTION: VC1 0x004b8ab0
bool ComponentCursorIs(const char* key)
{
    unsigned length;

    if (!key) {
        key = "";
    }
    length = static_cast<unsigned>(strlen(key));
    if (*reinterpret_cast<unsigned*>(g_componentCursor + 8) == length
        && MemCompare(key, *reinterpret_cast<const char**>(g_componentCursor + 4), length) == 0) {
        return true;
    }
    return false;
}

// "_cloned": the clone (stack 1) takes the source's userpointer (stack 2) as
// is; sq_setinstanceup is inlined and its error (0x004d2608) is ignored.
// FUNCTION: VC1 0x004bf070
SQInteger ComponentCloned(HSQUIRRELVM vm)
{
    IUnknown* source = SqPlus::GetInstance<IUnknown, false>(vm, 2);
    if (!source) {
        return -1;
    }
    sq_setinstanceup(vm, 1, source);
    return 1;
}

// "_tostring": "(component : %p)" in a 0x80-byte buffer through FormatBuffer
// (0x004969c0), pushed on SquirrelVM::_VM rather than vm.
// FUNCTION: VC1 0x004bf0c0
SQInteger ComponentToString(HSQUIRRELVM vm)
{
    char text[0x80];
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    if (!self) {
        return -1;
    }
    FormatBuffer(text, sizeof(text), kComponentFormat, self);
    sq_pushstring(SquirrelVM::_VM, text, -1);
    return 1;
}

// "_cmp": pushes the difference of the two userpointers in IUnknown* units
// (other - self); sq_pushinteger is inlined.
// FUNCTION: VC1 0x004bf120
SQInteger ComponentCmp(HSQUIRRELVM vm)
{
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    IUnknown* other = SqPlus::GetInstance<IUnknown, false>(vm, 2);
    sq_pushinteger(vm, other - self);
    return 1;
}

// "_typeof": the IScriptObject face's type name (vtable+0x14), pushed on
// SquirrelVM::_VM.
// FUNCTION: VC1 0x004bf190
SQInteger ComponentTypeOf(HSQUIRRELVM vm)
{
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    IScriptObject* script;

    if (!self) {
        return -1;
    }
    script = static_cast<IScriptObject*>(self->QueryInterface(&g_iidScriptObject));
    if (!script) {
        return -1;
    }
    sq_pushstring(SquirrelVM::_VM, script->GetTypeName(), -1);
    return 1;
}

// "_nexti" over the IComponentGroup name table (vtable+0x20). A null key
// restarts at the first record; the key under the cursor steps once;
// otherwise the cursor goes one past FindCountedRecord (0x00416250). Pushes
// the record name, or null at the end.
// FUNCTION: VC1 0x004bf1e0
SQInteger ComponentNextI(HSQUIRRELVM vm)
{
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    IComponentGroup* group;
    KdFactory* records;
    const char* key;
    const char* name;

    if (!self) {
        return -1;
    }
    group = static_cast<IComponentGroup*>(self->QueryInterface(&g_iidComponentGroup));
    if (!group) {
        return -1;
    }
    StackHandler sa(vm);
    key = sa.GetType(2) == OT_NULL ? 0 : sa.GetString(2);
    records = static_cast<KdFactory*>(group->GetRecords());
    if (!key) {
        g_componentCursor = reinterpret_cast<unsigned char*>(records->begin);
    }
    else if (g_componentCursor && ComponentCursorIs(key)) {
        g_componentCursor += 0x14;
    }
    else {
        KdString lookup(key);
        g_componentCursor = FindCountedRecord(records, &lookup) + 0x14;
    }
    name = g_componentCursor == reinterpret_cast<unsigned char*>(records->begin) + records->size
        ? 0
        : *reinterpret_cast<const char**>(g_componentCursor + 4);
    sq_pushstring(vm, name, -1);
    return 1;
}

// "_set": IScriptObject::SetMember(key, value) first. Otherwise the group
// removes key (vtable+0x14) and re-adds it (vtable+0x10) with the value's
// IAbstract face (SquirrelObject::Get 0x0041b830). Returns -1 for neither.
// FUNCTION: VC1 0x004bf2f0
SQInteger ComponentSet(HSQUIRRELVM vm)
{
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    IScriptObject* script;
    IComponentGroup* group;
    const char* key;
    HSQOBJECT handle;

    if (!self) {
        return -1;
    }
    StackHandler sa(vm);
    key = sa.GetString(2);
    sq_resetobject(&handle);
    if (sa.GetParamCount() >= 3) {
        sq_getstackobj(vm, 3, &handle);
    }
    SquirrelObject value(handle);
    script = static_cast<IScriptObject*>(self->QueryInterface(&g_iidScriptObject));
    if (script && script->SetMember(const_cast<char*>(key), &value)) {
        return 0;
    }
    group = static_cast<IComponentGroup*>(self->QueryInterface(&g_iidComponentGroup));
    if (!group) {
        return -1;
    }
    {
        KdString name(key);
        group->UnregisterObject(name);
    }
    {
        KdString name(key);
        KdPtr<IAbstract> object = value.Get<KdPtr<IAbstract> >();
        group->RegisterObject(name, object);
    }
    return 0;
}

// "_get": IScriptObject::GetMember(key, &out) first. Otherwise the group's
// lookup (vtable+0x18) wrapped as a Component instance (SquirrelObject::Set
// 0x004014a0). Returns -1 when neither has key.
// FUNCTION: VC1 0x004bf490
SQInteger ComponentGet(HSQUIRRELVM vm)
{
    IUnknown* self = SqPlus::GetInstance<IUnknown, false>(vm, 1);
    IScriptObject* script;
    IComponentGroup* group;
    const char* key;

    if (!self) {
        return -1;
    }
    StackHandler sa(vm);
    key = sa.GetString(2);
    script = static_cast<IScriptObject*>(self->QueryInterface(&g_iidScriptObject));
    if (script) {
        SquirrelObject out;
        if (script->GetMember(key, &out)) {
            sq_pushobject(vm, out.GetObjectHandle());
            return 1;
        }
    }
    group = static_cast<IComponentGroup*>(self->QueryInterface(&g_iidComponentGroup));
    if (!group) {
        return -1;
    }
    KdPtr<IUnknown> child = group->FindObject(KdString(key));
    if (!child.ptr) {
        return -1;
    }
    SquirrelObject result;
    result.Set(child);
    sq_pushobject(vm, result.GetObjectHandle());
    return 1;
}

}
