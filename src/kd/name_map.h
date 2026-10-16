#ifndef VC1_KD_NAME_MAP_H
#define VC1_KD_NAME_MAP_H

#include "kd/component_group.h"
#include "kd/factory.h"

#include <new>

namespace vc1 {

int MemCompare(const void* left, const void* right, unsigned size);

// Inferred name. Array of 0x10-byte KdFactory-based elements (KdArray<int>)
// in a KdFactory buffer. Unlike KdArray, every slot up to capacity is
// constructed (Reserve) and destroyed (destructor).
// SIZE 0x10
template <class T>
struct KdObjArray
{
    KdFactory items; // 0x00

    KdObjArray() {}
    explicit KdObjArray(unsigned count)
    {
        unsigned bytes = count * sizeof(T);
        if (items.capacity < bytes) {
            Reserve(bytes / sizeof(T));
        }
        items.size = bytes;
    }
    KdObjArray(const KdObjArray& source)
    {
        unsigned bytes = source.items.size / sizeof(T) * sizeof(T);
        if (items.capacity < bytes) {
            Reserve(bytes / sizeof(T));
        }
        items.size = bytes;
        CopyRange(source.Begin(), source.End(), Begin());
    }
    ~KdObjArray() throw()
    {
        if (items.capacity) {
            DestroyRange(Begin(), Begin() + items.capacity / sizeof(T));
        }
    }
    KdObjArray& operator=(const KdObjArray& source)
    {
        unsigned bytes = source.items.size / sizeof(T) * sizeof(T);
        if (items.capacity < bytes) {
            Reserve(bytes / sizeof(T));
        }
        items.size = bytes;
        CopyRange(source.Begin(), source.End(), Begin());
        return *this;
    }

    unsigned Count() const { return items.size / sizeof(T); }
    T* Begin() const { return reinterpret_cast<T*>(items.begin); }
    T* End() const
    {
        return reinterpret_cast<T*>(reinterpret_cast<char*>(items.begin) + items.size);
    }
    T& operator[](unsigned i) const { return Begin()[i]; }

    // Exact grow to count elements; the new slots are default-constructed.
    __declspec(noinline) void Reserve(unsigned count)
    {
        unsigned old = items.capacity / sizeof(T);
        T* slot;
        T* end;

        GrowCountedBuffer(&items, static_cast<int>(count * sizeof(T)), 1);
        if (items.capacity / sizeof(T) > old) {
            slot = Begin() + old;
            end = Begin() + items.capacity / sizeof(T);
            for (; slot < end; ++slot) {
                if (slot) {
                    ::new (static_cast<void*>(slot)) T();
                }
            }
        }
    }
    // Inline append: grow with the KdFactory slack, default-construct the new
    // slots, then assign the last element.
    __forceinline void PushBack(const T& value)
    {
        unsigned bytes = (Count() + 1) * sizeof(T);
        if (items.capacity < bytes) {
            unsigned old = items.capacity / sizeof(T);
            T* slot;
            T* end;

            GrowCountedBuffer(&items, static_cast<int>(bytes), 0);
            if (items.capacity / sizeof(T) > old) {
                slot = Begin() + old;
                end = Begin() + items.capacity / sizeof(T);
                for (; slot < end; ++slot) {
                    if (slot) {
                        ::new (static_cast<void*>(slot)) T();
                    }
                }
            }
        }
        items.size = bytes;
        End()[-1] = value;
    }
    // Grow to count elements, exact or with the KdFactory slack.
    __declspec(noinline) void Reserve(unsigned count, unsigned char exact)
    {
        unsigned old = items.capacity / sizeof(T);
        T* slot;
        T* end;

        GrowCountedBuffer(&items, static_cast<int>(count * sizeof(T)), exact);
        if (items.capacity / sizeof(T) > old) {
            slot = Begin() + old;
            end = Begin() + items.capacity / sizeof(T);
            for (; slot < end; ++slot) {
                if (slot) {
                    ::new (static_cast<void*>(slot)) T();
                }
            }
        }
    }
    static __declspec(noinline) void DestroyRange(T* first, T* last)
    {
        for (; first < last; ++first) {
            first->~T();
        }
    }
    static __declspec(noinline) T* CopyRange(const T* first, const T* last, T* dest)
    {
        for (; first != last; ++first, ++dest) {
            dest->Assign(*first);
        }
        return dest;
    }
};

// Inferred name. One record of a KdNameMap: the key is compared like the
// component-group records (RecordEq / LookupCountedRecord).
template <class V>
struct KdNameRecord
{
    KdString key; // 0x00
    V value;      // 0x10

    KdNameRecord() {}
    KdNameRecord(const KdString& name, const V& source) : key(name), value(source) {}
    KdNameRecord& operator=(const KdNameRecord& source)
    {
        key.Assign(source.key);
        value = source.value;
        return *this;
    }
};

// Inferred name. Name-sorted KdNameRecord<V> array in a KdFactory buffer.
// Every slot up to capacity holds a constructed record; an insert rotates the
// spare record at the end into place (InsertCountedBytes) and assigns it.
// SIZE 0x10
template <class V>
struct KdNameMap
{
    typedef KdNameRecord<V> Record;

    KdFactory items; // 0x00

    KdNameMap() {}
    ~KdNameMap() throw()
    {
        Record* it;
        Record* end;
        if (items.capacity) {
            it = Begin();
            end = it + items.capacity / sizeof(Record);
            for (; it < end; ++it) {
                it->~Record();
            }
        }
    }

    unsigned Count() const { return items.size / sizeof(Record); }
    Record* Begin() const { return reinterpret_cast<Record*>(items.begin); }
    Record* End() const
    {
        return reinterpret_cast<Record*>(reinterpret_cast<char*>(items.begin) + items.size);
    }

    void Reserve(unsigned count, char exact)
    {
        unsigned old = items.capacity / sizeof(Record);
        GrowCountedBuffer(&items, static_cast<int>(count * sizeof(Record)), exact);
        if (items.capacity / sizeof(Record) > old) {
            ConstructRange(Begin() + old, Begin() + items.capacity / sizeof(Record));
        }
    }
    static void ConstructRange(Record* first, Record* last)
    {
        for (; first < last; ++first) {
            if (first) {
                new (first) Record();
            }
        }
    }

    // First record whose key is not below name.
    Record* LowerBound(const KdString& name) const
    {
        Record* lo = Begin();
        Record* hi = End();
        Record* mid;
        unsigned n;
        int cmp;
        int lenCmp;

        if (lo == hi) {
            return lo;
        }
        while (lo != hi) {
            mid = lo + (hi - lo) / 2;
            lenCmp = static_cast<int>(mid->key.size) - static_cast<int>(name.size);
            lenCmp = (lenCmp > 0) - (lenCmp < 0);
            n = mid->key.size;
            if (name.size < n) {
                n = name.size;
            }
            cmp = MemCompare(mid->key.c_str(), name.c_str(), n);
            if (cmp == 0) {
                cmp = lenCmp;
            }
            if (cmp < 0) {
                lo = mid + 1;
            }
            else {
                hi = mid;
            }
        }
        return lo;
    }
    Record* Find(const KdString& name) const
    {
        Record* pos = LowerBound(name);
        Record* end = End();
        if (pos < end && RecordEq(const_cast<KdString*>(&name), pos)) {
            pos = end;
        }
        return pos;
    }
    Record* InsertSlot(Record* pos)
    {
        unsigned offset = static_cast<unsigned>(
            reinterpret_cast<char*>(pos) - reinterpret_cast<char*>(items.begin));
        Reserve(Count() + 1, 0);
        return static_cast<Record*>(InsertCountedBytes(
            &items, reinterpret_cast<char*>(items.begin) + offset, sizeof(Record)));
    }
    Record* Insert(Record* pos, const Record& source)
    {
        Record* slot = InsertSlot(pos);
        *slot = source;
        return slot;
    }
    // Inserts source unless its key is already present.
    void InsertUnique(const Record& source)
    {
        Record* pos = LowerBound(source.key);
        if (pos == End() || RecordEq(const_cast<KdString*>(&source.key), pos)) {
            Insert(pos, source);
        }
    }
    V& operator[](const KdString& name)
    {
        Record* pos = LowerBound(name);
        if (pos == End() || RecordEq(const_cast<KdString*>(&name), pos)) {
            pos = Insert(pos, Record(name, V()));
        }
        return pos->value;
    }

    KdNameMap(const KdNameMap& source)
    {
        Record* to;
        Record* from;
        Reserve(source.Count(), 1);
        items.size = source.items.size;
        to = Begin();
        for (from = source.Begin(); from != source.End(); ++from, ++to) {
            *to = *from;
        }
    }

private:
    KdNameMap& operator=(const KdNameMap&);
};

}

#endif
