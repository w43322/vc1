#ifndef VC1_KD_TRANSPORT_MOVER_H
#define VC1_KD_TRANSPORT_MOVER_H

#include "kd/iface/iabstract.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iroute.h"
#include "kd/iface/itsp_solver.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "kd/iface/iupdateable.h"
#include "kd/iface/irenderable.h"
#include "kd/name_map.h"
#include "bindings/matrix3.h"
#include "bindings/rect.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred name. A grid cell as two shorts; != is out of line (0x00407600).
// SIZE 0x04
struct KdGridCell
{
    unsigned short x; // 0x00
    unsigned short y; // 0x02

    __declspec(noinline) bool operator!=(const KdGridCell& other) const;
};

// Inferred names. One transport moved along its route.
// SIZE 0x1c
struct KdTransportRecord
{
    KdRef<IPositionable> transport; // 0x00
    KdRef<IRoute> route;            // 0x04 world space, built by InitMoving
    float distance;                 // 0x08 travelled along the route
    KdGridCell cell;                // 0x0c current grid cell; not set by AddTransport
    bool atRoadside;                // 0x10 parked beside the road: does not block
    bool moving;                    // 0x11 script "SetMoving"
    KdRef<IAbstract> blocker;       // 0x14 the transport this one waits behind
    bool paused;                    // 0x18 script "PauseMoving"

    KdTransportRecord() {}
    KdTransportRecord& operator=(const KdTransportRecord& source);
};

typedef char Vc1KdTransportRecordSizeCheck[sizeof(KdTransportRecord) == 0x1c ? 1 : -1];

// Out-of-line copy of the inline destructor below (KdTransportMover teardown).
// FUNCTION: VC1 0x0040b420
// ??1KdTransportRecords@vc1@@QAE@XZ

// Inferred name. The transport records in a KdFactory buffer; every slot up
// to capacity is constructed (Reserve) and destroyed (destructor).
// SIZE 0x10
struct KdTransportRecords
{
    KdFactory items; // 0x00

    ~KdTransportRecords() throw()
    {
        if (items.capacity) {
            DestroyRange(Begin(), Begin() + items.capacity / sizeof(KdTransportRecord));
        }
    }
    KdTransportRecord* Begin() const { return reinterpret_cast<KdTransportRecord*>(items.begin); }
    KdTransportRecord* End() const
    {
        return reinterpret_cast<KdTransportRecord*>(
            reinterpret_cast<unsigned char*>(items.begin) + items.size);
    }
    unsigned Count() const { return items.size / sizeof(KdTransportRecord); }
    KdTransportRecord& operator[](unsigned i) const { return Begin()[i]; }
    void Reserve(unsigned count);
    static void DestroyRange(KdTransportRecord* first, KdTransportRecord* last);
};

// Inferred name. Factory "TransportMover" (Level.nut CreateTransportMover):
// moves the city's transports along IRoute paths over the road cells of the
// IGridMap, which an IGridRouter searches. Scripts add transports, plan
// their routes ("FindPathToArea", "CreateOptimalRouteFromArea", ...) and
// start, pause and stop them; "OnMovingFinish" reports an arrival. QI
// 0x0049bd50 establishes the face offsets below.
// SIZE 0xb8
// VTABLE: VC1 0x004d2340 vc1::IScriptObject
// VTABLE: VC1 0x004d2358 vc1::IShutdownable
// VTABLE: VC1 0x004d2368 vc1::IUpdateable
// VTABLE: VC1 0x004d2380 vc1::IVisible
// VTABLE: VC1 0x004d2394 vc1::IRenderable
// VTABLE: VC1 0x004d23ac vc1::IAbstract
class KdTransportMover : public IScriptObject,
                         public IShutdownable,
                         public IUpdateable,
                         public IVisible,
                         public IRenderable,
                         public IAbstract
{
public:
    float cornerRadius;                 // 0x1c 0.2, the arcs inserted at route corners
    float cornerOuter;                  // 0x20 0.4, used as cornerOuter - cornerRadius
    int passableMask;                   // 0x24 road cell values
    float lookAhead;                    // 0x28 50, world units checked ahead for a blocker
    float vehicleWidth;                 // 0x2c 24, the blocker check's lane width
    bool visible;                       // 0x30
    KdRef<IGridMap> gridMap;            // 0x34
    Matrix3 worldToGrid;                // 0x38
    Matrix3 gridToWorld;                // 0x5c
    KdRef<IGridRouter> router;          // 0x80
    KdTransportRecords records;         // 0x84
    float speed;                        // 0x94 0.2
    KdArray<unsigned char> reachable;   // 0x98 GetOptimalRouteIndexesReachable
    KdArray<unsigned char> unreachable; // 0xa8 GetOptimalRouteIndexesUnreachable

    KdTransportMover() throw();
    ~KdTransportMover() throw();

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    int Shutdown();
    int GetUpdateLayer();
    int SetLayer(unsigned layer);
    void Update(unsigned sliceBit, int dt);
    void SetVisible(bool value);
    bool IsVisible();
    int Render(void* graphics);
    int GetRenderOrder();

    KdArray<unsigned char>& GetOptimalRouteIndexesUnreachable();
    KdArray<unsigned char>& GetOptimalRouteIndexesReachable();
    void SetSpeed(float value);
    void SetGridRouter(const KdPtr<IGridRouter>& value);
    void SetGridMap(const KdPtr<IGridMap>& value);
    bool WriteTransportToStream(const KdPtr<IOutputStream>& stream,
        const KdPtr<IPositionable>& transport);
    float GetRouteLength(const KdPtr<IPositionable>& transport);
    void PauseMoving(const KdPtr<IPositionable>& transport, bool value);
    void StopMoving(const KdPtr<IPositionable>& transport);
    void SetMoving(const KdPtr<IPositionable>& transport, bool value);
    void InitPosition(const KdPtr<IPositionable>& transport);
    bool MoveToRoadsite(const KdPtr<IPositionable>& transport);
    bool CanStartMoving(const KdPtr<IPositionable>& transport);
    void RemoveTransport(const KdPtr<IPositionable>& transport);
    void AddTransport(const KdPtr<IPositionable>& transport);
    bool ReadTransportFromStream(const KdPtr<IInputStream>& stream,
        const KdPtr<IPositionable>& transport);
    KdPtr<IUnknown> FindPathToArea(const Vector2& from, const Vector2& to, const Rect& area);
    KdPtr<IUnknown> FindPathBetweenAreas(const Vector2& from, const Rect& fromArea,
        const Vector2& to, const Rect& toArea);
    bool InitMoving(const KdPtr<IPositionable>& transport, const KdPtr<IRoute>& route);
    void CreateOptimalRouteFromPosition(const KdObjArray<Vector2>& targets,
        const KdObjArray<Rect>& areas, int first, const Vector2& from);
    void CreateOptimalRouteFromArea(const KdObjArray<Vector2>& targets,
        const KdObjArray<Rect>& areas, int first, const Vector2& from, const Rect& fromArea);

    static int __stdcall FindTransport(KdTransportMover* self,
        const KdPtr<IPositionable>& transport);
    bool CanParkBeside(const KdGridCell& cell, const Vector2* axes);
    bool IsBlocked(KdTransportRecord& record);
    void NotifyCellChange(KdTransportRecord& record);
    void UpdateRecord(KdTransportRecord& record, int dt);
    KdPtr<IRoute> BuildDrivingRoute(KdPtr<IRoute> route, bool fromRoadside);
};

typedef char Vc1KdTransportMoverSizeCheck[sizeof(KdTransportMover) == 0xb8 ? 1 : -1];

void** CreateKdTransportMover(void** out);

// Script bindings. SetSpeed's and the index getters' Calls are inlined into
// their Dispatch; SetGridMap's and SetGridRouter's fold with other classes'
// (0x004048c0, 0x00407e00).
// TEMPLATE: VC1 0x0040b480
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABV?$KdPtr@UIGridMap@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b4f0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABV?$KdPtr@UIGridRouter@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b560
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXM@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXM@Z@Z
// TEMPLATE: VC1 0x0040b5d0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b640
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIPositionable@vc1@@@2@ABV?$KdPtr@UIRoute@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE_NABV?$KdPtr@UIPositionable@vc1@@@0@ABV?$KdPtr@UIRoute@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b6b0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEMABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEMABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b720
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE_NABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b790
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@_N@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABV?$KdPtr@UIPositionable@vc1@@@0@_N@Z@Z
// TEMPLATE: VC1 0x0040b800
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@ABURect@2@01@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABUVector2@0@ABURect@0@01@Z@Z
// TEMPLATE: VC1 0x0040b870
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@0ABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABUVector2@0@0ABURect@0@@Z@Z
// TEMPLATE: VC1 0x0040b8e0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIOutputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE_NABV?$KdPtr@UIOutputStream@vc1@@@0@ABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b950
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIInputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AE_NABV?$KdPtr@UIInputStream@vc1@@@0@ABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x0040b9c0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@ABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABU?$KdObjArray@UVector2@vc1@@@0@ABU?$KdObjArray@URect@vc1@@@0@HABUVector2@0@ABURect@0@@Z@Z
// TEMPLATE: VC1 0x0040ba30
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEXABU?$KdObjArray@UVector2@vc1@@@0@ABU?$KdObjArray@URect@vc1@@@0@HABUVector2@0@@Z@Z
// TEMPLATE: VC1 0x0040baa0
// ??$CreateScriptMethod@VKdTransportMover@vc1@@P812@AEAAU?$KdArray@E@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdTransportMover@0@AEAAU?$KdArray@E@0@XZ@Z
// TEMPLATE: VC1 0x0040bc30
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AEXABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040bcd0
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@ABV?$KdPtr@UIRoute@vc1@@@2@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE_NABV?$KdPtr@UIPositionable@vc1@@@3@ABV?$KdPtr@UIRoute@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040be50
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@M@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AEMABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040bf10
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE_NABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c000
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@_N@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AEXABV?$KdPtr@UIPositionable@vc1@@@3@_N@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c0c0
// ??$Call@VKdTransportMover@vc1@@ABUVector2@2@ABURect@2@ABU32@ABU42@@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE?AV?$KdPtr@UIUnknown@vc1@@@3@ABUVector2@3@ABURect@3@12@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c160
// ??$Call@VKdTransportMover@vc1@@ABUVector2@2@ABU32@ABURect@2@@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE?AV?$KdPtr@UIUnknown@vc1@@@3@ABUVector2@3@1ABURect@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c1f0
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIOutputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE_NABV?$KdPtr@UIOutputStream@vc1@@@3@ABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c370
// ??$Call@VKdTransportMover@vc1@@ABV?$KdPtr@UIInputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AE_NABV?$KdPtr@UIInputStream@vc1@@@3@ABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c4f0
// ??$Call@VKdTransportMover@vc1@@ABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@ABURect@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AEXABU?$KdObjArray@UVector2@vc1@@@3@ABU?$KdObjArray@URect@vc1@@@3@HABUVector2@3@ABURect@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0040c600
// ??$Call@VKdTransportMover@vc1@@ABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTransportMover@vc1@@P823@AEXABU?$KdObjArray@UVector2@vc1@@@3@ABU?$KdObjArray@URect@vc1@@@3@HABUVector2@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0049d3e0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEAAU?$KdArray@E@2@XZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d480
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIInputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d520
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIOutputStream@vc1@@@2@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d5c0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@_N@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d660
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d700
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEMABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d7a0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE_NABV?$KdPtr@UIPositionable@vc1@@@2@ABV?$KdPtr@UIRoute@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d840
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d8e0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXM@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049d9c0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049da60
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABV?$KdPtr@UIGridMap@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049db00
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049dba0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AEXABU?$KdObjArray@UVector2@vc1@@@2@ABU?$KdObjArray@URect@vc1@@@2@HABUVector2@2@ABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049dc40
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@0ABURect@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049dce0
// ?Dispatch@?$ScriptMethodCall@VKdTransportMover@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@ABURect@2@01@Z@vc1@@SAHPAUSQVM@@@Z

// SYNTHETIC: VC1 0x0040b300
// vc1::KdTransportMover::`scalar deleting destructor'
// SYNTHETIC: VC1 0x0040c9d0
// ?Release@KdTransportMover@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040c9e0
// ?QueryInterface@KdTransportMover@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040c9f0
// ?AddRef@KdTransportMover@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040ca00
// ?Release@KdTransportMover@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040ca10
// ?QueryInterface@KdTransportMover@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040ca20
// ?AddRef@KdTransportMover@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040ca30
// ??_EKdTransportMover@vc1@@WBE@AEPAXI@Z
// SYNTHETIC: VC1 0x0040ca40
// ?AddRef@KdTransportMover@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x0040ca50
// ?AddRef@KdTransportMover@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x0040ca60
// ?Release@KdTransportMover@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040ca70
// ?QueryInterface@KdTransportMover@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040ca80
// ?Release@KdTransportMover@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040ca90
// ?QueryInterface@KdTransportMover@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040caa0
// ?Release@KdTransportMover@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x0040cab0
// ?QueryInterface@KdTransportMover@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040cac0
// ?AddRef@KdTransportMover@vc1@@W3AEIXZ

}

#endif
