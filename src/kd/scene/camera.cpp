#include "kd/factory.h"
#include "kd/iface/icamera.h"

namespace vc1 {

class KdCamera : public ICamera
{
};

// Also the scene object name GameLevel "GetObjectAt" looks up.
// GLOBAL: VC1 0x004d0dd8
extern const char kCameraName[] = "Camera";
// GLOBAL: VC1 0x004d0684
const char* g_kdNameCamera = kCameraName;

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
