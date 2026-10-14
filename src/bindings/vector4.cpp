#include "vector4.h"
#include "value_types.h"
#include "kd/factory.h"

#include <math.h>
#include <new>

namespace vc1 {

// Bound as "Lerp" (0x004d729c) via 0x00421720 / Register 0x00423c70.
// cdecl; out=a+t*(b-a) for xyzw. a=[esp+8], b=[esp+0xc], t=[esp+0x10], out=[esp+4].
// FUNCTION: VC1 0x00428280
Vector4 Vector4::Lerp(const Vector4& a, const Vector4& b, SQFloat t)
{
    return Vector4(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y),
        a.z + t * (b.z - a.z), a.w + t * (b.w - a.w));
}

// Bound as "GetCrossProduct" (0x004d728c) via 0x004216b0 / Register 0x00423c10.
// The xyz cross product as a Vector3: the dispatcher's Call helper
// (0x00426680) pushes the result as a "Vector3" instance. a=[esp+8],
// b=[esp+0xc], out=[esp+4].
// FUNCTION: VC1 0x004282d0
Vector3 Vector4::GetCrossProduct(const Vector4& a, const Vector4& b)
{
    return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// Bound as "GetDotProduct" (0x004d727c) via 0x00421640 / Register 0x00423bb0.
// cdecl; a=[esp+4], b=[esp+8]; y, x, z, w.
// FUNCTION: VC1 0x00428310
SQFloat Vector4::GetDotProduct(const Vector4& a, const Vector4& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Bound as "GetNormalized" (0x004d7248) via 0x00421380 from 0x004c0a4c.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428340
Vector4 Vector4::GetNormalized() const
{
    const SQFloat inv = 1.0f / GetMagnitude();
    return Vector4(x * inv, y * inv, z * inv, w * inv);
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c0a84.
// this in ECX; y, x, z, w then y*y + x*x + z*z + w*w.
// FUNCTION: VC1 0x004283c0
SQFloat Vector4::GetSquareMagnitude() const
{
    return x * x + y * y + z * z + w * w;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c0a68.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004283f0
SQFloat Vector4::GetMagnitude() const
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "Normalize" (0x004d723c) via 0x00421590 from 0x004c0a3d.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00428440
void Vector4::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
    z *= inv;
    w *= inv;
}

// Bound as "_tostring" (0x004d3710) from 0x004214e0. this in ECX. The text
// lives in a function-local static KdString (0x004ee444, guard 0x004ee454)
// with 0x40 reserved characters. Format 0x004d74fc.
// FUNCTION: VC1 0x004284b0
KdString& Vector4::ToString() const
{
    // GLOBAL: VC1 0x004ee444
    static KdString text(0x40, '\0');
    text.Format("Vector4(%f, %f, %f, %f)", x, y, z, w);
    return text;
}

// Bound as "_cmp" (0x004d36e0) from 0x00421430. this in ECX; other on stack.
// Also reused by another 16-byte binder at 0x00422960.
// FUNCTION: VC1 0x00428590
SQInteger Vector4::Cmp(const Vector4& other) const
{
    return MemCompare(this, &other, sizeof(Vector4));
}

// Bound as "_unm" (0x004d36d0) via 0x00421380 from 0x004c0a03.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004285b0
Vector4 Vector4::Unm() const
{
    return Vector4(-x, -y, -z, -w);
}

// Bound as "_div" (0x004d36c8) via 0x004212d0 from 0x004c09e7.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x004285e0
Vector4 Vector4::Div(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Vector4(x * inv, y * inv, z * inv, w * inv);
}

// Bound as "_mul" (0x004d36c0) via 0x004212d0 from 0x004c09cb.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428620
Vector4 Vector4::Mul(SQFloat scalar) const
{
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

// Bound as "_sub" (0x004d36b8) via 0x00421220 from 0x004c09af.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428650
Vector4 Vector4::Sub(const Vector4& other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

// Bound as "_add" (0x004d36b0) via 0x00421220 from 0x004c0993.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428680
Vector4 Vector4::Add(const Vector4& other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

// Registered from 0x0041e780 as "constructor" (0x004d2f10) with typemask "*"
// (0x004d6b14). Switch on the argument count (jump table 0x004be9c0), floats
// read right to left; missing components stay 0. Error string 0x004d6a38.
// FUNCTION: VC1 0x004be6c0
SQInteger Vector4::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Vector4* const vector = static_cast<Vector4*>(Vector4::operator new(sizeof(Vector4)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (vector) Vector4();
        break;
    case 2:
        ::new (vector) Vector4(sa.GetFloat(2));
        break;
    case 3:
        ::new (vector) Vector4(sa.GetFloat(2), sa.GetFloat(3));
        break;
    case 4:
        ::new (vector) Vector4(sa.GetFloat(2), sa.GetFloat(3), sa.GetFloat(4));
        break;
    case 5:
        ::new (vector) Vector4(sa.GetFloat(2), sa.GetFloat(3), sa.GetFloat(4), sa.GetFloat(5));
        break;
    default:
        Vector4::operator delete(vector);
        return sa.ThrowError("constructVector4: invalid arguments");
    }
    return SqPlus::PostConstruct<Vector4>(vm, vector, SqPlus::ConstructReleaseClass<Vector4>::release);
}

}
