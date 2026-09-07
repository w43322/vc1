#ifndef VC1_KD_IRESOURCE_MANAGER_H
#define VC1_KD_IRESOURCE_MANAGER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IResourceManager : IUnknown
{
};

typedef char Vc1IResourceManagerSizeCheck[sizeof(IResourceManager) == 0x04 ? 1 : -1];

}

#endif
