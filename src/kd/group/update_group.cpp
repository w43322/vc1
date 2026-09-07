#include "kd/factory.h"
#include "kd/iface/ifunctional_group.h"

namespace vc1 {

class KdUpdateGroup : public IFunctionalGroup
{
};

// STRING: VC1 0x004d4bc8
static const char kUpdateGroup[] = "UpdateGroup";
// GLOBAL: VC1 0x004d0664
const char* g_kdNameUpdateGroup = kUpdateGroup;

// STUB: VC1 0x004700e0
void** CreateKdUpdateGroup(void** out)
{
    (void)sizeof(KdUpdateGroup);
    if (out) {
        *out = 0;
    }
    return out;
}

}
