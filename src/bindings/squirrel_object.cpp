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

static void CaptureTop(SquirrelObject* out)
{
    sq_getstackobj(g_vm, -1, &out->obj);
    sq_addref(g_vm, &out->obj);
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
    CaptureTop(out);

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
    sq_getstackobj(vm, -1, &out->obj);
    sq_addref(vm, &out->obj);
    sq_settypetag(vm, -1, typeTag);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
    return SQTrue;
}

}
