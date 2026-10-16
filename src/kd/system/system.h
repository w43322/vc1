#ifndef VC1_KD_SYSTEM_H
#define VC1_KD_SYSTEM_H

#include "kd/iface/isystem.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ifont.h"
#include "kd/component_group.h"
#include "host/runtime.h"

namespace vc1 {

extern IAbstract* g_kdSystem;

// Inferred name. Built by its constructor (0x00462350, ESI=this) before
// CreateKdSystem writes the KdSystem vftables. The IComponentGroup slots of
// the +0x0c group reuse the ISystem overrides through WM@ thunks.
// SIZE 0xd8
// VTABLE: VC1 0x004d63f0 vc1::ISystem
// VTABLE: VC1 0x004d6444 vc1::IRenderable
// VTABLE: VC1 0x004d645c vc1::IScriptObject
// VTABLE: VC1 0x004d6474 vc1::KdComponentGroup
// VTABLE: VC1 0x004d649c vc1::IAbstract
class KdSystemBase : public ISystem,
                     public IRenderable,
                     public IScriptObject,
                     public KdComponentGroup,
                     public IAbstract
{
public:
    KdTimer* timer;             // 0x38  10 ms frame timer from Start
    KdRef<IDisplay> display;          // 0x3c
    KdRef<IUnknown> resourceManager;  // 0x40
    KdRef<IUnknown> camera;           // 0x44  default camera
    KdRef<IUnknown> activeCamera;     // 0x48
    KdRef<IFunctionalGroup> renderGroup; // 0x4c
    KdRef<IFunctionalGroup> updateGroup; // 0x50
    KdRef<IFunctionalGroup> inputGroup;  // 0x54
    float timeScale;            // 0x58
    KdFactory slices;           // 0x5c  int per update-slice bit (8 entries)
    unsigned clearColor;        // 0x6c  0xffffff = do not clear
    unsigned time;              // 0x70
    unsigned pad74;             // 0x74
    __int64 lastTicks;          // 0x78  ns
    __int64 startTicks;         // 0x80  ns
    unsigned heapBytes;         // 0x88
    unsigned field8c;           // 0x8c
    char consoleText[0x40];     // 0x90  overlay line; drawn while non-empty
    KdRef<IFont> consoleFont;   // 0xd0  res/fonts/console.fnt, loaded on first draw
    unsigned paddingd4;         // 0xd4

    KdSystemBase() throw();
    virtual ~KdSystemBase() throw();

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    int Shutdown() throw();
    void RegisterObject(const KdString& name, const KdPtr<IAbstract>& object);
    KdPtr<IUnknown> UnregisterObject(const KdString& name);
    KdPtr<IUnknown> FindObject(const KdString& name);
    KdFactory* CollectObjects(const char** iid);
    void* GetRecords();
    int ClearCollected();
    unsigned char Start(IUnknown** object);
    void Update();
    void Render() throw();
    void Frame();
    void ResetClock();
    unsigned GetTime();
    void** GetDisplay(void** out);
    KdPtr<IAbstract> GetCamera();
    void SetCamera(const KdPtr<IAbstract>& value);
    float GetTimeScale();
    void SetTimeScale(float scale);

    int Render(void* graphics);
    int GetRenderOrder();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* a, void* b);
    const char* GetTypeName();
};

typedef char Vc1KdSystemBaseSizeCheck[sizeof(KdSystemBase) == 0xd8 ? 1 : -1];

// SYNTHETIC: VC1 0x0041af40
// vc1::KdSystemBase::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0041b3f0
// ?AddRef@KdSystemBase@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x0041b410
// ?QueryInterface@KdSystemBase@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041b430
// ?QueryInterface@KdSystemBase@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041b440
// ?Release@KdSystemBase@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041b450
// ?QueryInterface@KdSystemBase@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041b460
// ?Release@KdSystemBase@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041b470
// ?Release@KdSystemBase@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0041b480
// ?AddRef@KdSystemBase@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0041b490
// ?QueryInterface@KdSystemBase@vc1@@WDA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0041b4a0
// ?AddRef@KdSystemBase@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0041b4c0
// ?Release@KdSystemBase@vc1@@WDA@AEIXZ
// SYNTHETIC: VC1 0x0041b4d0
// ??_EKdSystemBase@vc1@@WDA@AEPAXI@Z
// SYNTHETIC: VC1 0x0041b4e0
// ?AddRef@KdSystemBase@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004074c0
// ?CollectObjects@KdSystemBase@vc1@@WM@AEPAVKdFactory@2@PAPBD@Z
// SYNTHETIC: VC1 0x00407520
// ?GetRecords@KdSystemBase@vc1@@WM@AEPAXXZ
// SYNTHETIC: VC1 0x00407560
// ?ClearCollected@KdSystemBase@vc1@@WM@AEHXZ
// SYNTHETIC: VC1 0x00407570
// ?RegisterObject@KdSystemBase@vc1@@WM@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z
// SYNTHETIC: VC1 0x004075c0
// ?UnregisterObject@KdSystemBase@vc1@@WM@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z
// SYNTHETIC: VC1 0x004075e0
// ?Shutdown@KdSystemBase@vc1@@WM@AEHXZ
// SYNTHETIC: VC1 0x004075f0
// ?FindObject@KdSystemBase@vc1@@WM@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z

// SIZE 0xd8
// VTABLE: VC1 0x004d1930 vc1::ISystem
// VTABLE: VC1 0x004d1984 vc1::IRenderable
// VTABLE: VC1 0x004d199c vc1::IScriptObject
// VTABLE: VC1 0x004d19b4 vc1::KdComponentGroup
// VTABLE: VC1 0x004d19dc vc1::IAbstract
class KdSystem : public KdSystemBase
{
public:
    KdSystem() throw() {}

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char Start(IUnknown** object); // vtable+0x28
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* a, void* b);
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

void** CreateKdSystem(void** out);

}

#endif
