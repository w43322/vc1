#ifndef VC1_KD_ISYSTEM_H
#define VC1_KD_ISYSTEM_H

#include "kd/iface/icomponent_group.h"

namespace vc1 {

struct IAbstract;

// Slot names after IComponentGroup are inferred from the KdSystemBase bodies.
// SIZE 0x04
struct ISystem : IComponentGroup
{
    virtual unsigned char Start(IUnknown** object) = 0; // vtable+0x28
    virtual void Update() = 0;                      // vtable+0x2c
    virtual void Render() = 0;                      // vtable+0x30
    virtual void Frame() = 0;                       // vtable+0x34
    virtual void ResetClock() = 0;                  // vtable+0x38
    virtual unsigned GetTime() = 0;                 // vtable+0x3c
    virtual void** GetDisplay(void** out) = 0;      // vtable+0x40
    virtual KdPtr<IAbstract> GetCamera() = 0;                // vtable+0x44
    virtual void SetCamera(const KdPtr<IAbstract>& camera) = 0; // vtable+0x48
    virtual float GetTimeScale() = 0;               // vtable+0x4c
    virtual void SetTimeScale(float scale) = 0;     // vtable+0x50
};

typedef char Vc1ISystemSizeCheck[sizeof(ISystem) == 0x04 ? 1 : -1];

}

#endif
