#include "vector2.h"
#include "vector3.h"
#include "value_types.h"
#include "kdcore/platform.h"
#include "kd/factory.h"

#include <math.h>
#include <new>

namespace vc1 {

// GLOBAL: VC1 0x004e9964
Vector2 Vector2::AxisX(1.0f, 0.0f);
// GLOBAL: VC1 0x004e996c
Vector2 Vector2::AxisY(0.0f, 1.0f);
// GLOBAL: VC1 0x004ed638
Vector2 Vector2::Null(0.0f, 0.0f);

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c0883.
// this in ECX; x*x + y*y.
// FUNCTION: VC1 0x004073c0
SQFloat Vector2::GetSquareMagnitude() const
{
    return x * x + y * y;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c0867.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004073e0
SQFloat Vector2::GetMagnitude() const
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "GetNormalized" (0x004d7248) via 0x00420c70 from 0x004c084b.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00407410
Vector2 Vector2::GetNormalized() const
{
    const SQFloat inv = 1.0f / GetMagnitude();
    return Vector2(x * inv, y * inv);
}

// Bound as "Normalize" (0x004d723c) via 0x00420f30 from 0x004c083c.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x0040cb10
void Vector2::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
}

// Bound as "AngleBetween" (0x004d72d4) through staticFunc (0x00421090) from
// 0x004c08ca. cdecl; a=[esp+4], b=[esp+8]. Both vectors are widened to
// Vector3 (z=0): the sign is the z of the cross product (b first) and the dot
// product keeps the 0*0 term. Clamped acos 0x0040cd40.
// FUNCTION: VC1 0x0040cc00
SQFloat Vector2::AngleBetween(const Vector2& a, const Vector2& b)
{
    const SQFloat sign =
        Vector3::GetCrossProduct(Vector3(b.x, b.y), Vector3(a.x, a.y)).z < 0.0f ? -1.0f : 1.0f;
    return ClampedACos(Vector3::GetDotProduct(Vector3(a.x, a.y), Vector3(b.x, b.y))) * sign;
}

// Bound as "GetDotProduct" (0x004d727c) via 0x00421090 from 0x004c08ae.
// cdecl; a=[esp+4], b=[esp+8]; y*y then x*x.
// FUNCTION: VC1 0x0040cd20
SQFloat Vector2::GetDotProduct(const Vector2& a, const Vector2& b)
{
    return a.x * b.x + a.y * b.y;
}

// Called from AngleBetween and Quaternion::Slerp. cdecl; value=[ebp+8].
// pi is 0x004d0ab8 (0x40490fdb), -1 is 0x004d89f8; acos 0x004c4e10.
// FUNCTION: VC1 0x0040cd40
SQFloat ClampedACos(SQFloat value)
{
    if (value <= -1.0f) {
        return 3.14159265f;
    }
    if (value >= 1.0f) {
        return 0.0f;
    }
    return acosf(value);
}

// Bound as "GetOrthogonal" (0x004d72e4) via 0x004211b0 / Register 0x00423b50.
// cdecl; out=[esp+4], v=[esp+8]; (-y, x).
// FUNCTION: VC1 0x0040cd90
Vector2 Vector2::GetOrthogonal(const Vector2& v)
{
    return Vector2(-v.y, v.x);
}

// Bound as "Lerp" (0x004d729c) via 0x00421140 / Register 0x00423af0.
// cdecl; out=a+t*(b-a). a=[esp+8], b=[esp+0xc], t=[esp+0x10], out=[esp+4].
// FUNCTION: VC1 0x004286b0
Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, SQFloat t)
{
    return Vector2(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
}

// Bound as "IsValid" (0x004d72cc) via 0x00420fe0 from 0x004c089f; the
// dispatcher (0x00424a50) pushes a bool.
// FLD x; FLD +Inf (0x004e95cc / 0x7f800000); FCHS to -Inf; FCOM/FUCOM with TEST AH,5 (NaN/below).
// Same pair for y. Returns 1 if both finite. this in ECX.
// FUNCTION: VC1 0x004286e0
bool Vector2::IsValid() const
{
    return x > -g_kdInfinity.value && x < g_kdInfinity.value && y > -g_kdInfinity.value &&
        y < g_kdInfinity.value;
}

// Bound as "Equal" (0x004d72c4) via 0x00420e80 from 0x004c082d; the
// dispatcher (0x004241a0) pushes a bool.
// FUCOMPP x then y. this in ECX; other=[esp+4]; RET 4.
// FUNCTION: VC1 0x00428750
bool Vector2::Equal(const Vector2& other) const
{
    return x == other.x && y == other.y;
}

// Bound as "_tostring" (0x004d3710) from 0x00420dd0. this in ECX. The text
// lives in a function-local static KdString (0x004ee430, guard 0x004ee440)
// with 0x20 reserved characters.
// FUNCTION: VC1 0x00428780
KdString& Vector2::ToString() const
{
    // GLOBAL: VC1 0x004ee430
    static KdString text(0x20, '\0');
    text.Format("Vector2(%f, %f)", x, y);
    return text;
}

// Bound as "_cmp" (0x004d36e0) from 0x00420d20. this in ECX; other on stack.
// FUNCTION: VC1 0x00428890
SQInteger Vector2::Cmp(const Vector2& other) const
{
    return MemCompare(this, &other, sizeof(Vector2));
}

// Bound as "_unm" (0x004d36d0) via 0x00420c70 from 0x004c07f3.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004288b0
Vector2 Vector2::Unm() const
{
    return Vector2(-x, -y);
}

// Bound as "_div" (0x004d36c8) via 0x00420bc0 from 0x004c07d7.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x004288d0
Vector2 Vector2::Div(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Vector2(x * inv, y * inv);
}

// Bound as "_mul" (0x004d36c0) via 0x00420bc0 from 0x004c07bb.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428900
Vector2 Vector2::Mul(SQFloat scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

// Bound as "_sub" (0x004d36b8) via 0x00421dc0 from 0x004c079f.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428920
Vector2 Vector2::Sub(const Vector2& other) const
{
    return Vector2(x - other.x, y - other.y);
}

// Bound as "_add" (0x004d36b0) via 0x00421dc0 from 0x004c0783.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428940
Vector2 Vector2::Add(const Vector2& other) const
{
    return Vector2(x + other.x, y + other.y);
}

// Registered from 0x0041e620 as "constructor" (0x004d2f10) with typemask "*"
// (0x004d6b14). Same shape as Vector3::ConstructArgs: switch on the argument
// count, floats read right to left. Error string 0x004d6a14.
// FUNCTION: VC1 0x004be9e0
SQInteger Vector2::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Vector2* const vector = static_cast<Vector2*>(Vector2::operator new(sizeof(Vector2)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (vector) Vector2();
        break;
    case 2:
        ::new (vector) Vector2(sa.GetFloat(2));
        break;
    case 3:
        ::new (vector) Vector2(sa.GetFloat(2), sa.GetFloat(3));
        break;
    default:
        Vector2::operator delete(vector);
        return sa.ThrowError("constructVector2: invalid arguments");
    }
    return SqPlus::PostConstruct<Vector2>(vm, vector, SqPlus::ConstructReleaseClass<Vector2>::release);
}

}
