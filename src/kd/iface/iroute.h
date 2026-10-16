#ifndef VC1_KD_IROUTE_H
#define VC1_KD_IROUTE_H

#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/igrid_map.h"
#include "bindings/matrix3.h"
#include "bindings/vector2.h"

namespace vc1 {

// Inferred name. A route point: the position and the distance from the
// route start (the route class's AddPoint 0x004a9610 and GetSampleAt
// 0x004a9350).
// SIZE 0x14
struct KdRoutePoint
{
    Vector2 position; // 0x00
    Vector2 unused;   // 0x08 zeroed when a point is added
    float distance;   // 0x10
};

// Inferred name. A position on the route at some distance and the unit
// direction of the segment it lies on (GetSampleAt 0x004a9350).
// SIZE 0x14
struct KdRouteSample
{
    Vector2 position;  // 0x00
    Vector2 direction; // 0x08
    float distance;    // 0x10
};

// Inferred names. A polyline route; the slots are those of the route class's
// IRoute face (vftable 0x004d18e0, QI 0x004a9050 answers "IRoute" at +0x10).
// SIZE 0x04
struct IRoute : IUnknown
{
    virtual void AddPoint(const Vector2& point) = 0;                          // vtable+0x0c
    virtual void FindPath(const KdPtr<IGridRouter>& router, const Vector2& from,
        const Vector2& to, bool worldSpace) = 0;                              // vtable+0x10
    virtual void AssignPoints(const KdPtr<IGridRouter>& router) = 0;          // vtable+0x14
    virtual void Clear() = 0;                                                 // vtable+0x18
    virtual bool IsEmpty() = 0;                                               // vtable+0x1c fewer than two points
    virtual float GetLength() = 0;                                            // vtable+0x20
    virtual KdArray<KdRoutePoint>* GetPoints() = 0;                           // vtable+0x24
    virtual KdRouteSample GetSampleAt(float distance) = 0;                    // vtable+0x28
    virtual void Transform(const Matrix3& matrix) = 0;                        // vtable+0x2c
};

typedef char Vc1IRouteSizeCheck[sizeof(IRoute) == 0x04 ? 1 : -1];

template <>
struct KdIid<IRoute>
{
    static const char** Get() { return &g_iidRoute; }
};

}

#endif
