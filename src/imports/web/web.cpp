// The browser's entry point, standing in for the CRT's WinMainCRTStartup.
// The page (shell/loader.js) mounts the player's game folder as the working
// directory and the saves folder as %APPDATA%, then calls main.
//
// The recovered main loop never returns to the browser. The module is built
// with ASYNCIFY and yields inside it: the d3d8 Present waits for the next
// animation frame and SDL_Delay (Sleep) calls emscripten_sleep, so each frame
// reaches the screen and input events arrive.
#include "../sdl3/sdl3.h"

#include <emscripten.h>
#include <errno.h>
#include <pthread.h>

int main()
{
    return WinMain(reinterpret_cast<HINSTANCE>(0x00400000), 0, GetCommandLineA(), SW_SHOWNORMAL);
}

// pthreads-win32 needs these around its static library; Emscripten's
// pthreads (single-threaded here) do not.
extern "C" int pthread_win32_process_attach_np(void)
{
    return 1;
}

extern "C" int pthread_win32_process_detach_np(void)
{
    return 1;
}

extern "C" int pthread_win32_thread_detach_np(void)
{
    return 1;
}

// The pthread_t calls of vc1_web_crt.h: there are no other threads.
extern "C" int vc1_web_pthread_create(pthread_t* thread, const pthread_attr_t* attributes,
    void* (*start)(void*), void* argument)
{
    (void)attributes;
    (void)start;
    (void)argument;
    *thread = 0;
    return EAGAIN;
}

extern "C" int vc1_web_pthread_join(pthread_t thread, void** result)
{
    (void)thread;
    (void)result;
    return ESRCH;
}

extern "C" int vc1_web_pthread_detach(pthread_t thread)
{
    (void)thread;
    return ESRCH;
}

extern "C" pthread_t vc1_web_pthread_self(void)
{
    return 1;
}

extern "C" int vc1_web_pthread_equal(pthread_t a, pthread_t b)
{
    return a == b;
}
