#include "squirrel_object.h"

#include "host/runtime.h"

#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>
#include <sqpcheader.h>
#include <sqvm.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed13c
HSQUIRRELVM g_vm;

// GLOBAL: VC1 0x004cfa98
const char g_sqplusErrorFormat[] = "error: %s\n";

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

// DWORD loop then leftover bytes. Mismatch sign is (diff SAR 31) | 1.
// FUNCTION: VC1 0x00499d00
int MemCompare(const void* left, const void* right, unsigned size)
{
    const unsigned char* a = static_cast<const unsigned char*>(left);
    const unsigned char* b = static_cast<const unsigned char*>(right);
    int diff;

    while (size >= 4) {
        if (*reinterpret_cast<const unsigned*>(a)
            != *reinterpret_cast<const unsigned*>(b)) {
            break;
        }
        a += 4;
        b += 4;
        size -= 4;
    }
    if (size == 0) {
        return 0;
    }

    diff = static_cast<int>(a[0]) - static_cast<int>(b[0]);
    if (diff == 0 && size > 1) {
        diff = static_cast<int>(a[1]) - static_cast<int>(b[1]);
    }
    if (diff == 0 && size > 2) {
        diff = static_cast<int>(a[2]) - static_cast<int>(b[2]);
    }
    if (diff == 0 && size > 3) {
        diff = static_cast<int>(a[3]) - static_cast<int>(b[3]);
    }
    if (diff == 0) {
        return 0;
    }
    return (diff >> 31) | 1;
}

// Engine GetRootTable: lazily attaches the root table to g_rootTable.
// FUNCTION: VC1 0x00499fe0
SquirrelObject& SquirrelVM::GetRootTable()
{
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        g_rootTable.AttachToStackObject(-1);
        sq_pop(g_vm, 1);
    }
    return g_rootTable;
}

// Release the root table if it is refcounted, then sq_close-style teardown of g_vm.
// FUNCTION: VC1 0x0049a170
void SquirrelVM::Shutdown()
{
    HSQUIRRELVM vm = g_vm;
    if (g_rootTable.obj._type != OT_NULL) {
        if (g_vm) {
            sq_release(g_vm, &g_rootTable.obj);
        }
        g_rootTable.obj._unVal.pUserPointer = 0;
        g_rootTable.obj._type = OT_NULL;
    }

    g_vm = 0;
    if (vm) {
        g_sizeClassHeap.releasedBytes += 0xb4;
        sq_close(vm);
    }
}

// EAX=script, ECX=_this or 0 (push root), EBX=hidden return. sq_call nparams=1.
// The failure path stands in for SqPlus's throw and returns the empty object.
// FUNCTION: VC1 0x0049a270
SquirrelObject SquirrelVM::RunScript(const SquirrelObject& o, SquirrelObject* _this)
{
    SquirrelObject ret;
    sq_pushobject(g_vm, o.obj);
    if (_this) {
        sq_pushobject(g_vm, _this->obj);
    }
    else {
        sq_pushroottable(g_vm);
    }
    if (SQ_SUCCEEDED(sq_call(g_vm, 1, SQTrue, SQTrue))) {
        ret.AttachToStackObject(-1);
        sq_pop(g_vm, 2);
        return ret;
    }
    sq_pop(g_vm, 1);
    return ret;
}

// ECX=hidden return, EAX=scriptFuncName, [esp+4]=func, EBX=typeMask.
// FUNCTION: VC1 0x0049a370
SquirrelObject SquirrelVM::CreateFunction(SQFUNCTION func, const SQChar* scriptFuncName,
    const SQChar* typeMask)
{
    sq_pushstring(g_vm, scriptFuncName, -1);
    sq_newclosure(g_vm, func, 0);
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
            if (FormatBuffer(tm, sizeof(tm), "t|y|x%s", typeMask) < 0) {
                // STRING: VC1 0x004d2458
                KdLog(g_sqplusErrorFormat, "CreateFunction: typeMask string too long.");
                return ret;
            }
        }
    }
    else {
        FormatBuffer(tm, sizeof(tm), kPercentS,
            // STRING: VC1 0x004d2484
            "t|y|x");
    }
    if (ptm) {
        sq_setparamscheck(g_vm, numParams, ptm);
    }
    sq_newslot(g_vm, -3, SQFalse);
    return ret;
}

// If g_vm is unset: sq_open, printfunc at sharedstate+0xa4, stdlib batch, store g_vm.
// FUNCTION: VC1 0x0049a510
int SquirrelVM::Init()
{
    HSQUIRRELVM vm;

    if (g_vm) {
        return 1;
    }
    vm = sq_open(0);
    if (!vm) {
        return 0;
    }
    *reinterpret_cast<SQPRINTFUNCTION*>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa4)
        = ScriptPrint;
    sq_pushroottable(vm);
    sqstd_register_iolib(vm);
    sqstd_register_bloblib(vm);
    sqstd_register_mathlib(vm);
    sqstd_register_stringlib(vm);
    sqstd_register_systemlib(vm);
    sqstd_seterrorhandlers(vm);
    sq_pop(vm, 1);
    g_vm = vm;
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
        if (SQ_SUCCEEDED(sq_compile(g_vm, ConsoleBufferRead, &buf, debugInfo, SQTrue))) {
            ret.AttachToStackObject(-1);
            sq_pop(g_vm, 1);
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
    if (SQ_SUCCEEDED(sqstd_loadfile(g_vm, s, SQTrue))) {
        ret.AttachToStackObject(-1);
        sq_pop(g_vm, 1);
        return ret;
    }
    return ret;
}

// EAX=this, stdcall name, RET 4. sq_get is inlined (0x004906b0 Get, then
// "the index doesn't exist").
// FUNCTION: VC1 0x0049a910
SQBool SquirrelObject::GetSlot(const SQChar* name) const
{
    sq_pushobject(g_vm, obj);
    sq_pushstring(g_vm, name, -1);
    if (SQ_SUCCEEDED(sq_get(g_vm, -2))) {
        return SQTrue;
    }
    return SQFalse;
}

// Stdcall this; idx is always -1.
// FUNCTION: VC1 0x0049aaa0
void SquirrelObject::AttachToStackObject(SQInteger idx)
{
    HSQOBJECT t;
    sq_getstackobj(g_vm, idx, &t);
    sq_addref(g_vm, &t);
    sq_release(g_vm, &obj);
    obj = t;
}

// this=EBX, o=EAX.
// FUNCTION: VC1 0x0049ab30
SquirrelObject& SquirrelObject::operator=(const SquirrelObject& o)
{
    HSQOBJECT t;
    t = o.obj;
    sq_addref(g_vm, &t);
    sq_release(g_vm, &obj);
    obj = t;
    return *this;
}

// this=ESI, o=ECX.
// FUNCTION: VC1 0x0049ac30
SquirrelObject::SquirrelObject(const SquirrelObject& o)
{
    obj = o.obj;
    sq_addref(g_vm, &obj);
}

// this=ECX. Also inlined into most owners and the deleting destructor 0x00401e00.
// FUNCTION: VC1 0x0049ac80
SquirrelObject::~SquirrelObject()
{
    if (g_vm) {
        sq_release(g_vm, &obj);
    }
    obj._unVal.pUserPointer = 0;
    obj._type = OT_NULL;
}

// EAX=key, stdcall this, RET 4.
// FUNCTION: VC1 0x0049acc0
SQInteger SquirrelObject::GetInt(const SQChar* key) const
{
    SQInteger ret = 0;
    if (GetSlot(key)) {
        sq_getinteger(g_vm, -1, &ret);
        sq_pop(g_vm, 1);
    }
    sq_pop(g_vm, 1);
    return ret;
}

// EAX=key, EDI=hidden return, stdcall this, RET 4.
// FUNCTION: VC1 0x0049ad30
SquirrelObject SquirrelObject::GetValue(const SQChar* key) const
{
    SquirrelObject ret;
    if (GetSlot(key)) {
        ret.AttachToStackObject(-1);
        sq_pop(g_vm, 1);
    }
    sq_pop(g_vm, 1);
    return ret;
}

#define _SETVALUE_STR_BEGIN \
    SQBool ret = SQFalse; \
    SQInteger top = sq_gettop(g_vm); \
    sq_pushobject(g_vm, obj); \
    sq_pushstring(g_vm, key, -1);

#define _SETVALUE_STR_END \
    if (SQ_SUCCEEDED(sq_rawset(g_vm, -3))) { \
        ret = SQTrue; \
    } \
    sq_settop(g_vm, top); \
    return ret;

#define _SETVALUE_INT_BEGIN \
    SQBool ret = SQFalse; \
    SQInteger top = sq_gettop(g_vm); \
    sq_pushobject(g_vm, obj); \
    sq_pushinteger(g_vm, key);

#define _SETVALUE_INT_END _SETVALUE_STR_END

// EAX=this, stdcall key, RET 4 (b is always false here).
// FUNCTION: VC1 0x0049b020
SQBool SquirrelObject::SetValue(const SQChar* key, bool b)
{
    _SETVALUE_STR_BEGIN
    sq_pushbool(g_vm, b);
    _SETVALUE_STR_END
}

// EAX=this, stdcall key + float, RET 8.
// FUNCTION: VC1 0x0049b0c0
SQBool SquirrelObject::SetValue(const SQChar* key, SQFloat f)
{
    _SETVALUE_STR_BEGIN
    sq_pushfloat(g_vm, f);
    _SETVALUE_STR_END
}

// EAX=this, stdcall key + integer, RET 8. Color registrar 0x004bf610 at
// 0x004bfac1: "AlphaShift" (0x004d6b84), 0x18. ~155 xrefs.
// FUNCTION: VC1 0x0049b130
SQBool SquirrelObject::SetValue(const SQChar* key, SQInteger n)
{
    _SETVALUE_STR_BEGIN
    sq_pushinteger(g_vm, n);
    _SETVALUE_STR_END
}

// EAX=this, EDI=val, stdcall key, RET 4. root["ARGS"] from InitScriptEngine.
// FUNCTION: VC1 0x0049b1d0
SQBool SquirrelObject::SetValue(const SQChar* key, const SquirrelObject& val)
{
    _SETVALUE_STR_BEGIN
    sq_pushobject(g_vm, val.obj);
    _SETVALUE_STR_END
}

// EAX=this, stdcall key + string, RET 8. ARGS[i] = argv[i].
// FUNCTION: VC1 0x0049b250
SQBool SquirrelObject::SetValue(SQInteger key, const SQChar* s)
{
    _SETVALUE_INT_BEGIN
    sq_pushstring(g_vm, s, -1);
    _SETVALUE_INT_END
}

// FUNCTION: VC1 0x004c3240
SQBool CreateClass(HSQUIRRELVM vm, SquirrelObject& newClass,
    SQUserPointer classType, const SQChar* name)
{
    const SQInteger oldtop = sq_gettop(vm);
    sq_pushroottable(vm);
    sq_pushstring(vm, name, -1);
    if (SQ_FAILED(sq_newclass(vm, SQFalse))) {
        sq_settop(vm, oldtop);
        return SQFalse;
    }
    newClass.AttachToStackObject(-1);
    sq_settypetag(vm, -1, classType);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
    return SQTrue;
}

}
