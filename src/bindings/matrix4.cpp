#include "matrix4.h"
#include "value_types.h"
#include "kd/factory.h"

#include <new>

namespace vc1 {

// Script constant "Identity" (0x004d73a8); identity in the original .data.
// GLOBAL: VC1 0x004e9788
Matrix4 Matrix4::Identity;

// Script constant "Null" (0x004d72bc); zeros with _44 = 1 in the original .data.
// GLOBAL: VC1 0x004e97c8
Matrix4 Matrix4::Null(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

// Bound as "GetXVec" (0x004d731c) via 0x00422290 from 0x004c1005. Returns
// this; /OPT:ICF folded with Matrix3::GetXVec.
// FUNCTION: VC1 0x004060b0 FOLDED
Vector3& Matrix4::GetXVec()
{
    return *reinterpret_cast<Vector3*>(&_11);
}

// Bound as "GetOrigin" (0x004d732c) via 0x00422290 from 0x004c1059. Returns &_41.
// FUNCTION: VC1 0x004157f0
Vector3& Matrix4::GetOrigin()
{
    return *reinterpret_cast<Vector3*>(&_41);
}

// Bound as "GetYVec" (0x004d7324) via 0x00422290 from 0x004c1021. Returns &_21.
// FUNCTION: VC1 0x00415800
Vector3& Matrix4::GetYVec()
{
    return *reinterpret_cast<Vector3*>(&_21);
}

// Bound as "GetZVec" (0x004d73d0) via 0x00422290 from 0x004c103d. Returns &_31.
// FUNCTION: VC1 0x00415810
Vector3& Matrix4::GetZVec()
{
    return *reinterpret_cast<Vector3*>(&_31);
}

// Bound as "SetZVec" (0x004d73d8) via 0x00422340 from 0x004c10ad. RET 4.
// FUNCTION: VC1 0x00415820
void Matrix4::SetZVec(const Vector3& axis)
{
    _31 = axis.x;
    _32 = axis.y;
    _33 = axis.z;
}

// Bound as "SetYVec" (0x004d7340) via 0x00422340 from 0x004c1091. RET 4.
// FUNCTION: VC1 0x00415840
void Matrix4::SetYVec(const Vector3& axis)
{
    _21 = axis.x;
    _22 = axis.y;
    _23 = axis.z;
}

// Bound as "SetXVec" (0x004d7338) via 0x00422340 from 0x004c1075. RET 4.
// FUNCTION: VC1 0x00415860
void Matrix4::SetXVec(const Vector3& axis)
{
    _11 = axis.x;
    _12 = axis.y;
    _13 = axis.z;
}

// Matrix4(x, origin): Y is the normalized Vector3::GetOrthogonal(x)
// (0x004bc570, then GetNormalized 0x00415950), Z = x cross y. Built in a
// stack temporary and copied with REP MOVSD. ECX=x, EAX=origin, this on the
// stack. Called from ConstructArgs with two Vector3 arguments.
// FUNCTION: VC1 0x004159c0
Matrix4::Matrix4(const Vector3& x, const Vector3& origin)
{
    const Vector3 y = Vector3::GetOrthogonal(x).GetNormalized();
    *this = Matrix4(x, y, Vector3::GetCrossProduct(x, y), origin);
}

// Bound as "MultVecs" (0x004d7380) via 0x00422340 from 0x004c1173.
// Scales X, Y, Z by scale.x, .y, .z. this in ECX; RET 4.
// FUNCTION: VC1 0x004278c0
void Matrix4::MultVecs(const Vector3& scale)
{
    GetXVec() *= scale.x;
    GetYVec() *= scale.y;
    GetZVec() *= scale.z;
}

// Bound as "MultInvVrt" (0x004d7368) via 0x00422550 from 0x004c113b.
// out = (v-origin) dotted with X/Y/Z. this in ECX; RET 8.
// FUNCTION: VC1 0x00427940
Vector3 Matrix4::MultInvVrt(const Vector3& v) const
{
    const SQFloat dx = v.x - _41;
    const SQFloat dy = v.y - _42;
    const SQFloat dz = v.z - _43;
    return Vector3(dx * _11 + dy * _12 + dz * _13, dx * _21 + dy * _22 + dz * _23,
        dx * _31 + dy * _32 + dz * _33);
}

// Bound as "GetNormalized" (0x004d7248) via 0x00422080 from 0x004c10f4.
// Vector3::GetNormalized (0x00415950) on Z, Y, X, then the four-vector
// constructor with the origin. RET 4.
// FUNCTION: VC1 0x004279d0
Matrix4 Matrix4::GetNormalized() const
{
    const Vector3& x = reinterpret_cast<const Vector3&>(_11);
    const Vector3& y = reinterpret_cast<const Vector3&>(_21);
    const Vector3& z = reinterpret_cast<const Vector3&>(_31);
    const Vector3& origin = reinterpret_cast<const Vector3&>(_41);
    return Matrix4(x.GetNormalized(), y.GetNormalized(), z.GetNormalized(), origin);
}

// Bound as "Normalize" (0x004d723c) via 0x004223f0 (dispatcher 0x00424060).
// Unit X/Y/Z through Vector3::GetNormalized, then _34=_24=_14=0, _44=1.
// FUNCTION: VC1 0x00427a70
void Matrix4::Normalize()
{
    SetXVec(GetXVec().GetNormalized());
    SetYVec(GetYVec().GetNormalized());
    SetZVec(GetZVec().GetNormalized());
    _34 = 0.0f;
    _24 = 0.0f;
    _14 = 0.0f;
    _44 = 1.0f;
}

// Bound as "SetOrigin" (0x004d7348) via 0x00422340 from 0x004c10c9. RET 4.
// FUNCTION: VC1 0x00427af0
void Matrix4::SetOrigin(const Vector3& origin)
{
    _41 = origin.x;
    _42 = origin.y;
    _43 = origin.z;
}

// Bound as "_tostring" (0x004d3710) via 0x004221e0. this in ECX. Each row goes
// through Vector4::ToString, last row first; all four return the same static
// buffer, so the original prints the first row four times. Static KdString
// 0x004ee46c (guard 0x004ee47c) with 0x100 reserved characters.
// FUNCTION: VC1 0x00427b10
KdString& Matrix4::ToString() const
{
    // GLOBAL: VC1 0x004ee46c
    static KdString text(0x100, '\0');
    text.Format("Matrix4(%s, %s, %s, %s)",
        Vector4(_11, _12, _13, _14).ToString().c_str(),
        Vector4(_21, _22, _23, _24).ToString().c_str(),
        Vector4(_31, _32, _33, _34).ToString().c_str(),
        Vector4(_41, _42, _43, _44).ToString().c_str());
    return text;
}

// Bound as "_cmp" (0x004d36e0) via 0x00422130. this in EDX; other on stack; size 0x40.
// FUNCTION: VC1 0x00427c80
SQInteger Matrix4::Cmp(const Matrix4& other) const
{
    return MemCompare(this, &other, sizeof(Matrix4));
}

// Unary operator- behind "_unm" (0x00427d20): ECX=this, EAX=out.
// FUNCTION: VC1 0x00427ca0
Matrix4 Matrix4::operator-() const
{
    return Matrix4(-_11, -_12, -_13, -_14, -_21, -_22, -_23, -_24,
        -_31, -_32, -_33, -_34, -_41, -_42, -_43, -_44);
}

// Bound as "_unm" (0x004d36d0) via 0x00422080 from 0x004c0fcb.
// ESI=out, EAX=out, CALL 0x00427ca0, RET 4.
// FUNCTION: VC1 0x00427d20
Matrix4 Matrix4::Unm() const
{
    return -*this;
}

// Bound as "_div" (0x004d36c8) via 0x00421fd0 from 0x004c0fb2.
// FLD scalar, EAX=out, PUSH this, FSTP [esp], CALL 0x00474cb0, RET 8.
// FUNCTION: VC1 0x00427d40
Matrix4 Matrix4::Div(SQFloat scalar) const
{
    return *this / scalar;
}

// Bound as "_mul" (0x004d36c0) via 0x00421f20 from 0x004c0fa0.
// EAX=out, EDX=this, ECX=other, CALL 0x00474e70, RET 8.
// FUNCTION: VC1 0x00427d60
Matrix4 Matrix4::Mul(const Matrix4& other) const
{
    return *this * other;
}

// Bound as "_sub" (0x004d36b8) via 0x00421f20 from 0x004c0f84.
// EAX=out, EDX=this, ECX=other, CALL 0x00474d50, RET 8.
// FUNCTION: VC1 0x00427d80
Matrix4 Matrix4::Sub(const Matrix4& other) const
{
    return *this - other;
}

// Bound as "_add" (0x004d36b0) via 0x00421f20 from 0x004c0f68.
// EAX=out, EDX=this, ECX=other, CALL 0x00474de0, RET 8.
// FUNCTION: VC1 0x00427da0
Matrix4 Matrix4::Add(const Matrix4& other) const
{
    return *this + other;
}

// Bound as "Mult" (0x004d7354) via 0x004224a0 (dispatcher 0x004241f0, folded
// with Vector4's). out = v * this. this in ECX; RET 8.
// FUNCTION: VC1 0x00473ed0
Vector4 Matrix4::Mult(const Vector4& v) const
{
    return Vector4(_11 * v.x + _21 * v.y + _31 * v.z + _41 * v.w,
        _12 * v.x + _22 * v.y + _32 * v.z + _42 * v.w,
        _13 * v.x + _23 * v.y + _33 * v.z + _43 * v.w,
        _14 * v.x + _24 * v.y + _34 * v.z + _44 * v.w);
}

// Matrix4(x, y, z, origin), written column by column. LTCG ABI: EDI=x,
// ESI=y, EDX=z, ECX=origin, this on the stack; RET 4.
// FUNCTION: VC1 0x00473fe0
Matrix4::Matrix4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& origin)
{
    _11 = x.x;
    _21 = y.x;
    _31 = z.x;
    _41 = origin.x;
    _12 = x.y;
    _22 = y.y;
    _32 = z.y;
    _42 = origin.y;
    _13 = x.z;
    _23 = y.z;
    _33 = z.z;
    _43 = origin.z;
    _14 = 0.0f;
    _24 = 0.0f;
    _34 = 0.0f;
    _44 = 1.0f;
}

// Matrix4(origin): identity axes with a translation, column by column.
// EAX=this, ECX=origin.
// FUNCTION: VC1 0x00474040
Matrix4::Matrix4(const Vector3& origin)
{
    _11 = 1.0f;
    _21 = 0.0f;
    _31 = 0.0f;
    _41 = origin.x;
    _12 = 0.0f;
    _22 = 1.0f;
    _32 = 0.0f;
    _42 = origin.y;
    _13 = 0.0f;
    _23 = 0.0f;
    _33 = 1.0f;
    _43 = origin.z;
    _14 = 0.0f;
    _24 = 0.0f;
    _34 = 0.0f;
    _44 = 1.0f;
}

// Identity, column by column. EAX=this.
// FUNCTION: VC1 0x00474090
Matrix4::Matrix4()
{
    _11 = 1.0f;
    _21 = 0.0f;
    _31 = 0.0f;
    _41 = 0.0f;
    _12 = 0.0f;
    _22 = 1.0f;
    _32 = 0.0f;
    _42 = 0.0f;
    _13 = 0.0f;
    _23 = 0.0f;
    _33 = 1.0f;
    _43 = 0.0f;
    _14 = 0.0f;
    _24 = 0.0f;
    _34 = 0.0f;
    _44 = 1.0f;
}

// Bound as "Transpose" (0x004d739c) via 0x00422600 from 0x004c11c7.
// cdecl; out=[esp+4], src=[esp+8].
// FUNCTION: VC1 0x00474550
Matrix4 Matrix4::Transpose(const Matrix4& src)
{
    return Matrix4(src._11, src._21, src._31, src._41, src._12, src._22, src._32, src._42,
        src._13, src._23, src._33, src._43, src._14, src._24, src._34, src._44);
}

// Bound as "Inverse" (0x004d7394) via 0x00422600 from 0x004c11ab.
// cdecl; out=[esp+4], src=[esp+8]. Cofactor expansion.
// FUNCTION: VC1 0x004745c0
Matrix4 Matrix4::Inverse(const Matrix4& src)
{
    const SQFloat m10 = src.m[1];
    const SQFloat m20 = src.m[2];
    const SQFloat m30 = src.m[3];
    const SQFloat m11 = src.m[5];
    const SQFloat m21 = src.m[6];
    const SQFloat m31 = src.m[7];
    const SQFloat m12 = src.m[9];
    const SQFloat m22 = src.m[10];
    const SQFloat m32 = src.m[11];
    const SQFloat m13 = src.m[13];
    const SQFloat m23 = src.m[14];
    const SQFloat m33 = src.m[15];
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
    const SQFloat m00 = src.m[0];
    const SQFloat m01 = src.m[4];
    const SQFloat m02 = src.m[8];
    const SQFloat m03 = src.m[12];
    const SQFloat invDet = 1.0f / (m01 * c1 + m00 * c0 + m02 * c2 + m03 * c3);
    return Matrix4(
        invDet * c0,
        invDet * c1,
        invDet * c2,
        invDet * c3,
        invDet * (m03 * v0 + m02 * u0 + m01 * t0),
        (m03 * v1 + m00 * s0 + m02 * u1) * invDet,
        (m03 * v2 + m00 * s1 + m01 * t1) * invDet,
        (m02 * u2 + m00 * s2 + m01 * t2) * invDet,
        ((m32 * m11 - m12 * m31) * m03 + (m33 * m12 - m13 * m32) * m01
            + (m13 * m31 - m33 * m11) * m02) * invDet,
        ((m12 * m30 - m32 * m10) * m03 + (m13 * m32 - m33 * m12) * m00
            + (m33 * m10 - m13 * m30) * m02) * invDet,
        ((m31 * m10 - m11 * m30) * m03 + (m33 * m11 - m13 * m31) * m00
            + (m13 * m30 - m33 * m10) * m01) * invDet,
        ((m11 * m30 - m31 * m10) * m02 + (m12 * m31 - m32 * m11) * m00
            + (m32 * m10 - m12 * m30) * m01) * invDet,
        ((m12 * m21 - m22 * m11) * m03 + (m22 * m13 - m23 * m12) * m01
            + (m23 * m11 - m13 * m21) * m02) * invDet,
        ((m22 * m10 - m20 * m12) * m03 + (m23 * m12 - m22 * m13) * m00
            + (m20 * m13 - m23 * m10) * m02) * invDet,
        invDet * ((m13 * m21 - m23 * m11) * m00 + (m23 * m10 - m20 * m13) * m01
            + (m11 * m20 - m21 * m10) * m03),
        ((m21 * m10 - m11 * m20) * m02 + (m20 * m12 - m22 * m10) * m01
            + (m22 * m11 - m12 * m21) * m00) * invDet);
}

// Bound as "MultVrt" (0x004d738c) via 0x00422550 from 0x004c118f.
// out = v * this as a point (origin added). this in ECX; RET 8.
// FUNCTION: VC1 0x00474b90
Vector3 Matrix4::MultVrt(const Vector3& v) const
{
    return Vector3(_11 * v.x + _21 * v.y + _31 * v.z + _41,
        _12 * v.x + _22 * v.y + _32 * v.z + _42,
        _13 * v.x + _23 * v.y + _33 * v.z + _43);
}

// Bound as "MultInvVect" (0x004d735c) via 0x00422550 from 0x004c111f.
// out = (X.v, Y.v, Z.v). this in ECX; RET 8.
// FUNCTION: VC1 0x00474bf0
Vector3 Matrix4::MultInvVect(const Vector3& v) const
{
    return Vector3(_11 * v.x + _12 * v.y + _13 * v.z, _21 * v.x + _22 * v.y + _23 * v.z,
        _31 * v.x + _32 * v.y + _33 * v.z);
}

// Bound as "MultVect" (0x004d7374) via 0x00422550 from 0x004c1157.
// out = v * 3x3 linear part. this in ECX; RET 8.
// FUNCTION: VC1 0x00474c50
Vector3 Matrix4::MultVect(const Vector3& v) const
{
    return Vector3(_11 * v.x + _21 * v.y + _31 * v.z, _12 * v.x + _22 * v.y + _32 * v.z,
        _13 * v.x + _23 * v.y + _33 * v.z);
}

// operator/ behind "_div" (0x00427d40): this in ECX, scalar on the stack,
// out in EAX. inv = 1/scalar; first and last written as inv*this.
// FUNCTION: VC1 0x00474cb0
Matrix4 Matrix4::operator/(SQFloat scalar) const
{
    const SQFloat inv = 1.0f / scalar;
    return Matrix4(inv * _11, _12 * inv, _13 * inv, _14 * inv,
        _21 * inv, _22 * inv, _23 * inv, _24 * inv,
        _31 * inv, _32 * inv, _33 * inv, _34 * inv,
        _41 * inv, _42 * inv, _43 * inv, inv * _44);
}

// Binary operator- behind "_sub" (0x00427d80): EDX=this, ECX=other, EAX=out.
// FUNCTION: VC1 0x00474d50
Matrix4 Matrix4::operator-(const Matrix4& o) const
{
    return Matrix4(_11 - o._11, _12 - o._12, _13 - o._13, _14 - o._14,
        _21 - o._21, _22 - o._22, _23 - o._23, _24 - o._24,
        _31 - o._31, _32 - o._32, _33 - o._33, _34 - o._34,
        _41 - o._41, _42 - o._42, _43 - o._43, _44 - o._44);
}

// operator+ behind "_add" (0x00427da0): EDX=this, ECX=other, EAX=out;
// each element loads other first.
// FUNCTION: VC1 0x00474de0
Matrix4 Matrix4::operator+(const Matrix4& o) const
{
    return Matrix4(o._11 + _11, o._12 + _12, o._13 + _13, o._14 + _14,
        o._21 + _21, o._22 + _22, o._23 + _23, o._24 + _24,
        o._31 + _31, o._32 + _32, o._33 + _33, o._34 + _34,
        o._41 + _41, o._42 + _42, o._43 + _43, o._44 + _44);
}

// operator* behind "_mul" (0x00427d60) and the D3D projection build in
// DisplayD3dSetBuffers: EDX=this, ECX=other, EAX=out. Row r of the result is
// row r of other times this.
// FUNCTION: VC1 0x00474e70
Matrix4 Matrix4::operator*(const Matrix4& o) const
{
    return Matrix4(
        o._11 * _11 + o._12 * _21 + o._13 * _31 + o._14 * _41,
        o._11 * _12 + o._12 * _22 + o._13 * _32 + o._14 * _42,
        o._11 * _13 + o._12 * _23 + o._13 * _33 + o._14 * _43,
        o._11 * _14 + o._12 * _24 + o._13 * _34 + o._14 * _44,
        o._21 * _11 + o._22 * _21 + o._23 * _31 + o._24 * _41,
        o._21 * _12 + o._22 * _22 + o._23 * _32 + o._24 * _42,
        o._21 * _13 + o._22 * _23 + o._23 * _33 + o._24 * _43,
        o._21 * _14 + o._22 * _24 + o._23 * _34 + o._24 * _44,
        o._31 * _11 + o._32 * _21 + o._33 * _31 + o._34 * _41,
        o._31 * _12 + o._32 * _22 + o._33 * _32 + o._34 * _42,
        o._31 * _13 + o._32 * _23 + o._33 * _33 + o._34 * _43,
        o._31 * _14 + o._32 * _24 + o._33 * _34 + o._34 * _44,
        o._41 * _11 + o._42 * _21 + o._43 * _31 + o._44 * _41,
        o._41 * _12 + o._42 * _22 + o._43 * _32 + o._44 * _42,
        o._41 * _13 + o._42 * _23 + o._43 * _33 + o._44 * _43,
        o._41 * _14 + o._42 * _24 + o._43 * _34 + o._44 * _44);
}

// Registered from 0x0041e9c0 as "constructor" with typemask "*" (0x004d6b14).
// Switch on the argument count; Vector3 arguments through
// GetInstance<Vector3,false> (0x00429020). Three vectors (four parameters)
// has no case. Error string 0x004d6a80.
// FUNCTION: VC1 0x004be3b0
SQInteger Matrix4::ConstructArgs(HSQUIRRELVM vm)
{
    StackHandler sa(vm);
    Matrix4* const matrix = static_cast<Matrix4*>(Matrix4::operator new(sizeof(Matrix4)));
    switch (sa.GetParamCount()) {
    case 1:
        ::new (matrix) Matrix4();
        return SqPlus::PostConstruct<Matrix4>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix4>::release);
    case 2: {
        Vector3* const origin = SqPlus::GetInstance<Vector3, false>(vm, 2);
        if (!origin) {
            break;
        }
        ::new (matrix) Matrix4(*origin);
        return SqPlus::PostConstruct<Matrix4>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix4>::release);
    }
    case 3: {
        Vector3* const x = SqPlus::GetInstance<Vector3, false>(vm, 2);
        Vector3* const origin = SqPlus::GetInstance<Vector3, false>(vm, 3);
        if (!x || !origin) {
            break;
        }
        ::new (matrix) Matrix4(*x, *origin);
        return SqPlus::PostConstruct<Matrix4>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix4>::release);
    }
    case 5: {
        Vector3* const x = SqPlus::GetInstance<Vector3, false>(vm, 2);
        Vector3* const y = SqPlus::GetInstance<Vector3, false>(vm, 3);
        Vector3* const z = SqPlus::GetInstance<Vector3, false>(vm, 4);
        Vector3* const origin = SqPlus::GetInstance<Vector3, false>(vm, 5);
        if (!x || !y || !z || !origin) {
            break;
        }
        ::new (matrix) Matrix4(*x, *y, *z, *origin);
        return SqPlus::PostConstruct<Matrix4>(vm, matrix, SqPlus::ConstructReleaseClass<Matrix4>::release);
    }
    }
    Matrix4::operator delete(matrix);
    return sa.ThrowError("constructMatrix4: invalid arguments");
}

}
