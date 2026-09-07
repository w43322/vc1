#include "kd/sound/sound_driver.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed548
IAbstract* g_kdSoundDriver;

// STRING: VC1 0x004d4bfc
static const char kSoundDriver[] = "SoundDriver";
// GLOBAL: VC1 0x004d064c
const char* g_kdNameSoundDriver = kSoundDriver;

KdSoundDriver::KdSoundDriver()
{
    refs = 1;
}

// FUNCTION: VC1 0x004681a0
void* KdSoundDriver::QueryInterface(const char** iid)
{
    const char* name;

    if (!iid) {
        return 0;
    }
    name = *iid;
    if (name == g_iidScriptObject) {
        return &scriptObject;
    }
    if (name == g_iidSoundDriver) {
        return this;
    }
    if (name == g_iidShutdownable) {
        return &shutdownable;
    }
    if (name == g_iidAbstract) {
        return &abstract;
    }
    return 0;
}

// STUB: VC1 0x004681f0
unsigned KdSoundDriver::Release()
{
    if (refs) {
        refs -= 1;
    }
    return refs;
}

// STUB: VC1 0x00468210
unsigned KdSoundDriver::AddRef()
{
    refs += 1;
    return refs;
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
    g_kdSoundDriver = &object->abstract;
    queried = static_cast<IUnknown*>(object->abstract.QueryInterface(&g_iidAbstract));
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

// STUB: VC1 0x00468360
int KdSoundDriver::SetEnabled()
{
    return 1;
}

}
