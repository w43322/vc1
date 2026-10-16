#ifndef VC1_KD_RENDER_GROUP_RENDERER_H
#define VC1_KD_RENDER_GROUP_RENDERER_H

#include "kd/iface/icomponent_faces.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/scene/scripted_object.h"

namespace vc1 {

// Inferred name. Factory "GroupRenderer" (Level.nut CreateRoutingRenderer): a
// ScriptedObject with its own RenderGroup; objects the script registers
// ("RegisterObject") are drawn by that group after the object itself, while
// visible and when the classificators pass the render mask. QI 0x004aeaa0
// asks the ScriptedObject first, then answers IVisible.
// SIZE 0xe4
// VTABLE: VC1 0x004d10e0 vc1::IVisible
// VTABLE: VC1 0x004d10f4 vc1::IUpdateable
// VTABLE: VC1 0x004d110c vc1::IRenderable
// VTABLE: VC1 0x004d1124 vc1::IInputHandler
// VTABLE: VC1 0x004d1138 vc1::IStateProvider
// VTABLE: VC1 0x004d114c vc1::IScriptObject
// VTABLE: VC1 0x004d1164 vc1::IScriptLoadable
// VTABLE: VC1 0x004d1174 vc1::IPositionable
// VTABLE: VC1 0x004d1188 vc1::IIdentifiable
// VTABLE: VC1 0x004d11a0 vc1::IClassifiable
// VTABLE: VC1 0x004d11bc vc1::KdComponentGroup
// VTABLE: VC1 0x004d11e4 vc1::IAbstract
class KdGroupRenderer : public IVisible, public KdScriptedObject
{
public:
    KdRef<IRenderable> renderGroup; // 0xdc
    bool visible;                   // 0xe0 not set by the constructor

    KdGroupRenderer() throw();
    ~KdGroupRenderer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    int Shutdown();
    void SetVisible(bool value);
    bool IsVisible();
    int RenderMasked(void* graphics, void* mask);

    bool RegisterObject(const KdPtr<IAbstract>& object);
};

typedef char Vc1KdGroupRendererSizeCheck[sizeof(KdGroupRenderer) == 0xe4 ? 1 : -1];

// TEMPLATE: VC1 0x00405050
// ??$CreateScriptMethod@VKdGroupRenderer@vc1@@P812@AE_NABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGroupRenderer@0@AE_NABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x004050c0
// ??$Call@VKdGroupRenderer@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdGroupRenderer@vc1@@P823@AE_NABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004ae940
// ?Dispatch@?$ScriptMethodCall@VKdGroupRenderer@vc1@@P812@AE_NABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00405030
// vc1::KdGroupRenderer::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00405170
// ?Release@KdGroupRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00405180
// ?QueryInterface@KdGroupRenderer@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405190
// ?AddRef@KdGroupRenderer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004051a0
// ?Release@KdGroupRenderer@vc1@@WDM@AEIXZ
// SYNTHETIC: VC1 0x004051b0
// ?QueryInterface@KdGroupRenderer@vc1@@WDM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004051c0
// ?AddRef@KdGroupRenderer@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x004051d0
// ?Release@KdGroupRenderer@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x004051e0
// ?QueryInterface@KdGroupRenderer@vc1@@WCA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004051f0
// ?AddRef@KdGroupRenderer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00405200
// ?Release@KdGroupRenderer@vc1@@WGA@AEIXZ
// SYNTHETIC: VC1 0x00405210
// ?QueryInterface@KdGroupRenderer@vc1@@WGA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405220
// ??_EKdGroupRenderer@vc1@@WCA@AEPAXI@Z
// SYNTHETIC: VC1 0x00405230
// ?Release@KdGroupRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00405240
// ?QueryInterface@KdGroupRenderer@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405250
// ?AddRef@KdGroupRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00405260
// ??_EKdGroupRenderer@vc1@@WGA@AEPAXI@Z
// SYNTHETIC: VC1 0x00405270
// ?AddRef@KdGroupRenderer@vc1@@WDM@AEIXZ
// SYNTHETIC: VC1 0x00405280
// ?Release@KdGroupRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00405290
// ?QueryInterface@KdGroupRenderer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004052a0
// ?AddRef@KdGroupRenderer@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x004052b0
// ?AddRef@KdGroupRenderer@vc1@@WGA@AEIXZ
// SYNTHETIC: VC1 0x004052c0
// ?Release@KdGroupRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004052d0
// ?QueryInterface@KdGroupRenderer@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004052e0
// ?AddRef@KdGroupRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004052f0
// ?Release@KdGroupRenderer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00405300
// ?QueryInterface@KdGroupRenderer@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405310
// ?AddRef@KdGroupRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00405320
// ?Release@KdGroupRenderer@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00405330
// ?QueryInterface@KdGroupRenderer@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405340
// ?AddRef@KdGroupRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00405350
// ?Release@KdGroupRenderer@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x00405360
// ?QueryInterface@KdGroupRenderer@vc1@@WDE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405370
// ??_EKdGroupRenderer@vc1@@WDE@AEPAXI@Z
// SYNTHETIC: VC1 0x00405380
// ?Release@KdGroupRenderer@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00405390
// ?QueryInterface@KdGroupRenderer@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004053a0
// ?AddRef@KdGroupRenderer@vc1@@WBA@AEIXZ

}

#endif
