#include "kd/route/route.h"

#include "kd/iface/iid.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <math.h>
#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfd28
static const char kRoute[] = "Route";
// GLOBAL: VC1 0x004d0b18
const char* g_kdNameRoute = kRoute;

// STRING: VC1 0x004d17ac
static const char kScriptAddPosition[] = "AddPosition";
// STRING: VC1 0x004d17b8
static const char kScriptInitFromRouter[] = "InitFromRouter";
// STRING: VC1 0x004d17d0
static const char kScriptGetLength[] = "GetLength";
// STRING: VC1 0x004d17dc
static const char kScriptIsEmpty[] = "IsEmpty";
// STRING: VC1 0x004d17e4
static const char kScriptTransform[] = "Transform";
// STRING: VC1 0x004d17f0
static const char kScriptGetPointCount[] = "GetPointCount";
// STRING: VC1 0x004d1800
static const char kScriptGetPointPosition[] = "GetPointPosition";
// STRING: VC1 0x004d1814
static const char kScriptGetPointDirection[] = "GetPointDirection";
// STRING: VC1 0x004d1828
static const char kScriptGetPointDistance[] = "GetPointDistance";
// STRING: VC1 0x004d183c
static const char kScriptGetProgressPosition[] = "GetProgressPosition";
// STRING: VC1 0x004d1850
static const char kScriptGetProgressDirection[] = "GetProgressDirection";

// MOV EAX, "Route"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00406a50
const char* KdRoute::GetTypeName()
{
    return kRoute;
}

// Inferred name. ECX=result, EAX=a, EDX=b, ESI=&from, stack &to, &at: the
// position at distance at on the segment a (at from) to b (at to); a when the
// segment has zero length.
// FUNCTION: VC1 0x00406ee0
static __declspec(noinline) Vector2 InterpolateByDistance(const Vector2& a, const Vector2& b,
    const float& from, const float& to, const float& at)
{
    Vector2 result = a;
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    const float span = to - from;

    if (span != 0.0f) {
        const float offset = at - from;
        const float sx = dx * offset;
        const float sy = dy * offset;
        const float scale = 1.0f / span;
        result.x = result.x + sx * scale;
        result.y = sy * scale + result.y;
    }
    return result;
}

// IRoute vtable+0x24.
// FUNCTION: VC1 0x004a8f40
KdObjArray<KdRoutePoint>* KdRoute::GetPoints()
{
    return &points;
}

// IRoute vtable+0x20, script "GetLength".
// FUNCTION: VC1 0x004a8f50
float KdRoute::GetLength()
{
    return length;
}

// RET 0x10. IRoute vtable+0x10: the router's path (IGridRouter vtable+0x10),
// then its points (IRoute vtable+0x14).
// FUNCTION: VC1 0x004a8f60
void KdRoute::FindPath(const KdPtr<IGridRouter>& router, const Vector2& from, const Vector2& to,
    bool worldSpace)
{
    router->FindPath(from, to, worldSpace);
    AssignPoints(router);
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004a8f90
unsigned char KdRoute::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// RET 8. Script "GetProgressDirection": GetSampleAt(distance).direction.
// FUNCTION: VC1 0x004a8fa0
Vector2 KdRoute::GetProgressDirection(float distance)
{
    return GetSampleAt(distance).direction;
}

// RET 8. Script "GetProgressPosition": GetSampleAt(distance).position.
// FUNCTION: VC1 0x004a8fe0
Vector2 KdRoute::GetProgressPosition(float distance)
{
    return GetSampleAt(distance).position;
}

// Script "GetPointCount".
// FUNCTION: VC1 0x004a9020
unsigned KdRoute::GetPointCount()
{
    return points.Count();
}

// IRoute vtable+0x1c, script "IsEmpty": fewer than two points.
// FUNCTION: VC1 0x004a9030
bool KdRoute::IsEmpty()
{
    return points.Count() < 2;
}

// ADD ECX, 0x14; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004a90c0
unsigned KdRoute::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a90e0
unsigned KdRoute::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// Inferred name. EDI=this. Each point's distance from the start along the
// polyline; length is the last one.
// FUNCTION: VC1 0x004a90f0
__declspec(noinline) void KdRoute::UpdateDistances()
{
    unsigned i;

    length = 0.0f;
    if (points.Count() != 0) {
        points[0].distance = length;
    }
    for (i = 1; i < points.Count(); ++i) {
        KdRoutePoint* const point = &points[i];
        const float dx = point->position.x - point[-1].position.x;
        const float dy = point->position.y - point[-1].position.y;
        length = sqrtf(dx * dx + dy * dy) + length;
        point->distance = length;
    }
}

// RET 4. IRoute vtable+0x2c, script "Transform": every point through the
// affine matrix.
// FUNCTION: VC1 0x004a91b0
void KdRoute::Transform(const Matrix3& matrix)
{
    unsigned i;

    for (i = 0; i < points.Count(); ++i) {
        Vector2& position = points[i].position;
        const float x = position.x;
        const float y = position.y;
        position.x = x * matrix._11 + y * matrix._21 + matrix._31;
        position.y = y * matrix._22 + x * matrix._12 + matrix._32;
    }
    UpdateDistances();
}

// RET 4. IWriteable vtable+0x0c: the point count, then x and y of each point.
// FUNCTION: VC1 0x004a9230
bool KdRoute::WriteTo(const KdPtr<IOutputStream>& stream)
{
    unsigned count = points.Count();
    unsigned i;

    if (stream.ptr->Write(&count, 4) != 4) {
        return false;
    }
    for (i = 0; i < points.Count(); ++i) {
        if (stream.ptr->Write(&points[i].position.x, 4) != 4) {
            return false;
        }
        if (stream.ptr->Write(&points[i].position.y, 4) != 4) {
            return false;
        }
    }
    return true;
}

// RET 4. Script "GetPointDistance".
// FUNCTION: VC1 0x004a92e0
float KdRoute::GetPointDistance(unsigned index)
{
    return points[index].distance;
}

// RET 8. Script "GetPointDirection".
// FUNCTION: VC1 0x004a9300
Vector2 KdRoute::GetPointDirection(unsigned index)
{
    const Vector2& direction = points[index].direction;
    return Vector2(direction.x, direction.y);
}

// RET 8. Script "GetPointPosition".
// FUNCTION: VC1 0x004a9330
Vector2 KdRoute::GetPointPosition(unsigned index)
{
    const Vector2& position = points[index].position;
    return Vector2(position.x, position.y);
}

// RET 8. IRoute vtable+0x28: the first point at or past distance ends the
// segment; zeros (and distance untouched) when the route is shorter.
// FUNCTION: VC1 0x004a9350
KdRouteSample KdRoute::GetSampleAt(float distance)
{
    KdRouteSample sample;
    unsigned i;

    sample.position.x = 0.0f;
    sample.position.y = 0.0f;
    sample.direction.x = 0.0f;
    sample.direction.y = 0.0f;
    for (i = 1; i < points.Count(); ++i) {
        if (!(points[i].distance < distance)) {
            const KdRoutePoint* end = &points[i];
            float dx;
            float dy;
            float scale;

            sample.position = InterpolateByDistance(end[-1].position, end->position,
                end[-1].distance, end->distance, distance);
            sample.distance = distance;
            end = &points[i];
            dx = end->position.x - end[-1].position.x;
            dy = end->position.y - end[-1].position.y;
            scale = 1.0f / sqrtf(dx * dx + dy * dy);
            sample.direction.x = scale * dx;
            sample.direction.y = scale * dy;
            return sample;
        }
    }
    return sample;
}

// IRoute vtable+0x18, script "Clear".
// FUNCTION: VC1 0x004a9470
void KdRoute::Clear()
{
    points.items.size = 0;
    length = 0.0f;
}

// RET 4. IReadable vtable+0x0c: the format of WriteTo, then the distances.
// FUNCTION: VC1 0x004a9480
bool KdRoute::ReadFrom(const KdPtr<IInputStream>& stream)
{
    unsigned count;
    unsigned bytes;
    unsigned i;

    if (stream.ptr->Read(&count, 4) != 4) {
        return false;
    }
    bytes = count * sizeof(KdRoutePoint);
    if (points.items.capacity < bytes) {
        points.Grow(count, true);
    }
    points.items.size = bytes;
    for (i = 0; i < count; ++i) {
        if (stream.ptr->Read(&points[i].position.x, 4) != 4) {
            return false;
        }
        if (stream.ptr->Read(&points[i].position.y, 4) != 4) {
            return false;
        }
    }
    UpdateDistances();
    return true;
}

// RET 4. IRoute vtable+0x14, script "InitFromRouter": the router's last path
// (IGridRouter vtable+0x18 count, vtable+0x14 point).
// FUNCTION: VC1 0x004a9530
void KdRoute::AssignPoints(const KdPtr<IGridRouter>& router)
{
    unsigned i;

    Clear();
    for (i = 0; i < router->GetPointCount(); ++i) {
        KdRoutePoint& point = AppendPoint();
        point.position = router->GetPoint(i);
    }
    UpdateDistances();
}

// RET 4. IRoute vtable+0x0c, script "AddPosition".
// FUNCTION: VC1 0x004a9610
void KdRoute::AddPoint(const Vector2& position)
{
    KdRoutePoint& point = AppendPoint();
    point.position = position;
    UpdateDistances();
}

// ESI=this.
// FUNCTION: VC1 0x004a96b0
KdRoute::~KdRoute() throw()
{
    IScriptInstanceReset* reset = &script;
    reset->Reset();
}

// EAX=this.
// FUNCTION: VC1 0x004a9760
KdRoute::KdRoute() throw()
    : length(0.0f)
{
}

// FUNCTION: VC1 0x004a9800
void** CreateKdRoute(void** out)
{
    void* const memory = malloc(sizeof(KdRoute));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdRoute();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 4. IScriptLoadable vtable+0x0c, owner = the IScriptObject face.
// FUNCTION: VC1 0x004a9e00
bool KdRoute::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this)) != 0;
}

// RET 8. The script instance's members first, then the method table (table
// 0x004ee650, guard bit 0x004ee660).
// FUNCTION: VC1 0x004a9f60
unsigned char KdRoute::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    // GLOBAL: VC1 0x004ee650
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptAddPosition] = CreateScriptMethod<KdRoute>(&KdRoute::AddPoint);
        methods[kScriptInitFromRouter] = CreateScriptMethod<KdRoute>(&KdRoute::AssignPoints);
        methods[kScriptClear] = CreateScriptMethod<KdRoute>(&KdRoute::Clear);
        methods[kScriptGetLength] = CreateScriptMethod<KdRoute>(&KdRoute::GetLength);
        methods[kScriptIsEmpty] = CreateScriptMethod<KdRoute>(&KdRoute::IsEmpty);
        methods[kScriptTransform] = CreateScriptMethod<KdRoute>(&KdRoute::Transform);
        methods[kScriptGetPointCount] = CreateScriptMethod<KdRoute>(&KdRoute::GetPointCount);
        methods[kScriptGetPointPosition] =
            CreateScriptMethod<KdRoute>(&KdRoute::GetPointPosition);
        methods[kScriptGetPointDirection] =
            CreateScriptMethod<KdRoute>(&KdRoute::GetPointDirection);
        methods[kScriptGetPointDistance] =
            CreateScriptMethod<KdRoute>(&KdRoute::GetPointDistance);
        methods[kScriptGetProgressPosition] =
            CreateScriptMethod<KdRoute>(&KdRoute::GetProgressPosition);
        methods[kScriptGetProgressDirection] =
            CreateScriptMethod<KdRoute>(&KdRoute::GetProgressDirection);
    }
    return methods.Find(name, out);
}

}
