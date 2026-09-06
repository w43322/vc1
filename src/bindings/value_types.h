#ifndef VC1_BINDINGS_VALUE_TYPES_H
#define VC1_BINDINGS_VALUE_TYPES_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred names. _typeof strings: Color 0x004d7580, Vector3 0x004d7588,
// Vector2 0x004cfb44, Vector4 0x004d7590. Registered as "_typeof" (0x004d2f04).
struct Color
{
    SQUnsignedInteger argb; // 0xff000000 default in 0x0041fe10

    // thiscall; bound from 0x004bf610 as GetA/GetR/GetG/GetB.
    unsigned char GetA();
    unsigned char GetR();
    SQUnsignedInteger GetG();
    unsigned char GetB();
    static SQUnsignedInteger MakeARGB(unsigned char a, unsigned char r, unsigned char g,
        unsigned char b);
    void SetValue(SQUnsignedInteger value); // thiscall, RET 4
    SQUnsignedInteger GetValue();
    SQFloat GetBlue();
    SQFloat GetGreen();
    SQFloat GetRed();
    SQFloat GetAlpha();
    SQInteger Cmp(const Color* other); // thiscall RET 4; bound as "_cmp"

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

    SQInteger Cmp(const Vector2* other); // thiscall RET 4; bound as "_cmp"
    SQFloat GetSquareMagnitude();
    SQFloat GetMagnitude();
    Vector2* GetNormalized(Vector2* out); // thiscall RET 4

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

    SQInteger Cmp(const Vector3* other); // thiscall RET 4; bound as "_cmp"
    SQFloat GetSquareMagnitude();
    SQFloat GetMagnitude();
    Vector3* GetNormalized(Vector3* out); // thiscall RET 4

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

    SQInteger Cmp(const Vector4* other); // thiscall RET 4; bound as "_cmp"

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
