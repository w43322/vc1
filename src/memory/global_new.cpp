#include <new>
#include <stdlib.h>

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
