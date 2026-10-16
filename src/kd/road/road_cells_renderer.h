#ifndef VC1_KD_ROAD_ROAD_CELLS_RENDERER_H
#define VC1_KD_ROAD_ROAD_CELLS_RENDERER_H

#include "kd/name_map.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/irenderable.h"
#include "kd/iface/iroad_renderer.h"
#include "kd/iface/iscript_object.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "RoadCellsRenderer" (RoutingRenderer.nut): a copy of
// a set of RoadRenderer cells (script "SetCells" with grid points) that the
// RoadRenderer draws as route marks on this object's render layer. QI
// 0x004abeb0 establishes the face offsets below.
// SIZE 0x44
// VTABLE: VC1 0x004d15f0 vc1::IScriptObject
// VTABLE: VC1 0x004d1608 vc1::IRenderable
// VTABLE: VC1 0x004d1620 vc1::IIdentifiable
// VTABLE: VC1 0x004d1638 vc1::IClassifiable
// VTABLE: VC1 0x004d1654 vc1::IAbstract
class KdRoadCellsRenderer : public IScriptObject,
                            public IRenderable,
                            public IIdentifiable,
                            public IClassifiable,
                            public IAbstract
{
public:
    int renderLayer;                   // 0x2c not set by the constructor
    KdObjArray<RoadCell> cells;        // 0x30
    KdRef<IRoadRenderer> roadRenderer; // 0x40

    KdRoadCellsRenderer() throw();
    ~KdRoadCellsRenderer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    int RenderMasked(void* graphics, void* mask);
    int Render(void* graphics);
    int GetRenderOrder();

    void SetRenderLayer(int layer);
    void SetRoadRenderer(const KdPtr<IRoadRenderer>& renderer);
    void ClearCells();
    void SetCells(const KdObjArray<Vector2>& points);
};

typedef char Vc1KdRoadCellsRendererSizeCheck[sizeof(KdRoadCellsRenderer) == 0x44 ? 1 : -1];

void** CreateKdRoadCellsRenderer(void** out);

// TEMPLATE: VC1 0x00406130
// ??$CreateScriptMethod@VKdRoadCellsRenderer@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadCellsRenderer@0@AEXH@Z@Z
// TEMPLATE: VC1 0x004061a0
// ??$CreateScriptMethod@VKdRoadCellsRenderer@vc1@@P812@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadCellsRenderer@0@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00406210
// ??$CreateScriptMethod@VKdRoadCellsRenderer@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoadCellsRenderer@0@AEXABU?$KdObjArray@UVector2@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00406280
// ??$CreateScriptMethod@VKdRoadCellsRenderer@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdRoadCellsRenderer@0@AEXXZ@Z
// TEMPLATE: VC1 0x004062f0
// ?AssignRange@?$KdObjArray@URoadCell@vc1@@@vc1@@QAEXPBURoadCell@2@0@Z
// TEMPLATE: VC1 0x00406360
// ??$Call@VKdRoadCellsRenderer@vc1@@ABU?$KdObjArray@UVector2@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdRoadCellsRenderer@vc1@@P823@AEXABU?$KdObjArray@UVector2@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004ac230
// ?Dispatch@?$ScriptMethodCall@VKdRoadCellsRenderer@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ac2d0
// ?Dispatch@?$ScriptMethodCall@VKdRoadCellsRenderer@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ac370
// ?Dispatch@?$ScriptMethodCall@VKdRoadCellsRenderer@vc1@@P812@AEXABV?$KdPtr@UIRoadRenderer@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004ac410
// ?Dispatch@?$ScriptMethodCall@VKdRoadCellsRenderer@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00406110
// vc1::KdRoadCellsRenderer::`scalar deleting destructor'
// SYNTHETIC: VC1 0x00406580
// ?AddRef@KdRoadCellsRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00406590
// ?AddRef@KdRoadCellsRenderer@vc1@@WCE@AEIXZ
// SYNTHETIC: VC1 0x004065a0
// ?AddRef@KdRoadCellsRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x004065b0
// ?QueryInterface@KdRoadCellsRenderer@vc1@@WBM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004065c0
// ?Release@KdRoadCellsRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x004065d0
// ??_EKdRoadCellsRenderer@vc1@@WBM@AEPAXI@Z
// SYNTHETIC: VC1 0x004065e0
// ?QueryInterface@KdRoadCellsRenderer@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004065f0
// ?QueryInterface@KdRoadCellsRenderer@vc1@@WCE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406600
// ?QueryInterface@KdRoadCellsRenderer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00406610
// ?Release@KdRoadCellsRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00406620
// ?Release@KdRoadCellsRenderer@vc1@@WCE@AEIXZ
// SYNTHETIC: VC1 0x00406630
// ?AddRef@KdRoadCellsRenderer@vc1@@WBM@AEIXZ
// SYNTHETIC: VC1 0x00406640
// ??_EKdRoadCellsRenderer@vc1@@WCE@AEPAXI@Z
// SYNTHETIC: VC1 0x00406650
// ?Release@KdRoadCellsRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00406660
// ??_EKdRoadCellsRenderer@vc1@@W7AEPAXI@Z

}

#endif
