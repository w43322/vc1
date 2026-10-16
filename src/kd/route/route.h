#ifndef VC1_KD_ROUTE_ROUTE_H
#define VC1_KD_ROUTE_ROUTE_H

#include "kd/script_instance.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iroute.h"
#include "kd/iface/iscript_object.h"

class SquirrelObject;

namespace vc1 {

// Inferred names. Factory "Route": a polyline of points with the distance of
// each point from the start, filled point by point or from an IGridRouter's
// path, sampled by distance (position and segment direction), saved as the
// point count and positions. QI 0x004a9050 establishes the face offsets.
// SIZE 0x48
// VTABLE: VC1 0x004d1898 vc1::IScriptObject
// VTABLE: VC1 0x004d18b0 vc1::IScriptLoadable
// VTABLE: VC1 0x004d18c0 vc1::IWriteable
// VTABLE: VC1 0x004d18d0 vc1::IReadable
// VTABLE: VC1 0x004d18e0 vc1::IRoute
// VTABLE: VC1 0x004d1910 vc1::IAbstract
class KdRoute : public IScriptObject,
                public IScriptLoadable,
                public IWriteable,
                public IReadable,
                public IRoute,
                public IAbstract
{
public:
    KdScriptInstance script;          // 0x1c
    KdObjArray<KdRoutePoint> points;  // 0x34
    float length;                     // 0x44

    KdRoute() throw();
    ~KdRoute() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();

    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();

    bool LoadFromScript(SquirrelObject& table);
    bool WriteTo(const KdPtr<IOutputStream>& stream);
    bool ReadFrom(const KdPtr<IInputStream>& stream);

    void AddPoint(const Vector2& point);
    void FindPath(const KdPtr<IGridRouter>& router, const Vector2& from, const Vector2& to,
        bool worldSpace);
    void AssignPoints(const KdPtr<IGridRouter>& router);
    void Clear();
    bool IsEmpty();
    float GetLength();
    KdObjArray<KdRoutePoint>* GetPoints();
    KdRouteSample GetSampleAt(float distance);
    void Transform(const Matrix3& matrix);

    unsigned GetPointCount();
    Vector2 GetPointPosition(unsigned index);
    Vector2 GetPointDirection(unsigned index);
    float GetPointDistance(unsigned index);
    Vector2 GetProgressPosition(float distance);
    Vector2 GetProgressDirection(float distance);

    void UpdateDistances();

private:
    __forceinline KdRoutePoint& AppendPoint()
    {
        const unsigned count = points.Count() + 1;
        const unsigned bytes = count * sizeof(KdRoutePoint);
        KdRoutePoint* last;

        if (points.items.capacity < bytes) {
            points.Grow(count, false);
        }
        last = reinterpret_cast<KdRoutePoint*>(
            reinterpret_cast<char*>(points.items.begin) + bytes) - 1;
        points.items.size = bytes;
        *last = KdRoutePoint();
        return *last;
    }
};

typedef char Vc1KdRouteSizeCheck[sizeof(KdRoute) == 0x48 ? 1 : -1];

void** CreateKdRoute(void** out);

// SYNTHETIC: VC1 0x00406a60
// vc1::KdRoute::`scalar deleting destructor'
// TEMPLATE: VC1 0x00406a80
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEXABUVector2@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEXABUVector2@0@@Z@Z
// TEMPLATE: VC1 0x00406af0
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEXABV?$KdPtr@UIGridRouter@vc1@@@0@@Z@Z
// TEMPLATE: VC1 0x00406b60
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEXXZ@Z
// TEMPLATE: VC1 0x00406bd0
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEMXZ@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEMXZ@Z
// TEMPLATE: VC1 0x00406c40
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AE_NXZ@Z
// TEMPLATE: VC1 0x00406cb0
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEXABUMatrix3@0@@Z@Z
// TEMPLATE: VC1 0x00406d20
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEIXZ@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEIXZ@Z
// TEMPLATE: VC1 0x00406d90
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AE?AUVector2@2@I@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AE?AUVector2@0@I@Z@Z
// TEMPLATE: VC1 0x00406e00
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AEMI@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AEMI@Z@Z
// TEMPLATE: VC1 0x00406e70
// ??$CreateScriptMethod@VKdRoute@vc1@@P812@AE?AUVector2@2@M@Z@vc1@@YA?AVSquirrelObject@@P8KdRoute@0@AE?AUVector2@0@M@Z@Z
// TEMPLATE: VC1 0x00406f80
// ?Grow@?$KdObjArray@UKdRoutePoint@vc1@@@vc1@@QAEXI_N@Z
// TEMPLATE: VC1 0x00407000
// ??$Call@VKdRoute@vc1@@ABV?$KdPtr@UIGridRouter@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdRoute@vc1@@P823@AEXABV?$KdPtr@UIGridRouter@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x004070f0
// ??$Call@VKdRoute@vc1@@I@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAVKdRoute@vc1@@P823@AE?AUVector2@3@I@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00407170
// ??$Call@VKdRoute@vc1@@M@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAVKdRoute@vc1@@P823@AE?AUVector2@3@M@ZPAUSQVM@@H@Z
// SYNTHETIC: VC1 0x00407210
// ?QueryInterface@KdRoute@vc1@@WM@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407220
// ?Release@KdRoute@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00407230
// ?QueryInterface@KdRoute@vc1@@WBA@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407240
// ?Release@KdRoute@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x00407250
// ?AddRef@KdRoute@vc1@@W3AEIXZ
// SYNTHETIC: VC1 0x00407260
// ?QueryInterface@KdRoute@vc1@@WBE@AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407270
// ?Release@KdRoute@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x00407280
// ?AddRef@KdRoute@vc1@@W7AEIXZ
// SYNTHETIC: VC1 0x00407290
// ??_EKdRoute@vc1@@WBE@AEPAXI@Z
// SYNTHETIC: VC1 0x004072a0
// ?Release@KdRoute@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004072b0
// ?AddRef@KdRoute@vc1@@WM@AEIXZ
// SYNTHETIC: VC1 0x004072c0
// ?AddRef@KdRoute@vc1@@WBA@AEIXZ
// SYNTHETIC: VC1 0x004072d0
// ?AddRef@KdRoute@vc1@@WBE@AEIXZ
// SYNTHETIC: VC1 0x004072e0
// ?QueryInterface@KdRoute@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x004072f0
// ?QueryInterface@KdRoute@vc1@@W7AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00407300
// ?Release@KdRoute@vc1@@W3AEIXZ
// TEMPLATE: VC1 0x004a9850
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEMI@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9940
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEIXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9a10
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9ac0
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9b60
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEMXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9c10
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9cb0
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEXABV?$KdPtr@UIGridRouter@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9d50
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AEXABUVector2@2@@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9e20
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AE?AUVector2@2@M@Z@vc1@@SAHPAUSQVM@@@Z
// TEMPLATE: VC1 0x004a9ec0
// ?Dispatch@?$ScriptMethodCall@VKdRoute@vc1@@P812@AE?AUVector2@2@I@Z@vc1@@SAHPAUSQVM@@@Z

}

#endif
