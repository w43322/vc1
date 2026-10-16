#ifndef VC1_KD_ISOUND_H
#define VC1_KD_ISOUND_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// A playable BASS channel: music streams (KdMusic) and samples (KdSound).
// Slot names are inferred from the bodies (BASS attribute 1 = frequency,
// 2 = volume, 3 = pan) and from the script names bound to them ("Play",
// "Stop", "Pause", "Resume", "SetVolume", "SetLoop", "IsActive", "IsPaused",
// "IsSliding"; KdMusic's IsSliding only tests for a non-stopped stream).
// Volume and pan are percentages; the setters slide over time ms when time > 0.
// SIZE 0x04
struct ISound : IUnknown
{
    virtual int Play(bool restart) = 0;                // vtable+0x0c
    virtual bool Stop() = 0;                           // vtable+0x10
    virtual bool Pause() = 0;                          // vtable+0x14
    virtual bool Resume() = 0;                         // vtable+0x18
    virtual int GetFrequency() = 0;                    // vtable+0x1c
    virtual bool SetFrequency(int value, int time) = 0; // vtable+0x20
    virtual int GetVolume() = 0;                       // vtable+0x24
    virtual bool SetVolume(int value, int time) = 0;   // vtable+0x28
    virtual int GetPan() = 0;                          // vtable+0x2c
    virtual bool SetPan(int value, int time) = 0;      // vtable+0x30
    virtual bool SetLoop(bool loop) = 0;               // vtable+0x34
    virtual bool IsActive() = 0;                       // vtable+0x38 playing
    virtual bool IsPaused() = 0;                       // vtable+0x3c
    virtual bool IsSliding() = 0;                      // vtable+0x40
};

typedef char Vc1ISoundSizeCheck[sizeof(ISound) == 0x04 ? 1 : -1];

}

#endif
