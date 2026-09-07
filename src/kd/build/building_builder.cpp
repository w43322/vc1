#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdBuildingBuilder : public IUnknown
{
};

// STRING: VC1 0x004cfd30
static const char kBuildingBuilder[] = "BuildingBuilder";
// GLOBAL: VC1 0x004d0b14
const char* g_kdNameBuildingBuilder = kBuildingBuilder;

// STUB: VC1 0x004b3650
void** CreateKdBuildingBuilder(void** out)
{
    (void)sizeof(KdBuildingBuilder);
    if (out) {
        *out = 0;
    }
    return out;
}

}
