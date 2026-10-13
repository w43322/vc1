#include "host/runtime.h"

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <squirrel.h>

namespace vc1 {

// Last instruction of InitHostServices is JMP here.
// STUB: VC1 0x0042b1e0
static void BindHostApi()
{
}

static float FloatFromBits(unsigned bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
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

// SQPRINTFUNCTION at sharedstate+0xa4 from OpenScriptVm. Dest 0x004ed9d0 then KdLog.
// FUNCTION: VC1 0x00499e00
void ScriptPrint(HSQUIRRELVM, const SQChar* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _vsnprintf(g_scriptPrintBuf, (size_t)-1, fmt, args);
    va_end(args);
    KdLog(g_scriptPrintBuf);
}

// INFINITY/INT_*/FLT_* and UPDATE_LAYER_* on the root table, then host native binds.
// Last instruction is JMP 0x0042b1e0. Inferred name.
// FUNCTION: VC1 0x004b61b0
void InitHostServices()
{
    unsigned infBits = 0x7f800000;

    // STRING: VC1 0x004d7620
    SquirrelVM::GetRootTable().SetValue("INFINITY", FloatFromBits(infBits));
    // STRING: VC1 0x004d762c
    SquirrelVM::GetRootTable().SetValue("INT_MAX", 0x7fffffff);
    // STRING: VC1 0x004d7634
    SquirrelVM::GetRootTable().SetValue("INT_MIN", static_cast<SQInteger>(0x80000000));
    // STRING: VC1 0x004d763c
    SquirrelVM::GetRootTable().SetValue("FLT_MAX", FloatFromBits(0x7f7fffff));
    // STRING: VC1 0x004d7644
    SquirrelVM::GetRootTable().SetValue("FLT_MIN", FloatFromBits(0x00800000));
    // STRING: VC1 0x004d764c
    SquirrelVM::GetRootTable().SetValue("UPDATE_LAYER_MOVEMENT", 2);
    // STRING: VC1 0x004d7664
    SquirrelVM::GetRootTable().SetValue("UPDATE_LAYER_DYNAMICS", 4);
    // STRING: VC1 0x004d767c
    SquirrelVM::GetRootTable().SetValue("UPDATE_LAYER_EFFECTS", 0x40);
    // STRING: VC1 0x004d7694
    SquirrelVM::GetRootTable().SetValue("UPDATE_LAYER_MAX", 0x100);
    BindHostApi();
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
