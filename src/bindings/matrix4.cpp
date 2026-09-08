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
// FUNCTION: VC1 0x00427d60
Matrix4* Matrix4::Mul(Matrix4* out, const Matrix4* other)
{
    MulMatrix4(out->m, m, other->m);
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

// Bound as "Transpose" (0x004d739c) via 0x00422600 from 0x004c11c7.
// cdecl; out=[esp+4], src=[esp+8]. Column-major 4x4.
// FUNCTION: VC1 0x00474550
void Matrix4::Transpose(Matrix4* out, const Matrix4* src)
{
    out->m[0] = src->m[0];
    out->m[1] = src->m[4];
    out->m[2] = src->m[8];
    out->m[3] = src->m[12];
    out->m[4] = src->m[1];
    out->m[5] = src->m[5];
    out->m[6] = src->m[9];
    out->m[7] = src->m[13];
    out->m[8] = src->m[2];
    out->m[9] = src->m[6];
    out->m[10] = src->m[10];
    out->m[11] = src->m[14];
    out->m[12] = src->m[3];
    out->m[13] = src->m[7];
    out->m[14] = src->m[11];
    out->m[15] = src->m[15];
}

// Bound as "Inverse" (0x004d7394) via 0x00422600 from 0x004c11ab.
// cdecl; out=[esp+4], src=[esp+8]. Cofactor expansion matching FUN_004745c0.
// FUNCTION: VC1 0x004745c0
void Matrix4::Inverse(Matrix4* out, const Matrix4* src)
{
    const SQFloat m10 = src->m[1];
    const SQFloat m20 = src->m[2];
    const SQFloat m30 = src->m[3];
    const SQFloat m11 = src->m[5];
    const SQFloat m21 = src->m[6];
    const SQFloat m31 = src->m[7];
    const SQFloat m12 = src->m[9];
    const SQFloat m22 = src->m[10];
    const SQFloat m32 = src->m[11];
    const SQFloat m13 = src->m[13];
    const SQFloat m23 = src->m[14];
    const SQFloat m33 = src->m[15];
    const SQFloat s0 = m22 * m33 - m32 * m23;
    const SQFloat s1 = m31 * m23 - m33 * m21;
    const SQFloat s2 = m32 * m21 - m22 * m31;
    const SQFloat c0 = m13 * s2 + m12 * s1 + m11 * s0;
    const SQFloat t0 = m32 * m23 - m22 * m33;
    const SQFloat t1 = m20 * m33 - m30 * m23;
    const SQFloat t2 = m30 * m22 - m20 * m32;
    const SQFloat c1 = m10 * t0 + m12 * t1 + m13 * t2;
    const SQFloat u0 = m33 * m21 - m31 * m23;
    const SQFloat u1 = m30 * m23 - m20 * m33;
    const SQFloat u2 = m31 * m20 - m21 * m30;
    const SQFloat c2 = m11 * u1 + m10 * u0 + m13 * u2;
    const SQFloat v0 = m22 * m31 - m32 * m21;
    const SQFloat v1 = m20 * m32 - m30 * m22;
    const SQFloat v2 = m21 * m30 - m31 * m20;
    const SQFloat c3 = m11 * v1 + m10 * v0 + m12 * v2;
    const SQFloat m00 = src->m[0];
    const SQFloat m01 = src->m[4];
    const SQFloat m02 = src->m[8];
    const SQFloat m03 = src->m[12];
    const SQFloat invDet = 1.0f / (m01 * c1 + m00 * c0 + m02 * c2 + m03 * c3);
    out->m[0] = invDet * c0;
    out->m[1] = invDet * c1;
    out->m[2] = invDet * c2;
    out->m[3] = invDet * c3;
    out->m[4] = invDet * (m03 * v0 + m02 * u0 + m01 * t0);
    out->m[5] = (m03 * v1 + m00 * s0 + m02 * u1) * invDet;
    out->m[6] = (m03 * v2 + m00 * s1 + m01 * t1) * invDet;
    out->m[7] = (m02 * u2 + m00 * s2 + m01 * t2) * invDet;
    out->m[8] = (((m32 * m11 - m12 * m31) * m03 + (m33 * m12 - m13 * m32) * m01
        + (m13 * m31 - m33 * m11) * m02) * invDet);
    out->m[9] = (((m12 * m30 - m32 * m10) * m03 + (m13 * m32 - m33 * m12) * m00
        + (m33 * m10 - m13 * m30) * m02) * invDet);
    out->m[10] = (((m31 * m10 - m11 * m30) * m03 + (m33 * m11 - m13 * m31) * m00
        + (m13 * m30 - m33 * m10) * m01) * invDet);
    out->m[11] = (((m11 * m30 - m31 * m10) * m02 + (m12 * m31 - m32 * m11) * m00
        + (m32 * m10 - m12 * m30) * m01) * invDet);
    out->m[12] = (((m12 * m21 - m22 * m11) * m03 + (m22 * m13 - m23 * m12) * m01
        + (m23 * m11 - m13 * m21) * m02) * invDet);
    out->m[13] = (((m22 * m10 - m20 * m12) * m03 + (m23 * m12 - m22 * m13) * m00
        + (m20 * m13 - m23 * m10) * m02) * invDet);
    out->m[14] = invDet * ((m13 * m21 - m23 * m11) * m00 + (m23 * m10 - m20 * m13) * m01
        + (m11 * m20 - m21 * m10) * m03);
    out->m[15] = (((m21 * m10 - m11 * m20) * m02 + (m20 * m12 - m22 * m10) * m01
        + (m22 * m11 - m12 * m21) * m00) * invDet);
}

// EAX=out, EDX=a, ECX=b. Column-major 4x4. out = a * b. DisplayD3dSetBuffers copies the result.
// FUNCTION: VC1 0x00474e70
void MulMatrix4(float* out, const float* a, const float* b)
{
    out[0] = a[0] * b[0] + a[4] * b[1] + a[8] * b[2] + a[12] * b[3];
    out[1] = a[1] * b[0] + a[5] * b[1] + a[9] * b[2] + a[13] * b[3];
    out[2] = a[2] * b[0] + a[6] * b[1] + a[10] * b[2] + a[14] * b[3];
    out[3] = a[3] * b[0] + a[7] * b[1] + a[11] * b[2] + a[15] * b[3];
    out[4] = a[0] * b[4] + a[4] * b[5] + a[8] * b[6] + a[12] * b[7];
    out[5] = a[1] * b[4] + a[5] * b[5] + a[9] * b[6] + a[13] * b[7];
    out[6] = a[2] * b[4] + a[6] * b[5] + a[10] * b[6] + a[14] * b[7];
    out[7] = a[3] * b[4] + a[7] * b[5] + a[11] * b[6] + a[15] * b[7];
    out[8] = a[0] * b[8] + a[4] * b[9] + a[8] * b[10] + a[12] * b[11];
    out[9] = a[1] * b[8] + a[5] * b[9] + a[9] * b[10] + a[13] * b[11];
    out[10] = a[2] * b[8] + a[6] * b[9] + a[10] * b[10] + a[14] * b[11];
    out[11] = a[3] * b[8] + a[7] * b[9] + a[11] * b[10] + a[15] * b[11];
    out[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14] + a[12] * b[15];
    out[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14] + a[13] * b[15];
    out[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
    out[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
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
