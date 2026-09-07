#ifndef VC1_KD_ICAMERA_H
#define VC1_KD_ICAMERA_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct ICamera : IUnknown
{
};

typedef char Vc1ICameraSizeCheck[sizeof(ICamera) == 0x04 ? 1 : -1];

}

#endif
