#ifndef VC1_KD_SOUND_SOUND_H
#define VC1_KD_SOUND_SOUND_H

#include "kd/iface/isound.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

#include <stdlib.h>

namespace vc1 {

extern const char* g_kdNameSound;

// Inferred name. Script class name "Sound" (0x004d4bf4). A BASS sample
// loaded from a KD file into memory (ResourceManager GetSound, ISoundDriver
// vtable+0x10), played on up to 8 channels. Every ISound method refreshes
// channels with BASS_SampleGetChannels and applies to all of them. QI at
// 0x0045f8a0: ISound this, ILoadable +4, IScriptObject +8, IAbstract +0x0c.
// SIZE 0x38
// VTABLE: VC1 0x004d6618 vc1::ISound
// VTABLE: VC1 0x004d665c vc1::ILoadable
// VTABLE: VC1 0x004d666c vc1::IScriptObject
// VTABLE: VC1 0x004d6684 vc1::IAbstract
class KdSound : public ISound,
                public ILoadable,
                public IScriptObject,
                public IAbstract
{
public:
    unsigned long sample;      // 0x14  BASS sample handle
    unsigned long channels[8]; // 0x18  BASS_SampleGetChannels output

    KdSound();
    ~KdSound();
    void* QueryInterface(const char** iid);                        // vtable+0x00
    unsigned AddRef();                                             // vtable+0x04
    unsigned Release();                                            // vtable+0x08
    int Play(bool restart);                                        // ISound vtable+0x0c
    bool Stop();                                                   // ISound vtable+0x10
    bool Pause();                                                  // ISound vtable+0x14
    bool Resume();                                                 // ISound vtable+0x18
    int GetFrequency();                                            // ISound vtable+0x1c
    bool SetFrequency(int value, int time);                        // ISound vtable+0x20
    int GetVolume();                                               // ISound vtable+0x24
    bool SetVolume(int value, int time);                           // ISound vtable+0x28
    int GetPan();                                                  // ISound vtable+0x2c
    bool SetPan(int value, int time);                              // ISound vtable+0x30
    bool SetLoop(bool loop);                                       // ISound vtable+0x34
    bool IsActive();                                               // ISound vtable+0x38
    bool IsPaused();                                               // ISound vtable+0x3c
    bool IsSliding();                                              // ISound vtable+0x40
    unsigned char Load(const char* name);                          // ILoadable vtable+0x0c
    unsigned char Script0c(const char* name, SquirrelObject* out); // IScriptObject vtable+0x0c
    unsigned char Script10(void* a, void* b);                      // IScriptObject vtable+0x10
    const char* Script14();                                        // IScriptObject vtable+0x14

    int CountPlaying();
    bool SetChannelVolume(unsigned long channel, int value, int time);
    bool StopChannel(unsigned long channel);

    static void operator delete(void* p) { free(p); }
};

typedef char Vc1KdSoundSizeCheck[sizeof(KdSound) == 0x38 ? 1 : -1];

// Inferred name. malloc(0x38), constructor 0x0045f920, IAbstract face to
// *out (ESI in the original).
void** CreateKdSound(void** out);

// SYNTHETIC: VC1 0x0041be90
// vc1::KdSound::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041c2f0
// ?QueryInterface@KdSound@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c300
// ?QueryInterface@KdSound@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c310
// ?Release@KdSound@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041c320
// ?QueryInterface@KdSound@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c330
// ?Release@KdSound@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041c340
// ?Release@KdSound@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041c350
// ?AddRef@KdSound@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041c360
// ??_EKdSound@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x0041c370
// ?AddRef@KdSound@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041c380
// ?AddRef@KdSound@vc1@@WM@AEIXZ

// Script methods (Script0c).

// TEMPLATE: VC1 0x0041bee0
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AEH_N@Z@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AEH_N@Z@Z

// TEMPLATE: VC1 0x0041bf50
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AE_NXZ@Z

// TEMPLATE: VC1 0x0041bfc0
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AE_NKHH@Z@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AE_NKHH@Z@Z

// TEMPLATE: VC1 0x0041c030
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AE_NHH@Z@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AE_NHH@Z@Z

// TEMPLATE: VC1 0x0041c0a0
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AE_N_N@Z@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AE_N_N@Z@Z

// TEMPLATE: VC1 0x0041c110
// ??$CreateScriptMethod@VKdSound@vc1@@P812@AE_NK@Z@vc1@@YA?AVSquirrelObject@@P8KdSound@0@AE_NK@Z@Z

// TEMPLATE: VC1 0x0045fb60
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AE_NK@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045fc00
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AE_N_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045fca0
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AE_NHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045fd40
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AE_NKHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045fde0
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045fe80
// ?Dispatch@?$ScriptMethodCall@VKdSound@vc1@@P812@AEH_N@Z@vc1@@SAHPAUSQVM@@@Z

}

#endif
