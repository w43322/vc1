#include "platform/win32/runtime.h"
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
        CreateKdObject(KdConstructFail, 0);
        return;
    }
    if (*reinterpret_cast<HANDLE*>(dir + 0x244) != INVALID_HANDLE_VALUE) {
        FindClose(*reinterpret_cast<HANDLE*>(dir + 0x244));
    }
    free(dir);
    CreateKdObject(KdConstructFail, 0);
}

}
