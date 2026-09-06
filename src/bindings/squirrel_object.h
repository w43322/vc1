#ifndef VC1_BINDINGS_SQUIRREL_OBJECT_H
#define VC1_BINDINGS_SQUIRREL_OBJECT_H
#include <squirrel.h>

namespace vc1 {

// Inferred wrapper. Original vftable 0x004cfb98. Layout: vftable + HSQOBJECT.
struct SquirrelObject
{
    const void* vftable;
    HSQOBJECT obj;
};

typedef char Vc1SquirrelObjectSizeCheck[sizeof(SquirrelObject) == 12 ? 1 : -1];

// Global VM used by slot registration. Original 0x004ed13c.
extern HSQUIRRELVM g_vm;
extern const void* g_squirrelObjectVftable;

void InitSquirrelObject(SquirrelObject* object);

// 0x0049a370; original ABI: out=ECX, name=EAX, callback=[esp+4], typemask=EBX.
// Null typemask becomes "t|y|x"; "*" skips sq_setparamscheck; other masks prefix "t|y|x".
SquirrelObject* RegisterNativeSlot(SquirrelObject* out, const SQChar* name,
    SQFUNCTION callback, const SQChar* typemask);

// 0x004c3240; original ABI: vm=EAX, out=[esp+4], typeTag=[esp+8], className=[esp+0xc].
// Pushes root table, class name, sq_newclass(0); typetag is the NativeTypeInfo address.
SQBool CreateNativeClass(HSQUIRRELVM vm, SquirrelObject* out,
    SQUserPointer typeTag, const SQChar* className);

}

#endif
