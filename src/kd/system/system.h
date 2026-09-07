#ifndef VC1_KD_SYSTEM_H
#define VC1_KD_SYSTEM_H

#include "kd/iface/isystem.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

class KdSystem;

extern IAbstract* g_kdSystem;

// VTABLE: VC1 0x004d1930
// SIZE 0xd8
class KdSystem : public ISystem
{
public:
    Unknown face4;          // 0x04
    Unknown face8;          // 0x08
    Unknown faceC;          // 0x0c
    unsigned pad10[8];      // 0x10
    Abstract abstract;      // 0x30
    unsigned char tail[164]; // 0x34

    KdSystem();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int Unmapped0c();                       // vtable+0x0c
    int Unmapped10();                       // vtable+0x10
    int Unmapped14();                       // vtable+0x14
    int Unmapped18();                       // vtable+0x18
    int Unmapped1c();                       // vtable+0x1c
    int Unmapped20();                       // vtable+0x20
    int Unmapped24();                       // vtable+0x24
    int Start();                            // vtable+0x28
};

typedef char Vc1KdSystemSizeCheck[sizeof(KdSystem) == 0xd8 ? 1 : -1];

void ConstructKdSystem(KdSystem* self);
void** CreateKdSystem(void** out);

}

#endif
