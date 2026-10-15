#ifndef VC1_HOST_KD_STDIO_H
#define VC1_HOST_KD_STDIO_H

// Force-included into the stock sqstdio.cpp only (cmake/squirrel223.cmake).
// The game's sqstdio basic API runs on KD files instead of the CRT: the SQFILE
// is the OpenKdFile handle and every call goes through its driver slots
// (sqstd_loadfile 0x0048fa50 reads through driver+0x120, sqstd_fseek
// 0x0048f310 seeks through driver+0x140 after a null check that sets errno
// 0x11, the SQFile destructor 0x0040eea0 closes through driver+0x118 and frees
// the handle).

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "kd/stream/file_reader.h"

namespace vc1 {

inline void* KdStdioOpen(const char* path, const char* mode)
{
    return OpenKdFile(path, mode);
}

inline int KdStdioRead(void* buffer, int size, int count, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    return handle->driver->read(handle->driver, buffer, size, count, handle->native);
}

inline int KdStdioWrite(const void* buffer, int size, int count, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    return handle->driver->write(handle->driver, buffer, size, count, handle->native);
}

inline int KdStdioSeek(void* file, long offset, int origin)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    return handle->driver->seek(handle->driver, handle->native, offset, origin);
}

inline long KdStdioTell(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    return static_cast<long>(handle->driver->tell(handle->driver, handle->native));
}

inline int KdStdioFlush(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    return handle->driver->flush(handle->driver, handle->native);
}

inline int KdStdioClose(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    handle->driver->close(handle->driver, handle->native);
    free(handle);
    return 0;
}

inline int KdStdioEof(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    return handle->driver->eof(handle->driver, handle->native);
}

}

#define fopen(path, mode) static_cast<FILE*>(vc1::KdStdioOpen(path, mode))
#define fread(buffer, size, count, file) vc1::KdStdioRead(buffer, size, count, file)
#define fwrite(buffer, size, count, file) vc1::KdStdioWrite(buffer, size, count, file)
#define fseek(file, offset, origin) vc1::KdStdioSeek(file, offset, origin)
#define ftell(file) vc1::KdStdioTell(file)
#define fflush(file) vc1::KdStdioFlush(file)
#define fclose(file) vc1::KdStdioClose(file)
#define feof(file) vc1::KdStdioEof(file)

#endif
