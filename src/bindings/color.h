#ifndef VC1_BINDINGS_COLOR_H
#define VC1_BINDINGS_COLOR_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d7580, registered as "_typeof" (0x004d2f04).
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

typedef char Vc1ColorSizeCheck[sizeof(Color) == 4 ? 1 : -1];

}

#endif
