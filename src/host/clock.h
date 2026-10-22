#ifndef VC1_HOST_CLOCK_H
#define VC1_HOST_CLOCK_H

// Compile-time clock provider. Keep the sample shape and conversion in callers
// so the matching build retains the original QPC loads and x87 conversion.
#if defined(VC1_USE_SDL3)
#include "platform/sdl3/clock.h"
#else
#include "platform/win32/clock.h"
#endif

#endif
