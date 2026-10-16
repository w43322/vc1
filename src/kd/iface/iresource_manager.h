#ifndef VC1_KD_IRESOURCE_MANAGER_H
#define VC1_KD_IRESOURCE_MANAGER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

class KdString;

// Primary face of the resource manager (vftable 0x004d55dc). Slot names are
// the script methods bound to them (KdResourceManager::Script0c, 0x0046db00).
// Each getter returns the loaded resource registered under name, loading it
// on first use.
// SIZE 0x04
struct IResourceManager : IUnknown
{
    virtual KdPtr<IUnknown> GetTexture(const KdString& name) = 0; // vtable+0x0c
    virtual KdPtr<IUnknown> GetFont(const KdString& name) = 0;    // vtable+0x10
    virtual KdPtr<IUnknown> GetEffect(const KdString& name) = 0;  // vtable+0x14
    virtual KdPtr<IUnknown> GetSound(const KdString& name) = 0;   // vtable+0x18
    virtual KdPtr<IUnknown> GetMusic(const KdString& name) = 0;   // vtable+0x1c
    virtual void FreeResource(const KdString& name) = 0;          // vtable+0x20
    virtual void FreeUnusedResources() = 0;                       // vtable+0x24
};

typedef char Vc1IResourceManagerSizeCheck[sizeof(IResourceManager) == 0x04 ? 1 : -1];

IResourceManager** GetKdResourceManager(IResourceManager** out);

}

#endif
