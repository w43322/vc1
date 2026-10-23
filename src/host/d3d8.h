#ifndef VC1_HOST_D3D8_H
#define VC1_HOST_D3D8_H

// Native types for the recovered D3D8 and window calls: Win32 for the
// reference build, the SDL3 D3D8/Win32 subset otherwise.
#if defined(VC1_USE_SDL3)
#include "platform/sdl3/d3d8.h"
#else
#include "platform/win32/native.h"
#endif

#endif
