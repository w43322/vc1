#include "host/runtime.h"

#include <squirrel.h>

namespace vc1 {

static const unsigned kSqRefCounted = 0x08000000;

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
    UnmappedBindHostClasses();
    InitHostServices();

    InitSquirrelObject(&scratch);
    PushNewTable(g_vm, argc);
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

// EAX=vm, cdecl initialSize used as SQTable ctor EBX. Alloc 0x24 from pools[9] at 0x004ed47c.
// FUNCTION: VC1 0x00499130
void PushNewTable(HSQUIRRELVM vm, SQInteger initialSize)
{
    void* table;
    HSQOBJECT obj;

    g_sizeClassHeap.allocatedBytes += 0x24;
    table = g_sizeClassHeap.pools[9]->Allocate();
    if (table) {
        UnmappedTableCtor(
            table,
            *reinterpret_cast<void**>(reinterpret_cast<char*>(vm) + 0x8c),
            initialSize);
    }
    obj._type = OT_TABLE;
    obj._unVal.pTable = static_cast<SQTable*>(table);
    sq_addref(vm, &obj);
    sq_pushobject(vm, obj);
    sq_release(vm, &obj);
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
        UnmappedShutdownHost();
        g_sizeClassHeap.releasedBytes += 0xb4;
        sq_close(vm);
    }
}

// If g_vm is unset: 0x00499750, compiler error at sharedstate+0xa4, stdlib batch, store g_vm.
// FUNCTION: VC1 0x0049a510
int OpenScriptVm()
{
    HSQUIRRELVM vm;

    if (g_vm) {
        return 1;
    }
    vm = UnmappedSqOpen();
    if (!vm) {
        return 0;
    }
    *reinterpret_cast<void**>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa4)
        = reinterpret_cast<void*>(&UnmappedCompilerError);
    sq_pushroottable(vm);
    UnmappedRegisterStdIo(vm);
    UnmappedRegisterStdBlob(vm,
        // STRING: VC1 0x004d326c
        "blob", 0x80000002,
        // STRING: VC1 0x004d3274
        "std_blob",
        reinterpret_cast<void*>(0x004e9270), reinterpret_cast<void*>(0x004e9300));
    UnmappedRegisterStdMath(vm);
    UnmappedRegisterStdString(vm);
    UnmappedRegisterStdSystem(vm);
    UnmappedRegisterStdAux(vm);
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
        const SQBool ok = SQ_SUCCEEDED(sq_newslot(g_vm, -3, SQFalse));
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
        const SQBool ok = SQ_SUCCEEDED(sq_newslot(g_vm, -3, SQFalse));
        sq_settop(g_vm, savedTop);
        return ok;
    }
}

// Original 0x004bdc50 registers print/error closures then five extra stdlib batches on g_vm.
// FUNCTION: VC1 0x004bdc50
void RegisterStdBindings()
{
    UnmappedRegisterPrint(g_vm, 0);
    UnmappedRegisterPrintAlt(g_vm, 0);
    UnmappedRegisterExtra0();
    UnmappedRegisterExtra1();
    UnmappedRegisterExtra2();
    UnmappedRegisterExtra3();
    UnmappedRegisterExtra4();
}

}
