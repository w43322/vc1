#ifndef VC1_HOST_RUNTIME_H
#define VC1_HOST_RUNTIME_H

// Compile-time runtime selection for remaining consumers of host globals.
// Backend-independent consumers should include host/platform.h directly.
#if defined(VC1_USE_SDL3)
#include "platform/sdl3/runtime.h"
#else
#include "platform/win32/runtime.h"
#endif

#endif
