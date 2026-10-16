#ifndef VC1_KD_TERRAIN_BUILDER_H
#define VC1_KD_TERRAIN_BUILDER_H

#include "kd/iface/icamera.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iinput_handler.h"
#include "kd/iface/iroad_renderer.h"
#include "kd/iface/itile_layer.h"
#include "kd/scene/compound_object.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred name. Factory "TerrainBuilder" (Level.nut CreateTerrainBuilder):
// the level's terrain editing tool. In the build-road, build-forest and
// destroy modes (the MODE_* values of its script table) it draws a tint per
// grid cell over the IGridMap and turns pointer input into roads (a dragged
// route through the IGridRouter, previewed in the IRoadRenderer), planted
// forest tiles and removed roads/forest/waste. Each action first asks the
// script ("CanBuildRoad", "OnBuildRoad", "OnBuildForest", "OnDestroy*").
// QI 0x004a5ca0 establishes the face offsets below.
// SIZE 0x108
// VTABLE: VC1 0x004d1c0c vc1::IScriptObject
// VTABLE: VC1 0x004d1c24 vc1::IScriptLoadable
// VTABLE: VC1 0x004d1c34 vc1::IPositionable
// VTABLE: VC1 0x004d1c48 vc1::IIdentifiable
// VTABLE: VC1 0x004d1c60 vc1::IClassifiable
// VTABLE: VC1 0x004d1c7c vc1::KdComponentGroup
// VTABLE: VC1 0x004d1ca4 vc1::IAbstract
// VTABLE: VC1 0x004d1cb4 vc1::IInputHandler
// VTABLE: VC1 0x004d1cc8 vc1::IGridRenderer
class KdTerrainBuilder : public KdCompoundObject,
                         public IInputHandler,
                         public IGridRenderer
{
public:
    int modeDefault;                     // 0x78 "MODE_Default"
    int modeBuildRoad;                   // 0x7c "MODE_BuildRoad"
    int modeBuildForest;                 // 0x80 "MODE_BuildForest"
    int modeDestroy;                     // 0x84 "MODE_Destroy"
    unsigned roadBuildableMask;          // 0x88 1 << cell value
    KdRef<IGridMap> gridMap;             // 0x8c
    KdRef<IGridRouter> router;           // 0x90
    KdRef<IRoadRenderer> roadRenderer;   // 0x94
    KdRef<ICamera> camera;               // 0x98
    KdRef<ITileLayer> forestLayer;       // 0x9c
    KdArray<short> forestTiles;          // 0xa0
    KdArray<short> forestCuttedTiles;    // 0xb0
    KdArray<short> forestBurntTiles;     // 0xc0
    KdRef<ITileLayer> wasteLayer;        // 0xd0
    KdArray<short> wasteTiles;           // 0xd4
    int mode;                            // 0xe4
    bool dragging;                       // 0xe8
    CellCoord cursor;                    // 0xea the hovered cell
    CellCoord dragStart;                 // 0xee
    CellCoord dragEnd;                   // 0xf2
    KdArray<CellCoord> draft;            // 0xf8 the road being placed

    KdTerrainBuilder() throw();
    ~KdTerrainBuilder() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    int Shutdown();

    unsigned char OnKey(const unsigned* data);
    unsigned char OnPointer(const unsigned* data);
    void RenderGrid(void* graphics);

    void SetWasteLayer(const KdPtr<ITileLayer>& layer);
    void SetForestLayer(const KdPtr<ITileLayer>& layer);
    void SetGridRouter(const KdPtr<IGridRouter>& value);
    void RemoveRoadCell(int x, int y);
    void AddRoadCell(int x, int y, bool visible, bool strict);
    void SetCamera(const KdPtr<ICamera>& value);
    void SetRenderer(const KdPtr<IRoadRenderer>& value);
    void SetGridMap(const KdPtr<IGridMap>& value);
    int GetForestLevel();
    int GetCellCount(int value);
    void AddTerrainTile(const Vector2& position, const KdPtr<ITileLayer>& layer,
        const KdArray<short>& tiles, bool append);
    void SetMode(int value);
    void ReplaceTiles(int x, int y, const KdPtr<ITileLayer>& layer,
        const KdArray<short>& from, const KdArray<short>& to);
    void RemoveTiles(int x, int y, const KdPtr<ITileLayer>& layer,
        const KdArray<short>& ids, int maxCount);
    void SetWasteTiles(const KdArray<short>& tiles);
    void SetForestBurntTiles(const KdArray<short>& tiles);
    void SetForestCuttedTiles(const KdArray<short>& tiles);
    void SetForestTiles(const KdArray<short>& tiles);
    void SetDraftToRenderer();
    void BreakRoadCell(int x, int y);

    unsigned CellColor(int x, int y);
    void ScreenToCell(CellCoord* out, int x, int y);
    void SyncRoads();
    void ClearDraft();
    void PlantForest(const CellCoord* cell);
    bool CanBuildDraft();
    void UpdateRoadDraft();
    void HoverRoadCell();
    void DestroyAtCursor();
    void BuildForestAtCursor();
    void CommitRoadDraft();
};

typedef char Vc1KdTerrainBuilderSizeCheck[sizeof(KdTerrainBuilder) == 0x108 ? 1 : -1];

void** CreateKdTerrainBuilder(void** out);

// TEMPLATE: VC1 0x00407650
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABV?$KdPtr@UIGridMap@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x004076c0
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABV?$KdPtr@UIGridRouter@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00407730
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x004077a0
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABV?$KdPtr@UICamera@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00407810
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UITileLayer@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABV?$KdPtr@UITileLayer@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00407880
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABU?$KdArray@F@0@@Z@Z
// TEMPLATE: VC1 0x004078f0
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXH@Z@Z
// TEMPLATE: VC1 0x00407960
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEHH@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEHH@Z@Z
// TEMPLATE: VC1 0x004079d0
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEHXZ@Z
// TEMPLATE: VC1 0x00407a40
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXHH_N0@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXHH_N0@Z@Z
// TEMPLATE: VC1 0x00407ab0
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXHH@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXHH@Z@Z
// TEMPLATE: VC1 0x00407b20
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXABUVector2@2@ABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@_N@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXABUVector2@0@ABV?$KdPtr@UITileLayer@vc1@@@0@ABU?$KdArray@F@0@_N@Z@Z
// TEMPLATE: VC1 0x00407b90
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXHHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXHHABV?$KdPtr@UITileLayer@vc1@@@0@ABU?$KdArray@F@0@H@Z@Z
// TEMPLATE: VC1 0x00407c00
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXHHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@1@Z@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXHHABV?$KdPtr@UITileLayer@vc1@@@0@ABU?$KdArray@F@0@1@Z@Z
// TEMPLATE: VC1 0x00407c70
// ??$CreateScriptMethod@VKdTerrainBuilder@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdTerrainBuilder@0@AEXXZ@Z
// TEMPLATE: VC1 0x00407ce0
// ?AssignRange@?$KdArray@F@vc1@@QAEXPBF0@Z
// TEMPLATE: VC1 0x00407e00
// ??$Call@VKdTerrainBuilder@vc1@@ABV?$KdPtr@UIGridRouter@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABV?$KdPtr@UIGridRouter@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00407ea0
// ??$Call@VKdTerrainBuilder@vc1@@ABV?$KdPtr@UIRoadRenderer@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00407f40
// ??$Call@VKdTerrainBuilder@vc1@@ABV?$KdPtr@UICamera@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABV?$KdPtr@UICamera@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00407fe0
// ??$Call@VKdTerrainBuilder@vc1@@ABV?$KdPtr@UITileLayer@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABV?$KdPtr@UITileLayer@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004080e0
// ??$Call@VKdTerrainBuilder@vc1@@H@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEHH@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00408150
// ??$Call@VKdTerrainBuilder@vc1@@HH_N_N@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXHH_N1@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004081e0
// ??$Call@VKdTerrainBuilder@vc1@@ABUVector2@2@ABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@_N@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABUVector2@3@ABV?$KdPtr@UITileLayer@vc1@@@3@ABU?$KdArray@F@3@_N@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00408310
// ??$Call@VKdTerrainBuilder@vc1@@HHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@H@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXHHABV?$KdPtr@UITileLayer@vc1@@@3@ABU?$KdArray@F@3@H@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00408450
// ??$Call@VKdTerrainBuilder@vc1@@HHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@ABU42@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXHHABV?$KdPtr@UITileLayer@vc1@@@3@ABU?$KdArray@F@3@2@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004a6730
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a67d0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXHHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@1@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6870
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXHHABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@H@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6910
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABUVector2@2@ABV?$KdPtr@UITileLayer@vc1@@@2@ABU?$KdArray@F@2@_N@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a69b0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXHH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6ab0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXHH_N0@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6b50
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6c20
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEHH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6cc0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6dc0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6e60
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UITileLayer@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6f00
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a6fa0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a7040
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a70e0
// ?Dispatch@?$ScriptMethodCall@VKdTerrainBuilder@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00407630
// vc1::KdTerrainBuilder::`scalar deleting destructor'

// SYNTHETIC: VC1 0x004085c0
// ?AddRef@KdTerrainBuilder@vc1@@WHE@AEIXZ
// SYNTHETIC: VC1 0x004085d0
// ?Release@KdTerrainBuilder@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x004085e0
// ?QueryInterface@KdTerrainBuilder@vc1@@WCA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004085f0
// ??_EKdTerrainBuilder@vc1@@WCA@AEPAXI@Z
// SYNTHETIC: VC1 0x00408600
// ?Release@KdTerrainBuilder@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00408610
// ?QueryInterface@KdTerrainBuilder@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408620
// ?Release@KdTerrainBuilder@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00408630
// ?QueryInterface@KdTerrainBuilder@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408640
// ?Release@KdTerrainBuilder@vc1@@WCI@AEIXZ
// SYNTHETIC: VC1 0x00408650
// ?QueryInterface@KdTerrainBuilder@vc1@@WCI@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408660
// ?AddRef@KdTerrainBuilder@vc1@@WCA@AEIXZ
// SYNTHETIC: VC1 0x00408670
// ?Release@KdTerrainBuilder@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00408680
// ?QueryInterface@KdTerrainBuilder@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408690
// ?AddRef@KdTerrainBuilder@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004086a0
// ?Release@KdTerrainBuilder@vc1@@WEM@AEIXZ
// SYNTHETIC: VC1 0x004086b0
// ?QueryInterface@KdTerrainBuilder@vc1@@WEM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004086c0
// ??_EKdTerrainBuilder@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x004086d0
// ?AddRef@KdTerrainBuilder@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x004086e0
// ??_EKdTerrainBuilder@vc1@@WEM@AEPAXI@Z
// SYNTHETIC: VC1 0x004086f0
// ?AddRef@KdTerrainBuilder@vc1@@WCI@AEIXZ
// SYNTHETIC: VC1 0x00408700
// ?Release@KdTerrainBuilder@vc1@@WHA@AEIXZ
// SYNTHETIC: VC1 0x00408710
// ?QueryInterface@KdTerrainBuilder@vc1@@WHA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408720
// ?AddRef@KdTerrainBuilder@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00408730
// ?AddRef@KdTerrainBuilder@vc1@@WEM@AEIXZ
// SYNTHETIC: VC1 0x00408740
// ?Release@KdTerrainBuilder@vc1@@WHE@AEIXZ
// SYNTHETIC: VC1 0x00408750
// ?QueryInterface@KdTerrainBuilder@vc1@@WHE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00408760
// ?AddRef@KdTerrainBuilder@vc1@@WHA@AEIXZ

}

#endif
