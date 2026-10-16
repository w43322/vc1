#include "kd/route/transport_mover.h"

#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "sqplus/SquirrelVM.h"

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

float WrapAngle(float angle); // host_api.cpp
float fltsign(float value);   // host_api.cpp

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

float MinFloat(float a, float b); // display.cpp
extern const char kScriptOnCellChange[]; // pass_map.cpp
bool ReadShort(const KdPtr<IInputStream>& stream, short* value);         // pass_map.cpp
bool WriteShort(const KdPtr<IOutputStream>& stream, const short* value); // pass_map.cpp

// STRING: VC1 0x004d2330
static const char kScriptOnMovingFinish[] = "OnMovingFinish";

static const double kHalfPi = 1.5707963705062866;
static const double kQuarterPi = 0.7853981852531433;
static const double kPi = 3.1415927410125732;
static const double kFiveQuarterPi = 3.9269909858703613;
static const double kTwoPi = 6.2831854820251465;

// Inferred name. cdecl float, the absolute value on the FPU stack; kept out
// of line in the blocker check.
// FUNCTION: VC1 0x00404320
static __declspec(noinline) float FloatAbs(float value)
{
    if (value < 0.0f) {
        value = -value;
    }
    return value;
}

// Inferred name. EAX=&stream, cdecl value. Folded with the IOutputStream
// twin: both are the slot+0x0c call with 1 byte.
// FUNCTION: VC1 0x00406860 FOLDED
static __declspec(noinline) bool ReadBool(const KdPtr<IInputStream>& stream, bool* value)
{
    return stream.ptr->Read(value, 1) == 1;
}

// FUNCTION: VC1 0x00406860 FOLDED
static __declspec(noinline) bool WriteBool(const KdPtr<IOutputStream>& stream, const bool* value)
{
    return stream.ptr->Write(value, 1) == 1;
}

// ECX=this, EAX=other: 1 when x or y differ.
// FUNCTION: VC1 0x00407600
bool KdGridCell::operator!=(const KdGridCell& other) const
{
    if (x == other.x && y == other.y) {
        return false;
    }
    return true;
}

// MOV EAX, "TransportMover"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0040b2f0
const char* KdTransportMover::GetTypeName()
{
    return kTransportMover;
}

// Inferred name. ESI=slot, stack source, RET 4: releases the slot, then
// takes the source's counted IAbstract face.
// FUNCTION: VC1 0x0040b320
static __declspec(noinline) KdRef<IAbstract>& AssignAbstractFace(KdRef<IAbstract>& slot,
    const KdRef<IPositionable>& source)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source.ptr) {
        slot.ptr = 0;
        return slot;
    }
    slot.ptr = static_cast<IAbstract*>(source->QueryInterface(&g_iidAbstract));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
    return slot;
}

// Inferred name. ESI=slot, EDI=source: counted copy, AddRef first.
// FUNCTION: VC1 0x0040b3a0
static __declspec(noinline) KdRef<IRoute>& AssignRoute(KdRef<IRoute>& slot,
    const KdPtr<IRoute>& source)
{
    if (source.ptr) {
        source.ptr->AddRef();
    }
    if (slot.ptr) {
        slot.ptr->Release();
    }
    slot.ptr = source.ptr;
    return slot;
}

// Inferred name. ESI=slot, stack source, RET 4: releases the slot, then
// takes the source's counted IRoute face.
// FUNCTION: VC1 0x0040b3d0
static __declspec(noinline) KdRef<IRoute>& AssignRouteFace(KdRef<IRoute>& slot,
    const KdRef<IUnknown>& source)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source.ptr) {
        slot.ptr = 0;
        return slot;
    }
    slot.ptr = static_cast<IRoute*>(source->QueryInterface(&g_iidRoute));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
    return slot;
}

// EDI=source, ESI=this. Counted copies of the three object slots.
// FUNCTION: VC1 0x0040bb10
KdTransportRecord& KdTransportRecord::operator=(const KdTransportRecord& source)
{
    transport.Assign(source.transport);
    route.Assign(source.route);
    distance = source.distance;
    cell = source.cell;
    atRoadside = source.atRoadside;
    moving = source.moving;
    blocker.Assign(source.blocker);
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

// Inferred name. EDI=this, EAX=cell, ECX=axes (X then Y axis of the
// transport). The heading picks the cells ahead, behind and to the right of
// cell; a spot beside the road is free when ahead and behind are
// indestructible road and the right cell is not.
// FUNCTION: VC1 0x0049bb50
bool KdTransportMover::CanParkBeside(const KdGridCell& cell, const Vector2* axes)
{
    KdGridCell ahead = cell;
    KdGridCell behind = cell;
    KdGridCell side = cell;
    const float angle = WrapAngle(static_cast<float>(
        atan2(static_cast<double>(axes[0].y), static_cast<double>(axes[0].x))));

    if (angle < kQuarterPi) {
        ++ahead.x;
        --behind.x;
        ++side.y;
    }
    else if (angle < 2.3561945f) {
        ++ahead.y;
        --behind.y;
        --side.x;
    }
    else if (angle < kFiveQuarterPi) {
        ++behind.x;
        --ahead.x;
        --side.y;
    }
    else {
        ++behind.y;
        --ahead.y;
        ++side.x;
    }
    return gridMap->roadIndestructible == gridMap->GetCellValue(ahead.x, ahead.y)
        && gridMap->roadIndestructible == gridMap->GetCellValue(behind.x, behind.y)
        && gridMap->roadIndestructible != gridMap->GetCellValue(side.x, side.y);
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
// FUNCTION: VC1 0x0049be90
bool KdTransportMover::WriteTransportToStream(const KdPtr<IOutputStream>& stream,
    const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return false;
    }
    KdTransportRecord& record = records[index];
    {
        KdPtr<IWriteable> writeable;
        if (record.route.ptr) {
            writeable.ptr = static_cast<IWriteable*>(record.route->QueryInterface(&g_iidWriteable));
            if (writeable.ptr) {
                writeable.ptr->AddRef();
            }
        }
        bool hasRoute = writeable.ptr != 0;
        if (stream.ptr->Write(&hasRoute, 1) != 1) {
            return false;
        }
        if (writeable.ptr && !writeable->WriteTo(stream)) {
            return false;
        }
    }
    if (stream.ptr->Write(&record.distance, 4) != 4) {
        return false;
    }
    if (stream.ptr->Write(&record.cell.x, 2) != 2) {
        return false;
    }
    if (!WriteShort(stream, reinterpret_cast<const short*>(&record.cell.y))) {
        return false;
    }
    if (!WriteBool(stream, &record.atRoadside)) {
        return false;
    }
    if (!WriteBool(stream, &record.moving)) {
        return false;
    }
    return true;
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

// Both null, or the same object (IAbstract faces of a then b compared).
static __forceinline bool SameObject(IUnknown* a, IUnknown* b)
{
    void* face;

    if (!a) {
        return !b;
    }
    if (!b) {
        return false;
    }
    face = a->QueryInterface(&g_iidAbstract);
    return face == b->QueryInterface(&g_iidAbstract);
}

// Inferred name. Stack self, record, RET 8. True when another moving
// transport (not parked, not this one, not waiting behind this one) is at
// most lookAhead ahead along the heading, within half the vehicle width of
// the lane and facing within 90 degrees; it becomes the record's blocker.
// The heading is the transport's X axis, or the route start before moving.
// FUNCTION: VC1 0x0049c180
bool KdTransportMover::IsBlocked(KdTransportRecord& record)
{
    KdRouteSample sample;
    const Vector2* heading;
    KdTransportRecord* other;

    if (record.distance <= 0.0f) {
        sample = record.route->GetSampleAt(record.distance);
        heading = &sample.direction;
    }
    else {
        heading = reinterpret_cast<const Vector2*>(&record.transport->GetPosition()._11);
    }
    const float hx = heading->x;
    const float hy = heading->y;
    for (other = records.Begin(); other != records.End(); ++other) {
        if (SameObject(other->transport, record.transport)) {
            continue;
        }
        if (SameObject(other->blocker, record.transport)) {
            continue;
        }
        if (!other->atRoadside && other->moving) {
            const Matrix3& mine = record.transport->GetPosition();
            const Matrix3& its = other->transport->GetPosition();
            const float dx = its._31 - mine._31;
            const float dy = its._32 - mine._32;
            const float range = lookAhead;
            const float halfWidth = vehicleWidth * 0.5f;
            const float reach = halfWidth + range;
            if ((dx < 0.0f ? -dx : dx) <= reach && (dy < 0.0f ? -dy : dy) <= reach) {
                const Matrix3& otherPosition = other->transport->GetPosition();
                const float ox = otherPosition._11;
                const float oy = otherPosition._12;
                const Matrix3& position = record.transport->GetPosition();
                const float ax = position._21;
                const float ay = position._22;
                const float along = dy * hy + dx * hx;
                if (along >= 0.0f && along <= range
                    && FloatAbs(dy * ay + ax * dx) <= halfWidth
                    && ClampedACos(ox * hx + oy * hy) <= kHalfPi) {
                    AssignAbstractFace(record.blocker, other->transport);
                    return true;
                }
            }
        }
    }
    record.blocker.Assign(reinterpret_cast<const KdRef<IAbstract>&>(g_kdTransportMoverNullObject));
    return false;
}

// RET 4. Script "MoveToRoadsite". When the route's end is in another cell and
// the spot beside the current cell is free, the route is replaced by a short
// pull-over: half a cell ahead, cornerOuter - cornerRadius to the right, and
// a tiny step ahead. The transport is then parked and moving.
// FUNCTION: VC1 0x0049c4a0
bool KdTransportMover::MoveToRoadsite(const KdPtr<IPositionable>& transport)
{
    if (!gridMap) {
        return false;
    }
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return false;
    }
    KdTransportRecord& record = records[index];
    if (!record.route || record.route->IsEmpty()) {
        return false;
    }
    const Vector2 end = worldToGrid.MultVrt((record.route->GetPoints()->End() - 1)->position);
    const unsigned short endX = static_cast<unsigned short>(static_cast<int>(end.x));
    const unsigned short endY = static_cast<unsigned short>(static_cast<int>(end.y));
    if (record.cell.x == endX && record.cell.y == endY) {
        return false;
    }

    Vector2 axes[2];
    axes[0].x = record.transport->GetPosition()._11;
    axes[0].y = record.transport->GetPosition()._12;
    const Matrix3& matrix = record.transport->GetPosition();
    axes[1].x = matrix._21;
    axes[1].y = matrix._22;
    if (!CanParkBeside(record.cell, axes)) {
        return false;
    }

    const Matrix3& position = record.transport->GetPosition();
    const Vector2 origin = worldToGrid.MultVrt(*reinterpret_cast<const Vector2*>(&position._31));
    const float shift = cornerOuter - cornerRadius;
    const float sx = shift * axes[1].x;
    const float sy = shift * axes[1].y;
    const float hx = axes[0].x * 0.5f;
    const float hy = axes[0].y * 0.5f;
    Vector2 ahead;
    ahead.x = hx + origin.x;
    ahead.y = origin.y + hy;
    Vector2 aside;
    aside.x = ahead.x + sx;
    aside.y = ahead.y + sy;
    const float tx = static_cast<float>(axes[0].x * 0.0010000000474974513);
    const float ty = static_cast<float>(axes[0].y * 0.0010000000474974513);
    Vector2 stop;
    stop.x = tx + aside.x;
    stop.y = ty + aside.y;

    record.route->Clear();
    record.route->AddPoint(gridToWorld.MultVrt(ahead));
    record.route->AddPoint(gridToWorld.MultVrt(aside));
    record.route->AddPoint(gridToWorld.MultVrt(stop));
    record.distance = 0.0f;
    record.atRoadside = true;
    record.moving = true;
    return true;
}

// RET 4. Script "CanStartMoving". False while another moving transport on the
// road in the same cell faces less than 180 degrees from the route start.
// FUNCTION: VC1 0x0049c710
bool KdTransportMover::CanStartMoving(const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return false;
    }
    KdTransportRecord& record = records[index];
    const KdRouteSample start = record.route->GetSampleAt(0.0f);
    const float dx = start.direction.x;
    const float dy = start.direction.y;
    const float cx = static_cast<float>(record.cell.x);
    const float cy = static_cast<float>(record.cell.y);
    KdTransportRecord* other;

    for (other = records.Begin(); other != records.End(); ++other) {
        if (SameObject(other->transport, record.transport)) {
            continue;
        }
        if (!other->atRoadside && other->moving && static_cast<float>(other->cell.x) == cx
            && static_cast<float>(other->cell.y) == cy) {
            const Matrix3& its = other->transport->GetPosition();
            if (ClampedACos(its._11 * dx + its._12 * dy) < kPi) {
                return false;
            }
        }
    }
    return true;
}

static __forceinline Vector2 ToScreen(const Matrix3& m, float x, float y)
{
    return Vector2(x * m._11 + y * m._21 + m._31, x * m._12 + y * m._22 + m._32);
}

// RET 4. IRenderable vtable+0x10, a debug overlay while visible: per
// transport a green dot at its position, the lookAhead x vehicleWidth box
// IsBlocked checks, and its route in magenta.
// FUNCTION: VC1 0x0049c8c0
int KdTransportMover::Render(void* graphics)
{
    if (visible) {
        const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
        const Matrix3& m = static_cast<ICamera*>(*target.ptr->GetCamera())->GetWorldToScreen();
        KdTransportRecord* record;

        for (record = records.Begin(); record != records.End(); ++record) {
            target.ptr->SetColor(0xff90ee90);
            const Matrix3& position = record->transport->GetPosition();
            const float px = position._31;
            const float py = position._32;
            const Vector2 dot = ToScreen(m, px, py);
            target.ptr->DrawEllipse(static_cast<float>(dot.x - 3.0),
                static_cast<float>(dot.y - 3.0), 6.0f, 6.0f);

            const Matrix3& axisX = record->transport->GetPosition();
            const float xx = axisX._11;
            const float xy = axisX._12;
            const Matrix3& axisY = record->transport->GetPosition();
            const float yx = axisY._21;
            const float yy = axisY._22;
            const float negWidth = -vehicleWidth;
            const float rx = static_cast<float>(negWidth * yx * 0.5);
            const float ry = static_cast<float>(negWidth * yy * 0.5);
            const float rearRightX = rx + px;
            const float rearRightY = ry + py;
            const float range = lookAhead;
            const float frontRightX = range * xx + rearRightX;
            const float frontRightY = range * xy + rearRightY;
            const float width = vehicleWidth;
            const float lx = static_cast<float>(width * yx * 0.5);
            const float ly = static_cast<float>(width * yy * 0.5);
            const float rearLeftX = lx + px;
            const float rearLeftY = ly + py;
            const float frontLeftX = range * xx + rearLeftX;
            const float frontLeftY = range * xy + rearLeftY;

            const Vector2 a = ToScreen(m, rearRightX, rearRightY);
            const Vector2 b = ToScreen(m, rearLeftX, rearLeftY);
            const Vector2 c = ToScreen(m, frontLeftX, frontLeftY);
            const Vector2 d = ToScreen(m, frontRightX, frontRightY);
            target.ptr->DrawLine(a.x, a.y, b.x, b.y);
            target.ptr->DrawLine(b.x, b.y, c.x, c.y);
            target.ptr->DrawLine(c.x, c.y, d.x, d.y);
            target.ptr->DrawLine(d.x, d.y, a.x, a.y);

            if (record->route) {
                target.ptr->SetColor(0xffff00ff);
                KdObjArray<KdRoutePoint>* points = record->route->GetPoints();
                unsigned i;

                for (i = 1; i < points->Count(); ++i) {
                    const Vector2 from = ToScreen(m, (*points)[i - 1].position.x,
                        (*points)[i - 1].position.y);
                    const Vector2 to = ToScreen(m, (*points)[i].position.x,
                        (*points)[i].position.y);
                    target.ptr->DrawLine(from.x, from.y, to.x, to.y);
                }
            }
        }
    }
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
    lookAhead = 50.0f;
    vehicleWidth = 24.0f;
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

// WrapAngle (0x0040ccb0) and fltsign (0x0040cb60) as inlined in InsertArc.
static __forceinline float WrapAngleInline(float angle)
{
    if (angle >= kTwoPi || angle < 0.0f) {
        float turns = static_cast<float>(floor(static_cast<double>(static_cast<float>(angle / kTwoPi))));
        angle = static_cast<float>(angle - turns * kTwoPi);
    }
    return angle;
}

static __forceinline float SignInline(float value)
{
    if (value > 0.0f) {
        return 1.0f;
    }
    if (value < 0.0f) {
        return -1.0f;
    }
    return 0.0f;
}

// Inferred name. EAX=points, stack &at, centre x/y, radius, from and to
// angles, RET 0x18. Inserts the arc's points at at in eighths of the signed
// turn from from towards to, until the remaining turn changes sign; at ends
// after the last inserted point. The insert is InsertCountedBytes
// (0x004b8a20) inlined after a one-element grow.
// FUNCTION: VC1 0x0049d070
static __declspec(noinline) void InsertArc(KdObjArray<Vector2>& points, Vector2*& at, float cx,
    float cy, float radius, float from, float to)
{
    typedef unsigned (__thiscall *FnCount)(void* object);
    float step = WrapAngle(to - from);
    float rest;

    if (step > kPi) {
        step = static_cast<float>(step - kTwoPi);
    }
    step = static_cast<float>(step * 0.125);
    from -= step;
    const float sign = fltsign(step);
    do {
        from += step;
        const float px = static_cast<float>(cos(static_cast<double>(from))) * radius + cx;
        const float py = static_cast<float>(sin(static_cast<double>(from))) * radius + cy;
        const unsigned offset = static_cast<unsigned>(
            reinterpret_cast<unsigned char*>(at)
            - reinterpret_cast<unsigned char*>(points.items.begin));
        {
            const unsigned old = points.items.capacity / sizeof(Vector2);
            Vector2* slot;
            Vector2* end;

            GrowCountedBuffer(&points.items,
                static_cast<int>((points.Count() + 1) * sizeof(Vector2)), 0);
            if (points.items.capacity / sizeof(Vector2) > old) {
                slot = points.Begin() + old;
                end = points.Begin() + points.items.capacity / sizeof(Vector2);
                for (; slot < end; ++slot) {
                    if (slot) {
                        ::new (static_cast<void*>(slot)) Vector2();
                    }
                }
            }
        }
        {
            const unsigned newSize = points.items.size + sizeof(Vector2);
            const unsigned used = reinterpret_cast<FnCount>(
                (*reinterpret_cast<void***>(&points.items))[2])(&points.items);
            unsigned char* begin;

            if (points.items.capacity < used + newSize) {
                GrowCountedBuffer(&points.items, static_cast<int>(newSize), 0);
            }
            begin = reinterpret_cast<unsigned char*>(points.items.begin);
            points.items.size = newSize;
            EraseCountedBytes(begin + offset, begin + newSize - sizeof(Vector2), begin + newSize);
        }
        {
            Vector2* const slot = reinterpret_cast<Vector2*>(
                reinterpret_cast<unsigned char*>(points.items.begin) + offset);
            slot->x = px;
            slot->y = py;
            at = slot + 1;
        }
        rest = WrapAngleInline(to - from);
        if (rest > kPi) {
            rest = static_cast<float>(rest - kTwoPi);
        }
    } while (sign == SignInline(rest));
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
    record.atRoadside = false;
    record.moving = false;
    record.paused = false;
    count = records.Count() + 1;
    if (records.items.capacity < count * sizeof(KdTransportRecord)) {
        records.Reserve(count);
    }
    records.items.size = count * sizeof(KdTransportRecord);
    records[count - 1] = record;
}

// Inferred name. ECX=this, EAX=record. The transport's script
// "OnCellChange"(self, passable): whether its new cell is road.
// FUNCTION: VC1 0x0049dd80
void KdTransportMover::NotifyCellChange(KdTransportRecord& record)
{
    KdPtr<IScriptObject> object(record.transport.ptr
        ? static_cast<IScriptObject*>(record.transport->QueryInterface(&g_iidScriptObject))
        : 0);
    SquirrelObject member;

    object->GetMember(kScriptOnCellChange, &member);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return;
    }
    {
        const bool passable = (passableMask
            & 1 << gridMap->GetCellValue(record.cell.x, record.cell.y)) != 0;
        SquirrelObject self(object);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::PushParam(passable);
        SquirrelVM::EndCall();
    }
}

// Inferred name. EAX=record. The transport's script "OnMovingFinish"(self).
// FUNCTION: VC1 0x0049df10
static void NotifyMovingFinish(KdTransportRecord& record)
{
    KdPtr<IScriptObject> object(record.transport.ptr
        ? static_cast<IScriptObject*>(record.transport->QueryInterface(&g_iidScriptObject))
        : 0);
    SquirrelObject member;

    object->GetMember(kScriptOnMovingFinish, &member);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return;
    }
    {
        SquirrelObject self(object);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::EndCall();
    }
}

// Inferred name. Stack self, dt; EDI=record; RET 8. A moving, unpaused,
// unblocked record advances speed per second along its route; the transport
// is placed on the route turned along it, a new grid cell is reported, and
// at the route end the route is dropped and "OnMovingFinish" called.
// FUNCTION: VC1 0x0049e060
void KdTransportMover::UpdateRecord(KdTransportRecord& record, int dt)
{
    if (!record.route.ptr || record.route->IsEmpty() || !record.moving || record.paused) {
        return;
    }
    if (IsBlocked(record)) {
        return;
    }
    record.distance = static_cast<float>(speed * dt / 1000.0 + record.distance);
    record.distance = MinFloat(record.distance, record.route->GetLength());
    {
        const KdRouteSample sample = record.route->GetSampleAt(record.distance);
        Matrix3 position;
        KdGridCell cell;

        memcpy(&position, &record.transport->GetPosition(), sizeof(position));
        position._31 = sample.position.x;
        position._32 = sample.position.y;
        position._11 = sample.direction.x;
        position._12 = sample.direction.y;
        position._21 = -sample.direction.y;
        position._22 = sample.direction.x;
        record.transport->SetPosition(position);
        {
            const Vector2 grid = worldToGrid.MultVrt(sample.position);
            cell.x = static_cast<unsigned short>(static_cast<int>(grid.x));
            cell.y = static_cast<unsigned short>(static_cast<int>(grid.y));
        }
        if (record.cell != cell) {
            record.cell = cell;
            NotifyCellChange(record);
        }
    }
    if (record.route->GetLength() == record.distance) {
        AssignRouteFace(record.route, g_kdTransportMoverNullObject);
        record.moving = false;
        NotifyMovingFinish(record);
    }
}

// RET 8. IUpdateable vtable+0x14 (ECX=this+8): advances every record.
// FUNCTION: VC1 0x0049e210
void KdTransportMover::Update(unsigned, int dt)
{
    KdTransportRecord* record;

    for (record = records.Begin(); record != records.End(); ++record) {
        UpdateRecord(*record, dt);
    }
}

// Inferred name. ESI=this, stack result, route, fromRoadside; RET 0xc. The
// world-space route a transport drives for a cell route: cell centres moved
// cornerRadius to the right of the travel direction, corners sharper than 45
// degrees replaced by arcs of radius 0.5 - cornerRadius, the start moved
// cornerOuter - cornerRadius further right when leaving a roadside spot, all
// through gridToWorld into a new "Route".
// FUNCTION: VC1 0x0049e260
KdPtr<IRoute> KdTransportMover::BuildDrivingRoute(KdPtr<IRoute> route, bool fromRoadside)
{
    KdObjArray<Vector2> points;
    const KdObjArray<KdRoutePoint>& cells = *route->GetPoints();
    unsigned i;

    for (i = 0; i < cells.Count(); ++i) {
        points.PushBack(Vector2(cells[i].position.x + 0.5f, cells[i].position.y + 0.5f));
    }
    {
        const KdRoutePoint* const c = cells.Begin();
        const float nx = -(c[1].position.y - c[0].position.y);
        const float sy = cornerRadius * (c[1].position.x - c[0].position.x);
        points[0].x = points[0].x + cornerRadius * nx;
        points[0].y = points[0].y + sy;
    }
    for (i = 1; i < cells.Count(); ++i) {
        const float dx = cells[i].position.x - cells[i - 1].position.x;
        const float dy = cells[i].position.y - cells[i - 1].position.y;
        const float nx = -dy;
        const float sx = nx * cornerRadius;
        const float sy = cornerRadius * dx;
        points[i].x = sx + points[i].x;
        points[i].y = points[i].y + sy;
    }
    {
        Vector2 prev = points[0];
        Vector2 cur = points[1];
        Vector2* p = points.Begin() + 2;

        while (p != points.End()) {
            const Vector2 next = *p;
            float dx = cur.x - prev.x;
            float dy = cur.y - prev.y;
            float scale = 1.0f / sqrtf(dx * dx + dy * dy);
            const Vector2 d1(dx * scale, scale * dy);
            dx = next.x - cur.x;
            dy = next.y - cur.y;
            scale = 1.0f / sqrtf(dx * dx + dy * dy);
            const Vector2 d2(dx * scale, scale * dy);
            const float angle = Vector2::AngleBetween(d2, d1);

            if ((angle < 0.0f ? -angle : angle) > kQuarterPi) {
                const float sign = angle > 0.0f ? 1.0f : (angle < 0.0f ? -1.0f : 0.0f);
                const float bx = -d1.x;
                const float by = -d1.y;
                const float r = cornerRadius;
                const float ox = bx * r;
                const float oy = r * by;
                float k;
                float centerX;
                float centerY;
                Vector2* at;

                cur.x = ox * sign + cur.x;
                cur.y = cur.y + oy * sign;
                k = static_cast<float>(0.5 - r);
                centerX = k * bx + cur.x + k * by * sign;
                centerY = cur.y + k * by + k * d1.x * sign;
                EraseCountedBytes(p - 1, p, points.End());
                points.items.size -= sizeof(Vector2);
                at = points.Begin() + (p - 1 - points.Begin());
                InsertArc(points, at, centerX, centerY, static_cast<float>(0.5 - cornerRadius),
                    static_cast<float>(atan2(static_cast<double>(-sign * d1.x), static_cast<double>(-sign * by))),
                    static_cast<float>(atan2(static_cast<double>(d1.y), static_cast<double>(d1.x))));
                p = at;
            }
            prev = cur;
            cur = next;
            ++p;
        }
    }
    if (fromRoadside) {
        Vector2* const first = points.Begin();
        const float dx = first[1].x - first[0].x;
        const float ny = -(first[1].y - first[0].y);
        const float scale = 1.0f / sqrtf(dx * dx + ny * ny);
        const float sx = ny * scale;
        const float sy = scale * dx;
        const float shift = cornerOuter - cornerRadius;
        first[0].x = first[0].x + sx * shift;
        first[0].y = first[0].y + shift * sy;
    }
    {
        KdPtr<IRoute> result;
        void* unknown;
        Vector2* point;

        GetKdFactory()->QueryNamed(&unknown, &g_kdNameRoute);
        if (unknown) {
            result.ptr = static_cast<IRoute*>(
                static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidRoute));
            if (result.ptr) {
                result.ptr->AddRef();
            }
            static_cast<IUnknown*>(unknown)->Release();
        }
        for (point = points.Begin(); point != points.End(); ++point) {
            const Vector2 world(gridToWorld._11 * point->x + point->y * gridToWorld._21
                    + gridToWorld._31,
                point->x * gridToWorld._12 + gridToWorld._22 * point->y + gridToWorld._32);
            result->AddPoint(world);
        }
        return result;
    }
}

// RET 8. Script "ReadTransportFromStream".
// FUNCTION: VC1 0x0049e9d0
bool KdTransportMover::ReadTransportFromStream(const KdPtr<IInputStream>& stream,
    const KdPtr<IPositionable>& transport)
{
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return false;
    }
    KdTransportRecord& record = records[index];
    bool hasRoute;
    if (stream.ptr->Read(&hasRoute, 1) != 1) {
        return false;
    }
    if (hasRoute) {
        {
            KdRef<IUnknown> unknown;
            GetKdFactory()->QueryNamed(reinterpret_cast<void**>(&unknown.ptr), &g_kdNameRoute);
            AssignRouteFace(record.route, unknown);
        }
        KdPtr<IReadable> readable(record.route.AsPtr());
        if (!readable->ReadFrom(stream)) {
            return false;
        }
    }
    if (stream.ptr->Read(&record.distance, 4) != 4) {
        return false;
    }
    if (stream.ptr->Read(&record.cell.x, 2) != 2) {
        return false;
    }
    if (!ReadShort(stream, reinterpret_cast<short*>(&record.cell.y))) {
        return false;
    }
    if (!ReadBool(stream, &record.atRoadside)) {
        return false;
    }
    if (!ReadBool(stream, &record.moving)) {
        return false;
    }
    return true;
}

// The IRoute face of this file's null object: the "no route" result.
static __forceinline IUnknown* NullRoute()
{
    IUnknown* route = 0;

    if (g_kdTransportMoverNullObject.ptr) {
        route = static_cast<IUnknown*>(
            g_kdTransportMoverNullObject->QueryInterface(&g_iidRoute));
        if (route) {
            route->AddRef();
        }
    }
    return route;
}

// A new "Route" from the factory, counted IRoute face.
static __forceinline IRoute* CreateRoute()
{
    void* unknown;
    IRoute* route = 0;

    GetKdFactory()->QueryNamed(&unknown, &g_kdNameRoute);
    if (unknown) {
        route = static_cast<IRoute*>(static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidRoute));
        if (route) {
            route->AddRef();
        }
        static_cast<IUnknown*>(unknown)->Release();
    }
    return route;
}

// RET 0x10. Script "FindPathToArea": the shortest cell route from `from`
// to a cell of area grown by one (corners excluded). Reaching the target
// cell itself wins at once; no path gives the null route.
// FUNCTION: VC1 0x0049eaf0
KdPtr<IUnknown> KdTransportMover::FindPathToArea(const Vector2& from, const Vector2& target,
    const Rect& area)
{
    KdPtr<IUnknown> result;

    if (!gridMap.ptr || !router.ptr) {
        result.ptr = NullRoute();
        return result;
    }
    int best = 0x7fffffff;
    bool found = false;
    KdPtr<IRoute> route;
    route.ptr = CreateRoute();
    router->SetPassableMask(passableMask);
    int x;
    int y;

    for (x = area.x - 1; x <= area.x + area.width; ++x) {
        for (y = area.y - 1; y <= area.y + area.height; ++y) {
            if (x == area.x - 1 && y == area.y - 1) {
                continue;
            }
            if (x == area.x - 1 && y == area.y + area.height) {
                continue;
            }
            if (x == area.x + area.width && y == area.y - 1) {
                continue;
            }
            if (x == area.x + area.width && y == area.y + area.height) {
                continue;
            }
            const Vector2 cell(static_cast<float>(x), static_cast<float>(y));
            const int length = router->FindPath(from, cell, false);
            if (target.x == cell.x && target.y == cell.y) {
                if (length > 0) {
                    route->AssignPoints(router.AsPtr());
                    result.ptr = route.ptr;
                    route.ptr->AddRef();
                    return result;
                }
            }
            else if (length > 0 && length < best) {
                best = length;
                route->AssignPoints(router.AsPtr());
                found = true;
            }
        }
    }
    if (found) {
        result.ptr = route.ptr;
        if (result.ptr) {
            result.ptr->AddRef();
        }
        return result;
    }
    result.ptr = NullRoute();
    return result;
}

// RET 0x14. Script "FindPathBetweenAreas": FindPathToArea from every cell of
// fromArea grown by one (corners excluded), the shortest route by length;
// the route starting at `from` itself wins at once.
// FUNCTION: VC1 0x0049edd0
KdPtr<IUnknown> KdTransportMover::FindPathBetweenAreas(const Vector2& from, const Rect& fromArea,
    const Vector2& to, const Rect& toArea)
{
    KdPtr<IUnknown> best;
    best.ptr = NullRoute();
    int bestLength = 0x7fffffff;
    int x;
    int y;

    for (x = fromArea.x - 1; x <= fromArea.x + fromArea.width; ++x) {
        for (y = fromArea.y - 1; y <= fromArea.y + fromArea.height; ++y) {
            if (x == fromArea.x - 1 && y == fromArea.y - 1) {
                continue;
            }
            if (x == fromArea.x - 1 && y == fromArea.y + fromArea.height) {
                continue;
            }
            if (x == fromArea.x + fromArea.width && y == fromArea.y - 1) {
                continue;
            }
            if (x == fromArea.x + fromArea.width && y == fromArea.y + fromArea.height) {
                continue;
            }
            const Vector2 cell(static_cast<float>(x), static_cast<float>(y));
            KdPtr<IUnknown> path = FindPathToArea(cell, to, toArea);
            if (SameObject(path.ptr, g_kdTransportMoverNullObject.ptr)) {
                continue;
            }
            const int length = static_cast<int>(static_cast<IRoute*>(path.ptr)->GetLength());
            if (from.x == cell.x && from.y == cell.y) {
                return path;
            }
            if (length < bestLength) {
                bestLength = length;
                path.ptr->AddRef();
                if (best.ptr) {
                    best.ptr->Release();
                }
                best.ptr = path.ptr;
            }
        }
    }
    return best;
}

// RET 8. Script "InitMoving": gives the transport the driving route of a
// cell route (BuildDrivingRoute) from its start, stopped and off the
// roadside; the cell is the cell route's first point.
// FUNCTION: VC1 0x0049f020
bool KdTransportMover::InitMoving(const KdPtr<IPositionable>& transport,
    const KdPtr<IRoute>& route)
{
    if (!gridMap.ptr || !route.ptr || route->IsEmpty()) {
        return false;
    }
    const int index = FindTransport(this, transport);
    if (index < 0) {
        return false;
    }
    KdTransportRecord& record = records[index];
    record.distance = 0.0f;
    AssignRoute(record.route, BuildDrivingRoute(route, record.atRoadside));
    record.cell.x = static_cast<unsigned short>(static_cast<int>(route->GetSampleAt(0.0f).position.x));
    record.atRoadside = false;
    record.moving = false;
    record.cell.y = static_cast<unsigned short>(static_cast<int>(route->GetSampleAt(0.0f).position.y));
    return true;
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
