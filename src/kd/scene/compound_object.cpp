#include "kd/scene/compound_object.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "host/runtime.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4b90
static const char kCompoundObject[] = "CompoundObject";
// GLOBAL: VC1 0x004d0674
const char* g_kdNameCompoundObject = kCompoundObject;

// FUNCTION: VC1 0x00473800
KdCompoundObject::KdCompoundObject()
{
    AssignCountedString(this, 0, 0);
    memset(data, 0, sizeof(data));
    memset(tail, 0, sizeof(tail));
}

void* KdCompoundObject::QueryInterface(const char** iid)
{
    if (iid && *iid == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

unsigned KdCompoundObject::AddRef()
{
    return 1;
}

unsigned KdCompoundObject::Release()
{
    return 1;
}

// FUNCTION: VC1 0x004738e0
void** CreateKdCompoundObject(void** out)
{
    KdCompoundObject* memory = static_cast<KdCompoundObject*>(malloc(sizeof(KdCompoundObject)));
    IUnknown* queried;

    if (!memory) {
        *out = 0;
        return out;
    }
    new (memory) KdCompoundObject();
    queried = static_cast<IUnknown*>(memory->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

}
