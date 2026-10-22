#ifndef VC1_PLATFORM_WIN32_CLOCK_H
#define VC1_PLATFORM_WIN32_CLOCK_H

#include "platform/win32/native.h"

namespace vc1 {
typedef LARGE_INTEGER PlatformClockSample;
extern double g_nsPerTick;

inline void PlatformReadCounter(PlatformClockSample* sample)
{
    QueryPerformanceCounter(sample);
}
}
#endif
