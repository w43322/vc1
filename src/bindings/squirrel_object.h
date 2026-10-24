#ifndef VC1_BINDINGS_SQUIRREL_OBJECT_H
#define VC1_BINDINGS_SQUIRREL_OBJECT_H
#include <sqplus.h>

namespace vc1 {

// 0x00499d00; original ABI: size=EAX, left=EDX, right=ECX. Returns -1, 0, or 1.
// Used by Color/Vector `_cmp` (0x004d36e0) wrappers, other byte-wise compares
// and libpng (src/kdcore/kd_string_c.h), hence C linkage.
extern "C" int MemCompare(const void* left, const void* right, unsigned size);

// "error: %s\n"; KdLog format that replaces every SqPlus throw (SQPLUS_THROW).
extern const char g_sqplusErrorFormat[];

// Root table holder, SqPlus SquirrelVM::_root. Original 0x004e96e0.
extern SquirrelObject g_rootTable;

}

#endif
