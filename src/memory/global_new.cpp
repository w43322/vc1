#include <new>
#include <stdlib.h>
#include <string.h>

// The game replaces the global allocation operators with the CRT heap: every
// new / new[] in the original is an inlined call to malloc (0x0042f6bb), every
// delete / delete[] one to free (0x0042f5de), and the exe has no CRT
// operator new (no std::bad_alloc, no C++ exception runtime). They have no
// address of their own.

void* operator new(size_t size)
{
    return malloc(size);
}

void* operator new[](size_t size)
{
    return malloc(size);
}

void operator delete(void* p)
{
    free(p);
}

void operator delete[](void* p)
{
    free(p);
}

// zlib 1.1.3 is built with MY_ZCALLOC (cmake/libpng.cmake): the game supplies
// its allocators, set by inflateInit2_ (0x00448254) when the caller leaves
// zalloc/zfree empty. zcalloc is malloc plus a clear rather than calloc.
extern "C" {

// FUNCTION: VC1 0x004b4030
void zcfree(void* opaque, void* ptr)
{
    (void)opaque;
    free(ptr);
}

// FUNCTION: VC1 0x004b4040
void* zcalloc(void* opaque, unsigned items, unsigned size)
{
    void* const block = malloc(items * size);

    (void)opaque;
    memset(block, 0, items * size);
    return block;
}

}
