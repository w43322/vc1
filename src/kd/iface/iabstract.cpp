#include "kd/iface/iabstract.h"
#include "kd/iface/iid.h"

namespace vc1 {

void* Abstract::QueryInterface(const char** iid)
{
    if (iid && *iid == g_iidAbstract) {
        return this;
    }
    return 0;
}

unsigned Abstract::AddRef()
{
    return 0;
}

unsigned Abstract::Release()
{
    return 0;
}

int Abstract::Unmapped0c()
{
    return 0;
}

}
