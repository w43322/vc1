#include "bindings/value_types.h"
#include "bindings/squirrel_object.h"
#include "memory/fixed_pool.h"

// Reccmp harness entry; not the original WinMain.
// Address-takes keep recovered callbacks in the LTCG image. Not tests.
int main()
{
    vc1::InitSizeClassPools();
    vc1::Color color;
    volatile SQUnsignedInteger value = 0xff112233;
    color.argb = value;
    volatile unsigned keepA = color.GetA();
    volatile unsigned keepR = color.GetR();
    volatile unsigned keepG = color.GetG();
    volatile unsigned keepB = color.GetB();
    volatile unsigned keepV = color.GetValue();
    volatile SQFloat keepBlue = color.GetBlue();
    volatile SQFloat keepGreen = color.GetGreen();
    volatile SQFloat keepRed = color.GetRed();
    volatile SQFloat keepAlpha = color.GetAlpha();
    vc1::Color otherColor;
    otherColor.argb = value;
    volatile SQInteger keepColorCmp = color.Cmp(&otherColor);
    vc1::Vector2 vec2;
    vec2.x = 3.0f;
    vec2.y = 4.0f;
    volatile SQInteger keepVec2Cmp = vec2.Cmp(&vec2);
    volatile SQFloat keepVec2Sq = vec2.GetSquareMagnitude();
    volatile SQFloat keepVec2Mag = vec2.GetMagnitude();
    vec2.Normalize();
    vc1::Vector2 vec2n;
    volatile vc1::Vector2* keepVec2N = vec2.GetNormalized(&vec2n);
    vc1::Vector2 vec2u;
    volatile vc1::Vector2* keepVec2U = vec2.Unm(&vec2u);
    vc1::Vector2 vec2d;
    volatile vc1::Vector2* keepVec2D = vec2.Div(&vec2d, 2.0f);
    vc1::Vector2 vec2m;
    volatile vc1::Vector2* keepVec2M = vec2.Mul(&vec2m, 2.0f);
    vc1::Vector2 vec2s;
    volatile vc1::Vector2* keepVec2S = vec2.Sub(&vec2s, &vec2);
    vc1::Vector2 vec2a;
    volatile vc1::Vector2* keepVec2A = vec2.Add(&vec2a, &vec2);
    volatile SQBool keepVec2Eq = vec2.Equal(&vec2);
    volatile SQBool keepVec2Valid = vec2.IsValid();
    volatile SQFloat keepVec2Dot = vc1::Vector2::GetDotProduct(&vec2, &vec2);
    vc1::Vector3 vec3;
    vec3.x = 1.0f;
    vec3.y = 2.0f;
    vec3.z = 2.0f;
    volatile SQInteger keepVec3Cmp = vec3.Cmp(&vec3);
    volatile SQFloat keepVec3Sq = vec3.GetSquareMagnitude();
    volatile SQFloat keepVec3Mag = vec3.GetMagnitude();
    vec3.Normalize();
    vc1::Vector3 vec3n;
    volatile vc1::Vector3* keepVec3N = vec3.GetNormalized(&vec3n);
    vc1::Vector3 vec3u;
    volatile vc1::Vector3* keepVec3U = vec3.Unm(&vec3u);
    vc1::Vector3 vec3d;
    volatile vc1::Vector3* keepVec3D = vec3.Div(&vec3d, 2.0f);
    vc1::Vector3 vec3m;
    volatile vc1::Vector3* keepVec3M = vec3.Mul(&vec3m, 2.0f);
    vc1::Vector3 vec3s;
    volatile vc1::Vector3* keepVec3S = vec3.Sub(&vec3s, &vec3);
    vc1::Vector3 vec3a;
    volatile vc1::Vector3* keepVec3A = vec3.Add(&vec3a, &vec3);
    vc1::Vector4 vec4;
    vec4.x = 1.0f;
    vec4.y = 2.0f;
    vec4.z = 2.0f;
    vec4.w = 4.0f;
    volatile SQInteger keepVec4Cmp = vec4.Cmp(&vec4);
    volatile SQFloat keepVec4Sq = vec4.GetSquareMagnitude();
    volatile SQFloat keepVec4Mag = vec4.GetMagnitude();
    vec4.Normalize();
    vc1::Vector4 vec4n;
    volatile vc1::Vector4* keepVec4N = vec4.GetNormalized(&vec4n);
    vc1::Vector4 vec4u;
    volatile vc1::Vector4* keepVec4U = vec4.Unm(&vec4u);
    vc1::Vector4 vec4d;
    volatile vc1::Vector4* keepVec4D = vec4.Div(&vec4d, 2.0f);
    vc1::Vector4 vec4m;
    volatile vc1::Vector4* keepVec4M = vec4.Mul(&vec4m, 2.0f);
    vc1::Vector4 vec4s;
    volatile vc1::Vector4* keepVec4S = vec4.Sub(&vec4s, &vec4);
    vc1::Vector4 vec4a;
    volatile vc1::Vector4* keepVec4A = vec4.Add(&vec4a, &vec4);
    vc1::Matrix3 mat3;
    mat3.m[0] = 1.0f;
    mat3.m[1] = 0.0f;
    mat3.m[2] = 0.0f;
    mat3.m[3] = 0.0f;
    mat3.m[4] = 1.0f;
    mat3.m[5] = 0.0f;
    mat3.m[6] = 0.0f;
    mat3.m[7] = 0.0f;
    mat3.m[8] = 1.0f;
    volatile vc1::Vector2* keepM3X = mat3.GetXVec();
    volatile vc1::Vector2* keepM3Y = mat3.GetYVec();
    volatile vc1::Vector2* keepM3O = mat3.GetOrigin();
    mat3.SetXVec(&vec2);
    mat3.SetYVec(&vec2);
    mat3.SetOrigin(&vec2);
    mat3.MultVecs(&vec2);
    vc1::Vector2 mat3iv;
    volatile vc1::Vector2* keepM3IV = mat3.MultInvVrt(&mat3iv, &vec2);
    vc1::Matrix3 mat3n;
    volatile vc1::Matrix3* keepM3N = mat3.GetNormalized(&mat3n);
    mat3.Normalize();
    volatile SQInteger keepM3Cmp = mat3.Cmp(&mat3);
    vc1::Matrix3 mat3u;
    volatile vc1::Matrix3* keepM3U = mat3.Unm(&mat3u);
    vc1::Matrix3 mat3d;
    volatile vc1::Matrix3* keepM3D = mat3.Div(&mat3d, 2.0f);
    vc1::Matrix3 mat3m;
    volatile vc1::Matrix3* keepM3M = mat3.Mul(&mat3m, &mat3);
    vc1::Matrix3 mat3s;
    volatile vc1::Matrix3* keepM3S = mat3.Sub(&mat3s, &mat3);
    vc1::Matrix3 mat3a;
    volatile vc1::Matrix3* keepM3A = mat3.Add(&mat3a, &mat3);
    vc1::Vector3 mat3v;
    volatile vc1::Vector3* keepM3V = mat3.Mult(&mat3v, &vec3);
    vc1::Matrix4 mat4;
    {
        unsigned i;
        for (i = 0; i < 16; ++i) {
            mat4.m[i] = 0.0f;
        }
    }
    mat4.m[0] = 1.0f;
    mat4.m[5] = 1.0f;
    mat4.m[10] = 1.0f;
    mat4.m[15] = 1.0f;
    volatile SQInteger keepM4Cmp = mat4.Cmp(&mat4);
    vc1::Matrix4 mat4u;
    volatile vc1::Matrix4* keepM4U = mat4.Unm(&mat4u);
    vc1::Matrix4 mat4d;
    volatile vc1::Matrix4* keepM4D = mat4.Div(&mat4d, 2.0f);
    vc1::Matrix4 mat4m;
    volatile vc1::Matrix4* keepM4M = mat4.Mul(&mat4m, &mat4);
    vc1::Matrix4 mat4s;
    volatile vc1::Matrix4* keepM4S = mat4.Sub(&mat4s, &mat4);
    vc1::Matrix4 mat4a;
    volatile vc1::Matrix4* keepM4A = mat4.Add(&mat4a, &mat4);
    volatile unsigned char ba = static_cast<unsigned char>(value);
    volatile unsigned char br = static_cast<unsigned char>(value >> 8);
    volatile unsigned char bg = static_cast<unsigned char>(value >> 16);
    volatile unsigned char bb = static_cast<unsigned char>(value >> 24);
    color.SetValue(vc1::Color::MakeARGB(ba, br, bg, bb));
    (void)keepA;
    (void)keepR;
    (void)keepG;
    (void)keepB;
    (void)keepV;
    (void)keepBlue;
    (void)keepGreen;
    (void)keepRed;
    (void)keepAlpha;
    (void)keepColorCmp;
    (void)keepVec2Cmp;
    (void)keepVec2Sq;
    (void)keepVec2Mag;
    (void)keepVec2N;
    (void)keepVec2U;
    (void)keepVec2D;
    (void)keepVec2M;
    (void)keepVec2S;
    (void)keepVec2A;
    (void)keepVec2Eq;
    (void)keepVec2Valid;
    (void)keepVec2Dot;
    (void)keepVec3Cmp;
    (void)keepVec3Sq;
    (void)keepVec3Mag;
    (void)keepVec3N;
    (void)keepVec3U;
    (void)keepVec3D;
    (void)keepVec3M;
    (void)keepVec3S;
    (void)keepVec3A;
    (void)keepVec4Cmp;
    (void)keepVec4Sq;
    (void)keepVec4Mag;
    (void)keepVec4N;
    (void)keepVec4U;
    (void)keepVec4D;
    (void)keepVec4M;
    (void)keepVec4S;
    (void)keepVec4A;
    (void)keepM3X;
    (void)keepM3Y;
    (void)keepM3O;
    (void)keepM3IV;
    (void)keepM3N;
    (void)keepM3Cmp;
    (void)keepM3U;
    (void)keepM3D;
    (void)keepM3M;
    (void)keepM3S;
    (void)keepM3A;
    (void)keepM3V;
    (void)keepM4Cmp;
    (void)keepM4U;
    (void)keepM4D;
    (void)keepM4M;
    (void)keepM4S;
    (void)keepM4A;
    volatile SQFUNCTION keep[] = {
        vc1::Color::Constructor,
        vc1::Color::ConstructArgs,
        vc1::Color::TypeOf,
        vc1::Vector2::Constructor,
        vc1::Vector2::ConstructArgs,
        vc1::Vector2::TypeOf,
        vc1::Vector3::Constructor,
        vc1::Vector3::ConstructArgs,
        vc1::Vector3::TypeOf,
        vc1::Vector4::Constructor,
        vc1::Vector4::ConstructArgs,
        vc1::Vector4::TypeOf,
        vc1::Matrix3::Constructor,
        vc1::Matrix3::ConstructArgs,
        vc1::Matrix3::TypeOf,
        vc1::Matrix4::Constructor,
        vc1::Matrix4::ConstructArgs,
        vc1::Matrix4::TypeOf,
        vc1::Quaternion::Constructor,
        vc1::Quaternion::ConstructArgs,
        vc1::Quaternion::TypeOf,
        vc1::Rect::Constructor,
        vc1::Rect::ConstructArgs,
        vc1::Rect::TypeOf,
        vc1::RectF::Constructor,
        vc1::RectF::ConstructArgs,
        vc1::RectF::TypeOf
    };
    volatile void* keepBind[] = {
        (void*)&vc1::AttachStackTop,
        (void*)&vc1::AssignSquirrelObject,
        (void*)&vc1::CreateNativeClass,
        (void*)&vc1::MemCompare,
        (void*)&vc1::RegisterNativeSlot,
        (void*)&vc1::RegisterIntegerSlot,
        (void*)&vc1::Color::RegisterClass,
        (void*)&vc1::Color::CreateBinding,
        (void*)&vc1::Color::BindArgConstructor,
        (void*)&vc1::Vector2::RegisterClass,
        (void*)&vc1::Vector2::CreateBinding,
        (void*)&vc1::Vector2::BindArgConstructor,
        (void*)&vc1::Vector3::RegisterClass,
        (void*)&vc1::Vector3::CreateBinding,
        (void*)&vc1::Vector3::BindArgConstructor,
        (void*)&vc1::Vector4::RegisterClass,
        (void*)&vc1::Vector4::CreateBinding,
        (void*)&vc1::Vector4::BindArgConstructor,
        (void*)&vc1::Matrix3::RegisterClass,
        (void*)&vc1::Matrix3::CreateBinding,
        (void*)&vc1::Matrix3::BindArgConstructor,
        (void*)&vc1::Matrix4::RegisterClass,
        (void*)&vc1::Matrix4::CreateBinding,
        (void*)&vc1::Matrix4::BindArgConstructor,
        (void*)&vc1::Quaternion::RegisterClass,
        (void*)&vc1::Quaternion::CreateBinding,
        (void*)&vc1::Quaternion::BindArgConstructor,
        (void*)&vc1::Rect::RegisterClass,
        (void*)&vc1::Rect::CreateBinding,
        (void*)&vc1::Rect::BindArgConstructor,
        (void*)&vc1::RectF::RegisterClass,
        (void*)&vc1::RectF::CreateBinding,
        (void*)&vc1::RectF::BindArgConstructor
    };
    (void)keep;
    (void)keepBind;
    vc1::DestroySizeClassPools();
    return 0;
}
