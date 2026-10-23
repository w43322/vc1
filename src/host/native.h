#ifndef VC1_HOST_NATIVE_H
#define VC1_HOST_NATIVE_H

// Native types for the Win32 calls of recovered sources shared by both
// targets: the Win32 headers for the reference build, the SDL3 subset
// (platform/sdl3/win32.h) otherwise.
#if defined(VC1_USE_SDL3)
#include "platform/sdl3/win32.h"
#else
#include "platform/win32/native.h"
#endif

#endif
