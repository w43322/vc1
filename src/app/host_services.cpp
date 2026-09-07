#include "host/runtime.h"

#include <errno.h>
#include <math.h>
#include <string.h>

#include <squirrel.h>

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

// floor(x+0.5) unless |x| is +Inf at 0x004e95cc. Host extra used by math_round.
// FUNCTION: VC1 0x00499bc0
float RoundToNearest(float value)
{
    if (fabsf(value) != FloatFromBits(0x7f800000)) {
        value = (float)floor((double)(value + 0.5f));
    }
    return value;
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

// Cache KD_LOCALE or lang_COUNTRY from GetLocaleInfoA into DAT_004ed504.
// FUNCTION: VC1 0x004bab00
const char* GetResourceLocale()
{
    const char* env;
    unsigned length;
    LCID locale;
    char country[4];
    char lang[4];

    if (g_resourceLocale[0] != 0) {
        return g_resourceLocale;
    }
    // STRING: VC1 0x004d43fc
    env = GetKdEnv("KD_LOCALE");
    if (env) {
        length = 0;
        while (env[length] != 0) {
            length += 1;
        }
        length += 1;
        if (length <= 6) {
            memcpy(g_resourceLocale, env, length);
            return g_resourceLocale;
        }
        g_resourceLocale[0] = 0;
        errno = 0x11;
        return g_resourceLocale;
    }
    locale = GetThreadLocale();
    if (GetLocaleInfoA(locale, LOCALE_SISO3166CTRYNAME, country, 3)
        && GetLocaleInfoA(locale, LOCALE_SISO639LANGNAME, lang, 3)) {
        g_resourceLocale[0] = lang[0];
        g_resourceLocale[1] = lang[1];
        g_resourceLocale[2] = '_';
        g_resourceLocale[3] = country[0];
        g_resourceLocale[4] = country[1];
    }
    return g_resourceLocale;
}

// Host extra on the stock mathlib table as "fmod" (nparams 3). Stack slots 2/3, CRT fmod.
// FUNCTION: VC1 0x004bb800
SQInteger math_fmod(HSQUIRRELVM v)
{
    SQFloat x;
    SQFloat y;

    sq_getfloat(v, 2, &x);
    sq_getfloat(v, 3, &y);
    sq_pushfloat(v, (SQFloat)fmod((double)x, (double)y));
    return 1;
}

// Host extra on the stock mathlib table as "round" (nparams 2).
// FUNCTION: VC1 0x004bb890
SQInteger math_round(HSQUIRRELVM v)
{
    SQFloat x;

    sq_getfloat(v, 2, &x);
    sq_pushfloat(v, RoundToNearest(x));
    return 1;
}

}
