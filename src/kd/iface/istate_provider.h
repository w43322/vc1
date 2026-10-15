#ifndef VC1_KD_ISTATE_PROVIDER_H
#define VC1_KD_ISTATE_PROVIDER_H

#include "kd/iface/iunknown.h"

namespace vc1 {

// "IStateProvider" (0x004d4c64). ScriptedObject answers it when its script
// has "GetStateWeight" and forwards both slots to the script with the state
// name. Slot names follow those script members (inferred).
// SIZE 0x04
struct IStateProvider : IUnknown
{
    virtual float GetStateWeight(const char* state) = 0; // vtable+0x0c
    virtual float GetStatePhase(const char* state) = 0;  // vtable+0x10
};

typedef char Vc1IStateProviderSizeCheck[sizeof(IStateProvider) == 0x04 ? 1 : -1];

}

#endif
