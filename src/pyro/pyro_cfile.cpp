#include "pyro/pyro_crt.h"
#include "pyro/pyro_cfile.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "kd/stream/file_reader.h"
#include "kdcore/platform.h"

namespace vc1 {

// The null-handle checks the Pyro file streams inline into every slot.
static int PyroFileRead(void* buffer, int size, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->read(handle->driver, buffer, 1, size, handle->native);
}

static int PyroFileWrite(const void* buffer, int size, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->write(handle->driver, buffer, 1, size, handle->native);
}

static int PyroFileSeek(void* file, int offset, int origin)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    return handle->driver->seek(handle->driver, handle->native, offset, origin);
}

static int PyroFileTell(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return -1;
    }
    return static_cast<int>(handle->driver->tell(handle->driver, handle->native));
}

// XOR EAX, EAX; RET 8: the same body as the image stream reader's Write.
// FUNCTION: VC1 0x0042e860 FOLDED
int PyroCFile::Write(const void* buffer, int size)
{
    (void)buffer;
    (void)size;
    return 0;
}

// FUNCTION: VC1 0x0042f100
int PyroSubStream::Tell()
{
    return inner->Tell() - start;
}

// cdecl. The one caller logs CFile::Open failures.
// FUNCTION: VC1 0x00449f50
void PyroLog(const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    KdLogV(fmt, args);
    va_end(args);
}

// RET 8. Returns the clamped count, not what the inner stream read.
// FUNCTION: VC1 0x0044dae0
int PyroSubStream::Read(void* buffer, int size)
{
    const int available = start + length - position;

    if (size > available) {
        size = available;
    }
    position += inner->Read(buffer, size);
    return size;
}

// RET 8. SEEK_END ignores the offset; the position is clamped to the window.
// FUNCTION: VC1 0x0044db20
int PyroSubStream::Seek(int offset, int origin)
{
    if (origin == 0) {
        position = start + offset;
    }
    else if (origin == 1) {
        position += offset;
    }
    else if (origin == 2) {
        position = length + start;
    }
    if (position < start) {
        position = start;
    }
    if (position > start + length) {
        position = start + length;
    }
    return inner->Seek(position, 0) - start;
}

// RET 8.
// FUNCTION: VC1 0x0044db90
int PyroSubStream::Write(const void* buffer, int size)
{
    const int available = start + length - position;
    int written;

    if (size > available) {
        size = available;
    }
    written = inner->Write(buffer, size);
    position += written;
    return written;
}

// RET 8.
// FUNCTION: VC1 0x0044dd90
int PyroKdFileStream::Write(const void* buffer, int size)
{
    return PyroFileWrite(buffer, size, file);
}

// RET 8.
// FUNCTION: VC1 0x0044ddd0
int PyroKdFileStream::Read(void* buffer, int size)
{
    return PyroFileRead(buffer, size, file);
}

// FUNCTION: VC1 0x0044de10
int PyroKdFileStream::Tell()
{
    return PyroFileTell(file);
}

// RET 8.
// FUNCTION: VC1 0x0044de50
int PyroKdFileStream::Seek(int offset, int origin)
{
    return PyroFileSeek(file, offset, origin);
}

// Measures by seeking to the end and back.
// FUNCTION: VC1 0x0044dea0
int PyroKdFileStream::GetLength()
{
    const int position = PyroFileTell(file);
    int size;

    PyroFileSeek(file, 0, 2);
    size = PyroFileTell(file);
    PyroFileSeek(file, position, 0);
    return size;
}

// EDI=this, RET 4. Wraps the open handle in a file stream and a window over
// the whole file.
// FUNCTION: VC1 0x0044dff0
void PyroCFile::Attach(void* handle)
{
    file = new PyroKdFileStream(handle);
    stream = new PyroSubStream(file);
}

// ESI=this. Also inlined into the destructor (0x0042e740).
// FUNCTION: VC1 0x0044e090
void PyroCFile::Close()
{
    if (name.data != g_pyroStringEmpty.data) {
        name.Release();
        name.data = g_pyroStringEmptyData;
    }
    if (stream) {
        delete stream;
        stream = 0;
    }
    if (file) {
        KdFileHandle* const handle = static_cast<KdFileHandle*>(file->file);
        if (!handle) {
            errno = 0x11;
        }
        else {
            handle->driver->close(handle->driver, handle->native);
            free(handle);
        }
        if (file) {
            delete file;
            file = 0;
        }
    }
}

// ECX=this, EAX=path, RET 4. Backslashes become slashes in a 0x100-byte copy
// of the path (an empty copy and errno 0x11 when it does not fit).
// FUNCTION: VC1 0x0044e120
bool PyroFileSystem::Open(const char* path, PyroCFile* target)
{
    char buffer[0x100];
    void* handle;

    if (!enabled) {
        return false;
    }
    const unsigned size = static_cast<unsigned>(strlen(path)) + 1;
    if (size > sizeof(buffer)) {
        buffer[0] = 0;
        errno = 0x11;
    }
    else {
        memcpy(buffer, path, size);
    }
    for (int i = 0; i < static_cast<int>(strlen(path)); ++i) {
        if (buffer[i] == '\\') {
            buffer[i] = '/';
        }
    }
    if (!root[0]) {
        handle = OpenKdFile(buffer, "rb");
    }
    else {
        const char slash = '/';
        PyroString base;
        PyroString prefix;
        PyroString name;
        PyroString full;

        base.InitFrom(root);
        prefix.data = g_pyroStringEmptyData;
        prefix.ConcatCopy(base.GetLength(), base.data, 1, &slash);
        name.InitFrom(buffer);
        full.data = g_pyroStringEmptyData;
        full.ConcatCopy(prefix.GetLength(), prefix.data, name.GetLength(), name.data);
        prefix.Release();
        base.Release();
        name.Release();
        handle = OpenKdFile(full.Header() != &g_pyroStringEmpty.header ? full.data : 0, "rb");
        full.Release();
    }
    if (!handle) {
        return false;
    }
    target->Attach(handle);
    return true;
}

// Inferred name. The error path's function-local static: nothing reads it,
// only its (empty) destructor is registered.
struct PyroOpenErrorScope
{
    ~PyroOpenErrorScope() {}
};

// ESI=this, EDI=path. The file system is a function-local static (guard
// 0x004ee1d4, object 0x004ed8c8).
// FUNCTION: VC1 0x0044e3d0
bool PyroCFile::Open(const char* path)
{
    static PyroFileSystem fileSystem;

    if (!fileSystem.Open(path, this)) {
        static PyroOpenErrorScope scope;
        // STRING: VC1 0x004d7fcc
        PyroLog("CFile::Open(...) : Unable to open file '%s'", path);
        return false;
    }
    name.Assign(path);
    return true;
}

}
