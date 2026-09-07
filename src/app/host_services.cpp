#include "host/runtime.h"

#include <string.h>

namespace vc1 {

static float FloatFromBits(unsigned bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

static void EnsureRootTable()
{
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
}

// INFINITY/INT_*/FLT_* and UPDATE_LAYER_* on the root table, then host native binds.
// Last instruction is JMP 0x0042b1e0. Inferred name.
// FUNCTION: VC1 0x004b61b0
void InitHostServices()
{
    unsigned infBits = 0x7f800000;

    EnsureRootTable();
    // STRING: VC1 0x004d7620
    RegisterFloatSlot(&g_rootTable, "INFINITY", FloatFromBits(infBits));
    EnsureRootTable();
    // STRING: VC1 0x004d762c
    RegisterIntegerSlot(&g_rootTable, "INT_MAX", 0x7fffffff);
    EnsureRootTable();
    // STRING: VC1 0x004d7634
    RegisterIntegerSlot(&g_rootTable, "INT_MIN", static_cast<SQInteger>(0x80000000));
    EnsureRootTable();
    // STRING: VC1 0x004d763c
    RegisterFloatSlot(&g_rootTable, "FLT_MAX", FloatFromBits(0x7f7fffff));
    EnsureRootTable();
    // STRING: VC1 0x004d7644
    RegisterFloatSlot(&g_rootTable, "FLT_MIN", FloatFromBits(0x00800000));
    EnsureRootTable();
    // STRING: VC1 0x004d764c
    RegisterIntegerSlot(&g_rootTable, "UPDATE_LAYER_MOVEMENT", 2);
    EnsureRootTable();
    // STRING: VC1 0x004d7664
    RegisterIntegerSlot(&g_rootTable, "UPDATE_LAYER_DYNAMICS", 4);
    EnsureRootTable();
    // STRING: VC1 0x004d767c
    RegisterIntegerSlot(&g_rootTable, "UPDATE_LAYER_EFFECTS", 0x40);
    EnsureRootTable();
    // STRING: VC1 0x004d7694
    RegisterIntegerSlot(&g_rootTable, "UPDATE_LAYER_MAX", 0x100);
    UnmappedBindHostApi();
}

}
