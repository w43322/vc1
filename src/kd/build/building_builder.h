#ifndef VC1_KD_BUILDING_BUILDER_H
#define VC1_KD_BUILDING_BUILDER_H

#include "kd/iface/icamera.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iinput_handler.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/itile_object.h"
#include "kd/scene/compound_object.h"
#include "kd/tile/tile_animation_data.h"
#include "bindings/matrix3.h"
#include "bindings/rect.h"

namespace vc1 {

// Inferred name. Factory "BuildingBuilder" (Level.nut CreateBuildingBuilder,
// scripts/BuildingBuilder.nut): the placement tool for a new building. A
// TileObject child ("Tile") shows the building's "Draft" animation frame at
// the cursor; the grid face tints every occupied cell of the IGridMap and the
// renderable face marks the footprint cells that are blocked (not clean, or
// inside a margin rectangle the script added with "AddMarginRect"). Pointer
// moves snap the footprint to the grid ("OnBoundsChange"), a click asks the
// script to build ("OnBuild" with the position, or "OnRefuseBuild").
// QI 0x004b2a90 establishes the face offsets below.
// SIZE 0xd4
// VTABLE: VC1 0x004cf9a8 vc1::IInputHandler
// VTABLE: VC1 0x004cf9bc vc1::IRenderable
// VTABLE: VC1 0x004cf9d4 vc1::IGridRenderer
// VTABLE: VC1 0x004cf9e4 vc1::IScriptObject
// VTABLE: VC1 0x004cf9fc vc1::IScriptLoadable
// VTABLE: VC1 0x004cfa0c vc1::IPositionable
// VTABLE: VC1 0x004cfa20 vc1::IIdentifiable
// VTABLE: VC1 0x004cfa38 vc1::IClassifiable
// VTABLE: VC1 0x004cfa54 vc1::KdComponentGroup
// VTABLE: VC1 0x004cfa7c vc1::IAbstract
class KdBuildingBuilder : public IInputHandler,
                          public IRenderable,
                          public IGridRenderer,
                          public KdCompoundObject
{
public:
    KdRef<IGridMap> gridMap;        // 0x7c
    KdArray<unsigned char> margins; // 0x80 0 inside a margin rectangle, per cell
    KdRef<ICamera> camera;          // 0x90
    KdRef<ITileObject> tile;        // 0x94 the "Tile" child
    bool enabled;                   // 0x98
    Rect bounds;                    // 0x9c the footprint in cells
    Matrix3 position;               // 0xac
    int layerTop;                   // 0xd0 root table "LAYER_Top"

    KdBuildingBuilder() throw();
    ~KdBuildingBuilder() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    int Shutdown();
    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();

    unsigned char OnKey(const unsigned* data);
    unsigned char OnPointer(const unsigned* data);
    int Render(void* graphics);
    int GetRenderOrder();
    void RenderGrid(void* graphics);

    void ShowBuilding(bool value);
    bool IsEnabled();
    void SetEnabled(bool value);
    void SetCamera(const KdPtr<ICamera>& value);
    void AddMarginRect(const Rect& rect);
    bool CheckPlacement();
    void SetGridMap(const KdPtr<IGridMap>& value);
    void SetBuildingData(const KdPtr<ITileAnimationData>& data, int width, int height);

    bool IsCellFree(int x, int y);
    void ResetMargins();
    void TryBuild();
    void UpdateCursor(int x, int y);
};

typedef char Vc1KdBuildingBuilderSizeCheck[sizeof(KdBuildingBuilder) == 0xd4 ? 1 : -1];

void** CreateKdBuildingBuilder(void** out);

// TEMPLATE: VC1 0x00401140
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AEXABV?$KdPtr@UIGridMap@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x004011b0
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AEXABV?$KdPtr@UICamera@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00401220
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AEX_N@Z@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AEX_N@Z@Z
// TEMPLATE: VC1 0x00401290
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AE_NXZ@Z
// TEMPLATE: VC1 0x00401300
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UITileAnimationData@vc1@@@2@HH@Z@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AEXABV?$KdPtr@UITileAnimationData@vc1@@@0@HH@Z@Z
// TEMPLATE: VC1 0x00401370
// ??$CreateScriptMethod@VKdBuildingBuilder@vc1@@P812@AEXABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdBuildingBuilder@0@AEXABURect@0@@Z@Z
// TEMPLATE: VC1 0x004016b0
// ??$Call@VKdBuildingBuilder@vc1@@ABV?$KdPtr@UITileAnimationData@vc1@@@2@HH@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdBuildingBuilder@vc1@@P823@AEXABV?$KdPtr@UITileAnimationData@vc1@@@3@HH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004b2db0
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AEXABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b2e70
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UITileAnimationData@vc1@@@2@HH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b2f10
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b2fb0
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AEX_N@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b3080
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b3120
// ?Dispatch@?$ScriptMethodCall@VKdBuildingBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00401030
// vc1::KdBuildingBuilder::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00401890
// ?QueryInterface@KdBuildingBuilder@vc1@@WCM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004018a0
// ??_EKdBuildingBuilder@vc1@@WCM@AEPAXI@Z
// SYNTHETIC: VC1 0x004018b0
// ?QueryInterface@KdBuildingBuilder@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004018c0
// ?Release@KdBuildingBuilder@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004018d0
// ?AddRef@KdBuildingBuilder@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004018e0
// ?Release@KdBuildingBuilder@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x004018f0
// ?QueryInterface@KdBuildingBuilder@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401900
// ?Release@KdBuildingBuilder@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00401910
// ?AddRef@KdBuildingBuilder@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00401920
// ?QueryInterface@KdBuildingBuilder@vc1@@WDE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401930
// ?QueryInterface@KdBuildingBuilder@vc1@@WBI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401940
// ?Release@KdBuildingBuilder@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x00401950
// ?AddRef@KdBuildingBuilder@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00401960
// ?Release@KdBuildingBuilder@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x00401970
// ?AddRef@KdBuildingBuilder@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x00401980
// ?QueryInterface@KdBuildingBuilder@vc1@@WFI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401990
// ??_EKdBuildingBuilder@vc1@@WBI@AEPAXI@Z
// SYNTHETIC: VC1 0x004019a0
// ?Release@KdBuildingBuilder@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x004019b0
// ?AddRef@KdBuildingBuilder@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004019c0
// ??_EKdBuildingBuilder@vc1@@WFI@AEPAXI@Z
// SYNTHETIC: VC1 0x004019d0
// ?Release@KdBuildingBuilder@vc1@@WFI@AEIXZ
// SYNTHETIC: VC1 0x004019e0
// ?AddRef@KdBuildingBuilder@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004019f0
// ?AddRef@KdBuildingBuilder@vc1@@WDE@AEIXZ
// SYNTHETIC: VC1 0x00401a00
// ?QueryInterface@KdBuildingBuilder@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401a10
// ?AddRef@KdBuildingBuilder@vc1@@WBI@AEIXZ
// SYNTHETIC: VC1 0x00401a20
// ?AddRef@KdBuildingBuilder@vc1@@WFI@AEIXZ
// SYNTHETIC: VC1 0x00401a30
// ?QueryInterface@KdBuildingBuilder@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401a40
// ?Release@KdBuildingBuilder@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00401a50
// ?QueryInterface@KdBuildingBuilder@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00401a60
// ?Release@KdBuildingBuilder@vc1@@W7AEIXZ

}

#endif
