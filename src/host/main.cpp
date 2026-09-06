#include "bindings/value_types.h"
#include "bindings/squirrel_object.h"
#include "memory/fixed_pool.h"

// Reccmp harness entry; not the original WinMain.
// Address-takes keep recovered callbacks in the LTCG image. Not tests.
int main()
{
    vc1::InitSizeClassPools();
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
        (void*)&vc1::CreateNativeClass,
        (void*)&vc1::RegisterNativeSlot,
        (void*)&vc1::Color::RegisterClass,
        (void*)&vc1::Vector2::RegisterClass,
        (void*)&vc1::Vector3::RegisterClass,
        (void*)&vc1::Vector4::RegisterClass
    };
    (void)keep;
    (void)keepBind;
    vc1::DestroySizeClassPools();
    return 0;
}
