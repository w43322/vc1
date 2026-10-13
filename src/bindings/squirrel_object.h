#ifndef VC1_BINDINGS_SQUIRREL_OBJECT_H
#define VC1_BINDINGS_SQUIRREL_OBJECT_H
#include <squirrel.h>
#include <stdlib.h>

namespace vc1 {

// SqPlus SquirrelVM::_VM. Original 0x004ed13c.
extern HSQUIRRELVM g_vm;
// "error: %s\n"; KdLog format that replaces every SqPlus throw.
extern const char g_sqplusErrorFormat[];

// SqPlus SquirrelObject (SquirrelObject.h/.cpp of the 2008 SqPlus snapshot).
// The engine's SqPlus copy has no exceptions: every throw site logs
// "error: %s\n" through KdLog and returns. `obj` is SqPlus `_o`. The only
// virtual is the destructor (slot 0 is the deleting destructor 0x00401e00).
// SIZE 0x0c
// VTABLE: VC1 0x004cfb98
class SquirrelObject
{
public:
    SquirrelObject()
    {
        obj._unVal.pUserPointer = 0;
        obj._type = OT_NULL;
    }
    SquirrelObject(const SquirrelObject& o);
    virtual ~SquirrelObject();
    SquirrelObject& operator=(const SquirrelObject& o);
    void AttachToStackObject(SQInteger idx);

    SQBool SetValue(SQInteger key, const SQChar* s);
    SQBool SetValue(const SQChar* key, const SquirrelObject& val);
    SQBool SetValue(const SQChar* key, bool b);
    SQBool SetValue(const SQChar* key, SQFloat f);
    SQBool SetValue(const SQChar* key, SQInteger n);
    SquirrelObject GetValue(const SQChar* key) const;
    SQInteger GetInt(const SQChar* key) const;
    SQBool GetSlot(const SQChar* name) const;

    // The deleting destructor calls free() directly.
    static void operator delete(void* p) { free(p); }

    HSQOBJECT obj;
};

typedef char Vc1SquirrelObjectSizeCheck[sizeof(SquirrelObject) == 12 ? 1 : -1];

// SYNTHETIC: VC1 0x00401e00
// vc1::SquirrelObject::`scalar deleting destructor'

// SqPlus SquirrelVM (static members only). The engine keeps the root table as
// a SquirrelObject value (g_rootTable) instead of SqPlus's heap pointer.
class SquirrelVM
{
public:
    static SquirrelObject& GetRootTable();
    static void Shutdown();
    static int Init();
    static SquirrelObject RunScript(const SquirrelObject& o, SquirrelObject* _this);
    static SquirrelObject CreateFunction(SQFUNCTION func, const SQChar* scriptFuncName,
        const SQChar* typeMask);
    static SquirrelObject CreateFunction(SquirrelObject& so, SQFUNCTION func,
        const SQChar* scriptFuncName, const SQChar* typeMask)
    {
        sq_pushobject(g_vm, so.obj);
        SquirrelObject ret = CreateFunction(func, scriptFuncName, typeMask);
        sq_pop(g_vm, 1);
        return ret;
    }
    static SquirrelObject CreateArray(SQInteger size)
    {
        SquirrelObject ret;
        sq_newarray(g_vm, size);
        ret.AttachToStackObject(-1);
        sq_pop(g_vm, 1);
        return ret;
    }
    // SqPlus default debugInfo "console_buffer" is folded into the body.
    static SquirrelObject CompileBuffer(const SQChar* s);
    static SquirrelObject CompileScript(const SQChar* s);
};

// Inferred holder used by Color/Vector class setup (SqPlus SQClassDef<T>
// layout: v, name, newClass).
struct NativeClassBinding
{
    HSQUIRRELVM vm;          // +0x00, copied from g_vm
    const SQChar* className; // +0x04
    SquirrelObject obj;      // +0x08
};

typedef char Vc1NativeClassBindingSizeCheck[sizeof(NativeClassBinding) == 20 ? 1 : -1];

// Root table holder, SqPlus SquirrelVM::_root. Original 0x004e96e0.
extern SquirrelObject g_rootTable;

// 0x00499d00; original ABI: size=EAX, left=EDX, right=ECX. Returns -1, 0, or 1.
// Used by Color/Vector `_cmp` (0x004d36e0) wrappers and other byte-wise compares.
int MemCompare(const void* left, const void* right, unsigned size);

// SqPlus CreateClass; baseName is always 0 here, so the base lookup is gone.
// 0x004c3240; original ABI: vm=EAX, out=[esp+4], typeTag=[esp+8], className=[esp+0xc].
SQBool CreateClass(HSQUIRRELVM vm, SquirrelObject& newClass,
    SQUserPointer classType, const SQChar* name);

}

#endif
