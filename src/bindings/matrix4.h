#ifndef VC1_BINDINGS_MATRIX4_H
#define VC1_BINDINGS_MATRIX4_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d7598, registered as "_typeof" (0x004d2f04).
// 16 floats, column-major identity at 0x0041ff90. Size-class pools[16] / 0x004ed498.
struct Matrix4
{
    SQFloat m[16];

    SQInteger Cmp(const Matrix4* other); // thiscall RET 4; bound as "_cmp"
    Matrix4* Unm(Matrix4* out); // thiscall RET 4; bound as "_unm" via 0x00427d20
    Matrix4* Div(Matrix4* out, SQFloat scalar); // thiscall RET 8; bound as "_div"
    Matrix4* Mul(Matrix4* out, const Matrix4* other); // thiscall RET 8; bound as "_mul"
    Matrix4* Sub(Matrix4* out, const Matrix4* other); // thiscall RET 8; bound as "_sub"
    Matrix4* Add(Matrix4* out, const Matrix4* other); // thiscall RET 8; bound as "_add"
    static void Transpose(Matrix4* out, const Matrix4* src); // cdecl; "Transpose"
    static void Inverse(Matrix4* out, const Matrix4* src); // cdecl; "Inverse"

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Matrix4SizeCheck[sizeof(Matrix4) == 64 ? 1 : -1];

// EAX=out, EDX=a, ECX=b. Column-major 4x4 product.
void MulMatrix4(float* out, const float* a, const float* b);

}

#endif
