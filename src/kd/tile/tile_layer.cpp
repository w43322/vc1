#include "kd/tile/tile_layer.h"

#include "kd/group/ptr_set.h"
#include "kd/iface/icamera.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/iface/itile_object.h"
#include "kd/tile/tile_manager.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004e96b0
KdString g_kdTileLayerEmptyName = KdEmptyText();

// STRING: VC1 0x004cfd90
static const char kTileLayer[] = "TileLayer";
// GLOBAL: VC1 0x004d0af8
const char* g_kdNameTileLayer = kTileLayer;

// STRING: VC1 0x004d1e0c
static const char kScriptReadTiles[] = "ReadTiles";
// STRING: VC1 0x004d1e28
static const char kScriptSetBackTile[] = "SetBackTile";
// STRING: VC1 0x004d1e34
static const char kScriptSetBackParallax[] = "SetBackParallax";
// STRING: VC1 0x004d1e58
static const char kScriptClearObjects[] = "ClearObjects";

// The null KdPtr<ITileObject> Add/Remove compare against. Inferred name.
// GLOBAL: VC1 0x004ed6a0
KdPtr<IUnknown> g_kdNullTileObject;

// Side of one KdTileGrid cell in pixels.
static const int kTileGridCell = 128;

static inline Vector2 TransformPoint(const Matrix3& m, float x, float y)
{
    return Vector2(y * m._21 + x * m._11 + m._31, y * m._22 + x * m._12 + m._32);
}

// Inferred name. EDI=out. The "TileManager" component, created on first use
// and kept in a function-local static (guard 0x004ed83c, value 0x004ed840).
// FUNCTION: VC1 0x004099c0
static KdPtr<ITileManager> TileManager()
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

// IScriptObject+0x14.
// FUNCTION: VC1 0x00409a60
const char* KdTileLayer::GetTypeName()
{
    return kTileLayer;
}

// EAX=this.
// FUNCTION: VC1 0x00409a90
KdTileGrid::~KdTileGrid() throw()
{
}

static int* IntLowerBound(int* first, int* last, const int& key);

// Inferred name. EAX=set, stdcall out + value, RET 8. Inserts value into a
// KdArray<int> kept sorted by unsigned value; an existing value is kept.
// FUNCTION: VC1 0x00409bb0
static KdIntSetInsert IntSetInsert(KdArray<int>* set, const int& value)
{
    KdIntSetInsert result;
    const unsigned size = set->items.size;
    int* const begin = set->Data();
    int* const end = reinterpret_cast<int*>(reinterpret_cast<char*>(begin) + size);
    int* const found = IntLowerBound(begin, end, value);
    int* slot;

    if (found != end && !(static_cast<unsigned>(value) < static_cast<unsigned>(*found))) {
        result.position = found;
        result.inserted = 0;
        return result;
    }
    GrowCountedBuffer(&set->items, static_cast<int>((size >> 2) * 4 + 4), 0);
    slot = static_cast<int*>(InsertCountedBytes(&set->items,
        reinterpret_cast<char*>(set->items.begin) - reinterpret_cast<char*>(begin)
            + reinterpret_cast<char*>(found), 4));
    *slot = value;
    result.position = slot;
    result.inserted = 1;
    return result;
}


// Inferred name. Insertion sort of tile objects by GetDepth(camera); equal
// depths keep their order. The hole is closed with the byte rotate.
// FUNCTION: VC1 0x00409ee0
static void SortByDepth(ITileObject** first, ITileObject** last, IUnknown** camera)
{
    ITileObject** current;

    for (current = first + 1; current < last; ++current) {
        ITileObject* const object = *current;
        ITileObject** slot = current;

        while (slot > first) {
            const double before = slot[-1]->GetDepth(camera);
            if (!(object->GetDepth(camera) < before)) {
                break;
            }
            --slot;
        }
        EraseCountedBytes(slot, current, current + 1);
    }
}

// Inferred name. EAX=first, cdecl last + value. Assigns value to every cell.
// FUNCTION: VC1 0x00409f50
static void FillCells(KdArray<int>* first, KdArray<int>* last, const KdArray<int>& value)
{
    for (; first != last; ++first) {
        first->Assign(value);
    }
}

// Inferred name. ECX=first, EDX=last, EAX=key. Unsigned lower bound.
// FUNCTION: VC1 0x0040a010
static int* IntLowerBound(int* first, int* last, const int& key)
{
    while (first != last) {
        int* const middle = first + (last - first) / 2;
        if (static_cast<unsigned>(*middle) < static_cast<unsigned>(key)) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// Script "SetBackParallax".
// FUNCTION: VC1 0x004a26f0
void KdTileLayer::SetBackParallax(int shift)
{
    backParallax = shift;
}

// Script "SetBackTile".
// FUNCTION: VC1 0x004a2700
void KdTileLayer::SetBackTile(int tile)
{
    backTile = static_cast<short>(tile);
}

// Script "SetRenderLayer".
// FUNCTION: VC1 0x004a2710
void KdTileLayer::SetRenderLayer(int layer)
{
    renderLayer = layer;
}

// IRenderable+0x14; script "GetRenderLayer".
// FUNCTION: VC1 0x004a2720
int KdTileLayer::GetRenderOrder()
{
    return renderLayer;
}

// ITileLayer+0x0c.
// FUNCTION: VC1 0x004a2730
KdArray<Tile>& KdTileLayer::GetTiles()
{
    return tiles;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004a2740
unsigned char KdTileLayer::SetMember(void*, void*)
{
    return 0;
}

// ADD ECX, 0x34; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004a2800
unsigned KdTileLayer::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a2820
unsigned KdTileLayer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IFunctionalGroup+0x14; script "ClearObjects".
// FUNCTION: VC1 0x004a2830
void KdTileLayer::Clear()
{
    RefClear(&objects);
}

// IWriteable+0x0c. A 16-bit tile count, then id, x, y of every tile.
// FUNCTION: VC1 0x004a2860
bool KdTileLayer::WriteTo(const KdPtr<IOutputStream>& stream)
{
    unsigned short count = static_cast<unsigned short>(tiles.Count());
    unsigned i;

    if (stream.ptr->Write(&count, 2) != 2) {
        return false;
    }
    for (i = 0; i < tiles.Count(); ++i) {
        Tile& tile = tiles[i];
        if (stream.ptr->Write(&tile.id, 2) != 2) {
            return false;
        }
        if (stream.ptr->Write(&tile.x, 2) != 2) {
            return false;
        }
        if (stream.ptr->Write(&tile.y, 2) != 2) {
            return false;
        }
    }
    return true;
}

// IRenderable+0x0c, RET 8. When the layer's own classificators pass the mask
// it renders itself; otherwise only its tile objects get the masked pass,
// in depth order and only with a camera.
// FUNCTION: VC1 0x004a2990
int KdTileLayer::RenderMasked(void* graphics, void* mask)
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
    else {
        const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
        IUnknown** const camera = target.ptr->GetCamera();
        if (*camera) {
            unsigned i;

            if (objects.items.size != 0) {
                SortByDepth(reinterpret_cast<ITileObject**>(RefBegin(&objects)),
                    reinterpret_cast<ITileObject**>(RefEnd(&objects)), camera);
            }
            for (i = 0; i < objects.items.size >> 2; ++i) {
                static_cast<ITileObject*>(RefBegin(&objects)[i])->RenderMasked(graphics, mask);
            }
        }
    }
    return 0;
}

// IFunctionalGroup+0x10; script "UnregisterObject". Drops every slot holding
// the object's ITileObject face.
// FUNCTION: VC1 0x004a2ad0
void KdTileLayer::Remove(const KdPtr<IAbstract>& object)
{
    IUnknown* const source = object.ptr;
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidTileObject)) : 0);

    if (!PtrEqual(g_kdNullTileObject.ptr, face.ptr)) {
        IUnknown** const first = RefBegin(&objects);
        IUnknown** const last = RefEnd(&objects);
        IUnknown** const kept = PtrRemove(first, last, &face.ptr);
        const unsigned removed = static_cast<unsigned>(last - kept) * 4;
        EraseCountedBytes(kept, kept + (last - kept), RefEnd(&objects));
        objects.items.size -= removed;
    }
}

// IFunctionalGroup+0x0c; script "RegisterObject". Keeps tile objects of
// this layer's render order; always reports success.
// FUNCTION: VC1 0x004a2bb0
unsigned char KdTileLayer::Add(const KdPtr<IAbstract>& object)
{
    IUnknown* const source = object.ptr;
    KdPtr<IUnknown> face(source
        ? static_cast<IUnknown*>(source->QueryInterface(&g_iidTileObject)) : 0);

    if (!PtrEqual(g_kdNullTileObject.ptr, face.ptr)
        && static_cast<ITileObject*>(face.ptr)->GetRenderOrder()
            == static_cast<IRenderable*>(this)->GetRenderOrder()) {
        PtrListAppend(&objects, &face.ptr);
    }
    return 1;
}

// ESI=this.
// FUNCTION: VC1 0x004a2c70
KdTileLayer::~KdTileLayer() throw()
{
}

// Inferred name. Stack this, left, top, bottom; EAX=right. Collects the
// tiles of every cell that overlaps the world rect into visible.
// FUNCTION: VC1 0x004a2d40
KdArray<int>* KdTileGrid::Query(int left, int top, int right, int bottom)
{
    int firstColumn;
    int lastColumn;
    int firstRow;
    int lastRow;
    int column;
    int value;

    if (visible.items.capacity < visible.items.Count()) {
        GrowCountedBuffer(&visible.items, 0, 1);
    }
    visible.items.size = 0;

    value = left - bounds.x;
    firstColumn = (value - value % kTileGridCell) / kTileGridCell;
    value = right - bounds.x - 1;
    lastColumn = (value - value % kTileGridCell) / kTileGridCell + 1;
    value = top - bounds.y;
    firstRow = (value - value % kTileGridCell) / kTileGridCell;
    value = bottom + (-1 - bounds.y);
    lastRow = (value - value % kTileGridCell) / kTileGridCell + 1;
    if (firstColumn < 0) {
        firstColumn = 0;
    }
    if (columns < lastColumn) {
        lastColumn = columns;
    }
    if (firstRow < 0) {
        firstRow = 0;
    }
    if (rows < lastRow) {
        lastRow = rows;
    }
    for (column = firstColumn; column < lastColumn; ++column) {
        int row;
        for (row = firstRow; row < lastRow; ++row) {
            const KdArray<int>& cell = cells[columns * row + column];
            const int* from = cell.Data();
            const int* const to = reinterpret_cast<const int*>(
                reinterpret_cast<const char*>(from) + cell.items.size);
            const unsigned bytes = (static_cast<unsigned>(to - from) + visible.Count()) * 4
                + visible.items.Count();

            if (visible.items.capacity < bytes) {
                GrowCountedBuffer(&visible.items, static_cast<int>(bytes), 1);
            }
            for (; from < to; ++from) {
                IntSetInsert(&visible, *from);
            }
        }
    }
    return &visible;
}

// ESI=this.
// FUNCTION: VC1 0x004a3410
KdTileLayer::KdTileLayer() throw()
{
    renderLayer = 0;
    backParallax = 0;
    backTile = -1;
}

// RET 4. Returns the new layer as IAbstract.
// FUNCTION: VC1 0x004a35a0
void** CreateKdTileLayer(void** out)
{
    void* const memory = malloc(sizeof(KdTileLayer));
    KdTileLayer* const object = memory ? new (memory) KdTileLayer() : 0;
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

// Inferred name. Same lazily created "TileManager" as TileManager(), in its
// own static (guard 0x004ee768, value 0x004ee764), returned by reference.
// FUNCTION: VC1 0x004a3600
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

// IRenderable+0x10, RET 4. Needs a camera. Maps the clip rect into the world
// through the camera's +0x2c matrix, tiles the back tile over it (its origin
// scrolled by the view origin >> backParallax), then draws the visible tiles
// in grid order, each preceded by the tile objects whose depth is below the
// tile's y; the remaining objects come last.
// FUNCTION: VC1 0x004a3690
int KdTileLayer::Render(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
    IUnknown** const camera = target.ptr->GetCamera();

    if (*camera) {
        const Rect& clip = target.ptr->GetClipRect();
        const Matrix3& screen = static_cast<ICamera*>(*target.ptr->GetCamera())->GetUIToScreen();
        const Vector2 topLeft = TransformPoint(screen,
            static_cast<float>(clip.x), static_cast<float>(clip.y));
        const Vector2 bottomRight = TransformPoint(screen,
            static_cast<float>(clip.x + clip.width), static_cast<float>(clip.y + clip.height));
        const int left = static_cast<int>(topLeft.x);
        const int top = static_cast<int>(topLeft.y);
        const int right = static_cast<int>(bottomRight.x);
        const int bottom = static_cast<int>(bottomRight.y);
        ITileObject** object;
        KdArray<int>* visible;
        int* tile;

        if (backTile != -1) {
            const Rect back = TileManagerRef().ptr->GetRect(backTile, 0);
            int offsetX = (left >> backParallax) % back.width;
            int offsetY = (top >> backParallax) % back.height;
            int startX;
            int endX;
            int endY;
            int y;

            if (offsetX < 0) {
                offsetX += back.width;
            }
            if (offsetY < 0) {
                offsetY += back.height;
            }
            startX = left - offsetX;
            endX = startX - left + back.width + right;
            y = top - offsetY;
            endY = y - top + back.height + bottom;
            for (; y < endY; y += back.height) {
                int x;
                for (x = startX; x < endX; x += back.width) {
                    TileManagerRef().ptr->Render(target, backTile,
                        static_cast<float>(x), static_cast<float>(y), 0);
                }
            }
        }

        if (objects.items.size != 0) {
            SortByDepth(reinterpret_cast<ITileObject**>(RefBegin(&objects)),
                reinterpret_cast<ITileObject**>(RefEnd(&objects)), camera);
        }
        object = reinterpret_cast<ITileObject**>(RefBegin(&objects));
        {
            KdPtr<ITileManager>& manager = TileManagerRef();
            visible = grid.Query(left, top, right, bottom);
            for (tile = visible->Data();
                 tile != reinterpret_cast<int*>(
                     reinterpret_cast<char*>(visible->items.begin) + visible->items.size);
                 ++tile) {
                const Tile* const placed = reinterpret_cast<const Tile*>(*tile);

                while (object != reinterpret_cast<ITileObject**>(RefEnd(&objects))
                    && (*object)->GetDepth(camera) < static_cast<float>(placed->y)) {
                    (*object)->Render(graphics);
                    ++object;
                }
                target.ptr->SetColor(0xffffffff);
                manager.ptr->Render(target, placed->id,
                    static_cast<float>(placed->x), static_cast<float>(placed->y), 0);
            }
        }
        for (; object != reinterpret_cast<ITileObject**>(RefEnd(&objects)); ++object) {
            (*object)->Render(graphics);
        }
    }
    return 0;
}

// Inferred name. EBX=this, stack tiles, RET 4. Rebuilds the grid: bounds
// becomes the union of the tile rects (tile manager rect of id offset by
// x, y), then every tile is filed into each cell its rect touches.
// FUNCTION: VC1 0x004a3a30
void KdTileGrid::Build(const KdArray<Tile>& tiles)
{
    columns = 0;
    rows = 0;
    if (cells.items.capacity < cells.items.Count()) {
        GrowCountedBuffer(&cells.items, 0, 1);
    }
    cells.items.size = 0;
    if (tiles.items.size != 0) {
        KdObjArray<Rect> rects;
        {
            const unsigned bytes = tiles.Count() * sizeof(Rect);
            if (rects.items.capacity < bytes) {
                rects.Reserve(bytes / sizeof(Rect), 1);
            }
            rects.items.size = bytes;
        }
        const Tile* const first = tiles.Data();
        unsigned i;
        unsigned count;

        {
            Rect rect = TileManager().ptr->GetRect(first->id, 0);
            rect.x += first->x;
            rect.y += first->y;
            bounds = rect;
        }
        for (i = 0; i < tiles.Count(); ++i) {
            const Tile& tile = tiles[i];
            Rect& slot = rects[i];
            Rect rect = TileManager().ptr->GetRect(tile.id, 0);
            rect.x += tile.x;
            rect.y += tile.y;
            slot = rect;
            bounds.Union(slot);
        }
        rows = (bounds.height + kTileGridCell - 1) / kTileGridCell;
        columns = (bounds.width + kTileGridCell - 1) / kTileGridCell;
        count = rows * columns;
        if (cells.items.capacity < count * sizeof(KdArray<int>)) {
            cells.Reserve(count);
        }
        cells.items.size = count * sizeof(KdArray<int>);
        {
            KdArray<int> empty;
            FillCells(cells.Begin(), cells.End(), empty);
        }
        for (i = 0; i < rects.Count(); ++i) {
            Rect& rect = rects[i];
            int column;

            rect.x -= bounds.x;
            rect.y -= bounds.y;
            for (column = rect.x / kTileGridCell;
                 column <= (rect.x - 1 + rect.width) / kTileGridCell; ++column) {
                int row;
                for (row = rect.y / kTileGridCell;
                     row <= (rect.y - 1 + rect.height) / kTileGridCell; ++row) {
                    const int placed = reinterpret_cast<int>(tiles.Data() + i);
                    IntSetInsert(&cells[columns * row + column], placed);
                }
            }
        }
    }
}

// Script "ReadTiles": count tiles of id, x, y (16 bits each) from the
// stream's IInputStream face, then the grid rebuild. False on a short read.
// FUNCTION: VC1 0x004a3db0
bool KdTileLayer::ReadTiles(const KdPtr<IAbstract>& stream, int count)
{
    const unsigned bytes = static_cast<unsigned>(count) * sizeof(Tile);
    unsigned i;

    if (tiles.items.capacity < bytes) {
        GrowCountedBuffer(&tiles.items, static_cast<int>(bytes), 1);
    }
    tiles.items.size = bytes;
    for (i = 0; i < tiles.Count(); ++i) {
        Tile& tile = tiles[i];
        if (KdPtr<IInputStream>(stream).ptr->Read(&tile.id, 2) != 2
            || KdPtr<IInputStream>(stream).ptr->Read(&tile.x, 2) != 2
            || KdPtr<IInputStream>(stream).ptr->Read(&tile.y, 2) != 2) {
            return false;
        }
    }
    grid.Build(tiles);
    return true;
}

// IReadable+0x0c. A 16-bit tile count, then ReadTiles.
// FUNCTION: VC1 0x004a3f80
bool KdTileLayer::ReadFrom(const KdPtr<IInputStream>& stream)
{
    short count;

    if (stream.ptr->Read(&count, 2) != 2) {
        return false;
    }
    return ReadTiles(KdPtr<IAbstract>(stream), count);
}

// ITileLayer+0x1c.
// FUNCTION: VC1 0x004a4000
void KdTileLayer::SetTileId(int index, short id)
{
    tiles[index].id = id;
    grid.Build(tiles);
}

// ITileLayer+0x18.
// FUNCTION: VC1 0x004a4030
void KdTileLayer::RemoveTile(int index)
{
    Tile* const slot = &tiles[index];
    EraseCountedBytes(slot, slot + 1, tiles.Data() + tiles.Count());
    tiles.items.size -= sizeof(Tile);
    grid.Build(tiles);
}

// ITileLayer+0x14.
// FUNCTION: VC1 0x004a4070
void KdTileLayer::AppendTile(const Tile& tile)
{
    const unsigned bytes = (tiles.Count() + 1) * sizeof(Tile);
    Tile* slot;

    if (tiles.items.capacity < bytes) {
        GrowCountedBuffer(&tiles.items, static_cast<int>(bytes), 0);
    }
    tiles.items.size = bytes;
    slot = reinterpret_cast<Tile*>(reinterpret_cast<char*>(tiles.items.begin) + bytes) - 1;
    *slot = tile;
    grid.Build(tiles);
}

// ITileLayer+0x10. Inserts before the first tile whose y is not smaller.
// FUNCTION: VC1 0x004a40d0
void KdTileLayer::InsertTile(const Tile& tile)
{
    Tile* position = tiles.Data();
    Tile* const begin = position;
    Tile* slot;

    if (tile.y > position->y) {
        Tile* const end = reinterpret_cast<Tile*>(
            reinterpret_cast<char*>(tiles.items.begin) + tiles.items.size);
        while (position != end) {
            ++position;
            if (!(tile.y > position->y)) {
                break;
            }
        }
    }
    GrowCountedBuffer(&tiles.items, static_cast<int>((tiles.Count() + 1) * sizeof(Tile)), 0);
    slot = static_cast<Tile*>(InsertCountedBytes(&tiles.items,
        reinterpret_cast<char*>(tiles.items.begin) - reinterpret_cast<char*>(begin)
            + reinterpret_cast<char*>(position), sizeof(Tile)));
    *slot = tile;
    grid.Build(tiles);
}

// RET 8. The method table (table 0x004ee7b8, guard bit 0x004ee7c8).
// GetRenderLayer and the object methods bind the IRenderable /
// IFunctionalGroup slots (vcall thunks).
// FUNCTION: VC1 0x004a4150
unsigned char KdTileLayer::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptReadTiles] = CreateScriptMethod<KdTileLayer>(&KdTileLayer::ReadTiles);
        methods[kScriptGetRenderLayer] = CreateScriptMethod<KdTileLayer>(
            static_cast<int (KdTileLayer::*)()>(&IRenderable::GetRenderOrder));
        methods[kScriptSetRenderLayer] = CreateScriptMethod<KdTileLayer>(&KdTileLayer::SetRenderLayer);
        methods[kScriptSetBackTile] = CreateScriptMethod<KdTileLayer>(&KdTileLayer::SetBackTile);
        methods[kScriptSetBackParallax] = CreateScriptMethod<KdTileLayer>(&KdTileLayer::SetBackParallax);
        methods[kScriptRegisterObject] = CreateScriptMethod<KdTileLayer>(
            static_cast<unsigned char (KdTileLayer::*)(const KdPtr<IAbstract>&)>(&IFunctionalGroup::Add));
        methods[kScriptUnregisterObject] = CreateScriptMethod<KdTileLayer>(
            static_cast<void (KdTileLayer::*)(const KdPtr<IAbstract>&)>(&IFunctionalGroup::Remove));
        methods[kScriptClearObjects] = CreateScriptMethod<KdTileLayer>(
            static_cast<void (KdTileLayer::*)()>(&IFunctionalGroup::Clear));
    }
    return methods.Find(name, out);
}

}
