#ifndef VC1_BINDINGS_SQUIRREL_OBJECT_H
#define VC1_BINDINGS_SQUIRREL_OBJECT_H
#include <squirrel.h>

namespace vc1 {

// Inferred wrapper. Layout: vftable + HSQOBJECT. Recovered vftable storage is a dummy.
// VTABLE: VC1 0x004cfb98
struct SquirrelObject
{
    const void* vftable;
    HSQOBJECT obj;
};

typedef char Vc1SquirrelObjectSizeCheck[sizeof(SquirrelObject) == 12 ? 1 : -1];

// Inferred holder used by Color/Vector class setup. Original reads HSQOBJECT at +0x0c/+0x10.
struct NativeClassBinding
{
    HSQUIRRELVM vm;          // +0x00, copied from g_vm
    const SQChar* className; // +0x04
    SquirrelObject obj;      // +0x08
};

typedef char Vc1NativeClassBindingSizeCheck[sizeof(NativeClassBinding) == 20 ? 1 : -1];

// Global VM used by slot registration. Original 0x004ed13c.
extern HSQUIRRELVM g_vm;
extern const void* g_squirrelObjectVftable;

void InitSquirrelObject(SquirrelObject* object);

// 0x00489320; EAX=vm->_sharedstate+0x18, stdcall obj=[esp+4], RET 4. Inferred name.
SQBool __stdcall ReleaseSharedRef(void* refsTable, HSQOBJECT* obj);
void ReleaseObjectIfNeeded(HSQOBJECT* obj);

// 0x00498a40; EAX=vm. LTCG sq_rawset with the object at stack -3. Inferred name.
SQInteger RawSet(HSQUIRRELVM vm);

// 0x00499d00; original ABI: size=EAX, left=EDX, right=ECX. Returns -1, 0, or 1.
// Used by Color/Vector `_cmp` (0x004d36e0) wrappers and other byte-wise compares.
int MemCompare(const void* left, const void* right, unsigned size);

// 0x0049a270; EAX=script, ECX=env or 0 for root, EBX=dest. returns dest. Inferred name.
SquirrelObject* InvokeWithRoot(SquirrelObject* script, SquirrelObject* env,
    SquirrelObject* dest);

// 0x0049a5a0; EDX=source, EDI=dest. returns dest. Inferred name.
SquirrelObject* CompileString(const SQChar* source, SquirrelObject* dest);

// 0x0049a670; EAX=path, EDI=dest. returns dest. Inferred name.
SquirrelObject* CompilePath(const SQChar* path, SquirrelObject* dest);

// 0x0049a910; EAX=table, stdcall name=[esp+4], RET 4. Inferred name.
SQBool __stdcall HasNamedSlot(SquirrelObject* table, const SQChar* name);

// 0x0049aaa0; stdcall dest=[esp+4]. Copies stack top into dest->obj; addref new, release old.
void AttachStackTop(SquirrelObject* dest);

// 0x0049ab30; dest=EBX, source=EAX. addref source, release dest, copy HSQOBJECT. returns dest.
SquirrelObject* AssignSquirrelObject(SquirrelObject* dest, const SquirrelObject* source);

// 0x0049ac30; ESI=dest, ECX=source. copy HSQOBJECT and addref if 0x08000000. returns dest.
SquirrelObject* CopySquirrelObject(SquirrelObject* dest, const SquirrelObject* source);

// 0x0049acc0; EAX=name, stdcall table=[esp+4], RET 4. Inferred name.
SQInteger __stdcall ReadIntFromSlot(SquirrelObject* table, const SQChar* name);

// 0x0049ad30; EAX=name, EDI=dest, stdcall table=[esp+4], RET 4. Inferred name.
SquirrelObject* __stdcall GetNamedObject(SquirrelObject* table, const SQChar* name,
    SquirrelObject* dest);

// 0x0049b020; EAX=object, stdcall name=[esp+4], RET 4. Pushes OT_BOOL 0. Inferred name.
SQBool __stdcall RegisterBoolFalseSlot(SquirrelObject* object, const SQChar* name);

// 0x0049b0c0; EAX=object, stdcall name=[esp+4], float=[esp+8], RET 8. Inferred name.
SQBool __stdcall RegisterFloatSlot(SquirrelObject* object, const SQChar* name, SQFloat value);

// 0x0049b130; EAX=SquirrelObject*, name=[esp+4], value=[esp+8], RET 8.
// Pushes class, name, integer, then 0x00498a40 (newslot-like; not mapped sq_newslot 0x00499600).
SQBool RegisterIntegerSlot(SquirrelObject* object, const SQChar* name, SQInteger value);

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
