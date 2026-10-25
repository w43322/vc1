// The VC9 CRT details the recovered file driver relies on, for Emscripten's
// musl (the web counterpart of imports/sdl3/crt.cpp).
#include "../sdl3/sdl3.h"
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

// InitKdFilesystem stores stdin, stdout and stderr as the CRT array's first
// three entries. musl's streams are separate objects, so the array holds
// copies of them (FILE is the stand-in from vc1_web_crt.h, larger than musl's);
// the console only writes and flushes them.
struct _iobuf* __iob_func()
{
    static FILE iob[3];
    static bool ready;
    if (!ready) {
        memcpy(&iob[0], stdin, sizeof(iob[0]));
        memcpy(&iob[1], stdout, sizeof(iob[1]));
        memcpy(&iob[2], stderr, sizeof(iob[2]));
        ready = true;
    }
    return reinterpret_cast<struct _iobuf*>(iob);
}
