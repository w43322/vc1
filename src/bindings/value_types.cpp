#include "value_types.h"

#include "memory/fixed_pool.h"

namespace vc1 {

static void EnsureTypeInfo(NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName)
{
    if ((*ready & 1) == 0) {
        *ready |= 1;
        type->unused1 = 0;
        type->unused2 = 0;
        type->extra = -1;
        type->spec = spec;
        type->typeName = typeName;
    }
}

static void BindNamedMethod(SquirrelObject* classObject, const SQChar* name,
    SQFUNCTION fn)
{
    SquirrelObject slot;
    sq_pushobject(g_vm, classObject->obj);
    RegisterNativeSlot(&slot, name, fn, 0);
    sq_pop(g_vm, 1);
    sq_release(g_vm, &slot.obj);
}

SquirrelObject* RegisterValueClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className, NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName, SQFUNCTION ctor, SQFUNCTION typeOf)
{
    const SQInteger savedTop = sq_gettop(vm);
    InitSquirrelObject(out);
    EnsureTypeInfo(type, ready, spec, typeName);
    if (CreateNativeClass(vm, out, type, className)) {
        BindNamedMethod(out, "constructor", ctor);
        BindNamedMethod(out, "_typeof", typeOf);
    }
    sq_settop(vm, savedTop);
    return out;
}

NativeClassBinding* CreateValueBinding(NativeClassBinding* self, const SQChar* name,
    SquirrelObject* (*registerClass)(SquirrelObject*, HSQUIRRELVM, const SQChar*))
{
    SquirrelObject created;
    self->vm = g_vm;
    self->className = name;
    InitSquirrelObject(&self->obj);
    if (!self->className) {
        self->className = name;
    }
    registerClass(&created, self->vm, self->className);
    AssignSquirrelObject(&self->obj, &created);
    sq_release(g_vm, &created.obj);
    return self;
}

NativeClassBinding* BindStarConstructor(NativeClassBinding* self, SQFUNCTION fn)
{
    SquirrelObject slot;
    sq_pushobject(g_vm, self->obj.obj);
    RegisterNativeSlot(&slot, "constructor", fn, "*");
    sq_release(g_vm, &slot.obj);
    sq_pop(g_vm, 1);
    return self;
}

SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx)
{
    SQFloat value = 0.0f;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getfloat(vm, idx, &value);
    }
    return value;
}

SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx)
{
    SQInteger value = 0;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getinteger(vm, idx, &value);
    }
    return value;
}

// FUNCTION: VC1 0x0041e3b0
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook)
{
    sq_setinstanceup(vm, 1, nativeObject);
    sq_setreleasehook(vm, 1, releaseHook);
    return 1;
}

}
