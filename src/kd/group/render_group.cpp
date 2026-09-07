#include "kd/factory.h"
#include "kd/iface/ifunctional_group.h"

namespace vc1 {

class KdRenderGroup : public IFunctionalGroup
{
};

// STRING: VC1 0x004d4bbc
static const char kRenderGroup[] = "RenderGroup";
// GLOBAL: VC1 0x004d0668
const char* g_kdNameRenderGroup = kRenderGroup;

// STUB: VC1 0x0046d4b0
void** CreateKdRenderGroup(void** out)
{
    (void)sizeof(KdRenderGroup);
    if (out) {
        *out = 0;
    }
    return out;
}

}
