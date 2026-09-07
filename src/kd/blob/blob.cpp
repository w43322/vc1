#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdBlob : public IUnknown
{
};

// STRING: VC1 0x004d4b78
static const char kBlob[] = "Blob";
// GLOBAL: VC1 0x004d0680
const char* g_kdNameBlob = kBlob;

// STUB: VC1 0x0046cd80
void** CreateKdBlob(void** out)
{
    (void)sizeof(KdBlob);
    if (out) {
        *out = 0;
    }
    return out;
}

}
