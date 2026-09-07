#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdRoute : public IUnknown
{
};

// STRING: VC1 0x004cfd28
static const char kRoute[] = "Route";
// GLOBAL: VC1 0x004d0b18
const char* g_kdNameRoute = kRoute;

// STUB: VC1 0x004a9800
void** CreateKdRoute(void** out)
{
    (void)sizeof(KdRoute);
    if (out) {
        *out = 0;
    }
    return out;
}

}
