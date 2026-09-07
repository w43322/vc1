#ifndef VC1_KD_SOUND_DRIVER_H
#define VC1_KD_SOUND_DRIVER_H

#include "kd/iface/isound_driver.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

extern IAbstract* g_kdSoundDriver;

// Extra original vftables: 0x004d5b74, 0x004d5b84, 0x004d5b9c.
// VTABLE: VC1 0x004d5b48
// SIZE 0x14
class KdSoundDriver : public ISoundDriver,
                      public IShutdownable,
                      public IScriptObject,
                      public IAbstract
{
public:
    unsigned refs; // 0x10

    KdSoundDriver();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int SetEnabled();                       // vtable+0x0c
    int Shutdown();                         // vtable+0x0c
    int Unmapped0c();                       // vtable+0x0c
};

typedef char Vc1KdSoundDriverSizeCheck[sizeof(KdSoundDriver) == 0x14 ? 1 : -1];

void** CreateKdSoundDriver(void** out);

}

#endif
