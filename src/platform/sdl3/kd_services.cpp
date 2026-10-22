// Shared KD file routing and timer semantics over the SDL host services.
#include "platform/sdl3/runtime.h"
#include "host/clock.h"
#include "kd/stream/file_reader.h"
#include <cerrno>
enum { MAX_PATH = 260 };
namespace vc1 {
KdTimer* g_kdTimers;
static long long KdNow() {PlatformClockSample counter;PlatformReadCounter(&counter);return static_cast<long long>(double(counter.QuadPart)*g_nsPerTick);}
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

int KdFread(void* buffer, unsigned size, unsigned count, void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    if (!handle) {
        errno = 0x11;
        return 0;
    }
    return handle->driver->read(handle->driver, buffer, size, count, handle->native);
}

int KdFclose(void* file)
{
    KdFileHandle* const handle = static_cast<KdFileHandle*>(file);
    int result;

    if (!handle) {
        errno = 0x11;
        return -1;
    }
    result = handle->driver->close(handle->driver, handle->native);
    free(handle);
    return result;
}

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

    KdLog("mount: %s(%s)\n", object->scheme, object->name);
    return object;
}

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

    if (FindBytes(path, length, "res/", 4) == path) {
        locale = GetResourceLocale();

        FormatBuffer(buf, 0x100, "res_%s/%s", locale, path + 4);
        file = OpenKdPath(buf, mode);
        if (file) {
            return file;
        }

        FormatBuffer(buf, 0x100, "res_%.2s/%s", locale, path + 4);
        file = OpenKdPath(buf, mode);
        if (file) {
            return file;
        }
    }
    return OpenKdPath(path, mode);
}

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
