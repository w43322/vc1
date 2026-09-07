#include "kd/factory.h"
#include "kd/iface/iresource_manager.h"

namespace vc1 {

class KdResourceManager : public IResourceManager
{
};

// STRING: VC1 0x004d4ba0
static const char kResourceManager[] = "ResourceManager";
// GLOBAL: VC1 0x004d0670
const char* g_kdNameResourceManager = kResourceManager;

// STUB: VC1 0x0046d7a0
void** CreateKdResourceManager(void** out)
{
    (void)sizeof(KdResourceManager);
    if (out) {
        *out = 0;
    }
    return out;
}

}
