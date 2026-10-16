#include "kd/route/tsp_solver.h"

#include "kd/iface/iid.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfcd0
static const char kTspSolver[] = "TSPSolver";
// GLOBAL: VC1 0x004d0b2c
const char* g_kdNameTspSolver = kTspSolver;

// STRING: VC1 0x004d23bc
static const char kScriptSolve[] = "Solve";
// STRING: VC1 0x004d23c4
static const char kScriptGetResultID[] = "GetResultID";

// Inferred names. Farthest insertion scratch: the tour as successor links
// (-1 while a city is not on it) and each city's distance to the tour.
// GLOBAL: VC1 0x004ea850
int g_tspDistanceToTour[256];
// GLOBAL: VC1 0x004eac50
int g_tspNext[256];

// GLOBAL: VC1 0x004ed6ac
KdRef<IUnknown> g_kdTspSolver;

// IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0040cdf0
const char* KdTSPSolver::GetTypeName()
{
    return kTspSolver;
}

// RET 8. IScriptObject vtable+0x10: nothing is settable.
// FUNCTION: VC1 0x0049b400
unsigned char KdTSPSolver::SetMember(void*, void*)
{
    return 0;
}

// FUNCTION: VC1 0x0049b450
unsigned KdTSPSolver::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x0049b470
unsigned KdTSPSolver::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// Inferred name. RET 0x14. Farthest insertion from start: the city farthest
// from the tour is inserted where it lengthens the tour least, until all
// count cities are on it; order gets the tour from start, cost its length.
// FUNCTION: VC1 0x0049b480
static __declspec(noinline) void __stdcall FarthestInsertion(int count, int start,
    const KdObjArray<KdArray<int> >& distances, KdArray<int>& order, int* cost)
{
    int i;
    int step;
    int farthest = count;
    int insertAt;
    int insertBefore;

    for (i = 0; i < count; ++i) {
        g_tspNext[i] = -1;
    }
    g_tspNext[start] = start;
    for (i = 0; i < count; ++i) {
        g_tspDistanceToTour[i] = distances[start][i];
    }
    *cost = 0;
    int total = 0;
    for (step = 0; step < count - 1; ++step) {
        int best = -0x3fffffff;
        for (i = 0; i < count; ++i) {
            if (g_tspNext[i] == -1 && g_tspDistanceToTour[i] > best) {
                farthest = i;
                best = g_tspDistanceToTour[i];
            }
        }
        best = 0x3fffffff;
        int city = start;
        for (i = 0; i <= step; ++i) {
            const int next = g_tspNext[city];
            const KdArray<int>& row = distances[city];
            const int delta = row[farthest] - row[next] + distances[farthest][next];
            if (delta < best) {
                best = delta;
                insertBefore = next;
                insertAt = city;
            }
            city = next;
        }
        total += best;
        g_tspNext[farthest] = insertBefore;
        g_tspNext[insertAt] = farthest;
        for (i = 0; i < count; ++i) {
            if (g_tspNext[i] == -1) {
                const int distance = distances[farthest][i];
                if (distance < g_tspDistanceToTour[i]) {
                    g_tspDistanceToTour[i] = distance;
                }
            }
        }
    }
    if (step > 0) {
        *cost = total;
    }
    for (i = 0; i < count; ++i) {
        order[i] = start;
        start = g_tspNext[start];
    }
}

// RET 4.
// FUNCTION: VC1 0x0049b620
int KdTSPSolver::GetResultID(int index)
{
    return order[index];
}

// RET 8. The tour over all cities of the matrix, starting at start.
// FUNCTION: VC1 0x0049b630
void KdTSPSolver::Solve(const KdObjArray<KdArray<int> >& distances, int start)
{
    const unsigned bytes = distances.Count() * sizeof(int);
    unsigned i;
    int cost;

    if (order.items.capacity < bytes) {
        GrowCountedBuffer(&order.items, static_cast<int>(bytes), 1);
    }
    order.items.size = bytes;
    for (i = 0; i < order.Count(); ++i) {
        order[i] = i;
    }
    cost = 0;
    FarthestInsertion(order.Count(), start, distances, order, &cost);
}

// ESI=this.
// FUNCTION: VC1 0x0049b6f0
KdTSPSolver::KdTSPSolver() throw()
{
    GrowCountedBuffer(&order.items, 0x400, 1);
}

// The singleton's IAbstract face, created on first use.
// FUNCTION: VC1 0x0049b740
void** CreateKdTspSolver(void** out)
{
    if (!g_kdTspSolver.ptr) {
        void* const memory = malloc(sizeof(KdTSPSolver));
        IUnknown* queried = 0;

        if (memory) {
            KdTSPSolver* const object = new (memory) KdTSPSolver();
            if (object) {
                IAbstract* const face = object;
                if (face) {
                    queried = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
                    if (queried) {
                        queried->AddRef();
                        queried->AddRef();
                    }
                }
            }
        }
        if (g_kdTspSolver.ptr) {
            g_kdTspSolver.ptr->Release();
        }
        g_kdTspSolver.ptr = queried;
        if (queried) {
            queried->Release();
        }
        g_kdTspSolver.ptr->Release();
    }
    IUnknown* const solver = g_kdTspSolver.ptr;
    *out = solver;
    if (solver) {
        solver->AddRef();
    }
    return out;
}

// RET 8. IScriptObject vtable+0x0c: the static method table only.
// FUNCTION: VC1 0x0049b920
unsigned char KdTSPSolver::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSolve] = CreateScriptMethod<KdTSPSolver>(&KdTSPSolver::Solve);
        methods[kScriptGetResultID] = CreateScriptMethod<KdTSPSolver>(&KdTSPSolver::GetResultID);
    }
    return methods.Find(name, out);
}

}
