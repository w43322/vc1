#ifndef VC1_BINDINGS_QUATERNION_H
#define VC1_BINDINGS_QUATERNION_H
#include "squirrel_object.h"
#include "matrix4.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d75a0, registered as "_typeof" (0x004d2f04).
// 16 bytes; default ctor zeros xyzw. Release hook is Vector4::ReleaseHook 0x0041dd60.
struct Quaternion
{
    SQFloat x;
    SQFloat y;
    SQFloat z;
    SQFloat w;

    static void Inverse(Quaternion* out, const Quaternion* src); // cdecl; "Inverse"
    Quaternion* GetNormalized(Quaternion* out); // thiscall RET 4; "GetNormalized"
    SQFloat GetSquareMagnitude(); // thiscall; "GetSquareMagnitude"
    SQFloat GetMagnitude(); // thiscall; "GetMagnitude"
    Quaternion* Unm(Quaternion* out); // thiscall RET 4; "_unm" conjugates xyz, keeps w
    Quaternion* Div(Quaternion* out, SQFloat scalar); // thiscall RET 8; "_div"
    Quaternion* Mul(Quaternion* out, SQFloat scalar); // thiscall RET 8; "_mul" (scalar)
    Quaternion* Sub(Quaternion* out, const Quaternion* other); // thiscall RET 8; "_sub"
    Quaternion* Add(Quaternion* out, const Quaternion* other); // thiscall RET 8; "_add"
    void FromMatrix(const Matrix4* matrix); // thiscall; "FromMatrix"
    void ToMatrix(Matrix4* out) const; // thiscall; "ToMatrix"
    void Slerp(const Quaternion* a, const Quaternion* b, SQFloat t); // thiscall; "Slerp"

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject RegisterClass(HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1QuaternionSizeCheck[sizeof(Quaternion) == 16 ? 1 : -1];

}

#endif
