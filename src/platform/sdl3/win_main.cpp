// Transitional SDL3 game entry: retain the existing Windows display, file,
// threading, D3D8 and BASS implementations while SDL3 supplies time/gamepads.
// This is a separate portable-build implementation, not an original mapping.
#include "platform/win32/runtime.h"
#include "platform/sdl3/gamepad.h"
#include <SDL3/SDL.h>

namespace vc1 {
extern const char kVirtualCity[] = "Virtual City";
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    if (!vc1::GetKdEnv(vc1::kKdParentWindow)) {
        CreateMutexA(0, 0, vc1::kVirtualCity);
        if (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
            return SetForegroundWindow(FindWindowA(vc1::kKdWindowClass, "Virtual City"));
    }

    if (!vc1::InitSdlGamepad()) {
        SDL_Log("SDL3 gamepad initialization failed: %s", SDL_GetError());
        return -1;
    }
    vc1::InitTimingAndSeh();
    vc1::g_nsPerTick = 1.0e9 / static_cast<double>(SDL_GetPerformanceFrequency());
    const int result = vc1::RunApp();
    vc1::ShutdownSdlGamepad();
    vc1::ShutdownNetwork();
    vc1::FreeBumpHeaps();
    vc1::DestroyKdObjects();
    vc1::DestroyTlsBlock(vc1::g_tlsPrimary);
    vc1::DestroyTlsBlock(vc1::g_tlsSecondary);
    vc1::ShutdownApcAndLibraries();
    SDL_Quit();
    return result;
}
