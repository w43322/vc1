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
    vc1::Vector2 vec2n;
    volatile vc1::Vector2* keepVec2N = vec2.GetNormalized(&vec2n);
    vc1::Vector3 vec3;
    vec3.x = 1.0f;
    vec3.y = 2.0f;
    vec3.z = 2.0f;
    volatile SQInteger keepVec3Cmp = vec3.Cmp(&vec3);
    volatile SQFloat keepVec3Sq = vec3.GetSquareMagnitude();
    volatile SQFloat keepVec3Mag = vec3.GetMagnitude();
    vc1::Vector3 vec3n;
    volatile vc1::Vector3* keepVec3N = vec3.GetNormalized(&vec3n);
    vc1::Vector4 vec4;
    vec4.x = 1.0f;
    vec4.y = 2.0f;
    vec4.z = 2.0f;
    vec4.w = 4.0f;
    volatile SQInteger keepVec4Cmp = vec4.Cmp(&vec4);
    volatile SQFloat keepVec4Sq = vec4.GetSquareMagnitude();
    volatile SQFloat keepVec4Mag = vec4.GetMagnitude();
    vc1::Vector4 vec4n;
    volatile vc1::Vector4* keepVec4N = vec4.GetNormalized(&vec4n);
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
    (void)keepVec3Cmp;
    (void)keepVec3Sq;
    (void)keepVec3Mag;
    (void)keepVec3N;
    (void)keepVec4Cmp;
    (void)keepVec4Sq;
    (void)keepVec4Mag;
    (void)keepVec4N;
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
        vc1::Vector4::TypeOf
    };
    volatile void* keepBind[] = {
        (void*)&vc1::AttachStackTop,
        (void*)&vc1::AssignSquirrelObject,
        (void*)&vc1::CreateNativeClass,
        (void*)&vc1::MemCompare,
        (void*)&vc1::RegisterNativeSlot,
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
        (void*)&vc1::Vector4::BindArgConstructor
    };
    (void)keep;
    (void)keepBind;
    vc1::DestroySizeClassPools();
    return 0;
}
