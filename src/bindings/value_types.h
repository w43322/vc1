#ifndef VC1_BINDINGS_VALUE_TYPES_H
#define VC1_BINDINGS_VALUE_TYPES_H
#include "color.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix3.h"
#include "matrix4.h"
#include "quaternion.h"
#include "rect.h"
#include "rectf.h"

#include <new>

namespace vc1 {

struct NativeTypeInfo
{
    const void* spec;
    void* unused1;
    const SQChar* typeName;
    void* unused2;
    int extra;
};

// SqPlus RegisterClassType<T>; one instantiation per class, inlined into its
// RegisterClass.
inline void EnsureTypeInfo(NativeTypeInfo* type, unsigned* ready,
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

inline void BindNamedMethod(SquirrelObject& classObject, const SQChar* name,
    SQFUNCTION fn)
{
    SquirrelVM::CreateFunction(classObject, fn, name, 0);
}

template<typename T>
__forceinline SquirrelObject RegisterClassType(HSQUIRRELVM vm,
    const SQChar* className, NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName, SQFUNCTION ctor, SQFUNCTION typeOf)
{
    const SQInteger top = sq_gettop(vm);
    SquirrelObject newClass;
    EnsureTypeInfo(type, ready, spec, typeName);
    if (CreateClass(vm, newClass, type, className)) {
        BindNamedMethod(newClass, "constructor", ctor);
        BindNamedMethod(newClass, "_typeof", typeOf);
    }
    sq_settop(vm, top);
    return newClass;
}

// SqPlus SQClassDef<T> constructor and varargs "constructor" binding, inlined
// into each class's CreateBinding / BindArgConstructor.
__forceinline NativeClassBinding* CreateValueBinding(NativeClassBinding* self, const SQChar* name,
    SquirrelObject (*registerClass)(HSQUIRRELVM, const SQChar*))
{
    self->vm = g_vm;
    self->className = name;
    new (&self->obj) SquirrelObject;
    if (!self->className) {
        self->className = name;
    }
    self->obj = registerClass(self->vm, self->className);
    return self;
}

__forceinline NativeClassBinding* BindStarConstructor(NativeClassBinding* self, SQFUNCTION fn)
{
    SquirrelVM::CreateFunction(self->obj, fn, "constructor", "*");
    return self;
}

SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx);
SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx);

// 0x0041e3b0; original ABI: vm=EAX, nativeObject=[esp+4], releaseHook=ESI.
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook);

}

#endif
