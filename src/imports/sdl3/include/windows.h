#ifndef VC1_IMPORTS_SDL3_WINDOWS_H
#define VC1_IMPORTS_SDL3_WINDOWS_H

// The part of the Windows SDK the recovered sources use, for the SDL3 target.
// It replaces <windows.h>, <winsock2.h>, <mmsystem.h>, <shellapi.h>,
// <shlobj.h> and <shlwapi.h> (those include this file). Each function is
// implemented over SDL in the file named after the DLL that exports it
// (kernel32.cpp, user32.cpp, ...). Types keep their SDK layout where the
// recovered code reads them; functions have C++ linkage so they never collide
// with the real system libraries the CRT links. Only WinMain is extern "C":
// the CRT entry point calls the recovered one.

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

// Like the SDK's windef.h, so SqPlus takes BOOL / INT / FLOAT from here.
#define _WINDEF_

#define WINAPI __stdcall
#define CALLBACK __stdcall
#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define INFINITE 0xffffffffu

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef int INT;
typedef float FLOAT;
typedef unsigned int UINT;
typedef char CHAR;
typedef wchar_t WCHAR;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef unsigned long DWORD_PTR;
typedef unsigned int UINT_PTR;
typedef unsigned long LCID;
typedef unsigned short ATOM;
typedef UINT MMRESULT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef intptr_t LRESULT;
typedef uintptr_t SOCKET;
typedef void* HANDLE;
typedef struct HWND__* HWND;
typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
typedef struct HICON__* HICON;
typedef struct HCURSOR__* HCURSOR;
typedef struct HBRUSH__* HBRUSH;
typedef struct HMENU__* HMENU;
typedef void* HGDIOBJ;
typedef int (WINAPI* FARPROC)();
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#define INVALID_HANDLE_VALUE (reinterpret_cast<HANDLE>(-1))
#define MAKEINTRESOURCEA(id) (reinterpret_cast<LPCSTR>(static_cast<uintptr_t>(static_cast<WORD>(id))))
#define MAKEINTRESOURCE MAKEINTRESOURCEA
#define LOWORD(value) (static_cast<WORD>(static_cast<uintptr_t>(value) & 0xffff))
#define HIWORD(value) (static_cast<WORD>((static_cast<uintptr_t>(value) >> 16) & 0xffff))
#define MAKELPARAM(low, high) (static_cast<LPARAM>(static_cast<DWORD>( \
    static_cast<WORD>(low) | (static_cast<DWORD>(static_cast<WORD>(high)) << 16))))

struct LARGE_INTEGER
{
    long long QuadPart;
};

struct RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct POINT
{
    LONG x;
    LONG y;
};

// tagMSG matches SDL_system.h's own forward declaration.
struct tagMSG
{
    HWND hwnd;
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD time;
    POINT pt;
};
typedef struct tagMSG MSG;

struct MINMAXINFO
{
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
};

struct WNDCLASSA
{
    UINT style;
    WNDPROC lpfnWndProc;
    int cbClsExtra;
    int cbWndExtra;
    HINSTANCE hInstance;
    HICON hIcon;
    HCURSOR hCursor;
    HBRUSH hbrBackground;
    LPCSTR lpszMenuName;
    LPCSTR lpszClassName;
};

// An SDL mutex instead of the kernel structure.
struct CRITICAL_SECTION
{
    struct SDL_Mutex* mutex;
};

struct OSVERSIONINFOA
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR szCSDVersion[128];
};

// Only the fields the display code reads.
struct DEVMODEA
{
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
};

struct FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

// DirOpen / DirRead read cFileName at +0x2c and keep the handle at +0x140.
struct WIN32_FIND_DATAA
{
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
};

// x86 layouts; only the crash handler reads them (it never runs: dbghelp
// does not load).
struct EXCEPTION_RECORD
{
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    EXCEPTION_RECORD* ExceptionRecord;
    void* ExceptionAddress;
    DWORD NumberParameters;
    DWORD_PTR ExceptionInformation[15];
};

struct CONTEXT
{
    DWORD ContextFlags;
    DWORD Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
    BYTE FloatSave[0x70];
    DWORD SegGs, SegFs, SegEs, SegDs;
    DWORD Edi, Esi, Ebx, Edx, Ecx, Eax;
    DWORD Ebp, Eip, SegCs, EFlags, Esp, SegSs;
    BYTE ExtendedRegisters[512];
};

struct EXCEPTION_POINTERS
{
    EXCEPTION_RECORD* ExceptionRecord;
    CONTEXT* ContextRecord;
};
typedef LONG (WINAPI* LPTOP_LEVEL_EXCEPTION_FILTER)(EXCEPTION_POINTERS* info);

struct WSADATA
{
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
};

struct JOYINFOEX
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwXpos;
    DWORD dwYpos;
    DWORD dwZpos;
    DWORD dwRpos;
    DWORD dwUpos;
    DWORD dwVpos;
    DWORD dwButtons;
    DWORD dwButtonNumber;
    DWORD dwPOV;
    DWORD dwReserved1;
    DWORD dwReserved2;
};

// Window messages and constants.
const UINT WM_DESTROY = 0x0002;
const UINT WM_ACTIVATE = 0x0006;
const UINT WM_CLOSE = 0x0010;
const UINT WM_ERASEBKGND = 0x0014;
const UINT WM_GETMINMAXINFO = 0x0024;
const UINT WM_KEYDOWN = 0x0100;
const UINT WM_KEYUP = 0x0101;
const UINT WM_CHAR = 0x0102;
const UINT WM_DEADCHAR = 0x0103;
const UINT WM_SYSKEYDOWN = 0x0104;
const UINT WM_SYSKEYUP = 0x0105;
const UINT WM_SYSCHAR = 0x0106;
const UINT WM_SYSDEADCHAR = 0x0107;
const UINT WM_TIMER = 0x0113;
const UINT WM_MOUSEMOVE = 0x0200;
const UINT WM_LBUTTONDOWN = 0x0201;
const UINT WM_LBUTTONUP = 0x0202;
const UINT WM_RBUTTONDOWN = 0x0204;
const UINT WM_RBUTTONUP = 0x0205;
const UINT WM_MBUTTONDOWN = 0x0207;
const UINT WM_MBUTTONUP = 0x0208;
const UINT WM_MOUSEWHEEL = 0x020a;
const WPARAM MK_LBUTTON = 0x0001;
const WPARAM MK_RBUTTON = 0x0002;
const WPARAM MK_SHIFT = 0x0004;
const WPARAM MK_CONTROL = 0x0008;
const WPARAM MK_MBUTTON = 0x0010;
const UINT CS_VREDRAW = 0x0001;
const UINT CS_HREDRAW = 0x0002;
const int GWL_STYLE = -16;
const int GWL_EXSTYLE = -20;
const int GWL_USERDATA = -21;
const int GCL_HCURSOR = -12;
const LONG WS_VISIBLE = 0x10000000;
const LONG WS_CAPTION = 0x00c00000;
const LONG WS_EX_TOPMOST = 0x00000008;
const int CW_USEDEFAULT = static_cast<int>(0x80000000);
#define HWND_TOPMOST (reinterpret_cast<HWND>(-1))
#define HWND_NOTOPMOST (reinterpret_cast<HWND>(-2))
#define IDC_ARROW MAKEINTRESOURCEA(32512)
const UINT SWP_NOSIZE = 0x0001;
const UINT SWP_NOMOVE = 0x0002;
const UINT SWP_NOZORDER = 0x0004;
const UINT SWP_NOACTIVATE = 0x0010;
const UINT SWP_FRAMECHANGED = 0x0020;
const int SM_CXSCREEN = 0;
const int SM_CYSCREEN = 1;
const int SM_CYCAPTION = 4;
const int SM_CXDLGFRAME = 7;
const int SM_CYFIXEDFRAME = 8;
const int SM_CXFIXEDFRAME = SM_CXDLGFRAME;
const int SW_SHOWNORMAL = 1;
const UINT MB_ICONHAND = 0x0010;
const UINT PM_REMOVE = 0x0001;
const int BLACK_BRUSH = 4;
const DWORD ENUM_CURRENT_SETTINGS = 0xffffffffu;
const UINT SPI_GETWORKAREA = 0x0030;

// Kernel constants.
const DWORD TLS_OUT_OF_INDEXES = 0xffffffffu;
const DWORD ERROR_ACCESS_DENIED = 5;
const DWORD ERROR_ALREADY_EXISTS = 183;
const DWORD INVALID_FILE_ATTRIBUTES = 0xffffffffu;
const DWORD FILE_ATTRIBUTE_DIRECTORY = 0x10;
const UINT CP_ACP = 0;
const LCID LOCALE_SISO639LANGNAME = 0x59;
const LCID LOCALE_SISO3166CTRYNAME = 0x5a;
const int CSIDL_APPDATA = 0x001a;
const MMRESULT JOYERR_NOERROR = 0;

// kernel32.cpp
DWORD WINAPI TlsAlloc();
BOOL WINAPI TlsFree(DWORD index);
void* WINAPI TlsGetValue(DWORD index);
BOOL WINAPI TlsSetValue(DWORD index, void* value);
void WINAPI InitializeCriticalSection(CRITICAL_SECTION* section);
void WINAPI DeleteCriticalSection(CRITICAL_SECTION* section);
void WINAPI EnterCriticalSection(CRITICAL_SECTION* section);
void WINAPI LeaveCriticalSection(CRITICAL_SECTION* section);
LONG WINAPI InterlockedExchange(volatile LONG* target, LONG value);
LONG WINAPI InterlockedCompareExchange(volatile LONG* target, LONG exchange, LONG comparand);
BOOL WINAPI SetEvent(HANDLE event);
DWORD WINAPI WaitForSingleObject(HANDLE handle, DWORD milliseconds);
BOOL WINAPI CloseHandle(HANDLE handle);
HANDLE WINAPI CreateMutexA(void* attributes, BOOL owner, LPCSTR name);
HANDLE WINAPI CreateEventA(void* attributes, BOOL manualReset, BOOL initialState, LPCSTR name);
#define CreateEvent CreateEventA

// What pthreads-win32 (third_party/pthreads-w32-2-8-0) uses besides the above.
typedef LONG* LPLONG;
typedef unsigned long ULONG_PTR;
typedef void VOID;
typedef void* LPVOID;
typedef void (CALLBACK* PAPCFUNC)(ULONG_PTR parameter);
#define CREATE_SUSPENDED 0x00000004
#define DUPLICATE_SAME_ACCESS 0x00000002
#define WAIT_OBJECT_0 0x00000000
#define WAIT_ABANDONED 0x00000080
#define WAIT_TIMEOUT 0x00000102
#define WAIT_FAILED 0xffffffffu
#define THREAD_PRIORITY_LOWEST (-2)
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_ERROR_RETURN 0x7fffffff
#define THREAD_PRIORITY_TIME_CRITICAL 15
#define THREAD_PRIORITY_IDLE (-15)
LONG WINAPI InterlockedIncrement(volatile LONG* target);
LONG WINAPI InterlockedDecrement(volatile LONG* target);
LONG WINAPI InterlockedExchangeAdd(volatile LONG* target, LONG value);
BOOL WINAPI ResetEvent(HANDLE event);
DWORD WINAPI WaitForMultipleObjects(DWORD count, const HANDLE* handles, BOOL all, DWORD milliseconds);
DWORD WINAPI ResumeThread(HANDLE thread);
DWORD WINAPI SuspendThread(HANDLE thread);
int WINAPI GetThreadPriority(HANDLE thread);
BOOL WINAPI SetThreadPriority(HANDLE thread, int priority);
BOOL WINAPI DuplicateHandle(HANDLE sourceProcess, HANDLE source, HANDLE targetProcess,
    HANDLE* target, DWORD access, BOOL inherit, DWORD options);
DWORD WINAPI QueueUserAPC(PAPCFUNC function, HANDLE thread, ULONG_PTR data);
#define CONTEXT_CONTROL 0x00010001
BOOL WINAPI SetThreadContext(HANDLE thread, const CONTEXT* context);
#define TEXT(text) text
#define LoadLibrary LoadLibraryA
#define PROCESS_SET_INFORMATION 0x0200
#define PROCESS_QUERY_INFORMATION 0x0400
HANDLE WINAPI OpenProcess(DWORD access, BOOL inherit, DWORD processId);
HANDLE WINAPI CreateSemaphoreA(void* attributes, LONG initialCount, LONG maximumCount, LPCSTR name);
#define CreateSemaphore CreateSemaphoreA
BOOL WINAPI ReleaseSemaphore(HANDLE semaphore, LONG releaseCount, LONG* previousCount);
DWORD WINAPI GetLastError();
void WINAPI SetLastError(DWORD error);
HMODULE WINAPI LoadLibraryA(LPCSTR name);
FARPROC WINAPI GetProcAddress(HMODULE module, LPCSTR name);
BOOL WINAPI FreeLibrary(HMODULE module);
HMODULE WINAPI GetModuleHandleA(LPCSTR name);
DWORD WINAPI GetModuleFileNameA(HMODULE module, LPSTR path, DWORD size);
void WINAPI OutputDebugStringA(LPCSTR text);
HANDLE WINAPI GetCurrentProcess();
DWORD WINAPI GetCurrentProcessId();
DWORD WINAPI GetCurrentThreadId();
HANDLE WINAPI GetCurrentThread();
BOOL WINAPI GetThreadContext(HANDLE thread, CONTEXT* context);
BOOL WINAPI IsBadStringPtrA(LPCSTR text, UINT_PTR max);
void* WINAPI LocalFree(void* memory);
BOOL WINAPI GetProcessAffinityMask(HANDLE process, DWORD_PTR* processMask, DWORD_PTR* systemMask);
void WINAPI Sleep(DWORD milliseconds);
BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER* counter);
BOOL WINAPI QueryPerformanceFrequency(LARGE_INTEGER* frequency);
LPSTR WINAPI GetCommandLineA();
BOOL WINAPI GetVersionExA(OSVERSIONINFOA* info);
LCID WINAPI GetThreadLocale();
int WINAPI GetLocaleInfoA(LCID locale, LCID type, LPSTR out, int size);
int WINAPI MultiByteToWideChar(UINT codePage, DWORD flags, LPCSTR text, int length,
    WCHAR* out, int capacity);
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(
    LPTOP_LEVEL_EXCEPTION_FILTER filter);
DWORD WINAPI GetFileAttributesA(LPCSTR path);
HANDLE WINAPI FindFirstFileA(LPCSTR pattern, WIN32_FIND_DATAA* data);
BOOL WINAPI FindNextFileA(HANDLE find, WIN32_FIND_DATAA* data);
BOOL WINAPI FindClose(HANDLE find);

// user32.cpp
ATOM WINAPI RegisterClassA(const WNDCLASSA* windowClass);
HWND WINAPI CreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style,
    int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance,
    void* param);
BOOL WINAPI DestroyWindow(HWND window);
LRESULT WINAPI DefWindowProcA(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
BOOL WINAPI PeekMessageA(MSG* msg, HWND window, UINT first, UINT last, UINT remove);
BOOL WINAPI TranslateMessage(const MSG* msg);
LRESULT WINAPI DispatchMessageA(const MSG* msg);
LONG WINAPI GetWindowLongA(HWND window, int index);
LONG WINAPI SetWindowLongA(HWND window, int index, LONG value);
DWORD WINAPI SetClassLongA(HWND window, int index, LONG value);
BOOL WINAPI SetWindowPos(HWND window, HWND after, int x, int y, int width, int height,
    UINT flags);
BOOL WINAPI GetWindowRect(HWND window, RECT* rect);
BOOL WINAPI GetClientRect(HWND window, RECT* rect);
BOOL WINAPI ShowWindow(HWND window, int command);
BOOL WINAPI SetWindowTextA(HWND window, LPCSTR text);
HWND WINAPI FindWindowA(LPCSTR className, LPCSTR windowName);
BOOL WINAPI SetForegroundWindow(HWND window);
HWND WINAPI GetActiveWindow();
int WINAPI MessageBoxA(HWND window, LPCSTR text, LPCSTR caption, UINT type);

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_FROM_SYSTEM 0x00001000
DWORD WINAPI FormatMessageA(DWORD flags, const void* source, DWORD messageId, DWORD languageId,
    LPSTR buffer, DWORD size, va_list* arguments);

// wininet.dll: the "http" KD driver.
typedef void* HINTERNET;
#define HTTP_QUERY_STATUS_CODE 19
HINTERNET WINAPI InternetOpenA(LPCSTR agent, DWORD accessType, LPCSTR proxy, LPCSTR bypass,
    DWORD flags);
HINTERNET WINAPI InternetOpenUrlA(HINTERNET internet, LPCSTR url, LPCSTR headers,
    DWORD headersLength, DWORD flags, DWORD_PTR context);
BOOL WINAPI InternetCloseHandle(HINTERNET handle);
BOOL WINAPI InternetReadFile(HINTERNET file, void* buffer, DWORD size, DWORD* read);
DWORD WINAPI InternetSetFilePointer(HINTERNET file, LONG distance, LONG* distanceHigh,
    DWORD method, DWORD_PTR context);
BOOL WINAPI HttpQueryInfoA(HINTERNET request, DWORD level, void* buffer, DWORD* length,
    DWORD* index);
uintptr_t WINAPI SetTimer(HWND window, uintptr_t id, UINT milliseconds, void* callback);
BOOL WINAPI KillTimer(HWND window, uintptr_t id);
HWND WINAPI SetCapture(HWND window);
BOOL WINAPI ReleaseCapture();
HWND WINAPI GetCapture();
BOOL WINAPI GetCursorPos(POINT* point);
BOOL WINAPI ScreenToClient(HWND window, POINT* point);
HICON WINAPI LoadIconA(HINSTANCE instance, LPCSTR name);
HCURSOR WINAPI LoadCursorA(HINSTANCE instance, LPCSTR name);
HCURSOR WINAPI SetCursor(HCURSOR cursor);
int WINAPI GetSystemMetrics(int index);
BOOL WINAPI EnumDisplaySettingsA(LPCSTR device, DWORD mode, DEVMODEA* out);
BOOL WINAPI SystemParametersInfoA(UINT action, UINT param, void* out, UINT flags);

// gdi32.cpp
HGDIOBJ WINAPI GetStockObject(int object);

// shell32.cpp
HINSTANCE WINAPI ShellExecuteA(HWND window, LPCSTR operation, LPCSTR file, LPCSTR parameters,
    LPCSTR directory, int show);
BOOL WINAPI SHGetSpecialFolderPathA(HWND window, LPSTR path, int folder, BOOL create);

// shlwapi.cpp
BOOL WINAPI PathAppendA(LPSTR path, LPCSTR more);

// winmm.cpp
MMRESULT WINAPI joyGetPosEx(UINT joystick, JOYINFOEX* info);

// ws2_32.cpp
int WINAPI WSAStartup(WORD version, WSADATA* data);
int WINAPI WSACleanup();
int WINAPI closesocket(SOCKET socket);
int WINAPI WSAGetLastError();
void WINAPI WSASetLastError(int error);

// crt.cpp: the VC9 CRT's stdio array, which the modern CRT no longer exports.
struct _iobuf;
struct _iobuf* __iob_func();

extern "C" int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR commandLine,
    int show);

#endif
