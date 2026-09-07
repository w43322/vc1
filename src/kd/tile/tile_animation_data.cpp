#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTileAnimationData : public IUnknown
{
};

// STRING: VC1 0x004cfd9c
static const char kTileAnimationData[] = "TileAnimationData";
// GLOBAL: VC1 0x004d0af4
const char* g_kdNameTileAnimationData = kTileAnimationData;

// STUB: VC1 0x004a4660
void** CreateKdTileAnimationData(void** out)
{
    (void)sizeof(KdTileAnimationData);
    if (out) {
        *out = 0;
    }
    return out;
}

}
