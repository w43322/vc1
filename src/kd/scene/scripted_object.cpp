#include "kd/factory.h"
#include "kd/iface/iscript_object.h"

namespace vc1 {

class KdScriptedObject : public IScriptObject
{
};

// STRING: VC1 0x004d4b68
static const char kScriptedObject[] = "ScriptedObject";
// GLOBAL: VC1 0x004d0688
const char* g_kdNameScriptedObject = kScriptedObject;

// STUB: VC1 0x00471430
void** CreateKdScriptedObject(void** out)
{
    (void)sizeof(KdScriptedObject);
    if (out) {
        *out = 0;
    }
    return out;
}

}
