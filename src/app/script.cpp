#include "host/runtime.h"

#include <errno.h>
#include <squirrel.h>
#include <stdlib.h>

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

// ESI=file wrapper {inner, extra}. Null sets errno 0x11. Close is inner+0x118.
// FUNCTION: VC1 0x0048f260
int CloseKdFile(void* file)
{
    void* inner;
    void* extra;
    int status;
    typedef int (*KdCloseFn)(void* self, void* user);

    if (!file) {
        errno = 0x11;
        return -1;
    }
    inner = *reinterpret_cast<void**>(file);
    extra = reinterpret_cast<void**>(file)[1];
    status = (*reinterpret_cast<KdCloseFn*>(static_cast<char*>(inner) + 0x118))(inner, extra);
    free(file);
    return status;
}

// EAX=vm, cdecl user pointer (address of a KD object slot). sqstd_createfile with own=false.
// FUNCTION: VC1 0x0048f8b0
int CreateStdFile(HSQUIRRELVM vm, void* user)
{
    const SQInteger savedTop =
        *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30)
        - *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);
    HSQOBJECT* slot;
    SQInteger top;
    unsigned oldType;
    int* oldValue;

    sq_pushregistrytable(vm);
    sq_pushstring(vm, "std_file", -1);
    if (!UnmappedGet(vm)) {
        sq_pop(vm, 1);
        // STRING: VC1 0x004d26a4
        if (sq_throwerror(vm, "the index doesn't exist") < 0) {
            sq_settop(vm, savedTop);
            return 0;
        }
    }
    sq_remove(vm, -2);
    sq_pushroottable(vm);
    top = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    slot = reinterpret_cast<HSQOBJECT*>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x18) + top * 8);
    oldType = slot->_type;
    oldValue = reinterpret_cast<int*>(slot->_unVal.pUserPointer);
    slot->_unVal.pUserPointer = user;
    slot->_type = OT_USERPOINTER;
    *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30) = top + 1;
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
    sq_pushnull(vm);
    if (sq_call(vm, 3, SQTrue, SQFalse) < 0) {
        sq_settop(vm, savedTop);
        return 0;
    }
    sq_remove(vm, -2);
    return 0;
}

// ESI=vm. Caller OpenScriptVm. Top is vm+0x30 minus vm+0x34, not sq_gettop.
// FUNCTION: VC1 0x0048f990
int RegisterStdFile(HSQUIRRELVM vm)
{
    const SQInteger savedTop =
        *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30)
        - *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);
    UnmappedRegisterStdBlob(vm,
        // STRING: VC1 0x004d2f1c
        "file", 0x80000001,
        // STRING: VC1 0x004d2f70
        "std_file",
        reinterpret_cast<void*>(0x004e9360), reinterpret_cast<void*>(0x004e9390));
    // STRING: VC1 0x004d3004
    sq_pushstring(vm, "stdout", -1);
    CreateStdFile(vm, &g_kdObjectIt1);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d300c
    sq_pushstring(vm, "stdin", -1);
    CreateStdFile(vm, &g_kdObjectIt2);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d3014
    sq_pushstring(vm, "stderr", -1);
    CreateStdFile(vm, &g_kdObjectIt0);
    sq_newslot(vm, -3, SQFalse);
    sq_settop(vm, savedTop);
    return 0;
}

// EAX=vm, cdecl path + raiseerror. Open "rb", detect BOM/bytecode, then sq_compile.
// Inferred name. Called from CompilePath.
// FUNCTION: VC1 0x0048fa50
int LoadScriptFile(HSQUIRRELVM vm, const SQChar* path, int raiseerror)
{
    void* file;
    void* inner;
    void* extra;
    SQLEXREADFUNC reader;
    unsigned short bom;
    char third;
    int nread;

    typedef int (*KdReadFn)(void* self, void* buf, int size, int count, void* user);
    typedef int (*KdSeekFn)(void* self, void* user, int a, int b, int c);

    // STRING: VC1 0x004d240c
    file = OpenKdFile(path, "rb");
    reader = reinterpret_cast<SQLEXREADFUNC>(0x0048f5b0);
    if (!file) {
        // STRING: VC1 0x004d2fd8
        return sq_throwerror(vm, "cannot open the file");
    }
    inner = *reinterpret_cast<void**>(file);
    extra = reinterpret_cast<void**>(file)[1];
    nread = (*reinterpret_cast<KdReadFn*>(static_cast<char*>(inner) + 0x120))(
        inner, &bom, 1, 2, extra);
    if (nread == 2 && bom == 0xfafa) {
        inner = *reinterpret_cast<void**>(file);
        extra = reinterpret_cast<void**>(file)[1];
        (*reinterpret_cast<KdSeekFn*>(static_cast<char*>(inner) + 0x140))(
            inner, extra, 0, 0, 0);
        if (UnmappedLoadBytecode(vm) >= 0) {
            CloseKdFile(file);
            return 0;
        }
        CloseKdFile(file);
        return -1;
    }
    if (nread != 2) {
        bom = 0;
    }
    if (bom == 0xbbef) {
        inner = *reinterpret_cast<void**>(file);
        extra = reinterpret_cast<void**>(file)[1];
        nread = (*reinterpret_cast<KdReadFn*>(static_cast<char*>(inner) + 0x120))(
            inner, &third, 1, 1, extra);
        if (nread == 0) {
            CloseKdFile(file);
            // STRING: VC1 0x004d2720
            return sq_throwerror(vm, "io error");
        }
        if (third != '\xbf') {
            CloseKdFile(file);
            // STRING: VC1 0x004d2fc0
            return sq_throwerror(vm, "Unrecognozed ecoding");
        }
        reader = reinterpret_cast<SQLEXREADFUNC>(0x0048f500);
    }
    else if (bom == 0xfeff) {
        reader = reinterpret_cast<SQLEXREADFUNC>(0x0048f4c0);
    }
    else if (bom == 0xfffe) {
        reader = reinterpret_cast<SQLEXREADFUNC>(0x0048f470);
    }
    else {
        inner = *reinterpret_cast<void**>(file);
        extra = reinterpret_cast<void**>(file)[1];
        (*reinterpret_cast<KdSeekFn*>(static_cast<char*>(inner) + 0x140))(
            inner, extra, 0, 0, 0);
    }
    if (UnmappedSqCompile(vm, reader, file, path, raiseerror) >= 0) {
        CloseKdFile(file);
        return 0;
    }
    CloseKdFile(file);
    return -1;
}

// Native loadfile: string at idx 2, optional bool at idx 3, then LoadScriptFile. Returns 1 or -1.
// FUNCTION: VC1 0x0048fbd0
SQInteger IoLoadFile(HSQUIRRELVM vm)
{
    HSQOBJECT* obj;
    const SQChar* path;
    int raiseerror;
    SQInteger stackbase;
    SQInteger top;

    stackbase = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);
    obj = reinterpret_cast<HSQOBJECT*>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x18) + stackbase * 8 + 8);
    path = 0;
    if (obj->_type == OT_STRING) {
        path = reinterpret_cast<const SQChar*>(
            reinterpret_cast<char*>(obj->_unVal.pString) + 0x1c);
    }
    else {
        UnmappedWrongArgType(vm, obj);
    }
    raiseerror = 0;
    top = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    if (top - stackbase >= 3) {
        obj = reinterpret_cast<HSQOBJECT*>(
            *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x18) + stackbase * 8 + 0x10);
        if (obj->_type == OT_BOOL) {
            raiseerror = obj->_unVal.nInteger;
        }
    }
    if (LoadScriptFile(vm, path, raiseerror) >= 0) {
        return 1;
    }
    return -1;
}

// ECX=vm, EAX=path, cdecl raiseerror. Logs then load+push(-2)+call(1).
// FUNCTION: VC1 0x0048fc80
SQInteger DoScriptFile(HSQUIRRELVM vm, const SQChar* path, int raiseerror)
{
    // STRING: VC1 0x004d2ff0
    KdLog("[dofile] %s\n", path);
    if (LoadScriptFile(vm, path, raiseerror) < 0) {
        return -1;
    }
    sq_push(vm, -2);
    if (sq_call(vm, 1, SQTrue, SQTrue) < 0) {
        sq_pop(vm, 1);
        return -1;
    }
    sq_remove(vm, -2);
    return 1;
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

// Host sq_open. malloc 0xb4 sharedstate + 0xa8 VM; Init ECX=0x400. Caller OpenScriptVm.
// FUNCTION: VC1 0x00499750
HSQUIRRELVM SqOpen()
{
    void* ss;
    HSQUIRRELVM vm;
    unsigned oldType;
    int* oldValue;

    g_sizeClassHeap.allocatedBytes += 0xb4;
    ss = malloc(0xb4);
    if (ss) {
        UnmappedSharedStateCtor(ss);
    }
    UnmappedSharedStateInit(ss);

    g_sizeClassHeap.allocatedBytes += 0xa8;
    vm = static_cast<HSQUIRRELVM>(malloc(0xa8));
    if (vm) {
        UnmappedVmCtor(vm, ss);
    }
    reinterpret_cast<int*>(vm)[1] += 1;
    oldType = *reinterpret_cast<unsigned*>(static_cast<char*>(ss) + 0x48);
    oldValue = *reinterpret_cast<int**>(static_cast<char*>(ss) + 0x4c);
    *reinterpret_cast<void**>(static_cast<char*>(ss) + 0x4c) = vm;
    *reinterpret_cast<unsigned*>(static_cast<char*>(ss) + 0x48) = OT_THREAD;
    reinterpret_cast<int*>(vm)[1] += 1;
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
    reinterpret_cast<int*>(vm)[1] -= 1;
    if (reinterpret_cast<int*>(vm)[1] == 0) {
        UnmappedReleaseRefCounted(reinterpret_cast<int*>(vm));
    }
    if (UnmappedVmInit(vm, 0)) {
        return vm;
    }
    UnmappedVmDelete(vm);
    g_sizeClassHeap.releasedBytes += 0xa8;
    free(vm);
    return 0;
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
    vm = SqOpen();
    if (!vm) {
        return 0;
    }
    *reinterpret_cast<void**>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa4)
        = reinterpret_cast<void*>(0x00499e00);
    sq_pushroottable(vm);
    RegisterStdFile(vm);
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
        const SQBool ok = SQ_SUCCEEDED(RawSet(g_vm));
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
        const SQBool ok = SQ_SUCCEEDED(RawSet(g_vm));
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
