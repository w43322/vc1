#ifndef VC1_KD_COMPOUND_OBJECT_H
#define VC1_KD_COMPOUND_OBJECT_H

#include "kd/iface/iunknown.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

// Prefix before IAbstract. Ctor 0x00473800 writes IScriptObject/ICloneable and
// other vptrs here; recover those bases instead of composing IAbstract.
// SIZE 0x4c
struct KdCompoundObjectHead
{
    unsigned char data[0x4c];
};

typedef char Vc1KdCompoundHeadSizeCheck[sizeof(KdCompoundObjectHead) == 0x4c ? 1 : -1];

// SIZE 0x70. IAbstract at +0x4c, refs at +0x50.
class KdCompoundObject : public KdCompoundObjectHead,
                         public IAbstract
{
public:
    unsigned char tail[0x1c]; // 0x54

    KdCompoundObject();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1KdCompoundObjectSizeCheck[sizeof(KdCompoundObject) == 0x70 ? 1 : -1];

void** CreateKdCompoundObject(void** out);

}

#endif
