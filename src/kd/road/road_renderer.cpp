#include "kd/road/road_renderer.h"

#include "kd/iface/icamera.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/tile/tile_manager.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004ed694
KdRef<IUnknown> g_kdRoadRendererNullObject;
// GLOBAL: VC1 0x004e9690
KdString g_kdRoadRendererEmptyName = KdEmptyText();

// STRING: VC1 0x004cfd50
static const char kRoadRenderer[] = "RoadRenderer";
// GLOBAL: VC1 0x004d0b0c
const char* g_kdNameRoadRenderer = kRoadRenderer;

// STRING: VC1 0x004d1664
static const char kScriptSetRoadTiles[] = "SetRoadTiles";
// STRING: VC1 0x004d1674
static const char kScriptSetRoadBrokenTiles[] = "SetRoadBrokenTiles";
// STRING: VC1 0x004d1688
static const char kScriptHighlightRoadCells[] = "HighlightRoadCells";
// STRING: VC1 0x004d169c
static const char kScriptSetRouteMarkTile[] = "SetRouteMarkTile";

// Preview colours (ARGB) of a placeable and a blocked road.
static const unsigned kPreviewValid = 0xdddddddd;
static const unsigned kPreviewInvalid = 0xeeff0000;

static inline Vector2 TransformPoint(const Matrix3& m, const Vector2& p)
{
    return Vector2(m._21 * p.y + m._11 * p.x + m._31, m._22 * p.y + m._12 * p.x + m._32);
}

// The world centre of grid cell (x, y).
static inline Vector2 CellCentre(const Matrix3& m, int x, int y)
{
    const Vector2 centre(static_cast<SQFloat>(x + 0.5), static_cast<SQFloat>(y + 0.5));
    return TransformPoint(m, centre);
}

static inline RoadCell* NewRoadCell()
{
    void* const memory = malloc(sizeof(RoadCell));
    return memory ? new (memory) RoadCell() : 0;
}

static inline bool ContainsCoord(const KdArray<CellCoord>& coords, unsigned short x, unsigned short y)
{
    const CellCoord* it = coords.Data();
    const CellCoord* const end = reinterpret_cast<const CellCoord*>(
        reinterpret_cast<const char*>(it) + coords.items.size);

    for (; it != end; ++it) {
        if (it->x == x && it->y == y) {
            break;
        }
    }
    return it != reinterpret_cast<const CellCoord*>(
        reinterpret_cast<const char*>(coords.Data()) + coords.items.size);
}

// Inferred name. ESI=v. Script array of Vector2 instances at idx; sq_get is
// inlined, so a missing slot raises "the index doesn't exist" and is skipped.
// FUNCTION: VC1 0x00406430
KdObjArray<Vector2> Get(SqPlus::TypeWrapper<const KdObjArray<Vector2>&>, HSQUIRRELVM v, int idx)
{
    unsigned i;

    if (idx < 0) {
        idx = sq_gettop(v) + idx + 1;
    }
    KdObjArray<Vector2> result(sq_getsize(v, idx));
    for (i = 0; i < result.Count(); ++i) {
        sq_pushinteger(v, i);
        if (SQ_SUCCEEDED(sq_get(v, idx))) {
            result[i] = *SqPlus::GetInstance<Vector2, true>(v, sq_gettop(v));
            sq_pop(v, 1);
        }
    }
    return result;
}

// IScriptObject+0x14.
// FUNCTION: VC1 0x00406670
const char* KdRoadRenderer::GetTypeName()
{
    return kRoadRenderer;
}

// Script "SetRouteMarkTile".
// FUNCTION: VC1 0x004aa360
void KdRoadRenderer::SetRouteMarkTile(short tile)
{
    routeMarkTile = tile;
}

// Script "SetRenderLayer".
// FUNCTION: VC1 0x004aa370
void KdRoadRenderer::SetRenderLayer(int layer)
{
    renderLayer = layer;
}

// IRenderable+0x14.
// FUNCTION: VC1 0x004aa380
int KdRoadRenderer::GetRenderOrder()
{
    return renderLayer;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004aa390
unsigned char KdRoadRenderer::SetMember(void*, void*)
{
    return 0;
}

// ADD ECX, 0x2c; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004aa430
unsigned KdRoadRenderer::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004aa450
unsigned KdRoadRenderer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IRoadRenderer+0x20, RET 8.
// FUNCTION: VC1 0x004aa460
void KdRoadRenderer::BreakCell(int x, int y)
{
    const int index = width * y + x;
    if (cells[index]) {
        cells[index]->broken = true;
    }
}

// IRenderable+0x0c, RET 8. Renders when the own classificators pass the
// mask (no IClassifiable: only an empty mask passes).
// FUNCTION: VC1 0x004aa490
int KdRoadRenderer::RenderMasked(void* graphics, void* mask)
{
    const RenderMask& filter = *static_cast<const RenderMask*>(mask);
    IUnknown* const self = static_cast<IUnknown*>(
        static_cast<IAbstract*>(this)->QueryInterface(&g_iidAbstract));
    IAbstract* abstract = 0;
    IClassifiable* classifiable = 0;
    bool match;

    if (self) {
        abstract = static_cast<IAbstract*>(self->QueryInterface(&g_iidAbstract));
        if (abstract) {
            abstract->AddRef();
            classifiable = static_cast<IClassifiable*>(abstract->QueryInterface(&g_iidClassifiable));
            if (classifiable) {
                classifiable->AddRef();
            }
        }
    }
    if (classifiable) {
        const unsigned value = classifiable->GetClassificators();
        match = (value & filter.lo) == filter.lo && !(filter.hi & value);
    }
    else {
        match = filter.lo == 0;
    }
    if (classifiable) {
        classifiable->Release();
    }
    if (abstract) {
        abstract->Release();
    }
    if (match) {
        Render(graphics);
    }
    return 0;
}

// IWriteable+0x0c. The number of placed cells, then per cell its index,
// links, visible and broken.
// FUNCTION: VC1 0x004aa550
bool KdRoadRenderer::WriteTo(const KdPtr<IOutputStream>& stream)
{
    int count = 0;
    unsigned i;

    for (i = 0; i < cells.Count(); ++i) {
        if (cells[i]) {
            ++count;
        }
    }
    if (stream.ptr->Write(&count, 4) != 4) {
        return false;
    }
    for (i = 0; i < cells.Count(); ++i) {
        if (cells[i]) {
            if (stream.ptr->Write(&i, 4) != 4) {
                return false;
            }
            if (stream.ptr->Write(&cells[i]->links, 4) != 4) {
                return false;
            }
            if (stream.ptr->Write(&cells[i]->visible, 1) != 1) {
                return false;
            }
            if (stream.ptr->Write(&cells[i]->broken, 1) != 1) {
                return false;
            }
        }
    }
    return true;
}

// Inferred name. ECX=x, ESI=y, EDX=this. 0 outside the grid.
// FUNCTION: VC1 0x004aa680
RoadCell* KdRoadRenderer::GetCell(int x, int y)
{
    if (x >= 0 && y >= 0 && x < width && y < height) {
        return cells[width * y + x];
    }
    return 0;
}

// Inferred name. ESI=this, RET 4. Frees every cell and resizes the grid to
// count empty slots.
// FUNCTION: VC1 0x004aa6b0
void KdRoadRenderer::ResizeCells(unsigned count)
{
    unsigned i;

    for (i = 0; i < cells.Count(); ++i) {
        if (cells[i]) {
            free(cells[i]);
        }
    }
    if (cells.items.capacity < count * 4) {
        GrowCountedBuffer(&cells.items, static_cast<int>(count * 4), 1);
    }
    cells.items.size = count * 4;
    for (i = 0; i < cells.Count(); ++i) {
        cells[i] = 0;
    }
}

// IRoadRenderer+0x28, RET 8. Unlinks the non-broken neighbours.
// FUNCTION: VC1 0x004aa740
void KdRoadRenderer::RemoveCell(int x, int y)
{
    const int index = width * y + x;
    RoadCell* neighbour;

    if (cells[index]) {
        free(cells[index]);
        cells[index] = 0;
        neighbour = GetCell(x - 1, y);
        if (neighbour && !neighbour->broken) {
            neighbour->links &= ~2u;
        }
        neighbour = GetCell(x + 1, y);
        if (neighbour && !neighbour->broken) {
            neighbour->links &= ~1u;
        }
        neighbour = GetCell(x, y - 1);
        if (neighbour && !neighbour->broken) {
            neighbour->links &= ~8u;
        }
        neighbour = GetCell(x, y + 1);
        if (neighbour && !neighbour->broken) {
            neighbour->links &= ~4u;
        }
    }
}

// IRoadRenderer+0x18, RET 4. Takes the size and placement of the grid map
// and starts with an empty grid.
// FUNCTION: VC1 0x004aa860
void KdRoadRenderer::SetGridMap(const KdPtr<IGridMap>& map)
{
    width = map.ptr->GetWidth();
    height = map.ptr->GetHeight();
    transform = map.ptr->GetPosition();
    ResizeCells(width * height);
}

// IReadable+0x0c. Inverse of WriteTo; the cells are placed at their
// centres with the default colour.
// FUNCTION: VC1 0x004aa8c0
bool KdRoadRenderer::ReadFrom(const KdPtr<IInputStream>& stream)
{
    int count;
    int i;

    if (stream.ptr->Read(&count, 4) != 4) {
        return false;
    }
    ResizeCells(width * height);
    for (i = 0; i < count; ++i) {
        unsigned index;
        RoadCell* cell;

        if (stream.ptr->Read(&index, 4) != 4) {
            return false;
        }
        const int x = static_cast<int>(index % static_cast<unsigned>(width));
        const int y = static_cast<int>(index / static_cast<unsigned>(width));
        cell = NewRoadCell();
        cells[index] = cell;
        cell->position = CellCentre(transform, x, y);
        cell->color = 0xffffffff;
        cell->routeMark = false;
        if (stream.ptr->Read(&cell->links, 4) != 4) {
            return false;
        }
        if (stream.ptr->Read(&cell->visible, 1) != 1) {
            return false;
        }
        if (stream.ptr->Read(&cell->broken, 1) != 1) {
            return false;
        }
    }
    return true;
}

// Inferred name. EDI=cell, ESI=x, EDX=this, stack y + strict, RET 8.
// Recomputes the links of a cell and adds the reverse links; a strict link
// needs the neighbour in the same broken state.
// FUNCTION: VC1 0x004aaa40
void KdRoadRenderer::LinkCell(RoadCell* cell, int x, int y, bool strict)
{
    RoadCell* neighbour;

    cell->links = 0;
    neighbour = GetCell(x - 1, y);
    if (neighbour && (neighbour->broken == cell->broken || !strict)) {
        cell->links |= 1;
        neighbour->links |= 2;
    }
    neighbour = GetCell(x + 1, y);
    if (neighbour && (neighbour->broken == cell->broken || !strict)) {
        cell->links |= 2;
        neighbour->links |= 1;
    }
    neighbour = GetCell(x, y - 1);
    if (neighbour && (neighbour->broken == cell->broken || !strict)) {
        cell->links |= 4;
        neighbour->links |= 8;
    }
    neighbour = GetCell(x, y + 1);
    if (neighbour && (neighbour->broken == cell->broken || !strict)) {
        cell->links |= 8;
        neighbour->links |= 4;
    }
}

// Script "SetRoadBrokenTiles", RET 4.
// FUNCTION: VC1 0x004aab60
void KdRoadRenderer::SetRoadBrokenTiles(const KdArray<short>& tiles)
{
    int i;

    for (i = 0; i < 16; ++i) {
        brokenTiles[i] = tiles[i];
    }
}

// Script "SetRoadTiles", RET 4.
// FUNCTION: VC1 0x004aabe0
void KdRoadRenderer::SetRoadTiles(const KdArray<short>& tiles)
{
    int i;

    for (i = 0; i < 16; ++i) {
        roadTiles[i] = tiles[i];
    }
}

// IRoadRenderer+0x30, RET 8; script "HighlightRoadCells". Sets the route
// mark of the cells under the given grid points.
// FUNCTION: VC1 0x004aac60
void KdRoadRenderer::HighlightRoadCells(const KdObjArray<Vector2>& points, bool value)
{
    unsigned i;

    for (i = 0; i < points.Count(); ++i) {
        const Vector2 point = points[i];
        const int y = static_cast<int>(point.y);
        const int x = static_cast<int>(point.x);
        RoadCell* const cell = GetCell(x, y);
        if (cell) {
            cell->routeMark = value;
        }
    }
}

// IRoadRenderer+0x24, RET 8.
// FUNCTION: VC1 0x004aacf0
void KdRoadRenderer::RepairCell(int x, int y)
{
    const int index = width * y + x;
    if (cells[index]) {
        cells[index]->broken = false;
        LinkCell(cells[index], x, y, true);
    }
}

// IRoadRenderer+0x1c, RET 0x10. Places a road on an empty cell.
// FUNCTION: VC1 0x004aad30
void KdRoadRenderer::AddCell(int x, int y, bool visible, bool strict)
{
    const int index = width * y + x;
    RoadCell* cell;

    if (cells[index]) {
        return;
    }
    cell = NewRoadCell();
    cells[index] = cell;
    cell->position = CellCentre(transform, x, y);
    cell->visible = visible;
    cell->color = 0xffffffff;
    cell->routeMark = false;
    cell->broken = false;
    LinkCell(cell, x, y, strict);
}

// IRoadRenderer+0x3c, RET 8 (hidden result). Copies of the placed cells at
// the given coordinates.
// FUNCTION: VC1 0x004aae20
KdObjArray<RoadCell> KdRoadRenderer::GetCells(const KdArray<CellCoord>& coords)
{
    const unsigned total = coords.Count();
    KdObjArray<RoadCell> result;
    unsigned i;

    for (i = 0; i < total; ++i) {
        const CellCoord& coord = coords[i];
        RoadCell* const cell = GetCell(coord.x, coord.y);
        if (cell) {
            const unsigned count = result.Count() + 1;
            const unsigned bytes = count * sizeof(RoadCell);
            if (result.items.capacity < bytes) {
                result.Reserve(count, 0);
            }
            result.items.size = bytes;
            result.End()[-1] = *cell;
        }
    }
    return result;
}

// IRoadRenderer+0x2c, RET 8. Rebuilds the preview cells: one per
// coordinate, linked to the other preview cells and the intact roads.
// FUNCTION: VC1 0x004aaf10
void KdRoadRenderer::SetPreview(const KdArray<CellCoord>& coords, bool valid)
{
    const unsigned count = coords.Count();
    unsigned i;

    if (preview.items.capacity < count * sizeof(RoadCell)) {
        preview.Reserve(count, 1);
    }
    preview.items.size = count * sizeof(RoadCell);
    for (i = 0; i < count; ++i) {
        const CellCoord& coord = coords[i];
        RoadCell& cell = preview[i];
        const unsigned short x = coord.x;
        const unsigned short y = coord.y;
        unsigned short side;
        RoadCell* neighbour;

        cell.position = CellCentre(transform, x, y);
        cell.color = valid ? kPreviewValid : kPreviewInvalid;
        cell.links = 0;

        side = static_cast<unsigned short>(x - 1);
        neighbour = GetCell(side, y);
        if (ContainsCoord(coords, side, y) || (neighbour && !neighbour->broken)) {
            cell.links |= 1;
        }
        side = static_cast<unsigned short>(x + 1);
        neighbour = GetCell(side, y);
        if (ContainsCoord(coords, side, y) || (neighbour && !neighbour->broken)) {
            cell.links |= 2;
        }
        side = static_cast<unsigned short>(y - 1);
        neighbour = GetCell(x, side);
        if (ContainsCoord(coords, x, side) || (neighbour && !neighbour->broken)) {
            cell.links |= 4;
        }
        side = static_cast<unsigned short>(y + 1);
        neighbour = GetCell(x, side);
        if (ContainsCoord(coords, x, side) || (neighbour && !neighbour->broken)) {
            cell.links |= 8;
        }
    }
}

// EAX=this. Frees the cells before the members go.
// FUNCTION: VC1 0x004ab270
KdRoadRenderer::~KdRoadRenderer() throw()
{
    ResizeCells(0);
}

// ESI=this. The tile tables and the grid size stay uninitialized.
// FUNCTION: VC1 0x004ab640
KdRoadRenderer::KdRoadRenderer() throw()
{
}

// RET 4. Returns the new renderer as IAbstract.
// FUNCTION: VC1 0x004ab750
void** CreateKdRoadRenderer(void** out)
{
    void* const memory = malloc(sizeof(KdRoadRenderer));
    KdRoadRenderer* const object = memory ? new (memory) KdRoadRenderer() : 0;
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

// RET 8. The method table (guard 0x004ee5d4); HighlightRoadCells binds the
// IRoadRenderer slot (vcall thunk).
// FUNCTION: VC1 0x004ab7b0
unsigned char KdRoadRenderer::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetRenderLayer] = CreateScriptMethod<KdRoadRenderer>(&KdRoadRenderer::SetRenderLayer);
        methods[kScriptSetRoadTiles] = CreateScriptMethod<KdRoadRenderer>(&KdRoadRenderer::SetRoadTiles);
        methods[kScriptSetRoadBrokenTiles] = CreateScriptMethod<KdRoadRenderer>(
            &KdRoadRenderer::SetRoadBrokenTiles);
        methods[kScriptHighlightRoadCells] = CreateScriptMethod<KdRoadRenderer>(
            static_cast<void (KdRoadRenderer::*)(const KdObjArray<Vector2>&, bool)>(
                &IRoadRenderer::HighlightRoadCells));
        methods[kScriptSetRouteMarkTile] = CreateScriptMethod<KdRoadRenderer>(
            &KdRoadRenderer::SetRouteMarkTile);
    }
    return methods.Find(name, out);
}

// Inferred name. The lazily created "TileManager" component in this file's
// own static (guard 0x004ee770, value 0x004ee76c), returned by reference.
// FUNCTION: VC1 0x004aba70
static KdPtr<ITileManager>& TileManagerRef()
{
    static KdPtr<ITileManager> manager;
    static bool initialized;

    if (!initialized) {
        void* unknown;

        initialized = true;
        GetKdFactory()->QueryNamed(&unknown, &g_kdNameTileManager);
        if (unknown) {
            manager.ptr = static_cast<ITileManager*>(
                static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidTileManager));
            if (manager.ptr) {
                manager.ptr->AddRef();
            }
            static_cast<IUnknown*>(unknown)->Release();
        }
        else {
            manager.ptr = 0;
        }
    }
    return manager;
}

// IRoadRenderer+0x38, RET 8. The route mark tile on every listed cell.
// FUNCTION: VC1 0x004abb00
void KdRoadRenderer::RenderRouteMarks(const KdPtr<IGraphics>& graphics,
    const KdObjArray<RoadCell>& list)
{
    IUnknown** const camera = graphics.ptr->GetCamera();
    if (*camera) {
        KdPtr<ITileManager>& tiles = TileManagerRef();
        const Matrix3& view = static_cast<ICamera*>(*camera)->GetWorldToScreen();
        unsigned i;

        graphics.ptr->SetColor(0xffffffff);
        for (i = 0; i < list.Count(); ++i) {
            const RoadCell& cell = list[i];
            const SQFloat x = view._11 * cell.position.x + cell.position.y * view._21 + view._31;
            const SQFloat y = view._12 * cell.position.x + cell.position.y * view._22 + view._32;
            tiles.ptr->Render(graphics, routeMarkTile, x, y, 0);
        }
    }
}

// IRoadRenderer+0x34, RET 8. The listed cells as intact roads in their
// own colours.
// FUNCTION: VC1 0x004abbe0
void KdRoadRenderer::RenderCells(const KdPtr<IGraphics>& graphics,
    const KdObjArray<RoadCell>& list)
{
    IUnknown** const camera = graphics.ptr->GetCamera();
    if (*camera) {
        KdPtr<ITileManager>& tiles = TileManagerRef();
        const Matrix3& view = static_cast<ICamera*>(*camera)->GetWorldToScreen();
        unsigned i;

        for (i = 0; i < list.Count(); ++i) {
            const RoadCell& cell = list[i];
            const Vector2 point = TransformPoint(view, cell.position);
            graphics.ptr->SetColor(cell.color);
            tiles.ptr->Render(graphics, roadTiles[cell.links], point.x, point.y, 0);
        }
    }
}

// IRenderable+0x10, RET 4. Needs a camera. The visible cells (broken ones
// from the broken table, route marks on top), then the preview.
// FUNCTION: VC1 0x004abcd0
int KdRoadRenderer::Render(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
    IUnknown** const camera = target.ptr->GetCamera();

    if (*camera) {
        KdPtr<ITileManager>& tiles = TileManagerRef();
        const Matrix3& view = static_cast<ICamera*>(*camera)->GetWorldToScreen();
        unsigned i;

        for (i = 0; i < cells.Count(); ++i) {
            RoadCell* const cell = cells[i];
            if (cell && cell->visible) {
                const Vector2 point = TransformPoint(view, cell->position);
                const short tile = cell->broken ? brokenTiles[cell->links] : roadTiles[cell->links];
                target.ptr->SetColor(cell->color);
                tiles.ptr->Render(target, tile, point.x, point.y, 0);
                if (cell->routeMark) {
                    tiles.ptr->Render(target, routeMarkTile, point.x, point.y, 0);
                }
            }
        }
        RenderCells(target, preview);
    }
    return 0;
}

}
