#ifndef VC1_KD_FACTORY_H
#define VC1_KD_FACTORY_H

#include "kd/iface/iunknown.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

class KdFactory;
struct FactoryEntry;
struct FactoryInsert;

typedef void** (*KdFactoryFn)(void** out);

// SIZE 0x08
struct FactoryEntry
{
    const char* name; // 0x00
    KdFactoryFn create; // 0x04
};

typedef char Vc1FactoryEntrySizeCheck[sizeof(FactoryEntry) == 0x08 ? 1 : -1];

__declspec(noinline) void __stdcall GrowCountedBuffer(void* self, int bytes, char exact) throw();
__declspec(noinline) void EraseCountedBytes(void* first, void* middle, void* last);

// Inferred name. Borrowed (begin, size) view; KdFactory adds ownership. Every
// inlined KdFactory destructor ends by storing this vftable.
// SIZE 0x0c
// VTABLE: VC1 0x004cfb84
class KdRange
{
public:
    KdRange() : begin(0), size(0) {}
    virtual ~KdRange() {}  // vtable+0x00
    virtual void Clear();  // vtable+0x04

    // Deleting destructors of this hierarchy call free() directly.
    static void operator delete(void* p) { free(p); }

    FactoryEntry* begin; // 0x04
    unsigned size;       // 0x08
};

typedef char Vc1KdRangeSizeCheck[sizeof(KdRange) == 0x0c ? 1 : -1];

// SYNTHETIC: VC1 0x00401f80
// vc1::KdRange::`scalar deleting destructor'

// Inferred name. Generic counted byte buffer (begin, size in bytes, capacity);
// g_kdFactory's sorted name/create table is one instance. Count() is the
// number of reserved bytes past size (0 here, 1 for the NUL of KdString).
// SIZE 0x10
// VTABLE: VC1 0x004cfb8c
class KdFactory : public KdRange
{
public:
    KdFactory() : capacity(0) {}
    virtual __forceinline ~KdFactory() throw() // vtable+0x00; inlined at every owner
    {
        Release();
    }
    virtual void Clear();     // vtable+0x04
    virtual unsigned Count(); // vtable+0x08
    void** __stdcall QueryNamed(void** out, const char** name);
    KdPtr<IUnknown> Create(const char* name);

    unsigned capacity;   // 0x0c

    // Out of the destructor body so Clear() stays a virtual call, as in the
    // original's inlined destructors.
    __forceinline void Release()
    {
        if (capacity != 0) {
            free(begin);
        }
        Clear();
    }
    // Sets size to bytes, reserving bytes + Count() through the vptr.
    __forceinline void ResizeBytes(unsigned bytes)
    {
        if (capacity < Count() + bytes) {
            GrowCountedBuffer(this, static_cast<int>(bytes), 1);
        }
        size = bytes;
    }

protected:
    // Reserves bytes (+Count() through the KdFactory vptr) and sets size;
    // inlined into derived constructors before they write their own vptr.
    explicit KdFactory(unsigned bytes) : capacity(0)
    {
        if (capacity < Count() + bytes) {
            GrowCountedBuffer(this, static_cast<int>(bytes), 1);
        }
        size = bytes;
    }
};

typedef char Vc1KdFactorySizeCheck[sizeof(KdFactory) == 0x10 ? 1 : -1];

// SYNTHETIC: VC1 0x00401f30
// vc1::KdFactory::`scalar deleting destructor'

// Inferred name. Owning IUnknown* array in a KdFactory buffer (no vftable of
// its own). The out-of-line destructor releases every slot up to capacity,
// then destroys the buffer.
// SIZE 0x10
struct KdRefList;
void ReserveRefs(unsigned count, KdRefList* list, unsigned char exact);
IUnknown** CopyRefs(IUnknown** first, IUnknown** last, IUnknown** dest);

struct KdRefList
{
    KdFactory items; // 0x00

    KdRefList() {}
    // Inlined where a list returned by value is copied (the script call
    // 0x00403f10): exact reserve, then counted copies of the slots.
    KdRefList(const KdRefList& source)
    {
        const unsigned count = source.items.size >> 2;
        const unsigned bytes = count * sizeof(IUnknown*);
        if (bytes > 0) {
            ReserveRefs(count, this, 1);
        }
        items.size = bytes;
        CopyRefs(reinterpret_cast<IUnknown**>(source.items.begin),
            reinterpret_cast<IUnknown**>(reinterpret_cast<unsigned char*>(source.items.begin)
                + source.items.size),
            reinterpret_cast<IUnknown**>(items.begin));
    }
    ~KdRefList() throw();
    void Reserve(unsigned count);
    void Resize(unsigned count)
    {
        unsigned bytes = count * sizeof(IUnknown*);
        if (items.capacity < bytes) {
            Reserve(count);
        }
        items.size = bytes;
    }
};

extern unsigned g_kdLogTag;
void AssignCountedString(void* self, const char* text, unsigned len) throw();

// Inferred name. Counted char string on KdFactory storage. With capacity 0 it
// borrows its bytes (a literal, or g_kdLogTag as the shared empty string).
// The original vftable reuses KdFactory's destructor and Clear.
// SIZE 0x10
// VTABLE: VC1 0x004d0d9c
class KdString : public KdFactory
{
public:
    KdString()
    {
        begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        size = 0;
    }
    KdString(const char* text);
    KdString(const char* text, unsigned length)
    {
        begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        size = 0;
        AssignCountedString(this, text, length);
    }
    // length copies of fill plus the NUL. Inlined into the Vector2/Vector3
    // ToString statics (0x00428780, 0x004289a0).
    __forceinline KdString(unsigned length, char fill) : KdFactory(length + 1)
    {
        size = length;
        memset(begin, fill, length);
        reinterpret_cast<char*>(begin)[length] = 0;
    }
    // Range copy: starts from a zeroed buffer, not g_kdLogTag.
    KdString(const char* first, const char* last)
    {
        AssignCountedString(this, first, static_cast<unsigned>(last - first));
    }
    KdString(const KdString& source);
    KdString& operator=(const char* text);
    unsigned Count();
    void Resize(unsigned length) throw();
    int Find(const KdString& needle) const;
    void Append(char c);
    void Append(const char* text, unsigned length);
    int FindLastOf(const KdString& set, unsigned pos) const;
    unsigned FormatV(const char* fmt, va_list args);
    unsigned Format(const char* fmt, ...);
    // Out of line only where the console calls them; inlined elsewhere.
    __declspec(noinline) void Reset();
    __declspec(noinline) unsigned TextLength() const;
    __declspec(noinline) KdString Substr(unsigned first, unsigned count) const;
    __declspec(noinline) void Replace(char* first, char* last, const char* text,
        const char* textEnd);
    __declspec(noinline) void Erase(unsigned pos, unsigned count);

    const char* c_str() const { return reinterpret_cast<const char*>(begin); }
    char* data() { return reinterpret_cast<char*>(begin); }
    void Assign(const KdString& source)
    {
        AssignCountedString(this, source.c_str(), source.size);
    }
    // Offset of c at or after start, or -1.
    int IndexOf(char c, unsigned start) const
    {
        const char* text = c_str();
        const char* last = text + size;
        const char* p = text + (start < size ? start : size);
        while (p != last && *p != c) {
            ++p;
        }
        return p < last ? static_cast<int>(p - text) : -1;
    }
    // Copy of [first, last), both clamped; last -1 means the end.
    KdString Slice(unsigned first, unsigned last) const
    {
        const char* text = c_str();
        const char* stop = last == static_cast<unsigned>(-1)
            ? text + size : text + (last < size ? last : size);
        return KdString(text + (first < size ? first : size), stop);
    }

private:
    KdString& operator=(const KdString&);
};

typedef char Vc1KdStringSizeCheck[sizeof(KdString) == 0x10 ? 1 : -1];

// Inferred name. KdString array in a KdFactory buffer (no vftable of its
// own); IFont::WrapText returns one by value.
// SIZE 0x10
struct KdStringList
{
    KdFactory items; // 0x00

    KdStringList() {}
    KdStringList(const KdStringList& source);
    ~KdStringList() throw();

    void Reserve(unsigned count, char exact) throw();
    // PushBack where the original keeps it out of line (the console).
    __declspec(noinline) void Append(const KdString& value);
    void PushBack(const KdString& value)
    {
        unsigned bytes = ((items.size >> 4) + 1) << 4;
        if (items.capacity < bytes) {
            Reserve(bytes >> 4, 0);
        }
        items.size = bytes;
        AssignCountedString(reinterpret_cast<unsigned char*>(items.begin) + bytes - 0x10,
            value.c_str(), value.size);
    }

private:
    KdStringList& operator=(const KdStringList&);
};

// Inferred name. Array of 4-byte PODs in a KdFactory buffer (no vftable of
// its own); the script natives take and return int / code-point lists by
// value. The copy constructor is out of line (0x00402480).
// SIZE 0x10
template <class T>
struct KdArray
{
    KdFactory items; // 0x00

    KdArray() {}
    explicit KdArray(unsigned count)
    {
        unsigned bytes = count * sizeof(T);
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 1);
        }
        items.size = bytes;
    }
    __declspec(noinline) KdArray(const KdArray& source)
    {
        unsigned bytes = source.items.size & ~(sizeof(T) - 1);
        const T* from;
        const T* last;
        T* to;
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 1);
        }
        to = Data();
        items.size = bytes;
        from = source.Data();
        last = reinterpret_cast<const T*>(reinterpret_cast<const char*>(from) + source.items.size);
        for (; from != last; ++from, ++to) {
            *to = *from;
        }
    }

    unsigned Count() const { return items.size / sizeof(T); }
    T* Data() const { return reinterpret_cast<T*>(items.begin); }
    T& operator[](unsigned i) const { return Data()[i]; }
    void PushBack(T value)
    {
        unsigned bytes = (Count() + 1) * sizeof(T);
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 0);
        }
        items.size = bytes;
        reinterpret_cast<T*>(reinterpret_cast<char*>(items.begin) + bytes)[-1] = value;
    }
    // PushBack where the original keeps it out of line (EAX=this, value by
    // reference).
    __declspec(noinline) void Append(const T& value)
    {
        unsigned bytes = (Count() + 1) * sizeof(T);
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 0);
        }
        items.size = bytes;
        reinterpret_cast<T*>(reinterpret_cast<char*>(items.begin) + bytes)[-1] = value;
    }
    // Out of line in the original (EAX=this, ESI=count).
    __declspec(noinline) void Resize(unsigned count)
    {
        unsigned bytes = count * sizeof(T);
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 1);
        }
        items.size = bytes;
    }
    // Exact grow to [first, last), then an element copy. Out of line in the
    // original (EAX=this, stack first, EBX=last).
    __declspec(noinline) void AssignRange(const T* first, const T* last)
    {
        unsigned bytes = static_cast<unsigned>(last - first) * sizeof(T);
        T* to;
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 1);
        }
        to = Data();
        items.size = bytes;
        for (; first != last; ++first, ++to) {
            *to = *first;
        }
    }
    // Exact grow, then an element copy.
    void Assign(const KdArray& source)
    {
        unsigned bytes = source.items.size / sizeof(T) * sizeof(T);
        const T* from;
        const T* last;
        T* to;
        if (items.capacity < bytes) {
            GrowCountedBuffer(&items, static_cast<int>(bytes), 1);
        }
        items.size = bytes;
        to = Data();
        from = source.Data();
        last = reinterpret_cast<const T*>(reinterpret_cast<const char*>(from) + source.items.size);
        for (; from != last; ++from, ++to) {
            *to = *from;
        }
    }

private:
    KdArray& operator=(const KdArray&);
};

// TEMPLATE: VC1 0x00402480
// ??0?$KdArray@I@vc1@@QAE@ABU01@@Z

// SIZE 0x08
struct FactoryInsert
{
    FactoryEntry* entry;    // 0x00
    unsigned char inserted; // 0x04
};

typedef char Vc1FactoryInsertSizeCheck[sizeof(FactoryInsert) == 0x08 ? 1 : -1];

extern KdFactory g_kdFactory;
extern unsigned g_kdFactoryGuard;
extern IUnknown* g_kdDefaultUnknown;

extern const char* g_kdNameDisplay;
extern const char* g_kdNameTexture;
extern const char* g_kdNameSoundDriver;
extern const char* g_kdNameSystem;
extern const char* g_kdNameConsole;
extern const char* g_kdNameFont;
extern const char* g_kdNameLayer;
extern const char* g_kdNameUpdateGroup;
extern const char* g_kdNameRenderGroup;
extern const char* g_kdNameInputGroup;
extern const char* g_kdNameResourceManager;
extern const char* g_kdNameCompoundObject;
extern const char* g_kdNameFile;
extern const char* g_kdNameScene;
extern const char* g_kdNameBlob;
extern const char* g_kdNameCamera;
extern const char* g_kdNameScriptedObject;
extern const char* g_kdNameTileManager;
extern const char* g_kdNameGameLevel;
extern const char* g_kdNameGameObject;
extern const char* g_kdNameTileAnimationData;
extern const char* g_kdNameTileLayer;
extern const char* g_kdNameTileObject;
extern const char* g_kdNamePassMap;
extern const char* g_kdNameGridRouter;
extern const char* g_kdNameTerrainBuilder;
extern const char* g_kdNameRoadRenderer;
extern const char* g_kdNameTileAnimation;
extern const char* g_kdNameBuildingBuilder;
extern const char* g_kdNameRoute;
extern const char* g_kdNameTransportMover;
extern const char* g_kdNameGridRectRenderer;
extern const char* g_kdNameObjectWatcher;
extern const char* g_kdNameClassificatedRenderer;
extern const char* g_kdNameTspSolver;
extern const char* g_kdNameGroupRenderer;
extern const char* g_kdNameRoadCellsRenderer;

void** CreateKdDisplay(void** out);
void** CreateKdSoundDriver(void** out);
void** CreateKdSystem(void** out);
void** CreateKdConsole(void** out);
void** CreateKdFont(void** out);
void** CreateKdLayer(void** out);
void** CreateKdUpdateGroup(void** out);
void** CreateKdRenderGroup(void** out);
void** CreateKdInputGroup(void** out);
void** CreateKdResourceManager(void** out);
void** CreateKdCompoundObject(void** out);
void** CreateKdFile(void** out);
void** CreateKdScene(void** out);
void** CreateKdBlob(void** out);
void** CreateKdCamera(void** out);
void** CreateKdScriptedObject(void** out);
void** CreateKdTileManager(void** out);
void** CreateKdGameLevel(void** out);
void** CreateKdGameObject(void** out);
void** CreateKdTileAnimationData(void** out);
void** CreateKdTileLayer(void** out);
void** CreateKdTileObject(void** out);
void** CreateKdPassMap(void** out);
void** CreateKdGridRouter(void** out);
void** CreateKdTerrainBuilder(void** out);
void** CreateKdRoadRenderer(void** out);
void** CreateKdTileAnimation(void** out);
void** CreateKdBuildingBuilder(void** out);
void** CreateKdRoute(void** out);
void** CreateKdTransportMover(void** out);
void** CreateKdGridRectRenderer(void** out);
void** CreateKdObjectWatcher(void** out);
void** CreateKdClassificatedRenderer(void** out);
void** CreateKdTspSolver(void** out);
void** CreateKdGroupRenderer(void** out);
void** CreateKdRoadCellsRenderer(void** out);

FactoryInsert* RegisterFactory(FactoryEntry* entry, FactoryInsert* out);
FactoryEntry* FindFactoryEntry(KdFactory* factory, const char** key);
void ReserveFactoryTable(unsigned newCount);
void ReverseCountedBytes(void* first, void* last);
__declspec(noinline) void EraseCountedBytes(void* first, void* middle, void* last);
__declspec(noinline) void* InsertCountedBytes(void* self, void* pos, int bytes);
void RegisterFactoryPair(const char** name, KdFactoryFn fn);
KdFactory* GetKdFactory();
void** QueryKdFactory(void** out, const char** name);
void DestroyKdFactory();
IUnknown* AcquireNamed(const char** factoryName, const char** iid);

}

#endif
