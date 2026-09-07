#ifndef VC1_KD_DISPLAY_H
#define VC1_KD_DISPLAY_H

#include "kd/iface/idisplay.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

class KdDisplay;

extern IAbstract* g_kdDisplay;

// VTABLE: VC1 0x004d0b78
// SIZE 0x5c
class KdDisplay : public IDisplay
{
public:
    Unknown face4;           // 0x04
    unsigned pad08;          // 0x08
    unsigned pad0c;          // 0x0c
    Unknown face10;          // 0x10
    Unknown face14;          // 0x14
    Unknown face18;          // 0x18
    Abstract abstract;       // 0x1c
    unsigned pad20[4];       // 0x20
    float zero30;            // 0x30
    float one34;             // 0x34
    float zero38;            // 0x38
    float zero3c;            // 0x3c
    float zero40;            // 0x40
    float one44;             // 0x44
    float zero48;            // 0x48
    float zero4c;            // 0x4c
    float zero50;            // 0x50
    float one54;             // 0x54
    unsigned char enabled58; // 0x58

    KdDisplay();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int Unmapped0c();                       // vtable+0x0c
    int Unmapped10();                       // vtable+0x10
    int Unmapped14();                       // vtable+0x14
    int Unmapped18();                       // vtable+0x18
    int Unmapped1c();                       // vtable+0x1c
    int SetSize(unsigned width, unsigned height); // vtable+0x20
    int Unmapped24();                       // vtable+0x24
    int Unmapped28();                       // vtable+0x28
    void* GetMixer();                       // vtable+0x2c
    int Unmapped30();                       // vtable+0x30
    int Unmapped34();                       // vtable+0x34
    int Unmapped38();                       // vtable+0x38
};

typedef char Vc1KdDisplaySizeCheck[sizeof(KdDisplay) == 0x5c ? 1 : -1];

KdDisplay* ConstructKdDisplay(KdDisplay* self);
void** CreateKdDisplay(void** out);

}

#endif
