#include "vector2.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

#include <math.h>

namespace vc1 {

static NativeTypeInfo g_vector2Type;
static unsigned g_vector2TypeReady;
static const int kVector2TypeSpec = 0;

// Bound as "GetSquareMagnitude" (0x004d7268) via 0x00422b70 from 0x004c0883.
// this in ECX; x*x + y*y.
// FUNCTION: VC1 0x004073c0
SQFloat Vector2::GetSquareMagnitude()
{
    return x * x + y * y;
}

// Bound as "GetMagnitude" (0x004d7258) via 0x00422b70 from 0x004c0867.
// sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x004073e0
SQFloat Vector2::GetMagnitude()
{
    return sqrtf(GetSquareMagnitude());
}

// Bound as "GetNormalized" (0x004d7248) via 0x00420c70 from 0x004c084b.
// Writes this/|this| into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x00407410
Vector2* Vector2::GetNormalized(Vector2* out)
{
    const SQFloat inv = 1.0f / GetMagnitude();
    out->x = x * inv;
    out->y = y * inv;
    return out;
}

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

// this in ESI. Name 0x004cfb44.
// FUNCTION: VC1 0x0041df30
NativeClassBinding* Vector2::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector2", Vector2::RegisterClass);
}

// FUNCTION: VC1 0x0041e620
NativeClassBinding* Vector2::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector2::ConstructArgs);
}

// Once-flag 0x004ea818, type info 0x004ea830, spec 0x004cfb9c, name 0x004cfb44.
// FUNCTION: VC1 0x0041f2b0
SquirrelObject* Vector2::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_vector2Type, &g_vector2TypeReady,
        &kVector2TypeSpec, "Vector2", Vector2::Constructor, Vector2::TypeOf);
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

// FUNCTION: VC1 0x00423680
SQInteger Vector2::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector2", -1);
    return 1;
}

// Bound as "_cmp" (0x004d36e0) from 0x00420d20. this in ECX; other on stack.
// FUNCTION: VC1 0x00428890
SQInteger Vector2::Cmp(const Vector2* other)
{
    return MemCompare(this, other, sizeof(Vector2));
}

// Bound as "_unm" (0x004d36d0) via 0x00420c70 from 0x004c07f3.
// Writes -this into out=[esp+4]. this in ECX; RET 4.
// FUNCTION: VC1 0x004288b0
Vector2* Vector2::Unm(Vector2* out)
{
    out->x = -x;
    out->y = -y;
    return out;
}

// Registered from 0x0041e620 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1. Error string 0x004d6a14.
// FUNCTION: VC1 0x004be9e0
SQInteger Vector2::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector2);
    Vector2* const vector = static_cast<Vector2*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 2) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector2);
        return sq_throwerror(vm, "constructVector2: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
        }
        else if (extra == 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
        }
    }
    return BindNativeInstance(vm, vector, Vector2::ReleaseHook);
}

}
