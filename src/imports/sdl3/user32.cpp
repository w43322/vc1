// user32.dll over SDL3: windows, window classes, the message queue, cursors
// and display metrics.
//
// A HWND is a Window record. Windows with a caption get an SDL window; the
// others (the hidden [kdNetwork] window) are message-only. PeekMessageA turns
// SDL events into the Win32 messages the recovered window procedure handles.
// Mouse positions are reported in back buffer pixels (the renderer's logical
// presentation), which is what a fixed-size Win32 client area gives.
#include "sdl3.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <deque>
#include <map>
#include <string>
#include <vector>

namespace {

struct Window
{
    WNDPROC proc;
    LONG style;
    LONG exStyle;
    LONG userData;
    SDL_Window* sdl;
    int width;  // the size the game last gave it
    int height;
};

struct Timer
{
    HWND window;
    uintptr_t id;
    UINT period;
    Uint64 due;
};

std::map<std::string, WNDCLASSA> classes;
std::deque<MSG> messages;
std::vector<Timer> timers;
std::map<uintptr_t, SDL_Cursor*> cursors;
std::map<uintptr_t, SDL_Surface*> icons;
HWND capture;
const char kHwndProperty[] = "vc1.hwnd";

Window* Get(HWND window)
{
    return reinterpret_cast<Window*>(window);
}

HWND FromSdl(SDL_WindowID id)
{
    SDL_Window* sdl = SDL_GetWindowFromID(id);
    if (!sdl) {
        return 0;
    }
    return static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(sdl), kHwndProperty, 0));
}

SDL_DisplayID Display()
{
    SDL_Window* focus = SDL_GetKeyboardFocus();
    SDL_DisplayID display = focus ? SDL_GetDisplayForWindow(focus) : 0;
    return display ? display : SDL_GetPrimaryDisplay();
}

#ifdef __EMSCRIPTEN__
// In the browser the canvas fills the page (SDL_WINDOW_FILL_DOCUMENT) and the
// renderer letterboxes the back buffer into it. The screen is the game's
// display size (DisplayWidth / DisplayHeight in scripts/Settings.nut), so
// full screen and windowed render the same 1024x768 frame, scaled to the page
// like a resized desktop window, and the browser never goes full screen.
const SDL_WindowFlags kScreenWindowFlags = SDL_WINDOW_FILL_DOCUMENT;
const SDL_Point screen = {1024, 768};

bool DesktopBounds(SDL_Rect* bounds)
{
    bounds->x = 0;
    bounds->y = 0;
    bounds->w = screen.x;
    bounds->h = screen.y;
    return true;
}

bool WorkArea(SDL_Rect* area)
{
    return DesktopBounds(area);
}

bool DesktopSize(int* w, int* h)
{
    *w = screen.x;
    *h = screen.y;
    return true;
}

bool WindowOrigin(SDL_Window* sdl, int* x, int* y)
{
    *x = 0;
    *y = 0;
    return sdl != 0;
}

bool WindowSize(HWND hwnd, int* w, int* h)
{
    Window* window = Get(hwnd);
    if (!window || !window->sdl) {
        return false;
    }
    *w = window->width;
    *h = window->height;
    return true;
}
#else
const SDL_WindowFlags kScreenWindowFlags = 0;

bool DesktopBounds(SDL_Rect* bounds)
{
    return SDL_GetDisplayBounds(Display(), bounds);
}

bool WorkArea(SDL_Rect* area)
{
    return SDL_GetDisplayUsableBounds(Display(), area);
}

bool DesktopSize(int* w, int* h)
{
    const SDL_DisplayMode* desktop = SDL_GetDesktopDisplayMode(Display());
    if (!desktop) {
        return false;
    }
    *w = static_cast<int>(desktop->w * desktop->pixel_density + 0.5f);
    *h = static_cast<int>(desktop->h * desktop->pixel_density + 0.5f);
    return true;
}

bool WindowOrigin(SDL_Window* sdl, int* x, int* y)
{
    return SDL_GetWindowPosition(sdl, x, y);
}

bool WindowSize(HWND hwnd, int* w, int* h)
{
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    return sdl && SDL_GetWindowSize(sdl, w, h);
}
#endif

// The game's icon and cursors, extracted from the original's resources at
// configure time (tools/extract-sdl-resources.py).
SDL_Surface* LoadResource(const char* kind, unsigned id, int* hotX, int* hotY)
{
    char path[1024];
    SDL_snprintf(path, sizeof(path), "%splatform-resources/%s%u.bin", SDL_GetBasePath(), kind, id);
    size_t bytes = 0;
    unsigned* data = static_cast<unsigned*>(SDL_LoadFile(path, &bytes));
    if (!data) {
        return 0;
    }
    SDL_Surface* surface = 0;
    if (bytes >= 20 && !SDL_memcmp(data, "VCR1", 4) && data[1] && data[2] &&
        bytes == 20 + size_t(data[1]) * data[2] * 4) {
        surface = SDL_CreateSurface(data[1], data[2], SDL_PIXELFORMAT_ARGB8888);
        if (surface) {
            for (unsigned y = 0; y < data[2]; ++y) {
                SDL_memcpy(static_cast<char*>(surface->pixels) + y * surface->pitch,
                    data + 5 + y * data[1], data[1] * 4);
            }
            if (hotX) {
                *hotX = data[3];
            }
            if (hotY) {
                *hotY = data[4];
            }
        }
    }
    SDL_free(data);
    return surface;
}

void Post(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    MSG msg = {};
    msg.hwnd = window;
    msg.message = message;
    msg.wParam = wparam;
    msg.lParam = lparam;
    msg.time = static_cast<DWORD>(SDL_GetTicks());
    messages.push_back(msg);
}

LRESULT Send(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    Window* w = Get(window);
    return w && w->proc ? w->proc(window, message, wparam, lparam) : 0;
}

// Window coordinates to back buffer pixels.
LPARAM Position(HWND window, float x, float y)
{
    SDL_Renderer* renderer = sdl3::WindowOf(window) ? SDL_GetRenderer(sdl3::WindowOf(window)) : 0;
    if (renderer) {
        SDL_RenderCoordinatesFromWindow(renderer, x, y, &x, &y);
    }
    return MAKELPARAM(static_cast<int>(x), static_cast<int>(y));
}

WPARAM MouseKeys()
{
    const SDL_MouseButtonFlags buttons = SDL_GetMouseState(0, 0);
    const SDL_Keymod mods = SDL_GetModState();
    WPARAM keys = 0;
    if (buttons & SDL_BUTTON_LMASK) {
        keys |= MK_LBUTTON;
    }
    if (buttons & SDL_BUTTON_RMASK) {
        keys |= MK_RBUTTON;
    }
    if (buttons & SDL_BUTTON_MMASK) {
        keys |= MK_MBUTTON;
    }
    if (mods & SDL_KMOD_SHIFT) {
        keys |= MK_SHIFT;
    }
    if (mods & SDL_KMOD_CTRL) {
        keys |= MK_CONTROL;
    }
    return keys;
}

// SDL key to Windows virtual-key code.
WPARAM VirtualKey(SDL_Keycode key)
{
    if (key >= SDLK_A && key <= SDLK_Z) {
        return key - SDLK_A + 'A';
    }
    if (key >= SDLK_0 && key <= SDLK_9) {
        return key;
    }
    if (key >= SDLK_F1 && key <= SDLK_F12) {
        return 0x70 + key - SDLK_F1;
    }
    if (key >= SDLK_F13 && key <= SDLK_F24) {
        return 0x7c + key - SDLK_F13;
    }
    if (key >= SDLK_KP_1 && key <= SDLK_KP_9) {
        return 0x61 + key - SDLK_KP_1;
    }
    switch (key) {
    case SDLK_BACKSPACE: return 0x08;
    case SDLK_TAB: return 0x09;
    case SDLK_RETURN: case SDLK_KP_ENTER: return 0x0d;
    case SDLK_LSHIFT: case SDLK_RSHIFT: return 0x10;
    case SDLK_LCTRL: case SDLK_RCTRL: return 0x11;
    case SDLK_LALT: case SDLK_RALT: return 0x12;
    case SDLK_PAUSE: return 0x13;
    case SDLK_CAPSLOCK: return 0x14;
    case SDLK_ESCAPE: return 0x1b;
    case SDLK_SPACE: return 0x20;
    case SDLK_PAGEUP: return 0x21;
    case SDLK_PAGEDOWN: return 0x22;
    case SDLK_END: return 0x23;
    case SDLK_HOME: return 0x24;
    case SDLK_LEFT: return 0x25;
    case SDLK_UP: return 0x26;
    case SDLK_RIGHT: return 0x27;
    case SDLK_DOWN: return 0x28;
    case SDLK_PRINTSCREEN: return 0x2c;
    case SDLK_INSERT: return 0x2d;
    case SDLK_DELETE: return 0x2e;
    case SDLK_LGUI: return 0x5b;
    case SDLK_RGUI: return 0x5c;
    case SDLK_APPLICATION: return 0x5d;
    case SDLK_KP_0: return 0x60;
    case SDLK_KP_MULTIPLY: return 0x6a;
    case SDLK_KP_PLUS: return 0x6b;
    case SDLK_KP_MINUS: return 0x6d;
    case SDLK_KP_PERIOD: return 0x6e;
    case SDLK_KP_DIVIDE: return 0x6f;
    case SDLK_NUMLOCKCLEAR: return 0x90;
    case SDLK_SCROLLLOCK: return 0x91;
    case SDLK_SEMICOLON: return 0xba;
    case SDLK_EQUALS: return 0xbb;
    case SDLK_COMMA: return 0xbc;
    case SDLK_MINUS: return 0xbd;
    case SDLK_PERIOD: return 0xbe;
    case SDLK_SLASH: return 0xbf;
    case SDLK_GRAVE: return 0xc0;
    case SDLK_LEFTBRACKET: return 0xdb;
    case SDLK_BACKSLASH: return 0xdc;
    case SDLK_RIGHTBRACKET: return 0xdd;
    case SDLK_APOSTROPHE: return 0xde;
    default: return key < 0x80 ? key : 0;
    }
}

void PostKey(HWND window, const SDL_KeyboardEvent& key)
{
    const WPARAM vk = VirtualKey(key.key);
    if (!vk) {
        return;
    }
    // Repeat count 1, scan code, context (Alt), previous state, transition.
    LPARAM flags = 1 | ((static_cast<LPARAM>(key.scancode) & 0xff) << 16);
    if (key.mod & SDL_KMOD_ALT) {
        flags |= 1 << 29;
    }
    if (key.repeat || !key.down) {
        flags |= 1 << 30;
    }
    if (!key.down) {
        flags |= static_cast<LPARAM>(1u << 31);
    }
    const bool alt = (key.mod & SDL_KMOD_ALT) != 0;
    Post(window, key.down ? (alt ? WM_SYSKEYDOWN : WM_KEYDOWN) : (alt ? WM_SYSKEYUP : WM_KEYUP),
        vk, flags);
}

// WM_CHAR carries one ANSI byte; the code page here is Latin-1.
void PostText(HWND window, const char* text)
{
    const unsigned char* p = reinterpret_cast<const unsigned char*>(text);
    while (*p) {
        unsigned c = *p++;
        int extra = c < 0x80 ? 0 : (c & 0xe0) == 0xc0 ? 1 : (c & 0xf0) == 0xe0 ? 2 : 3;
        if (extra) {
            c &= (1u << (6 - extra)) - 1;
        }
        while (extra-- && *p) {
            c = (c << 6) | (*p++ & 0x3f);
        }
        if (c < 0x100) {
            Post(window, WM_CHAR, c, 1);
        }
    }
}

UINT ButtonMessage(Uint8 button, bool down)
{
    switch (button) {
    case SDL_BUTTON_LEFT: return down ? WM_LBUTTONDOWN : WM_LBUTTONUP;
    case SDL_BUTTON_RIGHT: return down ? WM_RBUTTONDOWN : WM_RBUTTONUP;
    case SDL_BUTTON_MIDDLE: return down ? WM_MBUTTONDOWN : WM_MBUTTONUP;
    default: return 0;
    }
}

void Translate(const SDL_Event& e)
{
    switch (e.type) {
    case SDL_EVENT_QUIT:
        // Closing the last window also sends a close request; this covers a
        // quit from elsewhere (system shutdown, Cmd+Q).
        if (SDL_Window* focus = SDL_GetKeyboardFocus()) {
            Post(FromSdl(SDL_GetWindowID(focus)), WM_CLOSE, 0, 0);
        }
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        Post(FromSdl(e.window.windowID), WM_CLOSE, 0, 0);
        break;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        Post(FromSdl(e.window.windowID), WM_ACTIVATE, 1, 0);
        break;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        Post(FromSdl(e.window.windowID), WM_ACTIVATE, 0, 0);
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        PostKey(FromSdl(e.key.windowID), e.key);
        break;
    case SDL_EVENT_TEXT_INPUT:
        PostText(FromSdl(e.text.windowID), e.text.text);
        break;
    case SDL_EVENT_MOUSE_MOTION: {
        const HWND window = FromSdl(e.motion.windowID);
        Post(window, WM_MOUSEMOVE, MouseKeys(), Position(window, e.motion.x, e.motion.y));
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        const HWND window = FromSdl(e.button.windowID);
        const UINT message = ButtonMessage(e.button.button, e.button.down);
        if (message) {
            Post(window, message, MouseKeys(), Position(window, e.button.x, e.button.y));
        }
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
        const HWND window = FromSdl(e.wheel.windowID);
        const float flip = e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.0f : 1.0f;
        const WPARAM delta = static_cast<WORD>(static_cast<short>(e.wheel.y * flip * 120));
        Post(window, WM_MOUSEWHEEL, (delta << 16) | MouseKeys(),
            Position(window, e.wheel.mouse_x, e.wheel.mouse_y));
        break;
    }
    }
}

void Pump()
{
    sdl3::UpdateAudio();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        Translate(e);
    }
    const Uint64 now = SDL_GetTicks();
    for (size_t i = 0; i < timers.size(); ++i) {
        if (now >= timers[i].due) {
            timers[i].due = now + timers[i].period;
            Post(timers[i].window, WM_TIMER, timers[i].id, 0);
        }
    }
}

}

namespace sdl3 {

SDL_Window* WindowOf(HWND window)
{
    return window ? Get(window)->sdl : 0;
}

bool DesktopPixels(int* w, int* h)
{
    return DesktopSize(w, h);
}

}

ATOM WINAPI RegisterClassA(const WNDCLASSA* windowClass)
{
    classes[windowClass->lpszClassName] = *windowClass;
    return 1;
}

HWND WINAPI CreateWindowExA(DWORD exStyle, LPCSTR className, LPCSTR windowName, DWORD style,
    int x, int y, int width, int height, HWND parent, HMENU menu, HINSTANCE instance,
    void* param)
{
    (void)parent;
    (void)menu;
    (void)instance;
    (void)param;
    std::map<std::string, WNDCLASSA>::iterator found = classes.find(className);
    if (found == classes.end()) {
        return 0;
    }
    Window* window = new Window();
    window->proc = found->second.lpfnWndProc;
    window->style = static_cast<LONG>(style);
    window->exStyle = static_cast<LONG>(exStyle);
    window->width = width == CW_USEDEFAULT ? 640 : width;
    window->height = height == CW_USEDEFAULT ? 480 : height;
    if (style & WS_CAPTION) {
        if (!sdl3::Init(SDL_INIT_VIDEO)) {
            delete window;
            return 0;
        }
        // Hidden until ShowWindow, like a Win32 window.
        window->sdl = SDL_CreateWindow(windowName, width == CW_USEDEFAULT ? 640 : width,
            height == CW_USEDEFAULT ? 480 : height,
            SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY |
                kScreenWindowFlags);
        if (!window->sdl) {
            SDL_Log("SDL window: %s", SDL_GetError());
            delete window;
            return 0;
        }
        if (x != CW_USEDEFAULT && y != CW_USEDEFAULT) {
            SDL_SetWindowPosition(window->sdl, x, y);
        }
        SDL_SetPointerProperty(SDL_GetWindowProperties(window->sdl), kHwndProperty, window);
        if (found->second.hIcon) {
            SDL_SetWindowIcon(window->sdl, reinterpret_cast<SDL_Surface*>(found->second.hIcon));
        }
        SDL_StartTextInput(window->sdl);
    }
    return reinterpret_cast<HWND>(window);
}

BOOL WINAPI DestroyWindow(HWND hwnd)
{
    Window* window = Get(hwnd);
    if (!window) {
        return FALSE;
    }
    Send(hwnd, WM_DESTROY, 0, 0);
    for (size_t i = timers.size(); i-- > 0;) {
        if (timers[i].window == hwnd) {
            timers.erase(timers.begin() + i);
        }
    }
    for (std::deque<MSG>::iterator i = messages.begin(); i != messages.end();) {
        i = i->hwnd == hwnd ? messages.erase(i) : i + 1;
    }
    if (capture == hwnd) {
        ReleaseCapture();
    }
    if (window->sdl) {
        SDL_DestroyWindow(window->sdl);
    }
    delete window;
    return TRUE;
}

LRESULT WINAPI DefWindowProcA(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    (void)wparam;
    (void)lparam;
    if (message == WM_CLOSE) {
        DestroyWindow(window);
    }
    return 0;
}

BOOL WINAPI PeekMessageA(MSG* msg, HWND window, UINT first, UINT last, UINT remove)
{
    (void)window;
    (void)first;
    (void)last;
    if (messages.empty()) {
        Pump();
    }
    if (messages.empty()) {
        return FALSE;
    }
    *msg = messages.front();
    if (remove & PM_REMOVE) {
        messages.pop_front();
    }
    return TRUE;
}

// WM_CHAR comes from SDL text input instead.
BOOL WINAPI TranslateMessage(const MSG* msg)
{
    (void)msg;
    return FALSE;
}

LRESULT WINAPI DispatchMessageA(const MSG* msg)
{
    return Send(msg->hwnd, msg->message, msg->wParam, msg->lParam);
}

LONG WINAPI GetWindowLongA(HWND hwnd, int index)
{
    Window* window = Get(hwnd);
    if (!window) {
        return 0;
    }
    switch (index) {
    case GWL_STYLE: {
        const bool visible = window->sdl && !(SDL_GetWindowFlags(window->sdl) & SDL_WINDOW_HIDDEN);
        return window->style | (visible ? WS_VISIBLE : 0);
    }
    case GWL_EXSTYLE:
        return window->exStyle;
    case GWL_USERDATA:
        return window->userData;
    default:
        return 0;
    }
}

LONG WINAPI SetWindowLongA(HWND hwnd, int index, LONG value)
{
    Window* window = Get(hwnd);
    LONG* slot = 0;
    if (!window) {
        return 0;
    }
    switch (index) {
    case GWL_STYLE:
        value &= ~WS_VISIBLE;
        slot = &window->style;
        break;
    case GWL_EXSTYLE:
        slot = &window->exStyle;
        break;
    case GWL_USERDATA:
        slot = &window->userData;
        break;
    default:
        return 0;
    }
    const LONG old = *slot;
    *slot = value;
    return old;
}

// Only the cursor is set, and SetCursor already applied it.
DWORD WINAPI SetClassLongA(HWND window, int index, LONG value)
{
    (void)window;
    (void)index;
    (void)value;
    return 0;
}

// A frame change applies GWL_STYLE: no caption means desktop full screen.
// The frame metrics are 0, so window and client rectangles are the same.
BOOL WINAPI SetWindowPos(HWND hwnd, HWND after, int x, int y, int width, int height, UINT flags)
{
    (void)after;
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    if (!sdl) {
        return FALSE;
    }
    if (!(flags & SWP_NOSIZE)) {
        Get(hwnd)->width = width;
        Get(hwnd)->height = height;
    }
#ifdef __EMSCRIPTEN__
    // The page shows every size and style the same way (see screen).
    (void)x;
    (void)y;
    return TRUE;
#endif
    if (flags & SWP_FRAMECHANGED) {
        const bool fullscreen = !(Get(hwnd)->style & WS_CAPTION);
        SDL_SetWindowFullscreen(sdl, fullscreen);
        if (fullscreen) {
            SDL_SyncWindow(sdl);
            return TRUE;
        }
    }
    if (!(flags & SWP_NOSIZE)) {
        SDL_SetWindowSize(sdl, width, height);
    }
    if (!(flags & SWP_NOMOVE)) {
        SDL_SetWindowPosition(sdl, x, y);
    }
    SDL_SyncWindow(sdl);
    return TRUE;
}

BOOL WINAPI GetWindowRect(HWND hwnd, RECT* rect)
{
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    if (!sdl || !WindowOrigin(sdl, &x, &y) || !WindowSize(hwnd, &w, &h)) {
        return FALSE;
    }
    rect->left = x;
    rect->top = y;
    rect->right = x + w;
    rect->bottom = y + h;
    return TRUE;
}

BOOL WINAPI GetClientRect(HWND hwnd, RECT* rect)
{
    int w = 0;
    int h = 0;
    if (!WindowSize(hwnd, &w, &h)) {
        return FALSE;
    }
    rect->left = 0;
    rect->top = 0;
    rect->right = w;
    rect->bottom = h;
    return TRUE;
}

BOOL WINAPI ShowWindow(HWND hwnd, int command)
{
    (void)command;
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    return sdl && SDL_ShowWindow(sdl);
}

BOOL WINAPI SetWindowTextA(HWND hwnd, LPCSTR text)
{
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    return sdl && SDL_SetWindowTitle(sdl, text);
}

// No other instance is ever found (CreateMutexA reports none).
HWND WINAPI FindWindowA(LPCSTR className, LPCSTR windowName)
{
    (void)className;
    (void)windowName;
    return 0;
}

BOOL WINAPI SetForegroundWindow(HWND window)
{
    SDL_Window* sdl = sdl3::WindowOf(window);
    return sdl && SDL_RaiseWindow(sdl);
}

HWND WINAPI GetActiveWindow()
{
    SDL_Window* focus = SDL_GetKeyboardFocus();
    return focus ? FromSdl(SDL_GetWindowID(focus)) : 0;
}

int WINAPI MessageBoxA(HWND window, LPCSTR text, LPCSTR caption, UINT type)
{
    (void)type;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, caption, text, sdl3::WindowOf(window));
    return 1;
}

uintptr_t WINAPI SetTimer(HWND window, uintptr_t id, UINT milliseconds, void* callback)
{
    (void)callback;
    KillTimer(window, id);
    Timer timer = {window, id, milliseconds, SDL_GetTicks() + milliseconds};
    timers.push_back(timer);
    return id;
}

BOOL WINAPI KillTimer(HWND window, uintptr_t id)
{
    for (size_t i = 0; i < timers.size(); ++i) {
        if (timers[i].window == window && timers[i].id == id) {
            timers.erase(timers.begin() + i);
            return TRUE;
        }
    }
    return FALSE;
}

HWND WINAPI SetCapture(HWND window)
{
    const HWND previous = capture;
    capture = window;
    SDL_CaptureMouse(true);
    return previous;
}

BOOL WINAPI ReleaseCapture()
{
    capture = 0;
    SDL_CaptureMouse(false);
    return TRUE;
}

HWND WINAPI GetCapture()
{
    return capture;
}

BOOL WINAPI GetCursorPos(POINT* point)
{
    float x = 0;
    float y = 0;
    SDL_GetGlobalMouseState(&x, &y);
    point->x = static_cast<LONG>(x);
    point->y = static_cast<LONG>(y);
    return TRUE;
}

// To back buffer pixels, like the mouse messages.
BOOL WINAPI ScreenToClient(HWND hwnd, POINT* point)
{
    SDL_Window* sdl = sdl3::WindowOf(hwnd);
    int x = 0;
    int y = 0;
    if (!sdl || !WindowOrigin(sdl, &x, &y)) {
        return FALSE;
    }
    const LPARAM position = Position(hwnd, static_cast<float>(point->x - x),
        static_cast<float>(point->y - y));
    point->x = static_cast<short>(LOWORD(position));
    point->y = static_cast<short>(HIWORD(position));
    return TRUE;
}

// An HICON is the icon's SDL surface.
HICON WINAPI LoadIconA(HINSTANCE instance, LPCSTR name)
{
    (void)instance;
    const uintptr_t id = reinterpret_cast<uintptr_t>(name);
    if (!icons.count(id)) {
        icons[id] = LoadResource("icon", static_cast<unsigned>(id), 0, 0);
    }
    return reinterpret_cast<HICON>(icons[id]);
}

// An HCURSOR is an SDL cursor; LoadCursorA(0, IDC_ARROW) is the system arrow.
HCURSOR WINAPI LoadCursorA(HINSTANCE instance, LPCSTR name)
{
    // The scripts set the first cursor before the window exists; SDL makes
    // cursors only once video is up.
    if (!sdl3::Init(SDL_INIT_VIDEO)) {
        return 0;
    }
    const uintptr_t id = reinterpret_cast<uintptr_t>(name);
    const uintptr_t key = instance ? id : id | 0x10000;
    if (!cursors.count(key)) {
        SDL_Cursor* cursor = 0;
        if (!instance) {
            cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
        }
        else {
            int hotX = 0;
            int hotY = 0;
            SDL_Surface* image = LoadResource("cursor", static_cast<unsigned>(id), &hotX, &hotY);
            if (image) {
                cursor = SDL_CreateColorCursor(image, hotX, hotY);
                SDL_DestroySurface(image);
            }
            // Without the extracted resources the arrow stands in: a null
            // cursor would hide the pointer.
            if (!cursor) {
                cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
            }
        }
        cursors[key] = cursor;
    }
    return reinterpret_cast<HCURSOR>(cursors[key]);
}

// A null cursor hides it, as on Win32.
HCURSOR WINAPI SetCursor(HCURSOR cursor)
{
    SDL_Cursor* previous = SDL_GetCursor();
    if (cursor) {
        SDL_SetCursor(reinterpret_cast<SDL_Cursor*>(cursor));
        SDL_ShowCursor();
    }
    else {
        SDL_HideCursor();
    }
    return reinterpret_cast<HCURSOR>(previous);
}

int WINAPI GetSystemMetrics(int index)
{
    SDL_Rect bounds;
    if ((index != SM_CXSCREEN && index != SM_CYSCREEN) || !DesktopBounds(&bounds)) {
        return 0;
    }
    return index == SM_CXSCREEN ? bounds.w : bounds.h;
}

// The desktop mode in pixels: the full-screen back buffer size.
BOOL WINAPI EnumDisplaySettingsA(LPCSTR device, DWORD mode, DEVMODEA* out)
{
    (void)device;
    (void)mode;
    int w = 0;
    int h = 0;
    if (!sdl3::DesktopPixels(&w, &h)) {
        return FALSE;
    }
    out->dmPelsWidth = static_cast<DWORD>(w);
    out->dmPelsHeight = static_cast<DWORD>(h);
    return TRUE;
}

BOOL WINAPI SystemParametersInfoA(UINT action, UINT param, void* out, UINT flags)
{
    (void)param;
    (void)flags;
    SDL_Rect area;
    if (action != SPI_GETWORKAREA || !WorkArea(&area)) {
        return FALSE;
    }
    RECT* rect = static_cast<RECT*>(out);
    rect->left = area.x;
    rect->top = area.y;
    rect->right = area.x + area.w;
    rect->bottom = area.y + area.h;
    return TRUE;
}
