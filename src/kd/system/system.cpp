#include "kd/system/system.h"

#include "kd/factory.h"
#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iupdateable.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iresource_manager.h"
#include "bindings/matrix3.h"
#include "bindings/squirrel_object.h"
#include "host/runtime.h"
#include "memory/fixed_pool.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed620
IAbstract* g_kdSystem;
// GLOBAL: VC1 0x004ed6f8
IUnknown* g_kdServices;
// GLOBAL: VC1 0x004ed4fc
void* g_timedEventHead;

// STRING: VC1 0x004d4be4
static const char kSystem[] = "System";
// GLOBAL: VC1 0x004d0658
const char* g_kdNameSystem = kSystem;

void StartPostMixer();
static void UnlinkCountedNode(void* block);
static void* AllocSystemBuffer(unsigned size, unsigned flags, unsigned owner);
static void FactoryClearSlots(void* begin, void* end);
static unsigned char* InsertCountedRecord(void* factory, unsigned char* pos, void* source);
static void ReserveRecordTable(void* factory, unsigned newCount);

// QPC in nanoseconds; inlined at every use (fild, fmul g_nsPerTick, _ftol2).
static __int64 SystemNow()
{
    LARGE_INTEGER qpc;

    QueryPerformanceCounter(&qpc);
    return static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick);
}

// RET 8. Default IRenderable vtable+0x0c: forwards to Render (vtable+0x10).
// FUNCTION: VC1 0x00401010
int IRenderable::Render0c(void* graphics, void* unused)
{
    (void)unused;
    return Render(graphics);
}

// EAX=key, ECX=record. MemCompare of +4/+8 string fields; AL = cmp < 0.
// FUNCTION: VC1 0x00409970
unsigned char RecordEq(void* key, void* record)
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

// ESI=this. Destroys the records up to capacity (capacity / 20 via
// 0xcccccccd); the KdFactory member destructor is inlined after it.
// FUNCTION: VC1 0x00415bf0
KdRecordList::~KdRecordList() throw()
{
    unsigned n;
    unsigned char* begin;

    if (items.capacity) {
        n = items.capacity / 20;
        begin = reinterpret_cast<unsigned char*>(items.begin);
        FactoryClearSlots(begin, begin + n * 20);
    }
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

// EAX=key, EBX=factory. Lookup then copy-insert a 0x14-byte name record.
// FUNCTION: VC1 0x004161b0
IUnknown** InsertCountedSlot(void* key, void* factory) throw()
{
    KdFactory* records = static_cast<KdFactory*>(factory);
    unsigned char* pos;
    unsigned char* end;

    pos = LookupCountedRecord(factory, key);
    end = reinterpret_cast<unsigned char*>(records->begin) + records->size;
    if (pos != end && RecordEq(key, pos) == 0) {
        return reinterpret_cast<IUnknown**>(pos + 0x10);
    }
    NamedRecord rec(*static_cast<KdString*>(key));
    pos = InsertCountedRecord(factory, pos, &rec);
    return reinterpret_cast<IUnknown**>(pos + 0x10);
}

// cdecl out, begin, end, dest factory. Copies IUnknown* range into a pointer vector.
// FUNCTION: VC1 0x00416280
void AppendCountedRange(void* out, void* begin, void* end, void* factory)
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

// EAX=factory, EDI=source record, stdcall pos RET 4. Inserts a 0x14-byte slot.
// FUNCTION: VC1 0x00416360
static unsigned char* InsertCountedRecord(void* factory, unsigned char* pos, void* source)
{
    KdFactory* records = static_cast<KdFactory*>(factory);
    unsigned char* oldBegin;
    unsigned char* inserted;
    IUnknown* object;

    oldBegin = reinterpret_cast<unsigned char*>(records->begin);
    ReserveRecordTable(factory, records->size / 20 + 1);
    inserted = static_cast<unsigned char*>(InsertCountedBytes(
        factory,
        reinterpret_cast<unsigned char*>(records->begin) + (pos - oldBegin),
        0x14));
    AssignCountedString(
        inserted,
        *reinterpret_cast<char**>(static_cast<unsigned char*>(source) + 4),
        *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(source) + 8));
    object = *reinterpret_cast<IUnknown**>(static_cast<unsigned char*>(source) + 0x10);
    if (object) {
        object->AddRef();
    }
    if (*reinterpret_cast<IUnknown**>(inserted + 0x10)) {
        (*reinterpret_cast<IUnknown**>(inserted + 0x10))->Release();
    }
    *reinterpret_cast<IUnknown**>(inserted + 0x10) = object;
    return inserted;
}

// EAX=factory, EDX=key. Binary search of 0x14-byte name records.
// FUNCTION: VC1 0x004163d0
unsigned char* LookupCountedRecord(void* factory, void* key)
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

// EAX=factory, ECX=new record count. Grow 0x14-byte slots; new ones use empty g_kdLogTag.
// FUNCTION: VC1 0x004164a0
static void ReserveRecordTable(void* factory, unsigned newCount)
{
    KdFactory* records = static_cast<KdFactory*>(factory);
    unsigned oldCap;
    unsigned char* slot;
    unsigned char* end;

    oldCap = records->capacity / 20;
    GrowCountedBuffer(records, static_cast<int>(newCount * 20), 0);
    if (records->capacity / 20 > oldCap) {
        slot = reinterpret_cast<unsigned char*>(records->begin) + oldCap * 20;
        end = reinterpret_cast<unsigned char*>(records->begin)
            + (records->capacity / 20) * 20;
        for (; slot < end; slot += 0x14) {
            new (slot) NamedRecord();
        }
    }
}

// EBX=this. Releases each IUnknown* up to capacity; the KdFactory member
// destructor is inlined after it. Owners: KdComponentGroup::extra, KdFont.
// FUNCTION: VC1 0x00419bb0
KdRefList::~KdRefList() throw()
{
    IUnknown** slot;
    IUnknown** end;
    IUnknown* object;

    if (items.capacity) {
        slot = reinterpret_cast<IUnknown**>(items.begin);
        end = slot + (items.capacity / sizeof(IUnknown*));
        for (; slot < end; slot += 1) {
            object = *slot;
            if (object) {
                object->Release();
            }
            *slot = 0;
        }
    }
}

// ECX=this, EAX=count. Grows to count slots and nulls the new ones.
// FUNCTION: VC1 0x00419ee0
void KdRefList::Reserve(unsigned count)
{
    unsigned old;
    unsigned now;
    IUnknown** slot;
    IUnknown** end;

    old = items.capacity / sizeof(IUnknown*);
    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(IUnknown*)), 1);
    now = items.capacity / sizeof(IUnknown*);
    if (now > old) {
        slot = reinterpret_cast<IUnknown**>(items.begin) + old;
        end = reinterpret_cast<IUnknown**>(items.begin) + now;
        for (; slot < end; ++slot) {
            new (slot) IUnknown*(0);
        }
    }
}

// XOR EAX,EAX; RET. IRenderable vtable+0x14; ICF-folded return-0.
// FUNCTION: VC1 0x0041adb0 FOLDED
int KdSystemBase::Render14()
{
    return 0;
}

// MOV EAX, 0x004d4be4; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041af30
const char* KdSystemBase::Script14()
{
    return kSystem;
}

// RET 4. ISystem vtable+0x50.
// FUNCTION: VC1 0x00461ea0
void KdSystemBase::SetTimeScale(float scale)
{
    timeScale = scale;
}

// ISystem vtable+0x4c.
// FUNCTION: VC1 0x00461eb0
float KdSystemBase::GetTimeScale()
{
    return timeScale;
}

// ISystem vtable+0x3c.
// FUNCTION: VC1 0x00461ec0
unsigned KdSystemBase::GetTime()
{
    return time;
}

// ISystem vtable+0x34: Update then Render, both virtual.
// FUNCTION: VC1 0x00461ed0
void KdSystemBase::Frame()
{
    Update();
    Render();
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x00461ef0
unsigned char KdSystemBase::Script10(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// LEA EAX, [ECX+0x10]; RET. KdComponentGroup::Unmapped20 inlined.
// FUNCTION: VC1 0x00461f60
void* KdSystemBase::Unmapped20()
{
    return KdComponentGroup::Unmapped20();
}

// RET 4. Null selects the default camera; otherwise QI ICamera.
// FUNCTION: VC1 0x00461f70
void KdSystemBase::SetCamera(IUnknown** value)
{
    IUnknown* queried;

    if (*value) {
        if (activeCamera) {
            activeCamera->Release();
        }
        if (!*value) {
            activeCamera = 0;
            return;
        }
        queried = static_cast<IUnknown*>((*value)->QueryInterface(&g_iidCamera));
        activeCamera = queried;
        if (queried) {
            queried->AddRef();
        }
        return;
    }
    if (camera) {
        camera->AddRef();
    }
    if (activeCamera) {
        activeCamera->Release();
    }
    activeCamera = camera.ptr;
}

// RET 4. Active camera as IAbstract.
// FUNCTION: VC1 0x00461ff0
void** KdSystemBase::GetCamera(void** out)
{
    IUnknown* queried;

    if (!activeCamera) {
        *out = 0;
        return out;
    }
    queried = static_cast<IUnknown*>(activeCamera->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    return out;
}

// RET 4.
// FUNCTION: VC1 0x00462030
void** KdSystemBase::GetDisplay(void** out)
{
    IDisplay* value = display;

    *out = value;
    if (value) {
        value->AddRef();
    }
    return out;
}

// ISystem vtable+0x38.
// FUNCTION: VC1 0x00462050
void KdSystemBase::ResetClock()
{
    lastTicks = SystemNow();
}

// IScriptObject this, IRenderable +4, ISystem for its three names, IAbstract +0x30.
// FUNCTION: VC1 0x00462080
void* KdSystemBase::QueryInterface(const char** iid)
{
    const char* name = *iid;

    if (name == g_iidScriptObject) {
        return static_cast<IScriptObject*>(this);
    }
    if (name == g_iidRenderable) {
        return static_cast<IRenderable*>(this);
    }
    if (name == g_iidShutdownable || name == g_iidComponentGroup || name == g_iidSystem) {
        return static_cast<ISystem*>(this);
    }
    if (name == g_iidAbstract) {
        return static_cast<IAbstract*>(this);
    }
    return 0;
}

// ADD ECX, 0x30; refs at IAbstract+4; deleting dtor on zero.
// FUNCTION: VC1 0x004620e0
unsigned KdSystemBase::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// refs at +0x34.
// FUNCTION: VC1 0x00462100
unsigned KdSystemBase::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. this = IRenderable. Clears with clearColor unless 0xffffff, then
// renders renderGroup through its IRenderable face.
// FUNCTION: VC1 0x004621e0
int KdSystemBase::Render(void* graphics)
{
    typedef void (__thiscall *FnClear)(IUnknown* graphics, unsigned color);
    IUnknown* target = *static_cast<IUnknown**>(graphics);
    IRenderable* renderable;

    if (clearColor != 0xffffff) {
        reinterpret_cast<FnClear>((*reinterpret_cast<void***>(target))[0xf])(
            target, clearColor);
    }
    if (renderGroup) {
        renderable = static_cast<IRenderable*>(renderGroup->QueryInterface(&g_iidRenderable));
        if (renderable) {
            renderable->AddRef();
        }
    }
    else {
        renderable = 0;
    }
    renderable->Render(graphics);
    return static_cast<int>(renderable->Release());
}

// ISystem vtable+0x2c. Advances time by the clamped (<= 100 ms) scaled frame
// delta, fed to updateGroup's IUpdateable in per-slice chunks for the eight
// slice bits 1..0x80.
// FUNCTION: VC1 0x00462240
void KdSystemBase::Update()
{
    __int64 now;
    int elapsed;
    int dt;
    int left;
    int chunk;
    unsigned bit;
    int slice;
    IUpdateable* updater;

    now = SystemNow();
    elapsed = static_cast<int>((now - lastTicks) / 1000000);
    if (elapsed > 100) {
        elapsed = 100;
    }
    dt = static_cast<int>(timeScale * elapsed);
    time += dt;
    lastTicks = now;
    if (updateGroup) {
        updater = static_cast<IUpdateable*>(updateGroup->QueryInterface(&g_iidUpdateable));
        if (updater) {
            updater->AddRef();
        }
    }
    else {
        updater = 0;
    }
    bit = 1;
    slice = 0;
    do {
        left = dt;
        do {
            chunk = reinterpret_cast<int*>(slices.begin)[slice];
            if (left < chunk) {
                chunk = left;
            }
            updater->Update(bit, chunk);
            left -= chunk;
        } while (left > 0);
        slice++;
        bit *= 2;
    } while (bit != 0x100);
    if (updater) {
        updater->Release();
    }
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
    typed = static_cast<IUnknown*>(static_cast<IUnknown*>(unknown)->QueryInterface(iid));
    *slot = typed;
    if (typed) {
        typed->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
}

// ESI=this. Binds the named services and groups, gives updateGroup its start
// time, fills eight 0x7fffffff slice limits and sets slice 2 to 10.
// FUNCTION: VC1 0x00462350
KdSystemBase::KdSystemBase() throw()
{
    IUpdateable* updater;
    unsigned* slot;
    unsigned needed;
    int grow;
    unsigned bit;
    int index;

    display = 0;
    resourceManager = 0;
    camera = 0;
    activeCamera = 0;
    renderGroup = 0;
    updateGroup = 0;
    inputGroup = 0;
    consoleFont = 0;
    clearColor = 0xff000000;
    heapBlock = 0;
    consoleText[0] = 0;
    time = 0;
    lastTicks = SystemNow();
    field8c = 0;
    startTicks = SystemNow();
    timeScale = 1.0f;
    heapBytes = g_sizeClassHeap.allocatedBytes;

    BindNamed(&resourceManager.ptr, &g_kdNameResourceManager, &g_iidResourceManager);
    BindNamed(&camera.ptr, &g_kdNameCamera, &g_iidCamera);
    if (camera) {
        camera->AddRef();
    }
    if (activeCamera) {
        activeCamera->Release();
    }
    activeCamera = camera.ptr;
    BindNamed(reinterpret_cast<IUnknown**>(&renderGroup.ptr), &g_kdNameRenderGroup,
        &g_iidFunctionalGroup);
    BindNamed(reinterpret_cast<IUnknown**>(&updateGroup.ptr), &g_kdNameUpdateGroup,
        &g_iidFunctionalGroup);
    BindNamed(reinterpret_cast<IUnknown**>(&inputGroup.ptr), &g_kdNameInputGroup,
        &g_iidFunctionalGroup);

    if (updateGroup) {
        updater = static_cast<IUpdateable*>(updateGroup->QueryInterface(&g_iidUpdateable));
        if (updater) {
            updater->AddRef();
        }
    }
    else {
        updater = 0;
    }
    updater->SetLayer(time);
    updater->Release();

    grow = 1;
    do {
        needed = ((slices.size >> 2) + 1) * 4;
        if (slices.capacity < needed) {
            GrowCountedBuffer(&slices, static_cast<int>(needed), 0);
        }
        slices.size = needed;
        slot = reinterpret_cast<unsigned*>(
            reinterpret_cast<unsigned char*>(slices.begin) + needed - 4);
        *slot = 0x7fffffff;
        grow *= 2;
    } while (grow != 0x100);

    bit = 1;
    index = 0;
    do {
        bit *= 2;
        index++;
    } while (bit < 4);
    reinterpret_cast<unsigned*>(slices.begin)[index] = 10;
}

// ADD ECX, 0xc; JMP 0x00472870.
// FUNCTION: VC1 0x00462700
int KdSystemBase::Unmapped24()
{
    return KdComponentGroup::Unmapped24();
}

// ADD ECX, 0xc; JMP 0x004728f0.
// FUNCTION: VC1 0x00462710
KdFactory* KdSystemBase::Unmapped1c(const char** iid)
{
    return KdComponentGroup::Unmapped1c(iid);
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

// Frees heapBlock, shuts the component table down, clears the three groups and
// points every service slot back at g_kdServices.
// FUNCTION: VC1 0x00462720
int KdSystemBase::Shutdown() throw()
{
    void* block = heapBlock;

    if (block) {
        UnlinkCountedNode(block);
        free(block);
    }
    heapBlock = 0;
    KdComponentGroup::Shutdown();

    if (renderGroup) {
        renderGroup->Clear();
    }
    if (updateGroup) {
        updateGroup->Clear();
    }
    if (inputGroup) {
        inputGroup->Clear();
    }

    RebindService(reinterpret_cast<IUnknown**>(&renderGroup.ptr), &g_iidFunctionalGroup);
    RebindService(reinterpret_cast<IUnknown**>(&updateGroup.ptr), &g_iidFunctionalGroup);
    RebindService(reinterpret_cast<IUnknown**>(&inputGroup.ptr), &g_iidFunctionalGroup);
    RebindService(&resourceManager.ptr, &g_iidResourceManager);
    RebindService(reinterpret_cast<IUnknown**>(&display.ptr), &g_iidDisplay);
    RebindService(&activeCamera.ptr, &g_iidCamera);
    RebindService(&camera.ptr, &g_iidCamera);
    return 0;
}

// EAX=this. Shutdown and clear g_kdSystem; the members (consoleFont, slices,
// inputGroup .. display) and the KdComponentGroup tables follow inline.
// FUNCTION: VC1 0x00462900
KdSystemBase::~KdSystemBase() throw()
{
    KdSystemBase::Shutdown();
    g_kdSystem = 0;
}

// RET 4. Takes the display, allocates heapBlock, starts the post mixer.
// FUNCTION: VC1 0x00462e80
int KdSystemBase::Start(IUnknown** object)
{
    IDisplay* face;

    if (display) {
        display->Release();
    }
    if (!object || !*object) {
        display = 0;
    }
    else {
        face = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        display = face;
        if (face) {
            face->AddRef();
        }
    }

    heapBlock = AllocSystemBuffer(10000000, 0, reinterpret_cast<unsigned>(this));
    StartPostMixer();

    if (!object || !*object) {
        face = 0;
    }
    else {
        face = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (face) {
            face->AddRef();
        }
    }
    if (face) {
        face->GetMixer();
        StartPostMixer();
        face->Release();
        face = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (face) {
            face->AddRef();
        }
        face->GetMixer();
        StartPostMixer();
        face->Release();
    }
    return 1;
}

// RET 8. KdComponentGroup::Unmapped18 (0x00472a70).
// FUNCTION: VC1 0x004630d0
void** KdSystemBase::Unmapped18(void** out, void* key)
{
    KdComponentGroup::Unmapped18(out, key);
    return out;
}

// STRING: VC1 0x004d5424
static const char kObjectNotFound[] = "Object %s not found";

// RET 8. Removes key; on success also removes it from the update, render and
// input groups and runs Unmapped24.
// FUNCTION: VC1 0x004630f0
void** KdSystemBase::Unmapped14(void** out, void* key)
{
    IUnknown* found;

    KdComponentGroup::Unmapped14(reinterpret_cast<void**>(&found), key);
    if (!found) {
        HostLogError(&g_kdNameSystem, kObjectNotFound,
            *reinterpret_cast<void**>(static_cast<char*>(key) + 4));
        *out = found;
        if (found) {
            found->AddRef();
            found->Release();
        }
        return out;
    }
    updateGroup->Remove(reinterpret_cast<void**>(&found));
    renderGroup->Remove(reinterpret_cast<void**>(&found));
    inputGroup->Remove(reinterpret_cast<void**>(&found));
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

static unsigned char IsRealService(IUnknown* object)
{
    if (!object) {
        return g_kdServices != 0;
    }
    if (!g_kdServices) {
        return 1;
    }
    return object->QueryInterface(&g_iidAbstract)
        != g_kdServices->QueryInterface(&g_iidAbstract);
}

// RET 8. Refuses an existing key; otherwise inserts, names an IIdentifiable
// object after key and adds it to the update, render and input groups.
// FUNCTION: VC1 0x004631b0
int KdSystemBase::Unmapped10(void* key, void** out)
{
    typedef void (__thiscall *FnSetName)(IUnknown* object, void* name);
    IUnknown* existing;
    IUnknown* ident;
    unsigned char flag;

    Unmapped18(reinterpret_cast<void**>(&existing), key);
    flag = IsRealService(existing);
    if (existing) {
        existing->Release();
    }
    if (flag) {
        HostLogError(&g_kdNameSystem, kObjectExists,
            *reinterpret_cast<void**>(static_cast<char*>(key) + 4));
        return 0;
    }
    KdComponentGroup::Unmapped10(key, out);
    Unmapped24();

    if (*out) {
        ident = static_cast<IUnknown*>(
            static_cast<IUnknown*>(*out)->QueryInterface(&g_iidIdentifiable));
        if (ident) {
            ident->AddRef();
        }
    }
    else {
        ident = 0;
    }
    flag = IsRealService(ident);
    if (ident) {
        ident->Release();
    }
    if (flag) {
        if (*out) {
            ident = static_cast<IUnknown*>(
                static_cast<IUnknown*>(*out)->QueryInterface(&g_iidIdentifiable));
            if (ident) {
                ident->AddRef();
            }
        }
        else {
            ident = 0;
        }
        reinterpret_cast<FnSetName>((*reinterpret_cast<void***>(ident))[4])(ident, key);
        ident->Release();
    }
    updateGroup->Add(out);
    renderGroup->Add(out);
    inputGroup->Add(out);
    return 0;
}

// RET 8. Script method table (GetObject, AddObject, ...). Not recovered.
// STUB: VC1 0x00463470
unsigned char KdSystemBase::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// STRING: VC1 0x004d5780
static const char kConsoleFont[] = "res/fonts/console.fnt";

// ISystem vtable+0x30. Begins a frame on the display for the active camera,
// renders through the IRenderable face, overlays consoleText on a black
// backdrop sized by the console font, then presents (display vtable+0x10).
// FUNCTION: VC1 0x00463980
void KdSystemBase::Render() throw()
{
    IGraphics* graphics;
    IFont* font;

    display->Display0c(&graphics, &activeCamera);
    if (!graphics) {
        return;
    }
    static_cast<IRenderable*>(this)->Render(&graphics);
    if (consoleText[0]) {
        if (!consoleFont) {
            KdString path(kConsoleFont);
            KdRef<IResourceManager> manager;
            KdRef<IUnknown> loaded;

            AssignFont(&consoleFont, reinterpret_cast<IUnknown**>(
                (*GetKdResourceManager(&manager.ptr))->LoadFont(
                    reinterpret_cast<void**>(&loaded.ptr), &path)));
        }
        graphics->SetTransform(&g_matrix3Identity);
        graphics->SetFont(reinterpret_cast<IUnknown* const*>(&consoleFont.ptr));
        graphics->SetColor(0xff000000);
        font = consoleFont;
        graphics->FillRect(0.0f, 0.0f, static_cast<float>(font->GetTextWidth(consoleText)),
            static_cast<float>(font->GetHeight()));
        graphics->SetColor(0xffffffff);
        graphics->DrawString(consoleText, 0.0f, 0.0f);
    }
    display->Display10();
    if (graphics) {
        graphics->Release();
    }
}

// EDX=block. Unlinks the timed/heap node at g_timedEventHead.
// FUNCTION: VC1 0x00486830
static void UnlinkCountedNode(void* block)
{
    void** prev;
    void* node;

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
    void* found = KdSystemBase::QueryInterface(iid);
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
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a8d70
unsigned KdSystem::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. Returns the singleton as IAbstract, creating it on first use.
// FUNCTION: VC1 0x004a8de0
void** CreateKdSystem(void** out)
{
    IUnknown* queried;
    void* memory;
    KdSystem* object;

    if (!g_kdSystem) {
        memory = malloc(sizeof(KdSystem));
        object = memory ? new (memory) KdSystem() : 0;
        g_kdSystem = static_cast<IAbstract*>(object);
        if (g_kdSystem) {
            queried = static_cast<IUnknown*>(g_kdSystem->QueryInterface(&g_iidAbstract));
            if (queried) {
                queried->AddRef();
            }
        }
        else {
            queried = 0;
        }
        queried->Release();
        *out = queried;
        queried->AddRef();
        queried->Release();
        return out;
    }
    queried = static_cast<IUnknown*>(g_kdSystem->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    return out;
}

// RET 4. Base Start, then one more mixer pass on the display.
// FUNCTION: VC1 0x004a8eb0
int KdSystem::Start(IUnknown** object)
{
    IDisplay* face;
    int ok;

    ok = KdSystemBase::Start(object);
    if (!ok) {
        return ok;
    }
    if (!object || !*object) {
        face = 0;
    }
    else {
        face = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        if (face) {
            face->AddRef();
        }
    }
    face->GetMixer();
    StartPostMixer();
    face->Release();
    return 1;
}

// RET 8. Script method table for the game system. Not recovered.
// STUB: VC1 0x004a8f20
unsigned char KdSystem::Script0c(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

}
