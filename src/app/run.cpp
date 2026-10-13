#include "host/runtime.h"

#include "kd/factory.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/isystem.h"
#include "kd/iface/isound_driver.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/iid.h"

#include <squirrel.h>

namespace vc1 {

static void ShutdownInterface(IUnknown* object)
{
    IShutdownable* const shutdown = static_cast<IShutdownable*>(
        KdQueryInterface(object, &g_iidShutdownable));
    KdAddRef(shutdown);
    if (shutdown) {
        shutdown->Shutdown();
    }
    KdRelease(shutdown);
    KdRelease(object);
}

// FUNCTION: VC1 0x004c38b0
int RunApp()
{
    OSVERSIONINFOA version;
    SquirrelObject mainNut;
    SquirrelObject slot;
    IDisplay* display = 0;
    ISystem* system = 0;
    IAbstract* abstractDisplay = 0;
    ISoundDriver* sound = 0;
    unsigned scratch0 = g_runScratch0;
    unsigned scratch1 = g_runScratch1;
    int argc;
    int i;
    int token;
    char** argv;

    InitScriptEngine(static_cast<int>(scratch0), reinterpret_cast<char**>(scratch1));
    RegisterStdBindings();

    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
    // STRING: VC1 0x004d0c58
    RegisterBoolFalseSlot(&g_rootTable, "IsCheatsEnabled");

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
    // STRING: VC1 0x004d0c88
    CompilePath("scripts/Main.nut", &mainNut);
    if (mainNut.obj._type == OT_NULL) {
        return -1;
    }

    InvokeWithRoot(&mainNut, 0, &slot);
    display = static_cast<IDisplay*>(AcquireNamed(&g_kdNameDisplay, &g_iidDisplay));
    if (!display) {
        ResetSquirrelObject(&mainNut);
        ResetSquirrelObject(&slot);
        return static_cast<int>(0xfffffffe);
    }

    {
        // STRING: VC1 0x004d0c9c
        const unsigned height = static_cast<unsigned>(ReadIntFromSlot(&slot, "DisplayHeight"));
        // STRING: VC1 0x004d0cac
        const unsigned width = static_cast<unsigned>(ReadIntFromSlot(&slot, "DisplayWidth"));
        if (!display->SetSize(width, height)) {
            display->Release();
            ResetSquirrelObject(&mainNut);
            return static_cast<int>(0xfffffffe);
        }
    }

    system = static_cast<ISystem*>(AcquireNamed(&g_kdNameSystem, &g_iidSystem));
    abstractDisplay = static_cast<IAbstract*>(
        display->QueryInterface(&g_iidAbstract));
    if (abstractDisplay) {
        abstractDisplay->AddRef();
    }
    if (!system || !system->Start(reinterpret_cast<IUnknown**>(&abstractDisplay))) {
        if (system) {
            system->Release();
        }
        display->Release();
        ResetSquirrelObject(&slot);
        ResetSquirrelObject(&mainNut);
        return static_cast<int>(0xfffffffd);
    }
    if (abstractDisplay) {
        abstractDisplay->Release();
    }

    PushRootTable();
    // STRING: VC1 0x004d0cbc
    GetNamedObject(&g_rootTable, "initApp", &slot);
    InvokeWithRoot(&slot, 0, &slot);
    sound = static_cast<ISoundDriver*>(
        AcquireNamed(&g_kdNameSoundDriver, &g_iidSoundDriver));
    {
        void* const mix = display->GetMixer();
        (void)mix;
        if (sound && !sound->SetEnabled()) {
            // STRING: VC1 0x004d0cc4
            RegisterBoolFalseSlot(&slot, "SoundEnabled");
        }
    }

    PushRootTable();
    // STRING: VC1 0x004d0cd4
    GetNamedObject(&g_rootTable, "startApp", &slot);
    InvokeWithRoot(&slot, 0, &slot);
    ResetSquirrelObject(&slot);

    argv = reinterpret_cast<char**>(scratch1);
    argc = static_cast<int>(scratch0);
    for (i = 0; i < argc; ++i) {
        char* const arg = argv[i];
        if (arg && arg[0] == '+') {
            CompileString(arg + 1, &slot);
            InvokeWithRoot(&slot, 0, &slot);
            ResetSquirrelObject(&slot);
        }
    }

    if (g_vm) {
        ReleaseObjectIfNeeded(&slot.obj);
        ReleaseObjectIfNeeded(&mainNut.obj);
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
            // STRING: VC1 0x004d0cf0
            if (HasNamedSlot(&g_rootTable, "pauseApp")) {
                AttachStackTop(&slot);
                if (g_vm) {
                    sq_pop(g_vm, 1);
                }
            }
            if (g_vm) {
                sq_pop(g_vm, 1);
            }
            InvokeWithRoot(&slot, 0, &mainNut);
            ResetSquirrelObject(&slot);
        }
        else if (token == 0x2e) {
            if (g_rootTable.obj._type == OT_NULL && g_vm) {
                sq_pushroottable(g_vm);
                AttachStackTop(&g_rootTable);
                sq_pop(g_vm, 1);
            }
            // STRING: VC1 0x004d0cfc
            GetNamedObject(&g_rootTable, "resumeApp", &slot);
            InvokeWithRoot(&slot, 0, &slot);
            ResetSquirrelObject(&slot);
        }
    }

    // STRING: VC1 0x004d0d08
    HostLog(&g_kdLogPrefix, "exiting...");
    if (g_rootTable.obj._type == OT_NULL && g_vm) {
        sq_pushroottable(g_vm);
        AttachStackTop(&g_rootTable);
        sq_pop(g_vm, 1);
    }
    // STRING: VC1 0x004d0d14
    GetNamedObject(&g_rootTable, "destroyApp", &slot);
    InvokeWithRoot(&slot, 0, &slot);
    ReleaseObjectIfNeeded(&slot.obj);
    ResetSquirrelObject(&slot);

    if (sound) {
        sound->SetEnabled();
        sound->Release();
    }

    {
        IUnknown* const liveSystem = AcquireNamed(&g_kdNameSystem, &g_iidSystem);
        KdAddRef(liveSystem);
        CloseScriptVm();
        DestroySizeClassPools();
        ShutdownInterface(liveSystem);
        KdRelease(liveSystem);
    }

    {
        IUnknown* const liveSound = AcquireNamed(&g_kdNameSoundDriver, &g_iidSoundDriver);
        KdAddRef(liveSound);
        ShutdownInterface(liveSound);
        KdRelease(liveSound);
    }

    {
        IUnknown* const liveDisplay = AcquireNamed(&g_kdNameDisplay, &g_iidDisplay);
        KdAddRef(liveDisplay);
        if (system) {
            system->Release();
        }
        display->Release();
        KdRelease(liveDisplay);
    }
    return 0;
}

}
