#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTileObject : public IUnknown
{
};

// STRING: VC1 0x004cfd84
static const char kTileObject[] = "TileObject";
// GLOBAL: VC1 0x004d0afc
const char* g_kdNameTileObject = kTileObject;

// STUB: VC1 0x004a0d00
void** CreateKdTileObject(void** out)
{
    (void)sizeof(KdTileObject);
    if (out) {
        *out = 0;
    }
    return out;
}

}
