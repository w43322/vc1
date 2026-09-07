#ifndef VC1_KD_ISOUND_DRIVER_H
#define VC1_KD_ISOUND_DRIVER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ISoundDriver : IUnknown
{
    virtual int SetEnabled() = 0; // vtable+0x0c
};

typedef char Vc1ISoundDriverSizeCheck[sizeof(ISoundDriver) == 0x04 ? 1 : -1];

}

#endif
