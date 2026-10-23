#ifndef VC1_PLATFORM_SDL3_RENDER_H
#define VC1_PLATFORM_SDL3_RENDER_H

#include <SDL3/SDL.h>
namespace vc1 {
// window.cpp / d3d8.cpp. The display drives them through the D3D8 and Win32
// subsets (platform/sdl3/d3d8.h).
extern SDL_Window* g_sdlWindow;
extern SDL_Renderer* g_sdlRenderer;
void SdlPollEvents();
}
#endif
