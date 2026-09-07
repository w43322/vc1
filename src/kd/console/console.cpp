#include "kd/factory.h"
#include "kd/iface/iunknown.h"

namespace vc1 {

class KdConsole : public IUnknown
{
};

// STRING: VC1 0x004d4bdc
static const char kConsole[] = "Console";
// GLOBAL: VC1 0x004d065c
const char* g_kdNameConsole = kConsole;

// STUB: VC1 0x0046bf20
void** CreateKdConsole(void** out)
{
    (void)sizeof(KdConsole);
    if (out) {
        *out = 0;
    }
    return out;
}

}
