// SDL's native entry wrapper calls this main. The VC9 reference keeps its
// recovered stdcall WinMain and original CRT entry unchanged.
#include "platform/sdl3/runtime.h"
#include "platform/sdl3/gamepad.h"
#include "platform/sdl3/render.h"
#include "kd/sound/bass_api.h"
#include "mod/mods.h"
#include <SDL3/SDL_main.h>
#include <ctime>

namespace vc1 { extern const char kVirtualCity[]="Virtual City"; }
namespace vc1 { void InitSdlMods(); }
int RunSdlBackendTests();
int main(int argc,char** argv)
{
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)) {SDL_Log("SDL initialization: %s",SDL_GetError());return 1;}
    if(!vc1::InitSdlGamepad()) {SDL_Quit();return 1;}
    vc1::g_nsPerTick=1.0e9/static_cast<double>(SDL_GetPerformanceFrequency());
    vc1::g_runScratch0=argc-1;
    vc1::g_runScratch1=reinterpret_cast<unsigned>(argv+1);
    srand(static_cast<unsigned>(time(nullptr)));
    vc1::InitThreading();
    vc1::InitKdFilesystem();
    if(argc==2 && !strcmp(argv[1],"--backend-test")) {
        int result=RunSdlBackendTests();
        vc1::ShutdownSdlGamepad();vc1::FreeBumpHeaps();vc1::DestroyKdObjects();SDL_Quit();return result;
    }
    vc1::InitSdlMods();
    int result=vc1::RunApp();
    vc1::ShutdownMods();
    vc1::ShutdownSdlGamepad();
    vc1::FreeBumpHeaps();
    vc1::DestroyKdObjects();
    vc1::ShutdownThreading();
    SDL_Quit();
    return result;
}
