#include "kdcore/runtime.h"

#include <squirrel.h>

namespace vc1 {

// The two KD thread keys (pthread_key_create in AllocPrimaryTls). The
// pthreads-win32 globals around them are listed in config/verified.csv.
// GLOBAL: VC1 0x004ed530
pthread_key_t g_kdUserKey;
// GLOBAL: VC1 0x004ed534
pthread_key_t g_kdThreadKey;

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
KdRef<IUnknown> g_serviceUnknown;

// GLOBAL: VC1 0x004e89a8
HMODULE g_dbghelp = reinterpret_cast<HMODULE>(-1);
// GLOBAL: VC1 0x004ef93c
FARPROC g_MiniDumpWriteDump;
// GLOBAL: VC1 0x004ef940
FARPROC g_SymInitialize;
// GLOBAL: VC1 0x004ef950
FARPROC g_SymSetOptions;
// GLOBAL: VC1 0x004ef958
FARPROC g_SymGetOptions;
// GLOBAL: VC1 0x004ef928
FARPROC g_SymCleanup;
// GLOBAL: VC1 0x004ef930
FARPROC g_SymGetTypeInfo;
// GLOBAL: VC1 0x004ef92c
FARPROC g_SymGetLineFromAddr64;
// GLOBAL: VC1 0x004ef94c
FARPROC g_SymEnumSymbols;
// GLOBAL: VC1 0x004ef954
FARPROC g_SymSetContext;
// GLOBAL: VC1 0x004ef938
FARPROC g_SymFromAddr;
// GLOBAL: VC1 0x004ef934
FARPROC g_StackWalk64;
// GLOBAL: VC1 0x004ef944
FARPROC g_SymFunctionTableAccess64;
// GLOBAL: VC1 0x004ef948
FARPROC g_SymGetModuleBase64;

}
