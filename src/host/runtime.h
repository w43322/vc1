#ifndef VC1_HOST_RUNTIME_H
#define VC1_HOST_RUNTIME_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <stdarg.h>

#include "bindings/squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

// Inferred names; not original symbols. Layout from WinMain callees.

struct TlsBlock
{
    DWORD index;     // +0x00 TlsAlloc result
    void* destructor; // +0x04, set when AllocTlsBlock gets a callback
    DWORD cookie;    // +0x08 0xffffffff when destructor is set
    void* waiters;   // +0x0c list head used by DestroyTlsBlock
};

typedef char Vc1TlsBlockSizeCheck[sizeof(TlsBlock) == 0x10 ? 1 : -1];

struct SocketNode
{
    SOCKET socket;      // +0x00
    unsigned unused04;  // +0x04
    SocketNode* prev;   // +0x08
    SocketNode* next;   // +0x0c
};

struct HeapNode
{
    unsigned pad[8];
    HeapNode* next;   // +0x20
    HeapNode* prev;   // +0x24
    void* owner;      // +0x28 compared to EBX in 0x004963b0
};

struct KdObject
{
    KdObject* prev; // +0x00
    KdObject* next; // +0x04
    unsigned pad[0x42];
    void (*destructor)(KdObject* self); // +0x110, index 0x44 of int*
    unsigned tail[0x12];
};

typedef char Vc1KdObjectSizeCheck[sizeof(KdObject) == 0x15c ? 1 : -1];

struct Mutex
{
    LONG state;          // +0x00
    LONG recursion;      // +0x04
    LONG kind;           // +0x08 0 = simple, else recursive
    DWORD ownerLo;       // +0x0c
    DWORD ownerHi;       // +0x10
    HANDLE event;        // +0x14
};

struct MutexRef
{
    Mutex* mutex;
};

enum {
    kKdEinval = 0x16,
    kKdEnomem = 0x0c,
    kKdEagain = 0x0b,
    kKdEbusy = 0x10,
    kKdEdeadlk = 0x24
};

extern TlsBlock* g_tlsPrimary;    // 0x004ed530
extern TlsBlock* g_tlsSecondary;  // 0x004ed534
extern TlsBlock* g_tlsSlot0;      // 0x004ed51c
extern TlsBlock* g_tlsSlot1;      // 0x004ed520
extern unsigned g_threadingReady; // 0x004ed518
extern unsigned g_cpuFlags;       // 0x004ed524
extern unsigned g_multiCpu;       // 0x004e89bc
extern HMODULE g_quserex;         // 0x004eb0e8
extern HMODULE g_kernel32Sync;    // 0x004eb0ec
extern FARPROC g_interlockedCompareExchange; // 0x004ed528
extern FARPROC g_queueUserApcEx;  // 0x004ed52c
extern CRITICAL_SECTION g_csList;     // 0x004ef9c0
extern CRITICAL_SECTION g_csMutex;    // 0x004efa00
extern CRITICAL_SECTION g_csPad0;     // 0x004ef980
extern CRITICAL_SECTION g_csPad1;     // 0x004ef9e0
extern CRITICAL_SECTION g_csPad2;     // 0x004ef9a0
extern CRITICAL_SECTION g_csPad3;     // 0x004ef960
extern void* g_threadNodeSentinel; // 0x004e89b4, walk stops at 1

extern SocketNode* g_socketHead;  // 0x004ed510
extern SocketNode* g_socketCursor; // 0x004ed50c
extern HWND g_kdNetworkWindow;    // 0x004ed514
extern WNDCLASSA g_kdNetworkClass; // 0x004e89d8; hInstance at +0x10 is 0x004e89e8

extern double g_nsPerTick;        // 0x004f0a20, 1e9 / QPC frequency
extern HeapNode* g_heapHead;      // 0x004ed448
extern HeapNode* g_heapTail;      // 0x004ed44c
extern unsigned* g_bumpCursor;    // 0x004ed144
extern unsigned char g_bumpArena[0x300]; // 0x004ed148..0x004ed447
extern void* g_heapPageList;      // 0x004e95c8
extern void* g_heapPageSentinel;  // 0x004e918c

extern KdObject* g_kdObjects;     // 0x004ed4f8
extern KdObject* g_kdObjectIt0;   // 0x004ed4f0
extern KdObject* g_kdObjectIt1;   // 0x004ed4e8
extern KdObject* g_kdObjectIt2;   // 0x004ed4e0
extern void* g_kdPath0;           // 0x004ed4e4
extern void* g_kdPath1;           // 0x004ed4ec
extern void* g_kdPath2;           // 0x004ed4f4
extern char g_resourceLocale[8];  // 0x004ed504
extern char g_scriptPrintBuf[0x400];  // 0x004ed9d0
extern HSQOBJECT g_sqNull;        // 0x004e9740
extern SquirrelObject g_rootTable; // 0x004e96e0, type at 0x004e96e4
extern char g_windowsVersionText[0x80]; // 0x004eb050
extern unsigned g_runScratch0;    // 0x004e9d94
extern unsigned g_runScratch1;    // 0x004e9d98
extern void* g_serviceUnknown;    // 0x004ed660

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

void* UnmappedAllocEvent();
void UnmappedVectorGrow(int newSize, char zero);
void* UnmappedVectorInsert(void* factory, void* pos, int bytes);
unsigned long long UnmappedQpcNow();
void UnmappedConstructDisplayBase(void* self);
void* UnmappedDisplayBaseQueryInterface(void* self, const char** iid);
void UnmappedConstructCompoundBase(void* self);
void UnmappedRegisterPrint(HSQUIRRELVM vm, void* fn);
void UnmappedRegisterPrintAlt(HSQUIRRELVM vm, void* fn);
void UnmappedRegisterExtra0();
void UnmappedRegisterExtra1();
void UnmappedRegisterExtra2();
void UnmappedRegisterExtra3();
void UnmappedRegisterExtra4();
void UnmappedBindHostClasses();
void UnmappedCompilerError();
void UnmappedSharedStateCtor(void* ss);
void __stdcall UnmappedSharedStateInit(void* ss);
void UnmappedVmCtor(HSQUIRRELVM vm, void* ss);
int __stdcall UnmappedVmInit(HSQUIRRELVM vm, HSQUIRRELVM friendvm);
void UnmappedVmDelete(HSQUIRRELVM vm);
void UnmappedWrongArgType(HSQUIRRELVM vm, HSQOBJECT* obj);
int UnmappedLoadBytecode(HSQUIRRELVM vm);
void* UnmappedCreateString(void* strings, const SQChar* name);
void UnmappedNewSlot(HSQUIRRELVM vm, HSQOBJECT* self, HSQOBJECT* key, HSQOBJECT* val,
    SQBool bstatic);
void UnmappedRaiseFmt(const char* text);
SQInteger UnmappedGetStringIdx(HSQUIRRELVM vm, SQInteger idx, const SQChar** out);
void UnmappedPrintCallstack(HSQUIRRELVM vm);
void UnmappedTableCtor(void* table, void* sharedstate, SQInteger initialSize);
void* UnmappedRefLookup(void* refsTable, HSQOBJECT* obj, unsigned* index, void** prev,
    int create);
void UnmappedClearRefSlot(unsigned* slot);
void UnmappedReleaseRefCounted(int* object);
int UnmappedSqCall(HSQUIRRELVM vm, SQInteger nparams);
int UnmappedSqCompile(HSQUIRRELVM vm, SQLEXREADFUNC read, SQUserPointer p,
    const SQChar* sourcename, int raiseerror);
SQInteger UnmappedConsoleBufferRead(SQUserPointer p);
int UnmappedGet(HSQUIRRELVM vm);
void UnmappedBindHostApi();
void UnmappedShutdownHost();
int UnmappedFindFirstFile(const char* path, WIN32_FIND_DATAA* data);
char* UnmappedCopyExeDir(char* dest);
void* UnmappedCrtPathSlot();
int UnmappedKdConstruct(KdObject* object);
int UnmappedKdConstructNamed(KdObject* object);
int UnmappedKdConstructFail(KdObject* object);
void UnmappedDirClose(void* dir);
int UnmappedTryLoadDbgHelpSoft();
void UnmappedLogOsInfo();
void UnmappedLogStackWalk();
void* UnmappedOpenDumpFile(const char* path);
int UnmappedMiniDumpCallback();
int UnmappedMutexCheck(MutexRef* ref);
unsigned long long UnmappedThreadId64();
void UnmappedMutexDestroyBody(void* mutex);
int UnmappedMutexLockInner(MutexRef* ref);
void UnmappedQueueEvent(void* node);
int UnmappedWaitQueue();
unsigned long long UnmappedQpcDelta();
void UnmappedSimpleLockFallback();
void UnmappedApcFallback();
LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* info);

// 0x00463b40; original EAX=argc, EBX=argv from g_runScratch0/1
void InitScriptEngine(int argc, char** argv);
// 0x00482160; EDI=haystack, EBX=needle, ECX=haystackLen, cdecl needleLen
char* FindBytes(const char* haystack, unsigned haystackLen, const char* needle,
    unsigned needleLen);
// 0x00486eb0; cdecl path, mode in EBX
void* OpenKdPath(const char* path, const char* mode);
// 0x00487140; cdecl path + mode
void* OpenKdFile(const char* path, const char* mode);
// 0x00476e60
int ProbeDebuggerPresent();
// 0x00477240
int LoadDbgHelp();
// 0x00477650
void LogCrashHeader();
// 0x00477a90
int WriteCrashDump();
// 0x0047a270; original: destructor=[esp+4], out=EDI
int AllocTlsBlock(void* destructor, TlsBlock** out);
// 0x0047a600; original this in EAX
void UnlinkWaiter(void* node);
// 0x0047a7c0; original this in EAX
int MutexUnlock(MutexRef* ref);
// 0x0047a840; original this in EAX
int MutexLock(MutexRef* ref);
// 0x0047b0a0; original this in EDI
int MutexDestroy(MutexRef* ref);
// 0x0047b2f0
void DestroyThreadLocal(void* thread);
// 0x0047b380; original this in EBX
int DestroyTlsBlock(TlsBlock* self);
// 0x0047b6b0
void ShutdownThreading();
// 0x0047b770
int InitThreading();
// 0x0047b890
int ShutdownApcAndLibraries();
// 0x0047bc70
int LoadSyncLibraries();
// 0x0047bdf0
void AllocPrimaryTls();
// 0x0047c100
void ShutdownNetwork();
// 0x0047c190
LRESULT CALLBACK KdNetworkWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
// 0x0047c3c0
void StartupNetwork();
// 0x00486aa0
void InitTimingAndSeh();
// 0x00486c40; original this in ESI
void* DirRead(unsigned char* dir);
// 0x00486f50; original name in EDI
KdObject* CreateKdObject(int (*ctor)(KdObject* object));
// 0x00487200
void DestroyKdObjects();
// 0x00487280
unsigned char* DirOpen();
// 0x00487450
void InitKdFilesystem();
// 0x004963b0; original owner in EBX
void UnlinkHeapByOwner(void* owner);
// 0x00496580
void FreeBumpHeaps();
// 0x00496840
void* PopTimedEvent();
// 0x00496950; original name in ESI
const char* GetKdEnv(const char* name);
// 0x004969c0; original EDX=cap, stack buf/fmt/varargs; ECX unused leftover
void FormatBuffer(char* buf, unsigned cap, const char* fmt, ...);
// 0x00496a40
void KdLog(const char* fmt, ...);
// 0x004b7010
void HostLogV(const char** prefix, const char* fmt, va_list args);
// 0x004b7070
void HostLog(const char* fmt, ...);
// 0x00499e00; SQPRINTFUNCTION, dest 0x004ed9d0 then KdLog
void ScriptPrint(HSQUIRRELVM v, const SQChar* fmt, ...);
// 0x00499fe0
SquirrelObject* PushRootTable();
// 0x0049a170
void CloseScriptVm();
// 0x0049a510
int OpenScriptVm();
// 0x0049ac80; original dest in ECX
void ResetSquirrelObject(SquirrelObject* object);
// 0x0049b1d0; EAX=table, EDI=value, stdcall name, RET 4
SQBool __stdcall RegisterObjectSlot(SquirrelObject* table, const SQChar* name,
    SquirrelObject* value);
// 0x0049b250; EAX=table, stdcall index + name, RET 8
SQBool __stdcall BindIndex(SquirrelObject* table, SQInteger index, const SQChar* name);
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
