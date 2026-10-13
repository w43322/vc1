#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iresource_manager.h"

namespace vc1 {

class KdResourceManager : public IResourceManager
{
};

// STRING: VC1 0x004d4ba0
static const char kResourceManager[] = "ResourceManager";
// GLOBAL: VC1 0x004d0670
const char* g_kdNameResourceManager = kResourceManager;

// EDI=out. Factory "ResourceManager" as IResourceManager (AddRef'd), or null.
// FUNCTION: VC1 0x00418010
IResourceManager** GetKdResourceManager(IResourceManager** out)
{
    void* unknown;
    IResourceManager* queried;

    GetKdFactory()->QueryNamed(&unknown, &g_kdNameResourceManager);
    if (!unknown) {
        *out = 0;
        return out;
    }
    queried = static_cast<IResourceManager*>(
        static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidResourceManager));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
    return out;
}

// STUB: VC1 0x0046d7a0
void** CreateKdResourceManager(void** out)
{
    (void)sizeof(KdResourceManager*);
    if (out) {
        *out = 0;
    }
    return out;
}

}
