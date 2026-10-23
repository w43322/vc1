// The VC9 CRT details the recovered file driver relies on, for the modern CRT.
#include "sdl3.h"
#include "kdcore/runtime.h"

#include <stdio.h>
#include <stdlib.h>

namespace vc1 {

// The map slot reads the whole file; unmap frees it (kdcore/startup.cpp).
void KdFileUnmap(KdObject*, int handle, void* view)
{
    (void)handle;
    free(view);
}

void* KdFileMap(KdObject*, int handle, unsigned* size)
{
    FILE* const file = reinterpret_cast<FILE*>(handle);
    const long position = ftell(file);
    if (fseek(file, 0, SEEK_END) != 0) {
        return 0;
    }
    const long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    void* data = length >= 0 ? malloc(length ? static_cast<size_t>(length) : 1) : 0;
    if (data && fread(data, 1, static_cast<size_t>(length), file) != static_cast<size_t>(length)) {
        free(data);
        data = 0;
    }
    fseek(file, position, SEEK_SET);
    if (data) {
        *size = static_cast<unsigned>(length);
    }
    return data;
}

}

// InitKdFilesystem stores the stdio array; only the address is kept.
struct _iobuf* __iob_func()
{
    return __acrt_iob_func(0);
}
