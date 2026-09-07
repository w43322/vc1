#include "kd/iface/ishutdownable.h"

namespace vc1 {

void* Shutdownable::QueryInterface(const char** iid)
{
    (void)iid;
    return 0;
}

unsigned Shutdownable::AddRef()
{
    return 0;
}

unsigned Shutdownable::Release()
{
    return 0;
}

int Shutdownable::Shutdown()
{
    return 0;
}

}
