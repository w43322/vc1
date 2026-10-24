#include "fixed_pool.h"

#include <stdlib.h>
#include <string.h>

#include <squirrel.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed454
SizeClassHeap g_sizeClassHeap;

}

// The game's sqmem.cpp (a host fork; the stock file is not built): Squirrel
// allocates from the size-class heap. Up to 0x80 bytes come from the pool of
// that size class, larger blocks from malloc; both are counted.
// ECX=size.
// FUNCTION: VC1 0x00497490
void* sq_vm_malloc(SQUnsignedInteger size)
{
    const unsigned index = vc1::SizeClassIndex(size);

    vc1::g_sizeClassHeap.allocatedBytes += size;
    if (index < vc1::kSizeClassCount) {
        return vc1::g_sizeClassHeap.pools[index]->malloc();
    }
    return malloc(size);
}

namespace vc1 {

// requested_size = index*4 and the default 32-chunk growth for every class,
// including the unused 0-byte slot. The global operator new is malloc, so a
// failed allocation leaves a null slot.
// FUNCTION: VC1 0x004b7bd0
void InitSizeClassPools()
{
    for (unsigned i = 0; i < kSizeClassCount; ++i) {
        g_sizeClassHeap.pools[i] = new FixedPool(i * 4);
    }
}

// ~pool is purge_memory: the block chain through the trailers, then
// next_size = start_size; operator delete frees the header.
// FUNCTION: VC1 0x004b7c60
void DestroySizeClassPools()
{
    for (unsigned i = 0; i < kSizeClassCount; ++i) {
        delete g_sizeClassHeap.pools[i];
    }
}

}

// Same size class: the block is kept. Two large blocks use realloc;
// otherwise a new block is taken, the old contents copied and the old block
// returned to its pool (or freed).
// EAX=size, stack p and oldsize.
// FUNCTION: VC1 0x004b7cc0
void* sq_vm_realloc(void* p, SQUnsignedInteger oldsize, SQUnsignedInteger size)
{
    const unsigned index = vc1::SizeClassIndex(size);
    const unsigned oldIndex = vc1::SizeClassIndex(oldsize);
    void* block;

    if (index >= vc1::kSizeClassCount && oldIndex >= vc1::kSizeClassCount) {
        vc1::g_sizeClassHeap.releasedBytes += oldsize;
        vc1::g_sizeClassHeap.allocatedBytes += size;
        return realloc(p, size);
    }
    if (index == oldIndex) {
        return p;
    }
    vc1::g_sizeClassHeap.allocatedBytes += size;
    if (index < vc1::kSizeClassCount) {
        block = vc1::g_sizeClassHeap.pools[index]->malloc();
    }
    else {
        block = malloc(size);
    }
    if (p) {
        memcpy(block, p, size < oldsize ? size : oldsize);
        vc1::g_sizeClassHeap.releasedBytes += oldsize;
        if (oldIndex < vc1::kSizeClassCount) {
            vc1::g_sizeClassHeap.pools[oldIndex]->free(p);
        }
        else {
            free(p);
        }
    }
    return block;
}

// Inlined at every caller (no address of its own).
void sq_vm_free(void* p, SQUnsignedInteger size)
{
    const unsigned index = vc1::SizeClassIndex(size);

    vc1::g_sizeClassHeap.releasedBytes += size;
    if (index < vc1::kSizeClassCount) {
        vc1::g_sizeClassHeap.pools[index]->free(p);
    }
    else {
        free(p);
    }
}
