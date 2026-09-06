#include "matrix4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_matrix4Type;
static unsigned g_matrix4TypeReady;
static const int kMatrix4TypeSpec = 0;

// Original writes 1 then zeros then diagonal 1s via FLD1/FLDZ (0x0041ffb4 / helper 0x00474090).
static void IdentityMatrix4(Matrix4* matrix)
{
    matrix->m[0] = 1.0f;
    matrix->m[1] = 0.0f;
    matrix->m[2] = 0.0f;
    matrix->m[3] = 0.0f;
    matrix->m[4] = 0.0f;
    matrix->m[5] = 1.0f;
    matrix->m[6] = 0.0f;
    matrix->m[7] = 0.0f;
    matrix->m[8] = 0.0f;
    matrix->m[9] = 0.0f;
    matrix->m[10] = 1.0f;
    matrix->m[11] = 0.0f;
    matrix->m[12] = 0.0f;
    matrix->m[13] = 0.0f;
    matrix->m[14] = 0.0f;
    matrix->m[15] = 1.0f;
}

// SQRELEASEHOOK; size unused. Puts 64 bytes back on pools[16] / 0x004ed498.
// FUNCTION: VC1 0x0041dd40
SQInteger Matrix4::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix4);
    }
    return 0;
}

// this in ESI. Name 0x004d7598.
// FUNCTION: VC1 0x0041e0e0
NativeClassBinding* Matrix4::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Matrix4", Matrix4::RegisterClass);
}

// FUNCTION: VC1 0x0041e9c0
NativeClassBinding* Matrix4::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Matrix4::ConstructArgs);
}

// Once-flag 0x004ed870, type info 0x004ed85c, spec 0x004d75c0, name 0x004d7598.
// FUNCTION: VC1 0x0041f6d0
SquirrelObject* Matrix4::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_matrix4Type, &g_matrix4TypeReady,
        &kMatrix4TypeSpec, "Matrix4", Matrix4::Constructor, Matrix4::TypeOf);
}

// Registered from 0x0041f6d0 as "constructor" (0x004d2f10) with null typemask.
// Writes column-major identity ([0],[20],[40],[60] = 1). Arg-taking factory is 0x004be3b0.
// FUNCTION: VC1 0x0041ff90
SQInteger Matrix4::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix4);
    Matrix4* const matrix = static_cast<Matrix4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Allocate());
    if (matrix) {
        IdentityMatrix4(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix4::ReleaseHook);
}

// FUNCTION: VC1 0x004236e0
SQInteger Matrix4::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Matrix4", -1);
    return 1;
}

// Bound as "_cmp" (0x004d36e0). this in EDX; other on stack; size 0x40.
// FUNCTION: VC1 0x00427c80
SQInteger Matrix4::Cmp(const Matrix4* other)
{
    return MemCompare(this, other, sizeof(Matrix4));
}

// Bound as "_unm" (0x004d36d0) via wrapper 0x00427d20 from 0x004c0fc1.
// Body 0x00427ca0 uses out in EAX; wrapper loads out and RET 4.
// FUNCTION: VC1 0x00427d20
Matrix4* Matrix4::Unm(Matrix4* out)
{
    out->m[0] = -m[0];
    out->m[1] = -m[1];
    out->m[2] = -m[2];
    out->m[3] = -m[3];
    out->m[4] = -m[4];
    out->m[5] = -m[5];
    out->m[6] = -m[6];
    out->m[7] = -m[7];
    out->m[8] = -m[8];
    out->m[9] = -m[9];
    out->m[10] = -m[10];
    out->m[11] = -m[11];
    out->m[12] = -m[12];
    out->m[13] = -m[13];
    out->m[14] = -m[14];
    out->m[15] = -m[15];
    return out;
}

// Bound as "_div" (0x004d36c8). Wrapper 0x00427d40: FLD scalar, EAX=out, PUSH this, CALL 0x00474cb0, RET 8.
// Helper 0x00474cb0: this in ECX, scalar on stack, out in EAX, writes (1/scalar)*this (16 floats).
// FUNCTION: VC1 0x00427d40
Matrix4* Matrix4::Div(Matrix4* out, SQFloat scalar)
{
    const SQFloat inv = 1.0f / scalar;
    out->m[0] = m[0] * inv;
    out->m[1] = m[1] * inv;
    out->m[2] = m[2] * inv;
    out->m[3] = m[3] * inv;
    out->m[4] = m[4] * inv;
    out->m[5] = m[5] * inv;
    out->m[6] = m[6] * inv;
    out->m[7] = m[7] * inv;
    out->m[8] = m[8] * inv;
    out->m[9] = m[9] * inv;
    out->m[10] = m[10] * inv;
    out->m[11] = m[11] * inv;
    out->m[12] = m[12] * inv;
    out->m[13] = m[13] * inv;
    out->m[14] = m[14] * inv;
    out->m[15] = m[15] * inv;
    return out;
}

// Bound as "_mul" (0x004d36c0). Wrapper 0x00427d60: EAX=out, EDX=this, ECX=other, CALL 0x00474e70, RET 8.
// Helper 0x00474e70: out = this * other, column-major 4x4.
// FUNCTION: VC1 0x00427d60
Matrix4* Matrix4::Mul(Matrix4* out, const Matrix4* other)
{
    const SQFloat* o = other->m;
    out->m[0] = m[0] * o[0] + m[4] * o[1] + m[8] * o[2] + m[12] * o[3];
    out->m[1] = m[1] * o[0] + m[5] * o[1] + m[9] * o[2] + m[13] * o[3];
    out->m[2] = m[2] * o[0] + m[6] * o[1] + m[10] * o[2] + m[14] * o[3];
    out->m[3] = m[3] * o[0] + m[7] * o[1] + m[11] * o[2] + m[15] * o[3];
    out->m[4] = m[0] * o[4] + m[4] * o[5] + m[8] * o[6] + m[12] * o[7];
    out->m[5] = m[1] * o[4] + m[5] * o[5] + m[9] * o[6] + m[13] * o[7];
    out->m[6] = m[2] * o[4] + m[6] * o[5] + m[10] * o[6] + m[14] * o[7];
    out->m[7] = m[3] * o[4] + m[7] * o[5] + m[11] * o[6] + m[15] * o[7];
    out->m[8] = m[0] * o[8] + m[4] * o[9] + m[8] * o[10] + m[12] * o[11];
    out->m[9] = m[1] * o[8] + m[5] * o[9] + m[9] * o[10] + m[13] * o[11];
    out->m[10] = m[2] * o[8] + m[6] * o[9] + m[10] * o[10] + m[14] * o[11];
    out->m[11] = m[3] * o[8] + m[7] * o[9] + m[11] * o[10] + m[15] * o[11];
    out->m[12] = m[0] * o[12] + m[4] * o[13] + m[8] * o[14] + m[12] * o[15];
    out->m[13] = m[1] * o[12] + m[5] * o[13] + m[9] * o[14] + m[13] * o[15];
    out->m[14] = m[2] * o[12] + m[6] * o[13] + m[10] * o[14] + m[14] * o[15];
    out->m[15] = m[3] * o[12] + m[7] * o[13] + m[11] * o[14] + m[15] * o[15];
    return out;
}

// Bound as "_sub" (0x004d36b8) from 0x004c0f7a. Wrapper 0x00427d80 calls 0x00474d50.
// Helper does this-other (param_2 - param_1 with EDX=this, ECX=other). RET 8.
// FUNCTION: VC1 0x00427d80
Matrix4* Matrix4::Sub(Matrix4* out, const Matrix4* other)
{
    out->m[0] = m[0] - other->m[0];
    out->m[1] = m[1] - other->m[1];
    out->m[2] = m[2] - other->m[2];
    out->m[3] = m[3] - other->m[3];
    out->m[4] = m[4] - other->m[4];
    out->m[5] = m[5] - other->m[5];
    out->m[6] = m[6] - other->m[6];
    out->m[7] = m[7] - other->m[7];
    out->m[8] = m[8] - other->m[8];
    out->m[9] = m[9] - other->m[9];
    out->m[10] = m[10] - other->m[10];
    out->m[11] = m[11] - other->m[11];
    out->m[12] = m[12] - other->m[12];
    out->m[13] = m[13] - other->m[13];
    out->m[14] = m[14] - other->m[14];
    out->m[15] = m[15] - other->m[15];
    return out;
}

// Bound as "_add" (0x004d36b0) from 0x004c0f5e. Wrapper 0x00427da0 calls 0x00474de0. RET 8.
// FUNCTION: VC1 0x00427da0
Matrix4* Matrix4::Add(Matrix4* out, const Matrix4* other)
{
    out->m[0] = m[0] + other->m[0];
    out->m[1] = m[1] + other->m[1];
    out->m[2] = m[2] + other->m[2];
    out->m[3] = m[3] + other->m[3];
    out->m[4] = m[4] + other->m[4];
    out->m[5] = m[5] + other->m[5];
    out->m[6] = m[6] + other->m[6];
    out->m[7] = m[7] + other->m[7];
    out->m[8] = m[8] + other->m[8];
    out->m[9] = m[9] + other->m[9];
    out->m[10] = m[10] + other->m[10];
    out->m[11] = m[11] + other->m[11];
    out->m[12] = m[12] + other->m[12];
    out->m[13] = m[13] + other->m[13];
    out->m[14] = m[14] + other->m[14];
    out->m[15] = m[15] + other->m[15];
    return out;
}

// Registered from 0x0041e9c0 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; switch cases 1,2,3,5 so extra 3 is invalid. Error string 0x004d6a80.
// FUNCTION: VC1 0x004be3b0
SQInteger Matrix4::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix4);
    Matrix4* const matrix = static_cast<Matrix4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra == 3 || extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix4))]->Release(matrix);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix4);
        return sq_throwerror(vm, "constructMatrix4: invalid arguments");
    }
    if (matrix) {
        IdentityMatrix4(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix4::ReleaseHook);
}

}
