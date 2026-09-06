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

// Bound as "Lerp" (0x004d729c) via 0x00423c70 / FUN_00425300.
// cdecl; out=a+t*(b-a) for xyzw. a=[esp+8], b=[esp+0xc], t=[esp+0x10], out=[esp+4].
// FUNCTION: VC1 0x00428280
Vector4* Vector4::Lerp(Vector4* out, const Vector4* a, const Vector4* b, SQFloat t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    out->z = a->z + t * (b->z - a->z);
    out->w = a->w + t * (b->w - a->w);
    return out;
}

// Bound as "GetCrossProduct" (0x004d728c) via 0x004216b0 / FUN_00423c10 from 0x004c0aaf.
// cdecl xyz cross into a Vector3. a=[esp+8], b=[esp+0xc], out=[esp+4].
// FUNCTION: VC1 0x004282d0
Vector3* Vector4::GetCrossProduct(Vector3* out, const Vector3* a, const Vector3* b)
{
    out->x = a->y * b->z - a->z * b->y;
    out->y = a->z * b->x - a->x * b->z;
    out->z = a->x * b->y - a->y * b->x;
    return out;
}

// Bound as "GetDotProduct" (0x004d727c) via 0x00421640 / FUN_00423bb0.
// cdecl; a=[esp+4], b=[esp+8]; y, x, z, w.
// FUNCTION: VC1 0x00428310
SQFloat Vector4::GetDotProduct(const Vector4* a, const Vector4* b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
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

// Bound as "Normalize" (0x004d723c) via 0x00421590 from 0x004c0a3d.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x00428440
void Vector4::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
    z *= inv;
    w *= inv;
}

// Bound as "_cmp" (0x004d36e0) from 0x00421430. this in ECX; other on stack.
// Also reused by another 16-byte binder at 0x00422960.
// FUNCTION: VC1 0x00428590
SQInteger Vector4::Cmp(const Vector4* other)
{
    return MemCompare(this, other, sizeof(Vector4));
}

// Bound as "_unm" (0x004d36d0) via 0x00421380 from 0x004c0a03.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004285b0
Vector4* Vector4::Unm(Vector4* out)
{
    out->x = -x;
    out->y = -y;
    out->z = -z;
    out->w = -w;
    return out;
}

// Bound as "_div" (0x004d36c8) via 0x004212d0 from 0x004c09e7.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x004285e0
Vector4* Vector4::Div(Vector4* out, SQFloat scalar)
{
    const SQFloat inv = 1.0f / scalar;
    out->x = x * inv;
    out->y = y * inv;
    out->z = z * inv;
    out->w = w * inv;
    return out;
}

// Bound as "_mul" (0x004d36c0) via 0x004212d0 from 0x004c09cb.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428620
Vector4* Vector4::Mul(Vector4* out, SQFloat scalar)
{
    out->x = x * scalar;
    out->y = y * scalar;
    out->z = z * scalar;
    out->w = w * scalar;
    return out;
}

// Bound as "_sub" (0x004d36b8) via 0x00421220 from 0x004c09af.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428650
Vector4* Vector4::Sub(Vector4* out, const Vector4* other)
{
    out->x = x - other->x;
    out->y = y - other->y;
    out->z = z - other->z;
    out->w = w - other->w;
    return out;
}

// Bound as "_add" (0x004d36b0) via 0x00421220 from 0x004c0993.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428680
Vector4* Vector4::Add(Vector4* out, const Vector4* other)
{
    out->x = x + other->x;
    out->y = y + other->y;
    out->z = z + other->z;
    out->w = w + other->w;
    return out;
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
