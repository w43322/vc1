#include "platform/sdl3/runtime.h"
namespace vc1 {
static SDL_SpinLock mutexInitLock;
int AllocTlsBlock(void* destructor,TlsBlock** out)
{
    *out=new TlsBlock{SDL_TLSID{},destructor};
    if(!SDL_SetTLS(&(*out)->id,nullptr,nullptr)) {delete *out;*out=nullptr;return kKdEnomem;}
    return 0;
}
int DestroyTlsBlock(TlsBlock* block)
{
    if(!block) return 0;
    void* value=SDL_GetTLS(&block->id);
    if(value && block->destructor) reinterpret_cast<void(*)(void*)>(block->destructor)(value);
    SDL_SetTLS(&block->id,nullptr,nullptr);delete block;return 0;
}
void DestroyThreadLocal(void*) {SDL_CleanupTLS();}
int MutexLock(MutexRef* ref)
{
    if(!ref) return kKdEinval;
    SDL_LockSpinlock(&mutexInitLock);
    if(!ref->mutex) ref->mutex=SDL_CreateMutex();
    SDL_UnlockSpinlock(&mutexInitLock);
    if(!ref->mutex) return kKdEnomem;SDL_LockMutex(ref->mutex);return 0;
}
int MutexUnlock(MutexRef* ref) {if(!ref || !ref->mutex) return kKdEinval;SDL_UnlockMutex(ref->mutex);return 0;}
int MutexDestroy(MutexRef* ref) {if(ref) {SDL_DestroyMutex(ref->mutex);ref->mutex=nullptr;}return 0;}
int InitThreading() {return 0;}
void ShutdownThreading() {SDL_CleanupTLS();}
}
