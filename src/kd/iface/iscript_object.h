#ifndef VC1_KD_ISCRIPT_OBJECT_H
#define VC1_KD_ISCRIPT_OBJECT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IScriptObject : IUnknown
{
};

typedef char Vc1IScriptObjectSizeCheck[sizeof(IScriptObject) == 0x04 ? 1 : -1];

// SIZE 0x04
struct ScriptObject : IScriptObject
{
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1ScriptObjectSizeCheck[sizeof(ScriptObject) == 0x04 ? 1 : -1];

}

#endif
