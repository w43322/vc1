#ifndef VC1_PLATFORM_SDL3_WIN32_H
#define VC1_PLATFORM_SDL3_WIN32_H

// The Win32 subset the shared recovered sources call (display window, event
// pump, clock, version, locale), implemented over SDL in window.cpp. HWND is
// the SDL_Window; GWL_STYLE without WS_CAPTION applied with SWP_FRAMECHANGED
// selects desktop fullscreen. The message pump is SdlPollEvents.

typedef struct HWND__* HWND;
typedef long LONG;
typedef unsigned long DWORD;
typedef int BOOL;

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

struct LARGE_INTEGER
{
    long long QuadPart;
};

// Messages never reach the game: PeekMessageA pumps SDL events instead.
// tagMSG matches SDL_system.h's own Windows forward declaration.
struct tagMSG
{
    HWND hwnd;
};
typedef struct tagMSG MSG;

typedef unsigned long LCID;

struct OSVERSIONINFOA
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    char szCSDVersion[128];
};

// Only the fields the display code reads.
struct DEVMODEA
{
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
};

const int GWL_STYLE = -16;
const int GWL_EXSTYLE = -20;
const int GWL_USERDATA = -21;
const LONG WS_VISIBLE = 0x10000000;
const LONG WS_CAPTION = 0x00c00000;
const LONG WS_EX_TOPMOST = 0x00000008;
#define HWND_TOPMOST (reinterpret_cast<HWND>(-1))
#define HWND_NOTOPMOST (reinterpret_cast<HWND>(-2))
const unsigned SWP_NOSIZE = 0x0001;
const unsigned SWP_NOMOVE = 0x0002;
const unsigned SWP_NOZORDER = 0x0004;
const unsigned SWP_NOACTIVATE = 0x0010;
const unsigned SWP_FRAMECHANGED = 0x0020;
const int SM_CXSCREEN = 0;
const int SM_CYSCREEN = 1;
const int SM_CYCAPTION = 4;
const int SM_CXDLGFRAME = 7;
const int SM_CYFIXEDFRAME = 8;
const unsigned MB_ICONHAND = 0x10;
const int SW_SHOWNORMAL = 1;
const DWORD ENUM_CURRENT_SETTINGS = 0xffffffffu;
const unsigned SPI_GETWORKAREA = 0x30;
const unsigned PM_REMOVE = 0x0001;
const unsigned MAX_PATH = 260;
const unsigned LOCALE_SISO639LANGNAME = 0x59;
const unsigned LOCALE_SISO3166CTRYNAME = 0x5a;

namespace vc1 {

LONG GetWindowLongA(HWND window, int index);
LONG SetWindowLongA(HWND window, int index, LONG value);
BOOL SetWindowPos(HWND window, HWND after, int x, int y, int width, int height, unsigned flags);
BOOL GetWindowRect(HWND window, RECT* rect);
BOOL GetClientRect(HWND window, RECT* rect);
int GetSystemMetrics(int index);
BOOL EnumDisplaySettingsA(const char* device, DWORD mode, DEVMODEA* out);
BOOL SystemParametersInfoA(unsigned action, unsigned param, void* out, unsigned flags);
BOOL ShowWindow(HWND window, int command);
BOOL SetWindowTextA(HWND window, const char* text);
int MessageBoxA(HWND window, const char* text, const char* caption, unsigned type);
BOOL KillTimer(HWND window, unsigned id);
BOOL DestroyWindow(HWND window);
BOOL PeekMessageA(MSG* msg, HWND window, unsigned first, unsigned last, unsigned remove);
BOOL TranslateMessage(const MSG* msg);
long DispatchMessageA(const MSG* msg);
void Sleep(DWORD milliseconds);
BOOL QueryPerformanceCounter(LARGE_INTEGER* counter);
BOOL GetVersionExA(OSVERSIONINFOA* info);
LCID GetThreadLocale();
int GetLocaleInfoA(LCID locale, unsigned type, char* out, int size);

}

#endif
