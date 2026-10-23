// kernel32.dll over SDL3: TLS, critical sections, interlocked operations,
// modules, the clock, locale and directory listing.
#include "sdl3.h"

#include <stdlib.h>
#include <string.h>

namespace {

const DWORD kTlsSlots = 64;
SDL_TLSID tlsSlots[kTlsSlots];
bool tlsUsed[kTlsSlots];
SDL_TLSID lastErrorSlot;

// LoadLibraryA only knows kernel32 itself, for the exports the game looks up.
HMODULE const kKernel32 = reinterpret_cast<HMODULE>(0x7c800000);
HMODULE const kExe = reinterpret_cast<HMODULE>(0x00400000);

struct Find
{
    char** names;
    int count;
    int next;
};

SDL_AtomicInt* Atomic(volatile LONG* target)
{
    return reinterpret_cast<SDL_AtomicInt*>(const_cast<LONG*>(target));
}

void FillFind(const Find* find, WIN32_FIND_DATAA* data)
{
    SDL_memset(data, 0, sizeof(*data));
    SDL_strlcpy(data->cFileName, find->names[find->next], sizeof(data->cFileName));
}

}

DWORD WINAPI TlsAlloc()
{
    for (DWORD i = 0; i < kTlsSlots; ++i) {
        if (!tlsUsed[i]) {
            tlsUsed[i] = true;
            return i;
        }
    }
    return TLS_OUT_OF_INDEXES;
}

BOOL WINAPI TlsFree(DWORD index)
{
    if (index >= kTlsSlots || !tlsUsed[index]) {
        return FALSE;
    }
    SDL_SetTLS(&tlsSlots[index], 0, 0);
    tlsUsed[index] = false;
    return TRUE;
}

void* WINAPI TlsGetValue(DWORD index)
{
    return index < kTlsSlots ? SDL_GetTLS(&tlsSlots[index]) : 0;
}

BOOL WINAPI TlsSetValue(DWORD index, void* value)
{
    return index < kTlsSlots && SDL_SetTLS(&tlsSlots[index], value, 0);
}

void WINAPI InitializeCriticalSection(CRITICAL_SECTION* section)
{
    section->mutex = SDL_CreateMutex();
}

void WINAPI DeleteCriticalSection(CRITICAL_SECTION* section)
{
    SDL_DestroyMutex(section->mutex);
    section->mutex = 0;
}

void WINAPI EnterCriticalSection(CRITICAL_SECTION* section)
{
    SDL_LockMutex(section->mutex);
}

void WINAPI LeaveCriticalSection(CRITICAL_SECTION* section)
{
    SDL_UnlockMutex(section->mutex);
}

LONG WINAPI InterlockedExchange(volatile LONG* target, LONG value)
{
    return SDL_SetAtomicInt(Atomic(target), value);
}

LONG WINAPI InterlockedCompareExchange(volatile LONG* target, LONG exchange, LONG comparand)
{
    for (;;) {
        const LONG current = SDL_GetAtomicInt(Atomic(target));
        if (current != comparand || SDL_CompareAndSwapAtomicInt(Atomic(target), current, exchange)) {
            return current;
        }
    }
}

// Only the console's queue mutex creates an event (KD_ENABLE_STDIO). Waiters
// spin on the mutex state, so the event never has to block or wake anyone.
HANDLE WINAPI CreateEventA(void* attributes, BOOL manualReset, BOOL initialState, LPCSTR name)
{
    (void)attributes;
    (void)manualReset;
    (void)initialState;
    (void)name;
    return reinterpret_cast<HANDLE>(1);
}

BOOL WINAPI SetEvent(HANDLE event)
{
    (void)event;
    return TRUE;
}

DWORD WINAPI WaitForSingleObject(HANDLE handle, DWORD milliseconds)
{
    (void)handle;
    (void)milliseconds;
    return 0;
}

BOOL WINAPI CloseHandle(HANDLE handle)
{
    (void)handle;
    return TRUE;
}

// The single-instance mutex: no other instance is reported.
HANDLE WINAPI CreateMutexA(void* attributes, BOOL owner, LPCSTR name)
{
    (void)attributes;
    (void)owner;
    (void)name;
    SetLastError(0);
    return reinterpret_cast<HANDLE>(1);
}

DWORD WINAPI GetLastError()
{
    return static_cast<DWORD>(reinterpret_cast<uintptr_t>(SDL_GetTLS(&lastErrorSlot)));
}

void WINAPI SetLastError(DWORD error)
{
    SDL_SetTLS(&lastErrorSlot, reinterpret_cast<void*>(static_cast<uintptr_t>(error)), 0);
}

// dbghelp, version and QUSEREX are absent, so crash dumps and the APC
// extension stay off, as on a system without them.
HMODULE WINAPI LoadLibraryA(LPCSTR name)
{
    return SDL_strcasecmp(name, "kernel32.dll") == 0 ? kKernel32 : 0;
}

FARPROC WINAPI GetProcAddress(HMODULE module, LPCSTR name)
{
    if (module == kKernel32 && SDL_strcmp(name, "InterlockedCompareExchange") == 0) {
        return reinterpret_cast<FARPROC>(&InterlockedCompareExchange);
    }
    return 0;
}

BOOL WINAPI FreeLibrary(HMODULE module)
{
    (void)module;
    return TRUE;
}

HMODULE WINAPI GetModuleHandleA(LPCSTR name)
{
    return name ? 0 : kExe;
}

// The executable's directory is SDL's base path; the file name is nominal.
DWORD WINAPI GetModuleFileNameA(HMODULE module, LPSTR path, DWORD size)
{
    const char* base = module ? 0 : SDL_GetBasePath();
    if (!base || !size) {
        return 0;
    }
    SDL_snprintf(path, size, "%sVirtualCity.exe", base);
    return static_cast<DWORD>(SDL_strlen(path));
}

// The VC1_LOG sink (kdcore/heap.cpp KdLogV).
void WINAPI OutputDebugStringA(LPCSTR text)
{
    size_t length = SDL_strlen(text);
    while (length && (text[length - 1] == '\n' || text[length - 1] == '\r')) {
        --length;
    }
    SDL_Log("%.*s", static_cast<int>(length), text);
}

HANDLE WINAPI GetCurrentProcess()
{
    return reinterpret_cast<HANDLE>(-1);
}

DWORD WINAPI GetCurrentProcessId()
{
    return 1;
}

HANDLE WINAPI GetCurrentThread()
{
    return reinterpret_cast<HANDLE>(-2);
}

// Only the crash handler asks; there is no context to give.
BOOL WINAPI GetThreadContext(HANDLE thread, CONTEXT* context)
{
    (void)thread;
    (void)context;
    return FALSE;
}

// No system message table: callers log the number instead.
DWORD WINAPI FormatMessageA(DWORD, const void*, DWORD, DWORD, LPSTR, DWORD, va_list*)
{
    return 0;
}

BOOL WINAPI IsBadStringPtrA(LPCSTR text, UINT_PTR max)
{
    (void)max;
    return text == 0;
}

void* WINAPI LocalFree(void* memory)
{
    (void)memory;
    return 0;
}

DWORD WINAPI GetCurrentThreadId()
{
    return static_cast<DWORD>(SDL_GetCurrentThreadID());
}

BOOL WINAPI GetProcessAffinityMask(HANDLE process, DWORD_PTR* processMask, DWORD_PTR* systemMask)
{
    (void)process;
    const int cores = SDL_GetNumLogicalCPUCores();
    const DWORD_PTR mask = cores >= 32 ? 0xffffffffu : (1u << cores) - 1;
    *processMask = mask;
    *systemMask = mask;
    return TRUE;
}

void WINAPI Sleep(DWORD milliseconds)
{
    SDL_Delay(milliseconds);
}

BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER* counter)
{
    counter->QuadPart = static_cast<long long>(SDL_GetPerformanceCounter());
    return TRUE;
}

BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER* frequency)
{
    frequency->QuadPart = static_cast<long long>(SDL_GetPerformanceFrequency());
    return TRUE;
}

LPSTR WINAPI GetCommandLineA()
{
    static char line[] = "VirtualCity";
    return line;
}

// SDL reports no OS version; the startup log reads "Windows v0.0.0".
BOOL WINAPI GetVersionExA(OSVERSIONINFOA* info)
{
    return info->dwOSVersionInfoSize == sizeof(*info);
}

// The first preferred SDL locale's ISO 639 language / ISO 3166 country.
LCID WINAPI GetThreadLocale()
{
    return 0;
}

int WINAPI GetLocaleInfoA(LCID locale, LCID type, LPSTR out, int size)
{
    (void)locale;
    SDL_Locale** locales = SDL_GetPreferredLocales(0);
    const char* value = 0;
    if (locales && locales[0]) {
        if (type == LOCALE_SISO639LANGNAME) {
            value = locales[0]->language;
        }
        else if (type == LOCALE_SISO3166CTRYNAME) {
            value = locales[0]->country;
        }
    }
    const int length = value ? static_cast<int>(SDL_strlcpy(out, value, size)) + 1 : 0;
    SDL_free(locales);
    return length <= size ? length : 0;
}

// The ANSI code page is Latin-1 here (see user32.cpp WM_CHAR).
int WINAPI MultiByteToWideChar(UINT codePage, DWORD flags, LPCSTR text, int length,
    WCHAR* out, int capacity)
{
    (void)codePage;
    (void)flags;
    if (length < 0) {
        length = static_cast<int>(SDL_strlen(text)) + 1;
    }
    if (length > capacity) {
        return 0;
    }
    for (int i = 0; i < length; ++i) {
        out[i] = static_cast<unsigned char>(text[i]);
    }
    return length;
}

// SDL catches no structured exceptions; the filter is only remembered.
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER filter)
{
    static LPTOP_LEVEL_EXCEPTION_FILTER current;
    const LPTOP_LEVEL_EXCEPTION_FILTER previous = current;
    current = filter;
    return previous;
}

DWORD WINAPI GetFileAttributesA(LPCSTR path)
{
    SDL_PathInfo info;
    if (!SDL_GetPathInfo(path, &info)) {
        return INVALID_FILE_ATTRIBUTES;
    }
    return info.type == SDL_PATHTYPE_DIRECTORY ? FILE_ATTRIBUTE_DIRECTORY : 0x80;
}

// The pattern is "<directory>\*.*" (DirFindFirst); every entry matches.
HANDLE WINAPI FindFirstFileA(LPCSTR pattern, WIN32_FIND_DATAA* data)
{
    char directory[MAX_PATH];
    SDL_strlcpy(directory, pattern, sizeof(directory));
    char* slash = SDL_strrchr(directory, '\\');
    if (!slash) {
        slash = SDL_strrchr(directory, '/');
    }
    if (slash) {
        *slash = 0;
    }
    Find* find = new Find();
    find->names = SDL_GlobDirectory(slash ? directory : ".", 0, 0, &find->count);
    if (!find->names || find->count == 0) {
        SDL_free(find->names);
        delete find;
        return INVALID_HANDLE_VALUE;
    }
    FillFind(find, data);
    return find;
}

BOOL WINAPI FindNextFileA(HANDLE handle, WIN32_FIND_DATAA* data)
{
    Find* find = static_cast<Find*>(handle);
    if (++find->next >= find->count) {
        return FALSE;
    }
    FillFind(find, data);
    return TRUE;
}

BOOL WINAPI FindClose(HANDLE handle)
{
    Find* find = static_cast<Find*>(handle);
    SDL_free(find->names);
    delete find;
    return TRUE;
}
