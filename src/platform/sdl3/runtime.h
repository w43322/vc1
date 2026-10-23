#ifndef VC1_PLATFORM_SDL3_RUNTIME_H
#define VC1_PLATFORM_SDL3_RUNTIME_H
#include "host/platform.h"
#include "host/window.h"
#include "app/script.h"
#include "bindings/squirrel_object.h"
#include "memory/fixed_pool.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
namespace vc1 {
struct TlsBlock { SDL_TLSID id; void* destructor; };
struct MutexRef { SDL_Mutex* mutex; };
extern double g_nsPerTick;
// The original keeps the 16-event pool (0x004ed148) directly before the queue
// head; AllocEvent ends its pool scan at &g_heapHead, so here they are one
// object with the original names as aliases.
struct KdEventPool { alignas(8) unsigned char arena[0x300]; KdEvent* head; KdEvent* tail; };
extern KdEventPool g_kdEventPool;
inline unsigned char (&g_bumpArena)[0x300]=g_kdEventPool.arena;
inline KdEvent*& g_heapHead=g_kdEventPool.head;
inline KdEvent*& g_heapTail=g_kdEventPool.tail;
extern KdEvent* g_bumpCursor;
extern KdCallback* g_kdCallbacks;
extern KdCallback g_kdDefaultCallback;
extern KdObject* g_kdObjects;
extern char g_resourceLocale[8];
extern char g_scriptPrintBuf[0x400];
extern char g_windowsVersionText[0x80];
extern HSQOBJECT g_sqNull;
extern unsigned g_runScratch0, g_runScratch1;
extern void* g_serviceUnknown;
extern const char kVirtualCity[];
extern const char kKdDataDir[];
extern const char kPercentS[];
void ScriptPrint(HSQUIRRELVM v,const SQChar* fmt,...);
int RunApp();
char* FindBytes(const char* haystack,unsigned haystackLen,const char* needle,unsigned needleLen);
void SdlPollEvents();
void PollGamepad();
void FireTimers();
void DropMatchingEvents(const KdEvent* event);
void SdlUpdateAudio();
}
#endif
