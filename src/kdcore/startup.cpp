#include "kdcore/runtime.h"
#include "kd/stream/file_reader.h"

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <wininet.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// The bar driver's 7z reader, built with callback input (cmake/lzma.cmake).
#define _LZMA_IN_CB
extern "C" {
#include "7zAlloc.h"
#include "7zExtract.h"
}

namespace vc1 {

// GLOBAL: VC1 0x004d43f0
extern const char kKdDataDir[] = "data/";

// "bar" driver (inferred names): one per *.bar file, a 7z archive read with
// the LZMA SDK 4.57 (third_party/lzma457). Opening a file extracts its whole
// solid block; the handle then reads from the file's offset to the end of
// that block, as in the original.

// SIZE 0x58
struct BarArchive
{
    ISzInStream stream;    // 0x00  BarStreamRead / BarStreamSeek
    void* file;            // 0x08  OpenKdFile of the archive
    ISzAlloc alloc;        // 0x0c  SzAlloc / SzFree, main and temp
    CArchiveDatabaseEx db; // 0x18
};

typedef char Vc1BarArchiveSizeCheck[sizeof(BarArchive) == 0x58 ? 1 : -1];

// SIZE 0x10
struct BarFile
{
    CFileItem* entry; // 0x00
    Byte* data;       // 0x04  the extracted solid block
    size_t size;      // 0x08  block size
    size_t position;  // 0x0c  starts at the file's offset in the block
};

// The archive read window.
// GLOBAL: VC1 0x004efa20
unsigned char g_barReadBuffer[0x1000];

// FUNCTION: VC1 0x0047c4e0
static void BarUnmap(KdObject*, int, void*)
{
}

// FUNCTION: VC1 0x0047c4f0
static void* BarMap(KdObject*, int handle, unsigned* size)
{
    const BarFile* const file = reinterpret_cast<BarFile*>(handle);

    *size = file->size;
    return file->data;
}

// Read-only: asking for write access fails.
// FUNCTION: VC1 0x0047c510
static int BarAccess(KdObject*, const char*, int mode)
{
    return (mode & 2) ? -1 : 0;
}

// FUNCTION: VC1 0x0047c520
static int BarFstat(KdObject*, int handle, KdFileStat* out)
{
    const BarFile* const file = reinterpret_cast<BarFile*>(handle);

    out->mode = 0x8000;
    out->mtime = *reinterpret_cast<const long long*>(&file->entry->LastWriteTime);
    out->size = file->size;
    return 0;
}

// FUNCTION: VC1 0x0047c550
static __int64 BarTell(KdObject*, int handle)
{
    return reinterpret_cast<BarFile*>(handle)->position;
}

// SEEK_END counts back from the block end; past the end fails.
// FUNCTION: VC1 0x0047c560
static int BarSeek(KdObject*, int handle, __int64 offset, int origin)
{
    BarFile* const file = reinterpret_cast<BarFile*>(handle);
    size_t position = file->position;

    switch (origin) {
    case 0:
        position = static_cast<size_t>(offset);
        break;
    case 1:
        position += static_cast<size_t>(offset);
        break;
    case 2:
        position = file->size - static_cast<size_t>(offset);
        break;
    }
    if (position > file->size) {
        return -1;
    }
    file->position = position;
    return 0;
}

// FUNCTION: VC1 0x0047c5a0
static void BarClearErr(KdObject*, int)
{
}

// FUNCTION: VC1 0x0047c5b0
static int BarError(KdObject*, int)
{
    return 0;
}

// FUNCTION: VC1 0x0047c5c0
static int BarEof(KdObject*, int handle)
{
    const BarFile* const file = reinterpret_cast<BarFile*>(handle);
    return file->position >= file->size;
}

// FUNCTION: VC1 0x0047c5d0
static int BarPutc(KdObject*, int, int)
{
    return -1;
}

// FUNCTION: VC1 0x0047c5e0
static int BarWrite(KdObject*, const void*, unsigned, unsigned, int)
{
    return 0;
}

// FUNCTION: VC1 0x0047c5f0
static int BarFlush(KdObject*, int)
{
    return 0;
}

// FUNCTION: VC1 0x0047c600
static int BarRead(KdObject*, void* buffer, unsigned size, unsigned count, int handle)
{
    BarFile* const file = reinterpret_cast<BarFile*>(handle);
    size_t length = file->size - file->position;

    if (length >= size * count) {
        length = size * count;
    }
    memcpy(buffer, file->data + file->position, length);
    file->position += length;
    return static_cast<int>(length / size);
}

// FUNCTION: VC1 0x0047c650
static int BarClose(KdObject* self, int handle)
{
    BarFile* const file = reinterpret_cast<BarFile*>(handle);

    static_cast<BarArchive*>(self->data)->alloc.Free(file->data);
    free(file);
    return 0;
}

// ECX=a, EAX=b. Inferred name. The name order of the archive index; inlined
// into BarLowerBound, with an uncalled out-of-line copy (external linkage).
// FUNCTION: VC1 0x0047c680
bool BarNameLess(const char* a, const char* b)
{
    return _stricmp(a, b) < 0;
}

// ISzInStream::Seek. The position goes to KdFseek as a long.
// FUNCTION: VC1 0x0047c6a0
static SZ_RESULT BarStreamSeek(void* object, CFileSize position)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(static_cast<BarArchive*>(object)->file);

    if (!handle) {
        errno = 0x11;
        return SZE_FAIL;
    }
    if (handle->driver->seek(handle->driver, handle->native, static_cast<long>(position), 0)) {
        return SZE_FAIL;
    }
    return SZ_OK;
}

// ISzInStream::Read: at most 0x1000 bytes into g_barReadBuffer.
// FUNCTION: VC1 0x0047c6f0
static SZ_RESULT BarStreamRead(void* object, void** buffer, size_t size, size_t* processed)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(static_cast<BarArchive*>(object)->file);
    size_t read;

    if (size > sizeof(g_barReadBuffer)) {
        size = sizeof(g_barReadBuffer);
    }
    if (!handle) {
        errno = 0x11;
        read = 0;
    }
    else {
        read = handle->driver->read(handle->driver, g_barReadBuffer, 1, size, handle->native);
    }
    *buffer = g_barReadBuffer;
    if (processed) {
        *processed = read;
    }
    return SZ_OK;
}

// One byte at a time into an int that is compared whole with '\n', as in
// the original.
// FUNCTION: VC1 0x0047c750
static char* BarGets(KdObject*, char* buffer, int size, int handle)
{
    BarFile* const file = reinterpret_cast<BarFile*>(handle);
    unsigned length = 0;
    int c;

    if (!size) {
        return 0;
    }
    while (length < static_cast<unsigned>(size - 1)) {
        size_t read = file->size - file->position;
        if (read >= 1) {
            read = 1;
        }
        memcpy(&c, file->data + file->position, read);
        file->position += read;
        if (!read) {
            break;
        }
        buffer[length++] = static_cast<char>(c);
        if (c == '\n') {
            break;
        }
    }
    buffer[length] = 0;
    return buffer;
}

// FUNCTION: VC1 0x0047c7d0
static int BarGetc(KdObject*, int handle)
{
    BarFile* const file = reinterpret_cast<BarFile*>(handle);
    size_t read = file->size - file->position;
    int c = 0;

    if (read >= 1) {
        read = 1;
    }
    memcpy(&c, file->data + file->position, read);
    file->position += read;
    if (!read) {
        return -1;
    }
    return c;
}

// EAX=first, EBX=last, EDI=name. The case-insensitive equal entry, or 0.
// FUNCTION: VC1 0x0047c820
static CFileItem* BarFindLinear(CFileItem* first, CFileItem* last, const char* name)
{
    CFileItem* item = first;

    for (; item != last; ++item) {
        if (_stricmp(name, item->Name) >= 0 && _stricmp(item->Name, name) >= 0) {
            break;
        }
    }
    return item != last ? item : 0;
}

// ECX=first, EAX=last, name on the stack. Lower bound by _stricmp.
// FUNCTION: VC1 0x0047c860
static CFileItem* BarLowerBound(CFileItem* first, CFileItem* last, const char* name)
{
    while (first != last) {
        CFileItem* const middle = first + (last - first) / 2;
        if (BarNameLess(middle->Name, name)) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// SzArDbExFree inlined; the archive file is closed through its driver.
// FUNCTION: VC1 0x0047c8b0
static void BarDestroy(KdObject* self)
{
    BarArchive* const archive = static_cast<BarArchive*>(self->data);
    KdFileHandle* handle;

    SzArDbExFree(&archive->db, archive->alloc.Free);
    handle = static_cast<KdFileHandle*>(archive->file);
    if (!handle) {
        errno = 0x11;
        free(archive);
        return;
    }
    handle->driver->close(handle->driver, handle->native);
    free(handle);
    free(archive);
}

// The sorted lookup, then a linear scan. Inlined into BarStat and BarOpen;
// the out-of-line copy (EAX=name, ECX=last, first on the stack) has no
// callers, which external linkage reproduces under /OPT:NOREF.
// FUNCTION: VC1 0x0047c950
CFileItem* BarFind(CFileItem* first, CFileItem* last, const char* name)
{
    CFileItem* const item = BarLowerBound(first, last, name);

    if (item != last && _stricmp(name, item->Name) >= 0) {
        return item;
    }
    return BarFindLinear(first, last, name);
}

// FUNCTION: VC1 0x0047c9a0
static int BarStat(KdObject* self, const char* path, KdFileStat* out)
{
    BarArchive* const archive = static_cast<BarArchive*>(self->data);
    CFileItem* const files = archive->db.Database.Files;
    const CFileItem* const item = BarFind(files, files + archive->db.Database.NumFiles, path);

    if (!item) {
        return -1;
    }
    out->mode = item->IsDirectory ? 0x4000 : 0x8000;
    out->mtime = *reinterpret_cast<const long long*>(&item->LastWriteTime);
    out->size = item->Size;
    return 0;
}

// The handle is 0 when the name is missing or the block does not extract.
// FUNCTION: VC1 0x0047ca30
static int BarOpen(KdObject* self, const char* path, const char* mode)
{
    BarArchive* const archive = static_cast<BarArchive*>(self->data);
    CFileItem* const files = archive->db.Database.Files;
    CFileItem* const item = BarFind(files, files + archive->db.Database.NumFiles, path);
    BarFile* file;
    UInt32 blockIndex;
    size_t processed;

    (void)mode;
    if (!item) {
        return 0;
    }
    blockIndex = 0xffffffff;
    file = static_cast<BarFile*>(malloc(sizeof(BarFile)));
    file->entry = item;
    file->data = 0;
    file->size = 0;
    if (SzExtract(&archive->stream, &archive->db, static_cast<UInt32>(item - files), &blockIndex,
            &file->data, &file->size, &file->position, &processed, &archive->alloc,
            &archive->alloc)) {
        free(file);
        return 0;
    }
    return reinterpret_cast<int>(file);
}

// Inferred name. "bar" driver constructor: opens the archive named by the
// node and reads its database; a failed open leaves the node out.
// FUNCTION: VC1 0x0047cb00
static int KdConstructNamed(KdObject* object)
{
    BarArchive* const archive = static_cast<BarArchive*>(malloc(sizeof(BarArchive)));
    int result;

    object->data = archive;
    // STRING: VC1 0x004d43f8
    object->scheme = "bar";
    object->destructor = BarDestroy;
    object->open = BarOpen;
    object->close = BarClose;
    object->flush = BarFlush;
    object->read = BarRead;
    object->write = BarWrite;
    object->getc = BarGetc;
    object->putc = BarPutc;
    object->gets = BarGets;
    object->eof = BarEof;
    object->error = BarError;
    object->clearerr = BarClearErr;
    object->seek = BarSeek;
    object->tell = BarTell;
    object->stat = BarStat;
    object->fstat = BarFstat;
    object->access = BarAccess;
    object->map = BarMap;
    object->unmap = BarUnmap;
    archive->file = OpenKdFile(object->name, "rb");
    archive->alloc.Alloc = SzAlloc;
    archive->alloc.Free = SzFree;
    archive->stream.Read = BarStreamRead;
    archive->stream.Seek = BarStreamSeek;
    if (!archive->file) {
        free(archive);
        return -1;
    }
    SzArDbExInit(&archive->db);
    result = SzArchiveOpen(&archive->stream, &archive->db, &archive->alloc, &archive->alloc);
    if (result) {
        SzArDbExFree(&archive->db, archive->alloc.Free);
        free(archive);
    }
    return result;
}

// "file" driver slots (inferred names). All cdecl with the driver first; the
// native handle is the CRT FILE*.
// FUNCTION: VC1 0x0047cc80
static void KdFileDestroy(KdObject*)
{
}

// FUNCTION: VC1 0x0047cd30
static __int64 KdFileTell(KdObject*, int handle)
{
    return ftell(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cd40
static int KdFileSeek(KdObject*, int handle, __int64 offset, int origin)
{
    return fseek(reinterpret_cast<FILE*>(handle), static_cast<long>(offset), origin);
}

// FUNCTION: VC1 0x0047cd60
static void KdFileClearErr(KdObject*, int handle)
{
    clearerr(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cd70
static int KdFileError(KdObject*, int handle)
{
    return ferror(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cd80
static int KdFileEof(KdObject*, int handle)
{
    return feof(reinterpret_cast<FILE*>(handle)) > 0 ? -1 : 0;
}

// FUNCTION: VC1 0x0047cda0
static char* KdFileGets(KdObject*, char* buffer, int size, int handle)
{
    return fgets(buffer, size, reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cdc0
static int KdFilePutc(KdObject*, int c, int handle)
{
    return fputc(c, reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cde0
static int KdFileGetc(KdObject*, int handle)
{
    return fgetc(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047cdf0
static int KdFileWrite(KdObject*, const void* buffer, unsigned size, unsigned count,
    int handle)
{
    return static_cast<int>(fwrite(buffer, size, count, reinterpret_cast<FILE*>(handle)));
}

// FUNCTION: VC1 0x0047ce10
static int KdFileRead(KdObject*, void* buffer, unsigned size, unsigned count, int handle)
{
    return static_cast<int>(fread(buffer, size, count, reinterpret_cast<FILE*>(handle)));
}

// FUNCTION: VC1 0x0047ce30
static int KdFileFlush(KdObject*, int handle)
{
    return fflush(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047ce40
static int KdFileClose(KdObject*, int handle)
{
    return fclose(reinterpret_cast<FILE*>(handle));
}

// FUNCTION: VC1 0x0047ce70
static int KdFileFstat(KdObject*, int handle, KdFileStat* out)
{
    struct _stat st;
    const int result = _fstat(_fileno(reinterpret_cast<FILE*>(handle)), &st);
    if (result != 0) {
        return result;
    }
    out->mode = st.st_mode;
    out->mtime = st.st_mtime;
    out->size = st.st_size;
    return 0;
}

// Inferred name. EAX=path, cdecl dest[MAX_PATH]. "data/..." resolves under
// %APPDATA%\Virtual City; any other path is returned unchanged.
// FUNCTION: VC1 0x0047cec0
char* CopyExeDir(const char* path, char* dest)
{
    if (FindBytes(path, static_cast<unsigned>(strlen(path)), kKdDataDir, 5) == path
        && SHGetSpecialFolderPathA(GetActiveWindow(), dest, CSIDL_APPDATA, TRUE)) {
        PathAppendA(dest, kVirtualCity);
        PathAppendA(dest, path + 5);
        return dest;
    }
    return const_cast<char*>(path);
}

// FUNCTION: VC1 0x0047cf30
static int KdFileAccess(KdObject*, const char* path, int mode)
{
    char buffer[MAX_PATH];
    if (path) {
        path = CopyExeDir(path, buffer);
    }
    return _access(path, mode);
}

// FUNCTION: VC1 0x0047cf80
static int KdFileStatPath(KdObject*, const char* path, KdFileStat* out)
{
    struct _stat st;
    char buffer[MAX_PATH];
    int result;

    if (path) {
        path = CopyExeDir(path, buffer);
    }
    result = _stat(path, &st);
    if (result != 0) {
        return result;
    }
    out->mode = st.st_mode;
    out->mtime = st.st_mtime;
    out->size = st.st_size;
    return 0;
}

// FUNCTION: VC1 0x0047cfe0
static int KdFileOpen(KdObject*, const char* path, const char* mode)
{
    char buffer[MAX_PATH];
    if (path) {
        path = CopyExeDir(path, buffer);
    }
    return reinterpret_cast<int>(fopen(path, mode));
}

// Inferred name. "file" driver constructor; also creates %APPDATA%\Virtual City
// for "data/" paths.
// FUNCTION: VC1 0x0047d030
static int KdConstruct(KdObject* object)
{
    char buffer[MAX_PATH];

    // STRING: VC1 0x004d2f1c
    object->scheme = "file";
    object->destructor = KdFileDestroy;
    object->open = KdFileOpen;
    object->close = KdFileClose;
    object->flush = KdFileFlush;
    object->read = KdFileRead;
    object->write = KdFileWrite;
    object->getc = KdFileGetc;
    object->putc = KdFilePutc;
    object->gets = KdFileGets;
    object->eof = KdFileEof;
    object->error = KdFileError;
    object->clearerr = KdFileClearErr;
    object->seek = KdFileSeek;
    object->tell = KdFileTell;
    object->stat = KdFileStatPath;
    object->fstat = KdFileFstat;
    object->access = KdFileAccess;
    object->map = KdFileMap;
    object->unmap = KdFileUnmap;
    _mkdir(CopyExeDir(kKdDataDir, buffer));
    return 0;
}

// "http" driver (inferred names): read-only URLs through wininet. The handle
// is a malloc'd HttpFile; size stays -1 until a read hits the end or a
// SEEK_END tells it.
// SIZE 0x0c
struct HttpFile
{
    HINTERNET request; // 0x00
    unsigned position; // 0x04
    unsigned size;     // 0x08
};

// GLOBAL: VC1 0x004ed500
HINTERNET g_kdInternet;

// FUNCTION: VC1 0x0047d120
static void HttpDestroy(KdObject*)
{
    if (g_kdInternet) {
        InternetCloseHandle(g_kdInternet);
    }
}

// FUNCTION: VC1 0x0047d140
static int HttpAccess(KdObject*, const char*, int)
{
    return -1;
}

// FUNCTION: VC1 0x0047d150
static int HttpFstat(KdObject*, int handle, KdFileStat* out)
{
    out->mode = 0x8000;
    out->mtime = 0;
    out->size = reinterpret_cast<HttpFile*>(handle)->size;
    return 0;
}

// FUNCTION: VC1 0x0047d180
static int HttpStat(KdObject*, const char*, KdFileStat*)
{
    return -1;
}

// FUNCTION: VC1 0x0047d190
static __int64 HttpTell(KdObject*, int handle)
{
    return reinterpret_cast<HttpFile*>(handle)->position;
}

// The handle doubles as the wininet context argument.
// FUNCTION: VC1 0x0047d1a0
static int HttpSeek(KdObject*, int handle, __int64 offset, int origin)
{
    HttpFile* const file = reinterpret_cast<HttpFile*>(handle);
    const DWORD position = InternetSetFilePointer(file->request, static_cast<LONG>(offset), 0,
        origin, reinterpret_cast<DWORD_PTR>(file));

    if (position == static_cast<DWORD>(-1)) {
        return -1;
    }
    if (origin == 2) {
        file->size = position - static_cast<DWORD>(offset);
    }
    file->position = position;
    return 0;
}

// FUNCTION: VC1 0x0047d1e0
static void HttpClearErr(KdObject*, int)
{
}

// FUNCTION: VC1 0x0047d1f0
static int HttpError(KdObject*, int)
{
    return 0;
}

// FUNCTION: VC1 0x0047d200
static int HttpEof(KdObject*, int handle)
{
    const HttpFile* const file = reinterpret_cast<HttpFile*>(handle);

    if (file->size == static_cast<unsigned>(-1)) {
        return 0;
    }
    return file->position >= file->size;
}

// FUNCTION: VC1 0x0047d220
static int HttpPutc(KdObject*, int, int)
{
    return -1;
}

// FUNCTION: VC1 0x0047d230
static int HttpWrite(KdObject*, const void*, unsigned, unsigned, int)
{
    return 0;
}

// FUNCTION: VC1 0x0047d240
static int HttpFlush(KdObject*, int)
{
    return 0;
}

// LogLastError's body (0x0047c480), inlined into HttpGets; HttpRead and
// HttpOpen call it.
static __forceinline void LogLastErrorInline(const char* name, DWORD error)
{
    char* message;

    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0,
            reinterpret_cast<LPSTR>(&message), 0, 0)) {
        KdLog("[%s] error: %s\n", name, message);
        LocalFree(message);
    }
    else {
        KdLog("[%s] error: %u\n", name, error);
    }
}

// FUNCTION: VC1 0x0047d250
static int HttpRead(KdObject* self, void* buffer, unsigned size, unsigned count, int handle)
{
    HttpFile* const file = reinterpret_cast<HttpFile*>(handle);
    DWORD read = 0;

    if (file->position >= file->size) {
        return 0;
    }
    if (!InternetReadFile(file->request, buffer, size * count, &read)) {
        LogLastError(self->scheme, GetLastError());
        return 0;
    }
    file->position += read;
    if (read == 0) {
        file->size = file->position;
    }
    return read / size;
}

// FUNCTION: VC1 0x0047d2d0
static int HttpClose(KdObject*, int handle)
{
    HttpFile* const file = reinterpret_cast<HttpFile*>(handle);

    InternetCloseHandle(file->request);
    free(file);
    return 0;
}

// One byte at a time into an int that is compared whole with '\n', as in
// the original.
// FUNCTION: VC1 0x0047d2f0
static char* HttpGets(KdObject* self, char* buffer, int size, int handle)
{
    HttpFile* const file = reinterpret_cast<HttpFile*>(handle);
    unsigned length = 0;

    if (!size) {
        return 0;
    }
    while (length < static_cast<unsigned>(size - 1)) {
        DWORD read = 0;
        int c;

        if (file->position >= file->size) {
            break;
        }
        if (!InternetReadFile(file->request, &c, 1, &read)) {
            LogLastErrorInline(self->scheme, GetLastError());
            break;
        }
        file->position += read;
        if (read == 0) {
            file->size = file->position;
            break;
        }
        buffer[length++] = static_cast<char>(c);
        if (c == '\n') {
            break;
        }
    }
    buffer[length] = 0;
    return buffer;
}

// FUNCTION: VC1 0x0047d410
static int HttpGetc(KdObject* self, int handle)
{
    int c = 0;

    if (!HttpRead(self, &c, 1, 1, handle)) {
        return -1;
    }
    return c;
}

// Only paths starting with the scheme. The session is opened on first use
// with the OpenKODE user agent; a request is kept only for status 200.
// FUNCTION: VC1 0x0047d450
static int HttpOpen(KdObject* self, const char* path, const char* mode)
{
    char agent[0x100];
    char status[8];
    DWORD length;
    DWORD index;
    HINTERNET request;
    HttpFile* file;

    (void)mode;
    if (FindBytes(path, static_cast<unsigned>(strlen(path)), self->scheme,
            static_cast<unsigned>(strlen(self->scheme))) != path) {
        return 0;
    }
    if (!g_kdInternet) {
        OSVERSIONINFOA version;

        version.dwOSVersionInfoSize = sizeof(version);
        GetVersionExA(&version);
        // STRING: VC1 0x004d282c
        FormatBuffer(g_windowsVersionText, sizeof(g_windowsVersionText), "Windows v%ld.%ld.%ld",
            version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
        // STRING: VC1 0x004d43a4
        FormatBufferOpen(agent, "OpenKODE %s %s %s",
            // STRING: VC1 0x004d27f0
            "G5 Entertainment AB",
            // STRING: VC1 0x004d2804
            "1.0.1 (Compiled Dec 21 2009 22:47:18)",
            g_windowsVersionText);
        g_kdInternet = InternetOpenA(agent, 0, 0, 0, 0);
        if (!g_kdInternet) {
            LogLastError(self->scheme, GetLastError());
        }
    }
    // STRING: VC1 0x004d43b8
    KdLog("[http] GET %s\n", path);
    request = InternetOpenUrlA(g_kdInternet, path, 0, 0, 0xa0400000, 0);
    if (!request) {
        LogLastError(self->scheme, GetLastError());
        return 0;
    }
    memset(status, 0, sizeof(status));
    length = sizeof(status);
    index = 0;
    if (!HttpQueryInfoA(request, HTTP_QUERY_STATUS_CODE, status, &length, &index)) {
        LogLastError(self->scheme, GetLastError());
        InternetCloseHandle(request);
        return 0;
    }
    if (strtol(status, 0, 10) != 200) {
        // STRING: VC1 0x004d43c8
        KdLog("[http] unexpected response: %s\n", status);
        InternetCloseHandle(request);
        return 0;
    }
    file = static_cast<HttpFile*>(malloc(sizeof(HttpFile)));
    file->request = request;
    file->position = 0;
    file->size = static_cast<unsigned>(-1);
    return reinterpret_cast<int>(file);
}

// Inferred name. "http" driver constructor; there is no map.
// FUNCTION: VC1 0x0047d650
static int KdConstructHttp(KdObject* object)
{
    // STRING: VC1 0x004d43e8
    object->scheme = "http";
    object->destructor = HttpDestroy;
    object->open = HttpOpen;
    object->close = HttpClose;
    object->flush = HttpFlush;
    object->read = HttpRead;
    object->write = HttpWrite;
    object->getc = HttpGetc;
    object->putc = HttpPutc;
    object->gets = HttpGets;
    object->eof = HttpEof;
    object->error = HttpError;
    object->clearerr = HttpClearErr;
    object->seek = HttpSeek;
    object->tell = HttpTell;
    object->stat = HttpStat;
    object->fstat = HttpFstat;
    object->access = HttpAccess;
    object->map = 0;
    object->unmap = 0;
    return 0;
}

// Inferred name. QueryPerformanceFrequency -> ns/tick, srand(time64(0)).
// Inlined into InitTimingAndSeh; the out-of-line copy has no callers.
// FUNCTION: VC1 0x00486800
void InitTiming()
{
    LARGE_INTEGER freq;

    QueryPerformanceFrequency(&freq);
    g_nsPerTick = 1000000000.0 / static_cast<double>(freq.QuadPart);
    srand(static_cast<unsigned>(_time64(0)));
}

// Timing, KD filesystem, SEH filter, WSA.
// FUNCTION: VC1 0x00486aa0
void InitTimingAndSeh()
{
    AllocPrimaryTls();
    InitTiming();
    InitKdFilesystem();
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    StartupNetwork();
}

// Copy cFileName from the FIND_DATA at +0x104 and FindNextFileA. Original dir in ESI.
// FUNCTION: VC1 0x00486c40
void* DirRead(unsigned char* dir)
{
    if (!dir) {
        return 0;
    }

    HANDLE* const handle = reinterpret_cast<HANDLE*>(dir + 0x244);
    if (*reinterpret_cast<int*>(dir + 0x244) != -1) {
        char* const name = reinterpret_cast<char*>(dir + 0x130);
        unsigned length = 0;
        while (name[length] != 0) {
            length += 1;
        }
        if (length + 1 < 0x101) {
            memcpy(dir, name, length + 1);
        }
        else {
            dir[0] = 0;
            errno = 0x11;
        }
        if (!FindNextFileA(*handle, reinterpret_cast<WIN32_FIND_DATAA*>(dir + 0x104))) {
            FindClose(*handle);
            *reinterpret_cast<int*>(dir + 0x244) = -1;
        }
        return dir + 0x248;
    }

    if (*reinterpret_cast<int*>(dir + 0x104) != -1) {
        *reinterpret_cast<int*>(dir + 0x104) = -1;
        return 0;
    }
    errno = 7;
    return 0;
}

// Append \*.* (or *.* after a separator) and FindFirstFileA. ECX=path, cdecl FIND_DATA.
// FUNCTION: VC1 0x00487020
static HANDLE DirFindFirst(const char* path, WIN32_FIND_DATAA* data)
{
    char pattern[MAX_PATH];
    const unsigned length = static_cast<unsigned>(strlen(path)) + 1;
    const char* wildcard;

    if (length > MAX_PATH) {
        pattern[0] = 0;
        errno = 0x11;
    }
    else {
        memcpy(pattern, path, length);
    }
    if (pattern[strlen(pattern) - 1] == '\\' || pattern[strlen(pattern) - 1] == '/') {
        // STRING: VC1 0x004d3b0c
        wildcard = "*.*";
    }
    else {
        // STRING: VC1 0x004d3b04
        wildcard = "\\*.*";
    }
    AppendPath(pattern, wildcard);
    return FindFirstFileA(pattern, data);
}

// GetFileAttributesA directory check, malloc(0x24c), FindFirstFile into +0x104.
// FUNCTION: VC1 0x00487280
unsigned char* DirOpen()
{
    char path[260];
    // STRING: VC1 0x004d3ad8
    char* const fileName = CopyExeDir(".", path);
    DWORD attributes;

    if (!fileName || fileName[0] == 0) {
        errno = 0x18;
        return 0;
    }
    attributes = GetFileAttributesA(fileName);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        errno = 0x18;
        return 0;
    }

    {
        unsigned char* const dir = static_cast<unsigned char*>(malloc(0x24c));
        int handle;
        if (!dir) {
            errno = 0x19;
            return 0;
        }
        *reinterpret_cast<unsigned char**>(dir + 0x248) = dir;
        handle = reinterpret_cast<int>(DirFindFirst(fileName,
            reinterpret_cast<WIN32_FIND_DATAA*>(dir + 0x104)));
        *reinterpret_cast<int*>(dir + 0x244) = handle;
        if (handle != -1) {
            char* const name = reinterpret_cast<char*>(dir + 0x130);
            unsigned length = 0;
            while (name[length] != 0) {
                length += 1;
            }
            if (length + 1 < 0x101) {
                memcpy(dir, name, length + 1);
                return dir;
            }
            dir[0] = 0;
            errno = 0x11;
            return dir;
        }
        free(dir);
        return 0;
    }
}

// Inferred name. EDI=oldPath, EAX=newPath. CRT rename on both paths after
// the "data/..." translation; a null path stays null.
// FUNCTION: VC1 0x004873c0
int KdRename(const char* oldPath, const char* newPath)
{
    char newBuffer[260];
    char oldBuffer[260];
    char* const newName = newPath ? CopyExeDir(newPath, newBuffer) : 0;

    if (!oldPath) {
        return rename(0, newName);
    }
    return rename(CopyExeDir(oldPath, oldBuffer), newName);
}

// Seed KD object iterators from DAT_004ed4f8 and CRT path slots, then scan the exe directory.
// FUNCTION: VC1 0x00487450
void InitKdFilesystem()
{
    unsigned char* dir;
    FILE* iob;

    CreateKdObject(KdConstruct, 0);
    g_kdObjectIt0 = g_kdObjects;
    g_kdObjectIt1 = g_kdObjects;
    g_kdObjectIt2 = g_kdObjects;
    iob = __iob_func();
    g_kdPath0 = iob;
    g_kdPath1 = iob + 1;
    g_kdPath2 = iob + 2;
    dir = DirOpen();

    {
        void** entry = static_cast<void**>(DirRead(dir));
        while (entry) {
            const char* const name = static_cast<const char*>(*entry);
            // STRING: VC1 0x004d3adc
            if (FindBytes(name, static_cast<unsigned>(strlen(name)), ".bar", 4)) {
                CreateKdObject(KdConstructNamed, name);
            }
            entry = static_cast<void**>(DirRead(dir));
        }
    }

    if (!dir) {
        errno = 7;
        CreateKdObject(KdConstructHttp, 0);
        return;
    }
    if (*reinterpret_cast<HANDLE*>(dir + 0x244) != INVALID_HANDLE_VALUE) {
        FindClose(*reinterpret_cast<HANDLE*>(dir + 0x244));
    }
    free(dir);
    CreateKdObject(KdConstructHttp, 0);
}

}
