#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

// SQRELEASEHOOK; size unused. Puts 8 bytes back on pools[2].
// FUNCTION: VC1 0x0041dd00
SQInteger Vector2::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector2);
    }
    return 0;
}

// FUNCTION: VC1 0x0041e3b0
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook)
{
    sq_setinstanceup(vm, 1, nativeObject);
    sq_setreleasehook(vm, 1, releaseHook);
    return 1;
}

// Squirrel class constructor registered at 0x0041eff0 as "constructor" with null typemask.
// Does not read extra stack args. Arg-taking factory is 0x004bed80 (not mapped yet).
// FUNCTION: VC1 0x0041fe10
SQInteger Color::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Color);
    Color* const color = static_cast<Color*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Allocate());
    if (color) {
        color->argb = 0xff000000;
    }
    return BindNativeInstance(vm, color, Color::ReleaseHook);
}

// FUNCTION: VC1 0x0041fea0
SQInteger Vector2::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector2);
    Vector2* const vector = static_cast<Vector2*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector2::ReleaseHook);
}

// FUNCTION: VC1 0x004201f0
SQInteger Color::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Color);
    }
    return 0;
}

// FUNCTION: VC1 0x00423640
SQInteger Color::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Color", -1);
    return 1;
}

// FUNCTION: VC1 0x00423660
SQInteger Vector3::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector3", -1);
    return 1;
}

// FUNCTION: VC1 0x00423680
SQInteger Vector2::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector2", -1);
    return 1;
}

// FUNCTION: VC1 0x004236a0
SQInteger Vector4::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector4", -1);
    return 1;
}

}
