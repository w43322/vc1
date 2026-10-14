#include "matrix3.h"
#include "value_types.h"
#include "kd/factory.h"

#include <math.h>
#include <new>

namespace vc1 {

// Script constant "Identity" (0x004d73a8); identity in the original .data.
// GLOBAL: VC1 0x004e9808
Matrix3 Matrix3::Identity;

// Script constant "Null" (0x004d72bc); zeros with _33 = 1 in the original .data.
// GLOBAL: VC1 0x004e982c
Matrix3 Matrix3::Null(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

// Bound as "GetOrigin" (0x004d732c) via 0x00421b00 from 0x004c0c96.
// Returns &_31. this in ECX.
// FUNCTION: VC1 0x00401e60
Vector2& Matrix3::GetOrigin()
{
    return *reinterpret_cast<Vector2*>(&_31);
}

// Bound as "SetOrigin" (0x004d7348) via 0x00421bb0 from 0x004c0ce8.
// this in ECX; RET 4.
// FUNCTION: VC1 0x00401e70
void Matrix3::SetOrigin(const Vector2& origin)
{
    _31 = origin.x;
    _32 = origin.y;
}

// Bound as "GetYVec" (0x004d7324) via 0x00421b00 from 0x004c0c7a.
// Returns &_21. this in ECX.
// FUNCTION: VC1 0x004060a0
Vector2& Matrix3::GetYVec()
{
    return *reinterpret_cast<Vector2*>(&_21);
}

// Bound as "GetXVec" (0x004d731c) via 0x00421b00 from 0x004c0c5e.
// Returns this as a Vector2. this in ECX. /OPT:ICF folded with Matrix4::GetXVec.
// FUNCTION: VC1 0x004060b0 FOLDED
Vector2& Matrix3::GetXVec()
{
    return *reinterpret_cast<Vector2*>(&_11);
}

// Bound as "SetYVec" (0x004d7340) via 0x00421bb0 from 0x004c0cce.
// this in ECX; RET 4.
// FUNCTION: VC1 0x0040cdb0
void Matrix3::SetYVec(const Vector2& axis)
{
    _21 = axis.x;
    _22 = axis.y;
}

// Bound as "SetXVec" (0x004d7338) via 0x00421bb0 from 0x004c0cb2.
// this in ECX; RET 4.
// FUNCTION: VC1 0x0040cdd0
void Matrix3::SetXVec(const Vector2& axis)
{
    _11 = axis.x;
    _12 = axis.y;
}

// Bound as "MultVecs" (0x004d7380) via 0x00421bb0 from 0x004c0d94.
// Scales X by v.x and Y by v.y. this in ECX; scale=[esp+4]; RET 4.
// FUNCTION: VC1 0x00427dc0
void Matrix3::MultVecs(const Vector2& scale)
{
    GetXVec() *= scale.x;
    GetYVec() *= scale.y;
}

// Bound as "MultInvVrt" (0x004d7368) via 0x00421dc0 from 0x004c0d5c.
// out = (v-origin) dotted with orthonormal X/Y. this in ECX; RET 8.
// FUNCTION: VC1 0x00427e00
Vector2 Matrix3::MultInvVrt(const Vector2& v) const
{
    const SQFloat dx = v.x - _31;
    const SQFloat dy = v.y - _32;
    return Vector2(dx * _11 + dy * _12, dx * _21 + dy * _22);
}

// Bound as "GetNormalized" (0x004d7248) via 0x004218f0 from 0x004c0d15.
// Unit X/Y, copy origin, _13=_23=0, _33=1. sqrt 0x004c4280. RET 4.
// Assembly takes the length of Y first, then X.
// FUNCTION: VC1 0x00427e50
Matrix3 Matrix3::GetNormalized() const
{
    const Vector2& x = reinterpret_cast<const Vector2&>(_11);
    const Vector2& y = reinterpret_cast<const Vector2&>(_21);
    const Vector2& origin = reinterpret_cast<const Vector2&>(_31);
    return Matrix3(x.GetNormalized(), y.GetNormalized(), origin);
}

// In-place unit X/Y, _13=_23=0, _33=1. sqrt helper 0x004c4280. this in ESI/ECX.
// Bound as "Normalize" via 0x00421c60 (dispatcher 0x00424060).
// FUNCTION: VC1 0x00427f20
void Matrix3::Normalize()
{
    SetXVec(GetXVec().GetNormalized());
    SetYVec(GetYVec().GetNormalized());
    _13 = 0.0f;
    _23 = 0.0f;
    _33 = 1.0f;
}

// Bound as "_tostring" (0x004d3710) via 0x00421a50. this in ECX. Each row goes
// through Vector3::ToString, last row first; all three return the same static
// buffer, so the original prints the first row three times. Static KdString
// 0x004ee458 (guard 0x004ee468) with 0x100 reserved characters. Format 0x004d752c.
// FUNCTION: VC1 0x00427fe0
KdString& Matrix3::ToString() const
{
    // GLOBAL: VC1 0x004ee458
    static KdString text(0x100, '\0');
    text.Format("Matrix3(%s, %s, %s)", Vector3(_11, _12, _13).ToString().c_str(),
        Vector3(_21, _22, _23).ToString().c_str(), Vector3(_31, _32, _33).ToString().c_str());
    return text;
}

// Bound as "_cmp" (0x004d36e0) via 0x004219a0. this in EDX; other on stack; size 0x24.
// FUNCTION: VC1 0x00428110
SQInteger Matrix3::Cmp(const Matrix3& other) const
{
    return MemCompare(this, &other, sizeof(Matrix3));
}

// Bound as "_unm" (0x004d36d0) via 0x004218f0 from 0x004c0c24.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428130
Matrix3 Matrix3::Unm() const
{
    return Matrix3(-_11, -_12, -_13, -_21, -_22, -_23, -_31, -_32, -_33);
}

// Bound as "_div" (0x004d36c8) via 0x00421840 from 0x004c0c15.
// FLD scalar, EAX=out, PUSH this, FSTP [esp], CALL 0x00474370, RET 8.
// FUNCTION: VC1 0x00428180
Matrix3 Matrix3::Div(SQFloat scalar) const
{
    return *this / scalar;
}

// Bound as "_mul" (0x004d36c0) via 0x00421790 from 0x004c0bf7.
// EAX=out, EDX=this, ECX=other, CALL 0x00474470, RET 8.
// FUNCTION: VC1 0x004281a0
Matrix3 Matrix3::Mul(const Matrix3& other) const
{
    return *this * other;
}

// Bound as "_sub" (0x004d36b8) via 0x00421790 from 0x004c0bdd.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x004281c0
Matrix3 Matrix3::Sub(const Matrix3& other) const
{
    return Matrix3(_11 - other._11, _12 - other._12, _13 - other._13,
        _21 - other._21, _22 - other._22, _23 - other._23,
        _31 - other._31, _32 - other._32, _33 - other._33);
}

// Bound as "_add" (0x004d36b0) via 0x00421790 from 0x004c0bc1.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428220
Matrix3 Matrix3::Add(const Matrix3& other) const
{
    return Matrix3(_11 + other._11, _12 + other._12, _13 + other._13,
        _21 + other._21, _22 + other._22, _23 + other._23,
        _31 + other._31, _32 + other._32, _33 + other._33);
}

// Inferred name. LTCG ABI: EAX=this (updated in place), ECX=left. Each column
// of this becomes left * column, i.e. this = left * this. Returns this.
// FUNCTION: VC1 0x00473cd0
Matrix3* Matrix3::LeftMultiply(const Matrix3* left)
{
    float a;
    float b;
    float c;
    int col;

    for (col = 0; col < 3; col++) {
        a = m[col];
        b = m[col + 3];
        c = m[col + 6];
        m[col] = left->m[0] * a + left->m[1] * b + left->m[2] * c;
        m[col + 3] = left->m[3] * a + left->m[4] * b + left->m[5] * c;
        m[col + 6] = left->m[6] * a + left->m[7] * b + left->m[8] * c;
    }
    return this;
}

// Matrix3(x, y, origin). LTCG ABI: EAX=this, ESI=x, EDX=y, ECX=origin.
// Called from ConstructArgs with three Vector2 arguments.
// FUNCTION: VC1 0x00473e50
Matrix3::Matrix3(const Vector2& x, const Vector2& y, const Vector2& origin)
    : _11(x.x), _12(x.y), _13(0.0f), _21(y.x), _22(y.y), _23(0.0f),
      _31(origin.x), _32(origin.y), _33(1.0f)
{
}

// Matrix3(origin): identity axes with a translation. EAX=this, ECX=origin.
// Inlined into IGraphics::Translate (0x00418830).
// FUNCTION: VC1 0x00473e80
Matrix3::Matrix3(const Vector2& origin)
{
    _11 = 1.0f;
    _21 = 0.0f;
    _31 = origin.x;
    _12 = 0.0f;
    _22 = 1.0f;
    _32 = origin.y;
    _13 = 0.0f;
    _23 = 0.0f;
    _33 = 1.0f;
}

// Bound as "Transpose" (0x004d739c) via 0x00421e70 from 0x004c0ded.
// cdecl; out=[esp+4], src=[esp+8].
// FUNCTION: VC1 0x004740d0
Matrix3 Matrix3::Transpose(const Matrix3& src)
{
    return Matrix3(src._11, src._21, src._31, src._12, src._22, src._32,
        src._13, src._23, src._33);
}

// Bound as "Inverse" (0x004d7394) via 0x00421e70 from 0x004c0dd1.
// cdecl; out=[esp+4], src=[esp+8]. Adjugate / det.
// FUNCTION: VC1 0x00474110
Matrix3 Matrix3::Inverse(const Matrix3& src)
{
    const SQFloat a = src._11;
    const SQFloat b = src._12;
    const SQFloat c = src._13;
    const SQFloat d = src._21;
    const SQFloat e = src._22;
    const SQFloat f = src._23;
    const SQFloat g = src._31;
    const SQFloat h = src._32;
    const SQFloat i = src._33;
    const SQFloat c0 = e * i - f * h;
    const SQFloat c1 = g * f - d * i;
    const SQFloat c2 = h * d - g * e;
    const SQFloat invDet = 1.0f / (b * c1 + a * c0 + c * c2);
    return Matrix3(invDet * c0, invDet * (c * h - i * b), (f * b - e * c) * invDet,
        invDet * c1, invDet * (a * i - g * c), invDet * (d * c - f * a),
        invDet * c2, invDet * (b * g - h * a), invDet * (e * a - b * d));
}

// Bound as "MultVrt" (0x004d738c) via 0x00421dc0 from 0x004c0d70.
// out = v * this as a point (origin added). this in ECX; RET 8.
// FUNCTION: VC1 0x00474280
Vector2 Matrix3::MultVrt(const Vector2& v) const
{
    return Vector2(_21 * v.y + v.x * _11 + _31, _12 * v.x + _22 * v.y + _32);
}

// Bound as "MultInvVect" (0x004d735c) via 0x00421dc0 from 0x004c0d4a.
// out = (X.v, Y.v). this in ECX; RET 8.
// FUNCTION: VC1 0x004742b0
Vector2 Matrix3::MultInvVect(const Vector2& v) const
{
    return Vector2(_11 * v.x + _12 * v.y, _21 * v.x + _22 * v.y);
}

// Bound as "MultVect" (0x004d7374) via 0x00421dc0 from 0x004c0d82.
// out = v * 2x2 linear part. this in ECX; RET 8.
// FUNCTION: VC1 0x004742e0
Vector2 Matrix3::MultVect(const Vector2& v) const
{
    return Vector2(_11 * v.x + _21 * v.y, _12 * v.x + _22 * v.y);
}

// Bound as "Mult" (0x004d7354) via 0x00421d10 from 0x004c0d31.
// out = v * this. this in ECX; RET 8.
// FUNCTION: VC1 0x00474310
Vector3 Matrix3::Mult(const Vector3& v) const
{
    return Vector3(_11 * v.x + _21 * v.y + _31 * v.z, _12 * v.x + _22 * v.y + _32 * v.z,
        _13 * v.x + _23 * v.y + _33 * v.z);
}

// operator/ behind "_div" (0x00428180): this in ECX, scalar on the stack,
// out in EAX. inv = 1/scalar; first and last written as inv*this.
// FUNCTION: VC1 0x00474370
Matrix3 Matrix3::operator/(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Matrix3(inv * _11, _12 * inv, _13 * inv, _21 * inv, _22 * inv, _23 * inv,
        _31 * inv, _32 * inv, inv * _33);
}

// operator* behind "_mul" (0x004281a0): EDX=this, ECX=other, EAX=out.
// out row r = row r of other times this.
// FUNCTION: VC1 0x00474470
Matrix3 Matrix3::operator*(const Matrix3& o) const
{
    return Matrix3(
        o._11 * _11 + o._12 * _21 + o._13 * _31,
        o._11 * _12 + o._12 * _22 + o._13 * _32,
        o._11 * _13 + o._12 * _23 + o._13 * _33,
        o._21 * _11 + o._22 * _21 + o._23 * _31,
        o._21 * _12 + o._22 * _22 + o._23 * _32,
        o._21 * _13 + o._22 * _23 + o._23 * _33,
        o._31 * _11 + o._32 * _21 + o._33 * _31,
        o._31 * _12 + o._32 * _22 + o._33 * _32,
        o._31 * _13 + o._32 * _23 + o._33 * _33);
}

// Matrix3(x, origin): Y is the left-hand orthogonal (-x.y, x.x) through a
// stack temporary. EAX=this, ECX=x, EDX=origin. Called from ConstructArgs with
// two Vector2 arguments.
// FUNCTION: VC1 0x00475080
Matrix3::Matrix3(const Vector2& x, const Vector2& origin)
{
    _11 = x.x;
    _12 = x.y;
    const Vector2 y = Vector2::GetOrthogonal(x);
    _21 = y.x;
    _22 = y.y;
    _31 = origin.x;
    _32 = origin.y;
    _13 = 0.0f;
    _23 = 0.0f;
    _33 = 1.0f;
}

// Registered from 0x0041e860 as "constructor" with typemask "*" (0x004d6b14).
// Switch on the argument count; Vector2 arguments through
// GetInstance<Vector2,false> (0x00429080). Error string 0x004d6a5c.
// FUNCTION: VC1 0x004be540
SQInteger Matrix3::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Matrix3* const matrix = static_cast<Matrix3*>(Matrix3::operator new(sizeof(Matrix3)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (matrix) Matrix3();
        return SqPlus::PostConstruct<Matrix3>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix3>::release);
    case 2: {
        Vector2* const origin = SqPlus::GetInstance<Vector2, false>(vm, 2);
        if (!origin) {
            break;
        }
        ::new (matrix) Matrix3(*origin);
        return SqPlus::PostConstruct<Matrix3>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix3>::release);
    }
    case 3: {
        Vector2* const x = SqPlus::GetInstance<Vector2, false>(vm, 2);
        Vector2* const origin = SqPlus::GetInstance<Vector2, false>(vm, 3);
        if (!x || !origin) {
            break;
        }
        ::new (matrix) Matrix3(*x, *origin);
        return SqPlus::PostConstruct<Matrix3>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix3>::release);
    }
    case 4: {
        Vector2* const x = SqPlus::GetInstance<Vector2, false>(vm, 2);
        Vector2* const y = SqPlus::GetInstance<Vector2, false>(vm, 3);
        Vector2* const origin = SqPlus::GetInstance<Vector2, false>(vm, 4);
        if (!x || !y || !origin) {
            break;
        }
        ::new (matrix) Matrix3(*x, *y, *origin);
        return SqPlus::PostConstruct<Matrix3>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix3>::release);
    }
    }
    Matrix3::operator delete(matrix);
    return sa.ThrowError("constructMatrix3: invalid arguments");
}

}
