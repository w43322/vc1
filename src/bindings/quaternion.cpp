#include "quaternion.h"
#include "vector2.h"
#include "value_types.h"
#include "kd/factory.h"

#include <math.h>
#include <new>

namespace vc1 {

// Bound as "Inverse" (0x004d7394) via 0x00422e30 / 0x00423dd0 (dispatcher
// 0x00425450). cdecl; out=[esp+4], src=[esp+8]. Conjugate / |q|^2.
// FUNCTION: VC1 0x00427390
Quaternion Quaternion::Inverse(const Quaternion& src)
{
    const Quaternion conjugate = src.Unm();
    return conjugate.Mul(1.0f / src.GetSquareMagnitude());
}

// Bound as "GetNormalized" (0x004d7248) via 0x004228b0 from 0x004c12f8.
// this in ECX; out=[esp+4]; RET 4. sqrt 0x004c4280.
// FUNCTION: VC1 0x00427450
Quaternion Quaternion::GetNormalized() const
{
    const SQFloat inv = 1.0f / GetMagnitude();
    return Quaternion(Vector3(v.x * inv, inv * v.y, inv * v.z), inv * w);
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c1330.
// FUNCTION: VC1 0x00427500
SQFloat Quaternion::GetSquareMagnitude() const
{
    return v.y * v.y + v.x * v.x + v.z * v.z + w * w;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c1314.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00427530
SQFloat Quaternion::GetMagnitude() const
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "Normalize" via 0x00422ac0 (dispatcher 0x00424060). this in ECX.
// FUNCTION: VC1 0x00427580
void Quaternion::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    v.x = inv * v.x;
    v.y = inv * v.y;
    v.z = v.z * inv;
    w = inv * w;
}

// Bound as "_tostring" via 0x00422a10 (dispatcher 0x00424470). this in ECX.
// Function-local static KdString (0x004ee480, guard 0x004ee490) with 0x40
// reserved characters.
// FUNCTION: VC1 0x00427600
KdString& Quaternion::ToString() const
{
    // GLOBAL: VC1 0x004ee480
    static KdString text(0x40, '\0');
    // STRING: VC1 0x004d7540
    text.Format("Quaternion(%s, %f)", v.ToString().c_str(), w);
    return text;
}

// Bound as "_unm" (0x004d36d0) via 0x004228b0 from 0x004c12af.
// Conjugate: (-x,-y,-z,w) into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004276d0
Quaternion Quaternion::Unm() const
{
    return Quaternion(Vector3(-v.x, -v.y, -v.z), w);
}

// this in ECX, out in EAX, scalar on the stack; RET 4. Called only by Div.
// FUNCTION: VC1 0x00427720
Quaternion Quaternion::operator/(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Quaternion(Vector3(inv * v.x, v.y * inv, v.z * inv), inv * w);
}

// Bound as "_div" (0x004d36c8) via 0x00422800 from 0x004c1293.
// FUNCTION: VC1 0x00427780
Quaternion Quaternion::Div(SQFloat scalar) const
{
    return *this / scalar;
}

// Bound as "_mul" (0x004d36c0) via 0x00422800 from 0x004c1277.
// Scalar multiply, not Hamilton product. this in ECX; RET 8.
// FUNCTION: VC1 0x004277a0
Quaternion Quaternion::Mul(SQFloat scalar) const
{
    return Quaternion(Vector3(scalar * v.x, v.y * scalar, v.z * scalar), scalar * w);
}

// Bound as "_sub" (0x004d36b8) via 0x00422750 from 0x004c125b. RET 8.
// FUNCTION: VC1 0x00427800
Quaternion Quaternion::Sub(const Quaternion& other) const
{
    return Quaternion(Vector3(v.x - other.v.x, v.y - other.v.y, v.z - other.v.z), w - other.w);
}

// Bound as "_add" (0x004d36b0) via 0x00422750 from 0x004c123f. RET 8.
// FUNCTION: VC1 0x00427860
Quaternion Quaternion::Add(const Quaternion& other) const
{
    return Quaternion(Vector3(other.v.x + v.x, other.v.y + v.y, other.v.z + v.z), other.w + w);
}

// Bound as "_cmp" via 0x00422960 (dispatcher 0x00424860). /OPT:ICF folded with
// Vector4::Cmp (same 16-byte memcmp).
// FUNCTION: VC1 0x00428590 FOLDED
SQInteger Quaternion::Cmp(const Quaternion& other) const
{
    return MemCompare(this, &other, sizeof(Quaternion));
}

// EAX = this; x, y, z, w on the stack; RET 0x10. Used by ConstructArgs.
// FUNCTION: VC1 0x00428db0
Quaternion::Quaternion(SQFloat ax, SQFloat ay, SQFloat az, SQFloat aw) : w(aw)
{
    v = Vector3(ax, ay, az);
}

// ESI = this, EDI = axis, angle on the stack. Half angle via the double
// constant 0x004d8a08; cos 0x004c4a50, sin 0x004c4920.
// FUNCTION: VC1 0x00428e90
Quaternion::Quaternion(SQFloat angle, const Vector3& axis)
{
    const SQFloat half = static_cast<SQFloat>(angle * 0.5);
    w = cosf(half);
    const SQFloat s = sinf(half);
    v = Vector3(s * axis.x, axis.y * s, s * axis.z);
}

// EAX = this. Zeros xyzw (not identity).
// FUNCTION: VC1 0x00428fb0
Quaternion::Quaternion() : w(0.0f)
{
    v = Vector3(0.0f, 0.0f, 0.0f);
}

// Bound as "FromMatrix" (0x004d73e0) via 0x00422c20 / 0x004248b0.
// Rotation extract from column-major Matrix4. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004b4480
void Quaternion::FromMatrix(const Matrix4& matrix)
{
    const SQFloat m00 = matrix.m[0];
    const SQFloat m11 = matrix.m[5];
    const SQFloat m22 = matrix.m[10];
    const SQFloat trace = m00 + m11 + m22;
    if (trace > 0.0f) {
        const SQFloat s = sqrtf(trace + 1.0f);
        w = s * 0.5f;
        const SQFloat inv = 0.25f / w;
        v.x = (matrix.m[6] - matrix.m[9]) * inv;
        v.y = (matrix.m[8] - matrix.m[2]) * inv;
        v.z = (matrix.m[1] - matrix.m[4]) * inv;
        return;
    }
    if (m11 < m00 && m22 < m00) {
        const SQFloat s = sqrtf(1.0f + m00 - m11 - m22);
        v.x = s * 0.5f;
        const SQFloat inv = 0.25f / v.x;
        v.y = (matrix.m[4] + matrix.m[1]) * inv;
        v.z = (matrix.m[8] + matrix.m[2]) * inv;
        w = (matrix.m[6] - matrix.m[9]) * inv;
        return;
    }
    if (m22 < m11) {
        const SQFloat s = sqrtf(1.0f + m11 - m00 - m22);
        v.y = s * 0.5f;
        const SQFloat inv = 0.25f / v.y;
        v.x = (matrix.m[4] + matrix.m[1]) * inv;
        v.z = (matrix.m[9] + matrix.m[6]) * inv;
        w = (matrix.m[8] - matrix.m[2]) * inv;
        return;
    }
    const SQFloat s = sqrtf(1.0f + m22 - m00 - m11);
    v.z = s * 0.5f;
    const SQFloat inv = 0.25f / v.z;
    v.x = (matrix.m[8] + matrix.m[2]) * inv;
    v.y = (matrix.m[9] + matrix.m[6]) * inv;
    w = (matrix.m[1] - matrix.m[4]) * inv;
}

// Bound as "ToMatrix" (0x004d73ec) via 0x00422cd0 / 0x00424910. this in ECX;
// out on the stack. Rows built as Vector3s and passed to the 4-vector Matrix4
// constructor with a zero origin.
// FUNCTION: VC1 0x004b46a0
Matrix4 Quaternion::ToMatrix() const
{
    return Matrix4(
        Vector3(1.0f - (v.y * v.y + v.z * v.z) * 2.0f,
            (v.x * v.y - w * v.z) * 2.0f,
            (w * v.y + v.x * v.z) * 2.0f),
        Vector3((v.x * v.y + w * v.z) * 2.0f,
            1.0f - (v.z * v.z + v.x * v.x) * 2.0f,
            (v.z * v.y - v.x * w) * 2.0f),
        Vector3((v.x * v.z - w * v.y) * 2.0f,
            (v.x * w + v.z * v.y) * 2.0f,
            1.0f - (v.x * v.x + v.y * v.y) * 2.0f),
        Vector3(0.0f, 0.0f, 0.0f));
}

// Bound as "Slerp" (0x004d73f8) via 0x00422d80 / 0x00424960.
// this is out. acos 0x0040cd40, sin 0x004c4920. this in ECX.
// FUNCTION: VC1 0x004b47b0
void Quaternion::Slerp(const Quaternion& a, const Quaternion& b, SQFloat t)
{
    SQFloat dot = Vector3::GetDotProduct(a.v, b.v) + a.w * b.w;
    SQFloat sign;
    if (dot < 0.0f) {
        dot = -dot;
        sign = -1.0f;
    } else {
        sign = 1.0f;
    }
    SQFloat s0;
    SQFloat s1;
    if (1.0f - dot > 1e-05) {
        const SQFloat omega = ClampedACos(dot);
        const SQFloat so = sinf(omega);
        s0 = sinf((1.0f - t) * omega) / so;
        s1 = sinf(omega * t) * sign / so;
    } else {
        s0 = 1.0f - t;
        s1 = t * sign;
    }
    *this = a.Mul(s0).Add(b.Mul(s1));
}

// Registered from 0x0041eb20 as "constructor" with typemask "*" (0x004d6b14).
// Switch on gettop-1. Error 0x004d6aa4.
// FUNCTION: VC1 0x004be0e0
SQInteger Quaternion::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Quaternion* const q = static_cast<Quaternion*>(Quaternion::operator new(sizeof(Quaternion)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (q) Quaternion();
        return SqPlus::PostConstruct<Quaternion>(vm, q, SqPlus::ConstructReleaseClass<Quaternion>::release);
    case 2: {
        Matrix4* const matrix = SqPlus::GetInstance<Matrix4, false>(vm, 2);
        if (!matrix) {
            break;
        }
        ::new (q) Quaternion(*matrix);
        return SqPlus::PostConstruct<Quaternion>(vm, q, SqPlus::ConstructReleaseClass<Quaternion>::release);
    }
    case 3: {
        Vector3* const axis = SqPlus::GetInstance<Vector3, false>(vm, 3);
        if (!axis) {
            break;
        }
        ::new (q) Quaternion(sa.GetFloat(2), *axis);
        return SqPlus::PostConstruct<Quaternion>(vm, q, SqPlus::ConstructReleaseClass<Quaternion>::release);
    }
    case 4: {
        Quaternion* const a = SqPlus::GetInstance<Quaternion, false>(vm, 2);
        Quaternion* const b = SqPlus::GetInstance<Quaternion, false>(vm, 3);
        if (!a || !b) {
            break;
        }
        ::new (q) Quaternion(*a, *b, sa.GetFloat(4));
        return SqPlus::PostConstruct<Quaternion>(vm, q, SqPlus::ConstructReleaseClass<Quaternion>::release);
    }
    case 5:
        ::new (q) Quaternion(sa.GetFloat(2), sa.GetFloat(3), sa.GetFloat(4), sa.GetFloat(5));
        return SqPlus::PostConstruct<Quaternion>(vm, q, SqPlus::ConstructReleaseClass<Quaternion>::release);
    }
    Quaternion::operator delete(q);
    // STRING: VC1 0x004d6aa4
    return sa.ThrowError("constructQuaternion: invalid arguments");
}

}
