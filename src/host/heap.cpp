#include "host/runtime.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

namespace vc1 {

static int PointerInBumpArena(const void* pointer)
{
    const unsigned char* const bytes = static_cast<const unsigned char*>(pointer);
    return bytes >= g_bumpArena && bytes <= g_bumpArena + sizeof(g_bumpArena) - 1;
}

static void ReleaseHeapNode(void* node)
{
    if (PointerInBumpArena(node)) {
        static_cast<unsigned*>(node)[2] = 0;
    }
    else {
        free(node);
    }
}

// Unlink heap nodes whose +0x28 owner matches EBX. Used by socket shutdown.
// FUNCTION: VC1 0x004963b0
void UnlinkHeapByOwner(void* owner)
{
    HeapNode* node = g_heapHead;
    while (node) {
        HeapNode* const next = node->next;
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

// ESI=event. Stamp QPC if the first qword is 0, then link onto DAT_004ed448.
// FUNCTION: VC1 0x00496430
void QueueEvent(void* node)
{
    LARGE_INTEGER now;
    unsigned char* const event = static_cast<unsigned char*>(node);

    if (!event) {
        return;
    }
    if (*reinterpret_cast<unsigned*>(event) == 0
        && *reinterpret_cast<unsigned*>(event + 4) == 0) {
        QueryPerformanceCounter(&now);
        *reinterpret_cast<unsigned long long*>(event) =
            static_cast<unsigned long long>(now.QuadPart);
    }
    *reinterpret_cast<HeapNode**>(event + 0x24) = g_heapTail;
    *reinterpret_cast<HeapNode**>(event + 0x20) = 0;
    if (!g_heapTail) {
        g_heapHead = reinterpret_cast<HeapNode*>(event);
    }
    else {
        g_heapTail->next = reinterpret_cast<HeapNode*>(event);
    }
    g_heapTail = reinterpret_cast<HeapNode*>(event);
}

// Walk the 0x30 bump slots, else malloc(0x30). Zero +0/+4/+0xc/+0x28.
// FUNCTION: VC1 0x004964a0
void* AllocEvent()
{
    unsigned char* slot = g_bumpArena;
    unsigned char* const end = reinterpret_cast<unsigned char*>(&g_heapHead);

    while (slot < end) {
        if (*reinterpret_cast<void**>(slot + 8) == 0) {
            *reinterpret_cast<void**>(slot + 8) = reinterpret_cast<void*>(0xffffffff);
            break;
        }
        slot += 0x30;
    }
    if (slot >= end) {
        slot = static_cast<unsigned char*>(malloc(0x30));
        if (!slot) {
            return 0;
        }
    }
    *reinterpret_cast<void**>(slot) = 0;
    *reinterpret_cast<void**>(slot + 4) = 0;
    *reinterpret_cast<void**>(slot + 0xc) = 0;
    *reinterpret_cast<void**>(slot + 0x28) = 0;
    return slot;
}

// Free DAT_004ed144 if it is not inside the 0x004ed148..0x004ed447 bump range, then the list.
// FUNCTION: VC1 0x00496580
void FreeBumpHeaps()
{
    void* pages = g_heapPageList;
    HeapNode* node = g_heapHead;

    if (g_bumpCursor) {
        ReleaseHeapNode(g_bumpCursor);
        pages = g_heapPageList;
        node = g_heapHead;
    }

    while (node) {
        HeapNode* const next = node->next;
        ReleaseHeapNode(node);
        pages = g_heapPageList;
        node = next;
    }

    if (pages != g_heapPageSentinel) {
        do {
            void* const next = static_cast<void**>(pages)[3];
            free(pages);
            pages = next;
        } while (pages != g_heapPageSentinel);
    }
}

// PeekMessageA drain then timed heap walk. Original returns 0 if a node remains, -1 if empty.
// STUB: VC1 0x00496740
static int WaitQueue()
{
    return 0;
}

// QPC wait until 0x00496740 is non-zero, then pop DAT_004ed448 onto DAT_004ed144.
// FUNCTION: VC1 0x00496840
void* PopTimedEvent()
{
    LARGE_INTEGER now;
    unsigned long long start;
    int pending;

    QueryPerformanceCounter(&now);
    start = static_cast<unsigned long long>(now.QuadPart);
    pending = WaitQueue();
    while (pending == 0) {
        Sleep(1);
        QueryPerformanceCounter(&now);
        if (static_cast<unsigned long long>(now.QuadPart) - start
            == static_cast<unsigned long long>(-1)) {
            break;
        }
        pending = WaitQueue();
    }
    (void)now;

    if (g_bumpCursor) {
        ReleaseHeapNode(g_bumpCursor);
    }

    {
        HeapNode* const node = g_heapHead;
        g_bumpCursor = reinterpret_cast<unsigned*>(node);
        if (node) {
            if (g_heapTail == node) {
                g_heapTail = node->prev;
            }
            g_heapHead = node->next;
            if (node->prev) {
                node->prev->next = node->next;
            }
            if (node->next) {
                node->next->prev = node->prev;
            }
        }
        return node;
    }
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
        const char* expected = "KD_UDID";
        const char* got = name;
        while (*got == *expected) {
            if (*got == 0) {
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

// EDX=buf, stack fmt then varargs. Cap is 0xffffffff.
// FUNCTION: VC1 0x004969f0
void FormatBufferOpen(char* buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, 0xffffffff, fmt, args);
    va_end(args);
}

// Thunk onto the alloca vsnprintf logger at 0x00496a10. Original format in EBX.
// FUNCTION: VC1 0x00496a40
void KdLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)args;
    va_end(args);
}

}
