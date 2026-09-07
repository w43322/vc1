#ifndef VC1_KD_ISCRIPT_OBJECT_H
#define VC1_KD_ISCRIPT_OBJECT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IScriptObject : IUnknown
{
};

typedef char Vc1IScriptObjectSizeCheck[sizeof(IScriptObject) == 0x04 ? 1 : -1];

}

#endif
