#include "kdcore/runtime.h"

#include <stdlib.h>

namespace vc1 {

// The KD threads over pthreads-win32 2.8.0 (third_party/pthreads-w32-2-8-0,
// mapped in pthreads.h there). Only these three bodies are KD's own; the
// library is linked around them.

// g_kdThreadKey's destructor: a finished thread frees its record when asked.
// FUNCTION: VC1 0x0047a7a0
static void KdThreadDestroy(void* thread)
{
    if (static_cast<KdThread*>(thread)->autoFree) {
        free(thread);
    }
}

// The start routine of every KdThread: publishes the record, runs it and
// detaches the thread from pthreads-win32 (the static library needs that).
// FUNCTION: VC1 0x0047b850
void* KdThreadStart(void* thread)
{
    KdThread* const record = static_cast<KdThread*>(thread);
    void* result;

    pthread_setspecific(g_kdThreadKey, record);
    result = record->proc(record->arg);
    pthread_win32_thread_detach_np();
    return result;
}

// ESI=thread. Inferred name (kdThreadJoin). Joins and frees the record;
// -1 if the join fails. No callers in the game.
// FUNCTION: VC1 0x0047bdc0
int KdThreadJoin(KdThread* thread, void** result)
{
    if (pthread_join(thread->handle, result)) {
        return -1;
    }
    free(thread);
    return 0;
}

// Called first by InitTimingAndSeh: the static pthreads-win32 needs its
// process attach, then the two KD keys.
// FUNCTION: VC1 0x0047bdf0
void AllocPrimaryTls()
{
    pthread_win32_process_attach_np();
    pthread_key_create(&g_kdThreadKey, KdThreadDestroy);
    pthread_key_create(&g_kdUserKey, 0);
}

}
