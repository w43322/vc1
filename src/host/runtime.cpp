#include "host/runtime.h"

#include <squirrel.h>

namespace vc1 {

TlsBlock* g_tlsPrimary;
TlsBlock* g_tlsSecondary;
TlsBlock* g_tlsSlot0;
TlsBlock* g_tlsSlot1;
unsigned g_threadingReady;
unsigned g_cpuFlags;
unsigned g_multiCpu;
HMODULE g_quserex;
HMODULE g_kernel32Sync;
FARPROC g_interlockedCompareExchange;
FARPROC g_queueUserApcEx;
CRITICAL_SECTION g_csList;
CRITICAL_SECTION g_csMutex;
CRITICAL_SECTION g_csPad0;
CRITICAL_SECTION g_csPad1;
CRITICAL_SECTION g_csPad2;
CRITICAL_SECTION g_csPad3;
void* g_threadNodeSentinel = reinterpret_cast<void*>(1);

SocketNode* g_socketHead;
SocketNode* g_socketCursor;
HWND g_kdNetworkWindow;

static const char kKdNetworkClassName[] = "[kdNetwork]";

// Original 0x004e89d8: style 0, WndProc 0x0047c190, class name 0x004d4428.
WNDCLASSA g_kdNetworkClass = {
    0,
    KdNetworkWndProc,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    kKdNetworkClassName
};

double g_nsPerTick;
HeapNode* g_heapHead;
HeapNode* g_heapTail;
unsigned* g_bumpCursor;
unsigned char g_bumpArena[0x300];
void* g_heapPageList;
void* g_heapPageSentinel;

KdObject* g_kdObjects;
KdObject* g_kdObjectIt0;
KdObject* g_kdObjectIt1;
KdObject* g_kdObjectIt2;
void* g_kdPath0;
void* g_kdPath1;
void* g_kdPath2;

// Original 0x004e96e0: vftable 0x004cfb98, type OT_NULL, value 0.
SquirrelObject g_rootTable = {
    0,
    { OT_NULL, { 0 } }
};
char g_windowsVersionText[0x80];
unsigned g_runScratch0;
unsigned g_runScratch1;
void* g_serviceUnknown;

HMODULE g_dbghelp = reinterpret_cast<HMODULE>(-1);
FARPROC g_MiniDumpWriteDump;
FARPROC g_SymInitialize;
FARPROC g_SymSetOptions;
FARPROC g_SymGetOptions;
FARPROC g_SymCleanup;
FARPROC g_SymGetTypeInfo;
FARPROC g_SymGetLineFromAddr64;
FARPROC g_SymEnumSymbols;
FARPROC g_SymSetContext;
FARPROC g_SymFromAddr;
FARPROC g_StackWalk64;
FARPROC g_SymFunctionTableAccess64;
FARPROC g_SymGetModuleBase64;

}
