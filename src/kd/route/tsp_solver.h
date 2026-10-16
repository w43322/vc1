#ifndef VC1_KD_ROUTE_TSP_SOLVER_H
#define VC1_KD_ROUTE_TSP_SOLVER_H

#include "kd/iface/iabstract.h"
#include "kd/iface/iscript_object.h"
#include "kd/iface/itsp_solver.h"

class SquirrelObject;

namespace vc1 {

// Inferred name. Factory "TSPSolver", a singleton: the farthest insertion
// tour of the transport mover's optimal routes. QI 0x0049b410 establishes
// the face offsets.
// SIZE 0x20
// VTABLE: VC1 0x004d23d0 vc1::ITSPSolver
// VTABLE: VC1 0x004d23e4 vc1::IScriptObject
// VTABLE: VC1 0x004d23fc vc1::IAbstract
class KdTSPSolver : public ITSPSolver,
                    public IScriptObject,
                    public IAbstract
{
public:
    KdArray<int> order; // 0x10 the last tour, reserved for 256 cities

    KdTSPSolver() throw();
    ~KdTSPSolver() throw() {}

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    void Solve(const KdObjArray<KdArray<int> >& distances, int start);
    int GetResultID(int index);

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
};

typedef char Vc1KdTSPSolverSizeCheck[sizeof(KdTSPSolver) == 0x20 ? 1 : -1];

void** CreateKdTspSolver(void** out);

// Script bindings; GetResultID's Call folds with KdTerrainBuilder's
// (0x004080e0).
// SYNTHETIC: VC1 0x0040ce00
// vc1::KdTSPSolver::`scalar deleting destructor'
// TEMPLATE: VC1 0x0040ce60
// ??$CreateScriptMethod@VKdTSPSolver@vc1@@P812@AEXABU?$KdObjArray@U?$KdArray@H@vc1@@@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdTSPSolver@0@AEXABU?$KdObjArray@U?$KdArray@H@vc1@@@0@H@Z@Z
// TEMPLATE: VC1 0x0040ced0
// ??$CreateScriptMethod@VKdTSPSolver@vc1@@P812@AEHH@Z@vc1@@YA?AVSquirrelObject@@P8KdTSPSolver@0@AEHH@Z@Z
// TEMPLATE: VC1 0x0040cf40
// ??$Call@VKdTSPSolver@vc1@@ABU?$KdObjArray@U?$KdArray@H@vc1@@@2@H@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTSPSolver@vc1@@P823@AEXABU?$KdObjArray@U?$KdArray@H@vc1@@@3@H@ZPAUSQVM@@H@Z
// SYNTHETIC: VC1 0x0040cfe0
// ?QueryInterface@KdTSPSolver@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040cff0
// ?QueryInterface@KdTSPSolver@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x0040d000
// ?Release@KdTSPSolver@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040d010
// ??_EKdTSPSolver@vc1@@W7AEPAXI@Z
// SYNTHETIC: VC1 0x0040d020
// ?Release@KdTSPSolver@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x0040d030
// ?AddRef@KdTSPSolver@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x0040d040
// ?AddRef@KdTSPSolver@vc1@@W7AEIXZ
// TEMPLATE: VC1 0x0049b7e0
// ?Dispatch@?$ScriptMethodCall@VKdTSPSolver@vc1@@P812@AEHH@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x0049b880
// ?Dispatch@?$ScriptMethodCall@VKdTSPSolver@vc1@@P812@AEXABU?$KdObjArray@U?$KdArray@H@vc1@@@2@H@Z@vc1@@SAHPAUSQVM@@@Z

}

#endif
