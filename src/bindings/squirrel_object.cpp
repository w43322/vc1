#include "squirrel_object.h"

#include "host/runtime.h"

#include <sqstdio.h>
#include <sqpcheader.h>
#include <sqvm.h>
#include <stdio.h>

namespace vc1 {

static const unsigned kSqRefCounted = 0x08000000;
static const unsigned kSqNumeric = 0x04000000;

// GLOBAL: VC1 0x004ed13c
HSQUIRRELVM g_vm;
static int g_squirrelObjectVftableStorage;
const void* g_squirrelObjectVftable = &g_squirrelObjectVftableStorage;

void InitSquirrelObject(SquirrelObject* object)
{
    object->vftable = g_squirrelObjectVftable;
    object->obj._unVal.pUserPointer = 0;
    object->obj._type = OT_NULL;
}

static void* SharedStateRefs(HSQUIRRELVM vm)
{
    char* const shared = *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c);
    return shared + 0x18;
}

void ReleaseObjectIfNeeded(HSQOBJECT* obj)
{
    if (g_vm && (obj->_type & kSqRefCounted) != 0) {
        static_cast<RefTable*>(SharedStateRefs(g_vm))->Release(
            *reinterpret_cast<SQObject*>(obj));
    }
}

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

// EAX=script SquirrelObject, ECX=env or 0 (push root), EBX=dest. sq_call nparams=1.
// On success pop 2 and copy the return; on fail pop 1 and copy the empty local.
// FUNCTION: VC1 0x0049a270
SquirrelObject* InvokeWithRoot(SquirrelObject* script, SquirrelObject* env,
    SquirrelObject* dest)
{
    SquirrelObject local;

    InitSquirrelObject(&local);
    sq_pushobject(g_vm, script->obj);
    if (!env) {
        sq_pushroottable(g_vm);
    }
    else {
        sq_pushobject(g_vm, env->obj);
    }
    if (sq_call(g_vm, 1, SQTrue, SQTrue) < 0) {
        sq_pop(g_vm, 1);
        return CopySquirrelObject(dest, &local);
    }
    AttachStackTop(&local);
    sq_pop(g_vm, 2);
    CopySquirrelObject(dest, &local);
    ReleaseObjectIfNeeded(&local.obj);
    return dest;
}

// FUNCTION: VC1 0x0049a370
SquirrelObject* RegisterNativeSlot(SquirrelObject* out, const SQChar* name,
    SQFUNCTION callback, const SQChar* typemask)
{
    SQChar mask[0x40];
    const SQChar* check = 0;

    InitSquirrelObject(out);
    sq_pushstring(g_vm, name, -1);
    sq_newclosure(g_vm, callback, 0);
    AttachStackTop(out);

    if (!typemask) {
        check = "t|y|x";
    }
    else if (typemask[0] != '*') {
        _snprintf(mask, 0x40, "t|y|x%s", typemask);
        mask[0x3f] = 0;
        check = mask;
    }
    if (check) {
        sq_setparamscheck(g_vm, SQ_MATCHTYPEMASKSTRING, check);
    }
    sq_newslot(g_vm, -3, SQFalse);
    return out;
}

// EDX=buffer, EDI=dest. strlen then 0x00499820 (reader 0x00496a90, raiseerror=1).
// FUNCTION: VC1 0x0049a5a0
SquirrelObject* CompileString(const SQChar* source, SquirrelObject* dest)
{
    SquirrelObject local;
    SQInteger length;
    const SQChar* cursor;

    InitSquirrelObject(&local);
    cursor = source;
    while (*cursor) {
        ++cursor;
    }
    length = static_cast<SQInteger>(cursor - source);
    {
        struct {
            const SQChar* buf;
            SQInteger ptr;
            SQInteger size;
        } buf;
        // STRING: VC1 0x004d0ce0
        const SQChar* const sourcename = "console_buffer";
        buf.buf = source;
        buf.ptr = 0;
        buf.size = length;
        if (sq_compile(g_vm, ConsoleBufferRead, &buf, sourcename, SQTrue) < 0) {
            return CopySquirrelObject(dest, &local);
        }
    }
    AttachStackTop(&local);
    sq_pop(g_vm, 1);
    CopySquirrelObject(dest, &local);
    ReleaseObjectIfNeeded(&local.obj);
    return dest;
}

// EAX=path, EDI=dest. PUSH 1; PUSH path; EAX=g_vm; CALL 0x0048fa50.
// FUNCTION: VC1 0x0049a670
SquirrelObject* CompilePath(const SQChar* path, SquirrelObject* dest)
{
    SquirrelObject local;

    InitSquirrelObject(&local);
    if (sqstd_loadfile(g_vm, path, SQTrue) < 0) {
        return CopySquirrelObject(dest, &local);
    }
    AttachStackTop(&local);
    sq_pop(g_vm, 1);
    CopySquirrelObject(dest, &local);
    ReleaseObjectIfNeeded(&local.obj);
    return dest;
}

// EAX=table, stdcall name. pushobject+pushstring then 0x004906b0; throw "the index doesn't exist".
// FUNCTION: VC1 0x0049a910
SQBool __stdcall HasNamedSlot(SquirrelObject* table, const SQChar* name)
{
    sq_pushobject(g_vm, table->obj);
    sq_pushstring(g_vm, name, -1);
    if (g_vm->Get(stack_get(g_vm, -2), g_vm->GetUp(-1), g_vm->GetUp(-1), false, false)) {
        return SQTrue;
    }
    sq_pop(g_vm, 1);
    // STRING: VC1 0x004d26a4
    if (sq_throwerror(g_vm, "the index doesn't exist") < 0) {
        return SQFalse;
    }
    return SQTrue;
}

// stdcall; used from RegisterNativeSlot 0x0049a3b9 and CreateNativeClass 0x004c32c4.
// FUNCTION: VC1 0x0049aaa0
void AttachStackTop(SquirrelObject* dest)
{
    HSQOBJECT top;
    sq_getstackobj(g_vm, -1, &top);
    sq_addref(g_vm, &top);
    sq_release(g_vm, &dest->obj);
    dest->obj = top;
}

// dest=EBX, source=EAX. Used from Color/Vector CreateBinding after RegisterClass.
// FUNCTION: VC1 0x0049ab30
SquirrelObject* AssignSquirrelObject(SquirrelObject* dest, const SquirrelObject* source)
{
    HSQOBJECT copy = source->obj;
    sq_addref(g_vm, &copy);
    sq_release(g_vm, &dest->obj);
    dest->obj = copy;
    return dest;
}

// ESI=dest, ECX=source. Writes dummy vftable 0x004cfb98, copies HSQOBJECT, addref via 0x004891e0.
// FUNCTION: VC1 0x0049ac30
SquirrelObject* CopySquirrelObject(SquirrelObject* dest, const SquirrelObject* source)
{
    dest->vftable = g_squirrelObjectVftable;
    dest->obj = source->obj;
    if ((dest->obj._type & kSqRefCounted) != 0 && g_vm) {
        sq_addref(g_vm, &dest->obj);
    }
    return dest;
}

// EAX=name, stdcall table. HasNamedSlot then read top as int/float; pop value and table.
// FUNCTION: VC1 0x0049acc0
SQInteger __stdcall ReadIntFromSlot(SquirrelObject* table, const SQChar* name)
{
    SQInteger value = 0;

    if (HasNamedSlot(table, name)) {
        HSQOBJECT top;
        sq_getstackobj(g_vm, -1, &top);
        if ((top._type & kSqNumeric) != 0) {
            if (top._type == OT_FLOAT) {
                value = static_cast<SQInteger>(top._unVal.fFloat);
            }
            else {
                value = top._unVal.nInteger;
            }
        }
        sq_pop(g_vm, 1);
    }
    sq_pop(g_vm, 1);
    return value;
}

// EAX=name, EDI=dest, stdcall table. Init dest, HasNamedSlot, AttachStackTop, always pop table.
// FUNCTION: VC1 0x0049ad30
SquirrelObject* __stdcall GetNamedObject(SquirrelObject* table, const SQChar* name,
    SquirrelObject* dest)
{
    InitSquirrelObject(dest);
    if (HasNamedSlot(table, name)) {
        AttachStackTop(dest);
        sq_pop(g_vm, 1);
    }
    sq_pop(g_vm, 1);
    return dest;
}

// EAX=object, stdcall name. pushobject, pushstring, OT_BOOL 0, 0x00498a40, sq_settop.
// FUNCTION: VC1 0x0049b020
SQBool __stdcall RegisterBoolFalseSlot(SquirrelObject* object, const SQChar* name)
{
    const SQInteger savedTop = sq_gettop(g_vm);
    sq_pushobject(g_vm, object->obj);
    sq_pushstring(g_vm, name, -1);
    sq_pushbool(g_vm, SQFalse);
    {
        const SQBool ok = SQ_SUCCEEDED(sq_rawset(g_vm, -3));
        sq_settop(g_vm, savedTop);
        return ok;
    }
}

// EAX=object, stdcall name + float. Same as RegisterIntegerSlot with 0x00496e50.
// FUNCTION: VC1 0x0049b0c0
SQBool __stdcall RegisterFloatSlot(SquirrelObject* object, const SQChar* name, SQFloat value)
{
    const SQInteger savedTop = sq_gettop(g_vm);
    sq_pushobject(g_vm, object->obj);
    sq_pushstring(g_vm, name, -1);
    sq_pushfloat(g_vm, value);
    {
        const SQBool ok = SQ_SUCCEEDED(sq_rawset(g_vm, -3));
        sq_settop(g_vm, savedTop);
        return ok;
    }
}

// EAX=class SquirrelObject (NativeClassBinding+8). Stack: name, integer. RET 8.
// Color registrar 0x004bf610 at 0x004bfac1: "AlphaShift" (0x004d6b84), 0x18. ~155 xrefs.
// Original newslot helper 0x00498a40, not mapped sq_newslot 0x00499600.
// FUNCTION: VC1 0x0049b130
SQBool RegisterIntegerSlot(SquirrelObject* object, const SQChar* name, SQInteger value)
{
    const SQInteger savedTop = sq_gettop(g_vm);
    sq_pushobject(g_vm, object->obj);
    sq_pushstring(g_vm, name, -1);
    sq_pushinteger(g_vm, value);
    const SQBool ok = SQ_SUCCEEDED(sq_rawset(g_vm, -3));
    sq_settop(g_vm, savedTop);
    return ok;
}

// FUNCTION: VC1 0x004c3240
SQBool CreateNativeClass(HSQUIRRELVM vm, SquirrelObject* out,
    SQUserPointer typeTag, const SQChar* className)
{
    const SQInteger savedTop = sq_gettop(vm);
    sq_pushroottable(vm);
    sq_pushstring(vm, className, -1);
    if (SQ_FAILED(sq_newclass(vm, SQFalse))) {
        sq_settop(vm, savedTop);
        return SQFalse;
    }
    AttachStackTop(out);
    sq_settypetag(vm, -1, typeTag);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
    return SQTrue;
}

}
