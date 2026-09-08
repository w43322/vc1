#include "host/runtime.h"

#include <squirrel.h>
#include <sqstdaux.h>
#include <sqstdblob.h>
#include <sqstdio.h>
#include <sqstdmath.h>
#include <sqstdstring.h>
#include <sqstdsystem.h>

namespace vc1 {

static void BindHostClasses();

// STUB: VC1 0x00402690
static void RegisterPrintBinding(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

// STUB: VC1 0x00402770
static void RegisterPrintAltBinding(HSQUIRRELVM vm, void* fn)
{
    (void)vm;
    (void)fn;
}

// STUB: VC1 0x00402850
static void RegisterStdExtra0()
{
}

// STUB: VC1 0x004028f0
static void RegisterStdExtra1()
{
}

// STUB: VC1 0x00402990
static void RegisterStdExtra2()
{
}

// STUB: VC1 0x00402a30
static void RegisterStdExtra3()
{
}

// STUB: VC1 0x00402ad0
static void RegisterStdExtra4()
{
}

static void EnsureRootTable()
{
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
}

// EAX=argc, EBX=argv from RunApp's g_runScratch0/1. Args table then root["ARGS"].
// FUNCTION: VC1 0x00463b40
void InitScriptEngine(int argc, char** argv)
{
    int i;
    SquirrelObject scratch;

    InitSizeClassPools();
    OpenScriptVm();
    BindHostClasses();
    InitHostServices();

    InitSquirrelObject(&scratch);
    sq_newarray(g_vm, argc);
    AttachStackTop(&scratch);
    sq_pop(g_vm, 1);
    for (i = 0; i < argc; ++i) {
        BindIndex(&scratch, i, argv[i]);
    }
    EnsureRootTable();
    // STRING: VC1 0x004d6384
    RegisterObjectSlot(&g_rootTable, "ARGS", &scratch);
    scratch.vftable = g_squirrelObjectVftable;
    ReleaseObjectIfNeeded(&scratch.obj);
}

// sq_pushroottable + AttachStackTop on 0x004e96e0 when the stored type is still OT_NULL.
// FUNCTION: VC1 0x00499fe0
SquirrelObject* PushRootTable()
{
    EnsureRootTable();
    return &g_rootTable;
}

// Release the root table if it is refcounted, then sq_close-style teardown of g_vm.
// FUNCTION: VC1 0x0049a170
void CloseScriptVm()
{
    HSQUIRRELVM vm = g_vm;
    if (g_rootTable.obj._type != OT_NULL) {
        ReleaseObjectIfNeeded(&g_rootTable.obj);
        g_rootTable.obj._unVal.pUserPointer = 0;
        g_rootTable.obj._type = OT_NULL;
    }

    g_vm = 0;
    if (vm) {
        g_sizeClassHeap.releasedBytes += 0xb4;
        sq_close(vm);
    }
}

// If g_vm is unset: sq_open, printfunc at sharedstate+0xa4, stdlib batch, store g_vm.
// FUNCTION: VC1 0x0049a510
int OpenScriptVm()
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

// Store the recovered vftable, release a refcounted HSQOBJECT, then write OT_NULL.
// FUNCTION: VC1 0x0049ac80
void ResetSquirrelObject(SquirrelObject* object)
{
    object->vftable = g_squirrelObjectVftable;
    ReleaseObjectIfNeeded(&object->obj);
    object->obj._unVal.pUserPointer = 0;
    object->obj._type = OT_NULL;
}

// EAX=table, EDI=value, stdcall name. root["ARGS"]=argv table from InitScriptEngine.
// FUNCTION: VC1 0x0049b1d0
SQBool __stdcall RegisterObjectSlot(SquirrelObject* table, const SQChar* name,
    SquirrelObject* value)
{
    const SQInteger savedTop = sq_gettop(g_vm);
    sq_pushobject(g_vm, table->obj);
    sq_pushstring(g_vm, name, -1);
    sq_pushobject(g_vm, value->obj);
    {
        const SQBool ok = SQ_SUCCEEDED(sq_rawset(g_vm, -3));
        sq_settop(g_vm, savedTop);
        return ok;
    }
}

// EAX=table, stdcall index + name. table[i] = argv[i]. RET 8.
// FUNCTION: VC1 0x0049b250
SQBool __stdcall BindIndex(SquirrelObject* table, SQInteger index, const SQChar* name)
{
    const SQInteger savedTop = sq_gettop(g_vm);
    sq_pushobject(g_vm, table->obj);
    sq_pushinteger(g_vm, index);
    sq_pushstring(g_vm, name, -1);
    {
        const SQBool ok = SQ_SUCCEEDED(sq_rawset(g_vm, -3));
        sq_settop(g_vm, savedTop);
        return ok;
    }
}

// Original 0x004bdc50 registers print/error closures then five extra stdlib batches on g_vm.
// FUNCTION: VC1 0x004bdc50
void RegisterStdBindings()
{
    RegisterPrintBinding(g_vm, 0);
    RegisterPrintAltBinding(g_vm, 0);
    RegisterStdExtra0();
    RegisterStdExtra1();
    RegisterStdExtra2();
    RegisterStdExtra3();
    RegisterStdExtra4();
}

// Color/Vector/Matrix/Rect native class table. Called from InitScriptEngine.
// STUB: VC1 0x004bf610
static void BindHostClasses()
{
}

}
