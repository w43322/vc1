#ifndef VC1_KD_ISOUND_DRIVER_H
#define VC1_KD_ISOUND_DRIVER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ISoundDriver : IUnknown
{
    // RunApp passes IDisplay vtable+0x2c; KdSoundDriver ignores it.
    virtual bool SetEnabled(void* mixer) = 0; // vtable+0x0c
    virtual void* Sound10(void* arg) = 0;  // vtable+0x10
    virtual void* Sound14(void* arg) = 0;  // vtable+0x14
    // Script "Start" / "Stop" / "Pause" / "GetVolume" / "SetVolume"
    // (KdSoundDriver::Script0c, 0x00468610).
    virtual bool Start() = 0;              // vtable+0x18
    virtual bool Stop() = 0;               // vtable+0x1c
    virtual bool Pause() = 0;              // vtable+0x20
    virtual int GetVolume() = 0;           // vtable+0x24
    virtual bool SetVolume(int volume) = 0; // vtable+0x28
};

typedef char Vc1ISoundDriverSizeCheck[sizeof(ISoundDriver) == 0x04 ? 1 : -1];

}

#endif
