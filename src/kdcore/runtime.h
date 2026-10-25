#ifndef VC1_KDCORE_RUNTIME_H
#define VC1_KDCORE_RUNTIME_H

#include "kdcore/native.h"
#include <pthread.h>
#include <stdarg.h>

#include "kdcore/platform.h"
#include "kdcore/window.h"
#include "app/script.h"

#include "bindings/squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

// Inferred names; not original symbols. Layout from WinMain callees.

struct SocketNode
{
    SOCKET socket;      // +0x00
    void* handler;      // +0x04 event callback, passed as the event userptr
    SocketNode* prev;   // +0x08
    SocketNode* next;   // +0x0c
};

// Inferred name. A KD thread: the pthread handle, then what KdThreadStart
// calls (proc(arg)); the thread key's destructor frees records that ask for
// it.
// SIZE 0x14
struct KdThread
{
    pthread_t handle;            // 0x00
    void* autoFree;              // 0x08  nonzero: freed by the key destructor
    void* (*proc)(void* arg);    // 0x0c
    void* arg;                   // 0x10
};

typedef char Vc1KdThreadSizeCheck[sizeof(KdThread) == 0x14 ? 1 : -1];

extern pthread_key_t g_kdUserKey;   // 0x004ed530
extern pthread_key_t g_kdThreadKey; // 0x004ed534, holds the running KdThread

extern SocketNode* g_socketHead;  // 0x004ed510
extern SocketNode* g_socketCursor; // 0x004ed50c
extern HWND g_kdNetworkWindow;    // 0x004ed514
extern WNDCLASSA g_kdNetworkClass; // 0x004e89d8; hInstance at +0x10 is 0x004e89e8

extern double g_nsPerTick;        // 0x004f0a20, 1e9 / QPC frequency
// Inferred name. The event queue block at 0x004ed144: AllocEvent scans the
// pool up to the queue head, so the four fields must stay one object.
// SIZE 0x30c
struct KdEventQueue
{
    KdEvent* cursor;            // +0x000 last event kdWaitEvent returned
    unsigned char arena[0x300]; // +0x004 16-event pool (kdCreateEvent)
    KdEvent* head;              // +0x304
    KdEvent* tail;              // +0x308
};

extern KdEventQueue g_kdEventQueue; // 0x004ed144
extern KdCallback* g_kdCallbacks; // 0x004e95c8
extern KdCallback g_kdDefaultCallback; // 0x004e918c

extern KdObject* g_kdObjects;     // 0x004ed4f8
extern KdObject* g_kdObjectIt0;   // 0x004ed4f0
extern KdObject* g_kdObjectIt1;   // 0x004ed4e8
extern KdObject* g_kdObjectIt2;   // 0x004ed4e0
extern void* g_kdPath0;           // 0x004ed4e4
extern void* g_kdPath1;           // 0x004ed4ec
extern void* g_kdPath2;           // 0x004ed4f4
extern char g_resourceLocale[8];  // 0x004ed504
extern char g_scriptPrintBuf[0x400];  // 0x004ed9d0
extern char g_windowsVersionText[0x80]; // 0x004eb050
extern unsigned g_runScratch0;    // 0x004e9d94
extern unsigned g_runScratch1;    // 0x004e9d98
extern KdRef<IUnknown> g_serviceUnknown;    // 0x004ed660

extern HMODULE g_dbghelp;         // 0x004e89a8, starts as -1
extern FARPROC g_MiniDumpWriteDump;
extern FARPROC g_SymInitialize;
extern FARPROC g_SymSetOptions;
extern FARPROC g_SymGetOptions;
extern FARPROC g_SymCleanup;
extern FARPROC g_SymGetTypeInfo;
extern FARPROC g_SymGetLineFromAddr64;
extern FARPROC g_SymEnumSymbols;
extern FARPROC g_SymSetContext;
extern FARPROC g_SymFromAddr;
extern FARPROC g_StackWalk64;
extern FARPROC g_SymFunctionTableAccess64;
extern FARPROC g_SymGetModuleBase64;

KdEvent* AllocEvent();
int QueueEvent(KdEvent* event);
// 0x004966b0; original event in EBX
void DropMatchingEvents(const KdEvent* event);
// 0x004babe0; XInput pad 0 to key events (0x38)
void PollGamepad();
LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* info);

// 0x00463b40; original EAX=argc, EBX=argv from g_runScratch0/1
void InitScriptEngine(int argc, char** argv);
// 0x004bf610; the value-type SQClassDefs (bindings/host_classes.cpp)
void BindHostClasses();
// 0x00482160; EDI=haystack, EBX=needle, ECX=haystackLen, cdecl needleLen
char* FindBytes(const char* haystack, unsigned haystackLen, const char* needle,
    unsigned needleLen);
// 0x00482220; ECX=haystack, EAX=needle
char* KdStrstr(const char* haystack, const char* needle);
// 0x00499c90; EAX=a, ECX=b
int KdStrcmp(const char* a, const char* b);
// 0x00486eb0; cdecl path, mode in EBX
void* OpenKdPath(const char* path, const char* mode);
// 0x00487140; cdecl path + mode
void* OpenKdFile(const char* path, const char* mode);
// 0x00486e20; ECX=file
int KdFread(void* buffer, unsigned size, unsigned count, void* file);
// 0x00486e70; ESI=file
int KdFclose(void* file);
// 0x004870c0; ESI=file, EBX=size out
void* KdMapFile(void* file, unsigned* size);
// 0x00476e60
int ProbeDebuggerPresent();
// 0x00477240
int LoadDbgHelp();
// 0x00477650; original record on the stack, context in ESI
void LogCrashHeader(const EXCEPTION_RECORD* record, const CONTEXT* context);
// 0x00477a90; original info in EAX
int WriteCrashDump(EXCEPTION_POINTERS* info);
// 0x0047bdf0; pthread_win32_process_attach_np and the two KD thread keys
void AllocPrimaryTls();
// 0x0047b850; the pthread start routine of every KdThread
void* KdThreadStart(void* thread);
// 0x0047c100
void ShutdownNetwork();
// 0x0047c190
LRESULT CALLBACK KdNetworkWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
// 0x0047c3c0
void StartupNetwork();
// 0x0047c480; original name in EDI, error in ESI
void LogLastError(const char* name, DWORD error);
// 0x00486aa0
void InitTimingAndSeh();
// 0x00486b90; kdStat, original path in EBX, out in EDI
int StatKdPath(const char* path, KdFileStat* out);
// 0x00486c40; original this in ESI
void* DirRead(unsigned char* dir);
// 0x0047cc90 / 0x0047ccc0; the file driver's map slots, imports/<target>/crt.cpp
void KdFileUnmap(KdObject* object, int handle, void* view);
void* KdFileMap(KdObject* object, int handle, unsigned* size);
// 0x0047cec0; original path in EAX
char* CopyExeDir(const char* path, char* dest);
// 0x004873c0; original oldPath in EDI, newPath in EAX
int KdRename(const char* oldPath, const char* newPath);
extern const char kKdDataDir[];   // "data/", startup.cpp
extern const char kVirtualCity[]; // "Virtual City", win_main.cpp
// 0x00486f50; original name in EDI
KdObject* CreateKdObject(int (*ctor)(KdObject* object), const char* name);
// 0x00499d90; original dest in EAX, src in EDX
int AppendPath(char* dest, const char* src);
// 0x00487200
void DestroyKdObjects();
// 0x00487280
unsigned char* DirOpen();
// 0x00487450
void InitKdFilesystem();
// 0x004963b0; original owner in EBX
void UnlinkHeapByOwner(void* owner);
// 0x00486830; kdCancelTimer's unlink, original timer in EDX
void UnlinkTimer(KdTimer* timer);
// 0x00486890; kdSetTimer with periodic folded to KD_TIMER_PERIODIC_AVERAGE
KdTimer* KdSetTimer(__int64 interval, void* userptr);
// 0x00486910; posts KD_EVENT_TIMER for every due timer
void FireTimers();
// 0x004964f0; kdInstallCallback, original ESI=func, EBX=type, EDI=userptr
int KdInstallCallback(KdCallbackFunc func, int type, void* userptr);
// 0x00496580
void FreeBumpHeaps();
// 0x00496840
KdEvent* PopTimedEvent();
// 0x00496950; original name in ESI
const char* GetKdEnv(const char* name);
extern const char kPercentS[]; // "%s", display.cpp
// 0x004969c0; original EDX=cap, stack buf/fmt/varargs; ECX unused leftover
int FormatBuffer(char* buf, unsigned cap, const char* fmt, ...);
// 0x004969f0; EDX=buf, stack fmt then varargs; cap is 0xffffffff
extern "C" int FormatBufferOpen(char* buf, const char* fmt, ...);
extern unsigned g_kdLogTag; // 0x004cfa94, also the empty counted-string bytes
// 0x00496a10; EBX=fmt, EDI=args
void KdLogV(const char* fmt, va_list args);
// 0x00496a40
void KdLog(const char* fmt, ...);
extern const char* g_kdLogPrefix; // 0x004d062c
// 0x004b7010
void HostLogV(const char** prefix, const char* tag, const char* fmt, va_list args);
// 0x004b7070
void HostLog(const char** prefix, const char* fmt, ...);
// 0x004b7090
void HostLogWarning(const char** prefix, const char* fmt, ...);
// 0x004b70b0
void HostLogError(const char** prefix, const char* fmt, ...);
// 0x004b70d0
void HostLogInfo(const char** prefix, const char* fmt, ...);
// 0x00499e00; SQPRINTFUNCTION, dest 0x004ed9d0 then KdLog
void ScriptPrint(HSQUIRRELVM v, const SQChar* fmt, ...);
// 0x004b61b0
void InitHostServices();
// 0x004bab00
const char* GetResourceLocale();
// 0x004bdc50
void RegisterStdBindings();
// 0x004c38b0
int RunApp();

}

#endif
