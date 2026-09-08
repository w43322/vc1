#include "host/runtime.h"

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// version.dll GetFileVersionInfo* on dbghelp.dll; 1 if file version >= 6.6.7.5.
// STUB: VC1 0x00476d80
static int TryLoadDbgHelpSoft()
{
    return 1;
}

// LoadLibraryA("kernel32.dll") + IsDebuggerPresent. Returns 1 if a debugger is attached.
// FUNCTION: VC1 0x00476e60
int ProbeDebuggerPresent()
{
    const HMODULE kernel32 = LoadLibraryA("kernel32.dll");
    const FARPROC isDebuggerPresent = GetProcAddress(kernel32, "IsDebuggerPresent");
    if (isDebuggerPresent) {
        const int present = isDebuggerPresent();
        if (present) {
            FreeLibrary(kernel32);
            return 1;
        }
    }
    FreeLibrary(kernel32);
    return 0;
}

// Command line, OS version, and host info dumped through KdLog.
// STUB: VC1 0x00476fd0
static void LogOsInfo()
{
}

// Load dbghelp.dll and resolve MiniDump/Sym* exports. Original compares each pointer to 0.
// FUNCTION: VC1 0x00477240
int LoadDbgHelp()
{
    if (!TryLoadDbgHelpSoft()) {
        return 0;
    }

    g_dbghelp = LoadLibraryA("dbghelp.dll");
    if (g_dbghelp == reinterpret_cast<HMODULE>(-1)) {
        return 0;
    }

    g_MiniDumpWriteDump = GetProcAddress(g_dbghelp, "MiniDumpWriteDump");
    g_SymInitialize = GetProcAddress(g_dbghelp, "SymInitialize");
    g_SymSetOptions = GetProcAddress(g_dbghelp, "SymSetOptions");
    g_SymGetOptions = GetProcAddress(g_dbghelp, "SymGetOptions");
    g_SymCleanup = GetProcAddress(g_dbghelp, "SymCleanup");
    g_SymGetTypeInfo = GetProcAddress(g_dbghelp, "SymGetTypeInfo");
    g_SymGetLineFromAddr64 = GetProcAddress(g_dbghelp, "SymGetLineFromAddr64");
    g_SymEnumSymbols = GetProcAddress(g_dbghelp, "SymEnumSymbols");
    g_SymSetContext = GetProcAddress(g_dbghelp, "SymSetContext");
    g_SymFromAddr = GetProcAddress(g_dbghelp, "SymFromAddr");
    g_StackWalk64 = GetProcAddress(g_dbghelp, "StackWalk64");
    g_SymFunctionTableAccess64 = GetProcAddress(g_dbghelp, "SymFunctionTableAccess64");
    g_SymGetModuleBase64 = GetProcAddress(g_dbghelp, "SymGetModuleBase64");

    if (g_MiniDumpWriteDump && g_SymInitialize && g_SymSetOptions && g_SymGetOptions
        && g_SymCleanup && g_SymGetTypeInfo && g_SymGetLineFromAddr64 && g_SymEnumSymbols
        && g_SymSetContext && g_SymFromAddr && g_SymGetModuleBase64 && g_StackWalk64
        && g_SymFunctionTableAccess64) {
        typedef unsigned (WINAPI *SymGetOptionsFn)();
        typedef unsigned (WINAPI *SymSetOptionsFn)(unsigned options);
        typedef int (WINAPI *SymInitializeFn)(HANDLE process, const char* search, int invade);
        const unsigned options =
            reinterpret_cast<SymGetOptionsFn>(g_SymGetOptions)();
        reinterpret_cast<SymSetOptionsFn>(g_SymSetOptions)(options | 0x14);
        reinterpret_cast<SymInitializeFn>(g_SymInitialize)(GetCurrentProcess(), 0, 1);
        return 1;
    }

    if (g_dbghelp) {
        FreeLibrary(g_dbghelp);
        g_dbghelp = 0;
    }
    return 0;
}

// Command line, OS version, and host info dumped through KdLog / 0x00476fd0.
// FUNCTION: VC1 0x00477650
void LogCrashHeader()
{
    OSVERSIONINFOA version;
    GetCommandLineA();
    KdLog("");
    KdLog("");
    memset(&version, 0, sizeof(version));
    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionExA(&version);
    KdLog("");
    LogOsInfo();
    KdLog("");
    KdLog("");
    KdLog("");
    KdLog("");
    KdLog("");
    (void)version;
}

// fopen-like "/data/kdCrashDump.dmp" then MiniDumpWriteDump. Original this dump info in EAX.
// FUNCTION: VC1 0x00477a90
int WriteCrashDump()
{
    FILE* const file = static_cast<FILE*>(OpenKdFile("/data/kdCrashDump.dmp", "wb"));
    HANDLE handle;
    if (!file) {
        return 0;
    }

    handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
    DWORD threadId = GetCurrentThreadId();
    (void)threadId;
    if (g_MiniDumpWriteDump) {
        typedef int (WINAPI *MiniDumpFn)(HANDLE process, DWORD pid, HANDLE file, unsigned dumpType,
            void* exception, void* user, void* callback);
        reinterpret_cast<MiniDumpFn>(g_MiniDumpWriteDump)(
            GetCurrentProcess(), GetCurrentProcessId(), handle, 0, 0, 0, 0);
    }
    free(file);
    return 1;
}

// StackWalk64 through the exception context. Original CONTEXT in EAX.
// STUB: VC1 0x00477b60
static void LogStackWalk()
{
}

// SetUnhandledExceptionFilter target. Original returns 1 after MessageBoxA, else 0.
// FUNCTION: VC1 0x00477cb0
LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* info)
{
    (void)info;
    if (g_dbghelp == reinterpret_cast<HMODULE>(-1) && ProbeDebuggerPresent() != 1
        && LoadDbgHelp() != 0) {
        KdLog("");
        LogCrashHeader();
        KdLog("");
        LogStackWalk();
        KdLog("");
        WriteCrashDump();
        if (g_SymCleanup) {
            typedef int (WINAPI *SymCleanupFn)(HANDLE process);
            reinterpret_cast<SymCleanupFn>(g_SymCleanup)(GetCurrentProcess());
        }
        FreeLibrary(g_dbghelp);
        MessageBoxA(GetActiveWindow(),
            "This application has halted due to an unexpected error.\n"
            "A crash report and minidump file were saved to disk:\n\n"
            "kdCrashDump.dmp\n"
            "kdLogMessage.txt\n\n"
            "Please send these files to developers to help us analyse\n"
            "and solve this problem.\n",
            "Exception",
            0x10);
        return 1;
    }
    return 0;
}

}
