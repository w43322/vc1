#ifndef VC1_KD_ISCRIPT_OBJECT_H
#define VC1_KD_ISCRIPT_OBJECT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Default vftable written by IScriptObject(); most-derived classes replace it.
// SIZE 0x04
// VTABLE: VC1 0x004d1e68
struct IScriptObject : IUnknown
{
    virtual unsigned char Script0c(void* a, void* b) = 0; // vtable+0x0c
    virtual unsigned char Script10(void* a, void* b) = 0; // vtable+0x10
    virtual const char* Script14() = 0;                  // vtable+0x14
};

typedef char Vc1IScriptObjectSizeCheck[sizeof(IScriptObject) == 0x04 ? 1 : -1];

}

#endif
