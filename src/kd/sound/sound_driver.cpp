#include "kd/sound/sound_driver.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"

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

// STUB: VC1 0x0044467a
static void BassFree()
{
}

// STUB: VC1 0x00444680
static unsigned BassGetVersion()
{
    return 0;
}

// STUB: VC1 0x00444686
static int BassInit(int device, unsigned freq, unsigned flags, void* win, void* clsid)
{
    (void)device;
    (void)freq;
    (void)flags;
    (void)win;
    (void)clsid;
    return 0;
}

// STUB: VC1 0x0044468c
static int BassErrorGetCode()
{
    return 0;
}

// STUB: VC1 0x00444692
static int BassGetDeviceInfo(unsigned device, void* info)
{
    (void)device;
    (void)info;
    return 0;
}

// STUB: VC1 0x00444698
static int BassGetDevice()
{
    return 0;
}

// STUB: VC1 0x0044469e
static int BassStart()
{
    return 0;
}

// STUB: VC1 0x004446a4
static int BassPause()
{
    return 0;
}

// STUB: VC1 0x004446aa
static int BassStop()
{
    return 0;
}

// STUB: VC1 0x004446b0
static float BassGetCPU()
{
    return 0.0f;
}

// STUB: VC1 0x0045e9b0
static void Sound14Body()
{
}

// STUB: VC1 0x0045f960
static void Sound10Body()
{
}

KdSoundDriver::KdSoundDriver()
{
}

// CALL IAT; NEG/SBB/NEG EAX. ISoundDriver vtable+0x24 then JMP CRT.
// STUB: VC1 0x00468100
float KdSoundDriver::Sound24()
{
    return BassGetCPU();
}

// CALL IAT; NEG/SBB/NEG EAX.
// STUB: VC1 0x00468110
unsigned char KdSoundDriver::Sound20()
{
    return BassStop() != 0;
}

// STUB: VC1 0x00468120
unsigned char KdSoundDriver::Sound1c()
{
    return BassPause() != 0;
}

// STUB: VC1 0x00468130
unsigned char KdSoundDriver::Sound18()
{
    return BassStart() != 0;
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

// STUB: VC1 0x00468220
void* KdSoundDriver::Sound14(void* arg)
{
    Sound14Body();
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

// STUB: VC1 0x00468360
int KdSoundDriver::SetEnabled()
{
    unsigned ver;
    int device;
    char* name;

    ver = BassGetVersion();
    HostLog(&g_kdNameSoundDriver, kBassVersion,
        static_cast<int>(static_cast<signed char>(ver >> 24)),
        static_cast<int>(static_cast<signed char>(ver >> 16)),
        static_cast<int>(static_cast<signed char>(ver >> 8)),
        static_cast<int>(static_cast<signed char>(ver)));
    if (!BassInit(-1, 0xac44, 0, 0, 0)) {
        HostLogError(&g_kdNameSoundDriver, kBassInit, BassErrorGetCode());
        return 0;
    }
    device = BassGetDevice();
    BassGetDeviceInfo(static_cast<unsigned>(device), &name);
    HostLog(&g_kdNameSoundDriver, kBassDevice, name);
    return 1;
}

int KdSoundDriver::Shutdown()
{
    return 0;
}

unsigned char KdSoundDriver::Script0c(void*, void*) { return 0; }

KdSoundDriver::~KdSoundDriver()
{
    BassFree();
    g_kdSoundDriver = 0;
}

}
