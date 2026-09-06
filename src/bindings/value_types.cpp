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

// FUNCTION: VC1 0x0041e3b0
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook)
{
    sq_setinstanceup(vm, 1, nativeObject);
    sq_setreleasehook(vm, 1, releaseHook);
    return 1;
}

// Squirrel class constructor registered at 0x0041eff0 as "constructor" (0x004d2f10)
// with null typemask. Does not read extra stack args. Arg-taking factory is 0x004bed80.
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

static SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx)
{
    SQFloat value = 0.0f;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getfloat(vm, idx, &value);
    }
    return value;
}

static SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx)
{
    SQInteger value = 0;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getinteger(vm, idx, &value);
    }
    return value;
}

static SQUnsignedInteger ColorByte(SQInteger value)
{
    return static_cast<SQUnsignedInteger>(value) & 0xff;
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

// Registered from 0x0041e440 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004bf058. extra==2 is invalid. Error string 0x004d69cc.
// FUNCTION: VC1 0x004bed80
SQInteger Color::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Color);
    Color* const color = static_cast<Color*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra == 2 || extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Release(color);
        g_sizeClassHeap.releasedBytes += sizeof(Color);
        return sq_throwerror(vm, "constructColor: invalid arguments");
    }
    if (color) {
        if (extra == 1) {
            color->argb = static_cast<SQUnsignedInteger>(NumericInt(vm, 2));
        }
        else if (extra == 3) {
            color->argb = 0xff000000
                | (ColorByte(NumericInt(vm, 2)) << 16)
                | (ColorByte(NumericInt(vm, 3)) << 8)
                | ColorByte(NumericInt(vm, 4));
        }
        else if (extra == 4) {
            color->argb = (ColorByte(NumericInt(vm, 2)) << 24)
                | (ColorByte(NumericInt(vm, 3)) << 16)
                | (ColorByte(NumericInt(vm, 4)) << 8)
                | ColorByte(NumericInt(vm, 5));
        }
        else {
            color->argb = 0xff000000;
        }
    }
    return BindNativeInstance(vm, color, Color::ReleaseHook);
}

}
