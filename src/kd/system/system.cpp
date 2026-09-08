#include "kd/system/system.h"

#include "kd/factory.h"
#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ishutdownable.h"
#include "bindings/squirrel_object.h"
#include "host/runtime.h"
#include "memory/fixed_pool.h"

#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed620
IAbstract* g_kdSystem;
// GLOBAL: VC1 0x004ed6f8
IUnknown* g_kdServices;
// GLOBAL: VC1 0x004ed4fc
static void* g_timedEventHead;
// GLOBAL: VC1 0x004ed6c0
static void* g_groupNotify;

// STRING: VC1 0x004d4be4
static const char kSystem[] = "System";
// GLOBAL: VC1 0x004d0658
const char* g_kdNameSystem = kSystem;

static void SystemDtorBase(KdSystem* self);
static unsigned char SystemStartBase(KdSystem* self, IUnknown** object);
void StartPostMixer();
static int InputStreamFinish(IInputStream* self);
static void InputStreamClear(IInputStream* self);
static int InputStreamUnmapped1c(IInputStream* self, void* arg);
static void** InputStreamUnmapped18(IInputStream* self, void** out, void* a);
static IUnknown** InputStreamFind(IInputStream* self, IUnknown** out, void* key);
static void InputStreamInsert(IInputStream* self, void* key, void** out);
static void UnlinkCountedNode(KdSystem* self, void* block);
static void* AllocSystemBuffer(unsigned size, unsigned flags, unsigned owner);
static void FactoryDtorTail(void* self);
static void FactoryClearSlots(void* begin, void* end);
static void FactoryReleasePointers(void* self);
static unsigned char RecordEq(void* key, void* record);
static IUnknown** InsertCountedSlot(void* factory);
static void AppendCountedRange(void* out, void* begin, void* end, void* factory);
static unsigned char* LookupCountedRecord(void* factory, void* key);

KdSystem::KdSystem()
{
    memset(pad04, 0, sizeof(pad04));
    refs = 0;
    memset(tail, 0, sizeof(tail));
}

// EAX=key, ECX=record. MemCompare of +4/+8 string fields; AL = cmp < 0.
// FUNCTION: VC1 0x00409970
static unsigned char RecordEq(void* key, void* record)
{
    unsigned keyLen;
    unsigned recLen;
    unsigned n;
    int cmp;
    int lenCmp;

    keyLen = *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(key) + 8);
    recLen = *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(record) + 8);
    n = keyLen;
    if (recLen < n) {
        n = recLen;
    }
    cmp = MemCompare(
        *reinterpret_cast<void**>(static_cast<unsigned char*>(record) + 4),
        *reinterpret_cast<void**>(static_cast<unsigned char*>(key) + 4),
        n);
    if (cmp == 0) {
        lenCmp = static_cast<int>(keyLen) - static_cast<int>(recLen);
        cmp = (lenCmp > 0) - (lenCmp < 0);
    }
    return cmp < 0;
}

// ESI=this. Walk 0x14-byte records then factory Clear. capacity / 20 via 0xcccccccd.
// FUNCTION: VC1 0x00415bf0
static void FactoryDtorTail(void* self)
{
    KdFactory* factory = static_cast<KdFactory*>(self);
    unsigned n;
    unsigned char* begin;

    if (factory->capacity) {
        n = factory->capacity / 20;
        begin = reinterpret_cast<unsigned char*>(factory->begin);
        FactoryClearSlots(begin, begin + n * 20);
    }
    if (factory->capacity) {
        free(factory->begin);
    }
    factory->Clear();
}

// EAX=begin, EDI=end. Each 0x14-byte record Releases +0x10 then nested factory dtor.
// FUNCTION: VC1 0x00415f10
static void FactoryClearSlots(void* begin, void* end)
{
    unsigned char* slot;
    KdFactory* nested;
    IUnknown* object;

    for (slot = static_cast<unsigned char*>(begin);
        slot < static_cast<unsigned char*>(end);
        slot += 0x14) {
        object = *reinterpret_cast<IUnknown**>(slot + 0x10);
        if (object) {
            object->Release();
        }
        *reinterpret_cast<IUnknown**>(slot + 0x10) = 0;
        nested = reinterpret_cast<KdFactory*>(slot);
        if (nested->capacity) {
            free(nested->begin);
        }
        nested->Clear();
    }
}

// EAX=factory. Returns slot for a new IUnknown* in the counted buffer.
// STUB: VC1 0x004161b0
static IUnknown** InsertCountedSlot(void* factory)
{
    (void)factory;
    return 0;
}

// cdecl out, begin, end, dest factory. Copies IUnknown* range into a pointer vector.
// FUNCTION: VC1 0x00416280
static void AppendCountedRange(void* out, void* begin, void* end, void* factory)
{
    KdFactory* dest = static_cast<KdFactory*>(factory);
    IUnknown** src;
    IUnknown** last;
    IUnknown** slot;
    unsigned needed;

    src = static_cast<IUnknown**>(begin);
    last = static_cast<IUnknown**>(end);
    for (; src != last; src += 1) {
        needed = ((dest->size >> 2) + 1) * 4;
        if (dest->capacity < needed) {
            GrowCountedBuffer(dest, static_cast<int>(needed), 0);
        }
        dest->size = needed;
        slot = reinterpret_cast<IUnknown**>(
            reinterpret_cast<unsigned char*>(dest->begin) + needed - 4);
        if (*src) {
            (*src)->AddRef();
        }
        if (*slot) {
            (*slot)->Release();
        }
        *slot = *src;
    }
    *static_cast<void**>(out) = factory;
}

// EAX=factory, EDX=key. Binary search of 0x14-byte name records.
// FUNCTION: VC1 0x004163d0
static unsigned char* LookupCountedRecord(void* factory, void* key)
{
    KdFactory* records = static_cast<KdFactory*>(factory);
    unsigned char* lo;
    unsigned char* hi;
    unsigned char* rec;
    unsigned span;
    unsigned mid;
    unsigned recLen;
    unsigned keyLen;
    unsigned n;
    int cmp;
    int lenCmp;

    lo = reinterpret_cast<unsigned char*>(records->begin);
    hi = lo + records->size;
    if (lo == hi) {
        return lo;
    }
    while (lo != hi) {
        span = static_cast<unsigned>(hi - lo);
        mid = (span / 20) / 2;
        rec = lo + mid * 20;
        recLen = *reinterpret_cast<unsigned*>(rec + 8);
        keyLen = *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(key) + 8);
        n = recLen;
        if (keyLen < n) {
            n = keyLen;
        }
        cmp = MemCompare(
            *reinterpret_cast<void**>(rec + 4),
            *reinterpret_cast<void**>(static_cast<unsigned char*>(key) + 4),
            n);
        if (cmp == 0) {
            lenCmp = static_cast<int>(recLen) - static_cast<int>(keyLen);
            cmp = (lenCmp > 0) - (lenCmp < 0);
        }
        if (cmp < 0) {
            lo = rec + 0x14;
        }
        else {
            hi = rec;
        }
    }
    return lo;
}

// EBX=this. Release each IUnknown* in the counted buffer, then factory Clear.
// FUNCTION: VC1 0x00419bb0
static void FactoryReleasePointers(void* self)
{
    KdFactory* factory = static_cast<KdFactory*>(self);
    IUnknown** slot;
    IUnknown** end;
    IUnknown* object;

    if (factory->capacity) {
        slot = reinterpret_cast<IUnknown**>(factory->begin);
        end = slot + (factory->capacity / sizeof(IUnknown*));
        for (; slot < end; slot += 1) {
            object = *slot;
            if (object) {
                object->Release();
            }
            *slot = 0;
        }
    }
    if (factory->capacity) {
        free(factory->begin);
    }
    factory->Clear();
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

// ADD ECX, 0xc; JMP 0x00472870. ISystem vtable+0x24.
// STUB: VC1 0x00462700
int KdSystem::Unmapped24()
{
    return InputStreamFinish(static_cast<IInputStream*>(this));
}

// ADD ECX, 0xc; JMP 0x004728f0. ISystem vtable+0x1c. RET 4 on the target.
// STUB: VC1 0x00462710
int KdSystem::Unmapped1c(void* arg)
{
    return InputStreamUnmapped1c(static_cast<IInputStream*>(this), arg);
}

static void RebindService(IUnknown** slot, const char** iid)
{
    IUnknown* queried;
    if (*slot) {
        (*slot)->Release();
    }
    if (!g_kdServices) {
        *slot = 0;
        return;
    }
    queried = static_cast<IUnknown*>(g_kdServices->QueryInterface(iid));
    *slot = queried;
    if (queried) {
        queried->AddRef();
    }
}

static void CallGroup14(IUnknown* group)
{
    typedef int (__thiscall *Fn14)(IUnknown* object);
    if (!group) {
        return;
    }
    reinterpret_cast<Fn14>((*reinterpret_cast<void***>(group))[5])(group);
}

// ESI=this. Frees tail[0], clears the input stream, rebinds services from g_kdServices.
// FUNCTION: VC1 0x00462720
int KdSystem::Shutdown()
{
    IUnknown** slots = reinterpret_cast<IUnknown**>(this);
    void* block = *reinterpret_cast<void**>(tail);

    if (block) {
        UnlinkCountedNode(this, block);
        free(block);
    }
    *reinterpret_cast<void**>(tail) = 0;
    InputStreamClear(static_cast<IInputStream*>(this));

    CallGroup14(slots[0x13]);
    CallGroup14(slots[0x14]);
    CallGroup14(slots[0x15]);

    RebindService(slots + 0x13, &g_iidFunctionalGroup);
    RebindService(slots + 0x14, &g_iidFunctionalGroup);
    RebindService(slots + 0x15, &g_iidFunctionalGroup);
    RebindService(slots + 0x10, &g_iidResourceManager);
    RebindService(slots + 0x0f, &g_iidDisplay);
    RebindService(slots + 0x12, &g_iidCamera);
    RebindService(slots + 0x11, &g_iidCamera);
    return 0;
}

// EAX=this. Shutdown, clear g_kdSystem, Release service slots, factory dtors.
// FUNCTION: VC1 0x00462900
static void SystemDtorBase(KdSystem* self)
{
    IUnknown** slots = reinterpret_cast<IUnknown**>(self);
    IUnknown* extra;

    self->Shutdown();
    g_kdSystem = 0;

    extra = *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0xd0);
    if (extra) {
        extra->Release();
    }
    *reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0xd0) = 0;

    FactoryDtorTail(reinterpret_cast<char*>(self) + 0x5c);

    if (slots[0x15]) {
        slots[0x15]->Release();
    }
    slots[0x15] = 0;
    if (slots[0x14]) {
        slots[0x14]->Release();
    }
    slots[0x14] = 0;
    if (slots[0x13]) {
        slots[0x13]->Release();
    }
    slots[0x13] = 0;
    if (slots[0x12]) {
        slots[0x12]->Release();
    }
    slots[0x12] = 0;
    if (slots[0x11]) {
        slots[0x11]->Release();
    }
    slots[0x11] = 0;
    if (slots[0x10]) {
        slots[0x10]->Release();
    }
    slots[0x10] = 0;
    if (slots[0x0f]) {
        slots[0x0f]->Release();
    }
    slots[0x0f] = 0;

    FactoryReleasePointers(reinterpret_cast<char*>(self) + 0x20);
    FactoryDtorTail(reinterpret_cast<char*>(self) + 0x10);
}

// this in ECX. Rebinds Display at +0x3c, allocates tail[0], StartPostMixer.
// FUNCTION: VC1 0x00462e80
static unsigned char SystemStartBase(KdSystem* self, IUnknown** object)
{
    IUnknown** displaySlot;
    IDisplay* display;
    IUnknown* queried;

    displaySlot = reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(self) + 0x3c);
    if (*displaySlot) {
        (*displaySlot)->Release();
    }
    if (!object || !*object) {
        *displaySlot = 0;
    }
    else {
        queried = static_cast<IUnknown*>((*object)->QueryInterface(&g_iidDisplay));
        *displaySlot = queried;
        if (queried) {
            queried->AddRef();
        }
    }

    *reinterpret_cast<void**>(self->tail) = AllocSystemBuffer(10000000, 0,
        static_cast<unsigned>(reinterpret_cast<unsigned>(self)));
    StartPostMixer();

    if (!object || !*object) {
        display = 0;
    }
    else {
        display = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (display) {
            display->AddRef();
        }
    }
    if (display) {
        display->GetMixer();
        StartPostMixer();
        display->Release();
        display = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (display) {
            display->AddRef();
        }
        display->GetMixer();
        StartPostMixer();
        display->Release();
    }
    return 1;
}

// ADD ECX, 0xc; CALL 0x00472a70; return first arg. RET 8. ISystem vtable+0x18.
// STUB: VC1 0x004630d0
void** KdSystem::Unmapped18(void** out, void* a)
{
    return InputStreamUnmapped18(static_cast<IInputStream*>(this), out, a);
}

// STRING: VC1 0x004d5424
static const char kObjectNotFound[] = "Object %s not found";

// RET 8. Looks up key on IInputStream; logs with EAX=g_kdNameSystem on miss.
// FUNCTION: VC1 0x004630f0
void** KdSystem::Unmapped14(void** out, void* key)
{
    IUnknown* found = 0;
    IUnknown* object;

    InputStreamFind(static_cast<IInputStream*>(this), &found, key);
    if (!found) {
        HostLogError(kObjectNotFound,
            *reinterpret_cast<void**>(reinterpret_cast<char*>(key) + 4));
        *out = found;
        if (found) {
            found->AddRef();
            found->Release();
        }
        return out;
    }
    object = *reinterpret_cast<IUnknown**>(tail + 0x18);
    (void)object;
    object = *reinterpret_cast<IUnknown**>(tail + 0x14);
    (void)object;
    object = *reinterpret_cast<IUnknown**>(tail + 0x1c);
    (void)object;
    Unmapped24();
    *out = found;
    if (found) {
        found->AddRef();
        found->Release();
    }
    return out;
}

// STRING: VC1 0x004d53f4
static const char kObjectExists[] = "Object %s already exists";

// RET 8. Unmapped18 lookup, error if already in g_kdServices, else insert.
// FUNCTION: VC1 0x004631b0
int KdSystem::Unmapped10(void* key, void** out)
{
    IUnknown* existing = 0;
    unsigned char exists;
    IUnknown* ident;
    IUnknown* group;
    void* absExisting;
    void* absServices;

    Unmapped18(reinterpret_cast<void**>(&existing), key);
    exists = 0;
    if (existing) {
        if (g_kdServices) {
            absExisting = existing->QueryInterface(&g_iidAbstract);
            absServices = g_kdServices->QueryInterface(&g_iidAbstract);
            exists = absExisting != absServices;
        }
        else {
            exists = 1;
        }
    }
    else if (g_kdServices) {
        exists = 1;
    }
    if (existing) {
        existing->Release();
    }
    if (exists) {
        HostLogError(kObjectExists,
            *reinterpret_cast<void**>(reinterpret_cast<char*>(key) + 4));
        return 0;
    }
    InputStreamInsert(static_cast<IInputStream*>(this), key, out);
    Unmapped24();
    ident = 0;
    if (out && *out) {
        ident = static_cast<IUnknown*>(
            static_cast<IUnknown*>(*out)->QueryInterface(&g_iidIdentifiable));
        if (ident) {
            ident->AddRef();
        }
    }
    exists = 0;
    if (ident) {
        if (g_kdServices) {
            absExisting = ident->QueryInterface(&g_iidAbstract);
            absServices = g_kdServices->QueryInterface(&g_iidAbstract);
            exists = absExisting != absServices;
        }
        else {
            exists = 1;
        }
        ident->Release();
    }
    else if (g_kdServices) {
        exists = 1;
    }
    (void)exists;
    group = *reinterpret_cast<IUnknown**>(tail + 0x18);
    (void)group;
    group = *reinterpret_cast<IUnknown**>(tail + 0x14);
    (void)group;
    group = *reinterpret_cast<IUnknown**>(tail + 0x1c);
    (void)group;
    (void)out;
    return 0;
}

// this=IInputStream. vtbl+0x24, then QI IGroupMember/IShutdownable on each 0x14 record.
// FUNCTION: VC1 0x004727a0
static void InputStreamClear(IInputStream* self)
{
    KdFactory* records;
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IUnknown* member;
    typedef int (__thiscall *Fn24)(IInputStream* stream);
    typedef void (__thiscall *Fn10)(IUnknown* object, void* arg);

    reinterpret_cast<Fn24>((*reinterpret_cast<void***>(self))[9])(self);

    records = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 4);
    it = reinterpret_cast<unsigned char*>(records->begin);
    end = it + records->size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        member = 0;
        if (object) {
            member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
            if (member) {
                member->AddRef();
                reinterpret_cast<Fn10>((*reinterpret_cast<void***>(member))[4])(
                    member, &g_groupNotify);
            }
        }
        if (object) {
            IShutdownable* shutFace;
            shutFace = static_cast<IShutdownable*>(
                object->QueryInterface(&g_iidShutdownable));
            if (shutFace) {
                shutFace->AddRef();
                shutFace->Shutdown();
                shutFace->Release();
            }
        }
        if (member) {
            member->Release();
        }
    }
    if (records->capacity < records->Count()) {
        GrowCountedBuffer(records, 0, 1);
    }
    records->size = 0;
}

// this=IInputStream. QI IComponentGroup on each record, then grow the factory at +0x14.
// FUNCTION: VC1 0x00472870
static int InputStreamFinish(IInputStream* self)
{
    KdFactory* records;
    KdFactory* extra;
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IUnknown* group;
    typedef int (__thiscall *Fn24)(IUnknown* object);

    records = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 4);
    extra = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 0x14);
    it = reinterpret_cast<unsigned char*>(records->begin);
    end = it + records->size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (!object) {
            continue;
        }
        group = static_cast<IUnknown*>(object->QueryInterface(&g_iidComponentGroup));
        if (group) {
            group->AddRef();
            reinterpret_cast<Fn24>((*reinterpret_cast<void***>(group))[9])(group);
            group->Release();
        }
    }
    if (extra->capacity < extra->Count()) {
        GrowCountedBuffer(extra, 0, 1);
    }
    extra->size = 0;
    return 0;
}

// this=IInputStream. RET 4. Clears factory at +0x14, then QI IComponentGroup per record.
// FUNCTION: VC1 0x004728f0
static int InputStreamUnmapped1c(IInputStream* self, void* arg)
{
    KdFactory* extra;
    KdFactory* records;
    unsigned char* it;
    unsigned char* end;
    IUnknown* object;
    IUnknown* group;
    IUnknown** slot;
    unsigned needed;
    typedef void* (__thiscall *Fn1c)(IUnknown* object, void* key);

    extra = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 0x14);
    if (extra->capacity < extra->Count()) {
        GrowCountedBuffer(extra, 0, 1);
    }
    extra->size = 0;

    records = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 4);
    it = reinterpret_cast<unsigned char*>(records->begin);
    end = it + records->size;
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        group = 0;
        if (object) {
            group = static_cast<IUnknown*>(object->QueryInterface(&g_iidComponentGroup));
            if (group) {
                group->AddRef();
                Fn1c fn1c = reinterpret_cast<Fn1c>((*reinterpret_cast<void***>(group))[7]);
                void* range = fn1c(group, arg);
                AppendCountedRange(0,
                    *reinterpret_cast<void**>(static_cast<unsigned char*>(range) + 4),
                    static_cast<unsigned char*>(
                        *reinterpret_cast<void**>(static_cast<unsigned char*>(range) + 4))
                        + *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(range) + 8),
                    extra);
                (void)range;
            }
        }
        if (object && object->QueryInterface(reinterpret_cast<const char**>(arg))) {
            needed = ((extra->size >> 2) + 1) * 4;
            if (extra->capacity < needed) {
                GrowCountedBuffer(extra, static_cast<int>(needed), 0);
            }
            extra->size = needed;
            slot = reinterpret_cast<IUnknown**>(
                reinterpret_cast<unsigned char*>(extra->begin) + needed - 4);
            if (object) {
                object->AddRef();
            }
            if (*slot) {
                (*slot)->Release();
            }
            *slot = object;
        }
        if (group) {
            group->Release();
        }
    }
    return reinterpret_cast<int>(extra);
}

// RET 8. Lookup key; on hit return the record's IUnknown* at +0x10.
// Miss: QI IComponentGroup, vtbl+0x18, keep if Abstract != g_groupNotify.
// FUNCTION: VC1 0x00472a70
static void** InputStreamUnmapped18(IInputStream* self, void** out, void* a)
{
    KdFactory* records;
    unsigned char* pos;
    unsigned char* end;
    unsigned char* it;
    IUnknown* object;
    IUnknown* group;
    IUnknown* found;
    void* absFound;
    void* absNotify;
    unsigned char take;
    typedef void** (__thiscall *Fn18)(IUnknown* group, void** dest, void* key);

    records = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 4);
    pos = LookupCountedRecord(records, a);
    end = reinterpret_cast<unsigned char*>(records->begin) + records->size;
    if (pos < end && RecordEq(a, pos)) {
        pos = end;
    }
    if (pos != end) {
        object = *reinterpret_cast<IUnknown**>(pos + 0x10);
        *out = object;
        if (object) {
            object->AddRef();
        }
        return out;
    }

    it = reinterpret_cast<unsigned char*>(records->begin);
    for (; it != end; it += 0x14) {
        object = *reinterpret_cast<IUnknown**>(it + 0x10);
        if (!object) {
            continue;
        }
        group = static_cast<IUnknown*>(object->QueryInterface(&g_iidComponentGroup));
        if (!group) {
            continue;
        }
        group->AddRef();
        found = 0;
        reinterpret_cast<Fn18>((*reinterpret_cast<void***>(group))[6])(
            group, reinterpret_cast<void**>(&found), a);
        take = 0;
        if (found) {
            if (!g_groupNotify) {
                take = 1;
            }
            else {
                absFound = found->QueryInterface(&g_iidAbstract);
                absNotify = static_cast<IUnknown*>(g_groupNotify)->QueryInterface(
                    &g_iidAbstract);
                if (absFound != absNotify) {
                    take = 1;
                }
                else {
                    found->Release();
                    found = 0;
                }
            }
        }
        else if (g_groupNotify) {
            take = 1;
        }
        if (take) {
            *out = found;
            if (found) {
                found->AddRef();
                found->Release();
            }
            group->Release();
            return out;
        }
        group->Release();
    }
    object = static_cast<IUnknown*>(g_groupNotify);
    *out = object;
    if (object) {
        object->AddRef();
    }
    return out;
}

// RET 8. Lookup key in the 0x14-byte table; on hit, AddRef and erase the slot.
// FUNCTION: VC1 0x00472bf0
static IUnknown** InputStreamFind(IInputStream* self, IUnknown** out, void* key)
{
    KdFactory* records;
    unsigned char* pos;
    unsigned char* end;
    IUnknown* object;
    IUnknown* member;

    records = reinterpret_cast<KdFactory*>(reinterpret_cast<char*>(self) + 4);
    pos = LookupCountedRecord(records, key);
    end = reinterpret_cast<unsigned char*>(records->begin) + records->size;
    if (pos < end && RecordEq(key, pos)) {
        pos = end;
    }
    *out = 0;
    if (pos != end) {
        object = *reinterpret_cast<IUnknown**>(pos + 0x10);
        member = 0;
        if (object) {
            member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
            if (member) {
                member->AddRef();
                typedef void (__thiscall *Fn10)(IUnknown* object, void* arg);
                reinterpret_cast<Fn10>((*reinterpret_cast<void***>(member))[4])(
                    member, &g_groupNotify);
            }
            object->AddRef();
        }
        if (*out) {
            (*out)->Release();
        }
        *out = object;
        EraseCountedBytes(pos);
        records->size -= 0x14;
        if (member) {
            member->Release();
        }
        return out;
    }
    return out;
}

// STUB: VC1 0x00472db0
static void InputStreamInsert(IInputStream* self, void* key, void** out)
{
    IUnknown** slot;
    IUnknown* object;
    IUnknown* member;

    (void)key;
    slot = InsertCountedSlot(self);
    object = out ? static_cast<IUnknown*>(*out) : 0;
    if (object) {
        object->AddRef();
    }
    if (slot) {
        if (*slot) {
            (*slot)->Release();
        }
        *slot = object;
    }
    if (object) {
        member = static_cast<IUnknown*>(object->QueryInterface(&g_iidGroupMember));
        if (member) {
            member->AddRef();
            typedef void (__thiscall *Fn10)(IUnknown* object, void* arg);
            reinterpret_cast<Fn10>((*reinterpret_cast<void***>(member))[4])(
                member, &g_groupNotify);
            member->Release();
        }
    }
}

// EDX=block. Unlinks the timed/heap node at g_timedEventHead.
// FUNCTION: VC1 0x00486830
static void UnlinkCountedNode(KdSystem* self, void* block)
{
    void** prev;
    void* node;

    (void)self;
    if (g_timedEventHead == block) {
        g_timedEventHead = *reinterpret_cast<void**>(static_cast<unsigned char*>(block) + 0x18);
        return;
    }
    prev = &g_timedEventHead;
    node = g_timedEventHead;
    while (node) {
        if (*reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0x18) == block) {
            *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0x18) =
                *reinterpret_cast<void**>(static_cast<unsigned char*>(block) + 0x18);
            return;
        }
        prev = reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0x18);
        node = *prev;
        (void)prev;
    }
}

// malloc(0x20) timed node: size, flags, owner, QPC timestamp; link at +0x18.
// FUNCTION: VC1 0x00486890
static void* AllocSystemBuffer(unsigned size, unsigned flags, unsigned owner)
{
    unsigned* node;
    void** tail;
    LARGE_INTEGER qpc;

    node = static_cast<unsigned*>(malloc(0x20));
    if (!node) {
        return 0;
    }
    node[0] = size;
    node[1] = flags;
    node[2] = 0x3f;
    node[3] = owner;
    QueryPerformanceCounter(&qpc);
    node[4] = static_cast<unsigned>(qpc.QuadPart);
    node[5] = static_cast<unsigned>(qpc.QuadPart >> 32);
    node[6] = 0;
    tail = &g_timedEventHead;
    if (g_timedEventHead) {
        do {
            tail = reinterpret_cast<void**>(static_cast<unsigned char*>(*tail) + 0x18);
        } while (*tail);
    }
    *tail = node;
    return node;
}

// STUB: VC1 0x004964f0
void StartPostMixer()
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

// FUNCTION: VC1 0x004a8eb0
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
