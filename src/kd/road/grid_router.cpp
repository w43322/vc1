#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdGridRouter : public IUnknown
{
};

// STRING: VC1 0x004cfd70
static const char kGridRouter[] = "GridRouter";
// GLOBAL: VC1 0x004d0b04
const char* g_kdNameGridRouter = kGridRouter;

// STUB: VC1 0x004af300
void** CreateKdGridRouter(void** out)
{
    (void)sizeof(KdGridRouter);
    if (out) {
        *out = 0;
    }
    return out;
}

}
