#include "kd/sound/sound_driver.h"
#include "kd/sound/bass_api.h"
#include "kd/sound/music.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"
#include "bindings/sqplus.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed548
IAbstract* g_kdSoundDriver;

// STRING: VC1 0x004d4bfc
static const char kSoundDriver[] = "SoundDriver";
// GLOBAL: VC1 0x004d064c
const char* g_kdNameSoundDriver = kSoundDriver;

// MOV EAX, 0x004d4bfc; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x004197f0
const char* KdSoundDriver::Script14()
{
    return kSoundDriver;
}

// STUB: VC1 0x0045f960
static void Sound10Body()
{
}

KdSoundDriver::KdSoundDriver()
{
}

// RET 4. ISoundDriver vtable+0x28: BASS_SetVolume((float)volume) != 0.
// FUNCTION: VC1 0x004680e0
bool KdSoundDriver::SetVolume(int volume)
{
    return BASS_SetVolume(static_cast<float>(volume)) != 0;
}

// ISoundDriver vtable+0x24: BASS_GetVolume, then JMP _ftol.
// FUNCTION: VC1 0x00468100
int KdSoundDriver::GetVolume()
{
    return static_cast<int>(BASS_GetVolume());
}

// CALL IAT; NEG/SBB/NEG EAX.
// FUNCTION: VC1 0x00468110
bool KdSoundDriver::Pause()
{
    return BASS_Pause() != 0;
}

// FUNCTION: VC1 0x00468120
bool KdSoundDriver::Stop()
{
    return BASS_Stop() != 0;
}

// FUNCTION: VC1 0x00468130
bool KdSoundDriver::Start()
{
    return BASS_Start() != 0;
}

// IShutdownable vtable+0x0c: JMP BASS_Free.
// FUNCTION: VC1 0x00468140
int KdSoundDriver::Shutdown()
{
    return BASS_Free();
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x00468150
unsigned char KdSoundDriver::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// FUNCTION: VC1 0x004681a0
void* KdSoundDriver::QueryInterface(const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        if (!this) {
            return 0;
        }
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidSoundDriver) {
        return this;
    }
    if (name == g_iidShutdownable) {
        if (!this) {
            return 0;
        }
        return static_cast<IShutdownable*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x004681f0
unsigned KdSoundDriver::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x00468210
unsigned KdSoundDriver::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// RET 4. ResourceManager GetMusic: a new KdMusic's IAbstract face to *arg.
// FUNCTION: VC1 0x00468220
void* KdSoundDriver::Sound14(void* arg)
{
    CreateKdMusic(static_cast<void**>(arg));
    return arg;
}

// STUB: VC1 0x00468230
void* KdSoundDriver::Sound10(void* arg)
{
    Sound10Body();
    return arg;
}

// FUNCTION: VC1 0x00468280
void** CreateKdSoundDriver(void** out)
{
    IUnknown* queried;
    KdSoundDriver* object;

    if (g_kdSoundDriver) {
        queried = static_cast<IUnknown*>(
            g_kdSoundDriver->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    object = static_cast<KdSoundDriver*>(malloc(sizeof(KdSoundDriver)));
    if (!object) {
        g_kdSoundDriver = 0;
        *out = 0;
        return out;
    }
    object->KdSoundDriver::KdSoundDriver();
    g_kdSoundDriver = static_cast<IAbstract*>(object);
    queried = static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract));
    if (queried) {
        queried->AddRef();
    }
    KdRelease(queried);
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(queried);
    return out;
}

// STRING: VC1 0x004d5b18
static const char kBassVersion[] = "BASS v%d.%d.%d.%d";
// STRING: VC1 0x004d5b2c
static const char kBassInit[] = "BASS_Init: %d";
// STRING: VC1 0x004d5b3c
static const char kBassDevice[] = "Device: %s\n";

// RET 4; the argument is unused. Logs the version bytes high to low,
// BASS_Init(-1, 44100, 0, 0, 0), then the default device name.
// FUNCTION: VC1 0x00468360
bool KdSoundDriver::SetEnabled(void* mixer)
{
    BassDword ver;
    BassDeviceInfo info;
    const signed char* const bytes = reinterpret_cast<const signed char*>(&ver);

    (void)mixer;
    ver = BASS_GetVersion();
    HostLog(&g_kdNameSoundDriver, kBassVersion, bytes[3], bytes[2], bytes[1], bytes[0]);
    if (!BASS_Init(-1, 0xac44, 0, 0, 0)) {
        HostLogError(&g_kdNameSoundDriver, kBassInit, BASS_ErrorGetCode());
        return false;
    }
    BASS_GetDeviceInfo(BASS_GetDevice(), &info);
    HostLog(&g_kdNameSoundDriver, kBassDevice, info.name);
    return true;
}


// STRING: VC1 0x004d5ae8
static const char kScriptStart[] = "Start";
// Stop, Pause and SetVolume are also KdMusic / KdSound script names.
// GLOBAL: VC1 0x004d5af0
extern const char kScriptStop[] = "Stop";
// GLOBAL: VC1 0x004d5af8
extern const char kScriptPause[] = "Pause";
// STRING: VC1 0x004d5b00
static const char kScriptGetVolume[] = "GetVolume";
// GLOBAL: VC1 0x004d5b0c
extern const char kScriptSetVolume[] = "SetVolume";

// RET 8. IScriptObject vtable+0x0c. Method table 0x004ee4f8 (guard bit
// 0x004ee508, atexit 0x004cabb0).
// FUNCTION: VC1 0x00468610
unsigned char KdSoundDriver::Script0c(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptStart] = CreateScriptMethod<KdSoundDriver>(&KdSoundDriver::Start);
        methods[kScriptStop] = CreateScriptMethod<KdSoundDriver>(&KdSoundDriver::Stop);
        methods[kScriptPause] = CreateScriptMethod<KdSoundDriver>(&KdSoundDriver::Pause);
        methods[kScriptGetVolume] = CreateScriptMethod<KdSoundDriver>(&KdSoundDriver::GetVolume);
        methods[kScriptSetVolume] = CreateScriptMethod<KdSoundDriver>(&KdSoundDriver::SetVolume);
    }
    return methods.Find(name, out);
}

KdSoundDriver::~KdSoundDriver()
{
    BASS_Free();
    g_kdSoundDriver = 0;
}

}
