#ifndef VC1_PLATFORM_SDL3_CLOCK_H
#define VC1_PLATFORM_SDL3_CLOCK_H

#include <SDL3/SDL_timer.h>

namespace vc1 {
struct PlatformClockSample { long long QuadPart; };
extern double g_nsPerTick;

inline void PlatformReadCounter(PlatformClockSample* sample)
{
    sample->QuadPart = static_cast<long long>(SDL_GetPerformanceCounter());
}
}
#endif
