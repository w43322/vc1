#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdClassificatedRenderer : public IUnknown
{
};

// STRING: VC1 0x004cfcdc
static const char kClassificatedRenderer[] = "ClassificatedRenderer";
// GLOBAL: VC1 0x004d0b28
const char* g_kdNameClassificatedRenderer = kClassificatedRenderer;

// STUB: VC1 0x004b2370
void** CreateKdClassificatedRenderer(void** out)
{
    (void)sizeof(KdClassificatedRenderer);
    if (out) {
        *out = 0;
    }
    return out;
}

}
