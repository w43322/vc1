#include "platform/win32/window.h"
#include "app/script.h"
#include "kd/factory.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/iid.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// GLOBAL: VC1 0x004ed450
void* g_kdWindowMixer;
static LRESULT CALLBACK KdWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
// Window class name, also used by WinMain's single-instance FindWindowA.
// GLOBAL: VC1 0x004d2844
extern const char kKdWindowClass[] = "G5__OpenKODE";

// GLOBAL: VC1 0x004e95a0
WNDCLASSA g_kdDisplayClass = {
    CS_HREDRAW | CS_VREDRAW, KdWindowProc, 0, 0, 0, 0, 0, 0, 0, kKdWindowClass
};

// GLOBAL: VC1 0x004d0b38
extern const char kKdParentWindow[] = "KD_PARENT_WINDOW";

// STRING: VC1 0x004d2854
static const char kWindowName[] = "Window";

// The G5__OpenKODE window procedure. Window messages become events on the
// window block: close (0x2c, swallowed), destroy (quit), activate (focus);
// key, char and mouse messages go through TranslateInput. A pointer move
// first drops the queued moves. The 100 ms timer of kdCreateWindow polls the
// cursor while it is not captured. Inferred name.
// FUNCTION: VC1 0x00495c10
static LRESULT CALLBACK KdWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int handled = 0;
    KdWinInput input;
    KdEvent event;
    KdWindow* window;

    memset(&event, 0, sizeof(event));
    window = reinterpret_cast<KdWindow*>(GetWindowLongA(hwnd, GWL_USERDATA));
    if (!window) {
        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }
    event.userptr = window->userptr;

    switch (msg) {
    case WM_CLOSE:
        event.type = 0x2c;
        handled = 1;
        break;
    case WM_DESTROY:
        event.type = KD_EVENT_QUIT;
        event.userptr = 0;
        handled = 1;
        break;
    case WM_ACTIVATE:
        event.type = 0x3c;
        event.data[0] = (LOWORD(wparam) != 0 && HIWORD(wparam) == 0) ? 1 : 0;
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_GETMINMAXINFO: {
        MINMAXINFO* const info = reinterpret_cast<MINMAXINFO*>(lparam);
        info->ptMinTrackSize.x = window->width + GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
        info->ptMinTrackSize.y = GetSystemMetrics(SM_CYCAPTION) + window->height +
            GetSystemMetrics(SM_CYFIXEDFRAME) * 2;
        handled = 1;
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        input.kind = 0;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        input.kind = 1;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    case WM_CHAR:
    case WM_SYSCHAR:
        input.kind = 2;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
        handled = 1;
        break;
    case WM_TIMER:
        if (wparam == 1 && GetCapture() != hwnd) {
            POINT cursor;
            GetCursorPos(&cursor);
            ScreenToClient(hwnd, &cursor);
            wparam = 0;
            lparam = MAKELPARAM(cursor.x, cursor.y);
            goto pointer_move;
        }
        break;
    case WM_MOUSEMOVE:
    pointer_move:
        if (window->lastPointer != lparam) {
            window->lastPointer = lparam;
            input.kind = 5;
            input.wparam = wparam;
            input.lparam = lparam;
            TranslateInput(window, &input, &event);
            DropPointerMoves(&event);
            handled = 1;
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        SetCapture(hwnd);
        input.kind = 3;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
        ReleaseCapture();
        input.kind = 4;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    case WM_MOUSEWHEEL:
        input.kind = 6;
        input.wparam = wparam;
        input.lparam = lparam;
        TranslateInput(window, &input, &event);
        handled = 1;
        break;
    }

    if (event.type) {
        KdEvent* const posted = AllocEvent();
        posted->data[0] = event.data[0];
        posted->type = event.type;
        posted->data[1] = event.data[1];
        posted->data[2] = event.data[2];
        posted->data[3] = event.data[3];
        posted->userptr = event.userptr;
        QueueEvent(posted);
    }
    if (handled) {
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

// RegisterClassA for the display window; hInstance at +0x10, class name at +0x24.
// FUNCTION: VC1 0x00496020
WNDCLASSA* GetKdWindowClass()
{
    if (!g_kdDisplayClass.hInstance) {
        g_kdDisplayClass.hInstance = GetModuleHandleA(0);
        g_kdDisplayClass.hIcon = LoadIconA(g_kdDisplayClass.hInstance, MAKEINTRESOURCE(100));
        g_kdDisplayClass.hCursor = LoadCursorA(g_kdDisplayClass.hInstance, MAKEINTRESOURCE(100));
        g_kdDisplayClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        if (!g_kdDisplayClass.hCursor) {
            g_kdDisplayClass.hCursor = LoadCursorA(0, IDC_ARROW);
        }
        RegisterClassA(&g_kdDisplayClass);
    }
    return &g_kdDisplayClass;
}

// CreateWindowExA mixer block; hwnd at +4, size at +0x10/+0x14.
// FUNCTION: VC1 0x004961b0
void* CreateKdWindow()
{
    const char* env;
    HWND parent;
    RECT client;
    int x;
    int y;
    int width;
    int height;
    DWORD style;
    HWND hwnd;
    unsigned char* mixer;
    WNDCLASSA* wnd;

    env = GetKdEnv(kKdParentWindow);
    parent = 0;
    if (env) {
        parent = reinterpret_cast<HWND>(strtoul(env, 0, 16));
    }

    x = static_cast<int>(CW_USEDEFAULT);
    y = static_cast<int>(CW_USEDEFAULT);
    width = static_cast<int>(CW_USEDEFAULT);
    height = static_cast<int>(CW_USEDEFAULT);
    if (parent) {
        GetClientRect(parent, &client);
        x = client.left;
        y = client.top;
        width = client.right - client.left;
        height = client.bottom - client.top;
    }

    wnd = GetKdWindowClass();
    style = 0xca0000;
    if (parent) {
        style += 0x3f360000;
    }
    hwnd = CreateWindowExA(0, wnd->lpszClassName, kWindowName, style,
        x, y, width, height, parent, 0, wnd->hInstance, 0);
    if (!hwnd) {
        *_errno() = 0x1f;
        return 0;
    }

    mixer = static_cast<unsigned char*>(malloc(0x18));
    g_kdWindowMixer = mixer;
    *reinterpret_cast<void**>(mixer) = mixer;
    *reinterpret_cast<HWND*>(mixer + 4) = hwnd;
    *reinterpret_cast<unsigned*>(mixer + 8) = 0;
    *reinterpret_cast<unsigned*>(mixer + 0xc) = 0;
    *reinterpret_cast<int*>(mixer + 0x10) = width;
    *reinterpret_cast<int*>(mixer + 0x14) = height;
    SetTimer(hwnd, 1, 100, 0);
    SetWindowLongA(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(mixer));
    return mixer;
}

// Script "setCursor": stores the resource id in the display window state
// (+0x0c), loads it from the module and makes it the window class cursor.
// FUNCTION: VC1 0x004bdb20
void ScriptSetCursor(int cursor)
{
    IDisplay* display = static_cast<IDisplay*>(AcquireNamed(&g_kdNameDisplay, &g_iidDisplay));
    unsigned char* window;
    HCURSOR handle;

    if (display) {
        window = static_cast<unsigned char*>(display->GetMixer());
        *reinterpret_cast<int*>(window + 0x0c) = cursor;
        handle = LoadCursorA(GetKdWindowClass()->hInstance, MAKEINTRESOURCEA(cursor & 0xffff));
        SetCursor(handle);
        SetClassLongA(*reinterpret_cast<HWND*>(window + 0x04), GCL_HCURSOR,
            reinterpret_cast<LONG>(handle));
        display->Release();
    }
}

}
