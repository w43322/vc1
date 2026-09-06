#include "squirrel_object.h"

#include <stdio.h>

namespace vc1 {

HSQUIRRELVM g_vm;
static int g_squirrelObjectVftableStorage;
const void* g_squirrelObjectVftable = &g_squirrelObjectVftableStorage;

void InitSquirrelObject(SquirrelObject* object)
{
    object->vftable = g_squirrelObjectVftable;
    object->obj._unVal.pUserPointer = 0;
    object->obj._type = OT_NULL;
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
