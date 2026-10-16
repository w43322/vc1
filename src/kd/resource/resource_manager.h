#ifndef VC1_KD_RESOURCE_MANAGER_H
#define VC1_KD_RESOURCE_MANAGER_H

#include "kd/iface/iabstract.h"
#include "kd/iface/iresource_manager.h"
#include "kd/iface/iscript_object.h"
#include "kd/component_group.h"

namespace vc1 {

// Inferred name. The singleton's IAbstract face (CreateKdResourceManager).
extern IAbstract* g_kdResourceManager;

// Factory "ResourceManager" (singleton). Loaded resources are children of the
// component group, keyed by file name; a getter loads (ILoadable::Load) and
// registers on a miss.
// SIZE 0x34
// VTABLE: VC1 0x004d55dc vc1::IResourceManager
// VTABLE: VC1 0x004d5604 vc1::IScriptObject
// VTABLE: VC1 0x004d561c vc1::KdComponentGroup
// VTABLE: VC1 0x004d5644 vc1::IAbstract
class KdResourceManager : public IResourceManager,
                          public IScriptObject,
                          public KdComponentGroup,
                          public IAbstract
{
public:
    KdResourceManager() throw();
    ~KdResourceManager() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    KdPtr<IUnknown> GetTexture(const KdString& name);
    KdPtr<IUnknown> GetFont(const KdString& name);
    KdPtr<IUnknown> GetEffect(const KdString& name);
    KdPtr<IUnknown> GetSound(const KdString& name);
    KdPtr<IUnknown> GetMusic(const KdString& name);
    void FreeResource(const KdString& name);
    void FreeUnusedResources();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
};

typedef char Vc1KdResourceManagerSizeCheck[sizeof(KdResourceManager) == 0x34 ? 1 : -1];

// SYNTHETIC: VC1 0x00417920
// vc1::KdResourceManager::`scalar deleting destructor'

}

#endif
