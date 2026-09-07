#ifndef VC1_KD_SOUND_DRIVER_H
#define VC1_KD_SOUND_DRIVER_H

#include "kd/iface/isound_driver.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

extern IAbstract* g_kdSoundDriver;

// VTABLE: VC1 0x004d5b48
// SIZE 0x14
class KdSoundDriver : public ISoundDriver
{
public:
    Shutdownable shutdownable; // 0x04
    ScriptObject scriptObject; // 0x08
    Abstract abstract;         // 0x0c
    unsigned refs;             // 0x10

    KdSoundDriver();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int SetEnabled();                       // vtable+0x0c
};

typedef char Vc1KdSoundDriverSizeCheck[sizeof(KdSoundDriver) == 0x14 ? 1 : -1];

void** CreateKdSoundDriver(void** out);

}

#endif
