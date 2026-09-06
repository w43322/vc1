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

// Bound as "Normalize" (0x004d723c) via 0x00420f30 from 0x004c083c.
// In-place this/=|this|. sqrt helper 0x004c4280. this in ECX.
// FUNCTION: VC1 0x0040cb10
void Vector2::Normalize()
{
    const SQFloat inv = 1.0f / GetMagnitude();
    x *= inv;
    y *= inv;
}

// Bound as "GetDotProduct" (0x004d727c) via 0x00421090 from 0x004c08ae.
// cdecl; a=[esp+4], b=[esp+8]; y*y then x*x.
// FUNCTION: VC1 0x0040cd20
SQFloat Vector2::GetDotProduct(const Vector2* a, const Vector2* b)
{
    return a->x * b->x + a->y * b->y;
}

// Bound as "GetOrthogonal" (0x004d72e4) via 0x00423b50 / FUN_00425190.
// cdecl; out=[esp+4], v=[esp+8]; (-y, x). RET.
// FUNCTION: VC1 0x0040cd90
Vector2* Vector2::GetOrthogonal(Vector2* out, const Vector2* v)
{
    out->x = -v->y;
    out->y = v->x;
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

// Bound as "Lerp" (0x004d729c) via 0x00423af0 / FUN_00425120.
// cdecl; out=a+t*(b-a). a=[esp+8], b=[esp+0xc], t=[esp+0x10], out=[esp+4].
// FUNCTION: VC1 0x004286b0
Vector2* Vector2::Lerp(Vector2* out, const Vector2* a, const Vector2* b, SQFloat t)
{
    out->x = a->x + t * (b->x - a->x);
    out->y = a->y + t * (b->y - a->y);
    return out;
}

// Bound as "IsValid" (0x004d72cc) via 0x00420fe0 from 0x004c089f.
// FLD x; FLD +Inf (0x004e95cc / 0x7f800000); FCHS to -Inf; FCOM/FUCOM with TEST AH,5 (NaN/below).
// Same pair for y. Returns 1 if both finite. this in ECX.
// FUNCTION: VC1 0x004286e0
SQBool Vector2::IsValid()
{
    union {
        unsigned bits;
        SQFloat value;
    } inf;
    inf.bits = 0x7f800000;
    return x > -inf.value && x < inf.value && y > -inf.value && y < inf.value;
}

// Bound as "Equal" (0x004d72c4) via 0x00420e80 from 0x004c082d.
// FUCOMPP x then y. this in ECX; other=[esp+4]; RET 4.
// FUNCTION: VC1 0x00428750
SQBool Vector2::Equal(const Vector2* other)
{
    return x == other->x && y == other->y;
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

// Bound as "_div" (0x004d36c8) via 0x00420bc0 from 0x004c07d7.
// inv=1/scalar into [esp+8], then out=this*inv. this in ECX; RET 8.
// FUNCTION: VC1 0x004288d0
Vector2* Vector2::Div(Vector2* out, SQFloat scalar)
{
    const SQFloat inv = 1.0f / scalar;
    out->x = x * inv;
    out->y = y * inv;
    return out;
}

// Bound as "_mul" (0x004d36c0) via 0x00420bc0 from 0x004c07bb.
// Writes this*scalar into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428900
Vector2* Vector2::Mul(Vector2* out, SQFloat scalar)
{
    out->x = x * scalar;
    out->y = y * scalar;
    return out;
}

// Bound as "_sub" (0x004d36b8) via 0x00421dc0 from 0x004c079f.
// Writes this-other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428920
Vector2* Vector2::Sub(Vector2* out, const Vector2* other)
{
    out->x = x - other->x;
    out->y = y - other->y;
    return out;
}

// Bound as "_add" (0x004d36b0) via 0x00421dc0 from 0x004c0783.
// Writes this+other into out=[esp+4]. this in ECX; RET 8.
// FUNCTION: VC1 0x00428940
Vector2* Vector2::Add(Vector2* out, const Vector2* other)
{
    out->x = x + other->x;
    out->y = y + other->y;
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
