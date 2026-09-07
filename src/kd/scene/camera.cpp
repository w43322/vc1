#include "kd/factory.h"
#include "kd/iface/icamera.h"

namespace vc1 {

class KdCamera : public ICamera
{
};

// STRING: VC1 0x004d0dd8
static const char kCamera[] = "Camera";
// GLOBAL: VC1 0x004d0684
const char* g_kdNameCamera = kCamera;

// STUB: VC1 0x004707c0
void** CreateKdCamera(void** out)
{
    (void)sizeof(KdCamera);
    if (out) {
        *out = 0;
    }
    return out;
}

}
