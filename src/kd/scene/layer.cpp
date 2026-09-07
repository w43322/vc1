#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdLayer : public IUnknown
{
};

// STRING: VC1 0x004d4c20
static const char kLayer[] = "Layer";
// GLOBAL: VC1 0x004d0640
const char* g_kdNameLayer = kLayer;

// STUB: VC1 0x00468ce0
void** CreateKdLayer(void** out)
{
    (void)sizeof(KdLayer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
