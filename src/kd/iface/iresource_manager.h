#ifndef VC1_KD_IRESOURCE_MANAGER_H
#define VC1_KD_IRESOURCE_MANAGER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

class KdString;

// Primary face of the resource manager (vftable 0x004d55dc).
// SIZE 0x04
struct IResourceManager : IUnknown
{
    virtual void** Resource0c(void** out, const KdString* name) = 0; // vtable+0x0c
    // 0x0046e060: cached lookup, else creates a "Font" through the factory.
    virtual void** LoadFont(void** out, const KdString* name) = 0;   // vtable+0x10
};

typedef char Vc1IResourceManagerSizeCheck[sizeof(IResourceManager) == 0x04 ? 1 : -1];

IResourceManager** GetKdResourceManager(IResourceManager** out);

}

#endif
