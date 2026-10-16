#ifndef VC1_HOST_KD_TGA_STDIO_H
#define VC1_HOST_KD_TGA_STDIO_H

// Force-included into the vendored libtga (cmake/libtga.cmake). Unlike sqstdio,
// the game's libtga goes through the KD stdio calls that check for a null
// handle (errno 0x11): TGARead 0x004ba1a0 and TGAReadRLE 0x004ba050 read
// through KdFread (0x00486e20, inlined into TGARead), TGAReadRLE's getc goes
// through driver+0x128, __TGASeek 0x00459e40 seeks and tells through
// driver+0x140/+0x144, and TGAClose 0x00459ec0 closes through driver+0x118.

#include <string.h>

#include "host/kd_stdio.h"

namespace vc1 {

// 0x00486e20; ECX=file
int KdFread(void* buffer, unsigned size, unsigned count, void* file);

inline int KdFgetc(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    return handle->driver->getc(handle->driver, handle->native);
}

inline int KdFclose(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    handle->driver->close(handle->driver, handle->native);
    free(handle);
    return 0;
}

}

#undef fread
#undef fclose
#undef getc
#define fread(buffer, size, count, file) vc1::KdFread(buffer, size, count, file)
#define fclose(file) vc1::KdFclose(file)
#define getc(file) vc1::KdFgetc(file)

#endif
