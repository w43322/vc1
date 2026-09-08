#include "host/runtime.h"

#include <stdarg.h>
#include <stdio.h>

namespace vc1 {

// GLOBAL: VC1 0x004cfa94
unsigned g_kdLogTag;
// GLOBAL: VC1 0x004d062c
const char* g_kdLogPrefix;

// STRING: VC1 0x004d5004
static const char kHostLogBracket[] = "[%s] %s%s\n";
// STRING: VC1 0x004d5010
static const char kHostLogPlain[] = "%s%s\n";
// STRING: VC1 0x004d5020
static const char kHostLogError[] = "error: ";

// ECX=fmt, EAX=va_list, ESI=&g_kdLogTag. Prefix slot is the caller's saved EAX (0x004d062c from RunApp).
// FUNCTION: VC1 0x004b7010
void HostLogV(const char** prefix, const char* fmt, va_list args)
{
    char buf[0x400];
    const char* tag;

    _vsnprintf(buf, 0x400, fmt, args);
    if (prefix && *prefix) {
        KdLog(kHostLogBracket, *prefix, reinterpret_cast<const char*>(&g_kdLogTag), buf);
        return;
    }
    (void)tag;
    KdLog(kHostLogPlain, reinterpret_cast<const char*>(&g_kdLogTag), buf);
}

// cdecl fmt. EAX=prefix slot. ESI forced to 0x004cfa94.
// FUNCTION: VC1 0x004b7070
void HostLog(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HostLogV(&g_kdLogPrefix, fmt, args);
    va_end(args);
}

// cdecl fmt. EAX=prefix slot. ESI forced to 0x004d5020.
// FUNCTION: VC1 0x004b70b0
void HostLogError(const char* fmt, ...)
{
    va_list args;
    const char* tag = kHostLogError;
    va_start(args, fmt);
    (void)tag;
    HostLogV(&g_kdLogPrefix, fmt, args);
    va_end(args);
}

}
