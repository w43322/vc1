#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTileManager : public IUnknown
{
};

// STRING: VC1 0x004d4b5c
static const char kTileManager[] = "TileManager";
// GLOBAL: VC1 0x004d068c
const char* g_kdNameTileManager = kTileManager;

// STUB: VC1 0x004a1aa0
void** CreateKdTileManager(void** out)
{
    (void)sizeof(KdTileManager);
    if (out) {
        *out = 0;
    }
    return out;
}

}
