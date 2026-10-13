#include "quaternion.h"
#include "vector4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

#include <math.h>

namespace vc1 {

static NativeTypeInfo g_quaternionType;
static unsigned g_quaternionTypeReady;
static const int kQuaternionTypeSpec = 0;

static void ZeroQuaternion(Quaternion* quaternion)
{
    quaternion->x = 0.0f;
    quaternion->y = 0.0f;
    quaternion->z = 0.0f;
    quaternion->w = 0.0f;
}

// this in ESI. Name 0x004d75a0.
// FUNCTION: VC1 0x0041e170
NativeClassBinding* Quaternion::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Quaternion", Quaternion::RegisterClass);
}

// FUNCTION: VC1 0x0041eb20
NativeClassBinding* Quaternion::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Quaternion::ConstructArgs);
}

// Once-flag 0x004ee3e4, type info 0x004ee1ec, spec 0x004d75c8, name 0x004d75a0.
// FUNCTION: VC1 0x0041f830
SquirrelObject Quaternion::RegisterClass(HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterClassType<Quaternion>(vm, className, &g_quaternionType, &g_quaternionTypeReady,
        &kQuaternionTypeSpec, "Quaternion", Quaternion::Constructor, Quaternion::TypeOf);
}

// Registered from 0x0041f830 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyzw (not identity). Release hook 0x0041dd60 (Vector4). Factory 0x004be0e0.
// FUNCTION: VC1 0x00420000
SQInteger Quaternion::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Quaternion);
    Quaternion* const quaternion = static_cast<Quaternion*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Allocate());
    if (quaternion) {
        ZeroQuaternion(quaternion);
    }
    return BindNativeInstance(vm, quaternion, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x00423700
SQInteger Quaternion::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Quaternion", -1);
    return 1;
}

// Bound as "Inverse" (0x004d7394) via 0x00422e30 / FUN_00423dd0.
// cdecl; out=[esp+4], src=[esp+8]. conjugate / |q|^2.
// FUNCTION: VC1 0x00427390
void Quaternion::Inverse(Quaternion* out, const Quaternion* src)
{
    const SQFloat inv = 1.0f / (src->x * src->x + src->y * src->y + src->z * src->z
        + src->w * src->w);
    out->x = -src->x * inv;
    out->y = -src->y * inv;
    out->z = -src->z * inv;
    out->w = src->w * inv;
}

// Bound as "GetNormalized" (0x004d7248) via 0x004228b0 from 0x004c12f8.
// Writes this/|this| into out=[ebp+8]. this in ESI/ECX; RET 4. sqrt 0x004c4280.
// FUNCTION: VC1 0x00427450
Quaternion* Quaternion::GetNormalized(Quaternion* out)
{
    const SQFloat inv = 1.0f / GetMagnitude();
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    out->w = w * inv;
    return out;
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c1330.
// this in ECX; y, x, z, w then y*y + x*x + z*z + w*w.
// FUNCTION: VC1 0x00427500
SQFloat Quaternion::GetSquareMagnitude()
{
    return x * x + y * y + z * z + w * w;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c1314.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00427530
SQFloat Quaternion::GetMagnitude()
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "_unm" (0x004d36d0) via 0x004228b0 from 0x004c12af.
// Conjugate: (-x,-y,-z,w) into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004276d0
Quaternion* Quaternion::Unm(Quaternion* out)
{
    out->x = -x;
    out->y = -y;
    out->z = -z;
    out->w = w;
    return out;
}

// Bound as "_div" (0x004d36c8) via 0x00422800 from 0x004c1293.
// Wrapper 0x00427780: FLD scalar, EAX=out, PUSH this, CALL 0x00427720, RET 8.
// Helper 0x00427720 writes (1/scalar)*this.
// FUNCTION: VC1 0x00427780
Quaternion* Quaternion::Div(Quaternion* out, SQFloat scalar)
{
    const SQFloat inv = 1.0f / scalar;
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    out->w = w * inv;
    return out;
}

// Bound as "_mul" (0x004d36c0) via 0x00422800 from 0x004c1277.
// Scalar multiply, not Hamilton product. this in ECX; RET 8.
// FUNCTION: VC1 0x004277a0
Quaternion* Quaternion::Mul(Quaternion* out, SQFloat scalar)
{
    out->x = x * scalar;
    out->y = y * scalar;
    out->z = z * scalar;
    out->w = w * scalar;
    return out;
}

// Bound as "_sub" (0x004d36b8) via 0x00422750 from 0x004c125b.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00427800
Quaternion* Quaternion::Sub(Quaternion* out, const Quaternion* other)
{
    out->x = x - other->x;
    out->y = y - other->y;
    out->z = z - other->z;
    out->w = w - other->w;
    return out;
}

// Bound as "_add" (0x004d36b0) via 0x00422750 from 0x004c123f.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00427860
Quaternion* Quaternion::Add(Quaternion* out, const Quaternion* other)
{
    out->x = x + other->x;
    out->y = y + other->y;
    out->z = z + other->z;
    out->w = w + other->w;
    return out;
}

// Bound as "FromMatrix" (0x004d73e0) via 0x00422c20 / FUN_004248b0.
// Rotation extract from column-major Matrix4. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004b4480
void Quaternion::FromMatrix(const Matrix4* matrix)
{
    const SQFloat m00 = matrix->m[0];
    const SQFloat m11 = matrix->m[5];
    const SQFloat m22 = matrix->m[10];
    const SQFloat trace = m00 + m11 + m22;
    if (trace > 0.0f) {
        const SQFloat s = sqrtf(trace + 1.0f);
        w = s * 0.5f;
        const SQFloat inv = 0.25f / w;
        x = (matrix->m[6] - matrix->m[9]) * inv;
        y = (matrix->m[8] - matrix->m[2]) * inv;
        z = (matrix->m[1] - matrix->m[4]) * inv;
        return;
    }
    if (m11 < m00 && m22 < m00) {
        const SQFloat s = sqrtf(1.0f + m00 - m11 - m22);
        x = s * 0.5f;
        const SQFloat inv = 0.25f / x;
        y = (matrix->m[4] + matrix->m[1]) * inv;
        z = (matrix->m[8] + matrix->m[2]) * inv;
        w = (matrix->m[6] - matrix->m[9]) * inv;
        return;
    }
    if (m22 < m11) {
        const SQFloat s = sqrtf(1.0f + m11 - m00 - m22);
        y = s * 0.5f;
        const SQFloat inv = 0.25f / y;
        x = (matrix->m[4] + matrix->m[1]) * inv;
        z = (matrix->m[9] + matrix->m[6]) * inv;
        w = (matrix->m[8] - matrix->m[2]) * inv;
        return;
    }
    const SQFloat s = sqrtf(1.0f + m22 - m00 - m11);
    z = s * 0.5f;
    const SQFloat inv = 0.25f / z;
    x = (matrix->m[8] + matrix->m[2]) * inv;
    y = (matrix->m[9] + matrix->m[6]) * inv;
    w = (matrix->m[1] - matrix->m[4]) * inv;
}

// Bound as "ToMatrix" (0x004d73ec) via 0x00422cd0 / FUN_00424910.
// Writes a column-major rotation Matrix4. this in ECX.
// FUNCTION: VC1 0x004b46a0
void Quaternion::ToMatrix(Matrix4* out) const
{
    const SQFloat xx = x * x;
    const SQFloat yy = y * y;
    const SQFloat zz = z * z;
    const SQFloat xy = x * y;
    const SQFloat xz = x * z;
    const SQFloat yz = y * z;
    const SQFloat wx = w * x;
    const SQFloat wy = w * y;
    const SQFloat wz = w * z;
    out->m[0] = 1.0f - (yy + zz) * 2.0f;
    out->m[4] = (xy + wz) * 2.0f;
    out->m[8] = (xz - wy) * 2.0f;
    out->m[12] = 0.0f;
    out->m[1] = (xy - wz) * 2.0f;
    out->m[5] = 1.0f - (zz + xx) * 2.0f;
    out->m[9] = (wx + yz) * 2.0f;
    out->m[13] = 0.0f;
    out->m[2] = (wy + xz) * 2.0f;
    out->m[6] = (yz - wx) * 2.0f;
    out->m[10] = 1.0f - (xx + yy) * 2.0f;
    out->m[14] = 0.0f;
    out->m[3] = 0.0f;
    out->m[7] = 0.0f;
    out->m[11] = 0.0f;
    out->m[15] = 1.0f;
}

// Bound as "Slerp" (0x004d73f8) via 0x00422d80 / FUN_00424960.
// this is out. acos 0x0040cd40, sin 0x004c4920. this in ECX.
// FUNCTION: VC1 0x004b47b0
void Quaternion::Slerp(const Quaternion* a, const Quaternion* b, SQFloat t)
{
    SQFloat dot = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    SQFloat sign = 1.0f;
    if (dot < 0.0f) {
        dot = -dot;
        sign = -1.0f;
    }
    SQFloat s0;
    SQFloat s1;
    if (1.0f - dot <= 1e-05f) {
        s0 = 1.0f - t;
        s1 = t * sign;
    } else {
        const SQFloat omega = acosf(dot);
        const SQFloat so = sinf(omega);
        s0 = sinf((1.0f - t) * omega) / so;
        s1 = (sinf(t * omega) * sign) / so;
    }
    x = s0 * a->x + s1 * b->x;
    y = s1 * b->y + s0 * a->y;
    z = a->z * s0 + b->z * s1;
    w = s0 * a->w + b->w * s1;
}

// Registered from 0x0041eb20 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; extra>4 throws. extra==4 reads four NumericFloat. Error 0x004d6aa4.
// extra 1-3 use unmapped axis/angle helpers; zeros kept as fallback.
// FUNCTION: VC1 0x004be0e0
SQInteger Quaternion::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Quaternion);
    Quaternion* const quaternion = static_cast<Quaternion*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Release(quaternion);
        g_sizeClassHeap.releasedBytes += sizeof(Quaternion);
        return sq_throwerror(vm, "constructQuaternion: invalid arguments");
    }
    if (quaternion) {
        ZeroQuaternion(quaternion);
        if (extra == 4) {
            quaternion->x = NumericFloat(vm, 2);
            quaternion->y = NumericFloat(vm, 3);
            quaternion->z = NumericFloat(vm, 4);
            quaternion->w = NumericFloat(vm, 5);
        }
    }
    return BindNativeInstance(vm, quaternion, Vector4::ReleaseHook);
}

}
