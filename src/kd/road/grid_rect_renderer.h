#ifndef VC1_KD_GRID_RECT_RENDERER_H
#define VC1_KD_GRID_RECT_RENDERER_H

#include "kd/name_map.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "bindings/rect.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "GridRectRenderer" (Level.nut
// CreateActivityRadiusRenderer / CreateGridMarginRenderer): outlines cell
// rectangles of an IGridMap with a 16-entry tile table indexed by which
// sides of a cell are inner (left 1, right 2, top 4, bottom 8). QI
// 0x004afd80 establishes the face offsets below.
// SIZE 0x48
// VTABLE: VC1 0x004d0fec vc1::IScriptObject
// VTABLE: VC1 0x004d1004 vc1::IShutdownable
// VTABLE: VC1 0x004d1014 vc1::IGridRenderer
// VTABLE: VC1 0x004d1024 vc1::IAbstract
class KdGridRectRenderer : public IScriptObject,
                           public IShutdownable,
                           public IGridRenderer,
                           public IAbstract
{
public:
    KdObjArray<Rect> rects; // 0x14 in grid cells
    short cellTiles[16];    // 0x24 indexed by the inner-side flags
    KdRef<IGridMap> gridMap; // 0x44

    KdGridRectRenderer() throw();
    ~KdGridRectRenderer() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    int Shutdown();
    void RenderGrid(void* graphics);

    void SetGridMap(const KdPtr<IGridMap>& map);
    void SetCellTiles(const KdArray<short>& tiles);
    void ClearRects();
    void SetRect(int index, const Rect& rect);
    void AddRect(const Rect& rect);
};

typedef char Vc1KdGridRectRendererSizeCheck[sizeof(KdGridRectRenderer) == 0x48 ? 1 : -1];

void** CreateKdGridRectRenderer(void** out);

// SYNTHETIC: VC1 0x00404590
// vc1::KdGridRectRenderer::`scalar deleting destructor'

// TEMPLATE: VC1 0x004045b0
// ??$CreateScriptMethod@VKdGridRectRenderer@vc1@@P812@AEXHABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRectRenderer@0@AEXHABURect@0@@Z@Z
// TEMPLATE: VC1 0x00404620
// ??$CreateScriptMethod@VKdGridRectRenderer@vc1@@P812@AEXABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRectRenderer@0@AEXABURect@0@@Z@Z
// TEMPLATE: VC1 0x00404690
// ??$CreateScriptMethod@VKdGridRectRenderer@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdGridRectRenderer@0@AEXXZ@Z
// TEMPLATE: VC1 0x00404700
// ??$CreateScriptMethod@VKdGridRectRenderer@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRectRenderer@0@AEXABU?$KdArray@F@0@@Z@Z
// TEMPLATE: VC1 0x00404770
// ??$CreateScriptMethod@VKdGridRectRenderer@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRectRenderer@0@AEXABV?$KdPtr@UIGridMap@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00404830
// ??$Call@VKdGridRectRenderer@vc1@@HABURect@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGridRectRenderer@vc1@@P823@AEXHABURect@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004b00c0
// ?Dispatch@?$ScriptMethodCall@VKdGridRectRenderer@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b0160
// ?Dispatch@?$ScriptMethodCall@VKdGridRectRenderer@vc1@@P812@AEXABU?$KdArray@F@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b0200
// ?Dispatch@?$ScriptMethodCall@VKdGridRectRenderer@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b02a0
// ?Dispatch@?$ScriptMethodCall@VKdGridRectRenderer@vc1@@P812@AEXABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004b0350
// ?Dispatch@?$ScriptMethodCall@VKdGridRectRenderer@vc1@@P812@AEXHABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x00404b50
// ?AddRef@KdGridRectRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404b60
// ?Release@KdGridRectRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00404b70
// ?QueryInterface@KdGridRectRenderer@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404b80
// ?Release@KdGridRectRenderer@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00404b90
// ?QueryInterface@KdGridRectRenderer@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404ba0
// ?Release@KdGridRectRenderer@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404bb0
// ?QueryInterface@KdGridRectRenderer@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404bc0
// ?AddRef@KdGridRectRenderer@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00404bd0
// ??_EKdGridRectRenderer@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x00404be0
// ?AddRef@KdGridRectRenderer@vc1@@W7AEIXZ

}

#endif
