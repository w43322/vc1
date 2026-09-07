#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdPassMap : public IUnknown
{
};

// STRING: VC1 0x004cfd7c
static const char kPassMap[] = "PassMap";
// GLOBAL: VC1 0x004d0b00
const char* g_kdNamePassMap = kPassMap;

// STUB: VC1 0x004ad640
void** CreateKdPassMap(void** out)
{
    (void)sizeof(KdPassMap);
    if (out) {
        *out = 0;
    }
    return out;
}

}
