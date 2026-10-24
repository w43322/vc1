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

// Inferred names; both always inlined into RunApp (no original address).
// The object's IShutdownable face is shut down and released, then the object.
static void ShutdownAndRelease(IUnknown* object)
{
    IShutdownable* const shutdown = static_cast<IShutdownable*>(
        object->QueryInterface(&g_iidShutdownable));
    if (shutdown) {
        shutdown->AddRef();
    }
    shutdown->Shutdown();
    shutdown->Release();
    object->Release();
}

// The service object's face for iid with its own reference, or 0.
static IUnknown* QueryService(const char** iid)
{
    IUnknown* object = 0;
    if (g_serviceUnknown.ptr) {
        object = static_cast<IUnknown*>(g_serviceUnknown.ptr->QueryInterface(iid));
        if (object) {
            object->AddRef();
        }
    }
    return object;
}

// FUNCTION: VC1 0x004c38b0
int RunApp()
{
    OSVERSIONINFOA version;
    IDisplay* display;
    ISystem* system;
    ISoundDriver* sound;
    IUnknown* liveSystem;
    IUnknown* liveSound;
    IUnknown* liveDisplay;
    const int argc = static_cast<int>(g_runScratch0);
    char** const argv = reinterpret_cast<char**>(g_runScratch1);
    int i;
    int token;

    InitScriptEngine(argc, argv);
    RegisterStdBindings();

    // STRING: VC1 0x004d0c58
    SquirrelVM::GetRootTable().SetValue("IsCheatsEnabled", false);

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
        if (!display->SetSize(
                // STRING: VC1 0x004d0cac
                config.GetInt("DisplayWidth"),
                // STRING: VC1 0x004d0c9c
                config.GetInt("DisplayHeight"))) {
            display->Release();
            return -2;
        }

        system = static_cast<ISystem*>(AcquireNamed(&g_kdNameSystem, &g_iidSystem));
        {
            bool failed;
            {
                KdPtr<IAbstract> abstractDisplay(static_cast<IAbstract*>(
                    display->QueryInterface(&g_iidAbstract)));
                failed = !system->Start(reinterpret_cast<IUnknown**>(&abstractDisplay.ptr));
            }
            if (failed) {
                system->Release();
                display->Release();
                return -3;
            }
        }

        // STRING: VC1 0x004d0cbc
        SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("initApp"), 0);
        sound = static_cast<ISoundDriver*>(
            AcquireNamed(&g_kdNameSoundDriver, &g_iidSoundDriver));
        if (!sound->SetEnabled(display->GetMixer())) {
            // STRING: VC1 0x004d0cc4
            config.SetValue("SoundEnabled", false);
        }

        // STRING: VC1 0x004d0cd4
        SquirrelVM::RunScript(SquirrelVM::GetRootTable().GetValue("startApp"), 0);

        for (i = 0; i < argc; ++i) {
            if (argv[i][0] == '+') {
                SquirrelVM::RunScript(SquirrelVM::CompileBuffer(argv[i] + 1), 0);
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
    liveSystem = QueryService(&g_iidSystem);
    SquirrelVM::Shutdown();
    DestroySizeClassPools();
    ShutdownAndRelease(sound);
    liveSound = QueryService(&g_iidSoundDriver);
    ShutdownAndRelease(display);
    liveDisplay = QueryService(&g_iidDisplay);
    if (liveSound) {
        liveSound->Release();
    }
    if (liveSystem) {
        liveSystem->Release();
    }
    if (liveDisplay) {
        liveDisplay->Release();
    }
    return 0;
}

}
