#ifndef VC1_KD_IRENDERABLE_H
#define VC1_KD_IRENDERABLE_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IRenderable : IUnknown
{
};

typedef char Vc1IRenderableSizeCheck[sizeof(IRenderable) == 0x04 ? 1 : -1];

}

#endif
