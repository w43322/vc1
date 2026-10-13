#include "kd/display/pyro.h"

#include <stdlib.h>

namespace vc1 {

// Inlined into the scalar deleting destructor (0x0042ed40).
PyroLibrary::~PyroLibrary()
{
    if (buffer) {
        free(buffer);
        buffer = 0;
    }
    if (resource) {
        delete resource;
        resource = 0;
    }
}

// Frees +0x0c and resets the library state; body not fully recovered.
// STUB: VC1 0x00450080
void PyroLibrary::Done()
{
    if (buffer) {
        free(buffer);
    }
    buffer = 0;
}

// RET 4. Statically linked Pyro library code; body not recovered.
// STUB: VC1 0x004500c0
int PyroLibrary::Init(IPyroDevice* value)
{
    device = value;
    field10 = 0x200;
    return 0;
}

// RET 4. Body not recovered.
// STUB: VC1 0x004566b0
int PyroLibrary::Pyro04(void* a)
{
    (void)a;
    return 0;
}

// RET 4. Body not recovered.
// STUB: VC1 0x00456800
int PyroLibrary::Pyro00(void* a)
{
    (void)a;
    return 0;
}

}
