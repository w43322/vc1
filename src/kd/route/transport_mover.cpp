#include "kd/route/transport_mover.h"

#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "sqplus/SquirrelVM.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// STRING: VC1 0x004cfd18
static const char kTransportMover[] = "TransportMover";
// GLOBAL: VC1 0x004d0b1c
const char* g_kdNameTransportMover = kTransportMover;

extern const char kScriptSetGridMap[];    // terrain_builder.cpp
extern const char kScriptSetGridRouter[]; // terrain_builder.cpp

// STRING: VC1 0x004d21bc
static const char kScriptSetSpeed[] = "SetSpeed";
// STRING: VC1 0x004d21c8
static const char kScriptAddTransport[] = "AddTransport";
// STRING: VC1 0x004d21d8
static const char kScriptRemoveTransport[] = "RemoveTransport";
// STRING: VC1 0x004d21e8
static const char kScriptInitMoving[] = "InitMoving";
// STRING: VC1 0x004d21f4
static const char kScriptInitPosition[] = "InitPosition";
// STRING: VC1 0x004d2204
static const char kScriptGetRouteLength[] = "GetRouteLength";
// STRING: VC1 0x004d2214
static const char kScriptCanStartMoving[] = "CanStartMoving";
// STRING: VC1 0x004d2224
static const char kScriptSetMoving[] = "SetMoving";
// STRING: VC1 0x004d2230
static const char kScriptStopMoving[] = "StopMoving";
// STRING: VC1 0x004d223c
static const char kScriptPauseMoving[] = "PauseMoving";
// STRING: VC1 0x004d2248
static const char kScriptMoveToRoadsite[] = "MoveToRoadsite";
// STRING: VC1 0x004d2258
static const char kScriptFindPathBetweenAreas[] = "FindPathBetweenAreas";
// STRING: VC1 0x004d2270
static const char kScriptFindPathToArea[] = "FindPathToArea";
// STRING: VC1 0x004d2280
static const char kScriptWriteTransportToStream[] = "WriteTransportToStream";
// STRING: VC1 0x004d2298
static const char kScriptReadTransportFromStream[] = "ReadTransportFromStream";
// STRING: VC1 0x004d22b0
static const char kScriptCreateOptimalRouteFromArea[] = "CreateOptimalRouteFromArea";
// STRING: VC1 0x004d22cc
static const char kScriptCreateOptimalRouteFromPosition[] = "CreateOptimalRouteFromPosition";
// STRING: VC1 0x004d22ec
static const char kScriptGetOptimalRouteIndexesReachable[] = "GetOptimalRouteIndexesReachable";
// STRING: VC1 0x004d230c
static const char kScriptGetOptimalRouteIndexesUnreachable[] = "GetOptimalRouteIndexesUnreachable";

// Inferred name. This file's own never-assigned null object; new records
// take their route from it.
// GLOBAL: VC1 0x004ed6a8
KdRef<IUnknown> g_kdTransportMoverNullObject;

// MOV EAX, "TransportMover"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0040b2f0
const char* KdTransportMover::GetTypeName()
{
    return kTransportMover;
}

// EDI=source, ESI=this. Counted copies of the three object slots.
// FUNCTION: VC1 0x0040bb10
KdTransportRecord& KdTransportRecord::operator=(const KdTransportRecord& source)
{
    transport.Assign(source.transport);
    route.Assign(source.route);
    distance = source.distance;
    unknown0c = source.unknown0c;
    arrived = source.arrived;
    moving = source.moving;
    target.Assign(source.target);
    paused = source.paused;
    return *this;
}

// EAX=this, EBX=count. Loose grow; the new records get null object slots.
// FUNCTION: VC1 0x0040bba0
void KdTransportRecords::Reserve(unsigned count)
{
    const unsigned old = items.capacity / sizeof(KdTransportRecord);
    KdTransportRecord* record;
    KdTransportRecord* end;

    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(KdTransportRecord)), 0);
    if (items.capacity / sizeof(KdTransportRecord) > old) {
        record = Begin() + old;
        end = Begin() + items.capacity / sizeof(KdTransportRecord);
        for (; record < end; ++record) {
            if (record) {
                ::new (static_cast<void*>(record)) KdTransportRecord();
            }
        }
    }
}

// EAX=first, EBX=last. Releases the object slots of [first, last).
// FUNCTION: VC1 0x0040c700
void KdTransportRecords::DestroyRange(KdTransportRecord* first, KdTransportRecord* last)
{
    for (; first < last; ++first) {
        first->~KdTransportRecord();
    }
}

// Inferred name. EAX=v. Script array of Rect instances at idx, the Rect twin
// of the Vector2 one (0x00406430); a missing slot is skipped.
// FUNCTION: VC1 0x0040c750
KdObjArray<Rect> Get(SqPlus::TypeWrapper<const KdObjArray<Rect>&>, HSQUIRRELVM v, int idx)
{
    unsigned i;

    if (idx < 0) {
        idx = sq_gettop(v) + idx + 1;
    }
    KdObjArray<Rect> result(sq_getsize(v, idx));
    for (i = 0; i < result.Count(); ++i) {
        sq_pushinteger(v, i);
        if (SQ_SUCCEEDED(sq_get(v, idx))) {
            result[i] = *SqPlus::GetInstance<Rect, true>(v, sq_gettop(v));
            sq_pop(v, 1);
        }
    }
    return result;
}

// ESI=v. New script array, then array[i] = value[i] for the bytes.
// FUNCTION: VC1 0x0040c8d0
void Push(HSQUIRRELVM v, KdArray<unsigned char>& value)
{
    unsigned i;

    sq_newarray(v, value.Count());
    for (i = 0; i < value.Count(); ++i) {
        sq_pushinteger(v, i);
        sq_pushinteger(v, value[i]);
        sq_set(v, -3);
    }
}

// Script "GetOptimalRouteIndexesUnreachable": the target indexes the last
// CreateOptimalRoute* could not reach.
// FUNCTION: VC1 0x0049bac0
KdArray<unsigned char>& KdTransportMover::GetOptimalRouteIndexesUnreachable()
{
    return unreachable;
}

// Script "GetOptimalRouteIndexesReachable": the visiting order of the targets
// the last CreateOptimalRoute* reached.
// FUNCTION: VC1 0x0049bad0
KdArray<unsigned char>& KdTransportMover::GetOptimalRouteIndexesReachable()
{
    return reachable;
}

// RET 4. Script "SetSpeed".
// FUNCTION: VC1 0x0049bae0
void KdTransportMover::SetSpeed(float value)
{
    speed = value;
}

// IRenderable vtable+0x14.
// FUNCTION: VC1 0x0049baf0
int KdTransportMover::GetRenderOrder()
{
    return 1000;
}

// IVisible vtable+0x10 (ECX=this+0x0c).
// FUNCTION: VC1 0x0049bb00
bool KdTransportMover::IsVisible()
{
    return visible;
}

// RET 4. IVisible vtable+0x0c.
// FUNCTION: VC1 0x0049bb10
void KdTransportMover::SetVisible(bool value)
{
    visible = value;
}

// RET 4. IUpdateable vtable+0x10: the layer is fixed.
// FUNCTION: VC1 0x0049bb20
int KdTransportMover::SetLayer(unsigned)
{
    return 0;
}

// IUpdateable vtable+0x0c.
// FUNCTION: VC1 0x0049bb30
int KdTransportMover::GetUpdateLayer()
{
    return 2;
}

// RET 8. IScriptObject vtable+0x10: no script members.
// FUNCTION: VC1 0x0049bb40
unsigned char KdTransportMover::SetMember(void*, void*)
{
    return 0;
}

// RET 4. Script "SetGridRouter".
// FUNCTION: VC1 0x0049bc80
void KdTransportMover::SetGridRouter(const KdPtr<IGridRouter>& value)
{
    router.Assign(value);
}

// RET 4. Script "SetGridMap": transports drive on road, indestructible road
// and road under construction; the grid map's position gives both grid
// transforms.
// FUNCTION: VC1 0x0049bcc0
void KdTransportMover::SetGridMap(const KdPtr<IGridMap>& value)
{
    gridMap.Assign(value);
    if (value.ptr) {
        passableMask = 1 << gridMap->roadConstruction | 1 << gridMap->roadIndestructible
            | 1 << gridMap->road;
        gridToWorld = value->GetPosition();
        worldToGrid = Matrix3::Inverse(gridToWorld);
    }
}

// IAbstract-face release (ECX=this, adjusted to the IAbstract face).
// FUNCTION: VC1 0x0049bdc0
unsigned KdTransportMover::Release()
{
    IAbstract* iface = static_cast<IAbstract*>(this);
    unsigned n;

    iface->refs -= 1;
    n = iface->refs;
    if (n == 0) {
        delete iface;
        return 0;
    }
    return n;
}

// FUNCTION: VC1 0x0049bde0
unsigned KdTransportMover::AddRef()
{
    unsigned n = refs;
    if (n != 0) {
        n += 1;
        refs = n;
    }
    return n;
}

// RET 8 (stdcall). Index of the record whose transport is the same object
// (IAbstract faces compared), or -1.
// FUNCTION: VC1 0x0049bdf0
int __stdcall KdTransportMover::FindTransport(KdTransportMover* self,
    const KdPtr<IPositionable>& transport)
{
    const unsigned count = self->records.Count();
    unsigned i;

    for (i = 0; i < count; ++i) {
        IPositionable* const item = self->records[i].transport;
        if (!item) {
            if (!transport.ptr) {
                return static_cast<int>(i);
            }
        } else if (transport.ptr) {
            void* const face = transport->QueryInterface(&g_iidAbstract);
            if (face == item->QueryInterface(&g_iidAbstract)) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

// RET 8. Script "WriteTransportToStream".
// STUB: VC1 0x0049be90
bool KdTransportMover::WriteTransportToStream(const KdPtr<IOutputStream>&,
    const KdPtr<IPositionable>&)
{
    return false;
}

// RET 4. Script "GetRouteLength": the length of the transport's route.
// FUNCTION: VC1 0x0049bfa0
float KdTransportMover::GetRouteLength(const KdPtr<IPositionable>& transport)
{
    if (gridMap.ptr) {
        const int index = FindTransport(this, transport);
        if (index >= 0) {
            IRoute* const route = records[index].route;
            if (!route) {
                return 0.0f;
            }
            return route->GetLength();
        }
    }
    return 0.0f;
}

// RET 8. Script "PauseMoving".
// FUNCTION: VC1 0x0049c000
void KdTransportMover::PauseMoving(const KdPtr<IPositionable>& transport, bool value)
{
    const int index = FindTransport(this, transport);
    if (index >= 0) {
        records[index].paused = value;
    }
}

// RET 4. Script "StopMoving": clears the transport's route.
// FUNCTION: VC1 0x0049c030
void KdTransportMover::StopMoving(const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index >= 0) {
        KdTransportRecord& record = records[index];
        if (record.route.ptr && !record.route->IsEmpty()) {
            record.route->Clear();
        }
    }
}

// RET 8. Script "SetMoving".
// FUNCTION: VC1 0x0049c080
void KdTransportMover::SetMoving(const KdPtr<IPositionable>& transport, bool value)
{
    const int index = FindTransport(this, transport);
    if (index >= 0) {
        records[index].moving = value;
    }
}

// RET 4. Script "InitPosition": puts the transport at the start of its route,
// turned along the first segment.
// FUNCTION: VC1 0x0049c0b0
void KdTransportMover::InitPosition(const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return;
    }
    KdTransportRecord& record = records[index];
    const KdRouteSample sample = record.route->GetSampleAt(0.0f);
    Matrix3 position;
    memcpy(&position, &record.transport->GetPosition(), sizeof(position));
    position._31 = sample.position.x;
    position._32 = sample.position.y;
    position._11 = sample.direction.x;
    position._12 = sample.direction.y;
    position._21 = -sample.direction.y;
    position._22 = sample.direction.x;
    record.transport->SetPosition(position);
}

// IShutdownable vtable+0x0c (ECX=this+4): forgets the records without
// releasing them.
// FUNCTION: VC1 0x0049c150
int KdTransportMover::Shutdown()
{
    records.items.ResizeBytes(0);
    return 0;
}

// RET 4. Script "MoveToRoadsite".
// STUB: VC1 0x0049c4a0
bool KdTransportMover::MoveToRoadsite(const KdPtr<IPositionable>&)
{
    return false;
}

// RET 4. Script "CanStartMoving".
// STUB: VC1 0x0049c710
bool KdTransportMover::CanStartMoving(const KdPtr<IPositionable>&)
{
    return false;
}

// RET 4. IRenderable vtable+0x10: the routes while visible.
// STUB: VC1 0x0049c8c0
int KdTransportMover::Render(void*)
{
    return 0;
}

// EDI=this. Member teardown only.
// FUNCTION: VC1 0x0049ce00
KdTransportMover::~KdTransportMover() throw()
{
}

// ESI=this. refs=1, identity grid transforms, both index arrays reserve 0xff
// bytes.
// FUNCTION: VC1 0x0049ced0
KdTransportMover::KdTransportMover() throw()
{
    cornerRadius = 0.2f;
    cornerOuter = 0.4f;
    visible = false;
    unknown28 = 50.0f;
    unknown2c = 24.0f;
    speed = 0.2f;
    GrowCountedBuffer(&reachable.items, 0xff, 1);
    GrowCountedBuffer(&unreachable.items, 0xff, 1);
}

// FUNCTION: VC1 0x0049d010
void** CreateKdTransportMover(void** out)
{
    void* const memory = malloc(sizeof(KdTransportMover));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdTransportMover();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 4. Script "RemoveTransport": closes the record's slot without
// releasing it.
// FUNCTION: VC1 0x0049d2a0
void KdTransportMover::RemoveTransport(const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index >= 0) {
        KdTransportRecord* const record = &records[index];
        EraseCountedBytes(record, record + 1, records.End());
        records.items.size -= sizeof(KdTransportRecord);
    }
}

// RET 4. Script "AddTransport": a stopped record with the null object's
// route.
// FUNCTION: VC1 0x0049d2f0
void KdTransportMover::AddTransport(const KdPtr<IPositionable>& transport)
{
    KdTransportRecord record;
    unsigned count;

    if (transport.ptr) {
        transport.ptr->AddRef();
    }
    record.transport = transport.ptr;
    if (g_kdTransportMoverNullObject.ptr) {
        record.route = static_cast<IRoute*>(
            g_kdTransportMoverNullObject->QueryInterface(&g_iidRoute));
        if (record.route.ptr) {
            record.route->AddRef();
        }
    }
    record.distance = 0.0f;
    record.arrived = false;
    record.moving = false;
    record.paused = false;
    count = records.Count() + 1;
    if (records.items.capacity < count * sizeof(KdTransportRecord)) {
        records.Reserve(count);
    }
    records.items.size = count * sizeof(KdTransportRecord);
    records[count - 1] = record;
}

// RET 8. IUpdateable vtable+0x14 (ECX=this+8): advances the moving records.
// STUB: VC1 0x0049e210
void KdTransportMover::Update(unsigned, int)
{
}

// RET 8. Script "ReadTransportFromStream".
// STUB: VC1 0x0049e9d0
bool KdTransportMover::ReadTransportFromStream(const KdPtr<IInputStream>&,
    const KdPtr<IPositionable>&)
{
    return false;
}

// Script "FindPathToArea": a new route from a point into a cell rect.
// STUB: VC1 0x0049eaf0
KdPtr<IUnknown> KdTransportMover::FindPathToArea(const Vector2&, const Vector2&, const Rect&)
{
    return KdPtr<IUnknown>();
}

// Script "FindPathBetweenAreas".
// STUB: VC1 0x0049edd0
KdPtr<IUnknown> KdTransportMover::FindPathBetweenAreas(const Vector2&, const Rect&,
    const Vector2&, const Rect&)
{
    return KdPtr<IUnknown>();
}

// RET 8. Script "InitMoving": gives the transport a route.
// STUB: VC1 0x0049f020
bool KdTransportMover::InitMoving(const KdPtr<IPositionable>&, const KdPtr<IRoute>&)
{
    return false;
}

// Script "CreateOptimalRouteFromPosition".
// STUB: VC1 0x0049f160
void KdTransportMover::CreateOptimalRouteFromPosition(const KdObjArray<Vector2>&,
    const KdObjArray<Rect>&, int, const Vector2&)
{
}

// Script "CreateOptimalRouteFromArea".
// STUB: VC1 0x0049f740
void KdTransportMover::CreateOptimalRouteFromArea(const KdObjArray<Vector2>&,
    const KdObjArray<Rect>&, int, const Vector2&, const Rect&)
{
}

// RET 8. IScriptObject vtable+0x0c: the static method table only.
// FUNCTION: VC1 0x0049f830
unsigned char KdTransportMover::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetGridMap] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::SetGridMap);
        methods[kScriptSetGridRouter] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::SetGridRouter);
        methods[kScriptSetSpeed] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::SetSpeed);
        methods[kScriptAddTransport] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::AddTransport);
        methods[kScriptRemoveTransport] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::RemoveTransport);
        methods[kScriptInitMoving] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::InitMoving);
        methods[kScriptInitPosition] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::InitPosition);
        methods[kScriptGetRouteLength] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::GetRouteLength);
        methods[kScriptCanStartMoving] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::CanStartMoving);
        methods[kScriptSetMoving] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::SetMoving);
        methods[kScriptStopMoving] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::StopMoving);
        methods[kScriptPauseMoving] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::PauseMoving);
        methods[kScriptMoveToRoadsite] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::MoveToRoadsite);
        methods[kScriptFindPathBetweenAreas] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::FindPathBetweenAreas);
        methods[kScriptFindPathToArea] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::FindPathToArea);
        methods[kScriptWriteTransportToStream] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::WriteTransportToStream);
        methods[kScriptReadTransportFromStream] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::ReadTransportFromStream);
        methods[kScriptCreateOptimalRouteFromArea] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::CreateOptimalRouteFromArea);
        methods[kScriptCreateOptimalRouteFromPosition] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::CreateOptimalRouteFromPosition);
        methods[kScriptGetOptimalRouteIndexesReachable] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::GetOptimalRouteIndexesReachable);
        methods[kScriptGetOptimalRouteIndexesUnreachable] =
            CreateScriptMethod<KdTransportMover>(&KdTransportMover::GetOptimalRouteIndexesUnreachable);
    }
    return methods.Find(name, out);
}

}
