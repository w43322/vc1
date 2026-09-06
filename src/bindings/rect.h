#ifndef VC1_BINDINGS_RECT_H
#define VC1_BINDINGS_RECT_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004cfb4c. 4 ints, 16 bytes.
// Release hook is Vector4::ReleaseHook 0x0041dd60.
struct Rect
{
    SQInteger left;
    SQInteger top;
    SQInteger right;
    SQInteger bottom;

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
