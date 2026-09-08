#ifndef VC1_KD_ISCRIPT_OBJECT_H
#define VC1_KD_ISCRIPT_OBJECT_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IScriptObject : IUnknown
{
    virtual unsigned char Script0c(void* a, void* b) = 0; // vtable+0x0c
    virtual unsigned char Script10(void* a, void* b) = 0; // vtable+0x10
    virtual int Script14() = 0;                            // vtable+0x14
};

typedef char Vc1IScriptObjectSizeCheck[sizeof(IScriptObject) == 0x04 ? 1 : -1];

}

#endif
