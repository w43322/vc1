#ifndef VC1_PYRO_PYRO_CRT_H
#define VC1_PYRO_PYRO_CRT_H

// The statically linked Pyro library was built without the memcpy/memset
// intrinsics: even constant-size copies call the CRT (PyroLayer::BuildQuad
// 0x004513d0 clears a matrix with memset(0x30) and copies it with
// memcpy(0x30)). Every Pyro source includes this first.
#include <string.h>

#pragma function(memcpy, memset)

#endif
