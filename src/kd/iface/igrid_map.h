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
    virtual void Render(void* graphics) = 0; // vtable+0x0c
};

template <>
struct KdIid<IGridRenderer>
{
    static const char** Get() { return &g_iidGridRenderer; }
};

// Inferred names. A width x height byte grid placed in the world by the
// IPositionable matrix (PassMap: the cell kinds of a level map).
// SIZE 0x04
struct IGridMap : IPositionable
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

}

#endif
