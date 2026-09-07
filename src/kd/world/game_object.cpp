#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdGameObject : public IUnknown
{
};

// STRING: VC1 0x004cfdb0
static const char kGameObject[] = "GameObject";
// GLOBAL: VC1 0x004d0af0
const char* g_kdNameGameObject = kGameObject;

// STUB: VC1 0x004b0b70
void** CreateKdGameObject(void** out)
{
    (void)sizeof(KdGameObject);
    if (out) {
        *out = 0;
    }
    return out;
}

}
