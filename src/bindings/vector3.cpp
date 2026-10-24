#include "vector3.h"
#include "value_types.h"
#include "kd/factory.h"

#include <math.h>
#include <new>

namespace vc1 {

static inline SQFloat AbsFloat(SQFloat value)
{
    return value < 0.0f ? -value : value;
}

// GLOBAL: VC1 0x004e9940
Vector3 Vector3::AxisX(1.0f, 0.0f, 0.0f);
// GLOBAL: VC1 0x004e994c
Vector3 Vector3::AxisY(0.0f, 1.0f, 0.0f);
// GLOBAL: VC1 0x004e9958
Vector3 Vector3::AxisZ(0.0f, 0.0f, 1.0f);
// GLOBAL: VC1 0x004ed62c
Vector3 Vector3::Null(0.0f, 0.0f, 0.0f);

// Bound as "GetCrossProduct" (0x004d728c) through staticFunc (0x00420ae0,
// Register 0x00423990). cdecl; out=[esp+4], a=[esp+8], b=[esp+0xc]. Stores z,
// then x, then y.
// FUNCTION: VC1 0x0040cb90
Vector3 Vector3::GetCrossProduct(const Vector3& a, const Vector3& b)
{
    Vector3 result;
    result.z = a.x * b.y - a.y * b.x;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    return result;
}

// Bound as "GetDotProduct" (0x004d727c) through staticFunc (0x00420a70,
// Register 0x00423930). cdecl; a=[esp+4], b=[esp+8]; y*y then x*x then z*z.
// FUNCTION: VC1 0x0040cbd0
SQFloat Vector3::GetDotProduct(const Vector3& a, const Vector3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Bound as "Normalize" (0x004d723c) from 0x004209c0.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00415880
void Vector3::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
    z *= inv;
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70.
// this in ECX; y, x, z then y*y + x*x + z*z.
// FUNCTION: VC1 0x004158e0
SQFloat Vector3::GetSquareMagnitude() const
{
    return x * x + y * y + z * z;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00415910
SQFloat Vector3::GetMagnitude() const
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "GetNormalized" (0x004d7248) via 0x004207b0.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00415950
Vector3 Vector3::GetNormalized() const
{
    const SQFloat inv = 1.0f / GetMagnitude();
    return Vector3(x * inv, y * inv, z * inv);
}

// Bound as "Lerp" (0x004d729c) through staticFunc (0x00420b50, Register
// 0x004239f0). cdecl; out=a+t*(b-a). a=[esp+8], b=[esp+0xc], t=[esp+0x10],
// out=[esp+4].
// FUNCTION: VC1 0x00428960
Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, SQFloat t)
{
    return Vector3(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z));
}

// Bound as "_tostring" (0x004d3710) from 0x00420910. this in ECX. The text
// lives in a function-local static KdString (0x004ee41c, guard 0x004ee42c)
// with 0x40 reserved characters.
// FUNCTION: VC1 0x004289a0
KdString& Vector3::ToString() const
{
    // GLOBAL: VC1 0x004ee41c
    static KdString text(0x40, '\0');
    // STRING: VC1 0x004d74d8
    text.Format("Vector3(%f, %f, %f)", x, y, z);
    return text;
}

// Bound as "_cmp" (0x004d36e0) from 0x00420860. this in ECX; other on stack.
// FUNCTION: VC1 0x00428a70
SQInteger Vector3::Cmp(const Vector3& other) const
{
    return MemCompare(this, &other, sizeof(Vector3));
}

// Bound as "_unm" (0x004d36d0) via 0x004207b0.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428a90
Vector3 Vector3::Unm() const
{
    return Vector3(-x, -y, -z);
}

// Bound as "_div" (0x004d36c8) via 0x00420700.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x00428ab0
Vector3 Vector3::Div(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Vector3(x * inv, y * inv, z * inv);
}

// Bound as "_mul" (0x004d36c0) via 0x00420700.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428ae0
Vector3 Vector3::Mul(SQFloat scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);
}

// Bound as "_sub" (0x004d36b8) via 0x00422550.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428b10
Vector3 Vector3::Sub(const Vector3& other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

// Bound as "_add" (0x004d36b0) via 0x00422550.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428b40
Vector3 Vector3::Add(const Vector3& other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

// Inferred name; not a script binding. Only caller is the Matrix4(x, origin)
// constructor (0x004159c0). Crosses v with the axis it is least aligned with.
// LTCG ABI: ECX=v, EDX=out. The absolute values branch on < 0 (no FABS).
// FUNCTION: VC1 0x004bc570
Vector3 Vector3::GetOrthogonal(const Vector3& v)
{
    if (AbsFloat(v.x) < AbsFloat(v.y)) {
        if (AbsFloat(v.x) < AbsFloat(v.z)) {
            return GetCrossProduct(v, AxisX);
        }
    }
    else if (AbsFloat(v.y) < AbsFloat(v.z)) {
        return GetCrossProduct(v, AxisY);
    }
    return GetCrossProduct(v, AxisZ);
}

// Registered from 0x0041e4c0 as "constructor" (0x004d2f10) with typemask "*"
// (0x004d6b14). Switch on the argument count (jump table 0x004bed64); each
// case placement-constructs the pooled block, reading the floats right to
// left. Error string 0x004d69f0.
// FUNCTION: VC1 0x004beb40
SQInteger Vector3::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Vector3* const vector = static_cast<Vector3*>(Vector3::operator new(sizeof(Vector3)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (vector) Vector3();
        break;
    case 2:
        ::new (vector) Vector3(sa.GetFloat(2));
        break;
    case 3:
        ::new (vector) Vector3(sa.GetFloat(2), sa.GetFloat(3));
        break;
    case 4:
        ::new (vector) Vector3(sa.GetFloat(2), sa.GetFloat(3), sa.GetFloat(4));
        break;
    default:
        Vector3::operator delete(vector);
        // STRING: VC1 0x004d69f0
        return sa.ThrowError("constructVector3: invalid arguments");
    }
    return SqPlus::PostConstruct<Vector3>(vm, vector, SqPlus::ConstructReleaseClass<Vector3>::release);
}

}
