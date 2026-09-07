#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdScene : public IUnknown
{
};

// STRING: VC1 0x004d4b80
static const char kScene[] = "Scene";
// GLOBAL: VC1 0x004d067c
const char* g_kdNameScene = kScene;

// STUB: VC1 0x0046f270
void** CreateKdScene(void** out)
{
    (void)sizeof(KdScene);
    if (out) {
        *out = 0;
    }
    return out;
}

}
