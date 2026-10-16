#ifndef VC1_KD_IFACE_IGRID_MAP_H
#define VC1_KD_IFACE_IGRID_MAP_H

#include "kd/iface/iid.h"
#include "kd/iface/ipositionable.h"

namespace vc1 {

// Inferred names. A layer drawn on top of a grid map (PassMap
// "AddRenderer"); Render gets the KdPtr<IGraphics> of IRenderable::Render.
// SIZE 0x04
struct IGridRenderer : IUnknown
{
    virtual void RenderGrid(void* graphics) = 0; // vtable+0x0c
};

template <>
struct KdIid<IGridRenderer>
{
    static const char** Get() { return &g_iidGridRenderer; }
};

// Inferred name. The cell kinds PassMap reads from its script table
// (CPassMap in Level.nut) by these member names; not initialized by the
// constructor. TerrainBuilder reads them through its IGridMap pointer.
// SIZE 0x34
struct KdPassMapCellValues
{
    int clean;              // 0x00 "CELL_VALUE_Clean"
    int occupied;           // 0x04 "CELL_VALUE_Occupied"
    int plant;              // 0x08 "CELL_VALUE_Plant"
    int roadBroken;         // 0x0c "CELL_VALUE_RoadBroken"
    int road;               // 0x10 "CELL_VALUE_Road"
    int roadIndestructible; // 0x14 "CELL_VALUE_RoadIndestructible"
    int roadConstruction;   // 0x18 "CELL_VALUE_RoadConstruction"
    int forestMin;          // 0x1c "CELL_VALUE_ForestMin"
    int forestMax;          // 0x20 "CELL_VALUE_ForestMax"
    int forestBurnt;        // 0x24 "CELL_VALUE_ForestBurnt"
    int forestCutted;       // 0x28 "CELL_VALUE_ForestCutted"
    int waste;              // 0x2c "CELL_VALUE_Waste"
    int roadInvisible;      // 0x30 "CELL_VALUE_RoadInvisible"
};

typedef char Vc1KdPassMapCellValuesSizeCheck[sizeof(KdPassMapCellValues) == 0x34 ? 1 : -1];

// Inferred names. A width x height byte grid placed in the world by the
// IPositionable matrix (PassMap: the cell kinds of a level map). The cell
// kinds follow the vptr.
// SIZE 0x38
struct IGridMap : IPositionable, KdPassMapCellValues
{
    virtual int GetWidth() = 0;                                     // vtable+0x14
    virtual int GetHeight() = 0;                                    // vtable+0x18
    virtual int GetCellValue(int x, int y) = 0;                     // vtable+0x1c
    virtual void SetCellValue(int x, int y, int value) = 0;         // vtable+0x20
    virtual void AddRenderer(const KdPtr<IGridRenderer>& renderer) = 0;    // vtable+0x24
    virtual void RemoveRenderer(const KdPtr<IGridRenderer>& renderer) = 0; // vtable+0x28
};

struct Vector2;

// Inferred names. Breadth-first path search over an IGridMap: cells whose
// value bit is set in the passable mask can be walked; FindPath fills the
// point list (script "FindPath" / "GetPoint").
// SIZE 0x04
struct IGridRouter : IUnknown
{
    virtual void SetPassableMask(int mask) = 0;              // vtable+0x0c
    virtual int FindPath(const Vector2& from, const Vector2& to,
        bool worldSpace) = 0;                                // vtable+0x10
    virtual Vector2& GetPoint(int index) = 0;                // vtable+0x14
    virtual int GetPointCount() = 0;                         // vtable+0x18
};

template <>
struct KdIid<IGridMap>
{
    static const char** Get() { return &g_iidGridMap; }
};

template <>
struct KdIid<IGridRouter>
{
    static const char** Get() { return &g_iidGridRouter; }
};

}

#endif
