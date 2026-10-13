#include "kd/factory.h"

#include "kd/iface/iunknown.h"
#include "host/runtime.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ee2a8
KdFactory g_kdFactory;
// GLOBAL: VC1 0x004ee72c
unsigned g_kdFactoryGuard;
// GLOBAL: VC1 0x004ed6d4
IUnknown* g_kdDefaultUnknown;

// FUNCTION: VC1 0x00401f70
void KdFactory::Clear()
{
    begin = 0;
    size = 0;
    capacity = 0;
}

// FUNCTION: VC1 0x00401fa0
void KdRange::Clear()
{
    begin = 0;
    size = 0;
}

// MOV EAX, 1; RET. KdString vtable+0x08: one byte reserved for the NUL.
// FUNCTION: VC1 0x00403aa0
unsigned KdString::Count()
{
    return 1;
}

// EBX=this. Destroys every KdString up to capacity through its virtual
// destructor; the KdFactory member destructor is inlined after it.
// FUNCTION: VC1 0x00408d40
KdStringList::~KdStringList() throw()
{
    KdString* item;
    KdString* end;

    if (items.capacity) {
        item = reinterpret_cast<KdString*>(items.begin);
        end = item + (items.capacity >> 4);
        for (; item < end; ++item) {
            item->~KdString();
        }
    }
}

// EAX=count, EDX=this, CL=exact. Grows to count strings and default-constructs
// the new slots.
// FUNCTION: VC1 0x00408da0
void KdStringList::Reserve(unsigned count, char exact) throw()
{
    unsigned old;
    KdString* slot;
    KdString* end;

    old = items.capacity >> 4;
    GrowCountedBuffer(&items, static_cast<int>(count << 4), exact);
    if ((items.capacity >> 4) > old) {
        slot = reinterpret_cast<KdString*>(items.begin) + old;
        end = reinterpret_cast<KdString*>(items.begin) + (items.capacity >> 4);
        for (; slot < end; ++slot) {
            new (slot) KdString();
        }
    }
}

// EAX=exclusive end, ECX=first. Byte reverse of [first, last).
// FUNCTION: VC1 0x0040e390
void ReverseCountedBytes(void* first, void* last)
{
    unsigned char* lo;
    unsigned char* hi;
    unsigned char tmp;
    int span;

    lo = static_cast<unsigned char*>(first);
    hi = static_cast<unsigned char*>(last);
    hi -= 1;
    span = static_cast<int>(hi - lo);
    if (span <= 0) {
        return;
    }
    do {
        tmp = *lo;
        *lo = *hi;
        *hi = tmp;
        hi -= 1;
        lo += 1;
        span = static_cast<int>(hi - lo);
    } while (span > 0);
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
// EAX=source, EDI=this. Exact-size copy; every string is copied, not shared.
// FUNCTION: VC1 0x00419e70
KdStringList::KdStringList(const KdStringList& source)
{
    unsigned count;
    unsigned bytes;
    KdString* to;
    const KdString* from;
    const KdString* last;

    count = source.items.size >> 4;
    bytes = count << 4;
    if (items.capacity < bytes) {
        Reserve(count, 1);
    }
    to = reinterpret_cast<KdString*>(items.begin);
    items.size = bytes;
    from = reinterpret_cast<const KdString*>(source.items.begin);
    last = reinterpret_cast<const KdString*>(
        reinterpret_cast<const unsigned char*>(source.items.begin) + source.items.size);
    for (; from != last; ++from, ++to) {
        AssignCountedString(to, from->c_str(), from->size);
    }
}

// FUNCTION: VC1 0x0041adb0 FOLDED
unsigned KdFactory::Count()
{
    return 0;
}

// EAX=last, ECX=first, EBX=middle. Byte rotate used to open/close a hole.
// FUNCTION: VC1 0x004b8830
void EraseCountedBytes(void* first, void* middle, void* last)
{
    unsigned char* esi;
    unsigned char* edi;
    unsigned char* ebx;
    unsigned char left;
    unsigned char right;

    if (first == middle || middle == last) {
        return;
    }
    ReverseCountedBytes(first, middle);
    ReverseCountedBytes(middle, last);
    esi = static_cast<unsigned char*>(first);
    edi = static_cast<unsigned char*>(last);
    ebx = static_cast<unsigned char*>(middle);
    while (ebx != edi) {
        left = *esi;
        right = *(edi - 1);
        edi -= 1;
        *esi = right;
        esi += 1;
        *edi = left;
        if (esi == ebx) {
            ReverseCountedBytes(esi, edi);
            return;
        }
    }
    if (esi == ebx) {
        ReverseCountedBytes(esi, edi);
        return;
    }
    ReverseCountedBytes(esi, ebx);
}

// EDI=counted buffer (vptr + begin/size/capacity). Aligns new size up to 0x40, CRT realloc.
// FUNCTION: VC1 0x004b8930
void GrowCountedBuffer(void* self, int bytes, char exact) throw()
{
    KdFactory* factory = static_cast<KdFactory*>(self);
    typedef unsigned (__thiscall *FnCount)(void* object);
    typedef void (__thiscall *FnClear)(void* object);
    unsigned used;
    unsigned needed;
    unsigned aligned;
    void* old;
    void* grown;
    unsigned copy;
    unsigned saved;

    used = reinterpret_cast<FnCount>((*reinterpret_cast<void***>(self))[2])(self);
    needed = static_cast<unsigned>(bytes) + used;
    if (needed <= factory->capacity) {
        return;
    }

    old = factory->capacity == 0 ? 0 : factory->begin;
    aligned = needed;
    if ((needed & 0x3f) != 0) {
        aligned = (needed - (needed & 0x3f)) + 0x40;
    }
    if (exact == 0) {
        needed = aligned;
    }
    grown = realloc(old, needed);
    if (old == 0 && factory->begin != 0 && grown) {
        copy = factory->size + 1;
        if (needed <= copy) {
            copy = needed;
        }
        memcpy(grown, factory->begin, copy);
    }
    saved = factory->size;
    reinterpret_cast<FnClear>((*reinterpret_cast<void***>(self))[1])(self);
    factory->size = saved;
    factory->begin = static_cast<FactoryEntry*>(grown);
    factory->capacity = needed;
}

// EAX=factory, stdcall pos + byte count RET 8. Grows then rotates a hole at pos.
// FUNCTION: VC1 0x004b8a20
void* InsertCountedBytes(void* self, void* pos, int bytes)
{
    KdFactory* factory = static_cast<KdFactory*>(self);
    typedef unsigned (__thiscall *FnCount)(void* object);
    unsigned offset;
    unsigned newSize;
    unsigned used;
    unsigned char* begin;
    unsigned char* first;
    unsigned char* middle;
    unsigned char* last;

    offset = static_cast<unsigned>(
        static_cast<unsigned char*>(pos)
        - reinterpret_cast<unsigned char*>(factory->begin));
    newSize = factory->size + static_cast<unsigned>(bytes);
    used = reinterpret_cast<FnCount>((*reinterpret_cast<void***>(self))[2])(self);
    if (factory->capacity < used + newSize) {
        GrowCountedBuffer(self, static_cast<int>(newSize), 0);
    }
    begin = reinterpret_cast<unsigned char*>(factory->begin);
    last = begin + newSize;
    middle = last - static_cast<unsigned>(bytes);
    first = begin + offset;
    factory->size = newSize;
    EraseCountedBytes(first, middle, last);
    return reinterpret_cast<unsigned char*>(factory->begin) + offset;
}

// EAX=this, ECX=text. Borrows text (capacity stays 0); null means empty.
// FUNCTION: VC1 0x004b8af0
KdString::KdString(const char* text)
{
    if (!text) {
        text = reinterpret_cast<const char*>(&g_kdLogTag);
    }
    begin = reinterpret_cast<FactoryEntry*>(const_cast<char*>(text));
    size = static_cast<unsigned>(strlen(text));
}

// EAX=this, stdcall source RET 4. A borrowed source (capacity 0) is shared;
// an owned one is copied with its NUL.
// FUNCTION: VC1 0x004b8bf0
KdString::KdString(const KdString& source)
    : KdFactory(source.capacity != 0 ? source.size + 1 : 0)
{
    unsigned left;
    unsigned char* out;
    const unsigned char* in;

    if (source.capacity == 0) {
        size = source.size;
        begin = source.begin;
        return;
    }
    out = reinterpret_cast<unsigned char*>(begin);
    in = reinterpret_cast<const unsigned char*>(source.begin);
    for (left = size; left != 0; left -= 1) {
        *out = *in;
        out += 1;
        in += 1;
    }
    size -= 1;
}

// ECX=counted buffer, EAX=length, EBX=source. Empty string uses g_kdLogTag.
// FUNCTION: VC1 0x004b9090
void AssignCountedString(void* self, const char* text, unsigned len) throw()
{
    KdFactory* dst = static_cast<KdFactory*>(self);
    typedef unsigned (__thiscall *FnCount)(void* object);
    unsigned n;
    unsigned used;
    unsigned char* out;
    const char* src;

    n = len;
    src = text;
    if (n != 0) {
        while (src[n - 1] == 0) {
            n -= 1;
            if (n == 0) {
                break;
            }
        }
    }
    if ((dst->capacity | n) == 0) {
        dst->begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        dst->size = n;
    }
    else {
        used = reinterpret_cast<FnCount>((*reinterpret_cast<void***>(self))[2])(self);
        if (dst->capacity < used + n) {
            GrowCountedBuffer(self, static_cast<int>(n), 1);
        }
        dst->size = n;
        reinterpret_cast<unsigned char*>(dst->begin)[n] = 0;
    }
    if (text) {
        out = reinterpret_cast<unsigned char*>(dst->begin);
        src = text;
        used = n;
        for (; used != 0; used -= 1) {
            *out = static_cast<unsigned char>(*src);
            out += 1;
            src += 1;
        }
    }
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
