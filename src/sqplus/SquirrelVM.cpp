#include <sqplus.h>

#include "bindings/squirrel_object.h"
#include "host/runtime.h"

#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

// GLOBAL: VC1 0x004ed13c
HSQUIRRELVM SquirrelVM::_VM;

namespace vc1 {

// Host BufState is {buf, ptr, size} (stock buf_lexfeed is {buf, size, ptr}).
// FUNCTION: VC1 0x00496a90
static SQInteger ConsoleBufferRead(SQUserPointer p)
{
    struct HostBufState
    {
        const SQChar* buf;
        SQInteger ptr;
        SQInteger size;
    };
    HostBufState* const buf = static_cast<HostBufState*>(p);
    const SQInteger ptr = buf->ptr;
    const SQInteger next = ptr + 1;
    SQInteger ch;

    if (buf->size < next) {
        return 0;
    }
    ch = static_cast<signed char>(buf->buf[ptr]);
    buf->ptr = next;
    return ch;
}

}

// Engine GetRootTable: lazily attaches the root table to g_rootTable.
// FUNCTION: VC1 0x00499fe0
SquirrelObject& SquirrelVM::GetRootTable()
{
    if (vc1::g_rootTable._o._type == OT_NULL && _VM) {
        sq_pushroottable(_VM);
        vc1::g_rootTable.AttachToStackObject(-1);
        sq_pop(_VM, 1);
    }
    return vc1::g_rootTable;
}

// Release the root table if it is refcounted, then sq_close-style teardown of _VM.
// FUNCTION: VC1 0x0049a170
void SquirrelVM::Shutdown()
{
    HSQUIRRELVM vm = _VM;
    if (vc1::g_rootTable._o._type != OT_NULL) {
        if (_VM) {
            sq_release(_VM, &vc1::g_rootTable._o);
        }
        vc1::g_rootTable._o._unVal.pUserPointer = 0;
        vc1::g_rootTable._o._type = OT_NULL;
    }

    _VM = 0;
    if (vm) {
        vc1::g_sizeClassHeap.releasedBytes += 0xb4;
        sq_close(vm);
    }
}

// EAX=script, ECX=_this or 0 (push root), EBX=hidden return. sq_call nparams=1.
// The failure path stands in for SqPlus's throw and returns the empty object.
// FUNCTION: VC1 0x0049a270
SquirrelObject SquirrelVM::RunScript(const SquirrelObject& o, SquirrelObject* _this)
{
    SquirrelObject ret;
    sq_pushobject(_VM, o._o);
    if (_this) {
        sq_pushobject(_VM, _this->_o);
    }
    else {
        sq_pushroottable(_VM);
    }
    if (SQ_SUCCEEDED(sq_call(_VM, 1, SQTrue, SQTrue))) {
        ret.AttachToStackObject(-1);
        sq_pop(_VM, 2);
        return ret;
    }
    sq_pop(_VM, 1);
    return ret;
}

// ECX=hidden return, EAX=scriptFuncName, [esp+4]=func, EBX=typeMask.
// FUNCTION: VC1 0x0049a370
SquirrelObject SquirrelVM::CreateFunction(SQFUNCTION func, const SQChar* scriptFuncName,
    const SQChar* typeMask)
{
    sq_pushstring(_VM, scriptFuncName, -1);
    sq_newclosure(_VM, func, 0);
    SquirrelObject ret;
    ret.AttachToStackObject(-1);
    SQChar tm[64];
    SQChar* ptm = tm;
    SQInteger numParams = SQ_MATCHTYPEMASKSTRING;
    if (typeMask) {
        if (typeMask[0] == '*') {
            ptm = 0;
            numParams = 0;
        }
        else {
            // STRING: VC1 0x004d2450
            if (vc1::FormatBuffer(tm, sizeof(tm), "t|y|x%s", typeMask) < 0) {
                // STRING: VC1 0x004d2458
                vc1::KdLog(vc1::g_sqplusErrorFormat, "CreateFunction: typeMask string too long.");
                return ret;
            }
        }
    }
    else {
        vc1::FormatBuffer(tm, sizeof(tm), vc1::kPercentS,
            // STRING: VC1 0x004d2484
            "t|y|x");
    }
    if (ptm) {
        sq_setparamscheck(_VM, numParams, ptm);
    }
    sq_newslot(_VM, -3, SQFalse);
    return ret;
}

// If _VM is unset: sq_open, printfunc at sharedstate+0xa4, stdlib batch, store _VM.
// FUNCTION: VC1 0x0049a510
int SquirrelVM::Init()
{
    HSQUIRRELVM vm;

    if (_VM) {
        return 1;
    }
    vm = sq_open(0);
    if (!vm) {
        return 0;
    }
    *reinterpret_cast<SQPRINTFUNCTION*>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa4)
        = vc1::ScriptPrint;
    sq_pushroottable(vm);
    sqstd_register_iolib(vm);
    sqstd_register_bloblib(vm);
    sqstd_register_mathlib(vm);
    sqstd_register_stringlib(vm);
    sqstd_register_systemlib(vm);
    sqstd_seterrorhandlers(vm);
    sq_pop(vm, 1);
    _VM = vm;
    return 1;
}

// EDX=buffer, EDI=hidden return. Host sq_compilebuffer inlined (reader 0x00496a90,
// sq_compile 0x00499820, raiseerror=1).
// FUNCTION: VC1 0x0049a5a0
SquirrelObject SquirrelVM::CompileBuffer(const SQChar* s)
{
    SquirrelObject ret;
    SQInteger length;
    const SQChar* cursor;

    cursor = s;
    while (*cursor) {
        ++cursor;
    }
    length = static_cast<SQInteger>(cursor - s);
    {
        struct {
            const SQChar* buf;
            SQInteger ptr;
            SQInteger size;
        } buf;
        // STRING: VC1 0x004d0ce0
        const SQChar* const debugInfo = "console_buffer";
        buf.buf = s;
        buf.ptr = 0;
        buf.size = length;
        if (SQ_SUCCEEDED(sq_compile(_VM, vc1::ConsoleBufferRead, &buf, debugInfo, SQTrue))) {
            ret.AttachToStackObject(-1);
            sq_pop(_VM, 1);
            return ret;
        }
    }
    return ret;
}

// EAX=path, EDI=hidden return. sqstd_loadfile 0x0048fa50 with printerror=1.
// FUNCTION: VC1 0x0049a670
SquirrelObject SquirrelVM::CompileScript(const SQChar* s)
{
    SquirrelObject ret;
    if (SQ_SUCCEEDED(sqstd_loadfile(_VM, s, SQTrue))) {
        ret.AttachToStackObject(-1);
        sq_pop(_VM, 1);
        return ret;
    }
    return ret;
}
