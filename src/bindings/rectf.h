#ifndef VC1_BINDINGS_RECTF_H
#define VC1_BINDINGS_RECTF_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d4ffc. 4 floats, 16 bytes.
// Slots X/Y/Width/Height at +0/+4/+8/+0xc. Release hook is Vector4::ReleaseHook.
struct RectF
{
    SQFloat x;
    SQFloat y;
    SQFloat width;
    SQFloat height;

    SQBool Union(const RectF* other); // thiscall; bound as "Union" (0x004d7418)

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject RegisterClass(HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1RectFSizeCheck[sizeof(RectF) == 16 ? 1 : -1];

}

#endif
