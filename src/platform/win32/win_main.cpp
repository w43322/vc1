#include "platform/win32/runtime.h"

namespace vc1 {
// GLOBAL: VC1 0x004d0c48
extern const char kVirtualCity[] = "Virtual City";
}

// Inferred name. CRT at 0x0042f564 pushes hInstance=0x400000, hPrev=0, lpCmdLine, nCmdShow
// then CALL 0x00486b00; RET 0x10. Four stdcall args are unused. Not an original symbol.
// FUNCTION: VC1 0x00486b00
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int show)
{
    (void)instance;
    (void)prevInstance;
    (void)cmdLine;
    (void)show;

    if (!vc1::GetKdEnv(vc1::kKdParentWindow)) {
        CreateMutexA(0, 0, vc1::kVirtualCity);
        if (GetLastError() != ERROR_ALREADY_EXISTS && GetLastError() != ERROR_ACCESS_DENIED) {
            // Unique instance: fall through to startup.
        }
        else {
            const HWND existing = FindWindowA(vc1::kKdWindowClass, "Virtual City");
            return SetForegroundWindow(existing);
        }
    }

    {
        int result;
        vc1::InitTimingAndSeh();
        result = vc1::RunApp();
        vc1::ShutdownNetwork();
        vc1::FreeBumpHeaps();
        vc1::DestroyKdObjects();
        vc1::DestroyTlsBlock(vc1::g_tlsPrimary);
        vc1::DestroyTlsBlock(vc1::g_tlsSecondary);
        vc1::ShutdownApcAndLibraries();
        return result;
    }
}
