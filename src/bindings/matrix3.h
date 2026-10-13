#ifndef VC1_BINDINGS_MATRIX3_H
#define VC1_BINDINGS_MATRIX3_H
#include "squirrel_object.h"
#include "vector2.h"
#include "vector3.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d1920, registered as "_typeof" (0x004d2f04).
// 9 floats, column-major identity at 0x0041ff30. Size-class pools[9] / 0x004ed47c.
// 2D affine: X=(m[0],m[1]), Y=(m[3],m[4]), origin=(m[6],m[7]).
struct Matrix3
{
    SQFloat m[9];

    Vector2* GetOrigin(); // thiscall; bound as "GetOrigin" (0x004d732c)
    void SetOrigin(const Vector2* origin); // thiscall RET 4; "SetOrigin" (0x004d7348)
    Vector2* GetYVec(); // thiscall; bound as "GetYVec" (0x004d7324)
    Vector2* GetXVec(); // thiscall; bound as "GetXVec" (0x004d731c)
    void SetYVec(const Vector2* axis); // thiscall RET 4; "SetYVec" (0x004d7340)
    void SetXVec(const Vector2* axis); // thiscall RET 4; "SetXVec" (0x004d7338)
    void MultVecs(const Vector2* scale); // thiscall RET 4; "MultVecs" (0x004d7380)
    Vector2* MultInvVrt(Vector2* out, const Vector2* v); // thiscall RET 8; "MultInvVrt"
    Matrix3* GetNormalized(Matrix3* out); // thiscall RET 4; "GetNormalized" (0x004d7248)
    void Normalize(); // thiscall; unit X/Y, m[2]=m[5]=0, m[8]=1
    SQInteger Cmp(const Matrix3* other); // thiscall RET 4; bound as "_cmp"
    Matrix3* Unm(Matrix3* out); // thiscall RET 4; bound as "_unm"
    Matrix3* Div(Matrix3* out, SQFloat scalar); // thiscall RET 8; bound as "_div"
    Matrix3* Mul(Matrix3* out, const Matrix3* other); // thiscall RET 8; bound as "_mul"
    Matrix3* Sub(Matrix3* out, const Matrix3* other); // thiscall RET 8; bound as "_sub"
    Matrix3* Add(Matrix3* out, const Matrix3* other); // thiscall RET 8; bound as "_add"
    Vector3* Mult(Vector3* out, const Vector3* v); // thiscall RET 8; bound as "Mult"
    Vector2* MultInvVect(Vector2* out, const Vector2* v); // thiscall RET 8; "MultInvVect"
    Vector2* MultVect(Vector2* out, const Vector2* v); // thiscall RET 8; "MultVect"
    Matrix3* LeftMultiply(const Matrix3* left); // this = left * this, in place
    static void Transpose(Matrix3* out, const Matrix3* src); // cdecl; "Transpose"
    static void Inverse(Matrix3* out, const Matrix3* src); // cdecl; "Inverse"

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

extern Matrix3 g_matrix3Identity;

}

#endif
