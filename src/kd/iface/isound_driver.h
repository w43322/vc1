#ifndef VC1_KD_ISOUND_DRIVER_H
#define VC1_KD_ISOUND_DRIVER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ISoundDriver : IUnknown
{
    virtual int SetEnabled() = 0;          // vtable+0x0c
    virtual void* Sound10(void* arg) = 0;  // vtable+0x10
    virtual void* Sound14(void* arg) = 0;  // vtable+0x14
    virtual unsigned char Sound18() = 0;   // vtable+0x18
    virtual unsigned char Sound1c() = 0;   // vtable+0x1c
    virtual unsigned char Sound20() = 0;   // vtable+0x20
    virtual float Sound24() = 0;          // vtable+0x24
};

typedef char Vc1ISoundDriverSizeCheck[sizeof(ISoundDriver) == 0x04 ? 1 : -1];

}

#endif
