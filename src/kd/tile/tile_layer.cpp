#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTileLayer : public IUnknown
{
};

// STRING: VC1 0x004cfd90
static const char kTileLayer[] = "TileLayer";
// GLOBAL: VC1 0x004d0af8
const char* g_kdNameTileLayer = kTileLayer;

// STUB: VC1 0x004a35a0
void** CreateKdTileLayer(void** out)
{
    (void)sizeof(KdTileLayer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
