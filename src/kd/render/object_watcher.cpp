#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdObjectWatcher : public IUnknown
{
};

// STRING: VC1 0x004cfcf4
static const char kObjectWatcher[] = "ObjectWatcher";
// GLOBAL: VC1 0x004d0b24
const char* g_kdNameObjectWatcher = kObjectWatcher;

// STUB: VC1 0x004ae250
void** CreateKdObjectWatcher(void** out)
{
    (void)sizeof(KdObjectWatcher);
    if (out) {
        *out = 0;
    }
    return out;
}

}
