#include "vector3.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

#include <math.h>

namespace vc1 {

static NativeTypeInfo g_vector3Type;
static unsigned g_vector3TypeReady;
static const int kVector3TypeSpec = 0;

// Bound as "GetCrossProduct" (0x004d728c) via 0x00420ae0 / FUN_00423990 from 0x004c06a4.
// cdecl; out=[esp+4], a=[esp+8], b=[esp+0xc]. Stores z, then x, then y.
// FUNCTION: VC1 0x0040cb90
Vector3* Vector3::GetCrossProduct(Vector3* out, const Vector3* a, const Vector3* b)
{
    const SQFloat x = a->y * b->z - a->z * b->y;
    const SQFloat y = a->z * b->x - a->x * b->z;
    const SQFloat z = a->x * b->y - a->y * b->x;
    out->x = x;
    out->y = y;
    out->z = z;
    return out;
}

// Bound as "GetDotProduct" (0x004d727c) via 0x00420a70 / FUN_00423930.
// cdecl; a=[esp+4], b=[esp+8]; y*y then x*x then z*z.
// FUNCTION: VC1 0x0040cbd0
SQFloat Vector3::GetDotProduct(const Vector3* a, const Vector3* b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Bound as "Normalize" (0x004d723c) via 0x004209c0 from 0x004c0632.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00415880
void Vector3::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
    z *= inv;
}

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c0679.
// this in ECX; y, x, z then y*y + x*x + z*z.
// FUNCTION: VC1 0x004158e0
SQFloat Vector3::GetSquareMagnitude()
{
    return x * x + y * y + z * z;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c065d.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00415910
SQFloat Vector3::GetMagnitude()
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "GetNormalized" (0x004d7248) via 0x004207b0 from 0x004c0641.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00415950
Vector3* Vector3::GetNormalized(Vector3* out)
{
    const SQFloat inv = 1.0f / GetMagnitude();
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    return out;
}

// this in ESI. Name 0x004d7588.
// FUNCTION: VC1 0x0041dea0
NativeClassBinding* Vector3::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector3", Vector3::RegisterClass);
}

// FUNCTION: VC1 0x0041e4c0
NativeClassBinding* Vector3::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector3::ConstructArgs);
}

// Once-flag 0x004ee3e0, type info 0x004ee1d8, spec 0x004d75b8, name 0x004d7588.
// FUNCTION: VC1 0x0041f150
SquirrelObject Vector3::RegisterClass(HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterClassType<Vector3>(vm, className, &g_vector3Type, &g_vector3TypeReady,
        &kVector3TypeSpec, "Vector3", Vector3::Constructor, Vector3::TypeOf);
}

// Registered from 0x0041f150 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyz. Arg-taking factory is 0x004beb40.
// FUNCTION: VC1 0x0041fe50
SQInteger Vector3::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector3);
    Vector3* const vector = static_cast<Vector3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector3::ReleaseHook);
}

// SQRELEASEHOOK; size unused. Puts 12 bytes back on pools[3] / 0x004ed464.
// FUNCTION: VC1 0x00420210
SQInteger Vector3::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector3);
    }
    return 0;
}

// FUNCTION: VC1 0x00423660
SQInteger Vector3::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector3", -1);
    return 1;
}

// Bound as "Lerp" (0x004d729c) via 0x004239f0 / FUN_00425020.
// cdecl; out=a+t*(b-a). a=[esp+8], b=[esp+0xc], t=[esp+0x10], out=[esp+4].
// FUNCTION: VC1 0x00428960
Vector3* Vector3::Lerp(Vector3* out, const Vector3* a, const Vector3* b, SQFloat t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    return out;
}

// Bound as "_cmp" (0x004d36e0) from 0x00420860. this in ECX; other on stack.
// FUNCTION: VC1 0x00428a70
SQInteger Vector3::Cmp(const Vector3* other)
{
    return MemCompare(this, other, sizeof(Vector3));
}

// Bound as "_unm" (0x004d36d0) via 0x004207b0 from 0x004c05f8.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00428a90
Vector3* Vector3::Unm(Vector3* out)
{
    out->x = -x;
    out->y = -y;
    out->z = -z;
    return out;
}

// Bound as "_div" (0x004d36c8) via 0x00420700 from 0x004c05dc.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x00428ab0
Vector3* Vector3::Div(Vector3* out, SQFloat scalar)
{
    const SQFloat inv = 1.0f / scalar;
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    return out;
}

// Bound as "_mul" (0x004d36c0) via 0x00420700 from 0x004c05c0.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428ae0
Vector3* Vector3::Mul(Vector3* out, SQFloat scalar)
{
    out->x = x * scalar;
    out->y = y * scalar;
    out->z = z * scalar;
    return out;
}

// Bound as "_sub" (0x004d36b8) via 0x00422550 from 0x004c05a4.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428b10
Vector3* Vector3::Sub(Vector3* out, const Vector3* other)
{
    out->x = x - other->x;
    out->y = y - other->y;
    out->z = z - other->z;
    return out;
}

// Bound as "_add" (0x004d36b0) via 0x00422550 from 0x004c0588.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428b40
Vector3* Vector3::Add(Vector3* out, const Vector3* other)
{
    out->x = x + other->x;
    out->y = y + other->y;
    out->z = z + other->z;
    return out;
}

// Registered from 0x0041e4c0 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004bed64. Error string 0x004d69f0.
// FUNCTION: VC1 0x004beb40
SQInteger Vector3::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector3);
    Vector3* const vector = static_cast<Vector3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 3) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector3);
        return sq_throwerror(vm, "constructVector3: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
            vector->z = scalar;
        }
        else if (extra >= 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
            if (extra >= 3) {
                vector->z = NumericFloat(vm, 4);
            }
        }
    }
    return BindNativeInstance(vm, vector, Vector3::ReleaseHook);
}

}
