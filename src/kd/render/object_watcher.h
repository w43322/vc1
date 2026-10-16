#ifndef VC1_KD_OBJECT_WATCHER_H
#define VC1_KD_OBJECT_WATCHER_H

#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iupdateable.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/icamera.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/irender_target.h"
#include "kd/iface/itexture.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred name ("ObjectWatcher" component). Every update it moves its camera
// onto the watched positionable (plus an offset) and renders the renderable
// into its own render target, keeping the sub-pixel rounding as the
// correction. Constructor 0x004ae1a0 (ESI=this), destructor 0x004adcb0.
// SIZE 0x54
// VTABLE: VC1 0x004d1264 vc1::IScriptObject
// VTABLE: VC1 0x004d127c vc1::IShutdownable
// VTABLE: VC1 0x004d128c vc1::IUpdateable
// VTABLE: VC1 0x004d12a4 vc1::IIdentifiable
// VTABLE: VC1 0x004d12bc vc1::IAbstract
class KdObjectWatcher : public IScriptObject,
                        public IShutdownable,
                        public IUpdateable,
                        public IIdentifiable,
                        public IAbstract
{
public:
    KdRef<IPositionable> positionable; // 0x28
    KdRef<IRenderable> renderable;     // 0x2c
    RenderMask renderMask;             // 0x30
    KdRef<ICamera> camera;             // 0x38
    KdRef<IRenderTarget> target;       // 0x3c
    bool enabled;                      // 0x40  not set by the constructor
    Vector2 offset;                    // 0x44
    Vector2 correction;                // 0x4c

    KdObjectWatcher();
    ~KdObjectWatcher();

    void* QueryInterface(const char** iid); // vtable+0x00
    unsigned AddRef();                      // vtable+0x04
    unsigned Release();                     // vtable+0x08
    unsigned char Script0c(const char* name, SquirrelObject* out);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
    int Shutdown();                         // IShutdownable vtable+0x0c
    int Unmapped0c();                       // IUpdateable vtable+0x0c
    int SetLayer(unsigned layer);           // IUpdateable vtable+0x10
    void Update(unsigned sliceBit, int dt); // IUpdateable vtable+0x14

    const Vector2& GetCorrection();
    void SetEnabled(bool value);
    void SetRenderable(const KdPtr<IRenderable>& value);
    void SetPositionable(const KdPtr<IPositionable>& value);
    KdPtr<ITexture> GetTexture();
    void SetCamera(const KdPtr<ICamera>& value, const Vector2& cameraOffset);
    void SetRenderMask(int lo, int hi);
    void InitRender(int width, int height);
};

typedef char Vc1KdObjectWatcherSizeCheck[sizeof(KdObjectWatcher) == 0x54 ? 1 : -1];

// SYNTHETIC: VC1 0x00405480
// vc1::KdObjectWatcher::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00405900
// ?QueryInterface@KdObjectWatcher@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405910
// ?Release@KdObjectWatcher@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00405920
// ?AddRef@KdObjectWatcher@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x00405930
// ?Release@KdObjectWatcher@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00405940
// ?AddRef@KdObjectWatcher@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00405950
// ??_EKdObjectWatcher@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x00405960
// ?AddRef@KdObjectWatcher@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00405970
// ?AddRef@KdObjectWatcher@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00405980
// ?QueryInterface@KdObjectWatcher@vc1@@WCA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405990
// ?QueryInterface@KdObjectWatcher@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004059a0
// ?Release@KdObjectWatcher@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x004059b0
// ??_EKdObjectWatcher@vc1@@WCA@AEPAXI@Z
// SYNTHETIC: VC1 0x004059c0
// ?QueryInterface@KdObjectWatcher@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004059d0
// ?Release@KdObjectWatcher@vc1@@W3AEIXZ

}

#endif
