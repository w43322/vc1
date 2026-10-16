#include "host/runtime.h"
#include "kd/stream/file_reader.h"

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace vc1 {

// GLOBAL: VC1 0x004d43f0
extern const char kKdDataDir[] = "data/";

// Inferred name. "bar" driver: one per *.bar 7z archive next to the exe
// (archive reader around 0x00479f60 / 0x00478e20). Not recovered yet; failing
// keeps the node out of g_kdObjects.
// STUB: VC1 0x0047cb00
static int KdConstructNamed(KdObject* object)
{
    (void)object;
    return -1;
}

// "file" driver slots (inferred names). All cdecl with the driver first; the
// native handle is the CRT FILE*.
// FUNCTION: VC1 0x0047cc80
static void KdFileDestroy(KdObject*)
{
}

// FUNCTION: VC1 0x0047cc90
static void KdFileUnmap(KdObject*, int handle, void* view)
{
    if (view) {
        FILE* const file = reinterpret_cast<FILE*>(handle);
        UnmapViewOfFile(view);
        CloseHandle(reinterpret_cast<HANDLE>(file->_tmpfname));
        file->_tmpfname = 0;
    }
}

// The mapping handle is parked in FILE::_tmpfname (+0x1c) until unmap.
// FUNCTION: VC1 0x0047ccc0
static void* KdFileMap(KdObject*, int handle, unsigned* size)
{
    FILE* const file = reinterpret_cast<FILE*>(handle);
    const HANDLE os = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    const HANDLE mapping = CreateFileMappingA(os, 0, PAGE_READONLY, 0, 0, 0);
    void* view;

    if (!mapping) {
        return 0;
    }
    view = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (!view) {
        CloseHandle(mapping);
        return 0;
    }
    *size = GetFileSize(os, 0);
    file->_tmpfname = reinterpret_cast<char*>(mapping);
    return view;
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

// Inferred name. "http" driver. Not recovered yet; failing keeps the node out
// of g_kdObjects.
// STUB: VC1 0x0047d650
static int KdConstructFail(KdObject* object)
{
    (void)object;
    return -1;
}

// EDI=haystack, EBX=needle, ECX=haystackLen, cdecl needleLen. Uses MemCompare for n>2.
// FUNCTION: VC1 0x00482160
char* FindBytes(const char* haystack, unsigned haystackLen, const char* needle,
    unsigned needleLen)
{
    unsigned i;
    unsigned skipMiss;
    unsigned skipMismatch;
    unsigned limit;

    if (needleLen > haystackLen || needleLen == 0 || haystackLen == 0) {
        return 0;
    }
    if (needleLen == 1) {
        while (*haystack != *needle) {
            haystack += 1;
            haystackLen -= 1;
            if (haystackLen == 0) {
                return 0;
            }
        }
        return const_cast<char*>(haystack);
    }

    if (needle[0] == needle[1]) {
        skipMiss = 2;
        skipMismatch = 1;
    }
    else {
        skipMiss = 1;
        skipMismatch = 2;
    }
    limit = haystackLen - needleLen;
    i = 0;
    while (i <= limit) {
        if (needle[1] != haystack[i + 1]) {
            i += skipMiss;
        }
        else if (MemCompare(needle + 2, haystack + i + 2, needleLen - 2) != 0
            || needle[0] != haystack[i]) {
            i += skipMismatch;
        }
        else {
            return const_cast<char*>(haystack + i);
        }
    }
    return 0;
}

// GLOBAL: VC1 0x004ed4fc
KdTimer* g_kdTimers;

// QPC in nanoseconds; inlined at every use (fild, fmul g_nsPerTick, _ftol2).
static __int64 KdNow()
{
    LARGE_INTEGER qpc;

    QueryPerformanceCounter(&qpc);
    return static_cast<__int64>(static_cast<double>(qpc.QuadPart) * g_nsPerTick);
}

// kdCancelTimer's unlink. EDX=timer; the caller frees it.
// FUNCTION: VC1 0x00486830
void UnlinkTimer(KdTimer* timer)
{
    KdTimer** link;

    if (timer == g_kdTimers) {
        g_kdTimers = timer->next;
        return;
    }
    for (link = &g_kdTimers; *link; link = &(*link)->next) {
        if ((*link)->next == timer) {
            (*link)->next = timer->next;
            return;
        }
    }
}

// kdSetTimer. Every caller passes KD_TIMER_PERIODIC_AVERAGE, which LTCG
// folded into the store. Appends to g_kdTimers; no allocation check.
// FUNCTION: VC1 0x00486890
KdTimer* KdSetTimer(__int64 interval, void* userptr)
{
    KdTimer* timer = static_cast<KdTimer*>(malloc(sizeof(KdTimer)));
    KdTimer** link = &g_kdTimers;

    timer->interval = interval;
    timer->periodic = 0x3f;
    timer->userptr = userptr;
    timer->last = KdNow();
    timer->next = 0;
    while (*link) {
        link = &(*link)->next;
    }
    *link = timer;
    return timer;
}

// For every timer whose interval has passed: restamp it, post a coalesced
// KD_EVENT_TIMER with its userptr, and drop KD_TIMER_ONCE (0x3d) timers.
// Inferred name.
// FUNCTION: VC1 0x00486910
void FireTimers()
{
    KdTimer* timer = g_kdTimers;
    const __int64 now = KdNow();

    while (timer) {
        KdTimer* const next = timer->next;
        if (static_cast<unsigned __int64>(now - timer->last)
            >= static_cast<unsigned __int64>(timer->interval)) {
            KdEvent* event;
            timer->last = now;
            event = AllocEvent();
            event->type = KD_EVENT_TIMER;
            event->userptr = timer->userptr;
            DropMatchingEvents(event);
            QueueEvent(event);
            if (timer->periodic == 0x3d) {
                UnlinkTimer(timer);
            }
        }
        timer = next;
    }
}

// QueryPerformanceFrequency -> ns/tick, srand(time64(0)), KD filesystem, SEH filter, WSA.
// FUNCTION: VC1 0x00486aa0
void InitTimingAndSeh()
{
    LARGE_INTEGER freq;

    AllocPrimaryTls();
    QueryPerformanceFrequency(&freq);
    g_nsPerTick = 1000000000.0 / static_cast<double>(freq.QuadPart);
    srand(static_cast<unsigned>(_time64(0)));
    InitKdFilesystem();
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    StartupNetwork();
}

// kdStat. EBX=path, EDI=out. The first mounted driver whose stat succeeds
// wins (0); -1 if none does.
// FUNCTION: VC1 0x00486b90
int StatKdPath(const char* path, KdFileStat* out)
{
    KdObject* object;

    for (object = g_kdObjects; object; object = object->next) {
        if (object->stat(object, path, out) == 0) {
            return 0;
        }
    }
    return -1;
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

// Inferred name. ECX=file; fread over an OpenKdFile handle (driver+0x120).
// FUNCTION: VC1 0x00486e20
int KdFread(void* buffer, unsigned size, unsigned count, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->read(handle->driver, buffer, size, count, handle->native);
}

// Walk DAT_004ed4f8. Each node +0x114 is cdecl (self, path, mode); mode is EBX from the caller.
// FUNCTION: VC1 0x00486eb0
void* OpenKdPath(const char* path, const char* mode)
{
    KdObject* node = g_kdObjects;
    int handle;
    void* wrap;

    while (node) {
        handle = node->open(node, path, mode);
        if (handle) {
            wrap = malloc(8);
            reinterpret_cast<KdObject**>(wrap)[0] = node;
            reinterpret_cast<int*>(wrap)[1] = handle;
            return wrap;
        }
        node = node->next;
    }
    return 0;
}

// malloc(0x15c), copy EDI name to +0x10, run ctor, append to DAT_004ed4f8.
// FUNCTION: VC1 0x00486f50
KdObject* CreateKdObject(int (*ctor)(KdObject* object), const char* name)
{
    KdObject* const object = static_cast<KdObject*>(malloc(sizeof(KdObject)));
    int status;

    memset(object, 0, sizeof(KdObject));
    if (name) {
        const unsigned length = static_cast<unsigned>(strlen(name)) + 1;
        if (length > 0x100) {
            object->name[0] = 0;
            errno = 0x11;
        }
        else {
            memcpy(object->name, name, length);
        }
    }
    status = ctor(object);
    if (status != 0) {
        // STRING: VC1 0x004d3a94
        KdLog("error: %d, can't mount %s(%s)\n", status, object->scheme, object->name);
        free(object);
        return 0;
    }

    if (!g_kdObjects) {
        g_kdObjects = object;
    }
    else {
        object->prev = g_kdObjects;
        while (object->prev->next) {
            object->prev = object->prev->next;
        }
        object->prev->next = object;
    }
    // STRING: VC1 0x004d3ab4
    KdLog("mount: %s(%s)\n", object->scheme, object->name);
    return object;
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

// Inferred name. ESI=file, EBX=size out. The whole file: the driver's map
// slot when it has one, else fstat, malloc and KdFread. The caller releases
// it through the unmap slot, or free() without one.
// FUNCTION: VC1 0x004870c0
void* KdMapFile(void* file, unsigned* size)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    KdFileStat stat;
    void* buffer;

    if (!handle) {
        errno = 0x11;
        return 0;
    }
    if (handle->driver->map) {
        return handle->driver->map(handle->driver, handle->native, size);
    }
    buffer = 0;
    if (handle->driver->fstat(handle->driver, handle->native, &stat) == 0) {
        buffer = malloc(static_cast<size_t>(stat.size));
        if (buffer) {
            KdFread(buffer, 1, static_cast<unsigned>(stat.size), file);
            *size = static_cast<unsigned>(stat.size);
        }
    }
    return buffer;
}

// cdecl path + mode. Strip leading '.' and '/', remap res/ via KD_LOCALE, then OpenKdPath.
// FUNCTION: VC1 0x00487140
void* OpenKdFile(const char* path, const char* mode)
{
    char buf[0x100];
    const char* locale;
    const char* cursor;
    unsigned length;
    void* file;

    while (*path == '.' || *path == '/') {
        path += 1;
    }
    cursor = path;
    while (*cursor) {
        cursor += 1;
    }
    length = static_cast<unsigned>(cursor - path);
    // STRING: VC1 0x004d3ae4
    if (FindBytes(path, length, "res/", 4) == path) {
        locale = GetResourceLocale();
        // STRING: VC1 0x004d3aec
        FormatBuffer(buf, 0x100, "res_%s/%s", locale, path + 4);
        file = OpenKdPath(buf, mode);
        if (file) {
            return file;
        }
        // STRING: VC1 0x004d3af8
        FormatBuffer(buf, 0x100, "res_%.2s/%s", locale, path + 4);
        file = OpenKdPath(buf, mode);
        if (file) {
            return file;
        }
    }
    return OpenKdPath(path, mode);
}

// Walk DAT_004ed4f8 to the last node, unlink, call +0x110, free.
// FUNCTION: VC1 0x00487200
void DestroyKdObjects()
{
    while (g_kdObjects) {
        KdObject* node = g_kdObjects;
        while (node->next) {
            node = node->next;
        }
        KdLog("");
        if (node->prev) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
        if (node == g_kdObjects) {
            g_kdObjects = node->next;
        }
        if (node->destructor) {
            node->destructor(node);
        }
        free(node);
    }
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
        CreateKdObject(KdConstructFail, 0);
        return;
    }
    if (*reinterpret_cast<HANDLE*>(dir + 0x244) != INVALID_HANDLE_VALUE) {
        FindClose(*reinterpret_cast<HANDLE*>(dir + 0x244));
    }
    free(dir);
    CreateKdObject(KdConstructFail, 0);
}

// Inferred name. EAX=dest[MAX_PATH], EDX=src; bounded strcat, errno 0x23 on overflow.
// FUNCTION: VC1 0x00499d90
int AppendPath(char* dest, const char* src)
{
    const char* end = dest;
    const char* srcEnd = src;
    unsigned used;
    unsigned count;

    while (end != dest + MAX_PATH && *end) {
        end += 1;
    }
    used = static_cast<unsigned>(end - dest);
    while (srcEnd != src + MAX_PATH && *srcEnd) {
        srcEnd += 1;
    }
    count = static_cast<unsigned>(srcEnd - src);
    if (count + used > MAX_PATH) {
        errno = 0x23;
        return 0x23;
    }
    memcpy(dest + used, src, count);
    dest[used + count] = 0;
    return 0;
}

}
