#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTransportMover : public IUnknown
{
};

// STRING: VC1 0x004cfd18
static const char kTransportMover[] = "TransportMover";
// GLOBAL: VC1 0x004d0b1c
const char* g_kdNameTransportMover = kTransportMover;

// STUB: VC1 0x0049d010
void** CreateKdTransportMover(void** out)
{
    (void)sizeof(KdTransportMover);
    if (out) {
        *out = 0;
    }
    return out;
}

}
