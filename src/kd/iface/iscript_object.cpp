#include "kd/iface/iscript_object.h"

namespace vc1 {

void* ScriptObject::QueryInterface(const char** iid)
{
    (void)iid;
    return 0;
}

unsigned ScriptObject::AddRef()
{
    return 0;
}

unsigned ScriptObject::Release()
{
    return 0;
}

}
