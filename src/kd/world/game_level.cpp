#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdGameLevel : public IUnknown
{
};

// STRING: VC1 0x004cfdbc
static const char kGameLevel[] = "GameLevel";
// GLOBAL: VC1 0x004d0aec
const char* g_kdNameGameLevel = kGameLevel;

// STUB: VC1 0x004b1600
void** CreateKdGameLevel(void** out)
{
    (void)sizeof(KdGameLevel);
    if (out) {
        *out = 0;
    }
    return out;
}

}
