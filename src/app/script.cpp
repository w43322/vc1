#include "host/runtime.h"

#include <squirrel.h>

namespace vc1 {

static const unsigned kSqRefCounted = 0x08000000;

// InitSizeClassPools, host/script bind, then attach the root table if it is still OT_NULL.
// FUNCTION: VC1 0x00463b40
void InitScriptEngine()
{
    unsigned i;

    InitSizeClassPools();
    UnmappedOpenVm();
    UnmappedBindHostClasses();
    UnmappedInitHostServices();

    {
        SquirrelObject scratch;
        InitSquirrelObject(&scratch);
        UnmappedPushRegistry();
        AttachStackTop(&scratch);
        if (g_vm) {
            sq_pop(g_vm, 1);
        }
        for (i = 0; i < 0; ++i) {
            UnmappedBindIndex(i);
        }
        if (g_rootTable.obj._type == OT_NULL && g_vm) {
            sq_pushroottable(g_vm);
            AttachStackTop(&g_rootTable);
            sq_pop(g_vm, 1);
        }
        UnmappedFinishRoot();
        scratch.vftable = g_squirrelObjectVftable;
        if (g_vm && (scratch.obj._type & kSqRefCounted) != 0) {
            UnmappedReleaseRef();
        }
    }
}

// sq_pushroottable + AttachStackTop on 0x004e96e0 when the stored type is still OT_NULL.
// FUNCTION: VC1 0x00499fe0
SquirrelObject* PushRootTable()
{
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
    return &g_rootTable;
}

// Release the root table if it is refcounted, then sq_close-style teardown of g_vm.
// FUNCTION: VC1 0x0049a170
void CloseScriptVm()
{
    HSQUIRRELVM vm = g_vm;
    if (g_rootTable.obj._type != OT_NULL) {
        if (g_vm && (g_rootTable.obj._type & kSqRefCounted) != 0) {
            UnmappedReleaseRef();
        }
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

// Store the recovered vftable, release a refcounted HSQOBJECT, then write OT_NULL.
// FUNCTION: VC1 0x0049ac80
void ResetSquirrelObject(SquirrelObject* object)
{
    object->vftable = g_squirrelObjectVftable;
    if (g_vm && (object->obj._type & kSqRefCounted) != 0) {
        UnmappedReleaseRef();
    }
    object->obj._unVal.pUserPointer = 0;
    object->obj._type = OT_NULL;
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
