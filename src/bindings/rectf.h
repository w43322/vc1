#ifndef VC1_BINDINGS_RECTF_H
#define VC1_BINDINGS_RECTF_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d4ffc. 4 floats, 16 bytes.
// Release hook is Vector4::ReleaseHook 0x0041dd60. Type spec overlaps Vector4 0x004cfba4.
struct RectF
{
    SQFloat left;
    SQFloat top;
    SQFloat right;
    SQFloat bottom;

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1RectFSizeCheck[sizeof(RectF) == 16 ? 1 : -1];

}

#endif
