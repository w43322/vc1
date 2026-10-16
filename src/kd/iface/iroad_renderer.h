#ifndef VC1_KD_IFACE_IROAD_RENDERER_H
#define VC1_KD_IFACE_IROAD_RENDERER_H

#include "bindings/vector2.h"
#include "kd/iface/igrid_map.h"
#include "kd/iface/irenderable.h"
#include "kd/name_map.h"

namespace vc1 {

struct IGraphics;

// Inferred names. One road cell of the RoadRenderer grid: its centre in the
// world, the links to the four neighbours, the draw colour and three flags.
// Links: 1 left, 2 right, 4 up, 8 down; they index the 16-entry road tile
// tables. Broken cells draw from the broken tile table and only link to
// broken neighbours when the link is strict.
// SIZE 0x14
struct RoadCell
{
    Vector2 position; // 0x00
    unsigned links;   // 0x08
    unsigned color;   // 0x0c ARGB, -1 for placed roads
    bool routeMark;   // 0x10 draws the route mark tile on top
    bool visible;     // 0x11
    bool broken;      // 0x12

    RoadCell() {}
    void Assign(const RoadCell& source) { *this = source; }
};

typedef char Vc1RoadCellSizeCheck[sizeof(RoadCell) == 0x14 ? 1 : -1];

// Inferred name. A grid cell given by 16-bit coordinates.
// SIZE 0x04
struct CellCoord
{
    unsigned short x; // 0x00
    unsigned short y; // 0x02
};

// Inferred names, from the bodies of KdRoadRenderer (0x004aa3a0 QI's
// "IRoadRenderer"); only "HighlightRoadCells" is a script name.
// SIZE 0x04
struct IRoadRenderer : IRenderable
{
    virtual void SetGridMap(const KdPtr<IGridMap>& map) = 0;                    // vtable+0x18
    virtual void AddCell(int x, int y, bool visible, bool strict) = 0;           // vtable+0x1c
    virtual void BreakCell(int x, int y) = 0;                                    // vtable+0x20
    virtual void RepairCell(int x, int y) = 0;                                   // vtable+0x24
    virtual void RemoveCell(int x, int y) = 0;                                   // vtable+0x28
    virtual void SetPreview(const KdArray<CellCoord>& cells, bool valid) = 0;    // vtable+0x2c
    virtual void HighlightRoadCells(const KdObjArray<Vector2>& cells, bool value) = 0; // vtable+0x30
    virtual void RenderCells(const KdPtr<IGraphics>& graphics,
        const KdObjArray<RoadCell>& cells) = 0;                                  // vtable+0x34
    virtual void RenderRouteMarks(const KdPtr<IGraphics>& graphics,
        const KdObjArray<RoadCell>& cells) = 0;                                  // vtable+0x38
    virtual KdObjArray<RoadCell> GetCells(const KdArray<CellCoord>& cells) = 0;  // vtable+0x3c
};

typedef char Vc1IRoadRendererSizeCheck[sizeof(IRoadRenderer) == 0x04 ? 1 : -1];

template <>
struct KdIid<IRoadRenderer>
{
    static const char** Get() { return &g_iidRoadRenderer; }
};

}

#endif
