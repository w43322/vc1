#ifndef VC1_KDCORE_KD_STDIO_C_H
#define VC1_KDCORE_KD_STDIO_C_H

/* Force-included into libpng's pngrio.c only (cmake/libpng.cmake). The game's
 * libpng reads through the KD file layer: png_default_read_data (0x004b4130)
 * calls io_ptr's driver read slot (+0x120) after a null check that sets errno
 * 0x11. A C view of KdFileHandle / KdObject (kdcore/platform.h), since libpng
 * is C. */

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

struct VcKdDriver
{
    char head[0x120];
    int (*read)(struct VcKdDriver* self, void* buffer, unsigned size, unsigned count,
        int handle); /* +0x120 */
};

struct VcKdFile
{
    struct VcKdDriver* driver;
    int native;
};

static __inline size_t VcKdFread(void* buffer, size_t size, size_t count, void* file)
{
    struct VcKdFile* const handle = (struct VcKdFile*)file;

    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return (size_t)handle->driver->read(handle->driver, buffer, (unsigned)size, (unsigned)count,
        handle->native);
}

#define fread(buffer, size, count, file) VcKdFread(buffer, size, count, file)

#endif
