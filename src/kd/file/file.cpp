#include "kd/file/file.h"

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "host/platform.h"
#include "bindings/sqplus.h"

#include <errno.h>
#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004d4b88
static const char kFile[] = "File";
// GLOBAL: VC1 0x004d0678
const char* g_kdNameFile = kFile;

// STRING: VC1 0x004d5860
static const char kScriptOpen[] = "Open";
// STRING: VC1 0x004d5868
static const char kCantOpen[] = "Can't open %s: %d";

static inline void CloseFile(KdFileHandle*& handle)
{
    if (handle) {
        handle->driver->close(handle->driver, handle->native);
        free(handle);
        handle = 0;
    }
}

// MOV EAX, "File"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00418580
const char* KdFile::GetTypeName()
{
    return kFile;
}

// XOR AL, AL; RET 8. IScriptObject vtable+0x10: no settable members.
// FUNCTION: VC1 0x0046a680
unsigned char KdFile::SetMember(void* name, void* value)
{
    (void)name;
    (void)value;
    return 0;
}

// FUNCTION: VC1 0x0046a6f0
unsigned KdFile::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x0046a710
unsigned KdFile::AddRef()
{
    return IAbstract::AddRef();
}

// RET 8. IOutputStream vtable+0x0c: size bytes through the driver's write
// slot; without an open file errno 0x11 and 0.
// FUNCTION: VC1 0x0046a720
unsigned KdFile::Write(const void* buffer, unsigned size)
{
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->write(handle->driver, buffer, 1, size, handle->native);
}

// RET 8. IInputStream vtable+0x0c: size bytes through the driver's read
// slot. A null buffer skips size bytes instead (seek from the current
// position) and returns size on success. Without an open file errno 0x11.
// FUNCTION: VC1 0x0046a760
unsigned KdFile::Read(void* buffer, unsigned size)
{
    if (!buffer) {
        if (!handle) {
            errno = 0x11;
            return 0;
        }
        if (handle->driver->seek(handle->driver, handle->native, size, SEEK_CUR) != 0) {
            return 0;
        }
        return size;
    }
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->read(handle->driver, buffer, 1, size, handle->native);
}

// IShutdownable vtable+0x0c: closes the file.
// FUNCTION: VC1 0x0046a7e0
int KdFile::Shutdown()
{
    CloseFile(handle);
    return 0;
}

// ESI=this.
// FUNCTION: VC1 0x0046a810
KdFile::~KdFile()
{
    CloseFile(handle);
}

// EAX=this.
// FUNCTION: VC1 0x0046a870
KdFile::KdFile() throw() : handle(0)
{
}

// FUNCTION: VC1 0x0046a8c0
void** CreateKdFile(void** out)
{
    KdFile* memory = static_cast<KdFile*>(malloc(sizeof(KdFile)));
    IUnknown* queried;

    if (memory) {
        new (memory) KdFile();
    }
    queried = memory
        ? static_cast<IUnknown*>(static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract))
        : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

// RET 8. Script "Open"(path, mode): closes the current file (through the
// IShutdownable slot) and opens path; logs errno when that fails.
// FUNCTION: VC1 0x0046a910
bool KdFile::Open(const char* path, const char* mode)
{
    IShutdownable* shutdown = this;

    shutdown->Shutdown();
    handle = static_cast<KdFileHandle*>(OpenKdFile(path, mode));
    if (!handle) {
        HostLogError(&g_kdNameFile, kCantOpen, path, errno);
        return false;
    }
    return true;
}

// RET 8. A one-entry method table (table 0x004ee59c, guard bit 0x004ee5ac).
// FUNCTION: VC1 0x0046aa10
unsigned char KdFile::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptOpen] = CreateScriptMethod<KdFile>(&KdFile::Open);
    }
    return methods.Find(name, out);
}

}
