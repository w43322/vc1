#include "kd/road/grid_rect_renderer.h"

#include "kd/iface/icamera.h"
#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/tile/tile_manager.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfd04
static const char kGridRectRenderer[] = "GridRectRenderer";
// GLOBAL: VC1 0x004d0b20
const char* g_kdNameGridRectRenderer = kGridRectRenderer;

// STRING: VC1 0x004d0fc0
static const char kScriptSetRect[] = "SetRect";
// STRING: VC1 0x004d0fc8
static const char kScriptAddRect[] = "AddRect";
// STRING: VC1 0x004d0fd0
static const char kScriptClearRects[] = "ClearRects";
// STRING: VC1 0x004d0fdc
static const char kScriptSetCellTiles[] = "SetCellTiles";
extern const char kScriptSetGridMap[];

// Inferred name. This file's own never-assigned null object (the same idea
// as g_kdNullObject at another address), released at exit by 0x004c9b30.
// GLOBAL: VC1 0x004ed674
KdRef<IUnknown> g_kdGridRectNullObject;

// MOV EAX, "GridRectRenderer"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00404580
const char* KdGridRectRenderer::GetTypeName()
{
    return kGridRectRenderer;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004afd30
unsigned char KdGridRectRenderer::SetMember(void*, void*)
{
    return 0;
}

// RET 4. Script "SetGridMap".
// FUNCTION: VC1 0x004afd40
void KdGridRectRenderer::SetGridMap(const KdPtr<IGridMap>& map)
{
    gridMap.Assign(map);
}

// ADD ECX, 0xc; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004afdd0
unsigned KdGridRectRenderer::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004afdf0
unsigned KdGridRectRenderer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// RET 4. Script "SetCellTiles".
// FUNCTION: VC1 0x004afe00
void KdGridRectRenderer::SetCellTiles(const KdArray<short>& tiles)
{
    int i;

    for (i = 0; i < 16; ++i) {
        cellTiles[i] = tiles[i];
    }
}

// Script "ClearRects".
// FUNCTION: VC1 0x004afe80
void KdGridRectRenderer::ClearRects()
{
    rects.items.ResizeBytes(0);
}

// RET 8. Script "SetRect"; ignores an index past the end.
// FUNCTION: VC1 0x004afeb0
void KdGridRectRenderer::SetRect(int index, const Rect& rect)
{
    if (static_cast<unsigned>(index) < rects.Count()) {
        rects[index] = rect;
    }
}

// IShutdownable+0x0c (ECX = IShutdownable face). Drops the grid map for the
// null object's IGridMap face and the rectangles.
// FUNCTION: VC1 0x004afef0
int KdGridRectRenderer::Shutdown()
{
    if (gridMap.ptr) {
        gridMap.ptr->Release();
    }
    if (g_kdGridRectNullObject.ptr) {
        gridMap.ptr = static_cast<IGridMap*>(
            g_kdGridRectNullObject.ptr->QueryInterface(&g_iidGridMap));
        if (gridMap.ptr) {
            gridMap.ptr->AddRef();
        }
    }
    else {
        gridMap.ptr = 0;
    }
    rects.items.ResizeBytes(0);
    return 0;
}

// ESI=this.
// FUNCTION: VC1 0x004aff60
KdGridRectRenderer::~KdGridRectRenderer() throw()
{
}

// EAX=this. The tile table stays uninitialized.
// FUNCTION: VC1 0x004affd0
KdGridRectRenderer::KdGridRectRenderer() throw()
{
}

// RET 4. Returns the new renderer as IAbstract.
// FUNCTION: VC1 0x004b0020
void** CreateKdGridRectRenderer(void** out)
{
    void* const memory = malloc(sizeof(KdGridRectRenderer));
    KdGridRectRenderer* const object = memory ? new (memory) KdGridRectRenderer() : 0;
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

// RET 4. Script "AddRect".
// FUNCTION: VC1 0x004b0070
void KdGridRectRenderer::AddRect(const Rect& rect)
{
    const unsigned count = rects.Count() + 1;
    const unsigned bytes = count * sizeof(Rect);

    if (rects.items.capacity < bytes) {
        rects.Reserve(count, 0);
    }
    rects.items.size = bytes;
    rects.End()[-1] = rect;
}

// RET 8. The method table (guard 0x004ee570).
// FUNCTION: VC1 0x004b03f0
unsigned char KdGridRectRenderer::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetRect] = CreateScriptMethod<KdGridRectRenderer>(&KdGridRectRenderer::SetRect);
        methods[kScriptAddRect] = CreateScriptMethod<KdGridRectRenderer>(&KdGridRectRenderer::AddRect);
        methods[kScriptClearRects] = CreateScriptMethod<KdGridRectRenderer>(&KdGridRectRenderer::ClearRects);
        methods[kScriptSetCellTiles] = CreateScriptMethod<KdGridRectRenderer>(
            &KdGridRectRenderer::SetCellTiles);
        methods[kScriptSetGridMap] = CreateScriptMethod<KdGridRectRenderer>(
            &KdGridRectRenderer::SetGridMap);
    }
    return methods.Find(name, out);
}

// Inferred name. The lazily created "TileManager" component in this file's
// own static (guard 0x004ee778, value 0x004ee774), returned by reference.
// FUNCTION: VC1 0x004b06b0
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

// IGridRenderer+0x0c, RET 4. Every cell of every rectangle, centred on the
// cell, with the tile for the sides that border other cells of the same
// rectangle.
// FUNCTION: VC1 0x004b0740
void KdGridRectRenderer::RenderGrid(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);

    if (gridMap.ptr) {
        target.ptr->SetColor(0xffffffff);
        ICamera* const camera = static_cast<ICamera*>(*target.ptr->GetCamera());
        const Matrix3& position = gridMap.ptr->GetPosition();
        const Matrix3 m = camera->GetWorldToScreen() * position;
        const Rect* rect;

        for (rect = rects.Begin(); rect != rects.End(); ++rect) {
            int x;
            for (x = rect->x; x < rect->x + rect->width; ++x) {
                int y;
                for (y = rect->y; y < rect->y + rect->height; ++y) {
                    const SQFloat cx = static_cast<SQFloat>(x + 0.5);
                    const SQFloat cy = static_cast<SQFloat>(y + 0.5);
                    const SQFloat px = cy * m._21 + cx * m._11 + m._31;
                    const SQFloat py = cy * m._22 + cx * m._12 + m._32;
                    int flags = 0;

                    if (x > rect->x) {
                        flags = 1;
                    }
                    if (x < rect->x + rect->width - 1) {
                        flags |= 2;
                    }
                    if (y > rect->y) {
                        flags |= 4;
                    }
                    if (y < rect->y + rect->height - 1) {
                        flags |= 8;
                    }
                    TileManagerRef().ptr->Render(target, cellTiles[flags], px, py, 0);
                }
            }
        }
    }
}

}
