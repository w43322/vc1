#ifndef VC1_KD_ROAD_GRID_ROUTER_H
#define VC1_KD_ROAD_GRID_ROUTER_H

#include "kd/factory.h"
#include "kd/name_map.h"
#include "kd/iface/iabstract.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/ishutdownable.h"
#include "bindings/matrix3.h"
#include "bindings/vector2.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "GridRouter" (Level.nut: Router.Initialize(PassMap)).
// Distances from the start cell spread breadth-first (4-neighbourhood, at
// most 200 steps and 200 frontier cells per step) over the passable cells;
// FindPath then walks back from the target to the start. QI 0x004aef00
// establishes the face offsets below.
// SIZE 0xb8
// VTABLE: VC1 0x004d1068 vc1::IGridRouter
// VTABLE: VC1 0x004d1084 vc1::IScriptObject
// VTABLE: VC1 0x004d109c vc1::IShutdownable
// VTABLE: VC1 0x004d10ac vc1::IAbstract
class KdGridRouter : public IGridRouter,
                     public IScriptObject,
                     public IShutdownable,
                     public IAbstract
{
public:
    short width;                // 0x14 of the map, cached by Initialize
    short height;               // 0x16
    KdRef<IGridMap> map;        // 0x18
    Matrix3 transform;          // 0x1c map position: cell to world
    Matrix3 inverse;            // 0x40 world to cell
    KdArray<int> distance;      // 0x64 per cell, -1: not reached
    KdArray<int> frontier;      // 0x74 cell indices of the current step
    KdArray<int> next;          // 0x84 cell indices of the next step
    KdObjArray<Vector2> path;   // 0x94 start to target
    int startX;                 // 0xa4
    int startY;                 // 0xa8
    int targetX;                // 0xac walked back to the start by FindPath
    int targetY;                // 0xb0
    int passableMask;           // 0xb4 bit per cell value

    KdGridRouter() throw();
    ~KdGridRouter() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    void SetPassableMask(int mask);
    int FindPath(const Vector2& from, const Vector2& to, bool worldSpace);
    Vector2& GetPoint(int index);
    int GetPointCount();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    int Shutdown();

    void Initialize(const KdPtr<IAbstract>& source);
    int Visit(int distanceValue, int x, int y);
};

typedef char Vc1KdGridRouterSizeCheck[sizeof(KdGridRouter) == 0xb8 ? 1 : -1];

void** CreateKdGridRouter(void** out);

// TEMPLATE: VC1 0x00404c40
// ?Append@?$KdArray@H@vc1@@QAEXABH@Z

// TEMPLATE: VC1 0x00404dd0
// ?Reserve@?$KdObjArray@UVector2@vc1@@@vc1@@QAEXI@Z

// SYNTHETIC: VC1 0x00404c20
// vc1::KdGridRouter::`scalar deleting destructor'

// SYNTHETIC: VC1 0x00404f50
// ?Release@KdGridRouter@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00404f60
// ?QueryInterface@KdGridRouter@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404f70
// ?Release@KdGridRouter@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404f80
// ?QueryInterface@KdGridRouter@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00404f90
// ?AddRef@KdGridRouter@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00404fa0
// ??_EKdGridRouter@vc1@@WM@AEPAXI@Z
// SYNTHETIC: VC1 0x00404fb0
// ?AddRef@KdGridRouter@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00404fc0
// ?AddRef@KdGridRouter@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00404fd0
// ?Release@KdGridRouter@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00404fe0
// ?QueryInterface@KdGridRouter@vc1@@W3AEPAXPAPBD@Z

}

#endif
