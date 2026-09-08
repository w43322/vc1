#include "kd/factory.h"

#include "kd/iface/iunknown.h"
#include "host/runtime.h"

#include <stdlib.h>

namespace vc1 {

// GLOBAL: VC1 0x004ee2a8
KdFactory g_kdFactory;
// GLOBAL: VC1 0x004ee72c
unsigned g_kdFactoryGuard;
// GLOBAL: VC1 0x004ed6d4
IUnknown* g_kdDefaultUnknown;

KdFactory::~KdFactory()
{
    if (capacity != 0) {
        free(begin);
    }
    Clear();
}

// FUNCTION: VC1 0x00401f70
void KdFactory::Clear()
{
    begin = 0;
    size = 0;
    capacity = 0;
}

static void RegisterOne(const char** nameSlot, KdFactoryFn fn)
{
    FactoryEntry pair;
    FactoryInsert inserted;
    pair.name = *nameSlot;
    pair.create = fn;
    RegisterFactory(&pair, &inserted);
}

// EDI=entry {name,fn}, stdcall out. Compares interned name pointers.
// FUNCTION: VC1 0x00417320
FactoryInsert* RegisterFactory(FactoryEntry* entry, FactoryInsert* out)
{
    FactoryEntry* found;
    FactoryEntry* oldBegin;
    unsigned oldSize;

    found = FindFactoryEntry(&g_kdFactory, reinterpret_cast<const char**>(entry));
    oldBegin = g_kdFactory.begin;
    oldSize = g_kdFactory.size;
    if (found != reinterpret_cast<FactoryEntry*>(
            reinterpret_cast<char*>(oldBegin) + oldSize)
        && found->name >= entry->name) {
        out->entry = found;
        out->inserted = 0;
        return out;
    }

    ReserveFactoryTable((oldSize >> 3) + 1);
    found = static_cast<FactoryEntry*>(InsertCountedBytes(&g_kdFactory,
        reinterpret_cast<FactoryEntry*>(
            reinterpret_cast<char*>(found) - reinterpret_cast<char*>(oldBegin)
            + reinterpret_cast<char*>(g_kdFactory.begin)),
        8));
    found->name = entry->name;
    found->create = entry->create;
    out->entry = found;
    out->inserted = 1;
    return out;
}

// EAX=factory, stdcall key (pointer whose first dword is the interned name).
// FUNCTION: VC1 0x004173a0
FactoryEntry* FindFactoryEntry(KdFactory* factory, const char** key)
{
    FactoryEntry* low = factory->begin;
    FactoryEntry* high = reinterpret_cast<FactoryEntry*>(
        reinterpret_cast<char*>(low) + factory->size);
    const char* name;

    if (low == high) {
        return low;
    }
    name = *key;
    while (low != high) {
        FactoryEntry* mid = low + ((high - low) >> 1);
        if (mid->name < name) {
            low = mid + 1;
        }
        else {
            high = mid;
        }
    }
    return low;
}

// EAX=new entry count.
// FUNCTION: VC1 0x004173e0
void ReserveFactoryTable(unsigned newCount)
{
    unsigned oldCap = g_kdFactory.capacity >> 3;
    GrowCountedBuffer(&g_kdFactory, static_cast<int>(newCount * 8), 0);
    if ((g_kdFactory.capacity >> 3) > oldCap) {
        FactoryEntry* slot = g_kdFactory.begin + oldCap;
        FactoryEntry* end = g_kdFactory.begin + (g_kdFactory.capacity >> 3);
        while (slot < end) {
            if (slot) {
                slot->name = 0;
                slot->create = 0;
            }
            slot += 1;
        }
    }
}

// XOR EAX,EAX; RET. ICF-folded return-0; factory vtable+0x08 and SQBlob::Flush.
// FUNCTION: VC1 0x0041adb0 FOLDED
unsigned KdFactory::Count()
{
    return 0;
}

// EDI=counted buffer (vptr + begin/size/capacity). Aligns new size up to 0x40, CRT realloc.
// STUB: VC1 0x004b8930
void GrowCountedBuffer(void* self, int bytes, char exact)
{
    (void)self;
    (void)bytes;
    (void)exact;
}

// EAX=counted buffer, stdcall pos + byte count. Grows then memmoves the tail.
// STUB: VC1 0x004b8a20
void* InsertCountedBytes(void* self, void* pos, int bytes)
{
    (void)self;
    (void)bytes;
    return pos;
}

// ECX=counted buffer, EAX=length, EBX=source. Empty string uses DAT_004cfa94.
// STUB: VC1 0x004b9090
void AssignCountedString(void* self, const char* text, unsigned len)
{
    (void)self;
    (void)text;
    (void)len;
}

// thiscall factory, stdcall out + name slot, RET 8.
// FUNCTION: VC1 0x004bcdd0
void** __stdcall KdFactory::QueryNamed(void** out, const char** name)
{
    FactoryEntry* found = FindFactoryEntry(this, name);
    FactoryEntry* end = reinterpret_cast<FactoryEntry*>(
        reinterpret_cast<char*>(begin) + size);
    void** created;

    if (found >= end || *name < found->name) {
        *out = g_kdDefaultUnknown;
        if (g_kdDefaultUnknown) {
            g_kdDefaultUnknown->AddRef();
        }
        return out;
    }

    created = found->create(reinterpret_cast<void**>(const_cast<char***>(&name)));
    *out = *created;
    if (*out) {
        static_cast<IUnknown*>(*out)->AddRef();
    }
    if (name) {
        KdRelease(static_cast<IUnknown*>(*reinterpret_cast<void**>(
            const_cast<char***>(&name))));
    }
    return out;
}

// EAX=name slot, EDX=fn.
// FUNCTION: VC1 0x004bce50
void RegisterFactoryPair(const char** name, KdFactoryFn fn)
{
    FactoryEntry pair;
    FactoryInsert inserted;
    pair.name = *name;
    pair.create = fn;
    RegisterFactory(&pair, &inserted);
}

// FUNCTION: VC1 0x004c3350
KdFactory* GetKdFactory()
{
    if ((g_kdFactoryGuard & 1) == 0) {
        g_kdFactoryGuard |= 1;
        g_kdFactory.begin = 0;
        g_kdFactory.size = 0;
        g_kdFactory.capacity = 0;
        atexit(DestroyKdFactory);
    }
    if (g_kdFactory.size == 0) {
        RegisterOne(&g_kdNameDisplay, CreateKdDisplay);
        RegisterOne(&g_kdNameSoundDriver, CreateKdSoundDriver);
        RegisterOne(&g_kdNameSystem, CreateKdSystem);
        RegisterOne(&g_kdNameConsole, CreateKdConsole);
        RegisterOne(&g_kdNameFont, CreateKdFont);
        RegisterOne(&g_kdNameLayer, CreateKdLayer);
        RegisterOne(&g_kdNameUpdateGroup, CreateKdUpdateGroup);
        RegisterOne(&g_kdNameRenderGroup, CreateKdRenderGroup);
        RegisterOne(&g_kdNameInputGroup, CreateKdInputGroup);
        RegisterOne(&g_kdNameResourceManager, CreateKdResourceManager);
        RegisterOne(&g_kdNameCompoundObject, CreateKdCompoundObject);
        RegisterOne(&g_kdNameFile, CreateKdFile);
        RegisterOne(&g_kdNameScene, CreateKdScene);
        RegisterOne(&g_kdNameBlob, CreateKdBlob);
        RegisterOne(&g_kdNameCamera, CreateKdCamera);
        RegisterOne(&g_kdNameScriptedObject, CreateKdScriptedObject);
        RegisterOne(&g_kdNameGameLevel, CreateKdGameLevel);
        RegisterOne(&g_kdNameGameObject, CreateKdGameObject);
        RegisterOne(&g_kdNameTileManager, CreateKdTileManager);
        RegisterOne(&g_kdNameTileAnimationData, CreateKdTileAnimationData);
        RegisterOne(&g_kdNameTileLayer, CreateKdTileLayer);
        RegisterOne(&g_kdNameTileObject, CreateKdTileObject);
        RegisterOne(&g_kdNamePassMap, CreateKdPassMap);
        RegisterOne(&g_kdNameGridRouter, CreateKdGridRouter);
        RegisterOne(&g_kdNameTerrainBuilder, CreateKdTerrainBuilder);
        RegisterOne(&g_kdNameRoadRenderer, CreateKdRoadRenderer);
        RegisterOne(&g_kdNameTileAnimation, CreateKdTileAnimation);
        RegisterOne(&g_kdNameBuildingBuilder, CreateKdBuildingBuilder);
        RegisterOne(&g_kdNameRoute, CreateKdRoute);
        RegisterOne(&g_kdNameTransportMover, CreateKdTransportMover);
        RegisterOne(&g_kdNameGridRectRenderer, CreateKdGridRectRenderer);
        RegisterOne(&g_kdNameObjectWatcher, CreateKdObjectWatcher);
        RegisterOne(&g_kdNameClassificatedRenderer, CreateKdClassificatedRenderer);
        RegisterOne(&g_kdNameTspSolver, CreateKdTspSolver);
        RegisterOne(&g_kdNameGroupRenderer, CreateKdGroupRenderer);
        RegisterOne(&g_kdNameRoadCellsRenderer, CreateKdRoadCellsRenderer);
    }
    return &g_kdFactory;
}

// EAX=name, ESI=out.
// FUNCTION: VC1 0x004c3890
void** QueryKdFactory(void** out, const char** name)
{
    GetKdFactory()->QueryNamed(out, name);
    return out;
}

// FUNCTION: VC1 0x004c9890
void DestroyKdFactory()
{
    if (g_kdFactory.capacity != 0) {
        free(g_kdFactory.begin);
    }
    g_kdFactory.Clear();
}

IUnknown* AcquireNamed(const char** factoryName, const char** iid)
{
    void* unknown = 0;
    IUnknown* typed;
    GetKdFactory()->QueryNamed(&unknown, factoryName);
    if (!unknown) {
        return 0;
    }
    typed = static_cast<IUnknown*>(
        static_cast<IUnknown*>(unknown)->QueryInterface(iid));
    if (typed) {
        typed->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
    return typed;
}

}
