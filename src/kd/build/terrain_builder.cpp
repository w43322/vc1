#include "kd/build/terrain_builder.h"

#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "app/host_api.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "sqplus/SquirrelVM.h"

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>

namespace SqPlus {
template<> void Push(HSQUIRRELVM v, vc1::KdObjArray<vc1::Vector2>& value);
}

namespace vc1 {

// STRING: VC1 0x004cfd60
static const char kTerrainBuilder[] = "TerrainBuilder";
// GLOBAL: VC1 0x004d0b08
const char* g_kdNameTerrainBuilder = kTerrainBuilder;

extern const char kScriptSetCamera[]; // system.cpp

// GLOBAL: VC1 0x004cf8bc
extern const char kScriptSetGridMap[] = "SetGridMap";
// GLOBAL: VC1 0x004d19ec
extern const char kScriptSetGridRouter[] = "SetGridRouter";
// STRING: VC1 0x004d19fc
static const char kScriptSetRenderer[] = "SetRenderer";
// STRING: VC1 0x004d1a08
static const char kScriptSetForestLayer[] = "SetForestLayer";
// STRING: VC1 0x004d1a18
static const char kScriptSetForestTiles[] = "SetForestTiles";
// STRING: VC1 0x004d1a28
static const char kScriptSetForestCuttedTiles[] = "SetForestCuttedTiles";
// STRING: VC1 0x004d1a40
static const char kScriptSetForestBurntTiles[] = "SetForestBurntTiles";
// STRING: VC1 0x004d1a54
static const char kScriptSetMode[] = "SetMode";
// STRING: VC1 0x004d1a5c
static const char kScriptGetCellCount[] = "GetCellCount";
// STRING: VC1 0x004d1a6c
static const char kScriptGetForestLevel[] = "GetForestLevel";
// STRING: VC1 0x004d1a7c
static const char kScriptAddRoadCell[] = "AddRoadCell";
// STRING: VC1 0x004d1a88
static const char kScriptBreakRoadCell[] = "BreakRoadCell";
// STRING: VC1 0x004d1a98
static const char kScriptRemoveRoadCell[] = "RemoveRoadCell";
// STRING: VC1 0x004d1aa8
static const char kScriptSetWasteLayer[] = "SetWasteLayer";
// STRING: VC1 0x004d1ab8
static const char kScriptSetWasteTiles[] = "SetWasteTiles";
// STRING: VC1 0x004d1ac8
static const char kScriptAddTerrainTile[] = "AddTerrainTile";
// STRING: VC1 0x004d1ad8
static const char kScriptRemoveTiles[] = "RemoveTiles";
// STRING: VC1 0x004d1ae4
static const char kScriptReplaceTiles[] = "ReplaceTiles";
// STRING: VC1 0x004d1af4
static const char kScriptSetDraftToRenderer[] = "SetDraftToRenderer";
// STRING: VC1 0x004d1b08
static const char kScriptModeDefault[] = "MODE_Default";
// STRING: VC1 0x004d1b18
static const char kScriptModeBuildRoad[] = "MODE_BuildRoad";
// STRING: VC1 0x004d1b28
static const char kScriptModeBuildForest[] = "MODE_BuildForest";
// STRING: VC1 0x004d1b3c
static const char kScriptModeDestroy[] = "MODE_Destroy";
// STRING: VC1 0x004d1b4c
static const char kScriptOnCellHovered[] = "OnCellHovered";
// STRING: VC1 0x004d1b5c
static const char kScriptOnBuildRoad[] = "OnBuildRoad";
// STRING: VC1 0x004d1b68
static const char kScriptOnBuildForest[] = "OnBuildForest";
// STRING: VC1 0x004d1b78
static const char kScriptOnDestroyRoad[] = "OnDestroyRoad";
// STRING: VC1 0x004d1b88
static const char kScriptOnDestroyRoadBroken[] = "OnDestroyRoadBroken";
// STRING: VC1 0x004d1b9c
static const char kScriptOnDestroyForest[] = "OnDestroyForest";
// STRING: VC1 0x004d1bac
static const char kScriptOnDestroyForestBurnt[] = "OnDestroyForestBurnt";
// STRING: VC1 0x004d1bc4
static const char kScriptOnDestroyForestCutted[] = "OnDestroyForestCutted";
// STRING: VC1 0x004d1bdc
static const char kScriptOnDestroyWaste[] = "OnDestroyWaste";
// STRING: VC1 0x004d1bec
static const char kScriptOnChangeRoads[] = "OnChangeRoads";
// STRING: VC1 0x004d1bfc
static const char kScriptCanBuildRoad[] = "CanBuildRoad";

// Cell tints of CellColor (ARGB).
static const unsigned kCellBlocked = 0x55000000;
static const unsigned kCellPlantable = 0x5500ff00;
static const unsigned kCellDestroyable = 0x55ffffff;
static const unsigned kCellClear = 0x00ffffff;

// The same as a script value, through SquirrelObject::Set<KdPtr<IUnknown> >
// (0x004014a0).
static __forceinline SquirrelObject ScriptSelf(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return SquirrelObject((KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0)));
}

static inline Vector2 CellPoint(const CellCoord& cell)
{
    return Vector2(static_cast<SQFloat>(cell.x), static_cast<SQFloat>(cell.y));
}

// Release slot, then the T face of the engine's null object (counted), or 0.
template <class T>
static inline void ResetToNullObject(KdRef<T>& slot)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!g_kdNullObject.ptr) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(g_kdNullObject.ptr->QueryInterface(KdIid<T>::Get()));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// MOV EAX, "TerrainBuilder"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00407620
const char* KdTerrainBuilder::GetTypeName()
{
    return kTerrainBuilder;
}

}

// Inferred placement. OnBuildRoad's point list as a script array of Vector2
// instance copies; reads the VM global itself.
// FUNCTION: VC1 0x00407d30
template<> void SqPlus::Push(HSQUIRRELVM v, vc1::KdObjArray<vc1::Vector2>& value)
{
    unsigned i;

    sq_newarray(v, value.Count());
    for (i = 0; i < value.Count(); ++i) {
        sq_pushinteger(v, i);
        Push(v, static_cast<const vc1::Vector2&>(value[i]));
        sq_set(v, -3);
    }
}

namespace vc1 {

// RET 4. Script "SetWasteLayer".
// FUNCTION: VC1 0x004a5580
void KdTerrainBuilder::SetWasteLayer(const KdPtr<ITileLayer>& layer)
{
    wasteLayer.Assign(layer);
}

// RET 4. Script "SetForestLayer".
// FUNCTION: VC1 0x004a55c0
void KdTerrainBuilder::SetForestLayer(const KdPtr<ITileLayer>& layer)
{
    forestLayer.Assign(layer);
}

// RET 4. Script "SetGridRouter".
// FUNCTION: VC1 0x004a5600
void KdTerrainBuilder::SetGridRouter(const KdPtr<IGridRouter>& value)
{
    router.Assign(value);
}

// XOR AL, AL; RET 4. IInputHandler vtable+0x0c: keys are not consumed.
// FUNCTION: VC1 0x004a5640
unsigned char KdTerrainBuilder::OnKey(const unsigned*)
{
    return 0;
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004a5650
unsigned char KdTerrainBuilder::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// RET 8. Script "RemoveRoadCell" (tail call into the road renderer).
// FUNCTION: VC1 0x004a5660
void KdTerrainBuilder::RemoveRoadCell(int x, int y)
{
    roadRenderer->RemoveCell(x, y);
}

// RET 0x10. Script "AddRoadCell" (tail call into the road renderer).
// FUNCTION: VC1 0x004a5670
void KdTerrainBuilder::AddRoadCell(int x, int y, bool visible, bool strict)
{
    roadRenderer->AddCell(x, y, visible, strict);
}

// Inferred name. EAX=this, EDI=x, EBX=y. The tint of one cell in the current
// mode: dark where a road / forest cannot go, green on the plantable cell
// under the cursor, white on the destroyable cell under the cursor.
// FUNCTION: VC1 0x004a5680
unsigned KdTerrainBuilder::CellColor(int x, int y)
{
    const int value = gridMap->GetCellValue(x, y);

    if (mode == modeBuildRoad && !(roadBuildableMask & (1 << value))) {
        return kCellBlocked;
    }
    if (mode == modeBuildForest) {
        if (value != gridMap->clean && value != gridMap->forestCutted
            && (value < gridMap->forestMin || value >= gridMap->forestMax)) {
            return kCellBlocked;
        }
        if (cursor.x == x && cursor.y == y) {
            return kCellPlantable;
        }
    }
    if (mode == modeDestroy && cursor.x == x && cursor.y == y
        && (value == gridMap->road || value == gridMap->roadBroken
            || value == gridMap->waste || value == gridMap->forestBurnt
            || value == gridMap->forestCutted
            || (gridMap->forestMin <= value && value <= gridMap->forestMax))) {
        return kCellDestroyable;
    }
    return kCellClear;
}

// Inferred name. ESI=out, EAX=this, RET 8. The grid cell under a UI point:
// UI to world through the camera, then into the grid map's space.
// FUNCTION: VC1 0x004a5750
void KdTerrainBuilder::ScreenToCell(CellCoord* out, int x, int y)
{
    const Matrix3& uiToWorld = camera->GetUIToWorld();
    const Matrix3 m = Matrix3::Inverse(gridMap->GetPosition()) * uiToWorld;
    const SQFloat fx = static_cast<SQFloat>(x);
    const SQFloat fy = static_cast<SQFloat>(y);

    out->x = static_cast<unsigned short>(static_cast<short>(m._31 + fy * m._21 + fx * m._11));
    out->y = static_cast<unsigned short>(static_cast<short>(fx * m._12 + fy * m._22 + m._32));
}

// Inferred name. EBX=this. Rebinds the road renderer to the grid map and
// adds every road cell of the map (invisible roads hidden, broken ones
// broken).
// FUNCTION: VC1 0x004a5840
void KdTerrainBuilder::SyncRoads()
{
    int x;
    int y;

    roadRenderer->SetGridMap(gridMap.AsPtr());
    for (x = 0; x < gridMap->GetWidth(); ++x) {
        for (y = 0; y < gridMap->GetHeight(); ++y) {
            const int value = gridMap->GetCellValue(x, y);
            if (value == gridMap->road || value == gridMap->roadIndestructible) {
                roadRenderer->AddCell(x, y, true, false);
            }
            else if (value == gridMap->roadInvisible) {
                roadRenderer->AddCell(x, y, false, false);
            }
            else if (value == gridMap->roadBroken) {
                roadRenderer->AddCell(x, y, true, false);
                roadRenderer->BreakCell(x, y);
            }
        }
    }
}

// Script "GetForestLevel": the sum of the forest levels (1 for
// CELL_VALUE_ForestMin) of every forest cell.
// FUNCTION: VC1 0x004a5910
int KdTerrainBuilder::GetForestLevel()
{
    int level;
    int width;
    int height;
    int x;
    int y;

    if (!gridMap) {
        return 0;
    }
    level = 0;
    width = gridMap->GetWidth();
    height = gridMap->GetHeight();
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            const int value = gridMap->GetCellValue(x, y);
            if (value >= gridMap->forestMin && value <= gridMap->forestMax) {
                level += value - gridMap->forestMin + 1;
            }
        }
    }
    return level;
}

// RET 4. Script "GetCellCount": the number of cells holding value.
// FUNCTION: VC1 0x004a59a0
int KdTerrainBuilder::GetCellCount(int value)
{
    int count;
    int width;
    int height;
    int x;
    int y;

    if (!gridMap) {
        return 0;
    }
    count = 0;
    width = gridMap->GetWidth();
    height = gridMap->GetHeight();
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            if (gridMap->GetCellValue(x, y) == value) {
                ++count;
            }
        }
    }
    return count;
}

// RET 4. Script "SetCamera".
// FUNCTION: VC1 0x004a5a30
void KdTerrainBuilder::SetCamera(const KdPtr<ICamera>& value)
{
    camera.Assign(value);
}

// RET 4. Script "SetRenderer": with a grid map bound, the roads are synced.
// FUNCTION: VC1 0x004a5a70
void KdTerrainBuilder::SetRenderer(const KdPtr<IRoadRenderer>& value)
{
    roadRenderer.Assign(value);
    if (gridMap) {
        SyncRoads();
    }
}

// RET 4. Script "SetGridMap": the cell values a road may be built on come
// from the map; with a renderer bound, the roads are synced.
// FUNCTION: VC1 0x004a5ac0
void KdTerrainBuilder::SetGridMap(const KdPtr<IGridMap>& value)
{
    gridMap.Assign(value);
    if (gridMap) {
        roadBuildableMask = 1 << gridMap->roadConstruction | 1 << gridMap->roadIndestructible
            | 1 << gridMap->road | 1 << gridMap->roadBroken | 1 << gridMap->clean;
    }
    if (roadRenderer) {
        SyncRoads();
    }
}

// RET 4. IGridRenderer vtable+0x0c: one CellColor filled unit square per
// cell, in grid space (graphics transform * world to screen * grid map).
// FUNCTION: VC1 0x004a5b50
void KdTerrainBuilder::RenderGrid(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
    Matrix3 saved;
    int width;
    int height;
    int x;
    int y;

    memcpy(&saved, &target->GetTransform(), sizeof(saved));
    {
        ICamera* const view = static_cast<ICamera*>(*target->GetCamera());
        const Matrix3& position = gridMap->GetPosition();
        target->SetTransform(saved * view->GetWorldToScreen() * position);
    }
    width = gridMap->GetWidth();
    height = gridMap->GetHeight();
    for (x = 0; x < width; ++x) {
        const SQFloat fx = static_cast<SQFloat>(x);
        for (y = 0; y < height; ++y) {
            target->SetColor(CellColor(x, y));
            target->FillRect(fx, static_cast<SQFloat>(y), 1.0f, 1.0f);
        }
    }
    target->SetTransform(saved);
}

// FUNCTION: VC1 0x004a5cf0
unsigned KdTerrainBuilder::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x004a5d10
unsigned KdTerrainBuilder::AddRef()
{
    return IAbstract::AddRef();
}

// RET 0x10. Script "AddTerrainTile": a random tile id of tiles placed at
// position in layer, appended or inserted.
// FUNCTION: VC1 0x004a5d20
void KdTerrainBuilder::AddTerrainTile(const Vector2& position, const KdPtr<ITileLayer>& layer,
    const KdArray<short>& tiles, bool append)
{
    if (roadRenderer && camera) {
        const int count = static_cast<int>(tiles.Count());
        const int index = count > 0
            ? static_cast<int>(static_cast<__int64>(rand()) * count / 0x8000)
            : 0;
        Tile tile;
        tile.id = tiles[index];
        tile.x = static_cast<short>(static_cast<int>(position.x));
        tile.y = static_cast<short>(static_cast<int>(position.y));
        if (append) {
            layer->AppendTile(tile);
        }
        else {
            layer->InsertTile(tile);
        }
    }
}

// Inferred name. EAX=this. Empties the road draft and its preview.
// FUNCTION: VC1 0x004a5dc0
void KdTerrainBuilder::ClearDraft()
{
    draft.items.size = 0;
    if (roadRenderer) {
        roadRenderer->SetPreview(draft, true);
    }
}

// RET 4. Script "SetMode": leaving build-road drops the draft; the builder
// draws over the grid map in every mode but MODE_Default.
// FUNCTION: VC1 0x004a5df0
void KdTerrainBuilder::SetMode(int value)
{
    if (value == mode) {
        return;
    }
    if (mode == modeBuildRoad) {
        draft.items.size = 0;
        if (roadRenderer) {
            roadRenderer->SetPreview(draft, true);
        }
    }
    mode = value;
    dragging = false;
    cursor.x = 0xffff;
    cursor.y = 0xffff;
    if (gridMap) {
        if (value == modeDefault) {
            KdPtr<IUnknown> self = SelfObject(this);
            KdPtr<IGridRenderer> renderer(self);
            gridMap->RemoveRenderer(renderer);
        }
        else {
            KdPtr<IUnknown> self = SelfObject(this);
            KdPtr<IGridRenderer> renderer(self);
            gridMap->AddRenderer(renderer);
        }
    }
}

// RET 0x14. Script "ReplaceTiles": every tile of layer in cell (x, y) whose
// id is in from gets the id at the same index of to.
// FUNCTION: VC1 0x004a5f00
void KdTerrainBuilder::ReplaceTiles(int x, int y, const KdPtr<ITileLayer>& layer,
    const KdArray<short>& from, const KdArray<short>& to)
{
    if (roadRenderer && camera) {
        const Matrix3& position = gridMap->GetPosition();
        const Matrix3 inverse = Matrix3::Inverse(camera->GetWorldToScreen() * position);
        KdArray<Tile>& tiles = layer->GetTiles();
        unsigned i;

        for (i = 0; i < tiles.Count(); ++i) {
            const short* it = from.Data();
            const short* end = reinterpret_cast<const short*>(
                reinterpret_cast<const char*>(from.items.begin) + from.items.size);
            for (; it != end; ++it) {
                if (*it == tiles[i].id) {
                    break;
                }
            }
            if (it != reinterpret_cast<const short*>(
                    reinterpret_cast<const char*>(from.items.begin) + from.items.size)) {
                const SQFloat fx = static_cast<SQFloat>(tiles[i].x);
                const SQFloat fy = static_cast<SQFloat>(tiles[i].y);
                const SQFloat cx = fy * inverse._21 + fx * inverse._11 + inverse._31;
                const SQFloat cy = fy * inverse._22 + inverse._12 * fx + inverse._32;
                if (x == static_cast<int>(floorf(cx)) && y == static_cast<int>(floorf(cy))) {
                    layer->SetTileId(i, to[it - from.Data()]);
                }
            }
        }
    }
}

// RET 0x14. Script "RemoveTiles": removes the tiles of layer in cell (x, y)
// whose id is in ids, at most maxCount of them when maxCount > 0.
// FUNCTION: VC1 0x004a60b0
void KdTerrainBuilder::RemoveTiles(int x, int y, const KdPtr<ITileLayer>& layer,
    const KdArray<short>& ids, int maxCount)
{
    int removed = 0;

    if (roadRenderer && camera) {
        const Matrix3& position = gridMap->GetPosition();
        const Matrix3 inverse = Matrix3::Inverse(camera->GetWorldToScreen() * position);
        KdArray<Tile>& tiles = layer->GetTiles();
        unsigned i = 0;

        while (i < tiles.Count()) {
            const short* it = ids.Data();
            const short* end = reinterpret_cast<const short*>(
                reinterpret_cast<const char*>(ids.items.begin) + ids.items.size);
            for (; it != end; ++it) {
                if (*it == tiles[i].id) {
                    break;
                }
            }
            if (it != reinterpret_cast<const short*>(
                    reinterpret_cast<const char*>(ids.items.begin) + ids.items.size)) {
                const SQFloat fx = static_cast<SQFloat>(tiles[i].x);
                const SQFloat fy = static_cast<SQFloat>(tiles[i].y);
                const SQFloat cy = fy * inverse._22 + inverse._12 * fx + inverse._32;
                const SQFloat cx = fy * inverse._21 + fx * inverse._11 + inverse._31;
                if (x == static_cast<int>(floorf(cx)) && y == static_cast<int>(floorf(cy))) {
                    layer->RemoveTile(i);
                    ++removed;
                    if (maxCount > 0 && removed >= maxCount) {
                        return;
                    }
                    continue;
                }
            }
            ++i;
        }
    }
}

// Inferred name. ESI=this, EDI=cell. Plants one forest tile at a random
// point inside the cell (0.04 in from its edges), replacing a cut forest
// tile there.
// FUNCTION: VC1 0x004a6270
void KdTerrainBuilder::PlantForest(const CellCoord* cell)
{
    if (roadRenderer && camera && forestLayer) {
        const Matrix3& position = gridMap->GetPosition();
        const Matrix3 m = camera->GetWorldToScreen() * position;
        const int x = cell->x;
        const int y = cell->y;
        const SQFloat rx = randVal(static_cast<SQFloat>(x + 0.04),
            static_cast<SQFloat>((x + 1) - 0.04));
        const SQFloat ry = randVal(static_cast<SQFloat>(y + 0.04),
            static_cast<SQFloat>((y + 1) - 0.04));
        const Vector2 point(ry * m._21 + rx * m._11 + m._31, ry * m._22 + rx * m._12 + m._32);

        RemoveTiles(cell->x, cell->y, forestLayer.AsPtr(),
            forestCuttedTiles, 1);
        AddTerrainTile(point, forestLayer.AsPtr(),
            forestTiles, false);
    }
}

// RET 4. Script "SetWasteTiles".
// FUNCTION: VC1 0x004a63c0
void KdTerrainBuilder::SetWasteTiles(const KdArray<short>& tiles)
{
    wasteTiles.AssignRange(tiles.Data(), reinterpret_cast<const short*>(
        reinterpret_cast<const char*>(tiles.items.begin) + tiles.items.size));
}

// RET 4. Script "SetForestBurntTiles".
// FUNCTION: VC1 0x004a63e0
void KdTerrainBuilder::SetForestBurntTiles(const KdArray<short>& tiles)
{
    forestBurntTiles.AssignRange(tiles.Data(), reinterpret_cast<const short*>(
        reinterpret_cast<const char*>(tiles.items.begin) + tiles.items.size));
}

// RET 4. Script "SetForestCuttedTiles".
// FUNCTION: VC1 0x004a6400
void KdTerrainBuilder::SetForestCuttedTiles(const KdArray<short>& tiles)
{
    forestCuttedTiles.AssignRange(tiles.Data(), reinterpret_cast<const short*>(
        reinterpret_cast<const char*>(tiles.items.begin) + tiles.items.size));
}

// RET 4. Script "SetForestTiles".
// FUNCTION: VC1 0x004a6420
void KdTerrainBuilder::SetForestTiles(const KdArray<short>& tiles)
{
    forestTiles.AssignRange(tiles.Data(), reinterpret_cast<const short*>(
        reinterpret_cast<const char*>(tiles.items.begin) + tiles.items.size));
}

// KdComponentGroup vtable+0x0c: the compound's shutdown, then the bound
// components become the null object's faces.
// FUNCTION: VC1 0x004a6440
int KdTerrainBuilder::Shutdown()
{
    KdCompoundObject::Shutdown();
    ResetToNullObject(gridMap);
    ResetToNullObject(router);
    ResetToNullObject(roadRenderer);
    ResetToNullObject(camera);
    ResetToNullObject(forestLayer);
    return 0;
}

// FUNCTION: VC1 0x004a6570
KdTerrainBuilder::~KdTerrainBuilder() throw()
{
}

// EAX=this. The MODE_* values are read by LoadFromScript.
// FUNCTION: VC1 0x004a7180
KdTerrainBuilder::KdTerrainBuilder() throw()
{
}

// FUNCTION: VC1 0x004a7280
void** CreateKdTerrainBuilder(void** out)
{
    KdTerrainBuilder* memory = static_cast<KdTerrainBuilder*>(malloc(sizeof(KdTerrainBuilder)));
    IUnknown* queried = 0;

    if (memory) {
        new (memory) KdTerrainBuilder();
        queried = static_cast<IUnknown*>(
            static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract));
    }
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

// Inferred name. EAX=this. Script "CanBuildRoad"(self, clean cells, broken
// road cells of the draft); false without the member.
// FUNCTION: VC1 0x004a72e0
bool KdTerrainBuilder::CanBuildDraft()
{
    IScriptInstance* instance = &script;
    SquirrelObject member;
    int cleanCells;
    int brokenCells;
    unsigned i;

    instance->GetMember(kScriptCanBuildRoad, member);
    if (member.GetObjectHandle()._type == OT_NULL) {
        return false;
    }
    cleanCells = 0;
    brokenCells = 0;
    for (i = 0; i < draft.Count(); ++i) {
        if (gridMap->GetCellValue(draft[i].x, draft[i].y) == gridMap->clean) {
            ++cleanCells;
        }
        else if (gridMap->GetCellValue(draft[i].x, draft[i].y) == gridMap->roadBroken) {
            ++brokenCells;
        }
    }
    {
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::PushParam(cleanCells);
        SquirrelVM::PushParam(brokenCells);
        SquirrelObject result = SquirrelVM::EndCall();
        return result.GetObjectHandle()._unVal.nInteger != 0;
    }
}

// Script "SetDraftToRenderer": the draft into the road renderer's preview.
// FUNCTION: VC1 0x004a74e0
void KdTerrainBuilder::SetDraftToRenderer()
{
    roadRenderer->SetPreview(draft, CanBuildDraft());
}

// RET 8. Script "BreakRoadCell": breaks the road cell in the renderer and
// the grid map, then tells the script ("OnChangeRoads"(self)).
// FUNCTION: VC1 0x004a7510
void KdTerrainBuilder::BreakRoadCell(int x, int y)
{
    IScriptInstance* instance = &script;
    SquirrelObject member;

    roadRenderer->BreakCell(x, y);
    gridMap->SetCellValue(x, y, gridMap->roadBroken);
    instance->GetMember(kScriptOnChangeRoads, member);
    if (member.GetObjectHandle()._type != OT_NULL) {
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::EndCall();
    }
}

// Inferred name. ESI=this. While dragging in build-road mode: the route
// from the drag start to the hovered cell becomes the draft.
// FUNCTION: VC1 0x004a76a0
void KdTerrainBuilder::UpdateRoadDraft()
{
    unsigned i;

    if (dragEnd.x == cursor.x && dragEnd.y == cursor.y) {
        return;
    }
    dragEnd = cursor;
    {
        const Vector2 from(static_cast<SQFloat>(dragStart.x), static_cast<SQFloat>(dragStart.y));
        const Vector2 to(static_cast<SQFloat>(dragEnd.x), static_cast<SQFloat>(dragEnd.y));
        router->SetPassableMask(roadBuildableMask);
        draft.Resize(router->FindPath(from, to, false));
    }
    for (i = 0; i < draft.Count(); ++i) {
        draft[i].x = static_cast<unsigned short>(static_cast<short>(router->GetPoint(i).x));
        draft[i].y = static_cast<unsigned short>(static_cast<short>(router->GetPoint(i).y));
    }
    roadRenderer->SetPreview(draft, CanBuildDraft());
}

// Inferred name. EAX=this. Hovering in build-road mode: the hovered cell is
// the draft when a road may go there.
// FUNCTION: VC1 0x004a7840
void KdTerrainBuilder::HoverRoadCell()
{
    if (dragStart.x == cursor.x && dragStart.y == cursor.y) {
        return;
    }
    dragStart = cursor;
    dragEnd = cursor;
    if (roadBuildableMask & (1 << gridMap->GetCellValue(dragStart.x, dragStart.y))) {
        draft.Resize(1);
        draft[0] = dragStart;
    }
    else {
        draft.items.size = 0;
    }
    roadRenderer->SetPreview(draft, CanBuildDraft());
}

// RET 4. IScriptLoadable vtable+0x0c: the compound's load (owner = the
// IScriptObject face), then the mode values.
// FUNCTION: VC1 0x004a7910
bool KdTerrainBuilder::LoadFromScript(SquirrelObject& table)
{
    if (!script.Load(table, static_cast<IScriptObject*>(this))) {
        return false;
    }
    modeDefault = table.GetInt(kScriptModeDefault);
    modeBuildRoad = table.GetInt(kScriptModeBuildRoad);
    modeBuildForest = table.GetInt(kScriptModeBuildForest);
    modeDestroy = table.GetInt(kScriptModeDestroy);
    return true;
}

// Inferred name. Destroy mode: asks the script member for the hovered
// cell's kind ("OnDestroyRoad"(self, cell), ..., "OnDestroyForest" also
// gets the forest level); when it agrees, removes the road or the tiles and
// clears the cell.
// FUNCTION: VC1 0x004a7980
void KdTerrainBuilder::DestroyAtCursor()
{
    const int value = gridMap->GetCellValue(cursor.x, cursor.y);
    const char* name;
    bool accepted = false;

    if (value == gridMap->road) {
        name = kScriptOnDestroyRoad;
    }
    else if (value == gridMap->roadBroken) {
        name = kScriptOnDestroyRoadBroken;
    }
    else if (value >= gridMap->forestMin && value <= gridMap->forestMax) {
        name = kScriptOnDestroyForest;
    }
    else if (value == gridMap->forestBurnt) {
        name = kScriptOnDestroyForestBurnt;
    }
    else if (value == gridMap->forestCutted) {
        name = kScriptOnDestroyForestCutted;
    }
    else if (value == gridMap->waste) {
        name = kScriptOnDestroyWaste;
    }
    else {
        return;
    }
    {
        IScriptInstance* instance = &script;
        SquirrelObject member;

        instance->GetMember(name, member);
        if (member.GetObjectHandle()._type != OT_NULL) {
            SquirrelObject self = ScriptSelf(this);
            SquirrelVM::BeginCall(member, self);
            SquirrelVM::PushParam(SquirrelObject(CellPoint(cursor)));
            if (strcmp(name, kScriptOnDestroyForest) == 0) {
                SquirrelVM::PushParam(value - gridMap->forestMin + 1);
            }
            SquirrelObject result = SquirrelVM::EndCall();
            accepted = result.GetObjectHandle()._unVal.nInteger != 0;
        }
    }
    if (!accepted) {
        return;
    }
    if (value == gridMap->road || value == gridMap->roadBroken) {
        roadRenderer->RemoveCell(cursor.x, cursor.y);
    }
    else if (value >= gridMap->forestMin && value <= gridMap->forestMax) {
        RemoveTiles(cursor.x, cursor.y, forestLayer.AsPtr(),
            forestTiles, 0);
        RemoveTiles(cursor.x, cursor.y, forestLayer.AsPtr(),
            forestCuttedTiles, 0);
    }
    else if (value == gridMap->forestBurnt) {
        RemoveTiles(cursor.x, cursor.y, forestLayer.AsPtr(),
            forestBurntTiles, 0);
    }
    else if (value == gridMap->forestCutted) {
        RemoveTiles(cursor.x, cursor.y, forestLayer.AsPtr(),
            forestCuttedTiles, 0);
    }
    else if (value == gridMap->waste) {
        RemoveTiles(cursor.x, cursor.y, wasteLayer.AsPtr(),
            wasteTiles, 0);
    }
    gridMap->SetCellValue(cursor.x, cursor.y, gridMap->clean);
}

// Inferred name. Build-forest click: on a clean, cut or not fully grown
// forest cell the script "OnBuildForest"(self, cell, level) decides; the
// cell then grows one level and a forest tile is planted.
// FUNCTION: VC1 0x004a7d70
void KdTerrainBuilder::BuildForestAtCursor()
{
    const int value = gridMap->GetCellValue(cursor.x, cursor.y);
    int level;

    if (value == gridMap->clean || value == gridMap->forestCutted) {
        level = 0;
    }
    else if (value >= gridMap->forestMin && value < gridMap->forestMax) {
        level = value - gridMap->forestMin + 1;
        if (level == static_cast<int>(0x80000000)) {
            return;
        }
    }
    else {
        return;
    }
    {
        IScriptInstance* instance = &script;
        SquirrelObject member;

        instance->GetMember(kScriptOnBuildForest, member);
        if (member.GetObjectHandle()._type != OT_NULL) {
            bool accepted;
            {
                SquirrelObject self = ScriptSelf(this);
                SquirrelVM::BeginCall(member, self);
                SquirrelVM::PushParam(SquirrelObject(CellPoint(cursor)));
                SquirrelVM::PushParam(level);
                SquirrelObject result = SquirrelVM::EndCall();
                accepted = result.GetObjectHandle()._unVal.nInteger != 0;
            }
            if (accepted) {
                gridMap->SetCellValue(cursor.x, cursor.y, gridMap->forestMin + level);
                PlantForest(&cursor);
            }
        }
    }
}

// Inferred name. Build-road release: the draft cells a road may take (clean
// or broken road) go to the script "OnBuildRoad"(self, points); when it
// agrees they become road (broken ones repaired).
// FUNCTION: VC1 0x004a7f50
void KdTerrainBuilder::CommitRoadDraft()
{
    KdObjArray<Vector2> points;
    unsigned i;

    for (i = 0; i < draft.Count(); ++i) {
        const int value = gridMap->GetCellValue(draft[i].x, draft[i].y);
        if (value == gridMap->clean || value == gridMap->roadBroken) {
            points.PushBack(Vector2(static_cast<SQFloat>(draft[i].x),
                static_cast<SQFloat>(draft[i].y)));
        }
    }
    {
        IScriptInstance* instance = &script;
        SquirrelObject member;
        bool accepted;

        instance->GetMember(kScriptOnBuildRoad, member);
        if (member.GetObjectHandle()._type == OT_NULL) {
            return;
        }
        {
            SquirrelObject self = ScriptSelf(this);
            SquirrelVM::BeginCall(member, self);
            SquirrelVM::PushParam(SquirrelObject(points));
            SquirrelObject result = SquirrelVM::EndCall();
            accepted = result.GetObjectHandle()._unVal.nInteger != 0;
        }
        if (!accepted) {
            return;
        }
        for (i = 0; i < points.Count(); ++i) {
            const int x = static_cast<int>(points[i].x);
            const int y = static_cast<int>(points[i].y);
            if (gridMap->GetCellValue(x, y) == gridMap->roadBroken) {
                roadRenderer->RepairCell(x, y);
            }
            else {
                roadRenderer->AddCell(x, y, true, true);
            }
            gridMap->SetCellValue(x, y, gridMap->road);
        }
    }
}

// RET 4. IInputHandler vtable+0x10 (data: kind, button, x, y). Outside
// MODE_Default every pointer event updates the hovered cell (script
// "OnCellHovered"(self, cell)); then build-road drags a route and commits
// it on release, build-forest plants on press, destroy destroys on press
// and while dragging.
// FUNCTION: VC1 0x004a83e0
unsigned char KdTerrainBuilder::OnPointer(const unsigned* data)
{
    if (mode == modeDefault) {
        return 0;
    }
    if (data[0] - 0x4001 <= 2) {
        CellCoord cell;
        ScreenToCell(&cell, data[2], data[3]);
        if (cell.x != cursor.x || cell.y != cursor.y) {
            IScriptInstance* instance = &script;
            SquirrelObject member;

            cursor = cell;
            instance->GetMember(kScriptOnCellHovered, member);
            if (member.GetObjectHandle()._type != OT_NULL) {
                SquirrelObject self((SelfObject(this)));
                SquirrelVM::BeginCall(member, self);
                SquirrelVM::PushParam(SquirrelObject(CellPoint(cursor)));
                SquirrelVM::EndCall();
            }
        }
    }
    if (mode == modeBuildRoad) {
        const int kind = static_cast<int>(data[0]);
        if (kind >= 0x4001) {
            if (kind <= 0x4002) {
                if (dragging) {
                    UpdateRoadDraft();
                    return 1;
                }
                HoverRoadCell();
            }
            else if (kind == 0x4003) {
                if (data[1] == 0) {
                    if (dragging) {
                        CommitRoadDraft();
                        HoverRoadCell();
                        dragging = false;
                        return 1;
                    }
                }
                else if (data[1] == 1) {
                    dragging = true;
                    return 1;
                }
            }
        }
    }
    else if (mode == modeBuildForest) {
        if (data[0] == 0x4003 && data[1] == 1) {
            BuildForestAtCursor();
            return 1;
        }
    }
    else if (mode == modeDestroy) {
        const int kind = static_cast<int>(data[0]);
        if (kind >= 0x4001) {
            if (kind <= 0x4002) {
                if (dragging) {
                    DestroyAtCursor();
                    return 1;
                }
            }
            else if (kind == 0x4003) {
                if (data[1] == 0) {
                    if (dragging) {
                        dragging = false;
                        return 1;
                    }
                }
                else if (data[1] == 1) {
                    DestroyAtCursor();
                    dragging = true;
                    return 1;
                }
            }
        }
    }
    return 0;
}

// RET 8. The script instance's members, then the compound's, then the
// method table (table 0x004ee6a0, guard bit 0x004ee6b0).
// FUNCTION: VC1 0x004a8630
unsigned char KdTerrainBuilder::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    if (KdCompoundObject::GetMember(name, out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptSetGridMap] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetGridMap);
        methods[kScriptSetGridRouter] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetGridRouter);
        methods[kScriptSetRenderer] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetRenderer);
        methods[kScriptSetCamera] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetCamera);
        methods[kScriptSetForestLayer] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetForestLayer);
        methods[kScriptSetForestTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetForestTiles);
        methods[kScriptSetForestCuttedTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetForestCuttedTiles);
        methods[kScriptSetForestBurntTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetForestBurntTiles);
        methods[kScriptSetMode] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetMode);
        methods[kScriptGetCellCount] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::GetCellCount);
        methods[kScriptGetForestLevel] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::GetForestLevel);
        methods[kScriptAddRoadCell] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::AddRoadCell);
        methods[kScriptBreakRoadCell] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::BreakRoadCell);
        methods[kScriptRemoveRoadCell] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::RemoveRoadCell);
        methods[kScriptSetWasteLayer] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetWasteLayer);
        methods[kScriptSetWasteTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetWasteTiles);
        methods[kScriptAddTerrainTile] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::AddTerrainTile);
        methods[kScriptRemoveTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::RemoveTiles);
        methods[kScriptReplaceTiles] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::ReplaceTiles);
        methods[kScriptSetDraftToRenderer] = CreateScriptMethod<KdTerrainBuilder>(
            &KdTerrainBuilder::SetDraftToRenderer);
    }
    return methods.Find(name, out);
}

}
