#include "kd/iface/iunknown.h"

namespace vc1 {

void* KdQueryInterface(IUnknown* object, const char** iid)
{
    if (!object) {
        return 0;
    }
    return object->QueryInterface(iid);
}

void KdAddRef(IUnknown* object)
{
    if (object) {
        object->AddRef();
    }
}

void KdRelease(IUnknown* object)
{
    if (object) {
        object->Release();
    }
}

}
