#ifndef VC1_KD_SYSTEM_H
#define VC1_KD_SYSTEM_H

#include "kd/iface/isystem.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iinput_stream.h"
#include "kd/iface/iabstract.h"

namespace vc1 {

class KdSystem;

extern IAbstract* g_kdSystem;

// VTABLE: VC1 0x004d1930 vc1::ISystem
// VTABLE: VC1 0x004d1984 vc1::IRenderable
// VTABLE: VC1 0x004d199c vc1::IScriptObject
// VTABLE: VC1 0x004d19b4 vc1::IInputStream
// VTABLE: VC1 0x004d19dc vc1::IAbstract
// SIZE 0xd8
class KdSystem : public ISystem,
                 public IRenderable,
                 public IScriptObject,
                 public IInputStream,
                 public IAbstract
{
public:
    unsigned refs;           // 0x34
    unsigned char tail[160]; // 0x38

    KdSystem();
    ~KdSystem();
    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    int Shutdown();                         // vtable+0x0c
    int Unmapped10();                       // vtable+0x10
    int Unmapped14();                       // vtable+0x14
    int Unmapped18();                       // vtable+0x18
    int Unmapped1c();                       // vtable+0x1c
    void* Unmapped20();                     // vtable+0x20
    int Unmapped24();                       // vtable+0x24
    int Start(IUnknown** object);           // vtable+0x28
    unsigned char Script0c(void* a, void* b);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
};

typedef char Vc1KdSystemSizeCheck[sizeof(KdSystem) == 0xd8 ? 1 : -1];

// SYNTHETIC: VC1 0x00407470
// vc1::KdSystem::`scalar deleting destructor'
// SYNTHETIC: VC1 0x004074d0
// ?QueryInterface@KdSystem@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004074e0
// ?QueryInterface@KdSystem@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004074f0
// ?Release@KdSystem@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00407500
// ?QueryInterface@KdSystem@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407510
// ?Release@KdSystem@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00407530
// ?Release@KdSystem@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00407540
// ?AddRef@KdSystem@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00407550
// ?QueryInterface@KdSystem@vc1@@WDA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407580
// ?AddRef@KdSystem@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00407590
// ?Release@KdSystem@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x004075a0
// ??_EKdSystem@vc1@@WDA@AEPAXI@Z
// SYNTHETIC: VC1 0x004075b0
// ?AddRef@KdSystem@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004075d0
// ?AddRef@KdSystem@vc1@@WDA@AEIXZ

void ConstructKdSystem(KdSystem* self);
void** CreateKdSystem(void** out);
void* SystemBaseQueryInterface(KdSystem* self, const char** iid);

}

#endif
