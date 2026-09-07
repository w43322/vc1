#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdGroupRenderer : public IUnknown
{
};

// STRING: VC1 0x004cfcc0
static const char kGroupRenderer[] = "GroupRenderer";
// GLOBAL: VC1 0x004d0b30
const char* g_kdNameGroupRenderer = kGroupRenderer;

// STUB: VC1 0x004aebd0
void** CreateKdGroupRenderer(void** out)
{
    (void)sizeof(KdGroupRenderer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
