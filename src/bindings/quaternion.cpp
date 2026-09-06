#include "quaternion.h"
#include "vector4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_quaternionType;
static unsigned g_quaternionTypeReady;
static const int kQuaternionTypeSpec = 0;

static void ZeroQuaternion(Quaternion* quaternion)
{
    quaternion->x = 0.0f;
    quaternion->y = 0.0f;
    quaternion->z = 0.0f;
    quaternion->w = 0.0f;
}

// this in ESI. Name 0x004d75a0.
// FUNCTION: VC1 0x0041e170
NativeClassBinding* Quaternion::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Quaternion", Quaternion::RegisterClass);
}

// FUNCTION: VC1 0x0041eb20
NativeClassBinding* Quaternion::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Quaternion::ConstructArgs);
}

// Once-flag 0x004ee3e4, type info 0x004ee1ec, spec 0x004d75c8, name 0x004d75a0.
// FUNCTION: VC1 0x0041f830
SquirrelObject* Quaternion::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_quaternionType, &g_quaternionTypeReady,
        &kQuaternionTypeSpec, "Quaternion", Quaternion::Constructor, Quaternion::TypeOf);
}

// Registered from 0x0041f830 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyzw (not identity). Release hook 0x0041dd60 (Vector4). Factory 0x004be0e0.
// FUNCTION: VC1 0x00420000
SQInteger Quaternion::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Quaternion);
    Quaternion* const quaternion = static_cast<Quaternion*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Allocate());
    if (quaternion) {
        ZeroQuaternion(quaternion);
    }
    return BindNativeInstance(vm, quaternion, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x00423700
SQInteger Quaternion::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Quaternion", -1);
    return 1;
}

// Registered from 0x0041eb20 as "constructor" with typemask "*" (0x004d6b14).
// extra = gettop-1; extra>4 throws. extra==4 reads four NumericFloat. Error 0x004d6aa4.
// extra 1-3 use unmapped axis/angle helpers; zeros kept as fallback.
// FUNCTION: VC1 0x004be0e0
SQInteger Quaternion::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Quaternion);
    Quaternion* const quaternion = static_cast<Quaternion*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Quaternion))]->Release(quaternion);
        g_sizeClassHeap.releasedBytes += sizeof(Quaternion);
        return sq_throwerror(vm, "constructQuaternion: invalid arguments");
    }
    if (quaternion) {
        ZeroQuaternion(quaternion);
        if (extra == 4) {
            quaternion->x = NumericFloat(vm, 2);
            quaternion->y = NumericFloat(vm, 3);
            quaternion->z = NumericFloat(vm, 4);
            quaternion->w = NumericFloat(vm, 5);
        }
    }
    return BindNativeInstance(vm, quaternion, Vector4::ReleaseHook);
}

}
