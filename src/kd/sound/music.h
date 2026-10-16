#ifndef VC1_KD_SOUND_MUSIC_H
#define VC1_KD_SOUND_MUSIC_H

#include "kd/iface/isound.h"
#include "kd/iface/iloadable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

#include <stdlib.h>

namespace vc1 {

extern const char* g_kdNameMusic;

// Inferred name. Script class name "Music" (0x004d4bec). A BASS file stream
// over a KD file (ResourceManager GetMusic, ISoundDriver vtable+0x14). QI at
// 0x0045e7c0: ISound this, ILoadable +4, IScriptObject +8, IAbstract +0x0c.
// SIZE 0x18
// VTABLE: VC1 0x004d66f0 vc1::ISound
// VTABLE: VC1 0x004d6734 vc1::ILoadable
// VTABLE: VC1 0x004d6744 vc1::IScriptObject
// VTABLE: VC1 0x004d675c vc1::IAbstract
class KdMusic : public ISound,
                public ILoadable,
                public IScriptObject,
                public IAbstract
{
public:
    unsigned long stream; // 0x14  BASS stream handle

    KdMusic();
    ~KdMusic();
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

    float GetPosition();
    bool SetPosition(float seconds);

    static void operator delete(void* p) { free(p); }
};

typedef char Vc1KdMusicSizeCheck[sizeof(KdMusic) == 0x18 ? 1 : -1];

// Inferred name. malloc(0x18), constructor 0x0045e970, IAbstract face to
// *out (ESI in the original).
void** CreateKdMusic(void** out);

// SYNTHETIC: VC1 0x0041c3b0
// vc1::KdMusic::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041c830
// ?QueryInterface@KdMusic@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c840
// ?Release@KdMusic@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041c850
// ?Release@KdMusic@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041c860
// ?AddRef@KdMusic@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041c870
// ??_EKdMusic@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x0041c880
// ?AddRef@KdMusic@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041c890
// ?AddRef@KdMusic@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041c8a0
// ?QueryInterface@KdMusic@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c8b0
// ?QueryInterface@KdMusic@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041c8c0
// ?Release@KdMusic@vc1@@W3AEIXZ

// Script methods (Script0c).

// TEMPLATE: VC1 0x0041c400
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AEH_N@Z@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AEH_N@Z@Z

// TEMPLATE: VC1 0x0041c470
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AE_NXZ@Z

// TEMPLATE: VC1 0x0041c4e0
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AE_NM@Z@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AE_NM@Z@Z

// TEMPLATE: VC1 0x0041c550
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AEMXZ@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AEMXZ@Z

// TEMPLATE: VC1 0x0041c5c0
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AE_NHH@Z@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AE_NHH@Z@Z

// TEMPLATE: VC1 0x0041c630
// ??$CreateScriptMethod@VKdMusic@vc1@@P812@AE_N_N@Z@vc1@@YA?AVSquirrelObject@@P8KdMusic@0@AE_N_N@Z@Z

// TEMPLATE: VC1 0x0045eaf0
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AE_N_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045eb90
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AE_NHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045ec30
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AEMXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045ecf0
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AE_NM@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045ed90
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045ee30
// ?Dispatch@?$ScriptMethodCall@VKdMusic@vc1@@P812@AEH_N@Z@vc1@@SAHPAUSQVM@@@Z

}

#endif
