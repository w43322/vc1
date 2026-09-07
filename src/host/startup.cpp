#include "host/runtime.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

namespace vc1 {

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

// Walk DAT_004ed4f8. Each node +0x114 is cdecl (self, path, mode); mode is EBX from the caller.
// FUNCTION: VC1 0x00486eb0
void* OpenKdPath(const char* path, const char* mode)
{
    KdObject* node = g_kdObjects;
    int handle;
    void* wrap;
    typedef int (*KdOpenFn)(KdObject* self, const char* openPath, const char* openMode);

    while (node) {
        handle = (*reinterpret_cast<KdOpenFn*>(reinterpret_cast<char*>(node) + 0x114))(
            node, path, mode);
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

// malloc(0x15c), copy EDI name to +0x10, run ctor, insert at DAT_004ed4f8.
// FUNCTION: VC1 0x00486f50
KdObject* CreateKdObject(int (*ctor)(KdObject* object))
{
    KdObject* const object = static_cast<KdObject*>(malloc(sizeof(KdObject)));
    memset(object, 0, sizeof(KdObject));
    if (ctor) {
        const int status = ctor(object);
        if (status != 0) {
            KdLog("");
            free(object);
            return 0;
        }
    }

    {
        KdObject* head = g_kdObjects;
        if (head) {
            object->prev = head;
            while (head->next) {
                object->prev = head->next;
                head = head->next;
            }
            object->prev->next = object;
        }
        else {
            g_kdObjects = object;
        }
        if (!head) {
            g_kdObjects = object;
        }
        KdLog("");
        return object;
    }
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
    char* const fileName = UnmappedCopyExeDir(path);
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
        handle = UnmappedFindFirstFile(fileName, reinterpret_cast<WIN32_FIND_DATAA*>(dir + 0x104));
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
    void** path;

    CreateKdObject(UnmappedKdConstruct);
    g_kdObjectIt0 = g_kdObjects;
    g_kdObjectIt1 = g_kdObjects;
    g_kdObjectIt2 = g_kdObjects;
    g_kdPath0 = UnmappedCrtPathSlot();
    path = static_cast<void**>(UnmappedCrtPathSlot());
    g_kdPath1 = path + 2;
    g_kdPath2 = static_cast<unsigned char*>(UnmappedCrtPathSlot()) + 0x10;
    dir = DirOpen();

    {
        void** entry = static_cast<void**>(DirRead(dir));
        while (entry) {
            char* name = static_cast<char*>(*entry);
            unsigned length = 0;
            if (name) {
                while (name[length] != 0) {
                    length += 1;
                }
                if (FindBytes(name, length, ".dll", 4)) {
                    CreateKdObject(UnmappedKdConstructNamed);
                }
            }
            entry = static_cast<void**>(DirRead(dir));
        }
    }

    if (!dir) {
        errno = 7;
        CreateKdObject(UnmappedKdConstructFail);
        return;
    }
    if (*reinterpret_cast<HANDLE*>(dir + 0x244) != INVALID_HANDLE_VALUE) {
        FindClose(*reinterpret_cast<HANDLE*>(dir + 0x244));
    }
    free(dir);
    CreateKdObject(UnmappedKdConstructFail);
}

}
