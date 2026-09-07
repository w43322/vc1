#ifndef VC1_KD_IDISPLAY_H
#define VC1_KD_IDISPLAY_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04
struct IDisplay : IUnknown
{
    virtual int Unmapped0c() = 0;                          // vtable+0x0c
    virtual int Unmapped10() = 0;                          // vtable+0x10
    virtual int Unmapped14() = 0;                          // vtable+0x14
    virtual int Unmapped18() = 0;                          // vtable+0x18
    virtual int Unmapped1c() = 0;                          // vtable+0x1c
    virtual int SetSize(unsigned width, unsigned height) = 0; // vtable+0x20
    virtual int Unmapped24() = 0;                          // vtable+0x24
    virtual int Unmapped28() = 0;                          // vtable+0x28
    virtual void* GetMixer() = 0;                          // vtable+0x2c
    virtual int Unmapped30() = 0;                          // vtable+0x30
    virtual int Unmapped34() = 0;                          // vtable+0x34
    virtual int Unmapped38() = 0;                          // vtable+0x38
};

typedef char Vc1IDisplaySizeCheck[sizeof(IDisplay) == 0x04 ? 1 : -1];

}

#endif
