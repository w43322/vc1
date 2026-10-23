#ifndef VC1_KDCORE_NATIVE_H
#define VC1_KDCORE_NATIVE_H

// The Win32 SDK headers the original was built against. The reference build
// gets the real SDK; the SDL3 build puts imports/sdl3/include first on the
// include path, which declares only what the game imports.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>

#endif
