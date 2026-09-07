#ifndef VC1_KD_COMPOUND_OBJECT_H
#define VC1_KD_COMPOUND_OBJECT_H

#include "kd/iface/iunknown.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

// SIZE 0x70
class KdCompoundObject : public IUnknown
{
public:
    void* unk04;             // 0x04
    void* unk08;             // 0x08
    void* unk0c;             // 0x0c
    unsigned pad[15];        // 0x10
    Abstract abstract;       // 0x4c
    unsigned char tail[32];  // 0x50

    KdCompoundObject();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
};

typedef char Vc1KdCompoundObjectSizeCheck[sizeof(KdCompoundObject) == 0x70 ? 1 : -1];

KdCompoundObject* ConstructKdCompoundObject(KdCompoundObject* self);
void** CreateKdCompoundObject(void** out);

}

#endif
