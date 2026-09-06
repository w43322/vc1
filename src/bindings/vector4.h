#ifndef VC1_BINDINGS_VECTOR4_H
#define VC1_BINDINGS_VECTOR4_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d7590, registered as "_typeof" (0x004d2f04).
struct Vector4
{
    SQFloat x;
    SQFloat y;
    SQFloat z;
    SQFloat w;

    SQInteger Cmp(const Vector4* other); // thiscall RET 4; bound as "_cmp"
    Vector4* Unm(Vector4* out); // thiscall RET 4; bound as "_unm"
    Vector4* Div(Vector4* out, SQFloat scalar); // thiscall RET 8; bound as "_div"
    Vector4* Mul(Vector4* out, SQFloat scalar); // thiscall RET 8; bound as "_mul"
    SQFloat GetSquareMagnitude();
    SQFloat GetMagnitude();
    Vector4* GetNormalized(Vector4* out); // thiscall RET 4

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Vector4SizeCheck[sizeof(Vector4) == 16 ? 1 : -1];

}

#endif
