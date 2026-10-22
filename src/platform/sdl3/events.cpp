// SDL event queue and KD callback contract. No original mappings.
#include "platform/sdl3/runtime.h"
#include "host/clock.h"

#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace vc1 {

// Inferred name. G5 OpenKODE event core: kdCreateEvent / kdPostEvent /
// kdInstallCallback / kdWaitEvent and the message pump behind kdWaitEvent.

static void ReleaseHeapNode(KdEvent* event)
{
    free(event);
}

static void UnlinkEvent(KdEvent* event)
{
    if (g_heapHead == event) {
        g_heapHead = event->next;
    }
    if (g_heapTail == event) {
        g_heapTail = event->prev;
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

static KdCallbackFunc FindCallback(const KdEvent* event)
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

static void DefaultEventCallback(const KdEvent* event)
{
    (void)event;
}

KdCallback g_kdDefaultCallback = { DefaultEventCallback, 0, 0, 0, 0 };

KdCallback* g_kdCallbacks = &g_kdDefaultCallback;

// Unlink heap nodes whose +0x28 owner matches EBX. Used by socket shutdown.

void UnlinkHeapByOwner(void* owner)
{
    KdEvent* node = g_heapHead;
    while (node) {
        KdEvent* const next = node->next;
        if (node->owner == owner) {
            if (g_heapHead == node) {
                g_heapHead = next;
            }
            if (g_heapTail == node) {
                g_heapTail = node->prev;
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

int QueueEvent(KdEvent* event)
{
    PlatformClockSample now;

    if (!event) {
        return -1;
    }
    if (event->timestamp == 0) {
        PlatformReadCounter(&now);
        event->timestamp = static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
    }
    event->prev = g_heapTail;
    event->next = 0;
    if (!g_heapTail) {
        g_heapHead = event;
    }
    else {
        g_heapTail->next = event;
    }
    g_heapTail = event;
    return 0;
}

// kdCreateEvent. A free slot of the 16-event pool, else malloc(0x30).

KdEvent* AllocEvent() { return static_cast<KdEvent*>(calloc(1,sizeof(KdEvent))); }


// kdInstallCallback. ESI=func, EBX=type, EDI=userptr. A null func removes the
// matching record (-1 if none); otherwise a new record goes to the front.

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
                if(node->next) node->next->prev=node->prev;
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

void FreeBumpHeaps()
{
    KdEvent* event;
    KdCallback* node;

    if (g_bumpCursor) {
        ReleaseHeapNode(g_bumpCursor);
    }
    event = g_heapHead;
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
    g_bumpCursor=0;g_heapHead=0;g_heapTail=0;
    g_kdCallbacks=&g_kdDefaultCallback;g_kdDefaultCallback.prev=0;
}

// Window procedure, cdecl event. Drops queued pointer moves (0x4001) of the
// same type and window, so only the newest move stays queued. Inferred name.

void DropPointerMoves(const KdEvent* event)
{
    KdEvent* node = g_heapHead;

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

void DropMatchingEvents(const KdEvent* event)
{
    KdEvent* node = g_heapHead;

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

static int PumpEvents()
{
    KdEvent* event;

    SdlPollEvents();
    PollGamepad();
    FireTimers();

    event = g_heapHead;
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

KdEvent* PopTimedEvent()
{
    PlatformClockSample now;
    __int64 start;
    KdEvent* event;

    PlatformReadCounter(&now);
    start = static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick);
    if (PumpEvents() != 0) {
        do {
            SDL_Delay(1);
            PlatformReadCounter(&now);
            if (static_cast<unsigned __int64>(
                    static_cast<__int64>(static_cast<double>(now.QuadPart) * g_nsPerTick) - start)
                >= static_cast<unsigned __int64>(-1)) {
                break;
            }
        } while (PumpEvents() != 0);
    }

    if (g_bumpCursor) {
        ReleaseHeapNode(g_bumpCursor);
    }
    event = g_heapHead;
    g_bumpCursor = event;
    if (event) {
        g_heapHead = event->next;
        if (g_heapTail == event) {
            g_heapTail = event->prev;
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

const char* GetKdEnv(const char* name) { const char* value=getenv(name); if(value) return value; if(!strcmp(name,"KD_UDID")) { value=getenv("COMPUTERNAME");return value ? value : getenv("HOSTNAME"); } return 0; }


// _vsnprintf(buf, cap, fmt, va). Original EDX=cap, stack buf/fmt then varargs; ECX unused.

int FormatBuffer(char* buf, unsigned cap, const char* fmt, ...)
{
    va_list args;
    int written;
    va_start(args, fmt);
    written = _vsnprintf(buf, cap, fmt, args);
    va_end(args);
    return written;
}

// EDX=buf, stack fmt then varargs. Cap is 0xffffffff.

void FormatBufferOpen(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, 0xffffffff, fmt, args);
    va_end(args);
}

// EBX=fmt, EDI=args. Sizes the message, formats it into an alloca buffer and
// drops it: the release build has no log sink.

void KdLogV(const char* fmt,va_list args) { char buffer[8192]; vsnprintf(buffer,sizeof(buffer),fmt,args); SDL_Log("%s",buffer); }


__declspec(noinline) void KdLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    KdLogV(fmt, args);
    va_end(args);
}

}
