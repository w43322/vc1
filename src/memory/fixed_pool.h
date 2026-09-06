#ifndef VC1_MEMORY_FIXED_POOL_H
#define VC1_MEMORY_FIXED_POOL_H

namespace vc1 {

// Original size-class heap at 0x004ed454:
//   +0x00 allocatedBytes
//   +0x04 pools[0x21] starting at 0x004ed458; index i has itemSize i*4
//   +0x88 releasedBytes at 0x004ed4dc
// Color (4 bytes) uses pools[1] / 0x004ed45c; Vector2 (8 bytes) uses pools[2] / 0x004ed460.
// Other 4-byte clients also use slot 1 (e.g. 0x0041fdd0). Names are inferred.
enum { kSizeClassCount = 0x21 };
enum { kInitialGrowthCount = 32 };

struct FixedPool
{
    void* freeList;                 // +0x00
    unsigned char* latestBlock;     // +0x04
    unsigned latestBlockBytes;      // +0x08
    unsigned itemSize;              // +0x0c
    unsigned growthCount;           // +0x10, doubled by Grow
    unsigned initialGrowthCount;    // +0x14, restored by Destroy; both set to 32 in Init

    // 0x0040f1a0; this in EBX. stride = lcm(itemSize, 4); block = growth*stride+8.
    void* Grow();

    void* Allocate()
    {
        void* item = freeList;
        if (item) {
            freeList = *static_cast<void**>(item);
        }
        else {
            item = Grow();
        }
        return item;
    }

    void Release(void* item)
    {
        if (item) {
            *static_cast<void**>(item) = freeList;
            freeList = item;
        }
    }
};

typedef char Vc1FixedPoolSizeCheck[sizeof(FixedPool) == 0x18 ? 1 : -1];

struct SizeClassHeap
{
    unsigned allocatedBytes;
    FixedPool* pools[kSizeClassCount];
    unsigned releasedBytes;
};

extern SizeClassHeap g_sizeClassHeap;

// 0x004b7bd0; malloc(0x18) per slot. Called from 0x00463b40.
void InitSizeClassPools();
// 0x004b7c60; frees block chain then the header; does not null the table. Called from 0x004c38b0.
void DestroySizeClassPools();

inline unsigned SizeClassIndex(unsigned bytes)
{
    return (bytes + 3) >> 2;
}

}

#endif
