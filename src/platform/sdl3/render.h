#ifndef VC1_PLATFORM_SDL3_RENDER_H
#define VC1_PLATFORM_SDL3_RENDER_H

#include <SDL3/SDL.h>
namespace vc1 {
struct IUnknown;
struct KdRenderTargetNode;
extern SDL_Window* g_sdlWindow;
extern SDL_Renderer* g_sdlRenderer;
bool SdlCreateDisplay(unsigned width, unsigned height);
void SdlDestroyDisplay();
void SdlPollEvents();
void SdlClear(unsigned color);
unsigned SdlTextureWidth(IUnknown* texture);
unsigned SdlTextureHeight(IUnknown* texture);
void SdlUnlockTexture(IUnknown* texture);
unsigned long SdlReleaseTexture(IUnknown* texture);
bool SdlBeginTarget(KdRenderTargetNode* target);
void SdlEndFrame();
}
#endif
