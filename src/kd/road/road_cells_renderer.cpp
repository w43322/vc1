#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdRoadCellsRenderer : public IUnknown
{
};

// STRING: VC1 0x004cfcac
static const char kRoadCellsRenderer[] = "RoadCellsRenderer";
// GLOBAL: VC1 0x004d0b34
const char* g_kdNameRoadCellsRenderer = kRoadCellsRenderer;

// STUB: VC1 0x004ac800
void** CreateKdRoadCellsRenderer(void** out)
{
    (void)sizeof(KdRoadCellsRenderer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
