#ifndef VC1_IMPORTS_SDL3_SDL3_H
#define VC1_IMPORTS_SDL3_SDL3_H

// Helpers shared by the SDL3 implementations of the original's imports.
#include <windows.h>
#include <SDL3/SDL.h>

namespace sdl3 {

// Initializes an SDL subsystem once; SDL quits at exit (sdl3.cpp).
bool Init(SDL_InitFlags flags);

// The SDL window behind a HWND; 0 for a message-only window (user32.cpp).
SDL_Window* WindowOf(HWND window);

// Advances BASS volume slides; the message pump calls it (bass.cpp).
void UpdateAudio();

}

#endif
