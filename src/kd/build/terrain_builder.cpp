#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTerrainBuilder : public IUnknown
{
};

// STRING: VC1 0x004cfd60
static const char kTerrainBuilder[] = "TerrainBuilder";
// GLOBAL: VC1 0x004d0b08
const char* g_kdNameTerrainBuilder = kTerrainBuilder;

// STUB: VC1 0x004a7280
void** CreateKdTerrainBuilder(void** out)
{
    (void)sizeof(KdTerrainBuilder);
    if (out) {
        *out = 0;
    }
    return out;
}

}
