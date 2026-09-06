#ifndef VC1_BINDINGS_VALUE_TYPES_H
#define VC1_BINDINGS_VALUE_TYPES_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred names. _typeof strings: Color 0x004d7580, Vector3 0x004d7588,
// Vector2 0x004cfb44, Vector4 0x004d7590. Registered as "_typeof" (0x004d2f04).
struct Color
{
    SQUnsignedInteger argb; // 0xff000000 default in 0x0041fe10

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

struct Vector2
{
    SQFloat x;
    SQFloat y;

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

struct Vector3
{
    SQFloat x;
    SQFloat y;
    SQFloat z;

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

struct Vector4
{
    SQFloat x;
    SQFloat y;
    SQFloat z;
    SQFloat w;

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger ReleaseHook(SQUserPointer object, SQInteger size);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1Vector3SizeCheck[sizeof(Vector3) == 12 ? 1 : -1];
typedef char Vc1Vector4SizeCheck[sizeof(Vector4) == 16 ? 1 : -1];

// 0x0041e3b0; original ABI: vm=EAX, nativeObject=[esp+4], releaseHook=ESI.
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook);

}

#endif
