#include "platform/sdl3/runtime.h"
#include <cstddef>
namespace vc1 {
double g_nsPerTick;
KdEventPool g_kdEventPool;
static_assert(offsetof(KdEventPool,head)==sizeof(g_kdEventPool.arena),"pool before queue head");
KdEvent* g_bumpCursor;
KdObject* g_kdObjects;
char g_resourceLocale[8];
char g_scriptPrintBuf[0x400];
char g_windowsVersionText[0x80];
HSQOBJECT g_sqNull={OT_NULL,{0}};
SquirrelObject g_rootTable;
unsigned g_runScratch0,g_runScratch1;
void* g_serviceUnknown;
}
