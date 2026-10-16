#ifndef VC1_KD_IFACE_ITSP_SOLVER_H
#define VC1_KD_IFACE_ITSP_SOLVER_H

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iunknown.h"
#include "kd/name_map.h"

namespace vc1 {

// Inferred names. "ITSPSolver" (0x004d0ae8): a travelling salesman tour
// over a square distance matrix (one KdArray<int> row per city).
// SIZE 0x04
struct ITSPSolver : IUnknown
{
    virtual void Solve(const KdObjArray<KdArray<int> >& distances, int start) = 0; // vtable+0x0c "Solve"
    virtual int GetResultID(int index) = 0; // vtable+0x10 "GetResultID", the index-th city of the tour
};

template <>
struct KdIid<ITSPSolver>
{
    static const char** Get() { return &g_iidTSPSolver; }
};

}

#endif
