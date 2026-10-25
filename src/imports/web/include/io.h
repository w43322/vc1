#ifndef VC1_IMPORTS_WEB_IO_H
#define VC1_IMPORTS_WEB_IO_H

// <io.h> for the web build: _access, _fileno and friends are mapped in
// vc1_web_crt.h. There are no OS file handles; the crash handler that asks
// for one (kdcore/crash.cpp) gets none, as for a closed descriptor.
#include <stdint.h>

static inline intptr_t _get_osfhandle(int fd)
{
    (void)fd;
    return -1;
}

#endif
