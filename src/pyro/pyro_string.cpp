#include "pyro/pyro_string.h"

#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004e9a24
char* g_pyroStringEmptyData = g_pyroStringEmpty.data;
// GLOBAL: VC1 0x004e9a28
PyroStringEmpty g_pyroStringEmpty = { { -1, 0, 0 }, { 0, 0, 0, 0 } };

static char* PyroStringAlloc(int length)
{
    PyroStringData* header = static_cast<PyroStringData*>(malloc(length + 0x0d));
    header->capacity = length;
    header->refs = 1;
    header->Data()[length] = 0;
    header->length = length;
    return header->Data();
}

// EBX=this. Inferred name (MFC CopyBeforeWrite): a shared buffer is replaced
// by a private copy of its length + 1 bytes.
// FUNCTION: VC1 0x0042eeb0
void PyroString::CopyBeforeWrite()
{
    char* const old = data;
    PyroStringData* const header = reinterpret_cast<PyroStringData*>(old) - 1;
    int length;

    if (header->refs <= 1) {
        return;
    }
    if (header != &g_pyroStringEmpty.header) {
        if (--header->refs <= 0) {
            free(Header());
        }
        data = g_pyroStringEmptyData;
    }
    length = header->length;
    if (length == 0) {
        data = g_pyroStringEmptyData;
    }
    else {
        data = PyroStringAlloc(length);
    }
    memcpy(data, old, header->length + 1);
}

// EDI=this, ESI=length. Inferred name (MFC AllocBeforeWrite): a shared or too
// small buffer is released and a fresh one of exactly length is allocated.
// FUNCTION: VC1 0x0042ef30
void PyroString::AllocBeforeWrite(int length)
{
    if (Header()->refs <= 1 && length <= Header()->capacity) {
        return;
    }
    if (Header() != &g_pyroStringEmpty.header) {
        if (--Header()->refs <= 0) {
            free(Header());
        }
        data = g_pyroStringEmptyData;
    }
    if (length == 0) {
        data = g_pyroStringEmptyData;
        return;
    }
    data = PyroStringAlloc(length);
}

// ECX=this, EAX=text; returns this. Inferred name. Ends by storing the
// terminated length into the capacity field, as the original does.
// FUNCTION: VC1 0x0042efa0
PyroString& PyroString::Assign(const char* text)
{
    int length;

    if (text) {
        length = static_cast<int>(strlen(text));
    }
    else {
        length = 0;
    }
    AllocBeforeWrite(length);
    if (text) {
        memcpy(data, text, length + 1);
    }
    Header()->length = length;
    CopyBeforeWrite();
    length = static_cast<int>(strlen(data));
    Header()->capacity = length;
    data[length] = 0;
    return *this;
}

// EAX=this, EDI=text. Inferred name (construct from a C string).
// FUNCTION: VC1 0x0042f190
void PyroString::InitFrom(const char* text)
{
    int length;

    data = g_pyroStringEmptyData;
    if (!text || !*text) {
        return;
    }
    length = static_cast<int>(strlen(text));
    if (length == 0) {
        data = g_pyroStringEmptyData;
    }
    else {
        data = PyroStringAlloc(length);
    }
    memcpy(data, text, length + 1);
    CopyBeforeWrite();
    length = static_cast<int>(strlen(data));
    Header()->capacity = length;
    data[length] = 0;
}

}
