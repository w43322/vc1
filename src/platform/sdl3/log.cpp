#include "platform/sdl3/runtime.h"

#include <stdarg.h>
#include <stdio.h>

namespace vc1 {

unsigned g_kdLogTag;

const char* g_kdLogPrefix;

const char* g_emptyCountedBegin = reinterpret_cast<const char*>(&g_kdLogTag);

unsigned g_emptyCountedSize;

static const char kHostLogBracket[] = "[%s] %s%s\n";

static const char kHostLogPlain[] = "%s%s\n";

static const char kHostLogError[] = "error: ";

// ECX=fmt, EAX=va_list, ESI=tag, cdecl prefix. "[name] tag text" when the
// caller's name slot (a factory name such as g_kdNameDisplay, or the app's
// g_kdLogPrefix) is set. The original keeps it out of line in all four
// HostLog* wrappers.

__declspec(noinline) void HostLogV(const char** prefix, const char* tag, const char* fmt, va_list args)
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

void HostLog(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HostLogV(prefix, reinterpret_cast<const char*>(&g_kdLogTag), fmt, args);
    va_end(args);
}

// EAX=prefix, cdecl fmt and varargs. Tag "warning: ".

void HostLogWarning(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    HostLogV(prefix, "warning: ", fmt, args);
    va_end(args);
}

// EAX=prefix, cdecl fmt and varargs. Tag "error: ".

void HostLogError(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    HostLogV(prefix, kHostLogError, fmt, args);
    va_end(args);
}

// EAX=prefix, cdecl fmt and varargs. Tag "info: ".

void HostLogInfo(const char** prefix, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    HostLogV(prefix, "info: ", fmt, args);
    va_end(args);
}

}
