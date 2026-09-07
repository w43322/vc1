#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdGridRectRenderer : public IUnknown
{
};

// STRING: VC1 0x004cfd04
static const char kGridRectRenderer[] = "GridRectRenderer";
// GLOBAL: VC1 0x004d0b20
const char* g_kdNameGridRectRenderer = kGridRectRenderer;

// STUB: VC1 0x004b0020
void** CreateKdGridRectRenderer(void** out)
{
    (void)sizeof(KdGridRectRenderer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
