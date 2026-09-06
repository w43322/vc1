#ifndef VC1_BINDINGS_VALUE_TYPES_H
#define VC1_BINDINGS_VALUE_TYPES_H
#include "color.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

namespace vc1 {

struct NativeTypeInfo
{
    const void* spec;
    void* unused1;
    const SQChar* typeName;
    void* unused2;
    int extra;
};

// Shared helpers used by Color/Vector RegisterClass, CreateBinding, BindArgConstructor.
SquirrelObject* RegisterValueClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className, NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName, SQFUNCTION ctor, SQFUNCTION typeOf);
NativeClassBinding* CreateValueBinding(NativeClassBinding* self, const SQChar* name,
    SquirrelObject* (*registerClass)(SquirrelObject*, HSQUIRRELVM, const SQChar*));
NativeClassBinding* BindStarConstructor(NativeClassBinding* self, SQFUNCTION fn);
SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx);
SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx);

// 0x0041e3b0; original ABI: vm=EAX, nativeObject=[esp+4], releaseHook=ESI.
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook);

}

#endif
