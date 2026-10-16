#ifndef VC1_KD_ROAD_RENDERER_H
#define VC1_KD_ROAD_RENDERER_H

#include "kd/factory.h"
#include "kd/name_map.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/iroad_renderer.h"
#include "kd/iface/iscript_object.h"
#include "bindings/matrix3.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "RoadRenderer" (Level.nut CreateRoadRenderer): the
// road network of a level map, one malloc'd RoadCell per grid cell of the
// IGridMap it is bound to, drawn through the "TileManager" component from
// the 16-entry road / broken road tile tables indexed by the cell links.
// The preview cells show a road being placed. QI 0x004aa3a0 establishes
// the face offsets below.
// SIZE 0xc8
// VTABLE: VC1 0x004d16f0 vc1::IRoadRenderer
// VTABLE: VC1 0x004d1730 vc1::IScriptObject
// VTABLE: VC1 0x004d1748 vc1::IWriteable
// VTABLE: VC1 0x004d1758 vc1::IReadable
// VTABLE: VC1 0x004d1768 vc1::IIdentifiable
// VTABLE: VC1 0x004d1780 vc1::IClassifiable
// VTABLE: VC1 0x004d179c vc1::IAbstract
class KdRoadRenderer : public IRoadRenderer,
                       public IScriptObject,
                       public IWriteable,
                       public IReadable,
                       public IIdentifiable,
                       public IClassifiable,
                       public IAbstract
{
public:
    int renderLayer;              // 0x34
    short roadTiles[16];          // 0x38 indexed by RoadCell::links
    short brokenTiles[16];        // 0x58
    short routeMarkTile;          // 0x78
    int width;                    // 0x7c
    int height;                   // 0x80
    Matrix3 transform;            // 0x84 grid to world, from the grid map
    KdArray<RoadCell*> cells;     // 0xa8 width*height, row-major
    KdObjArray<RoadCell> preview; // 0xb8

    KdRoadRenderer() throw();
    ~KdRoadRenderer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    int RenderMasked(void* graphics, void* mask);
    int Render(void* graphics);
    int GetRenderOrder();

    void SetGridMap(const KdPtr<IGridMap>& map);
    void AddCell(int x, int y, bool visible, bool strict);
    void BreakCell(int x, int y);
    void RepairCell(int x, int y);
    void RemoveCell(int x, int y);
    void SetPreview(const KdArray<CellCoord>& coords, bool valid);
    void HighlightRoadCells(const KdObjArray<Vector2>& points, bool value);
    void RenderCells(const KdPtr<IGraphics>& graphics, const KdObjArray<RoadCell>& list);
    void RenderRouteMarks(const KdPtr<IGraphics>& graphics, const KdObjArray<RoadCell>& list);
    KdObjArray<RoadCell> GetCells(const KdArray<CellCoord>& coords);

    bool WriteTo(const KdPtr<IOutputStream>& stream);
    bool ReadFrom(const KdPtr<IInputStream>& stream);

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    void SetRouteMarkTile(short tile);
    void SetRenderLayer(int layer);
    void SetRoadBrokenTiles(const KdArray<short>& tiles);
    void SetRoadTiles(const KdArray<short>& tiles);

    RoadCell* GetCell(int x, int y);
    void ResizeCells(unsigned count);
    void LinkCell(RoadCell* cell, int x, int y, bool strict);
};

typedef char Vc1KdRoadRendererSizeCheck[sizeof(KdRoadRenderer) == 0xc8 ? 1 : -1];

void** CreateKdRoadRenderer(void** out);

// TEMPLATE: VC1 0x004063c0
// ?Reserve@?$KdObjArray@URoadCell@vc1@@@vc1@@QAEXIE@Z

// TEMPLATE: VC1 0x004066a0
// ??$CreateScriptMethod@VKdRoadRenderer@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadRenderer@0@AEXH@Z@Z
// TEMPLATE: VC1 0x00406710
// ??$CreateScriptMethod@VKdRoadRenderer@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadRenderer@0@AEXABU?$KdArray@F@0@@Z@Z
// TEMPLATE: VC1 0x00406780
// ??$CreateScriptMethod@VKdRoadRenderer@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@_N@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadRenderer@0@AEXABU?$KdObjArray@UVector2@vc1@@@0@_N@Z@Z
// TEMPLATE: VC1 0x004067f0
// ??$CreateScriptMethod@VKdRoadRenderer@vc1@@P812@AEXF@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadRenderer@0@AEXF@Z@Z
// TEMPLATE: VC1 0x00406880
// ??$Call@VKdRoadRenderer@vc1@@ABU?$KdObjArray@UVector2@vc1@@@2@_N@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdRoadRenderer@vc1@@P823@AEXABU?$KdObjArray@UVector2@vc1@@@3@_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004ab350
// ?Dispatch@?$ScriptMethodCall@VKdRoadRenderer@vc1@@P812@AEXF@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ab3f0
// ?Dispatch@?$ScriptMethodCall@VKdRoadRenderer@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ab490
// ?Dispatch@?$ScriptMethodCall@VKdRoadRenderer@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ab5a0
// ?Dispatch@?$ScriptMethodCall@VKdRoadRenderer@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@_N@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00406680
// vc1::KdRoadRenderer::`scalar deleting destructor'

// SYNTHETIC: VC1 0x00406900
// ?AddRef@KdRoadRenderer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00406910
// ?QueryInterface@KdRoadRenderer@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406920
// ?QueryInterface@KdRoadRenderer@vc1@@WCE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406930
// ??_EKdRoadRenderer@vc1@@WCE@AEPAXI@Z
// SYNTHETIC: VC1 0x00406940
// ?QueryInterface@KdRoadRenderer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406950
// ?Release@KdRoadRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00406960
// ?Release@KdRoadRenderer@vc1@@WCE@AEIXZ
// SYNTHETIC: VC1 0x00406970
// ?QueryInterface@KdRoadRenderer@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406980
// ?Release@KdRoadRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00406990
// ?QueryInterface@KdRoadRenderer@vc1@@WCM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004069a0
// ??_EKdRoadRenderer@vc1@@WCM@AEPAXI@Z
// SYNTHETIC: VC1 0x004069b0
// ?QueryInterface@KdRoadRenderer@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004069c0
// ?Release@KdRoadRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004069d0
// ?AddRef@KdRoadRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x004069e0
// ?Release@KdRoadRenderer@vc1@@WCM@AEIXZ
// SYNTHETIC: VC1 0x004069f0
// ?AddRef@KdRoadRenderer@vc1@@WCE@AEIXZ
// SYNTHETIC: VC1 0x00406a00
// ??_EKdRoadRenderer@vc1@@WBA@AEPAXI@Z
// SYNTHETIC: VC1 0x00406a10
// ?Release@KdRoadRenderer@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00406a20
// ?AddRef@KdRoadRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00406a30
// ?AddRef@KdRoadRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00406a40
// ?AddRef@KdRoadRenderer@vc1@@WCM@AEIXZ

}

#endif
