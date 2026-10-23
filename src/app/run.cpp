#include "kdcore/runtime.h"
#include "kdcore/native.h"

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

    // STRING: VC1 0x004d0c58
    SquirrelVM::GetRootTable().SetValue("IsCheatsEnabled", false);

    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionExA(&version);
    FormatBuffer(g_windowsVersionText, 0x80, "Windows v%ld.%ld.%ld",
        version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
    KdLog("OpenKODE Core v%s by %s on %s\n",
        "1.0.1 (Compiled Dec 21 2009 22:47:18)",
        "G5 Entertainment AB",
        g_windowsVersionText);

    {
        // STRING: VC1 0x004d0c88
        SquirrelObject mainNut = SquirrelVM::CompileScript("scripts/Main.nut");
        if (mainNut.IsNull()) {
            return -1;
        }

        SquirrelObject config = SquirrelVM::RunScript(mainNut, 0);
        display = static_cast<IDisplay*>(AcquireNamed(&g_kdNameDisplay, &g_iidDisplay));
        if (!display) {
            return static_cast<int>(0xfffffffe);
        }

        {
            // STRING: VC1 0x004d0c9c
            const unsigned height = static_cast<unsigned>(config.GetInt("DisplayHeight"));
            // STRING: VC1 0x004d0cac
            const unsigned width = static_cast<unsigned>(config.GetInt("DisplayWidth"));
            if (!display->SetSize(width, height)) {
                display->Release();
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
            return static_cast<int>(0xfffffffd);
        }
        if (abstractDisplay) {
            abstractDisplay->Release();
        }

        // STRING: VC1 0x004d0cbc
        SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("initApp"), 0);
        sound = static_cast<ISoundDriver*>(
            AcquireNamed(&g_kdNameSoundDriver, &g_iidSoundDriver));
        {
            if (!sound->SetEnabled(display->GetMixer())) {
                // STRING: VC1 0x004d0cc4
                config.SetValue("SoundEnabled", false);
            }
        }

        // STRING: VC1 0x004d0cd4
        SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("startApp"), 0);

        argv = reinterpret_cast<char**>(scratch1);
        argc = static_cast<int>(scratch0);
        for (i = 0; i < argc; ++i) {
            char* const arg = argv[i];
            if (arg && arg[0] == '+') {
                SquirrelVM::RunScript(SquirrelVM::CompileBuffer(arg + 1), 0);
            }
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
            // STRING: VC1 0x004d0cf0
            SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("pauseApp"), 0);
        }
        else if (token == 0x2e) {
            // STRING: VC1 0x004d0cfc
            SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("resumeApp"), 0);
        }
    }

    // STRING: VC1 0x004d0d08
    HostLog(&g_kdLogPrefix, "exiting...");
    // STRING: VC1 0x004d0d14
    SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("destroyApp"), 0);

    system->Shutdown();
    system->Release();
    if (sound) {
        sound->Release();
    }

    {
        IUnknown* const liveSystem = AcquireNamed(&g_kdNameSystem, &g_iidSystem);
        KdAddRef(liveSystem);
        SquirrelVM::Shutdown();
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
        display->Release();
        KdRelease(liveDisplay);
    }
    return 0;
}

}
