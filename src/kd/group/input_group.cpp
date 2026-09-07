#include "kd/factory.h"
#include "kd/iface/ifunctional_group.h"

namespace vc1 {

class KdInputGroup : public IFunctionalGroup
{
};

// STRING: VC1 0x004d4bb0
static const char kInputGroup[] = "InputGroup";
// GLOBAL: VC1 0x004d066c
const char* g_kdNameInputGroup = kInputGroup;

// STUB: VC1 0x004b6cd0
void** CreateKdInputGroup(void** out)
{
    (void)sizeof(KdInputGroup);
    if (out) {
        *out = 0;
    }
    return out;
}

}
