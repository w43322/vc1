#include "host/runtime.h"
#include "app/host_api.h"
#include "bindings/matrix3.h"
#include "bindings/matrix4.h"
#include "bindings/rectf.h"
#include "bindings/sqplus.h"

#include <mmsystem.h>

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <squirrel.h>

namespace vc1 {

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

// INFINITY/INT_*/FLT_* and UPDATE_LAYER_* on the root table, then the g5
// natives (host_api.cpp). The RegisterGlobal instantiations with one caller
// are folded into helpers (the last, 0x0042b1e0, is a tail jump). Inferred
// name.
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
    // STRING: VC1 0x004d76a8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getComponent, "getComponent");
    // STRING: VC1 0x004d76b8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getComponentIDs, "getComponentIDs");
    // STRING: VC1 0x004d76c8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &quit, "quit");
    // STRING: VC1 0x004d76d0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &logMessage, "logMessage");
    // STRING: VC1 0x004d76dc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &logWarning, "logWarning");
    // STRING: VC1 0x004d76e8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &logError, "logError");
    // STRING: VC1 0x004d76f4
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &logInfo, "logInfo");
    // STRING: VC1 0x004d76fc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &shutdown, "shutdown");
    // STRING: VC1 0x004d7708
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &loadFromFile, "loadFromFile");
    // STRING: VC1 0x004d7718
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &saveToFile, "saveToFile");
    // STRING: VC1 0x004d7724
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &cloneComponent, "cloneComponent");
    // STRING: VC1 0x004d7734
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &enableControl, "enableControl");
    // STRING: VC1 0x004d7744
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &isControlEnabled, "isControlEnabled");
    // STRING: VC1 0x004d7758
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setSlaveObject, "setSlaveObject");
    // STRING: VC1 0x004d7768
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &loadFromScript, "loadFromScript");
    // STRING: VC1 0x004d7778
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &saveToScript, "saveToScript");
    // STRING: VC1 0x004d7788
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &isIdentifiable, "isIdentifiable");
    // STRING: VC1 0x004d7798
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getIdentificator, "getIdentificator");
    // STRING: VC1 0x004d77ac
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setIdentificator, "setIdentificator");
    // STRING: VC1 0x004d77c0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &addClassificator, "addClassificator");
    // STRING: VC1 0x004d77d4
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &removeClassificator, "removeClassificator");
    // STRING: VC1 0x004d77e8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getClassificators, "getClassificators");
    // STRING: VC1 0x004d77fc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &checkMask, "checkMask");
    // STRING: VC1 0x004d7808
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getPosition, "getPosition");
    // STRING: VC1 0x004d7814
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setPosition, "setPosition");
    // STRING: VC1 0x004d7820
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setPositionable, "setPositionable");
    // STRING: VC1 0x004d7830
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setAnimation, "setAnimation");
    // STRING: VC1 0x004d7840
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &isVisible, "isVisible");
    // STRING: VC1 0x004d784c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setVisible, "setVisible");
    // STRING: VC1 0x004d7858
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readComponent, "readComponent");
    // STRING: VC1 0x004d7868
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readInt, "readInt");
    // STRING: VC1 0x004d7870
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readShort, "readShort");
    // STRING: VC1 0x004d787c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readUnsignedShort, "readUnsignedShort");
    // STRING: VC1 0x004d7890
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readByte, "readByte");
    // STRING: VC1 0x004d789c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readUnsignedByte, "readUnsignedByte");
    // STRING: VC1 0x004d78b0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readBool, "readBool");
    // STRING: VC1 0x004d78bc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &readString, "readString");
    // STRING: VC1 0x004d78c8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeComponent, "writeComponent");
    // STRING: VC1 0x004d78d8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeBuffer, "writeBuffer");
    // STRING: VC1 0x004d78e4
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeInt, "writeInt");
    // STRING: VC1 0x004d78f0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeShort, "writeShort");
    // STRING: VC1 0x004d78fc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeByte, "writeByte");
    // STRING: VC1 0x004d7908
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeBool, "writeBool");
    // STRING: VC1 0x004d7914
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &writeString, "writeString");
    // STRING: VC1 0x004d7920
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &updateComponent, "updateComponent");
    // STRING: VC1 0x004d7930
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &renderComponent, "renderComponent");
    // STRING: VC1 0x004d7940
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &beginRender, "beginRender");
    // STRING: VC1 0x004d794c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &endRender, "endRender");
    // STRING: VC1 0x004d7958
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getRenderTargetTexture, "getRenderTargetTexture");
    // STRING: VC1 0x004d7970
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getBoundingRect, "getBoundingRect");
    // STRING: VC1 0x004d7980
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getLifeTime, "getLifeTime");
    // STRING: VC1 0x004d798c
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &isExpired, "isExpired");
    // STRING: VC1 0x004d7998
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getTime, "getTime");
    // STRING: VC1 0x004d79a0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &getSystemTime, "getSystemTime");
    // STRING: VC1 0x004d79b0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &setTimer, "setTimer");
    // STRING: VC1 0x004d79bc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &cancelTimer, "cancelTimer");
    // STRING: VC1 0x004d79c8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &fileExists, "fileExists");
    // STRING: VC1 0x004d79d4
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &randNum, "randNum");
    // STRING: VC1 0x004d79dc
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &randVal, "randVal");
    // STRING: VC1 0x004d79e4
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &angleDelta, "angleDelta");
    // STRING: VC1 0x004d79f0
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &fltsign, "fltsign");
    // STRING: VC1 0x004d79f8
    SqPlus::RegisterGlobal(SquirrelVM::_VM, &TextureFilter_ColorMatrix, "TextureFilter_ColorMatrix");
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

// Preserve this WinMM implementation in its original translation unit:
// moving it into a header changes VC9 register allocation. The SDL3 target
// supplies the same PollGamepad service from platform/sdl3/gamepad.cpp.
#if !defined(VC1_USE_SDL3)
// Inferred names. Joystick 0 as read by PollGamepad.
// GLOBAL: VC1 0x004e9080
JOYINFOEX g_joyInfo;
// GLOBAL: VC1 0x004e90b4
MMRESULT g_joyResult;
// GLOBAL: VC1 0x004ea84c
DWORD g_joyButtons;
// Point-of-view hat directions up, left, right, down (0/1).
// GLOBAL: VC1 0x004eb0d8
unsigned g_joyHat[4];

// joyGetPosEx(0) into g_joyInfo; posts an input event (0x38) for every
// changed button (key 0x9001 + bit, data 1 when pressed) and for every
// changed hat direction (key 0x8002 + direction). The hat events are created
// and queued inline without the pool's malloc check.
// FUNCTION: VC1 0x004babe0
void PollGamepad()
{
    unsigned hat[4];
    LARGE_INTEGER now;
    unsigned i;

    g_joyResult = joyGetPosEx(0, &g_joyInfo);
    if (g_joyResult != JOYERR_NOERROR) {
        return;
    }
    const DWORD buttons = g_joyInfo.dwButtons;
    const DWORD pov = g_joyInfo.dwPOV;
    unsigned bit = 1;
    for (i = 0; i < 32; ++i) {
        const DWORD pressed = bit & buttons;
        if ((bit & g_joyButtons) != pressed) {
            KdEvent* const event = AllocEvent();
            event->type = 0x38;
            event->userptr = g_kdWindowMixer ? *static_cast<void**>(g_kdWindowMixer) : 0;
            event->data[0] = i + 0x9001;
            *reinterpret_cast<__int64*>(&event->data[2]) = pressed != 0;
            QueueEvent(event);
        }
        bit = _rotl(bit, 1);
    }
    if (pov == 0xffff) {
        hat[0] = 0;
        hat[1] = 0;
        hat[2] = 0;
        hat[3] = 0;
    }
    else {
        hat[0] = pov - 9000 > 18000;
        hat[1] = pov > 18000;
        hat[2] = pov - 1 < 17999;
        hat[3] = pov - 9001 < 17999;
    }
    for (i = 0; i < 4; ++i) {
        const unsigned value = hat[i];
        if (g_joyHat[i] != value) {
            KdEvent* event = reinterpret_cast<KdEvent*>(g_bumpArena);
            for (;;) {
                if (event->type == 0) {
                    event->type = -1;
                    break;
                }
                ++event;
                if (event >= reinterpret_cast<KdEvent*>(&g_heapHead)) {
                    event = static_cast<KdEvent*>(malloc(sizeof(KdEvent)));
                    break;
                }
            }
            event->timestamp = 0;
            event->userptr = 0;
            event->owner = 0;
            event->type = 0x38;
            event->userptr = g_kdWindowMixer ? *static_cast<void**>(g_kdWindowMixer) : 0;
            *reinterpret_cast<__int64*>(&event->data[2]) = static_cast<int>(value);
            event->data[0] = i + 0x8002;
            QueryPerformanceCounter(&now);
            event->timestamp =
                static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
            event->prev = g_heapTail;
            event->next = 0;
            g_heapTail = event;
            if (!event->prev) {
                g_heapHead = event;
            }
            else {
                event->prev->next = event;
            }
        }
    }
    g_joyButtons = buttons;
    g_joyHat[0] = hat[0];
    g_joyHat[1] = hat[1];
    g_joyHat[2] = hat[2];
    g_joyHat[3] = hat[3];
}

#endif

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
