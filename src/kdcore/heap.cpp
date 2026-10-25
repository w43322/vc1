#include "kdcore/runtime.h"
#include "kdcore/native.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace vc1 {

// Inferred name. G5 OpenKODE event core: kdCreateEvent / kdPostEvent /
// kdInstallCallback / kdWaitEvent and the message pump behind kdWaitEvent.

static int PointerInBumpArena(const void* pointer)
{
    const unsigned char* const bytes = static_cast<const unsigned char*>(pointer);
    return bytes >= g_kdEventQueue.arena && bytes < reinterpret_cast<const unsigned char*>(&g_kdEventQueue.head);
}

// kdFreeEvent, inlined at its callers (the original also keeps an
// out-of-line copy, src/app/out_of_line_copies.h): pool slots are marked free, others freed.
static void ReleaseHeapNode(KdEvent* event)
{
    if (PointerInBumpArena(event)) {
        event->type = 0;
    }
    else {
        free(event);
    }
}

static void UnlinkEvent(KdEvent* event)
{
    if (g_kdEventQueue.head == event) {
        g_kdEventQueue.head = event->next;
    }
    if (g_kdEventQueue.tail == event) {
        g_kdEventQueue.tail = event->prev;
    }
    if (event->prev) {
        event->prev->next = event->next;
    }
    if (event->next) {
        event->next->prev = event->prev;
    }
}

// ESI=event. First installed callback whose type (0 = any) and userptr
// (0 = any) match; the static default record at the end never matches.
// Inferred name.
// FUNCTION: VC1 0x004962b0
__declspec(noinline) static KdCallbackFunc FindCallback(const KdEvent* event)
{
    KdCallback* node;

    for (node = g_kdCallbacks; node->next; node = node->next) {
        if ((node->type == 0 || node->type == event->type)
            && (node->userptr == 0 || node->userptr == event->userptr)) {
            return node->func;
        }
    }
    return 0;
}

// The static terminator's callback. Inferred name.
// FUNCTION: VC1 0x00496380
static void DefaultEventCallback(const KdEvent* event)
{
    (void)event;
}

// GLOBAL: VC1 0x004e918c
KdCallback g_kdDefaultCallback = { DefaultEventCallback, 0, 0, 0, 0 };
// GLOBAL: VC1 0x004e95c8
KdCallback* g_kdCallbacks = &g_kdDefaultCallback;
// GLOBAL: VC1 0x004e95cc
KdFloatBits g_kdInfinity = { 0x7f800000 };

// Unlink heap nodes whose +0x28 owner matches EBX. Used by socket shutdown.
// FUNCTION: VC1 0x004963b0
void UnlinkHeapByOwner(void* owner)
{
    KdEvent* node = g_kdEventQueue.head;
    while (node) {
        KdEvent* const next = node->next;
        if (node->owner == owner) {
            if (g_kdEventQueue.head == node) {
                g_kdEventQueue.head = next;
            }
            if (g_kdEventQueue.tail == node) {
                g_kdEventQueue.tail = node->prev;
            }
            if (node->prev) {
                node->prev->next = next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
        }
        node = next;
    }
}

// kdPostEvent. ESI=event. Stamps the time (QPC ns) if unset, then appends.
// FUNCTION: VC1 0x00496430
int QueueEvent(KdEvent* event)
{
    LARGE_INTEGER now;

    if (!event) {
        return -1;
    }
    if (event->timestamp == 0) {
        QueryPerformanceCounter(&now);
        event->timestamp = static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
    }
    event->prev = g_kdEventQueue.tail;
    event->next = 0;
    if (!g_kdEventQueue.tail) {
        g_kdEventQueue.head = event;
    }
    else {
        g_kdEventQueue.tail->next = event;
    }
    g_kdEventQueue.tail = event;
    return 0;
}

// kdCreateEvent. A free slot of the 16-event pool, else malloc(0x30).
// FUNCTION: VC1 0x004964a0
KdEvent* AllocEvent()
{
    KdEvent* event = reinterpret_cast<KdEvent*>(g_kdEventQueue.arena);
    KdEvent* const end = reinterpret_cast<KdEvent*>(&g_kdEventQueue.head);

    while (event < end) {
        if (event->type == 0) {
            event->type = -1;
            break;
        }
        ++event;
    }
    if (event >= end) {
        event = static_cast<KdEvent*>(malloc(sizeof(KdEvent)));
        if (!event) {
            return 0;
        }
    }
    event->timestamp = 0;
    event->userptr = 0;
    event->owner = 0;
    return event;
}

// kdInstallCallback. ESI=func, EBX=type, EDI=userptr. A null func removes the
// matching record (-1 if none); otherwise a new record goes to the front.
// FUNCTION: VC1 0x004964f0
int KdInstallCallback(KdCallbackFunc func, int type, void* userptr)
{
    KdCallback* node;

    if (!func) {
        for (node = g_kdCallbacks; node->next; node = node->next) {
            if (node->type == type && node->userptr == userptr) {
                if (g_kdCallbacks == node) {
                    g_kdCallbacks = node->next;
                }
                else {
                    node->prev->next = node->next;
                }
                free(node);
                return 0;
            }
        }
        return -1;
    }
    node = static_cast<KdCallback*>(malloc(sizeof(KdCallback)));
    node->func = func;
    node->type = type;
    node->userptr = userptr;
    node->next = g_kdCallbacks;
    node->prev = 0;
    g_kdCallbacks->prev = node;
    g_kdCallbacks = node;
    return 0;
}

// Frees the last returned event, the queue and every installed callback.
// FUNCTION: VC1 0x00496580
void FreeBumpHeaps()
{
    KdEvent* event;
    KdCallback* node;

    if (g_kdEventQueue.cursor) {
        ReleaseHeapNode(g_kdEventQueue.cursor);
    }
    event = g_kdEventQueue.head;
    while (event) {
        KdEvent* const next = event->next;
        ReleaseHeapNode(event);
        event = next;
    }
    node = g_kdCallbacks;
    while (node != &g_kdDefaultCallback) {
        KdCallback* const next = node->next;
        free(node);
        node = next;
    }
}

// Window procedure, cdecl event. Drops queued pointer moves (0x4001) of the
// same type and window, so only the newest move stays queued. Inferred name.
// FUNCTION: VC1 0x00496610
void DropPointerMoves(const KdEvent* event)
{
    KdEvent* node = g_kdEventQueue.head;

    while (node) {
        KdEvent* const next = node->next;
        if (node->type == event->type && node->data[0] == 0x4001 &&
            node->userptr == event->userptr) {
            UnlinkEvent(node);
            ReleaseHeapNode(node);
        }
        node = next;
    }
}

// EBX=event. Drops queued events of the same type and userptr, so a due
// timer replaces its pending event. Inferred name.
// FUNCTION: VC1 0x004966b0
void DropMatchingEvents(const KdEvent* event)
{
    KdEvent* node = g_kdEventQueue.head;

    while (node) {
        KdEvent* const next = node->next;
        if (node->type == event->type && node->userptr == event->userptr) {
            UnlinkEvent(node);
            ReleaseHeapNode(node);
        }
        node = next;
    }
}

// Drains the Win32 queue, polls the pad, fires timers, then runs every queued
// event through its callback; handled events are freed. -1 if the queue was
// empty, else 0. Inferred name.
// FUNCTION: VC1 0x00496740
static int PumpEvents()
{
    MSG msg;
    KdEvent* event;

    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    PollGamepad();
    FireTimers();

    event = g_kdEventQueue.head;
    if (!event) {
        return -1;
    }
    do {
        KdCallbackFunc func = FindCallback(event);
        KdEvent* const next = event->next;
        if (func) {
            func(event);
            UnlinkEvent(event);
            ReleaseHeapNode(event);
        }
        event = next;
    } while (event);
    return 0;
}

// kdWaitEvent with the timeout folded to -1: pump until an event is queued,
// free the previously returned one and pop the head.
// FUNCTION: VC1 0x00496840
KdEvent* PopTimedEvent()
{
    LARGE_INTEGER now;
    __int64 start;
    KdEvent* event;

    QueryPerformanceCounter(&now);
    start = static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
    if (PumpEvents() != 0) {
        do {
            Sleep(1);
            QueryPerformanceCounter(&now);
            if (static_cast<unsigned __int64>(
                    static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick) - start)
                >= static_cast<unsigned __int64>(-1)) {
                break;
            }
        } while (PumpEvents() != 0);
    }

    if (g_kdEventQueue.cursor) {
        ReleaseHeapNode(g_kdEventQueue.cursor);
    }
    event = g_kdEventQueue.head;
    g_kdEventQueue.cursor = event;
    if (event) {
        g_kdEventQueue.head = event->next;
        if (g_kdEventQueue.tail == event) {
            g_kdEventQueue.tail = event->prev;
        }
        if (event->prev) {
            event->prev->next = event->next;
        }
        if (event->next) {
            event->next->prev = event->prev;
        }
    }
    return event;
}

// getenv(name); if missing and name is KD_UDID, getenv("COMPUTERNAME"). Original name in ESI.
// FUNCTION: VC1 0x00496950
const char* GetKdEnv(const char* name)
{
    const char* value = getenv(name);
    if (value) {
        return value;
    }

    {
        // STRING: VC1 0x004d27d0
        const char* expected = "KD_UDID";
        const char* got = name;
        while (*got == *expected) {
            if (*got == 0) {
                // STRING: VC1 0x004d27d8
                return getenv("COMPUTERNAME");
            }
            if (got[1] != expected[1]) {
                break;
            }
            if (got[1] == 0) {
                return getenv("COMPUTERNAME");
            }
            got += 2;
            expected += 2;
        }
    }
    return 0;
}

// _vsnprintf(buf, cap, fmt, va). Original EDX=cap, stack buf/fmt then varargs; ECX unused.
// FUNCTION: VC1 0x004969c0
int FormatBuffer(char* buf, unsigned cap, const char* fmt, ...)
{
    va_list args;
    int written;
    va_start(args, fmt);
    written = _vsnprintf(buf, cap, fmt, args);
    va_end(args);
    return written;
}

// EDX=buf, stack fmt then varargs. Cap is 0xffffffff: the game's sprintf,
// also for Squirrel's scsprintf and libpng's sprintf, hence C linkage.
// FUNCTION: VC1 0x004969f0
extern "C" int FormatBufferOpen(char* buf, const char* fmt, ...)
{
    va_list args;
    int result;
    va_start(args, fmt);
    result = _vsnprintf(buf, 0xffffffff, fmt, args);
    va_end(args);
    return result;
}

// EBX=fmt, EDI=args. Sizes the message, formats it into an alloca buffer and
// drops it: the release build has no log sink.
// FUNCTION: VC1 0x00496a10
void KdLogV(const char* fmt, va_list args)
{
    const int size = _vsnprintf(0, 0, fmt, args) + 1;
    char* const buf = static_cast<char*>(_alloca(size));
    _vsnprintf(buf, size, fmt, args);
#if VC1_LOG
    // Not original: the VC1_LOG option sends the text to the debug output.
    OutputDebugStringA(buf);
#endif
}

// FUNCTION: VC1 0x00496a40
extern "C" void KdLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    KdLogV(fmt, args);
    va_end(args);
}

}
