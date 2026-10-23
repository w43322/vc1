// SDL setup for the SDL3 imports. There is no SDL main: the CRT entry point
// calls the recovered WinMain, and each import initializes what it needs.
#include "sdl3.h"

// SDL_SetMainReady only; SDL must not supply its own WinMain.
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>

#include <stdlib.h>

namespace sdl3 {

static void Quit()
{
    SDL_Quit();
}

bool Init(SDL_InitFlags flags)
{
    static bool ready = false;
    if (!ready) {
        SDL_SetMainReady();
        atexit(Quit);
        ready = true;
    }
    if ((SDL_WasInit(flags) & flags) == flags) {
        return true;
    }
    if (!SDL_InitSubSystem(flags)) {
        SDL_Log("SDL initialization: %s", SDL_GetError());
        return false;
    }
    return true;
}

}
