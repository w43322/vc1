#ifndef VC1_KD_TILE_ANIMATION_DATA_H
#define VC1_KD_TILE_ANIMATION_DATA_H

#include "kd/iface/icomponent_faces.h"
#include "kd/name_map.h"

namespace vc1 {

// Inferred name. One named animation of a TileAnimationData script table:
// { Time = int, Channels = [[tile, ...], ...] }.
// SIZE 0x14
struct TileAnimation
{
    KdObjArray<KdArray<int> > channels; // 0x00
    int time;                           // 0x10

    TileAnimation() : time(0) {}
    TileAnimation(const KdObjArray<KdArray<int> >& frames, int duration)
        : channels(frames), time(duration)
    {
    }
    TileAnimation& operator=(const TileAnimation& source)
    {
        channels = source.channels;
        time = source.time;
        return *this;
    }
};

typedef char Vc1TileAnimationSizeCheck[sizeof(TileAnimation) == 0x14 ? 1 : -1];
typedef char Vc1TileAnimationRecordSizeCheck[sizeof(KdNameRecord<TileAnimation>) == 0x24 ? 1 : -1];
typedef char Vc1ScriptRecordSizeCheck[sizeof(KdNameRecord<SquirrelObject>) == 0x1c ? 1 : -1];

// Inferred name; IID string "ITileAnimationData" (0x004cfe2c).
// SIZE 0x04
struct ITileAnimationData : IUnknown
{
    virtual const TileAnimation* Find(const KdString& name) = 0; // vtable+0x0c
    virtual const KdStringList& GetNames() = 0;                  // vtable+0x10
};

extern const char* g_iidTileAnimationData;

// Inferred name. Factory string "TileAnimationData" (0x004cfd9c). Animations
// by name plus the names in script iteration order.
// SIZE 0x30
// VTABLE: VC1 0x004d1dd8 vc1::ITileAnimationData
// VTABLE: VC1 0x004d1dec vc1::IScriptLoadable
// VTABLE: VC1 0x004d1dfc vc1::IAbstract
class KdTileAnimationData : public ITileAnimationData,
                            public IScriptLoadable,
                            public IAbstract
{
public:
    KdNameMap<TileAnimation> animations; // 0x10
    KdStringList names;                  // 0x20

    // Inlined into the factory; the original also keeps an unreferenced
    // out-of-line copy (EAX=this) at 0x004a4610.
    __forceinline KdTileAnimationData() {}
    void* QueryInterface(const char** iid);       // vtable+0x00
    unsigned AddRef();                            // vtable+0x04
    unsigned Release();                           // vtable+0x08
    const TileAnimation* Find(const KdString& name);
    const KdStringList& GetNames();
    bool LoadFromScript(SquirrelObject& table);   // IScriptLoadable vtable+0x0c

    static void operator delete(void* p) { free(p); }
};

typedef char Vc1KdTileAnimationDataSizeCheck[sizeof(KdTileAnimationData) == 0x30 ? 1 : -1];

// Container instantiations used by LoadFromScript.

// TEMPLATE: VC1 0x00408c20
// ??1?$KdNameRecord@UTileAnimation@vc1@@@vc1@@QAE@XZ

// TEMPLATE: VC1 0x00408c90
// ??1?$KdNameMap@UTileAnimation@vc1@@@vc1@@QAE@XZ

// TEMPLATE: VC1 0x00408cf0
// ?Reserve@?$KdNameMap@UTileAnimation@vc1@@@vc1@@QAEXID@Z

// TEMPLATE: VC1 0x00408e40
// ??1?$KdNameMap@VSquirrelObject@@@vc1@@QAE@XZ

// TEMPLATE: VC1 0x00408eb0
// ??$GetScriptValue@U?$KdNameMap@VSquirrelObject@@@vc1@@@vc1@@YA?AU?$KdNameMap@VSquirrelObject@@@0@ABVSquirrelObject@@@Z

// TEMPLATE: VC1 0x00408f30
// ??$GetScriptValue@U?$KdObjArray@U?$KdArray@H@vc1@@@vc1@@@vc1@@YA?AU?$KdObjArray@U?$KdArray@H@vc1@@@0@ABVSquirrelObject@@@Z

// TEMPLATE: VC1 0x00408fc0
// ?CopyRange@?$KdObjArray@U?$KdArray@H@vc1@@@vc1@@SAPAU?$KdArray@H@2@PBU32@0PAU32@@Z

// TEMPLATE: VC1 0x00409030
// ?ConstructRange@?$KdNameMap@UTileAnimation@vc1@@@vc1@@SAXPAU?$KdNameRecord@UTileAnimation@vc1@@@2@0@Z

// TEMPLATE: VC1 0x00409080
// ?LowerBound@?$KdNameMap@UTileAnimation@vc1@@@vc1@@QBEPAU?$KdNameRecord@UTileAnimation@vc1@@@2@ABVKdString@2@@Z

// TEMPLATE: VC1 0x00409150
// ?LowerBound@?$KdNameMap@VSquirrelObject@@@vc1@@QBEPAU?$KdNameRecord@VSquirrelObject@@@2@ABVKdString@2@@Z

// TEMPLATE: VC1 0x00409600
// ??4?$KdNameRecord@UTileAnimation@vc1@@@vc1@@QAEAAU01@ABU01@@Z

// TEMPLATE: VC1 0x00409670
// ?InsertSlot@?$KdNameMap@UTileAnimation@vc1@@@vc1@@QAEPAU?$KdNameRecord@UTileAnimation@vc1@@@2@PAU32@@Z

// TEMPLATE: VC1 0x004096e0
// ??A?$KdNameMap@VSquirrelObject@@@vc1@@QAEAAVSquirrelObject@@ABVKdString@1@@Z

// TEMPLATE: VC1 0x00409760
// ??1?$KdNameRecord@VSquirrelObject@@@vc1@@QAE@XZ

// TEMPLATE: VC1 0x004097d0
// ?Reserve@?$KdNameMap@VSquirrelObject@@@vc1@@QAEXID@Z

// TEMPLATE: VC1 0x00409850
// ?Insert@?$KdNameMap@VSquirrelObject@@@vc1@@QAEPAU?$KdNameRecord@VSquirrelObject@@@2@PAU32@ABU32@@Z

// TEMPLATE: VC1 0x004098b0
// ?ConstructRange@?$KdNameMap@VSquirrelObject@@@vc1@@SAXPAU?$KdNameRecord@VSquirrelObject@@@2@0@Z

// TEMPLATE: VC1 0x00409fb0
// ?Reserve@?$KdObjArray@U?$KdArray@H@vc1@@@vc1@@QAEXI@Z

// TEMPLATE: VC1 0x0040a170
// ?DestroyRange@?$KdObjArray@U?$KdArray@H@vc1@@@vc1@@SAXPAU?$KdArray@H@2@0@Z

// SYNTHETIC: VC1 0x00408bd0
// vc1::KdTileAnimationData::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00409900
// ?Release@KdTileAnimationData@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00409910
// ?QueryInterface@KdTileAnimationData@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00409920
// ?Release@KdTileAnimationData@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00409930
// ?QueryInterface@KdTileAnimationData@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00409940
// ??_EKdTileAnimationData@vc1@@W7AEPAXI@Z
// SYNTHETIC: VC1 0x00409950
// ?AddRef@KdTileAnimationData@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00409960
// ?AddRef@KdTileAnimationData@vc1@@W7AEIXZ

}

#endif
