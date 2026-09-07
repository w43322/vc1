#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdTSPSolver : public IUnknown
{
};

// STRING: VC1 0x004cfcd0
static const char kTspSolver[] = "TSPSolver";
// GLOBAL: VC1 0x004d0b2c
const char* g_kdNameTspSolver = kTspSolver;

// STUB: VC1 0x0049b740
void** CreateKdTspSolver(void** out)
{
    (void)sizeof(KdTSPSolver);
    if (out) {
        *out = 0;
    }
    return out;
}

}
