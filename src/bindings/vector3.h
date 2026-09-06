#ifndef VC1_BINDINGS_VECTOR3_H
#define VC1_BINDINGS_VECTOR3_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d7588, registered as "_typeof" (0x004d2f04).
struct Vector3
{
    SQFloat x;
    SQFloat y;
    SQFloat z;

    SQInteger Cmp(const Vector3* other); // thiscall RET 4; bound as "_cmp"
    Vector3* Unm(Vector3* out); // thiscall RET 4; bound as "_unm"
    Vector3* Div(Vector3* out, SQFloat scalar); // thiscall RET 8; bound as "_div"
    Vector3* Mul(Vector3* out, SQFloat scalar); // thiscall RET 8; bound as "_mul"
    SQFloat GetSquareMagnitude();
    SQFloat GetMagnitude();
    Vector3* GetNormalized(Vector3* out); // thiscall RET 4

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Vector3SizeCheck[sizeof(Vector3) == 12 ? 1 : -1];

}

#endif
