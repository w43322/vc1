#include "kd/factory.h"

#include "bindings/squirrel_object.h"

#include "kd/iface/iunknown.h"
#include "kdcore/platform.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ee2a8
KdFactory g_kdFactory;
// GLOBAL: VC1 0x004ee72c
unsigned g_kdFactoryGuard;
// GLOBAL: VC1 0x004ed6d4
KdRef<IUnknown> g_kdDefaultUnknown;

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

// EAX=dest, ECX=first, EBX=last. Counted copy of a ref range; returns the
// end of dest.
// FUNCTION: VC1 0x00403cb0
IUnknown** CopyRefs(IUnknown** first, IUnknown** last, IUnknown** dest)
{
    for (; first != last; ++first, ++dest) {
        if (*first) {
            (*first)->AddRef();
        }
        if (*dest) {
            (*dest)->Release();
        }
        *dest = *first;
    }
    return dest;
}

// ECX=count, EDX=list, AL=exact. KdRefList::Reserve with the exact flag;
// the new slots are nulled.
// FUNCTION: VC1 0x00403d90
void ReserveRefs(unsigned count, KdRefList* list, unsigned char exact)
{
    const unsigned old = list->items.capacity >> 2;
    IUnknown** slot;
    IUnknown** end;

    GrowCountedBuffer(&list->items, static_cast<int>(count * 4), static_cast<char>(exact));
    if ((list->items.capacity >> 2) > old) {
        slot = reinterpret_cast<IUnknown**>(list->items.begin) + old;
        end = reinterpret_cast<IUnknown**>(list->items.begin) + (list->items.capacity >> 2);
        for (; slot < end; ++slot) {
            if (slot) {
                *slot = 0;
            }
        }
    }
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
__declspec(noinline) void KdStringList::Reserve(unsigned count, char exact) throw()
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

// EDI=this, RET 4. PushBack out of line: grow by one string and copy value.
// FUNCTION: VC1 0x00408e00
void KdStringList::Append(const KdString& value)
{
    PushBack(value);
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

// EAX=this. Empties the string: a borrowed one points back at g_kdLogTag, an
// owned one keeps its buffer (reserving Count()) and stores the NUL.
// FUNCTION: VC1 0x0040e3d0
void KdString::Reset()
{
    if (capacity == 0) {
        begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        size = 0;
        return;
    }
    if (capacity < Count()) {
        GrowCountedBuffer(this, 0, 1);
    }
    size = 0;
    data()[0] = 0;
}

// ESI=this, EAX=text. Copies text (null means empty); SqPlus setVar's
// std::string case is the only caller.
// FUNCTION: VC1 0x0040e410
KdString& KdString::operator=(const char* text)
{
    if (!text) {
        text = reinterpret_cast<const char*>(&g_kdLogTag);
    }
    AssignCountedString(this, text, static_cast<unsigned>(strlen(text)));
    return *this;
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
        && !(entry->name < found->name)) {
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

// ECX=this. Bytes before the first NUL (or the whole size).
// FUNCTION: VC1 0x004184f0
unsigned KdString::TextLength() const
{
    const char* const text = c_str();
    const char* const last = text + size;
    const char* p = text;

    while (p != last && *p != 0) {
        ++p;
    }
    return static_cast<unsigned>(p - text);
}

// EAX=this, ESI=result, EDI=first, ECX=count (-1 for the rest). Copy of
// count bytes from first, both clamped to the size.
// FUNCTION: VC1 0x00418510
KdString KdString::Substr(unsigned first, unsigned count) const
{
    const char* const text = c_str();
    const char* const stop = count == static_cast<unsigned>(-1)
        ? text + size : text + (first + count < size ? first + count : size);
    return KdString(text + (first < size ? first : size), stop);
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

// EAX=last, ECX=first, EBX=middle. Byte rotate used to open/close a hole:
// VC9's bidirectional std::rotate (reverse both halves, swap inward, reverse
// the rest) over ReverseCountedBytes.
// FUNCTION: VC1 0x004b8830
void EraseCountedBytes(void* first, void* middle, void* last)
{
    unsigned char* f = static_cast<unsigned char*>(first);
    unsigned char* const m = static_cast<unsigned char*>(middle);
    unsigned char* l = static_cast<unsigned char*>(last);

    if (f == m || m == l) {
        return;
    }
    ReverseCountedBytes(f, m);
    ReverseCountedBytes(m, l);
    while (f != m && m != l) {
        const unsigned char t = *f;
        *f++ = *--l;
        *l = t;
    }
    if (f == m) {
        ReverseCountedBytes(m, l);
    }
    else {
        ReverseCountedBytes(f, m);
    }
}

// EDI=counted buffer (vptr + begin/size/capacity). Aligns new size up to 0x40, CRT realloc.
// Callee pops bytes and exact (RET 8).
// FUNCTION: VC1 0x004b8930
void __stdcall GrowCountedBuffer(void* self, int bytes, char exact) throw()
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

// EAX=this, ESI=length. Keeps the bytes, NUL-terminates at length; an empty
// borrowed string goes back to g_kdLogTag.
// FUNCTION: VC1 0x004b8b40
void KdString::Resize(unsigned length) throw()
{
    if ((capacity | length) == 0) {
        begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        size = 0;
        return;
    }
    if (capacity < Count() + length) {
        GrowCountedBuffer(this, static_cast<int>(length), 1);
    }
    size = length;
    reinterpret_cast<char*>(begin)[length] = 0;
}

// EAX=this, stdcall source RET 4. A borrowed source (capacity 0) is shared;
// an owned one is copied with its NUL.
// FUNCTION: VC1 0x004b8bf0
__declspec(noinline) KdString::KdString(const KdString& source)
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

// ECX=this, EAX=args, fmt on the stack, RET 4. Grows and reformats until the
// text fits below capacity, then truncates to it. Returns _vsnprintf's count
// clamped to capacity.
// FUNCTION: VC1 0x004b8c90
unsigned KdString::FormatV(const char* fmt, va_list args)
{
    unsigned written = size;
    unsigned length;

    do {
        GrowCountedBuffer(this, static_cast<int>(written), 1);
        written = static_cast<unsigned>(_vsnprintf(data(), capacity, fmt, args));
        if (written >= capacity) {
            written = capacity;
        }
    } while (written > capacity - (capacity != 0));
    length = written;
    if (length >= capacity - (capacity != 0)) {
        length = capacity - (capacity != 0);
    }
    if ((capacity | length) == 0) {
        begin = reinterpret_cast<FactoryEntry*>(&g_kdLogTag);
        size = 0;
        return written;
    }
    if (capacity < Count() + length) {
        GrowCountedBuffer(this, static_cast<int>(length), 1);
    }
    size = length;
    data()[length] = 0;
    return written;
}

// EAX=needle, haystack (this) on the stack, RET 4. Horspool-style: scans for
// the needle's last byte, compares the whole needle with MemCompare and skips
// by that byte's distance to its previous occurrence. -1 when absent.
// FUNCTION: VC1 0x004b8d30
int KdString::Find(const KdString& needle) const
{
    unsigned length;
    unsigned tail;
    unsigned i;
    unsigned j;
    unsigned skip;
    char last;
    const char* pattern;
    const char* text;
    const char* end;
    const char* p;
    const char* q;

    length = needle.size;
    if (length == 0 || size < length) {
        return -1;
    }
    pattern = needle.c_str();
    last = pattern[length - 1];
    tail = length - 1;
    i = tail;
    do {
        j = i;
        i -= 1;
    } while (j != 0 && pattern[i] != last);
    skip = tail - i;
    text = c_str();
    end = text + size;
    p = text + (0 < size ? 0 : size) + tail;
    if (p >= end) {
        return -1;
    }
    for (;;) {
        q = p;
        while (q != text + size && *q != last) {
            ++q;
        }
        if (q >= end) {
            return -1;
        }
        if (MemCompare(q - tail, pattern, length) == 0) {
            return static_cast<int>(q - text - tail);
        }
        p = q + skip;
        if (p >= end) {
            return -1;
        }
    }
}

// ECX=first, EAX=last, EDX=textEnd, stack this and text, RET 8. Replaces
// [first, last) with [text, textEnd), moving the tail as needed.
// FUNCTION: VC1 0x004b8e60
void KdString::Replace(char* first, char* last, const char* text, const char* textEnd)
{
    const unsigned oldLength = static_cast<unsigned>(last - first);
    unsigned length = static_cast<unsigned>(textEnd - text);

    if (oldLength > length) {
        const unsigned drop = oldLength - length;
        char* const before = data();
        EraseCountedBytes(first, first + drop, data() + size);
        size -= drop;
        first += data() - before;
    }
    else if (oldLength < length) {
        first = static_cast<char*>(InsertCountedBytes(this, first,
            static_cast<int>(length - oldLength)));
    }
    if (length == 1) {
        *first = *text;
    }
    else {
        for (; length != 0; --length) {
            *first++ = *text++;
        }
    }
    data()[size] = 0;
}

// EAX=this, RET 4. Resize(size + 1) inlined, then the new last byte.
// FUNCTION: VC1 0x004b8f70
void KdString::Append(char c)
{
    Resize(size + 1);
    data()[size - 1] = c;
}

// ECX=this, EAX=length, RET 4 (text). Trailing NULs of text are dropped.
// FUNCTION: VC1 0x004b8fe0
void KdString::Append(const char* text, unsigned length)
{
    char* out;

    while (length != 0 && text[length - 1] == 0) {
        length -= 1;
    }
    Resize(size + length);
    out = data() + size - length;
    for (; length != 0; length -= 1) {
        *out = *text;
        out += 1;
        text += 1;
    }
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

// ECX=this; fmt and the varargs on the stack.
// FUNCTION: VC1 0x004b9130
unsigned KdString::Format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    return FormatV(fmt, args);
}

// EDX=this, ECX=set, EAX=pos. Last index at or before pos whose byte is in
// set, or -1.
// FUNCTION: VC1 0x004b9140
int KdString::FindLastOf(const KdString& set, unsigned pos) const
{
    int i;

    i = static_cast<int>(size - 1);
    if (pos < static_cast<unsigned>(i)) {
        i = static_cast<int>(pos);
    }
    for (; i >= 0; --i) {
        if (set.IndexOf(c_str()[i], 0) != -1) {
            return i;
        }
    }
    return -1;
}

// ESI=this, ECX=pos, EAX=count (-1 for the rest). Removes count bytes at pos
// (clamped) and stores the NUL.
// FUNCTION: VC1 0x004b91b0
void KdString::Erase(unsigned pos, unsigned count)
{
    char* const text = data();
    char* const first = text + (pos < size ? pos : size);

    if (count == static_cast<unsigned>(-1)) {
        count = static_cast<unsigned>(text - first) + size;
    }
    EraseCountedBytes(first, first + count, text + size);
    size -= count;
    data()[size] = 0;
}

// EAX=this, EBX=name, out on the stack, RET 4. Linear strcmp search of the
// table by name text (getComponent); g_kdDefaultUnknown if missing.
// Inferred name.
// FUNCTION: VC1 0x004bccf0
KdPtr<IUnknown> KdFactory::Create(const char* name)
{
    FactoryEntry* entry = begin;
    FactoryEntry* const end = reinterpret_cast<FactoryEntry*>(
        reinterpret_cast<char*>(begin) + size);

    for (; entry != end; ++entry) {
        if (strcmp(entry->name, name) == 0) {
            void* created;
            KdPtr<IUnknown> result(static_cast<IUnknown*>(*entry->create(&created)));
            if (created) {
                static_cast<IUnknown*>(created)->Release();
            }
            return result;
        }
    }
    return KdPtr<IUnknown>(g_kdDefaultUnknown);
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

}
