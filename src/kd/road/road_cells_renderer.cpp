#include "kd/road/road_cells_renderer.h"

#include "kd/iface/iid.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004ed690
KdRef<IUnknown> g_kdRoadCellsNullObject;
// GLOBAL: VC1 0x004e9680
KdString g_kdRoadCellsEmptyName = KdEmptyText();

// STRING: VC1 0x004cfcac
static const char kRoadCellsRenderer[] = "RoadCellsRenderer";
// GLOBAL: VC1 0x004d0b34
const char* g_kdNameRoadCellsRenderer = kRoadCellsRenderer;

// STRING: VC1 0x004d15ac
static const char kScriptSetRoadRenderer[] = "SetRoadRenderer";
// STRING: VC1 0x004d15bc
static const char kScriptSetCells[] = "SetCells";
// STRING: VC1 0x004d15c8
static const char kScriptClearCells[] = "ClearCells";

// IScriptObject+0x14.
// FUNCTION: VC1 0x00406100
const char* KdRoadCellsRenderer::GetTypeName()
{
    return kRoadCellsRenderer;
}

// RET 4. Script "SetRenderLayer".
// FUNCTION: VC1 0x004abe20
void KdRoadCellsRenderer::SetRenderLayer(int layer)
{
    renderLayer = layer;
}

// RET 4. Script "SetRoadRenderer".
// FUNCTION: VC1 0x004abe30
void KdRoadCellsRenderer::SetRoadRenderer(const KdPtr<IRoadRenderer>& renderer)
{
    roadRenderer.Assign(renderer);
}

// RET 4. IRenderable+0x10: the RoadRenderer draws the cells as route marks.
// FUNCTION: VC1 0x004abe70
int KdRoadCellsRenderer::Render(void* graphics)
{
    roadRenderer->RenderRouteMarks(*static_cast<const KdPtr<IGraphics>*>(graphics), cells);
    return 0;
}

// IRenderable+0x14.
// FUNCTION: VC1 0x004abe90
int KdRoadCellsRenderer::GetRenderOrder()
{
    return renderLayer;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004abea0
unsigned char KdRoadCellsRenderer::SetMember(void*, void*)
{
    return 0;
}

// ADD ECX, 0x24; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004abf10
unsigned KdRoadCellsRenderer::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004abf30
unsigned KdRoadCellsRenderer::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IRenderable+0x0c, RET 8. Renders (virtually) when the own classificators
// pass the mask; without IClassifiable only an empty lo mask passes.
// FUNCTION: VC1 0x004abf40
int KdRoadCellsRenderer::RenderMasked(void* graphics, void* mask)
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
        return static_cast<IRenderable*>(this)->Render(graphics);
    }
    return 0;
}

// Script "ClearCells".
// FUNCTION: VC1 0x004ac000
void KdRoadCellsRenderer::ClearCells()
{
    cells.items.ResizeBytes(0);
}

// RET 4. Script "SetCells": the grid points (truncated to cells) looked up
// in the RoadRenderer, copied.
// FUNCTION: VC1 0x004ac030
void KdRoadCellsRenderer::SetCells(const KdObjArray<Vector2>& points)
{
    const unsigned count = points.items.size >> 3;
    KdArray<CellCoord> coords(count);
    unsigned i;

    for (i = 0; i < count; ++i) {
        coords[i].x = static_cast<unsigned short>(static_cast<int>(points[i].x));
        coords[i].y = static_cast<unsigned short>(static_cast<int>(points[i].y));
    }
    KdObjArray<RoadCell> found(roadRenderer->GetCells(coords));
    cells.AssignRange(found.Begin(), found.End());
}

// EAX=this.
// FUNCTION: VC1 0x004ac180
KdRoadCellsRenderer::~KdRoadCellsRenderer() throw()
{
}

// RET 8. No script instance; the own table only.
// FUNCTION: VC1 0x004ac510
unsigned char KdRoadCellsRenderer::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptSetRenderLayer] =
            CreateScriptMethod<KdRoadCellsRenderer>(&KdRoadCellsRenderer::SetRenderLayer);
        methods[kScriptSetRoadRenderer] =
            CreateScriptMethod<KdRoadCellsRenderer>(&KdRoadCellsRenderer::SetRoadRenderer);
        methods[kScriptSetCells] =
            CreateScriptMethod<KdRoadCellsRenderer>(&KdRoadCellsRenderer::SetCells);
        methods[kScriptClearCells] =
            CreateScriptMethod<KdRoadCellsRenderer>(&KdRoadCellsRenderer::ClearCells);
    }
    return methods.Find(name, out);
}

// ESI=this.
// FUNCTION: VC1 0x004ac770
KdRoadCellsRenderer::KdRoadCellsRenderer() throw()
{
}

// FUNCTION: VC1 0x004ac800
void** CreateKdRoadCellsRenderer(void** out)
{
    void* const memory = malloc(sizeof(KdRoadCellsRenderer));
    IAbstract* object = 0;
    IUnknown* queried;

    if (memory) {
        object = new (memory) KdRoadCellsRenderer();
    }
    queried = object ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract)) : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    static_cast<IUnknown*>(*out)->Release();
    return out;
}

}
