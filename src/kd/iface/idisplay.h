#ifndef VC1_KD_IDISPLAY_H
#define VC1_KD_IDISPLAY_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// SIZE 0x04. Method names from Script0cBody: CreateRenderTarget, GetWidth,
// GetHeight, IsWindowed, SetWindowed, ShowKeyboard.
struct IDisplay : IUnknown
{
    virtual void* Display0c(void* a, void* b) = 0;          // vtable+0x0c
    virtual void* Display10() = 0;                         // vtable+0x10
    virtual int GetWidth() = 0;                            // vtable+0x14
    virtual int GetHeight() = 0;                           // vtable+0x18
    virtual void* Display1c() = 0;                         // vtable+0x1c
    virtual int SetSize(unsigned width, unsigned height) = 0; // vtable+0x20
    virtual unsigned char IsWindowed() = 0;                // vtable+0x24
    virtual int SetWindowed(unsigned char arg);             // vtable+0x28
    virtual void* GetMixer();                              // vtable+0x2c
    virtual void Display30(int unused) = 0;                // vtable+0x30
    virtual void** CreateTexture(void** out) = 0;     // vtable+0x34
    virtual void** CreateRenderTarget(void** out, unsigned width, unsigned height) = 0; // vtable+0x38
};

typedef char Vc1IDisplaySizeCheck[sizeof(IDisplay) == 0x04 ? 1 : -1];

}

#endif
