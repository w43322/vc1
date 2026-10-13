#ifndef VC1_KD_IDISPLAY_H
#define VC1_KD_IDISPLAY_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// Default vftable written by IDisplay(); every slot is _purecall.
// Method names from KdDisplayBase::Script0c (0x00466b00): CreateRenderTarget, GetWidth,
// GetHeight, IsWindowed, SetWindowed, ShowKeyboard.
// SIZE 0x04
// VTABLE: VC1 0x004d6200
struct IDisplay : IUnknown
{
    virtual void* Display0c(void* a, void* b) = 0;          // vtable+0x0c
    virtual void* Display10() = 0;                         // vtable+0x10
    virtual int GetWidth() = 0;                            // vtable+0x14
    virtual int GetHeight() = 0;                           // vtable+0x18
    virtual void* Display1c() = 0;                         // vtable+0x1c
    virtual unsigned char SetSize(unsigned width, unsigned height) = 0; // vtable+0x20
    virtual unsigned char IsWindowed() = 0;                // vtable+0x24
    virtual int SetWindowed(unsigned char arg) = 0;        // vtable+0x28
    virtual void* GetMixer() = 0;                          // vtable+0x2c
    virtual void Display30(int unused) = 0;                // vtable+0x30
    virtual void** CreateTexture(void** out) = 0;     // vtable+0x34
    virtual void** CreateRenderTarget(void** out, unsigned width, unsigned height) = 0; // vtable+0x38
    virtual void** Display3c(void** out) = 0;              // vtable+0x3c
};

typedef char Vc1IDisplaySizeCheck[sizeof(IDisplay) == 0x04 ? 1 : -1];

}

#endif
