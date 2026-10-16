#include "kd/road/grid_router.h"

#include "kd/iface/icomponent_faces.h"
#include "kd/iface/iid.h"
#include "kd/iface/ipositionable.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <math.h>
#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfd70
static const char kGridRouter[] = "GridRouter";
// GLOBAL: VC1 0x004d0b04
const char* g_kdNameGridRouter = kGridRouter;

// STRING: VC1 0x004d1044
static const char kScriptInitialize[] = "Initialize";
// STRING: VC1 0x004d1050
static const char kScriptFindPath[] = "FindPath";
// STRING: VC1 0x004d105c
static const char kScriptGetPoint[] = "GetPoint";

// The null map Shutdown assigns. Inferred name.
// GLOBAL: VC1 0x004ed678
KdPtr<IUnknown> g_kdNullGridMap;

// Visit results.
enum
{
    kVisitNew = 0,
    kVisitSeen = 1,
    kVisitBlocked = 2,
    kVisitTarget = 3
};

// Most steps of the breadth-first spread, and the frontier limit in bytes.
static const int kMaxSteps = 200;
static const unsigned kMaxFrontierBytes = 800;

// Release the slot, then take the iid face of source (AddRef'd), or null.
template <class T>
static inline void AssignFace(KdRef<T>& slot, IUnknown* source, const char** iid)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(source->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// Empties an array without shrinking it.
static inline void ClearItems(KdFactory& items)
{
    if (items.capacity < items.Count()) {
        GrowCountedBuffer(&items, 0, 1);
    }
    items.size = 0;
}

static inline int Clamp(int value, int last)
{
    if (value >= last) {
        value = last;
    }
    return value < 0 ? 0 : value;
}

// MOV EAX, "GridRouter"; RET. IScriptObject+0x14.
// FUNCTION: VC1 0x00404c10
const char* KdGridRouter::GetTypeName()
{
    return kGridRouter;
}

// IGridRouter+0x0c.
// FUNCTION: VC1 0x004aee60
void KdGridRouter::SetPassableMask(int mask)
{
    passableMask = mask;
}

// XOR AL, AL; RET 8. IScriptObject+0x10: no writable members.
// FUNCTION: VC1 0x004aee70
unsigned char KdGridRouter::SetMember(void*, void*)
{
    return 0;
}

// IGridRouter+0x18.
// FUNCTION: VC1 0x004aeeb0
int KdGridRouter::GetPointCount()
{
    return static_cast<int>(path.items.size >> 3);
}

// IShutdownable+0x0c. Drops the map.
// FUNCTION: VC1 0x004aeec0
int KdGridRouter::Shutdown()
{
    AssignFace(map, g_kdNullGridMap.ptr, &g_iidGridMap);
    return 0;
}

// ADD ECX, 0x0c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004aef50
unsigned KdGridRouter::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004aef70
unsigned KdGridRouter::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. IGridRouter+0x14; script "GetPoint".
// FUNCTION: VC1 0x004aef80
Vector2& KdGridRouter::GetPoint(int index)
{
    return path[index];
}

// Inferred name. ESI=this, EDI=x, EBX=y, RET 4. Marks an unreached passable
// cell with the step distance; kVisitTarget when it is the target cell.
// FUNCTION: VC1 0x004aef90
int KdGridRouter::Visit(int distanceValue, int x, int y)
{
    if (distance[width * y + x] != -1) {
        return kVisitSeen;
    }
    if (!(passableMask & (1 << map->GetCellValue(x, y)))) {
        return kVisitBlocked;
    }
    distance[width * y + x] = distanceValue;
    if (targetX == x && targetY == y) {
        return kVisitTarget;
    }
    return kVisitNew;
}

// RET 4; script "Initialize"(map). Caches the IGridMap face, its size and
// placement, and sizes the distance array to one int per cell.
// FUNCTION: VC1 0x004af000
void KdGridRouter::Initialize(const KdPtr<IAbstract>& source)
{
    AssignFace(map, source.ptr, &g_iidGridMap);
    width = static_cast<short>(map->GetWidth());
    height = static_cast<short>(map->GetHeight());
    {
        KdPtr<IPositionable> positionable(map.ptr
            ? static_cast<IPositionable*>(map.ptr->QueryInterface(&g_iidPositionable))
            : 0);
        transform = positionable->GetPosition();
    }
    inverse = Matrix3::Inverse(transform);
    distance.Resize(width * height);
}

// ESI=this.
// FUNCTION: VC1 0x004af100
KdGridRouter::~KdGridRouter() throw()
{
}

// ESI=this. The frontier arrays start with room for 200 cells.
// FUNCTION: VC1 0x004af1f0
KdGridRouter::KdGridRouter() throw()
    : width(0), height(0), startX(0), startY(0), targetX(0), targetY(0)
{
    GrowCountedBuffer(&frontier.items, static_cast<int>(kMaxFrontierBytes), 1);
    GrowCountedBuffer(&next.items, static_cast<int>(kMaxFrontierBytes), 1);
}

// RET 4. Returns the new router as IAbstract.
// FUNCTION: VC1 0x004af300
void** CreateKdGridRouter(void** out)
{
    void* const memory = malloc(sizeof(KdGridRouter));
    KdGridRouter* const object = memory ? new (memory) KdGridRouter() : 0;
    IUnknown* queried = 0;

    if (object) {
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(object)->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// RET 0xc. IGridRouter+0x10; script "FindPath". from / to are world points
// when worldSpace is set, else cell coordinates. Returns the point count (0:
// no path; both end cells must be passable). The path keeps from and to as
// its ends; the cells in between are their centres in world space.
// FUNCTION: VC1 0x004af360
int KdGridRouter::FindPath(const Vector2& from, const Vector2& to, bool worldSpace)
{
    const Vector2* source;
    const Vector2* target;
    Vector2 localFrom;
    Vector2 localTo;
    Vector2 start;
    Vector2 end;
    bool found;
    int step;
    int length;
    int i;

    if (!map.ptr) {
        return 0;
    }
    ClearItems(path.items);

    source = &from;
    if (worldSpace) {
        localFrom.x = inverse._11 * from.x + inverse._21 * from.y + inverse._31;
        localFrom.y = inverse._12 * from.x + inverse._22 * from.y + inverse._32;
        source = &localFrom;
    }
    start = *source;
    target = &to;
    if (worldSpace) {
        localTo.x = to.x * inverse._11 + inverse._21 * to.y + inverse._31;
        localTo.y = inverse._22 * to.y + inverse._12 * to.x + inverse._32;
        target = &localTo;
    }
    end = *target;

    startX = static_cast<int>(floor(start.x));
    startY = static_cast<int>(floor(start.y));
    targetX = static_cast<int>(floor(end.x));
    targetY = static_cast<int>(floor(end.y));

    if (!(passableMask & (1 << map->GetCellValue(startX, startY)))) {
        return 0;
    }
    if (!(passableMask & (1 << map->GetCellValue(targetX, targetY)))) {
        return 0;
    }

    {
        int* cell = distance.Data();
        int* const last = cell + distance.Count();
        for (; cell != last; ++cell) {
            *cell = -1;
        }
    }
    ClearItems(frontier.items);
    ClearItems(next.items);

    found = Visit(0, startX, startY) == kVisitTarget;
    {
        const int first = width * startY + startX;
        frontier.Append(first);
    }

    for (step = 1; step < kMaxSteps; ++step) {
        unsigned n;

        if (frontier.Count() == 0) {
            break;
        }
        if (found) {
            break;
        }
        ClearItems(next.items);
        for (n = 0; n < frontier.Count(); ++n) {
            const int cell = frontier[n];
            const int y = cell / width;
            const int x = cell % width;
            int direction;

            for (direction = 0; direction < 8; ++direction) {
                int nx;
                int ny;
                int result;

                if (found || (next.items.size & ~3u) >= kMaxFrontierBytes) {
                    break;
                }
                nx = Clamp(direction == 0 ? x - 1 : direction == 1 ? x + 1 : x, width - 1);
                ny = Clamp(direction == 2 ? y - 1 : direction == 3 ? y + 1 : y, height - 1);
                result = Visit(step, nx, ny);
                if (result == kVisitNew) {
                    next.PushBack(width * ny + nx);
                }
                else if (result == kVisitTarget) {
                    found = true;
                }
            }
        }
        {
            FactoryEntry* const begin = frontier.items.begin;
            const unsigned size = frontier.items.size;
            const unsigned capacity = frontier.items.capacity;
            frontier.items.begin = next.items.begin;
            next.items.begin = begin;
            frontier.items.size = next.items.size;
            next.items.size = size;
            frontier.items.capacity = next.items.capacity;
            next.items.capacity = capacity;
        }
    }
    if (!found) {
        return 0;
    }

    length = distance[width * targetY + targetX];
    {
        const unsigned bytes = (length + 1) * sizeof(Vector2);
        if (path.items.capacity < bytes) {
            path.Reserve(length + 1);
        }
        path.items.size = bytes;
    }
    for (i = length; i >= 0; --i) {
        Vector2* const points = path.Begin();
        int direction;

        if (i == 0) {
            points[0] = from;
        }
        else if (!worldSpace) {
            points[i].x = static_cast<SQFloat>(targetX);
            points[i].y = static_cast<SQFloat>(targetY);
        }
        else if (i == length) {
            points[i] = to;
        }
        else {
            const SQFloat cx = static_cast<SQFloat>(targetX) + 0.5f;
            const SQFloat cy = static_cast<SQFloat>(targetY) + 0.5f;
            points[i].x = cy * transform._21 + cx * transform._11 + transform._31;
            points[i].y = cy * transform._22 + transform._12 * cx + transform._32;
        }
        if (i - 1 < 0) {
            break;
        }
        for (direction = 0; direction < 4; ++direction) {
            const int nx = Clamp(direction == 0 ? targetX - 1
                : direction == 1 ? targetX + 1 : targetX, width - 1);
            const int ny = Clamp(direction == 2 ? targetY - 1
                : direction == 3 ? targetY + 1 : targetY, height - 1);
            if (distance[width * ny + nx] == i - 1) {
                targetX = nx;
                targetY = ny;
                break;
            }
        }
    }
    return static_cast<int>(path.items.size >> 3);
}

// RET 8. The method table (table 0x004ee6b4, guard bit 0x004ee6c4, atexit
// destructor 0x004c9b50); there is no script instance. FindPath and
// GetPoint bind the IGridRouter slots (vcall thunks).
// FUNCTION: VC1 0x004afb30
unsigned char KdGridRouter::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptInitialize] = CreateScriptMethod<KdGridRouter>(&KdGridRouter::Initialize);
        methods[kScriptFindPath] = CreateScriptMethod<KdGridRouter>(
            static_cast<int (KdGridRouter::*)(const Vector2&, const Vector2&, bool)>(
                &IGridRouter::FindPath));
        methods[kScriptGetPoint] = CreateScriptMethod<KdGridRouter>(
            static_cast<Vector2& (KdGridRouter::*)(int)>(&IGridRouter::GetPoint));
    }
    return methods.Find(name, out);
}

}
