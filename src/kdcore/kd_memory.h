#ifndef VC1_KDCORE_KD_MEMORY_H
#define VC1_KDCORE_KD_MEMORY_H

#include <string.h>

namespace vc1 {

// Inferred name (OpenKODE kdMemcpy). The game's own copies go through this
// wrapper: its memcpy has a variable size, so it stays a CRT call after
// inlining even where the caller's size is constant (Matrix3 copies, the
// saved transforms in the builders' Render). No original address: inlined
// everywhere. Stock SqPlus calls memcpy directly and keeps the intrinsic.
inline void* KdMemcpy(void* destination, const void* source, unsigned size)
{
    return memcpy(destination, source, size);
}

}

#endif
