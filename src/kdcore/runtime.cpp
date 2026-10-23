#include "kdcore/runtime.h"

#include <squirrel.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed530
TlsBlock* g_tlsPrimary;
// GLOBAL: VC1 0x004ed534
TlsBlock* g_tlsSecondary;
// GLOBAL: VC1 0x004ed51c
TlsBlock* g_tlsSlot0;
// GLOBAL: VC1 0x004ed520
TlsBlock* g_tlsSlot1;
// GLOBAL: VC1 0x004ed518
unsigned g_threadingReady;
// GLOBAL: VC1 0x004ed524
unsigned g_cpuFlags;
// GLOBAL: VC1 0x004e89bc
unsigned g_multiCpu;
// GLOBAL: VC1 0x004eb0e8
HMODULE g_quserex;
// GLOBAL: VC1 0x004eb0ec
HMODULE g_kernel32Sync;
// GLOBAL: VC1 0x004ed528
FARPROC g_interlockedCompareExchange;
// GLOBAL: VC1 0x004ed52c
FARPROC g_queueUserApcEx;
// GLOBAL: VC1 0x004ef9c0
CRITICAL_SECTION g_csList;
// GLOBAL: VC1 0x004efa00
CRITICAL_SECTION g_csMutex;
// GLOBAL: VC1 0x004ef980
CRITICAL_SECTION g_csPad0;
// GLOBAL: VC1 0x004ef9e0
CRITICAL_SECTION g_csPad1;
// GLOBAL: VC1 0x004ef9a0
CRITICAL_SECTION g_csPad2;
// GLOBAL: VC1 0x004ef960
CRITICAL_SECTION g_csPad3;
// GLOBAL: VC1 0x004e89b4
void* g_threadNodeSentinel = reinterpret_cast<void*>(1);

// GLOBAL: VC1 0x004ed510
SocketNode* g_socketHead;
// GLOBAL: VC1 0x004ed50c
SocketNode* g_socketCursor;
// GLOBAL: VC1 0x004ed514
HWND g_kdNetworkWindow;

// STRING: VC1 0x004d4428
static const char kKdNetworkClassName[] = "[kdNetwork]";

// Original WNDCLASSA at 0x004e89d8: style 0, WndProc 0x0047c190, class name 0x004d4428.
// GLOBAL: VC1 0x004e89d8
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

// GLOBAL: VC1 0x004f0a20
double g_nsPerTick;
// GLOBAL: VC1 0x004ed144
KdEventQueue g_kdEventQueue;

// GLOBAL: VC1 0x004ed4f8
KdObject* g_kdObjects;
// GLOBAL: VC1 0x004ed4f0
KdObject* g_kdObjectIt0;
// GLOBAL: VC1 0x004ed4e8
KdObject* g_kdObjectIt1;
// GLOBAL: VC1 0x004ed4e0
KdObject* g_kdObjectIt2;
// GLOBAL: VC1 0x004ed4e4
void* g_kdPath0;
// GLOBAL: VC1 0x004ed4ec
void* g_kdPath1;
// GLOBAL: VC1 0x004ed4f4
void* g_kdPath2;

// GLOBAL: VC1 0x004ed504
char g_resourceLocale[8];

// Scratch for ScriptPrint (SQPRINTFUNCTION). Size not pinned by a second xref.
// GLOBAL: VC1 0x004ed9d0
char g_scriptPrintBuf[0x400];

// GLOBAL: VC1 0x004e9740
HSQOBJECT g_sqNull = { OT_NULL, { 0 } };

// Its atexit destructor is 0x004ca490.
// GLOBAL: VC1 0x004e96e0
SquirrelObject g_rootTable;
// GLOBAL: VC1 0x004eb050
char g_windowsVersionText[0x80];
// GLOBAL: VC1 0x004e9d94
unsigned g_runScratch0;
// GLOBAL: VC1 0x004e9d98
unsigned g_runScratch1;
// GLOBAL: VC1 0x004ed660
void* g_serviceUnknown;

// GLOBAL: VC1 0x004e89a8
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
