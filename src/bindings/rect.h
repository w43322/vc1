#ifndef VC1_BINDINGS_RECT_H
#define VC1_BINDINGS_RECT_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004cfb4c. 4 ints, 16 bytes.
// Slots X/Y/Width/Height at +0/+4/+8/+0xc. Release hook is Vector4::ReleaseHook.
struct Rect
{
    SQInteger x;
    SQInteger y;
    SQInteger width;
    SQInteger height;

    SQBool Union(const Rect* other); // thiscall; bound as "Union" (0x004d7418)

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1RectSizeCheck[sizeof(Rect) == 16 ? 1 : -1];

}

#endif
