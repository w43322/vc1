#include "host/runtime.h"

#include <stdarg.h>
#include <stdio.h>

namespace vc1 {

// GLOBAL: VC1 0x004cfa94
unsigned g_kdLogTag;
// GLOBAL: VC1 0x004d062c
const char* g_kdLogPrefix;
// GLOBAL: VC1 0x004e9664
const char* g_emptyCountedBegin = reinterpret_cast<const char*>(&g_kdLogTag);
// GLOBAL: VC1 0x004e9668
unsigned g_emptyCountedSize;

// STRING: VC1 0x004d5004
static const char kHostLogBracket[] = "[%s] %s%s\n";
// STRING: VC1 0x004d5010
static const char kHostLogPlain[] = "%s%s\n";
// STRING: VC1 0x004d5020
static const char kHostLogError[] = "error: ";

// ECX=fmt, EAX=va_list, ESI=tag, cdecl prefix. "[name] tag text" when the
// caller's name slot (a factory name such as g_kdNameDisplay, or the app's
// g_kdLogPrefix) is set.
// FUNCTION: VC1 0x004b7010
void HostLogV(const char** prefix, const char* tag, const char* fmt, va_list args)
{
    char buf[0x400];

    _vsnprintf(buf, 0x400, fmt, args);
    if (*prefix) {
        KdLog(kHostLogBracket, *prefix, tag, buf);
        return;
    }
    KdLog(kHostLogPlain, tag, buf);
}

// EAX=prefix, cdecl fmt and varargs. Tag is the empty string.
// FUNCTION: VC1 0x004b7070
void HostLog(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HostLogV(prefix, reinterpret_cast<const char*>(&g_kdLogTag), fmt, args);
    va_end(args);
}

// EAX=prefix, cdecl fmt and varargs. Tag "error: ".
// FUNCTION: VC1 0x004b70b0
void HostLogError(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HostLogV(prefix, kHostLogError, fmt, args);
    va_end(args);
}

}
