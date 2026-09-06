#ifndef VC1_BINDINGS_QUATERNION_H
#define VC1_BINDINGS_QUATERNION_H
#include "squirrel_object.h"

namespace vc1 {

// Inferred name. _typeof string 0x004d75a0, registered as "_typeof" (0x004d2f04).
// 16 bytes; default ctor zeros xyzw. Release hook is Vector4::ReleaseHook 0x0041dd60.
struct Quaternion
{
    SQFloat x;
    SQFloat y;
    SQFloat z;
    SQFloat w;

    static SQInteger Constructor(HSQUIRRELVM vm);
    static SQInteger ConstructArgs(HSQUIRRELVM vm);
    static SQInteger TypeOf(HSQUIRRELVM vm);
    static SquirrelObject* RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
        const SQChar* className);
    static NativeClassBinding* CreateBinding(NativeClassBinding* self);
    static NativeClassBinding* BindArgConstructor(NativeClassBinding* self);
};

typedef char Vc1QuaternionSizeCheck[sizeof(Quaternion) == 16 ? 1 : -1];

}

#endif
