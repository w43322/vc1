#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdRoadRenderer : public IUnknown
{
};

// STRING: VC1 0x004cfd50
static const char kRoadRenderer[] = "RoadRenderer";
// GLOBAL: VC1 0x004d0b0c
const char* g_kdNameRoadRenderer = kRoadRenderer;

// STUB: VC1 0x004ab750
void** CreateKdRoadRenderer(void** out)
{
    (void)sizeof(KdRoadRenderer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
