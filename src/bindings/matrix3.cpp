#include "matrix3.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

#include <math.h>

namespace vc1 {

static NativeTypeInfo g_matrix3Type;
static unsigned g_matrix3TypeReady;
static const int kMatrix3TypeSpec = 0;

static void IdentityMatrix3(Matrix3* matrix)
{
    matrix->m[0] = 1.0f;
    matrix->m[1] = 0.0f;
    matrix->m[2] = 0.0f;
    matrix->m[3] = 0.0f;
    matrix->m[4] = 1.0f;
    matrix->m[5] = 0.0f;
    matrix->m[6] = 0.0f;
    matrix->m[7] = 0.0f;
    matrix->m[8] = 1.0f;
}

// Bound as "GetOrigin" (0x004d732c) via 0x00421b00 from 0x004c0c96.
// Returns &m[6]. this in ECX.
// FUNCTION: VC1 0x00401e60
Vector2* Matrix3::GetOrigin()
{
    return (Vector2*)(m + 6);
}

// Bound as "SetOrigin" (0x004d7348) via 0x00421bb0 from 0x004c0ce8.
// Writes two floats into m[6], m[7]. this in ECX; RET 4.
// FUNCTION: VC1 0x00401e70
void Matrix3::SetOrigin(const Vector2* origin)
{
    m[6] = origin->x;
    m[7] = origin->y;
}

// Bound as "GetYVec" (0x004d7324) via 0x00421b00 from 0x004c0c7a.
// Returns &m[3]. this in ECX.
// FUNCTION: VC1 0x004060a0
Vector2* Matrix3::GetYVec()
{
    return (Vector2*)(m + 3);
}

// Bound as "GetXVec" (0x004d731c) via 0x00421b00 from 0x004c0c5e.
// Returns this as Vector2*. this in ECX.
// FUNCTION: VC1 0x004060b0
Vector2* Matrix3::GetXVec()
{
    return (Vector2*)m;
}

// Bound as "SetYVec" (0x004d7340) via 0x00421bb0 from 0x004c0cce.
// Writes two floats into m[3], m[4]. this in ECX; RET 4.
// FUNCTION: VC1 0x0040cdb0
void Matrix3::SetYVec(const Vector2* axis)
{
    m[3] = axis->x;
    m[4] = axis->y;
}

// Bound as "SetXVec" (0x004d7338) via 0x00421bb0 from 0x004c0cb2.
// Writes two floats into m[0], m[1]. this in ECX; RET 4.
// FUNCTION: VC1 0x0040cdd0
void Matrix3::SetXVec(const Vector2* axis)
{
    m[0] = axis->x;
    m[1] = axis->y;
}

// SQRELEASEHOOK; size unused. Puts 36 bytes back on pools[9] / 0x004ed47c.
// FUNCTION: VC1 0x0041dd20
SQInteger Matrix3::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix3);
    }
    return 0;
}

// this in ESI. Name 0x004d1920.
// FUNCTION: VC1 0x0041e050
NativeClassBinding* Matrix3::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Matrix3", Matrix3::RegisterClass);
}

// FUNCTION: VC1 0x0041e860
NativeClassBinding* Matrix3::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Matrix3::ConstructArgs);
}

// Once-flag 0x004ed838, type info 0x004ed824, spec 0x004d1928, name 0x004d1920.
// FUNCTION: VC1 0x0041f570
SquirrelObject* Matrix3::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_matrix3Type, &g_matrix3TypeReady,
        &kMatrix3TypeSpec, "Matrix3", Matrix3::Constructor, Matrix3::TypeOf);
}

// Registered from 0x0041f570 as "constructor" (0x004d2f10) with null typemask.
// Writes column-major identity. Arg-taking factory is 0x004be540.
// FUNCTION: VC1 0x0041ff30
SQInteger Matrix3::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix3);
    Matrix3* const matrix = static_cast<Matrix3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Allocate());
    if (matrix) {
        IdentityMatrix3(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix3::ReleaseHook);
}

// FUNCTION: VC1 0x004236c0
SQInteger Matrix3::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Matrix3", -1);
    return 1;
}

// Bound as "MultVecs" (0x004d7380) via 0x00421bb0 from 0x004c0d94.
// Scales X by v.x and Y by v.y. this in ECX; scale=[esp+4]; RET 4.
// FUNCTION: VC1 0x00427dc0
void Matrix3::MultVecs(const Vector2* scale)
{
    m[0] *= scale->x;
    m[1] *= scale->x;
    m[3] *= scale->y;
    m[4] *= scale->y;
}

// Bound as "MultInvVrt" (0x004d7368) via 0x00421dc0 from 0x004c0d5c.
// out = (v-origin) dotted with orthonormal X/Y. this in ECX; RET 8.
// FUNCTION: VC1 0x00427e00
Vector2* Matrix3::MultInvVrt(Vector2* out, const Vector2* v)
{
    const SQFloat dx = v->x - m[6];
    const SQFloat dy = v->y - m[7];
    out->x = dx * m[0] + dy * m[1];
    out->y = dx * m[3] + dy * m[4];
    return out;
}

// Bound as "GetNormalized" (0x004d7248) via 0x004218f0 from 0x004c0d15.
// Unit X/Y, copy origin, m[2]=m[5]=0, m[8]=1. sqrt 0x004c4280. RET 4.
// Assembly length of Y (m[3],m[4]) first, then X (m[0],m[1]); stores y-hat, origin, x-hat.
// FUNCTION: VC1 0x00427e50
Matrix3* Matrix3::GetNormalized(Matrix3* out)
{
    const SQFloat invY = 1.0f / sqrtf(m[3] * m[3] + m[4] * m[4]);
    const SQFloat y0 = m[3] * invY;
    const SQFloat y1 = m[4] * invY;
    const SQFloat invX = 1.0f / sqrtf(m[0] * m[0] + m[1] * m[1]);
    out->m[0] = m[0] * invX;
    out->m[3] = y0;
    out->m[6] = m[6];
    out->m[1] = m[1] * invX;
    out->m[4] = y1;
    out->m[7] = m[7];
    out->m[2] = 0.0f;
    out->m[5] = 0.0f;
    out->m[8] = 1.0f;
    return out;
}

// In-place unit X/Y, m[2]=m[5]=0, m[8]=1. sqrt helper 0x004c4280. this in ESI/ECX.
// Assembly normalizes X (m[0],m[1]) first, then Y (m[3],m[4]); zeros m[5] then m[2].
// Not the Squirrel "Mult" binder at 0x00474310; this is the in-place companion of GetNormalized.
// FUNCTION: VC1 0x00427f20
void Matrix3::Normalize()
{
    SQFloat inv = 1.0f / sqrtf(m[0] * m[0] + m[1] * m[1]);
    m[0] *= inv;
    m[1] *= inv;
    inv = 1.0f / sqrtf(m[3] * m[3] + m[4] * m[4]);
    m[3] *= inv;
    m[4] *= inv;
    m[2] = 0.0f;
    m[5] = 0.0f;
    m[8] = 1.0f;
}

// Bound as "_cmp" (0x004d36e0). this in EDX; other on stack; size 0x24.
// FUNCTION: VC1 0x00428110
SQInteger Matrix3::Cmp(const Matrix3* other)
{
    return MemCompare(this, other, sizeof(Matrix3));
}

// Bound as "_unm" (0x004d36d0) via 0x004218f0 from 0x004c0c24.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428130
Matrix3* Matrix3::Unm(Matrix3* out)
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
    return out;
}

// Bound as "_div" (0x004d36c8) via 0x00421840 from 0x004c0c15.
// Wrapper 0x00428180: FLD scalar, EAX=out, PUSH this, FSTP [esp], CALL 0x00474370, RET 8.
// Helper 0x00474370: this in ECX, scalar on stack, out in EAX, writes (1/scalar)*this.
// FUNCTION: VC1 0x00428180
Matrix3* Matrix3::Div(Matrix3* out, SQFloat scalar)
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
    return out;
}

// Bound as "_mul" (0x004d36c0) via 0x00421790 from 0x004c0bf7.
// Wrapper 0x004281a0: EAX=out, EDX=this, ECX=other, CALL 0x00474470, RET 8.
// Helper 0x00474470: out = this * other, column-major 3x3 (EDX=this, ECX=other, EAX=out).
// FUNCTION: VC1 0x004281a0
Matrix3* Matrix3::Mul(Matrix3* out, const Matrix3* other)
{
    const SQFloat* o = other->m;
    out->m[0] = m[0] * o[0] + m[3] * o[1] + m[6] * o[2];
    out->m[1] = m[1] * o[0] + m[4] * o[1] + m[7] * o[2];
    out->m[2] = m[2] * o[0] + m[5] * o[1] + m[8] * o[2];
    out->m[3] = m[0] * o[3] + m[3] * o[4] + m[6] * o[5];
    out->m[4] = m[1] * o[3] + m[4] * o[4] + m[7] * o[5];
    out->m[5] = m[2] * o[3] + m[5] * o[4] + m[8] * o[5];
    out->m[6] = m[0] * o[6] + m[3] * o[7] + m[6] * o[8];
    out->m[7] = m[1] * o[6] + m[4] * o[7] + m[7] * o[8];
    out->m[8] = m[2] * o[6] + m[5] * o[7] + m[8] * o[8];
    return out;
}

// Bound as "_sub" (0x004d36b8) via 0x00421790 from 0x004c0bdd.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x004281c0
Matrix3* Matrix3::Sub(Matrix3* out, const Matrix3* other)
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
    return out;
}

// Bound as "_add" (0x004d36b0) via 0x00421790 from 0x004c0bc1.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428220
Matrix3* Matrix3::Add(Matrix3* out, const Matrix3* other)
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
    return out;
}

// Bound as "Transpose" (0x004d739c) via 0x00421e70 from 0x004c0ded.
// cdecl; out=[esp+4], src=[esp+8]. Column-major 3x3.
// FUNCTION: VC1 0x004740d0
void Matrix3::Transpose(Matrix3* out, const Matrix3* src)
{
    out->m[0] = src->m[0];
    out->m[1] = src->m[3];
    out->m[2] = src->m[6];
    out->m[3] = src->m[1];
    out->m[4] = src->m[4];
    out->m[5] = src->m[7];
    out->m[6] = src->m[2];
    out->m[7] = src->m[5];
    out->m[8] = src->m[8];
}

// Bound as "Inverse" (0x004d7394) via 0x00421e70 from 0x004c0dd1.
// cdecl; out=[esp+4], src=[esp+8]. Adjugate / det of column-major 3x3.
// FUNCTION: VC1 0x00474110
void Matrix3::Inverse(Matrix3* out, const Matrix3* src)
{
    const SQFloat a = src->m[0];
    const SQFloat b = src->m[1];
    const SQFloat c = src->m[2];
    const SQFloat d = src->m[3];
    const SQFloat e = src->m[4];
    const SQFloat f = src->m[5];
    const SQFloat g = src->m[6];
    const SQFloat h = src->m[7];
    const SQFloat i = src->m[8];
    const SQFloat c0 = e * i - f * h;
    const SQFloat c1 = g * f - d * i;
    const SQFloat c2 = h * d - g * e;
    const SQFloat invDet = 1.0f / (b * c1 + a * c0 + c * c2);
    out->m[0] = invDet * c0;
    out->m[1] = invDet * (c * h - i * b);
    out->m[2] = (f * b - e * c) * invDet;
    out->m[3] = invDet * c1;
    out->m[4] = invDet * (a * i - g * c);
    out->m[5] = invDet * (d * c - f * a);
    out->m[6] = invDet * c2;
    out->m[7] = invDet * (b * g - h * a);
    out->m[8] = invDet * (e * a - b * d);
}

// Bound as "MultInvVect" (0x004d735c) via 0x00421dc0 from 0x004c0d4a.
// out = (X·v, Y·v). this in ECX; RET 8.
// FUNCTION: VC1 0x004742b0
Vector2* Matrix3::MultInvVect(Vector2* out, const Vector2* v)
{
    out->x = m[0] * v->x + m[1] * v->y;
    out->y = m[3] * v->x + m[4] * v->y;
    return out;
}

// Bound as "MultVect" (0x004d7374) via 0x00421dc0 from 0x004c0d82.
// out = 2x2 linear part * v. this in ECX; RET 8.
// FUNCTION: VC1 0x004742e0
Vector2* Matrix3::MultVect(Vector2* out, const Vector2* v)
{
    out->x = m[0] * v->x + m[3] * v->y;
    out->y = m[1] * v->x + m[4] * v->y;
    return out;
}

// Bound as "Mult" (0x004d7354) via 0x00421d10 from 0x004c0d31.
// out = this * v, column-major. this in ECX; RET 8.
// FUNCTION: VC1 0x00474310
Vector3* Matrix3::Mult(Vector3* out, const Vector3* v)
{
    out->x = m[0] * v->x + m[3] * v->y + m[6] * v->z;
    out->y = m[1] * v->x + m[4] * v->y + m[7] * v->z;
    out->z = m[2] * v->x + m[5] * v->y + m[8] * v->z;
    return out;
}

// Registered from 0x0041e860 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; identity when extra==0. Error string 0x004d6a5c.
// extra 1-3 copy/from-vectors via unmapped helpers; identity kept as fallback.
// FUNCTION: VC1 0x004be540
SQInteger Matrix3::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Matrix3);
    Matrix3* const matrix = static_cast<Matrix3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 3) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Matrix3))]->Release(matrix);
        g_sizeClassHeap.releasedBytes += sizeof(Matrix3);
        return sq_throwerror(vm, "constructMatrix3: invalid arguments");
    }
    if (matrix) {
        IdentityMatrix3(matrix);
    }
    return BindNativeInstance(vm, matrix, Matrix3::ReleaseHook);
}

}
