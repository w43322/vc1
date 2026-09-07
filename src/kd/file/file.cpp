#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdFile : public IUnknown
{
};

// STRING: VC1 0x004d4b88
static const char kFile[] = "File";
// GLOBAL: VC1 0x004d0678
const char* g_kdNameFile = kFile;

// STUB: VC1 0x0046a8c0
void** CreateKdFile(void** out)
{
    (void)sizeof(KdFile);
    if (out) {
        *out = 0;
    }
    return out;
}

}
