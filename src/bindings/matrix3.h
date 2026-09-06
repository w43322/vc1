#ifndef VC1_BINDINGS_MATRIX3_H
#define VC1_BINDINGS_MATRIX3_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d1920, registered as "_typeof" (0x004d2f04).
// 9 floats, column-major identity at 0x0041ff30. Size-class pools[9] / 0x004ed47c.
struct Matrix3
{
    SQFloat m[9];

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Matrix3SizeCheck[sizeof(Matrix3) == 36 ? 1 : -1];

}

#endif
