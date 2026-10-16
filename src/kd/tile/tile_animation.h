#ifndef VC1_KD_TILE_TILE_ANIMATION_H
#define VC1_KD_TILE_TILE_ANIMATION_H

#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itile_object.h"
#include "kd/tile/tile_animation_data.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "TileAnimation" (AnimatedObject.nut CreateAnim):
// plays one named animation of a TileAnimationData on its slave TileObject.
// The channel follows the direction of the positionable's X axis (one
// channel per equal angle sector), the frame the elapsed time. QI 0x004a4d60
// establishes the face offsets below.
// SIZE 0x34
// VTABLE: VC1 0x004d1d40 vc1::IScriptObject
// VTABLE: VC1 0x004d1d58 vc1::IShutdownable
// VTABLE: VC1 0x004d1d68 vc1::IPositionableUser
// VTABLE: VC1 0x004d1d78 vc1::IAnimateable
// VTABLE: VC1 0x004d1d8c vc1::IObjectControl
// VTABLE: VC1 0x004d1d9c vc1::IUpdateable
// VTABLE: VC1 0x004d1db4 vc1::IAbstract
class KdTileAnimation : public IScriptObject,
                        public IShutdownable,
                        public IPositionableUser,
                        public IAnimateable,
                        public IObjectControl,
                        public IUpdateable,
                        public IAbstract
{
public:
    KdRef<ITileAnimationData> data;     // 0x20
    KdRef<ITileObject> slave;           // 0x24
    KdRef<IPositionable> positionable;  // 0x28
    const TileAnimation* current;       // 0x2c
    int elapsed;                        // 0x30 ms; not set by the constructor

    KdTileAnimation() throw();
    ~KdTileAnimation() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    int Shutdown();
    void SetPositionable(const KdPtr<IPositionable>& source);
    bool SetAnimation(const KdString& name, float phase);
    const KdStringList& GetAnimationNames();
    void SetSlaveObject(const KdPtr<IAbstract>& slave);
    int GetUpdateLayer();
    int SetLayer(unsigned layer);
    void Update(unsigned sliceBit, int dt);

    void SetAnimationData(const KdPtr<ITileAnimationData>& value);
    int GetAnimationTime(const KdString& name);
    const char* GetAnimationName();
};

typedef char Vc1KdTileAnimationSizeCheck[sizeof(KdTileAnimation) == 0x34 ? 1 : -1];

void** CreateKdTileAnimation(void** out);

// SYNTHETIC: VC1 0x004087f0
// vc1::KdTileAnimation::`scalar deleting destructor'
// TEMPLATE: VC1 0x00408810
// ??$CreateScriptMethod@VKdTileAnimation@vc1@@P812@AEXABV?$KdPtr@UITileAnimationData@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileAnimation@0@AEXABV?$KdPtr@UITileAnimationData@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00408880
// ??$CreateScriptMethod@VKdTileAnimation@vc1@@P812@AEPBDXZ@vc1@@YA?AVSquirrelObject@@P8KdTileAnimation@0@AEPBDXZ@Z
// TEMPLATE: VC1 0x004088f0
// ??$CreateScriptMethod@VKdTileAnimation@vc1@@P812@AEHABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileAnimation@0@AEHABVKdString@0@@Z@Z
// TEMPLATE: VC1 0x00408960
// ??$Call@VKdTileAnimation@vc1@@ABV?$KdPtr@UITileAnimationData@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTileAnimation@vc1@@P823@AEXABV?$KdPtr@UITileAnimationData@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00408a00
// ??$Call@VKdTileAnimation@vc1@@ABVKdString@2@@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdTileAnimation@vc1@@P823@AEHABVKdString@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004a51a0
// ?Dispatch@?$ScriptMethodCall@VKdTileAnimation@vc1@@P812@AEHABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a5240
// ?Dispatch@?$ScriptMethodCall@VKdTileAnimation@vc1@@P812@AEPBDXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a52e0
// ?Dispatch@?$ScriptMethodCall@VKdTileAnimation@vc1@@P812@AEXABV?$KdPtr@UITileAnimationData@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// SYNTHETIC: VC1 0x00408aa0
// ?QueryInterface@KdTileAnimation@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408ab0
// ?AddRef@KdTileAnimation@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00408ac0
// ?QueryInterface@KdTileAnimation@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408ad0
// ?Release@KdTileAnimation@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00408ae0
// ?QueryInterface@KdTileAnimation@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408af0
// ?Release@KdTileAnimation@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00408b00
// ?QueryInterface@KdTileAnimation@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408b10
// ?Release@KdTileAnimation@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00408b20
// ?AddRef@KdTileAnimation@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00408b30
// ?QueryInterface@KdTileAnimation@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408b40
// ?Release@KdTileAnimation@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00408b50
// ?AddRef@KdTileAnimation@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00408b60
// ?QueryInterface@KdTileAnimation@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408b70
// ?Release@KdTileAnimation@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00408b80
// ?AddRef@KdTileAnimation@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00408b90
// ?Release@KdTileAnimation@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00408ba0
// ?AddRef@KdTileAnimation@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00408bb0
// ??_EKdTileAnimation@vc1@@WBI@AEPAXI@Z
// SYNTHETIC: VC1 0x00408bc0
// ?AddRef@KdTileAnimation@vc1@@WBE@AEIXZ

}

#endif
