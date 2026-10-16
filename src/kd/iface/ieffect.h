#ifndef VC1_KD_IEFFECT_H
#define VC1_KD_IEFFECT_H

#include "kd/iface/irenderable.h"

namespace vc1 {

struct Matrix3;

// Inferred name; IID string "IEffectEmitter" (0x004d4e1c). A running
// instance of one emitter of an effect file. Slot names from the script
// methods of KdEffectEmitter::GetMember.
// SIZE 0x04
// VTABLE: VC1 0x004d7a5c
struct IEffectEmitter : IRenderable
{
    virtual void Reset() = 0;                                 // vtable+0x18
    virtual void SetRenderLayer(int layer) = 0;               // vtable+0x1c
    virtual void SetEnabled(bool enabled) = 0;                // vtable+0x20
    virtual void SetLocalMode(bool local) = 0;                // vtable+0x24
    virtual void SetLocalMatrix(const Matrix3& matrix) = 0;   // vtable+0x28
};

// Inferred name; IID string "IEffect" (0x004d4e2c). A loaded .pyro file
// (ResourceManager GetEffect, IDisplay::CreateEffect).
// SIZE 0x04
struct IEffect : IUnknown
{
    // Script "CreateEmitter". A null name takes the file's first emitter; an
    // unknown name gives null.
    virtual KdPtr<IEffectEmitter> CreateEmitter(const char* name) = 0; // vtable+0x0c
};

typedef char Vc1IEffectSizeCheck[sizeof(IEffect) == 0x04 ? 1 : -1];

extern const char* g_iidEffect;
extern const char* g_iidEffectEmitter;

template <>
struct KdIid<IEffectEmitter>
{
    static const char** Get() { return &g_iidEffectEmitter; }
};

}

#endif
