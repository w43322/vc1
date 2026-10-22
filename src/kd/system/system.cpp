#include "host/clock.h"
#include "kd/system/system.h"

#include "kd/factory.h"
#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iupdateable.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iresource_manager.h"
#include "kd/iface/iinput_handler.h"
#include "bindings/matrix3.h"
#include "bindings/squirrel_object.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "host/platform.h"
#include "memory/fixed_pool.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed620
IAbstract* g_kdSystem;
// GLOBAL: VC1 0x004ed6f8
IUnknown* g_kdServices;
extern Matrix3 g_displayMatrixInv; // display.cpp

// STRING: VC1 0x004d4be4
static const char kSystem[] = "System";
// GLOBAL: VC1 0x004d0658
const char* g_kdNameSystem = kSystem;

static void FactoryClearSlots(void* begin, void* end);
static unsigned char* InsertCountedRecord(void* factory, unsigned char* pos, void* source);
static void ReserveRecordTable(void* factory, unsigned newCount);

// QPC in nanoseconds; inlined at every use (fild, fmul g_nsPerTick, _ftol2).
static __int64 SystemNow()
{
    PlatformClockSample qpc;

    PlatformReadCounter(&qpc);
    return static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick);
}

// RET 8. Default IRenderable vtable+0x0c: forwards to Render (vtable+0x10).
// FUNCTION: VC1 0x00401010
int IRenderable::RenderMasked(void* graphics, void* unused)
{
    (void)unused;
    return Render(graphics);
}

// EAX=key, ECX=record. AL = key < record: MemCompare (left=EDX) gets the key
// text in EDX and the record text in ECX, length as tiebreak.
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
    lenCmp = static_cast<int>(keyLen) - static_cast<int>(recLen);
    lenCmp = (lenCmp > 0) - (lenCmp < 0);
    n = keyLen;
    if (recLen < n) {
        n = recLen;
    }
    cmp = MemCompare(
        *reinterpret_cast<void**>(static_cast<unsigned char*>(key) + 4),
        *reinterpret_cast<void**>(static_cast<unsigned char*>(record) + 4),
        n);
    if (cmp == 0) {
        cmp = lenCmp;
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

// EAX=factory, EBX=key. LookupCountedRecord, then end unless the hit equals key.
// FUNCTION: VC1 0x00416250
unsigned char* FindCountedRecord(void* factory, void* key)
{
    KdFactory* records = static_cast<KdFactory*>(factory);
    unsigned char* pos;
    unsigned char* end;

    pos = LookupCountedRecord(factory, key);
    end = reinterpret_cast<unsigned char*>(records->begin) + records->size;
    if (pos < end && RecordEq(key, pos)) {
        return end;
    }
    return pos;
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
int KdSystemBase::GetRenderOrder()
{
    return 0;
}

// MOV EAX, 0x004d4be4; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0041af30
const char* KdSystemBase::GetTypeName()
{
    return kSystem;
}

// KD_EVENT_TIMER callback installed by Start for its 10 ms timer: one frame.
// Inferred name.
// FUNCTION: VC1 0x00461e80
static void SystemTimerCallback(const KdEvent* event)
{
    KdSystemBase* system = g_kdSystem ? static_cast<KdSystemBase*>(g_kdSystem) : 0;

    (void)event;
    system->Frame();
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
unsigned char KdSystemBase::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Event 0x39 callback installed by KdSystemBase::Start on the display window:
// the input group's pointer handler. Inferred name.
// FUNCTION: VC1 0x00461f00
static void SystemPointerCallback(const KdEvent* event)
{
    KdSystemBase* system = g_kdSystem ? static_cast<KdSystemBase*>(g_kdSystem) : 0;
    KdPtr<IInputHandler> input(system->inputGroup
        ? static_cast<IInputHandler*>(system->inputGroup->QueryInterface(&g_iidInputHandler))
        : 0);

    input->OnPointer(event->data);
}

// LEA EAX, [ECX+0x10]; RET. KdComponentGroup::GetRecords inlined.
// FUNCTION: VC1 0x00461f60
void* KdSystemBase::GetRecords()
{
    return KdComponentGroup::GetRecords();
}

// RET 4. Null selects the default camera; otherwise QI ICamera.
// FUNCTION: VC1 0x00461f70
void KdSystemBase::SetCamera(const KdPtr<IAbstract>& value)
{
    IUnknown* queried;

    if (value.ptr) {
        if (activeCamera) {
            activeCamera->Release();
        }
        if (!value.ptr) {
            activeCamera = 0;
            return;
        }
        queried = static_cast<IUnknown*>(value.ptr->QueryInterface(&g_iidCamera));
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
KdPtr<IAbstract> KdSystemBase::GetCamera()
{
    if (!activeCamera) {
        return KdPtr<IAbstract>();
    }
    return KdPtr<IAbstract>(static_cast<IAbstract*>(activeCamera->QueryInterface(&g_iidAbstract)));
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

// Event 0x38 (key) callback installed by Start on the display window. On a
// press with the 0x20000000 modifier bit in the high dword, key 0x4000000d
// toggles windowed mode and 0x40000073 posts KD_EVENT_QUIT; every key event
// then goes to the input group's key handler. Inferred name.
// FUNCTION: VC1 0x00462110
static void SystemKeyCallback(const KdEvent* event)
{
    KdSystemBase* system = g_kdSystem ? static_cast<KdSystemBase*>(g_kdSystem) : 0;

    if (event->data[2] != 0) {
        const unsigned __int64 flags =
            *reinterpret_cast<const unsigned __int64*>(&event->data[2]);
        if (event->data[0] == 0x4000000d) {
            if ((flags >> 32) & 0x20000000) {
                IDisplay* display = system->display;
                display->SetWindowed(!display->IsWindowed());
            }
        }
        else if (event->data[0] == 0x40000073) {
            if ((flags >> 32) & 0x20000000) {
                KdEvent* quit = AllocEvent();
                quit->type = KD_EVENT_QUIT;
                QueueEvent(quit);
            }
        }
    }
    {
        KdPtr<IInputHandler> input(system->inputGroup
            ? static_cast<IInputHandler*>(system->inputGroup->QueryInterface(&g_iidInputHandler))
            : 0);
        input->OnKey(event->data);
    }
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
    timer = 0;
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
int KdSystemBase::ClearCollected()
{
    return KdComponentGroup::ClearCollected();
}

// ADD ECX, 0xc; JMP 0x004728f0.
// FUNCTION: VC1 0x00462710
KdFactory* KdSystemBase::CollectObjects(const char** iid)
{
    return KdComponentGroup::CollectObjects(iid);
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

// Cancels the frame timer, shuts the component table down, clears the three groups and
// points every service slot back at g_kdServices.
// FUNCTION: VC1 0x00462720
int KdSystemBase::Shutdown() throw()
{
    KdTimer* frameTimer = timer;

    if (frameTimer) {
        UnlinkTimer(frameTimer);
        free(frameTimer);
    }
    timer = 0;
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

// RET 4. Takes the display, starts the 10 ms frame timer, hooks the timer and
// the window's key/pointer events, then reads Settings.ClearColor.
// FUNCTION: VC1 0x00462e80
unsigned char KdSystemBase::Start(IUnknown** object)
{
    IDisplay* face;

    if (display) {
        display->Release();
    }
    if (!*object) {
        display = 0;
    }
    else {
        face = static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay));
        display = face;
        if (face) {
            face->AddRef();
        }
    }

    timer = KdSetTimer(10000000, this);
    KdInstallCallback(SystemTimerCallback, KD_EVENT_TIMER, this);
    {
        KdPtr<IDisplay> window(*object
            ? static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay)) : 0);
        KdInstallCallback(SystemKeyCallback, 0x38, window->GetMixer());
    }
    {
        KdPtr<IDisplay> window(*object
            ? static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay)) : 0);
        KdInstallCallback(SystemPointerCallback, 0x39, window->GetMixer());
    }

    // STRING: VC1 0x004d63d8
    if (SquirrelVM::GetRootTable().Exists("Settings")) {
        SquirrelObject settings = SquirrelVM::GetRootTable().GetValue("Settings");
        // STRING: VC1 0x004d63e4
        if (settings.Exists("ClearColor")) {
            clearColor = settings.GetInt("ClearColor");
        }
    }
    return 1;
}

// RET 8. KdComponentGroup::FindObject (0x00472a70).
// FUNCTION: VC1 0x004630d0
KdPtr<IUnknown> KdSystemBase::FindObject(const KdString& name)
{
    return KdComponentGroup::FindObject(name);
}

// GLOBAL: VC1 0x004d5424
extern const char kSystemObjectNotFound[] = "Object %s not found";

// RET 8. Removes key; on success also removes it from the update, render and
// input groups and runs ClearCollected.
// FUNCTION: VC1 0x004630f0
KdPtr<IUnknown> KdSystemBase::UnregisterObject(const KdString& name)
{
    KdPtr<IUnknown> found = KdComponentGroup::UnregisterObject(name);
    if (!found.ptr) {
        HostLogError(&g_kdNameSystem, kSystemObjectNotFound,
            *reinterpret_cast<void* const*>(reinterpret_cast<const char*>(&name) + 4));
        return found;
    }
    updateGroup->Remove(*reinterpret_cast<const KdPtr<IAbstract>*>(&found));
    renderGroup->Remove(*reinterpret_cast<const KdPtr<IAbstract>*>(&found));
    inputGroup->Remove(*reinterpret_cast<const KdPtr<IAbstract>*>(&found));
    ClearCollected();
    return found;
}

// GLOBAL: VC1 0x004d53f4
extern const char kSystemObjectExists[] = "Object %s already exists";

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
void KdSystemBase::RegisterObject(const KdString& name, const KdPtr<IAbstract>& object)
{
    typedef void (__thiscall *FnSetName)(IUnknown* object, const void* name);
    void** out = reinterpret_cast<void**>(const_cast<IAbstract**>(&object.ptr));
    IUnknown* ident;
    unsigned char flag;

    {
        KdPtr<IUnknown> existing = FindObject(name);
        flag = IsRealService(existing.ptr);
    }
    if (flag) {
        HostLogError(&g_kdNameSystem, kSystemObjectExists,
            *reinterpret_cast<void* const*>(reinterpret_cast<const char*>(&name) + 4));
        return;
    }
    KdComponentGroup::RegisterObject(name, object);
    ClearCollected();

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
        reinterpret_cast<FnSetName>((*reinterpret_cast<void***>(ident))[4])(ident, &name);
        ident->Release();
    }
    updateGroup->Add(*reinterpret_cast<const KdPtr<IAbstract>*>(out));
    renderGroup->Add(*reinterpret_cast<const KdPtr<IAbstract>*>(out));
    inputGroup->Add(*reinterpret_cast<const KdPtr<IAbstract>*>(out));
}

// GLOBAL: VC1 0x004cf8c8
extern const char kScriptSetCamera[] = "SetCamera";

// RET 8. Script method lookup (table 0x004ee5d8, guard bit 0x004ee5e8),
// every entry a vcall thunk on the ISystem face.
// FUNCTION: VC1 0x00463470
unsigned char KdSystemBase::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptRegisterObject] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::RegisterObject);
        methods[kScriptUnregisterObject] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::UnregisterObject);
        methods[kScriptGetObject] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::FindObject);
        methods[kScriptGetCamera] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::GetCamera);
        methods[kScriptSetCamera] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::SetCamera);
        // STRING: VC1 0x004d638c
        methods["UpdateFrame"] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::Update);
        // STRING: VC1 0x004d6398
        methods["RenderFrame"] = CreateScriptMethod<KdSystemBase>(
            static_cast<void (KdSystemBase::*)()>(&KdSystemBase::Render));
        // STRING: VC1 0x004d63a4
        methods["GetUpdateSpeed"] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::GetTimeScale);
        // STRING: VC1 0x004d63b4
        methods["SetUpdateSpeed"] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::SetTimeScale);
        // STRING: VC1 0x004d63c4
        methods["ResetTime"] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::ResetClock);
        // STRING: VC1 0x004d63d0
        methods["GetTime"] = CreateScriptMethod<KdSystemBase>(&KdSystemBase::GetTime);
    }
    return methods.Find(name, out);
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

    display->BeginFrame(&graphics, &activeCamera);
    if (!graphics) {
        return;
    }
    static_cast<IRenderable*>(this)->Render(&graphics);
    if (consoleText[0]) {
        if (!consoleFont) {
            KdString path(kConsoleFont);
            KdRef<IResourceManager> manager;
            KdPtr<IUnknown> loaded((*GetKdResourceManager(&manager.ptr))->GetFont(path));

            AssignFont(&consoleFont, &loaded.ptr);
        }
        graphics->SetTransform(Matrix3::Identity);
        graphics->SetFont(*reinterpret_cast<const KdPtr<IFont>*>(&consoleFont.ptr));
        graphics->SetColor(0xff000000);
        font = consoleFont;
        graphics->FillRect(0.0f, 0.0f, static_cast<float>(font->GetTextWidth(consoleText)),
            static_cast<float>(font->GetHeight()));
        graphics->SetColor(0xffffffff);
        graphics->DrawString(consoleText, 0.0f, 0.0f);
    }
    display->EndFrame();
    if (graphics) {
        graphics->Release();
    }
}

// XOR AL,AL; RET 8. IScriptObject vtable+0x10.
// FUNCTION: VC1 0x004a8c30
unsigned char KdSystem::SetMember(void* a, void* b)
{
    (void)a;
    (void)b;
    return 0;
}

// Event 0x39 callback of KdSystem::Start. Unless the display reports
// IsWindowed, maps the pointer position through g_displayMatrixInv first.
// Inferred name.
// FUNCTION: VC1 0x004a8c40
static void GamePointerCallback(const KdEvent* event)
{
    KdSystemBase* system = g_kdSystem ? static_cast<KdSystemBase*>(g_kdSystem) : 0;
    unsigned* data = const_cast<unsigned*>(event->data);

    if (!system->display->IsWindowed()) {
        float x = static_cast<float>(static_cast<int>(data[2]));
        float y = static_cast<float>(static_cast<int>(data[3]));
        float mappedX = x * g_displayMatrixInv._11 + y * g_displayMatrixInv._21
            + g_displayMatrixInv._31;
        float mappedY = x * g_displayMatrixInv._12 + y * g_displayMatrixInv._22
            + g_displayMatrixInv._32;
        data[2] = static_cast<unsigned>(static_cast<int>(mappedX));
        data[3] = static_cast<unsigned>(static_cast<int>(mappedY));
    }
    {
        KdPtr<IInputHandler> input(system->inputGroup
            ? static_cast<IInputHandler*>(system->inputGroup->QueryInterface(&g_iidInputHandler))
            : 0);
        input->OnPointer(event->data);
    }
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

// RET 4. Base Start, then the game's own pointer callback on the window. It is
// installed last, so it shadows the base one.
// FUNCTION: VC1 0x004a8eb0
unsigned char KdSystem::Start(IUnknown** object)
{
    if (!KdSystemBase::Start(object)) {
        return 0;
    }
    {
        KdPtr<IDisplay> window(*object
            ? static_cast<IDisplay*>((*object)->QueryInterface(&g_iidDisplay)) : 0);
        KdInstallCallback(GamePointerCallback, 0x39, window->GetMixer());
    }
    return 1;
}

// RET 8. KdSystemBase::GetMember.
// FUNCTION: VC1 0x004a8f20
unsigned char KdSystem::GetMember(const char* name, SquirrelObject* out)
{
    return KdSystemBase::GetMember(name, out) != 0;
}

}
