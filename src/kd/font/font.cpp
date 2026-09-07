#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdFont : public IUnknown
{
};

// STRING: VC1 0x004d4bd4
static const char kFont[] = "Font";
// GLOBAL: VC1 0x004d0660
const char* g_kdNameFont = kFont;

// STUB: VC1 0x00466f50
void** CreateKdFont(void** out)
{
    (void)sizeof(KdFont);
    if (out) {
        *out = 0;
    }
    return out;
}

}
