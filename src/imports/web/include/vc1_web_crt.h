#ifndef VC1_IMPORTS_WEB_VC1_WEB_CRT_H
#define VC1_IMPORTS_WEB_VC1_WEB_CRT_H

// Force-included into every source of the web target (cmake/web.cmake): the
// MSVC CRT names the recovered sources and vendored libraries use, mapped to
// Emscripten's musl. Semantics follow the VC9 CRT where the game relies on
// them.

// InitKdFilesystem indexes the CRT's stdio array (iob + 1, iob + 2), which
// needs a complete FILE. musl's public headers give it one byte; this stand-in is larger
// than musl's FILE, and __iob_func (web/crt.cpp) fills it with copies.
struct _IO_FILE
{
    unsigned char vc1_opaque[256];
};
#define __DEFINED_struct__IO_FILE

// KdThread (kdcore/runtime.h, SIZE 0x14) starts with pthreads-win32's 8-byte
// pthread_t. musl's is one word, so the type is widened here, and the calls
// that take it by value go to web/web.cpp. The web build runs no threads (the
// SDL3 build keeps them suspended): creating one fails as Emscripten's
// single-threaded pthreads would.
// ptw32_handle_t is two words, aligned to 4.
typedef unsigned long long __attribute__((aligned(4))) pthread_t;
#define __DEFINED_pthread_t
#define pthread_create vc1_web_pthread_create
#define pthread_join vc1_web_pthread_join
#define pthread_detach vc1_web_pthread_detach
#define pthread_self vc1_web_pthread_self
#define pthread_equal vc1_web_pthread_equal

#include <alloca.h>
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#ifdef __cplusplus
// Before Squirrel's function-like macros (sqobject.h's type(obj)), which
// libc++'s <math.h> would otherwise meet.
#include <cmath>
#include <limits>
#endif

#ifdef __cplusplus
extern "C" {
#endif
// The pthreads-win32 extensions the KD runtime calls; the web build uses
// Emscripten's pthreads, where they have nothing to do (web/web.cpp).
int pthread_win32_process_attach_np(void);
int pthread_win32_process_detach_np(void);
int pthread_win32_thread_detach_np(void);
// klibc's scanf (third_party/klibc) declares this in its own <inttypes.h>.
uintmax_t strntoumax(const char* text, char** end, int base, size_t length);
#ifdef __cplusplus
}
#endif

// The VC9 CRT's FILE is struct _iobuf (the SDK subset declares __iob_func).
#define _iobuf _IO_FILE
// musl's time_t is 64-bit already.
#define _time64 time
#define __time64_t time_t
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#define stricmp strcasecmp
#define _snprintf snprintf
#define _vsnprintf vc1_web_vsnprintf
#define _stat stat
#define _fstat fstat
#define _fileno fileno
#define _access access
#define _alloca alloca
#define _errno() (&errno)
#define _mkdir(path) mkdir((path), 0777)

// VC9's _vsnprintf: -1 when the text does not fit, and an "unbounded" count
// (the game passes 0xffffffff) is fine; musl refuses counts above INT_MAX.
static inline int vc1_web_vsnprintf(char* buffer, size_t count, const char* format, va_list args)
{
    const int length = vsnprintf(buffer, count > 0x7fffffff ? 0x7fffffff : count, format, args);
    if (!buffer && !count) {
        return length;
    }
    return length < 0 || (size_t)length > count ? -1 : length;
}

// _rotl / _rotr are clang builtins in MSVC-extension mode.

static inline char* _itoa(int value, char* out, int radix)
{
    static const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buffer[34];
    char* p = buffer + sizeof(buffer) - 1;
    unsigned int magnitude = (radix == 10 && value < 0) ? 0u - (unsigned int)value
        : (unsigned int)value;
    char* q = out;
    *p = 0;
    do {
        *--p = digits[magnitude % (unsigned int)radix];
        magnitude /= (unsigned int)radix;
    } while (magnitude);
    if (radix == 10 && value < 0) {
        *q++ = '-';
    }
    while ((*q++ = *p++) != 0) {
    }
    return out;
}

#endif
