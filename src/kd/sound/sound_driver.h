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
    ~KdSoundDriver();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int SetEnabled();                       // vtable+0x0c
    void* Sound10(void* arg);               // vtable+0x10
    void* Sound14(void* arg);               // vtable+0x14
    int Shutdown();                         // vtable+0x0c
    unsigned char Script0c(void* a, void* b);
    unsigned char Script10(void* a, void* b);
    int Script14();
};

typedef char Vc1KdSoundDriverSizeCheck[sizeof(KdSoundDriver) == 0x14 ? 1 : -1];

// FUNCTION: VC1 0x00419800
// vc1::KdSoundDriver::`scalar deleting destructor'

void** CreateKdSoundDriver(void** out);

}

#endif
