#include "kd/scene/compound_object.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"

#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004d4b90
static const char kCompoundObject[] = "CompoundObject";
// GLOBAL: VC1 0x004d0674
const char* g_kdNameCompoundObject = kCompoundObject;

KdCompoundObject::KdCompoundObject()
{
    memset(pad, 0, sizeof(pad));
    memset(tail, 0, sizeof(tail));
}

void* KdCompoundObject::QueryInterface(const char** iid)
{
    if (iid && *iid == g_iidAbstract) {
        return &abstract;
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

// FUNCTION: VC1 0x00473800
KdCompoundObject* ConstructKdCompoundObject(KdCompoundObject* self)
{
    UnmappedConstructCompoundBase(self);
    self->KdCompoundObject::KdCompoundObject();
    return self;
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
    ConstructKdCompoundObject(memory);
    queried = static_cast<IUnknown*>(memory->abstract.QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

}
