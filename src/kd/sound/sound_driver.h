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
    unsigned char Sound18();              // vtable+0x18
    unsigned char Sound1c();              // vtable+0x1c
    unsigned char Sound20();              // vtable+0x20
    float Sound24();                       // vtable+0x24
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

void** CreateKdSoundDriver(void** out);

}

#endif
