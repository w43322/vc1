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

// QPC wait until 0x00496740 is non-zero, then pop DAT_004ed448 onto DAT_004ed144.
// FUNCTION: VC1 0x00496840
void* PopTimedEvent()
{
    LARGE_INTEGER now;
    unsigned long long start;
    int pending;

    QueryPerformanceCounter(&now);
    start = UnmappedQpcDelta();
    pending = UnmappedWaitQueue();
    while (pending == 0) {
        Sleep(1);
        QueryPerformanceCounter(&now);
        if (UnmappedQpcDelta() - start == static_cast<unsigned long long>(-1)) {
            break;
        }
        pending = UnmappedWaitQueue();
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
void FormatBuffer(char* buf, unsigned cap, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    _vsnprintf(buf, cap, fmt, args);
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
