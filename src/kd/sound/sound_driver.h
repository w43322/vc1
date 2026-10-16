#ifndef VC1_KD_SOUND_DRIVER_H
#define VC1_KD_SOUND_DRIVER_H

#include "kd/iface/isound_driver.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

extern IAbstract* g_kdSoundDriver;

// SIZE 0x14
// VTABLE: VC1 0x004d5b48 vc1::ISoundDriver
// VTABLE: VC1 0x004d5b74 vc1::IShutdownable
// VTABLE: VC1 0x004d5b84 vc1::IScriptObject
// VTABLE: VC1 0x004d5b9c vc1::IAbstract
class KdSoundDriver : public ISoundDriver,
                      public IShutdownable,
                      public IScriptObject,
                      public IAbstract
{
public:

    KdSoundDriver();
    ~KdSoundDriver();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int SetEnabled();                       // vtable+0x0c
    void* Sound10(void* arg);               // vtable+0x10
    void* Sound14(void* arg);               // vtable+0x14
    bool Start();                           // vtable+0x18
    bool Stop();                            // vtable+0x1c
    bool Pause();                           // vtable+0x20
    int GetVolume();                        // vtable+0x24
    bool SetVolume(int volume);             // vtable+0x28
    int Shutdown();                         // vtable+0x0c
    unsigned char Script0c(const char* name, SquirrelObject* out);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
};

typedef char Vc1KdSoundDriverSizeCheck[sizeof(KdSoundDriver) == 0x14 ? 1 : -1];

// SYNTHETIC: VC1 0x00419800
// vc1::KdSoundDriver::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00419a60
// ?Release@KdSoundDriver@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00419a70
// ?QueryInterface@KdSoundDriver@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00419a80
// ?Release@KdSoundDriver@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00419a90
// ?QueryInterface@KdSoundDriver@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00419aa0
// ?Release@KdSoundDriver@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00419ab0
// ?QueryInterface@KdSoundDriver@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00419ac0
// ?AddRef@KdSoundDriver@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00419ad0
// ??_EKdSoundDriver@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x00419ae0
// ?AddRef@KdSoundDriver@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00419af0
// ?AddRef@KdSoundDriver@vc1@@WM@AEIXZ

// Script methods (Script0c): Start / Stop / Pause share one closure type.

// TEMPLATE: VC1 0x00419850
// ??$CreateScriptMethod@VKdSoundDriver@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdSoundDriver@0@AE_NXZ@Z

// TEMPLATE: VC1 0x004198c0
// ??$CreateScriptMethod@VKdSoundDriver@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdSoundDriver@0@AEHXZ@Z

// TEMPLATE: VC1 0x00419930
// ??$CreateScriptMethod@VKdSoundDriver@vc1@@P812@AE_NH@Z@vc1@@YA?AVSquirrelObject@@P8KdSoundDriver@0@AE_NH@Z@Z

// TEMPLATE: VC1 0x004199a0
// ??$Call@VKdSoundDriver@vc1@@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdSoundDriver@vc1@@P823@AE_NXZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004199f0
// ??$Call@VKdSoundDriver@vc1@@H@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdSoundDriver@vc1@@P823@AE_NH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00468400
// ?Dispatch@?$ScriptMethodCall@VKdSoundDriver@vc1@@P812@AE_NH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004684a0
// ?Dispatch@?$ScriptMethodCall@VKdSoundDriver@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00468570
// ?Dispatch@?$ScriptMethodCall@VKdSoundDriver@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z

void** CreateKdSoundDriver(void** out);

}

#endif
