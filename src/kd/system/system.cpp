#include "kd/system/system.h"

#include "kd/factory.h"
#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/iface/idisplay.h"
#include "host/runtime.h"
#include "memory/fixed_pool.h"

#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed620
IAbstract* g_kdSystem;

// STRING: VC1 0x004d4be4
static const char kSystem[] = "System";
// GLOBAL: VC1 0x004d0658
const char* g_kdNameSystem = kSystem;

static void SystemDtorBase(KdSystem* self);
static unsigned char SystemStartBase(KdSystem* self, IUnknown** object);
static void StartPostMixer();

KdSystem::KdSystem()
{
    memset(pad04, 0, sizeof(pad04));
    refs = 0;
    memset(tail, 0, sizeof(tail));
}

// MOV EAX, 0x004d4be4; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041af30
const char* KdSystem::Script14()
{
    return kSystem;
}

// LEA EAX, [ECX+0x10]; RET. ISystem vtable+0x20. IInputStream starts at +0x0c.
// FUNCTION: VC1 0x00461f60
void* KdSystem::Unmapped20()
{
    return reinterpret_cast<char*>(this) + 0x10;
}

// FUNCTION: VC1 0x00462080
void* SystemBaseQueryInterface(KdSystem* self, const char** iid)
{
    const char* name = *iid;
    if (name == g_iidScriptObject) {
        if (!self) {
            return 0;
        }
        return static_cast<IScriptObject*>(self);
    }
    if (name == g_iidRenderable) {
        if (!self) {
            return 0;
        }
        return static_cast<IRenderable*>(self);
    }
    if (name == g_iidShutdownable || name == g_iidComponentGroup
        || name == g_iidSystem) {
        return self;
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(self);
    }
    return 0;
}

static void BindNamed(IUnknown** slot, const char** factoryName, const char** iid)
{
    void* unknown = 0;
    IUnknown* typed;
    if (*slot) {
        (*slot)->Release();
    }
    GetKdFactory()->QueryNamed(&unknown, factoryName);
    if (!unknown) {
        *slot = 0;
        return;
    }
    typed = static_cast<IUnknown*>(
        static_cast<IUnknown*>(unknown)->QueryInterface(iid));
    *slot = typed;
    if (typed) {
        typed->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
}

// FUNCTION: VC1 0x00462350
void ConstructKdSystem(KdSystem* self)
{
    unsigned* object;
    LARGE_INTEGER qpc;
    unsigned long long ticks;
    IUnknown** slots;
    IUnknown* updateable;
    int grow;
    unsigned bit;
    int index;

    self->KdSystem::KdSystem();
    object = reinterpret_cast<unsigned*>(self);
    object[0xd] = 1;
    object[0x16] = 0x3f800000;
    object[0x1b] = 0xff000000;
    QueryPerformanceCounter(&qpc);
    ticks = static_cast<unsigned long long>(qpc.QuadPart);
    *reinterpret_cast<unsigned long long*>(object + 0x1e) = ticks;
    QueryPerformanceCounter(&qpc);
    ticks = static_cast<unsigned long long>(qpc.QuadPart);
    object[0x20] = static_cast<unsigned>(ticks);
    object[0x21] = static_cast<unsigned>(ticks >> 32);
    object[0x22] = g_sizeClassHeap.allocatedBytes;
    (void)qpc;

    slots = reinterpret_cast<IUnknown**>(object);
    BindNamed(slots + 0x10, &g_kdNameResourceManager, &g_iidResourceManager);
    BindNamed(slots + 0x11, &g_kdNameCamera, &g_iidCamera);
    if (slots[0x11]) {
        slots[0x11]->AddRef();
    }
    if (slots[0x12]) {
        slots[0x12]->Release();
    }
    slots[0x12] = slots[0x11];
    BindNamed(slots + 0x13, &g_kdNameRenderGroup, &g_iidFunctionalGroup);
    BindNamed(slots + 0x14, &g_kdNameUpdateGroup, &g_iidFunctionalGroup);
    BindNamed(slots + 0x15, &g_kdNameInputGroup, &g_iidFunctionalGroup);

    if (!slots[0x14]) {
        updateable = 0;
    }
    else {
        updateable = static_cast<IUnknown*>(slots[0x14]->QueryInterface(&g_iidUpdateable));
        if (updateable) {
            updateable->AddRef();
        }
    }
    if (updateable) {
        typedef int (__thiscall *SetLayerFn)(IUnknown* self, unsigned layer);
        void** const vtbl = *reinterpret_cast<void***>(updateable);
        if (vtbl && vtbl[4]) {
            (reinterpret_cast<SetLayerFn>(vtbl[4]))(updateable, object[0x1c]);
        }
        updateable->Release();
    }

    grow = 1;
    do {
        unsigned needed = ((object[0x19] >> 2) + 1) * 4;
        if (object[0x1a] < needed) {
            GrowCountedBuffer(object, static_cast<int>(needed), 0);
        }
        object[0x19] = needed;
        if (object[0x18]) {
            *reinterpret_cast<unsigned*>(object[0x18] + needed - 4) = 0x7fffffff;
        }
        grow *= 2;
    } while (grow != 0x100);

    bit = 1;
    index = 0;
    do {
        bit *= 2;
        index += 1;
    } while (bit < 4);
    if (object[0x18]) {
        *reinterpret_cast<unsigned*>(object[0x18] + index * 4) = 10;
    }
}

// STUB: VC1 0x00462700
int KdSystem::Unmapped24() { return 0; }

// STUB: VC1 0x00462710
int KdSystem::Unmapped1c() { return 0; }

// STUB: VC1 0x00462720
int KdSystem::Shutdown() { return 0; }

// STUB: VC1 0x00462900
static void SystemDtorBase(KdSystem* self)
{
    (void)self;
}

// STUB: VC1 0x00462e80
static unsigned char SystemStartBase(KdSystem* self, IUnknown** object)
{
    (void)self;
    (void)object;
    return 1;
}

// STUB: VC1 0x004630d0
int KdSystem::Unmapped18() { return 0; }

// STUB: VC1 0x004630f0
int KdSystem::Unmapped14() { return 0; }

// STUB: VC1 0x004631b0
int KdSystem::Unmapped10() { return 0; }

// STUB: VC1 0x004964f0
static void StartPostMixer()
{
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x004a8c30
unsigned char KdSystem::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// FUNCTION: VC1 0x004a8d20
void* KdSystem::QueryInterface(const char** iid)
{
    void* found = SystemBaseQueryInterface(this, iid);
    if (found) {
        return found;
    }
    if (*iid == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// FUNCTION: VC1 0x004a8d50
unsigned KdSystem::Release()
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

// FUNCTION: VC1 0x004a8d70
unsigned KdSystem::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// FUNCTION: VC1 0x004a8de0
void** CreateKdSystem(void** out)
{
    IUnknown* queried;
    KdSystem* memory;

    if (g_kdSystem) {
        queried = static_cast<IUnknown*>(g_kdSystem->QueryInterface(&g_iidAbstract));
        *out = queried;
        if (queried) {
            queried->AddRef();
        }
        return out;
    }

    memory = static_cast<KdSystem*>(malloc(sizeof(KdSystem)));
    if (!memory) {
        g_kdSystem = 0;
        *out = 0;
        return out;
    }
    ConstructKdSystem(memory);
    g_kdSystem = static_cast<IAbstract*>(memory);
    queried = static_cast<IUnknown*>(memory->QueryInterface(&g_iidAbstract));
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

// STUB: VC1 0x004a8eb0
int KdSystem::Start(IUnknown** object)
{
    IDisplay* display;
    unsigned char ok;

    ok = SystemStartBase(this, object);
    if (!ok) {
        return ok;
    }
    if (!object || !*object) {
        display = 0;
    }
    else {
        display = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (display) {
            display->AddRef();
        }
    }
    display->GetMixer();
    StartPostMixer();
    display->Release();
    return 1;
}

unsigned char KdSystem::Script0c(void*, void*) { return 0; }

KdSystem::~KdSystem()
{
    SystemDtorBase(this);
}

}
