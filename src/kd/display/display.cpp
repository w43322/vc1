#include "kd/display/display.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "host/runtime.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed54c
IAbstract* g_kdDisplay;

// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;

KdDisplay::KdDisplay()
{
    pad08 = 0;
    pad0c = 0;
    pad20[0] = 0;
    pad20[1] = 0;
    pad20[2] = 0;
    pad20[3] = 0;
    zero30 = 0.0f;
    one34 = 1.0f;
    zero38 = 0.0f;
    zero3c = 0.0f;
    zero40 = 0.0f;
    one44 = 1.0f;
    zero48 = 0.0f;
    zero4c = 0.0f;
    zero50 = 0.0f;
    one54 = 1.0f;
    enabled58 = 1;
}

int KdDisplay::Unmapped14() { return 0; }

// STUB: VC1 0x004022e0
int KdDisplay::SetSize(unsigned, unsigned)
{
    return 1;
}

// STUB: VC1 0x004023e0
int KdDisplay::Unmapped24() { return 0; }

// STUB: VC1 0x004023f0
int KdDisplay::Unmapped1c() { return 0; }

// STUB: VC1 0x00466290
int KdDisplay::Unmapped34() { return 0; }

// STUB: VC1 0x004662a0
int KdDisplay::Unmapped30() { return 0; }

// STUB: VC1 0x004662e0
int KdDisplay::Unmapped38() { return 0; }

// STUB: VC1 0x004b1c90
int KdDisplay::Unmapped18() { return 0; }

// STUB: VC1 0x004b1d30
int KdDisplay::Unmapped10() { return 0; }

// FUNCTION: VC1 0x004b1d40
void* KdDisplay::QueryInterface(const char** iid)
{
    void* const base = UnmappedDisplayBaseQueryInterface(this, iid);
    if (base) {
        return base;
    }
    if (iid && *iid == g_iidAbstract) {
        return &abstract;
    }
    if (iid && *iid == g_iidDisplay) {
        return this;
    }
    return 0;
}

// STUB: VC1 0x004b1d70
unsigned KdDisplay::Release()
{
    return 1;
}

// STUB: VC1 0x004b1d90
unsigned KdDisplay::AddRef()
{
    return 1;
}

// EAX=this. Calls FUN_004665c0 with this in ESI, then overwrites vptrs/floats.
// FUNCTION: VC1 0x004b1e40
KdDisplay* ConstructKdDisplay(KdDisplay* self)
{
    UnmappedConstructDisplayBase(self);
    self->KdDisplay::KdDisplay();
    return self;
}

// FUNCTION: VC1 0x004b1ea0
void** CreateKdDisplay(void** out)
{
    IUnknown* queried;
    KdDisplay* object;

    if (g_kdDisplay) {
        queried = static_cast<IUnknown*>(
            g_kdDisplay->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    object = static_cast<KdDisplay*>(malloc(sizeof(KdDisplay)));
    if (!object) {
        g_kdDisplay = 0;
        *out = 0;
        return out;
    }
    ConstructKdDisplay(object);
    g_kdDisplay = &object->abstract;
    queried = static_cast<IUnknown*>(object->abstract.QueryInterface(&g_iidAbstract));
    if (queried) {
        queried->AddRef();
    }
    KdRelease(queried);
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(queried);
    return out;
}

// STUB: VC1 0x004b1f50
void* KdDisplay::GetMixer()
{
    return 0;
}

// STUB: VC1 0x004b20c0
int KdDisplay::Unmapped28() { return 0; }

// STUB: VC1 0x004b22e0
int KdDisplay::Unmapped0c() { return 0; }

}
