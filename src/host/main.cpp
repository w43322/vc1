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
