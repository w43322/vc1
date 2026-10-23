#ifndef VC1_PYRO_STRING_H
#define VC1_PYRO_STRING_H

#include <stdlib.h>

// The Pyro library allocates straight from malloc (no CRT operator new in
// the original); array news keep the compiler's count cookie.
#define VC1_PYRO_HEAP \
    static void* operator new(size_t size) { return malloc(size); } \
    static void* operator new[](size_t size) { return malloc(size); } \
    static void operator delete(void* p) { free(p); } \
    static void operator delete[](void* p) { free(p); }

namespace vc1 {

// Inferred names. The statically linked Pyro particle library (Fenomen Games,
// proprietary) keeps strings as an MFC CString-style shared buffer: the data
// pointer is preceded by a 0x0c-byte header, the empty string points at the
// static sentinel 0x004e9a34 (header 0x004e9a28, refs -1).
struct PyroStringData
{
    int refs;     // -0x0c
    int length;   // -0x08
    int capacity; // -0x04
    char* Data() { return reinterpret_cast<char*>(this + 1); }
};

struct PyroStringEmpty
{
    PyroStringData header;
    char data[4];
};

extern PyroStringEmpty g_pyroStringEmpty;
extern char* g_pyroStringEmptyData;

// SIZE 0x04
struct PyroString
{
    char* data;

    PyroStringData* Header() const { return reinterpret_cast<PyroStringData*>(data) - 1; }
    int GetLength() const { return Header()->length; }
    const char* c_str() const { return data; }

    void InitFrom(const char* text);
    void CopyBeforeWrite();
    void AllocBeforeWrite(int length);
    PyroString& Assign(const char* text);
    void ConcatCopy(int length1, const char* text1, int length2, const char* text2);

    void Release()
    {
        if (data != g_pyroStringEmpty.data) {
            if (--Header()->refs <= 0) {
                free(Header());
            }
        }
    }
};

typedef char Vc1PyroStringSizeCheck[sizeof(PyroString) == 0x04 ? 1 : -1];

}

#endif
