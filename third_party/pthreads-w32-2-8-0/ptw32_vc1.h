// pthreads-win32 2.8.0 mappings for VirtualCity.Unwrapped.exe. Upstream
// files in this directory are unchanged (cmake/pthreads.cmake has the
// archive and its SHA-256); this file only names the original addresses.
//
// Version: 2.8.0, not 2.9.x. pthread_mutex_lock 0x0047a840 has no robust
// mutex branch (2.9.1 has one), and pthread_win32_process_attach_np
// 0x0047bc70 is 2.8.0's body (affinity count, KERNEL32.DLL
// InterlockedCompareExchange, QUSEREX.DLL QueueUserAPCEx). Build: the
// "nmake VC-static" configuration, i.e. the inlined pthread.c with
// __CLEANUP_C (ptw32_threadStart 0x0047b600 calls setjmp around the start
// routine and ends with _endthreadex), compiled with the game under /GL, so
// several helpers take register arguments.
//
// Identities: Ghidra pseudocode of each original body against the
// 2.8.0 source, ranked by instruction similarity to the recompiled bodies.
// KD's own functions in this range (KdThreadDestroy 0x0047a7a0, KdThreadStart
// 0x0047b850, AllocPrimaryTls 0x0047bdf0) are in src/kdcore/thread.cpp; the
// library's globals are config/verified.csv rows.

// FUNCTION: VC1 0x0047a230
// pthread_getspecific
// FUNCTION: VC1 0x0047a270
// pthread_key_create
// FUNCTION: VC1 0x0047a300
// pthread_mutex_init
// FUNCTION: VC1 0x0047a3a0
// ptw32_mutex_check_need_init
// FUNCTION: VC1 0x0047a490
// ptw32_threadReusePush
// FUNCTION: VC1 0x0047a500
// ptw32_threadReusePop
// FUNCTION: VC1 0x0047a550
// ptw32_new
// FUNCTION: VC1 0x0047a600
// ptw32_tkAssocDestroy
// FUNCTION: VC1 0x0047a660
// ptw32_tkAssocCreate
// FUNCTION: VC1 0x0047a720
// ptw32_InterlockedCompareExchange
// FUNCTION: VC1 0x0047a7c0
// pthread_mutex_unlock
// FUNCTION: VC1 0x0047a840
// pthread_mutex_lock
// FUNCTION: VC1 0x0047a990
// ptw32_semwait
// FUNCTION: VC1 0x0047aa50
// pthread_setspecific
// FUNCTION: VC1 0x0047ab70
// sem_post
// FUNCTION: VC1 0x0047ac10
// ptw32_sem_timedwait_cleanup
// FUNCTION: VC1 0x0047ac60
// ptw32_sem_wait_cleanup
// FUNCTION: VC1 0x0047aca0
// ptw32_setthreadpriority
// FUNCTION: VC1 0x0047ad30
// ptw32_rwlock_cancelwrwait
// FUNCTION: VC1 0x0047ad60
// pthread_self
// FUNCTION: VC1 0x0047ae50
// ptw32_cond_wait_cleanup
// FUNCTION: VC1 0x0047afb0
// ptw32_pop_cleanup_all
// FUNCTION: VC1 0x0047b020
// pthread_mutex_trylock
// FUNCTION: VC1 0x0047b0a0
// pthread_mutex_destroy
// FUNCTION: VC1 0x0047b170
// ptw32_callUserDestroyRoutines
// FUNCTION: VC1 0x0047b2f0
// ptw32_threadDestroy
// FUNCTION: VC1 0x0047b380
// pthread_key_delete
// FUNCTION: VC1 0x0047b430
// pthread_detach
// FUNCTION: VC1 0x0047b4e0
// pthread_win32_thread_detach_np
// FUNCTION: VC1 0x0047b590
// ptw32_throw
// FUNCTION: VC1 0x0047b600
// ptw32_threadStart
// FUNCTION: VC1 0x0047b6b0
// ptw32_processTerminate
// FUNCTION: VC1 0x0047b770
// ptw32_processInitialize
// FUNCTION: VC1 0x0047b890
// pthread_win32_process_detach_np
// FUNCTION: VC1 0x0047b950
// ptw32_cancelable_wait
// FUNCTION: VC1 0x0047ba20
// pthread_create
// FUNCTION: VC1 0x0047bb80
// ptw32_RegisterCancelation
// FUNCTION: VC1 0x0047bbd0
// pthread_join
// FUNCTION: VC1 0x0047bc70
// pthread_win32_process_attach_np
