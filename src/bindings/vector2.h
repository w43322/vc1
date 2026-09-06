#ifndef VC1_BINDINGS_VECTOR2_H
#define VC1_BINDINGS_VECTOR2_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004cfb44, registered as "_typeof" (0x004d2f04).
struct Vector2
{
    SQFloat x;
    SQFloat y;

    SQInteger Cmp(const Vector2* other); // thiscall RET 4; bound as "_cmp"
    SQBool Equal(const Vector2* other); // thiscall RET 4; bound as "Equal"
    Vector2* Unm(Vector2* out); // thiscall RET 4; bound as "_unm"
    Vector2* Div(Vector2* out, SQFloat scalar); // thiscall RET 8; bound as "_div"
    Vector2* Mul(Vector2* out, SQFloat scalar); // thiscall RET 8; bound as "_mul"
    Vector2* Sub(Vector2* out, const Vector2* other); // thiscall RET 8; bound as "_sub"
    Vector2* Add(Vector2* out, const Vector2* other); // thiscall RET 8; bound as "_add"
    SQFloat GetSquareMagnitude();
    SQFloat GetMagnitude();
    Vector2* GetNormalized(Vector2* out); // thiscall RET 4
    void Normalize(); // thiscall; bound as "Normalize"
    static SQFloat GetDotProduct(const Vector2* a, const Vector2* b); // cdecl; "GetDotProduct"
    SQBool IsValid(); // thiscall; bound as "IsValid" (0x004d72cc)

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Vector2SizeCheck[sizeof(Vector2) == 8 ? 1 : -1];

}

#endif
