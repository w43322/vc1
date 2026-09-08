#include "kd/display/display.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed54c
IAbstract* g_kdDisplay;

// STRING: VC1 0x004d4c4c
static const char kDisplay[] = "Display";
// GLOBAL: VC1 0x004d0630
const char* g_kdNameDisplay = kDisplay;

static void DisplayDtorBase(KdDisplay* self);
static void ConstructDisplayBase(KdDisplay* self);
static unsigned char Script0cBody(void* a, void* b);

KdDisplay::KdDisplay()
{
    target = 0;
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    matrix[4] = 1.0f;
    matrix[5] = 0.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 1.0f;
    enabled58 = 1;
}

int KdDisplay::Display14() { return 0; }
int KdDisplay::SetSize(unsigned, unsigned) { return 1; }
int KdDisplay::Display24() { return 0; }
int KdDisplay::Display1c() { return 0; }
int KdDisplay::Display34() { return 0; }
int KdDisplay::Display30() { return 0; }
int KdDisplay::Display38() { return 0; }
int KdDisplay::Display18() { return 0; }
int KdDisplay::Display10() { return 0; }
int KdDisplay::Script14() { return 0; }

KdDisplay::~KdDisplay()
{
    if (target) {
        target->Release();
    }
    target = 0;
    DisplayDtorBase(this);
}

// this = IDisplay (KdDisplay+0x14). IDisplay 0x004d4e50, IShutdownable +4, IAbstract +8.
// FUNCTION: VC1 0x00463e60
void* DisplayFaceQueryInterface(IDisplay* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidDisplay) {
        if (reinterpret_cast<char*>(self) - 4 == 0) {
            return 0;
        }
        return self;
    }
    if (name == g_iidShutdownable) {
        if (reinterpret_cast<char*>(self) - 4 == 0) {
            return 0;
        }
        return reinterpret_cast<char*>(self) + 4;
    }
    if (name == g_iidAbstract) {
        return reinterpret_cast<char*>(self) + 8;
    }
    return 0;
}

// FUNCTION: VC1 0x00466320
void* DisplayBaseQueryInterface(KdDisplay* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        return self;
    }
    if (name == g_iidRenderTarget) {
        if (!self) {
            return 0;
        }
        return static_cast<IRenderTarget*>(self);
    }
    {
        void* found = DisplayFaceQueryInterface(static_cast<IDisplay*>(self), iid);
        if (found) {
            return found;
        }
        if (name == g_iidAbstract) {
            return static_cast<IAbstract*>(self);
        }
    }
    return 0;
}

// STUB: VC1 0x004664a0
static void DisplayDtorBase(KdDisplay* self)
{
    (void)self;
}

// STUB: VC1 0x004665c0
static void ConstructDisplayBase(KdDisplay* self)
{
    (void)self;
}

// STUB: VC1 0x00466b00
static unsigned char Script0cBody(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// FUNCTION: VC1 0x004b1d30
unsigned char KdDisplay::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// FUNCTION: VC1 0x004b1d40
void* KdDisplay::QueryInterface(const char** iid)
{
    void* found = DisplayBaseQueryInterface(this, iid);
    if (found) {
        return found;
    }
    if (*iid == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x004b1d70
unsigned KdDisplay::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned* p = reinterpret_cast<unsigned*>(iface + 1);
    unsigned n;

    *p -= 1;
    n = *p;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x004b1d90
unsigned KdDisplay::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// EAX=this. Calls FUN_004665c0 with this in ESI, then overwrites vptrs/floats.
// FUNCTION: VC1 0x004b1e40
KdDisplay* ConstructKdDisplay(KdDisplay* self)
{
    ConstructDisplayBase(self);
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
    g_kdDisplay = static_cast<IAbstract*>(object);
    queried = static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract));
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

void* KdDisplay::GetMixer()
{
    return 0;
}

int KdDisplay::Display28() { return 0; }
int KdDisplay::Display0c() { return 0; }
int KdDisplay::Shutdown() { return 0; }

// STUB: VC1 0x004b22e0
unsigned char KdDisplay::Script0c(void* a, void* b)
{
    return Script0cBody(a, b) != 0;
}

}
