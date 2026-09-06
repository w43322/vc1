#include "fixed_pool.h"

#include <stdlib.h>

namespace vc1 {

SizeClassHeap g_sizeClassHeap;

static unsigned GreatestCommonDivisor(unsigned left, unsigned right)
{
    while (right) {
        const unsigned remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

// Inferred name. malloc is original 0x0042f6bb.
// FUNCTION: VC1 0x0040f1a0
void* FixedPool::Grow()
{
    const unsigned divisor = GreatestCommonDivisor(itemSize, sizeof(void*));
    const unsigned stride = itemSize / divisor * sizeof(void*);
    const unsigned bytes = growthCount * stride + 2 * sizeof(void*);
    unsigned char* const block = static_cast<unsigned char*>(malloc(bytes));
    if (!block) {
        return 0;
    }

    unsigned char* item = block + bytes - stride - 2 * sizeof(void*);
    *reinterpret_cast<void**>(item) = freeList;
    while (item != block) {
        unsigned char* const previous = item - stride;
        *reinterpret_cast<void**>(previous) = item;
        item = previous;
    }

    freeList = block;
    *reinterpret_cast<unsigned char**>(block + bytes - 2 * sizeof(void*)) = latestBlock;
    *reinterpret_cast<unsigned*>(block + bytes - sizeof(void*)) = latestBlockBytes;
    growthCount *= 2;
    latestBlock = block;
    latestBlockBytes = bytes;

    void* const result = freeList;
    freeList = *static_cast<void**>(result);
    return result;
}

// itemSize = index*4, growth 32 for every class including the unused 0-byte slot.
// FUNCTION: VC1 0x004b7bd0
void InitSizeClassPools()
{
    for (unsigned i = 0; i < kSizeClassCount; ++i) {
        FixedPool* pool = static_cast<FixedPool*>(malloc(sizeof(FixedPool)));
        if (pool) {
            pool->freeList = 0;
            pool->latestBlock = 0;
            pool->latestBlockBytes = 0;
            pool->itemSize = i * 4;
            pool->growthCount = kInitialGrowthCount;
            pool->initialGrowthCount = kInitialGrowthCount;
        }
        g_sizeClassHeap.pools[i] = pool;
    }
}

// Walk [block+size-8]/[block+size-4]; restore growthCount from +0x14; free header.
// FUNCTION: VC1 0x004b7c60
void DestroySizeClassPools()
{
    for (unsigned i = 0; i < kSizeClassCount; ++i) {
        FixedPool* const pool = g_sizeClassHeap.pools[i];
        if (!pool) {
            continue;
        }

        unsigned char* block = pool->latestBlock;
        unsigned bytes = pool->latestBlockBytes;
        if (block) {
            unsigned char* previous;
            do {
                unsigned char* const trailer = block + bytes;
                previous = *reinterpret_cast<unsigned char**>(trailer - 2 * sizeof(void*));
                bytes = *reinterpret_cast<unsigned*>(trailer - sizeof(void*));
                free(block);
                block = previous;
            } while (previous);
            pool->freeList = 0;
            pool->latestBlock = 0;
            pool->growthCount = pool->initialGrowthCount;
        }
        free(pool);
    }
}

}
