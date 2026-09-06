#include "host/runtime.h"

#include <squirrel.h>

namespace vc1 {

static const unsigned kSqRefCounted = 0x08000000;

typedef void* (__thiscall *KdQueryInterfaceFn)(void* self, const char* name);
typedef unsigned (__thiscall *KdAddRefFn)(void* self);
typedef unsigned (__thiscall *KdReleaseFn)(void* self);
typedef int (__thiscall *KdBoolMethodFn)(void* self);
typedef int (__thiscall *KdBoolMethod1Fn)(void* self, unsigned arg);
typedef void* (__thiscall *KdPtrMethodFn)(void* self);

static void** Vtable(void* object)
{
    return object ? *reinterpret_cast<void***>(object) : 0;
}

static void* QueryInterface(void* object, const char* name)
{
    void** const vtbl = Vtable(object);
    if (!vtbl || !vtbl[0]) {
        return 0;
    }
    return (reinterpret_cast<KdQueryInterfaceFn>(vtbl[0]))(object, name);
}

static void AddRef(void* object)
{
    void** const vtbl = Vtable(object);
    if (object && vtbl && vtbl[1]) {
        (reinterpret_cast<KdAddRefFn>(vtbl[1]))(object);
    }
}

static void Release(void* object)
{
    void** const vtbl = Vtable(object);
    if (object && vtbl && vtbl[2]) {
        (reinterpret_cast<KdReleaseFn>(vtbl[2]))(object);
    }
}

static int CallBool(void* object, unsigned offset)
{
    void** const vtbl = Vtable(object);
    const unsigned index = offset / sizeof(void*);
    if (!object || !vtbl || !vtbl[index]) {
        return 0;
    }
    return (reinterpret_cast<KdBoolMethodFn>(vtbl[index]))(object);
}

static int CallBool1(void* object, unsigned offset, unsigned arg)
{
    void** const vtbl = Vtable(object);
    const unsigned index = offset / sizeof(void*);
    if (!object || !vtbl || !vtbl[index]) {
        return 0;
    }
    return (reinterpret_cast<KdBoolMethod1Fn>(vtbl[index]))(object, arg);
}

static void* CallPtr(void* object, unsigned offset)
{
    void** const vtbl = Vtable(object);
    const unsigned index = offset / sizeof(void*);
    if (!object || !vtbl || !vtbl[index]) {
        return 0;
    }
    return (reinterpret_cast<KdPtrMethodFn>(vtbl[index]))(object);
}

static void ShutdownInterface(void* object)
{
    void* const shutdown = QueryInterface(object, "IShutdownable");
    AddRef(shutdown);
    CallBool(shutdown, 0x0c);
    Release(shutdown);
    Release(object);
}

static void* AcquireNamed(const char* factoryName, const char* interfaceName)
{
    void* unknown = 0;
    void* typed = 0;
    UnmappedQueryNamed(UnmappedFactory(), &unknown, factoryName);
    if (!unknown) {
        return 0;
    }
    typed = QueryInterface(unknown, interfaceName);
    AddRef(typed);
    Release(unknown);
    return typed;
}

// WinMain's body: script VM, IDisplay/ISystem/ISoundDriver, +cmdline, token loop, shutdown.
// Returns 0, -1 (Main.nut missing), -2 (display), or -3 (system). Inferred name.
// FUNCTION: VC1 0x004c38b0
int RunApp()
{
    OSVERSIONINFOA version;
    SquirrelObject mainNut;
    SquirrelObject slot;
    void* display = 0;
    void* system = 0;
    void* abstractDisplay = 0;
    void* sound = 0;
    unsigned scratch0 = g_runScratch0;
    unsigned scratch1 = g_runScratch1;
    int argc;
    int i;
    int token;
    (void)scratch0;
    (void)scratch1;

    InitScriptEngine();
    RegisterStdBindings();

    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
    UnmappedGetSlot(&g_rootTable, "IsCheatsEnabled");

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionExA(&version);
    FormatBuffer(g_windowsVersionText, 0x80, "Windows v%ld.%ld.%ld",
        version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
    KdLog("OpenKODE Core v%s by %s on %s\n",
        "1.0.1 (Compiled Dec 21 2009 22:47:18)",
        "G5 Entertainment AB",
        g_windowsVersionText);

    InitSquirrelObject(&mainNut);
    UnmappedLoadNut(&mainNut, "scripts/Main.nut");
    if (mainNut.obj._type == OT_NULL) {
        return -1;
    }

    UnmappedNewSlot(&mainNut, 0);
    display = AcquireNamed("Display", "IDisplay");
    if (!display) {
        ResetSquirrelObject(&mainNut);
        ResetSquirrelObject(&slot);
        return static_cast<int>(0xfffffffe);
    }

    {
        const unsigned width = UnmappedReadInt();
        const unsigned height = UnmappedReadInt();
        if (!CallBool1(display, 0x20, width | height)) {
            Release(display);
            ResetSquirrelObject(&mainNut);
            return static_cast<int>(0xfffffffe);
        }
    }

    system = AcquireNamed("System", "ISystem");
    abstractDisplay = QueryInterface(display, "IAbstract");
    AddRef(abstractDisplay);
    if (!CallBool(system, 0x28)) {
        Release(system);
        Release(display);
        ResetSquirrelObject(&slot);
        ResetSquirrelObject(&mainNut);
        return static_cast<int>(0xfffffffd);
    }
    if (abstractDisplay) {
        Release(abstractDisplay);
    }

    PushRootTable();
    UnmappedPopValue(&slot);
    UnmappedNewSlot(&mainNut, 0);
    sound = AcquireNamed("SoundDriver", "ISoundDriver");
    {
        void* const mix = CallPtr(display, 0x2c);
        (void)mix;
        if (!CallBool(sound, 0x0c)) {
            UnmappedGetSlot(&g_rootTable, "IsCheatsEnabled");
        }
    }

    PushRootTable();
    UnmappedPopValue(&slot);
    UnmappedNewSlot(&mainNut, 0);
    ResetSquirrelObject(&slot);

    argc = 0;
    for (i = 0; i < argc; ++i) {
        char* const arg = 0;
        if (arg && arg[0] == '+') {
            UnmappedRunConsole(arg + 1);
            UnmappedNewSlot(&mainNut, 0);
            ResetSquirrelObject(&slot);
        }
    }

    if (g_vm) {
        if ((abstractDisplay != 0) && (reinterpret_cast<unsigned>(abstractDisplay) & kSqRefCounted)) {
            UnmappedReleaseRef();
        }
        if (g_vm && (slot.obj._type & kSqRefCounted) != 0) {
            UnmappedReleaseRef();
        }
    }

    for (;;) {
        void* event;
        do {
            event = PopTimedEvent();
        } while (!event);
        token = static_cast<int*>(event)[2];
        if (token == 0x2b) {
            break;
        }
        if (token == 0x2d) {
            if (g_rootTable.obj._type == OT_NULL && g_vm) {
                sq_pushroottable(g_vm);
                AttachStackTop(&g_rootTable);
                sq_pop(g_vm, 1);
            }
            if (UnmappedHasSlot()) {
                AttachStackTop(&slot);
                if (g_vm) {
                    sq_pop(g_vm, 1);
                }
            }
            if (g_vm) {
                sq_pop(g_vm, 1);
            }
            UnmappedNewSlot(&mainNut, 0);
            ResetSquirrelObject(&slot);
        }
        else if (token == 0x2e) {
            if (g_rootTable.obj._type == OT_NULL && g_vm) {
                sq_pushroottable(g_vm);
                AttachStackTop(&g_rootTable);
                sq_pop(g_vm, 1);
            }
            UnmappedPopValue(&slot);
            UnmappedNewSlot(&mainNut, 0);
            ResetSquirrelObject(&slot);
        }
    }

    UnmappedLog("exiting...");
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
    UnmappedPopValue(&slot);
    UnmappedNewSlot(&mainNut, 0);
    if (g_vm && (slot.obj._type & kSqRefCounted) != 0) {
        UnmappedReleaseRef();
    }
    ResetSquirrelObject(&slot);

    CallBool(sound, 0x0c);
    Release(sound);

    {
        void* const liveSystem = QueryInterface(g_serviceUnknown, "ISystem");
        AddRef(liveSystem);
        CloseScriptVm();
        DestroySizeClassPools();
        ShutdownInterface(liveSystem);
        Release(liveSystem);
    }

    {
        void* const liveSound = QueryInterface(g_serviceUnknown, "ISoundDriver");
        AddRef(liveSound);
        ShutdownInterface(liveSound);
        Release(liveSound);
    }

    {
        void* const liveDisplay = QueryInterface(g_serviceUnknown, "IDisplay");
        AddRef(liveDisplay);
        Release(system);
        Release(display);
        Release(liveDisplay);
    }
    return 0;
}

}
