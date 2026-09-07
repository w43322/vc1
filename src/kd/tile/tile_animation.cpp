#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTileAnimation : public IUnknown
{
};

// STRING: VC1 0x004cfd40
static const char kTileAnimation[] = "TileAnimation";
// GLOBAL: VC1 0x004d0b10
const char* g_kdNameTileAnimation = kTileAnimation;

// STUB: VC1 0x004a4fb0
void** CreateKdTileAnimation(void** out)
{
    (void)sizeof(KdTileAnimation);
    if (out) {
        *out = 0;
    }
    return out;
}

}
