#include "kdcore/runtime.h"

#include <stdlib.h>

namespace vc1 {

static int InterlockedCompareExchangeThunk(volatile LONG* dest, LONG exchange, LONG comparand)
{
    if (g_interlockedCompareExchange) {
        typedef LONG(WINAPI * IceFn)(volatile LONG*, LONG, LONG);
        return reinterpret_cast<IceFn>(g_interlockedCompareExchange)(dest, exchange, comparand);
    }
    return InterlockedCompareExchange(dest, exchange, comparand);
}

static int MutexCheck(MutexRef* ref);
static unsigned long long CurrentThreadId64();
static int MutexLockInner(MutexRef* ref);
static LONG WINAPI IceFallback(volatile LONG* dest, LONG exchange, LONG comparand);
static void TlsMutexDestructor(void* thread);
static void ResetThreadNode(void* thread);
static DWORD WINAPI ApcFallback();

// calloc(1, 0x10) + TlsAlloc; destructor at +4 and cookie 0xffffffff when param != 0.
// Original out pointer lives in EDI (DAT_004ed534 from 0x0047bdf0).
// FUNCTION: VC1 0x0047a270
int AllocTlsBlock(void* destructor, TlsBlock** out)
{
    TlsBlock* const block = static_cast<TlsBlock*>(calloc(1, sizeof(TlsBlock)));
    if (!block) {
        *out = 0;
        return kKdEnomem;
    }

    block->index = TlsAlloc();
    if (block->index == TLS_OUT_OF_INDEXES) {
        free(block);
        *out = 0;
        return kKdEagain;
    }

    if (destructor) {
        block->cookie = 0xffffffff;
        block->destructor = destructor;
    }
    *out = block;
    return 0;
}

// Lazy-create the mutex behind a 0xfffffffd/-2/-3 sentinel. Original MutexRef in EBX.
// STUB: VC1 0x0047a3a0
static int MutexCheck(MutexRef* ref)
{
    (void)ref;
    return 0;
}

// Zero the 0x8c thread node and splice it onto DAT_004e89b8. cdecl.
// STUB: VC1 0x0047a490
static void ResetThreadNode(void* thread)
{
    (void)thread;
}

// Unlink a waiter node from two intrusive lists then free. Original node in EAX.
// FUNCTION: VC1 0x0047a600
void UnlinkWaiter(void* node)
{
    if (!node) {
        return;
    }

    unsigned* const fields = static_cast<unsigned*>(node);
    const unsigned nextA = fields[2];
    const unsigned prevA = fields[4];
    if (prevA) {
        *reinterpret_cast<unsigned*>(prevA + 8) = nextA;
    }
    if (nextA) {
        *reinterpret_cast<unsigned*>(nextA + 0x10) = prevA;
    }

    unsigned* const owner = *reinterpret_cast<unsigned**>(fields);
    if (owner) {
        if (*reinterpret_cast<unsigned**>(owner + (0x84 / 4)) == fields) {
            owner[0x84 / 4] = nextA;
        }
        if (*reinterpret_cast<unsigned**>(owner + (0x88 / 4)) == fields) {
            owner[0x88 / 4] = nextA;
        }
    }

    const unsigned nextB = fields[3];
    const unsigned prevB = fields[5];
    if (prevB) {
        *reinterpret_cast<unsigned*>(prevB + 0xc) = nextB;
    }
    if (nextB) {
        *reinterpret_cast<unsigned*>(nextB + 0x14) = prevB;
    }
    if (fields[1] && *reinterpret_cast<unsigned**>(fields[1] + 0xc) == fields) {
        *reinterpret_cast<unsigned*>(fields[1] + 0xc) = nextB;
    }
    free(node);
}

// GetProcAddress miss for InterlockedCompareExchange. Stored at DAT_004ed528.
// STUB: VC1 0x0047a720
static LONG WINAPI IceFallback(volatile LONG* dest, LONG exchange, LONG comparand)
{
    (void)dest;
    (void)exchange;
    (void)comparand;
    return 0;
}

// AllocTlsBlock destructor for g_tlsSecondary.
// STUB: VC1 0x0047a7a0
static void TlsMutexDestructor(void* thread)
{
    (void)thread;
}

// Release a simple or recursive mutex. Original MutexRef in EAX.
// FUNCTION: VC1 0x0047a7c0
int MutexUnlock(MutexRef* ref)
{
    Mutex* const mutex = ref ? ref->mutex : 0;
    if (reinterpret_cast<unsigned>(mutex) >= 0xfffffffd) {
        return kKdEinval;
    }

    HANDLE event = 0;
    if (mutex->kind == 0) {
        const LONG previous = InterlockedExchange(&mutex->state, 0);
        if (previous == 0) {
            return 1;
        }
        if (previous >= 0) {
            return 0;
        }
        event = mutex->event;
    }
    else {
        const unsigned long long tid = CurrentThreadId64();
        const unsigned long long owner =
            (static_cast<unsigned long long>(mutex->ownerHi) << 32) | mutex->ownerLo;
        if (tid != owner) {
            return 1;
        }
        if (mutex->kind == 1) {
            mutex->recursion -= 1;
            if (mutex->recursion != 0) {
                return 0;
            }
        }
        mutex->ownerLo = 0;
        const LONG previous = InterlockedExchange(&mutex->state, 0);
        if (previous >= 0) {
            return 0;
        }
        event = mutex->event;
    }

    if (SetEvent(event)) {
        return 0;
    }
    return kKdEinval;
}

// Acquire a simple or recursive mutex. Original MutexRef in EAX. 0x16 on a null pointer.
// FUNCTION: VC1 0x0047a840
int MutexLock(MutexRef* ref)
{
    int status = 0;
    if (!ref || !ref->mutex) {
        return kKdEinval;
    }

    if (reinterpret_cast<unsigned>(ref->mutex) >= 0xfffffffd) {
        status = MutexCheck(ref);
        if (status != 0) {
            return status;
        }
    }

    Mutex* const mutex = ref->mutex;
    if (mutex->kind == 0) {
        LONG previous = InterlockedExchange(&mutex->state, 1);
        if (previous != 0) {
            previous = InterlockedExchange(&mutex->state, -1);
            if (previous != 0) {
                do {
                    if (WaitForSingleObject(mutex->event, INFINITE) != 0) {
                        return kKdEinval;
                    }
                    previous = InterlockedExchange(&mutex->state, -1);
                } while (previous != 0);
                return status;
            }
        }
        return status;
    }

    const unsigned long long tid = CurrentThreadId64();
    const int exchanged = InterlockedCompareExchangeThunk(&mutex->state, 1, 0);
    if (exchanged == 0) {
        mutex->recursion = 1;
        mutex->ownerLo = static_cast<DWORD>(tid);
        mutex->ownerHi = static_cast<DWORD>(tid >> 32);
        return status;
    }

    const unsigned long long owner =
        (static_cast<unsigned long long>(mutex->ownerHi) << 32) | mutex->ownerLo;
    if (tid == owner) {
        if (mutex->kind == 1) {
            mutex->recursion += 1;
            return status;
        }
        return kKdEdeadlk;
    }

    int held = InterlockedExchange(&mutex->state, -1);
    while (held != 0) {
        if (WaitForSingleObject(mutex->event, INFINITE) != 0) {
            return kKdEinval;
        }
        held = InterlockedExchange(&mutex->state, -1);
    }
    if (status == 0) {
        mutex->recursion = 1;
        mutex->ownerLo = static_cast<DWORD>(tid);
        mutex->ownerHi = static_cast<DWORD>(tid >> 32);
    }
    return status;
}

// TLS thread record: TlsGetValue then DuplicateHandle. Used as the 64-bit owner id.
// STUB: VC1 0x0047ad60
static unsigned long long CurrentThreadId64()
{
    return GetCurrentThreadId();
}

// Try-lock. Original MutexRef in EAX. Takes a free mutex (a recursive one
// records the owner), re-enters a recursive mutex this thread owns, else 0x10.
// FUNCTION: VC1 0x0047b020
static int MutexLockInner(MutexRef* ref)
{
    int status = 0;

    if (reinterpret_cast<unsigned>(ref->mutex) >= 0xfffffffd) {
        status = MutexCheck(ref);
        if (status != 0) {
            return status;
        }
    }
    Mutex* const mutex = ref->mutex;
    if (InterlockedCompareExchangeThunk(&mutex->state, 1, 0) == 0) {
        if (mutex->kind != 0) {
            mutex->recursion = 1;
            const unsigned long long tid = CurrentThreadId64();
            mutex->ownerLo = static_cast<DWORD>(tid);
            mutex->ownerHi = static_cast<DWORD>(tid >> 32);
        }
        return status;
    }
    if (mutex->kind == 1) {
        const unsigned long long tid = CurrentThreadId64();
        if (static_cast<DWORD>(tid) == mutex->ownerLo
            && static_cast<DWORD>(tid >> 32) == mutex->ownerHi) {
            mutex->recursion += 1;
            return status;
        }
    }
    return kKdEbusy;
}

// Destroy the mutex behind *ref, spinning on 0x10 (busy). Original ref in EDI.
// The try-lock and unlock go through a local copy because *ref is cleared
// first.
// FUNCTION: VC1 0x0047b0a0
int MutexDestroy(MutexRef* ref)
{
    Mutex* mutex = ref->mutex;
    if (reinterpret_cast<unsigned>(mutex) < 0xfffffffd) {
        MutexRef local;
        local.mutex = mutex;
        const int inner = MutexLockInner(&local);
        if (inner == 0) {
            mutex = local.mutex;
            if (mutex->kind == 1 && mutex->recursion != 1) {
                mutex->recursion -= 1;
                return kKdEbusy;
            }
            ref->mutex = 0;
            const int unlocked = MutexUnlock(&local);
            if (unlocked != 0) {
                ref->mutex = mutex;
                return unlocked;
            }
            if (!CloseHandle(mutex->event)) {
                ref->mutex = mutex;
                return kKdEinval;
            }
            free(mutex);
            return 0;
        }
        return inner;
    }

    EnterCriticalSection(&g_csMutex);
    if (reinterpret_cast<unsigned>(ref->mutex) > 0xfffffffc) {
        ref->mutex = 0;
        LeaveCriticalSection(&g_csMutex);
        return 0;
    }
    LeaveCriticalSection(&g_csMutex);
    return kKdEbusy;
}

// Copy 0x23 dwords off the thread object, drop the mutex/event handles, then 0x0047a490.
// FUNCTION: VC1 0x0047b2f0
void DestroyThreadLocal(void* thread)
{
    HANDLE copy[0x23];
    unsigned i;

    if (!thread) {
        return;
    }

    unsigned* src = static_cast<unsigned*>(thread);
    for (i = 0; i < 0x23; ++i) {
        copy[i] = *reinterpret_cast<HANDLE*>(src);
        src += 1;
    }
    ResetThreadNode(thread);
    if (copy[0x0f]) {
        CloseHandle(copy[0x0f]);
    }
    MutexDestroy(reinterpret_cast<MutexRef*>(&copy[0x0c]));
    MutexDestroy(reinterpret_cast<MutexRef*>(&copy[0x0a]));
    if (copy[1]) {
        CloseHandle(copy[1]);
    }
}

// TlsFree + waiter drain + free. WinMain loads EBX from 0x004ed530 / 0x004ed534 then CALLs.
// FUNCTION: VC1 0x0047b380
int DestroyTlsBlock(TlsBlock* self)
{
    if (!self) {
        return 0;
    }

    MutexRef* const lock =
        reinterpret_cast<MutexRef*>(reinterpret_cast<unsigned char*>(self) + 8);
    if (self->waiters && self->destructor) {
        if (MutexLock(lock) == 0) {
            void* waiter = self->waiters;
            while (waiter) {
                MutexRef* const waiterLock = reinterpret_cast<MutexRef*>(
                    *static_cast<unsigned char**>(waiter) + 0x28);
                if (MutexLock(waiterLock) == 0) {
                    UnlinkWaiter(waiter);
                    MutexUnlock(waiterLock);
                }
                else {
                    UnlinkWaiter(waiter);
                }
                waiter = self->waiters;
            }
            MutexUnlock(lock);
        }
    }

    TlsFree(self->index);
    if (self->destructor) {
        int busy = MutexDestroy(lock);
        while (busy == kKdEbusy) {
            Sleep(1);
            busy = MutexDestroy(lock);
        }
    }
    free(self);
    return 0;
}

// Destroy both TLS slots, free the node list, DeleteCriticalSection on the six CS objects.
// FUNCTION: VC1 0x0047b6b0
void ShutdownThreading()
{
    if (!g_threadingReady) {
        return;
    }

    if (g_tlsSlot0) {
        DestroyTlsBlock(g_tlsSlot0);
        g_tlsSlot0 = 0;
    }
    if (g_tlsSlot1) {
        DestroyTlsBlock(g_tlsSlot1);
        g_tlsSlot1 = 0;
    }

    EnterCriticalSection(&g_csList);
    void* node = g_threadNodeSentinel;
    while (node != reinterpret_cast<void*>(1)) {
        void* const next = *reinterpret_cast<void**>(static_cast<unsigned char*>(node) + 0x10);
        free(node);
        node = next;
    }
    LeaveCriticalSection(&g_csList);

    DeleteCriticalSection(&g_csPad3);
    DeleteCriticalSection(&g_csPad2);
    DeleteCriticalSection(&g_csPad1);
    DeleteCriticalSection(&g_csPad0);
    DeleteCriticalSection(&g_csMutex);
    DeleteCriticalSection(&g_csList);
    g_threadingReady = 0;
}

// First-time TLS slots + six InitializeCriticalSection calls. Returns g_threadingReady.
// FUNCTION: VC1 0x0047b770
int InitThreading()
{
    TlsBlock* block;
    int status;

    if (g_threadingReady) {
        return 1;
    }

    g_threadingReady = 1;
    status = 0;
    block = static_cast<TlsBlock*>(calloc(1, sizeof(TlsBlock)));
    if (!block) {
        status = kKdEnomem;
    }
    else {
        block->index = TlsAlloc();
        if (block->index == TLS_OUT_OF_INDEXES) {
            status = kKdEagain;
            free(block);
            block = 0;
        }
    }
    g_tlsSlot0 = block;

    if (status == 0) {
        block = static_cast<TlsBlock*>(calloc(1, sizeof(TlsBlock)));
        if (!block) {
            status = kKdEnomem;
        }
        else {
            block->index = TlsAlloc();
            status = 0;
            if (block->index == TLS_OUT_OF_INDEXES) {
                status = kKdEagain;
                free(block);
                block = 0;
            }
        }
        g_tlsSlot1 = block;
        if (status != 0) {
            ShutdownThreading();
        }
    }
    else {
        ShutdownThreading();
    }

    InitializeCriticalSection(&g_csList);
    InitializeCriticalSection(&g_csMutex);
    InitializeCriticalSection(&g_csPad0);
    InitializeCriticalSection(&g_csPad1);
    InitializeCriticalSection(&g_csPad2);
    InitializeCriticalSection(&g_csPad3);
    return static_cast<int>(g_threadingReady);
}

// Preserve GetLastError/WSAGetLastError around TlsGetValue, then QueueUserAPCEx_Fini.
// FUNCTION: VC1 0x0047b890
int ShutdownApcAndLibraries()
{
    if (!g_threadingReady) {
        return 1;
    }

    if (g_tlsSlot0) {
        const DWORD lastError = GetLastError();
        const int wsaError = WSAGetLastError();
        void* const tls = TlsGetValue(g_tlsSlot0->index);
        SetLastError(lastError);
        WSASetLastError(wsaError);
        if (tls && *reinterpret_cast<int*>(static_cast<unsigned char*>(tls) + 0x24) == 1) {
            DestroyThreadLocal(*reinterpret_cast<void**>(static_cast<unsigned char*>(tls) + 8));
            TlsSetValue(g_tlsSlot0->index, 0);
        }
    }

    ShutdownThreading();

    if (g_quserex) {
        const FARPROC fini = GetProcAddress(g_quserex, "QueueUserAPCEx_Fini");
        if (fini) {
            fini();
        }
        FreeLibrary(g_quserex);
        g_quserex = 0;
    }
    if (g_kernel32Sync) {
        FreeLibrary(g_kernel32Sync);
        g_kernel32Sync = 0;
    }
    return 1;
}

// GetProcAddress miss for QueueUserAPCEx. Stored at DAT_004ed52c.
// pthreads-win32 pthread_create, specialised by LTCG for the console's stdin
// reader: thread record from 0x0047a550, _beginthreadex suspended on
// 0x0047b600, ResumeThread; EAGAIN (0x0b) on failure. Not recovered yet (the
// thread library should be vendored); failing leaves the console without a
// reader thread.
// STUB: VC1 0x0047ba20
int CreatePthread(PthreadHandle* thread, void* start)
{
    (void)thread;
    (void)start;
    return kKdEagain;
}

// STUB: VC1 0x0047bb80
static DWORD WINAPI ApcFallback()
{
    return 0;
}

// Count affinity bits, LoadLibrary KERNEL32.DLL / QUSEREX.DLL, bind ICE and QueueUserAPCEx.
// pthreads-win32 pthread_join (ESRCH 3 for an unknown thread, EDEADLK 0x24
// for self). Not recovered yet, like CreatePthread.
// STUB: VC1 0x0047bbd0
int JoinPthread(void* thread, unsigned reuse)
{
    (void)thread;
    (void)reuse;
    return 3;
}

// FUNCTION: VC1 0x0047bc70
int LoadSyncLibraries()
{
    DWORD_PTR processMask = 0;
    DWORD_PTR systemMask = 0;
    unsigned bit;
    int cpus;

    const int ready = InitThreading();
    g_cpuFlags = 0;
    if (!GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask)) {
        g_multiCpu = 0;
    }
    else {
        cpus = 0;
        bit = 1;
        do {
            if (systemMask & bit) {
                cpus += 1;
            }
            bit *= 2;
        } while (bit != 0);
        g_multiCpu = (cpus > 1) ? 1 : 0;
    }

    g_kernel32Sync = LoadLibraryA("KERNEL32.DLL");
    g_interlockedCompareExchange = GetProcAddress(g_kernel32Sync, "InterlockedCompareExchange");
    if (!g_interlockedCompareExchange) {
        g_interlockedCompareExchange = reinterpret_cast<FARPROC>(&IceFallback);
        FreeLibrary(g_kernel32Sync);
        g_kernel32Sync = 0;
    }
    else {
        g_cpuFlags |= 1;
    }

    g_quserex = LoadLibraryA("QUSEREX.DLL");
    if (g_quserex) {
        g_queueUserApcEx = GetProcAddress(g_quserex, "QueueUserAPCEx");
    }
    if (!g_queueUserApcEx) {
        g_queueUserApcEx = reinterpret_cast<FARPROC>(&ApcFallback);
        if (g_quserex) {
            FreeLibrary(g_quserex);
        }
        g_quserex = 0;
        return ready;
    }

    {
        const FARPROC init = GetProcAddress(g_quserex, "QueueUserAPCEx_Init");
        if (init) {
            const int ok = init();
            if (ok != 0) {
                if (g_quserex) {
                    g_cpuFlags |= 2;
                }
                return ready;
            }
        }
    }

    g_queueUserApcEx = reinterpret_cast<FARPROC>(&ApcFallback);
    FreeLibrary(g_quserex);
    g_quserex = 0;
    return ready;
}

// LoadSyncLibraries, AllocTlsBlock(0x0047a7a0, &g_tlsSecondary), calloc/TlsAlloc for primary.
// FUNCTION: VC1 0x0047bdf0
void AllocPrimaryTls()
{
    TlsBlock* block;

    LoadSyncLibraries();
    AllocTlsBlock(reinterpret_cast<void*>(&TlsMutexDestructor), &g_tlsSecondary);

    block = static_cast<TlsBlock*>(calloc(1, sizeof(TlsBlock)));
    if (block) {
        block->index = TlsAlloc();
        if (block->index == TLS_OUT_OF_INDEXES) {
            free(block);
            g_tlsPrimary = 0;
            return;
        }
    }
    g_tlsPrimary = block;
}

}
