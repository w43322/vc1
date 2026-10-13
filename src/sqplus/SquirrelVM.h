#ifndef _SQUIRREL_VM_H_
#define _SQUIRREL_VM_H_

#include "SquirrelObject.h"

// The engine's SquirrelVM, standing in for third_party/sqplus's (left out of
// the vendored tree). Only static members, and none of upstream's VM
// reference handling (_vm, _no_vm_ref, _sandboxVM, _CallState): Init opens one
// VM with the engine's stdlib set, the root table is a SquirrelObject value
// (vc1::g_rootTable), and SquirrelError is gone (see SQPLUS_THROW in
// SqPlusSetup.h).
class SquirrelVM
{
public:
    // Upstream's multi-VM Init(v) at the top of every SqPlus native
    // (setVarFunc, getInstanceVarFunc, RegisterClassType, ...) leaves no trace
    // in the original.
    static void Init(HSQUIRRELVM) {}
    static int Init();
    static void Shutdown();

    static SquirrelObject CompileScript(const SQChar* s);
    // Upstream's default debugInfo "console_buffer" is folded into the body.
    static SquirrelObject CompileBuffer(const SQChar* s);
    static SquirrelObject RunScript(const SquirrelObject& o, SquirrelObject* _this = 0);

    // Inlined into VarRef::VarRef (0x004270b0).
    static SquirrelObject CreateTable()
    {
        SquirrelObject ret;
        sq_newtable(_VM);
        ret.AttachToStackObject(-1);
        sq_pop(_VM, 1);
        return ret;
    }
    static SquirrelObject CreateArray(int size)
    {
        SquirrelObject ret;
        sq_newarray(_VM, size);
        ret.AttachToStackObject(-1);
        sq_pop(_VM, 1);
        return ret;
    }

    static SquirrelObject& GetRootTable();
    static HSQUIRRELVM GetVMPtr() { return _VM; }

    static void PushObject(SquirrelObject& so) { sq_pushobject(_VM, so.GetObjectHandle()); }
    static void Pop(SQInteger nelemstopop) { sq_pop(_VM, nelemstopop); }

    static SquirrelObject CreateFunction(SQFUNCTION func, const SQChar* scriptFuncName,
        const SQChar* typeMask = 0);
    static SquirrelObject CreateFunction(SquirrelObject& so, SQFUNCTION func,
        const SQChar* scriptFuncName, const SQChar* typeMask = 0)
    {
        PushObject(so);
        SquirrelObject ret = CreateFunction(func, scriptFuncName, typeMask);
        Pop(1);
        return ret;
    }

    static HSQUIRRELVM _VM;
};

#endif
