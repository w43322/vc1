#ifndef VC1_MEMORY_FIXED_POOL_H
#define VC1_MEMORY_FIXED_POOL_H

#include <stddef.h>

#include <boost/pool/pool.hpp>

namespace vc1 {

// Original size-class heap at 0x004ed454:
//   +0x00 allocatedBytes
//   +0x04 pools[0x21] starting at 0x004ed458; index i has itemSize i*4
//   +0x88 releasedBytes at 0x004ed4dc
// Color (4 bytes) uses pools[1] / 0x004ed45c; Vector2 (8) pools[2] / 0x004ed460;
// Vector3 (12) pools[3] / 0x004ed464; Vector4/Quaternion/Rect/RectF (16) pools[4] / 0x004ed468;
// Matrix3 (36) pools[9] / 0x004ed47c; Matrix4 (64) pools[16] / 0x004ed498.
// Other 4-byte clients also use slot 1 (e.g. 0x0041fdd0). Names are inferred.
enum { kSizeClassCount = 0x21 };

// One size class: boost::pool (Boost 1.41.0, third_party/boost_1_41_0), 0x18
// bytes: first (free list), list.ptr / list.sz (newest block and its size;
// each block's trailer links the previous one), requested_size = index*4,
// next_size (doubled per block) and start_size = 32. malloc() is inlined
// everywhere; malloc_need_resize is 0x0040f1a0 (this in EBX).
typedef boost::pool<boost::default_user_allocator_malloc_free> FixedPool;

typedef char Vc1FixedPoolSizeCheck[sizeof(FixedPool) == 0x18 ? 1 : -1];

struct SizeClassHeap
{
    unsigned allocatedBytes;
    FixedPool* pools[kSizeClassCount];
    unsigned releasedBytes;
};

extern SizeClassHeap g_sizeClassHeap;

// 0x004b7bd0; new FixedPool(i * 4) per slot. Called from 0x00463b40.
void InitSizeClassPools();
// 0x004b7c60; deletes every pool (purge_memory); does not null the table. Called from 0x004c38b0.
void DestroySizeClassPools();

inline unsigned SizeClassIndex(unsigned bytes)
{
    return (bytes + 3) >> 2;
}

// Class-level new/delete of the script value types on the size-class heap
// (SqPlus ConstructReleaseClass<T> news and deletes them). A macro rather
// than a base so the types stay aggregates.
#define VC1_SIZE_CLASS_NEW(T) \
    static void* operator new(size_t) \
    { \
        g_sizeClassHeap.allocatedBytes += sizeof(T); \
        return g_sizeClassHeap.pools[SizeClassIndex(sizeof(T))]->malloc(); \
    } \
    static void operator delete(void* p) \
    { \
        if (p) { \
            g_sizeClassHeap.pools[SizeClassIndex(sizeof(T))]->free(p); \
            g_sizeClassHeap.releasedBytes += sizeof(T); \
        } \
    }

}

#endif
