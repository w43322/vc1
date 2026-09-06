#include "vector4.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

#include <math.h>

namespace vc1 {

static NativeTypeInfo g_vector4Type;
static unsigned g_vector4TypeReady;
static const int kVector4TypeSpec = 0;

// SQRELEASEHOOK; size unused. Puts 16 bytes back on pools[4] / 0x004ed468.
// FUNCTION: VC1 0x0041dd60
SQInteger Vector4::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector4);
    }
    return 0;
}

// this in ESI. Name 0x004d7590.
// FUNCTION: VC1 0x0041dfc0
NativeClassBinding* Vector4::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector4", Vector4::RegisterClass);
}

// FUNCTION: VC1 0x0041e780
NativeClassBinding* Vector4::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector4::ConstructArgs);
}

// Once-flag 0x004ee3e8, type info 0x004ee200, spec 0x004cfba4, name 0x004d7590.
// FUNCTION: VC1 0x0041f410
SquirrelObject* Vector4::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_vector4Type, &g_vector4TypeReady,
        &kVector4TypeSpec, "Vector4", Vector4::Constructor, Vector4::TypeOf);
}

// Registered from 0x0041f410 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyzw. Release hook also used from 0x004bdd60 / 0x004bdf10 / 0x004be0e0 / 0x004be6c0.
// FUNCTION: VC1 0x0041fee0
SQInteger Vector4::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector4);
    Vector4* const vector = static_cast<Vector4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        vector->w = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x004236a0
SQInteger Vector4::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector4", -1);
    return 1;
}

// Bound as "GetNormalized" (0x004d7248) via 0x00421380 from 0x004c0a4c.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428340
Vector4* Vector4::GetNormalized(Vector4* out)
{
    const SQFloat inv = 1.0f / GetMagnitude();
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    out->w = w * inv;
    return out;
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c0a84.
// this in ECX; y, x, z, w then y*y + x*x + z*z + w*w.
// FUNCTION: VC1 0x004283c0
SQFloat Vector4::GetSquareMagnitude()
{
    return x * x + y * y + z * z + w * w;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c0a68.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004283f0
SQFloat Vector4::GetMagnitude()
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "_cmp" (0x004d36e0) from 0x00421430. this in ECX; other on stack.
// Also reused by another 16-byte binder at 0x00422960.
// FUNCTION: VC1 0x00428590
SQInteger Vector4::Cmp(const Vector4* other)
{
    return MemCompare(this, other, sizeof(Vector4));
}

// Registered from 0x0041e780 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004be9c0. Error string 0x004d6a38.
// FUNCTION: VC1 0x004be6c0
SQInteger Vector4::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector4);
    Vector4* const vector = static_cast<Vector4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector4);
        return sq_throwerror(vm, "constructVector4: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        vector->w = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
            vector->z = scalar;
            vector->w = scalar;
        }
        else if (extra >= 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
            if (extra >= 3) {
                vector->z = NumericFloat(vm, 4);
            }
            if (extra >= 4) {
                vector->w = NumericFloat(vm, 5);
            }
        }
    }
    return BindNativeInstance(vm, vector, Vector4::ReleaseHook);
}

}
