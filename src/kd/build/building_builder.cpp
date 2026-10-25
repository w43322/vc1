#include "kd/build/building_builder.h"
#include "kdcore/kd_memory.h"

#include "kd/iface/igraphics.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "app/host_api.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"
#include "sqplus/SquirrelVM.h"

#include <math.h>
#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004e95d8
KdString g_kdBuildingBuilderEmptyName = KdEmptyText();

// STRING: VC1 0x004cfd30
static const char kBuildingBuilder[] = "BuildingBuilder";
// GLOBAL: VC1 0x004d0b14
const char* g_kdNameBuildingBuilder = kBuildingBuilder;

extern const char kScriptSetGridMap[];  // terrain_builder.cpp
extern const char kScriptSetCamera[];   // system.cpp
extern const char kScriptSetEnabled[];  // object_watcher.cpp

// STRING: VC1 0x004cf8e0
static const char kScriptIsEnabled[] = "IsEnabled";
// STRING: VC1 0x004cf8ec
static const char kScriptShowBuilding[] = "ShowBuilding";
// STRING: VC1 0x004cf8fc
static const char kScriptSetBuildingData[] = "SetBuildingData";
// STRING: VC1 0x004cf90c
static const char kScriptAddMarginRect[] = "AddMarginRect";
// STRING: VC1 0x004cf91c
static const char kScriptCheckPlacement[] = "CheckPlacement";
// STRING: VC1 0x004cf92c
static const char kScriptLayerTop[] = "LAYER_Top";
// STRING: VC1 0x004cf938
static const char kTileChild[] = "Tile";
// STRING: VC1 0x004cf940
static const char kDraftAnimation[] = "Draft";
// STRING: VC1 0x004cf948
static const char kCantFindAnimation[] = "Can't find building animation";
// STRING: VC1 0x004cf968
static const char kScriptOnBoundsChange[] = "OnBoundsChange";
// STRING: VC1 0x004cf978
static const char kScriptOnRefuseBuild[] = "OnRefuseBuild";
// STRING: VC1 0x004cf988
static const char kScriptOnBuild[] = "OnBuild";

// Inferred name. This file's own never-assigned null object.
// GLOBAL: VC1 0x004ed640
KdRef<IUnknown> g_kdBuildingBuilderNullObject;

// Footprint cells drawn by Render: blocked (red) and the grid tint.
static const unsigned kCellBlocked = 0x55ff0000;
static const unsigned kCellOccupied = 0x55000000;
static const unsigned kDraftColor = 0xaaffffff;

// The same as a script value, through SquirrelObject::Set<KdPtr<IUnknown> >
// (0x004014a0).
static __forceinline SquirrelObject ScriptSelf(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return SquirrelObject((KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0)));
}

// Release slot, then the face iid of source (counted), or 0.
template <class T>
static inline void ResetFrom(KdRef<T>& slot, IUnknown* source, const char** iid)
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

// MOV EAX, "BuildingBuilder"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00401020
const char* KdBuildingBuilder::GetTypeName()
{
    return kBuildingBuilder;
}

// RET 4. Script "ShowBuilding": the Tile child's visibility.
// FUNCTION: VC1 0x004b2460
void KdBuildingBuilder::ShowBuilding(bool value)
{
    if (tile.ptr) {
        KdPtr<IVisible> visible(tile.AsPtr());
        if (visible.ptr) {
            visible->SetVisible(value);
        }
    }
}

// Script "IsEnabled".
// FUNCTION: VC1 0x004b24a0
bool KdBuildingBuilder::IsEnabled()
{
    return enabled;
}

// RET 4. Script "SetEnabled": shows the Tile child and adds the grid face to
// the grid map's renderers, or hides and removes them.
// FUNCTION: VC1 0x004b24b0
void KdBuildingBuilder::SetEnabled(bool value)
{
    if (enabled == value) {
        return;
    }
    enabled = value;
    if (tile.ptr) {
        KdPtr<IVisible> visible(tile.AsPtr());
        if (visible.ptr) {
            visible->SetVisible(value);
        }
    }
    if (gridMap.ptr) {
        if (enabled) {
            KdPtr<IGridRenderer> renderer(SelfObject(this));
            gridMap->AddRenderer(renderer);
        }
        else {
            KdPtr<IGridRenderer> renderer(SelfObject(this));
            gridMap->RemoveRenderer(renderer);
        }
    }
}

// RET 4. Script "SetCamera".
// FUNCTION: VC1 0x004b25b0
void KdBuildingBuilder::SetCamera(const KdPtr<ICamera>& value)
{
    camera.Assign(value);
}

// IRenderable vtable+0x14.
// FUNCTION: VC1 0x004b25f0
int KdBuildingBuilder::GetRenderOrder()
{
    return layerTop;
}

// FUNCTION: VC1 0x004b2600
const Matrix3& KdBuildingBuilder::GetPosition()
{
    return position;
}

// RET 4.
// FUNCTION: VC1 0x004b2610
void KdBuildingBuilder::SetPosition(const Matrix3& value)
{
    position = value;
}

// RET 4. Keys are not handled.
// FUNCTION: VC1 0x004b2630
unsigned char KdBuildingBuilder::OnKey(const unsigned* data)
{
    (void)data;
    return 0;
}

// FUNCTION: VC1 0x004b2640
unsigned char KdBuildingBuilder::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// FUNCTION: VC1 0x004b2650
unsigned KdBuildingBuilder::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x004b2670
unsigned KdBuildingBuilder::AddRef()
{
    return IAbstract::AddRef();
}

// Inferred name. EDI=this, stack x, EBX=y, RET 4. A clean cell outside every
// margin rectangle. Unreferenced in the original (CheckPlacement and Render
// inline the same test).
// FUNCTION: VC1 0x004b2680
bool KdBuildingBuilder::IsCellFree(int x, int y)
{
    return gridMap->GetCellValue(x, y) == gridMap->clean
        && margins[gridMap->GetWidth() * y + x] != 0;
}

// Inferred name. ECX=this. Clears every margin rectangle.
// FUNCTION: VC1 0x004b26d0
void KdBuildingBuilder::ResetMargins()
{
    unsigned i;

    for (i = 0; i < margins.items.size; ++i) {
        margins[i] = 1;
    }
}

// RET 4. Script "AddMarginRect": cells the footprint must keep clear of.
// FUNCTION: VC1 0x004b2700
void KdBuildingBuilder::AddMarginRect(const Rect& rect)
{
    int x;

    for (x = rect.x; x < rect.x + rect.width; ++x) {
        int y;
        for (y = rect.y; y < rect.y + rect.height; ++y) {
            margins[gridMap->GetWidth() * y + x] = 0;
        }
    }
}

// RET 4. IGridRenderer vtable+0x0c: every cell that is not clean, tinted, in
// grid space (graphics transform * world to screen * grid map).
// FUNCTION: VC1 0x004b2760
void KdBuildingBuilder::RenderGrid(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
    Matrix3 saved;
    int width;
    int height;
    int x;
    int y;

    KdMemcpy(&saved, &target->GetTransform(), sizeof(saved));
    {
        ICamera* const view = static_cast<ICamera*>(*target->GetCamera());
        const Matrix3& grid = gridMap->GetPosition();
        target->SetTransform(saved * view->GetWorldToScreen() * grid);
    }
    target->SetColor(kCellOccupied);
    width = gridMap->GetWidth();
    height = gridMap->GetHeight();
    for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
            IGridMap* const map = gridMap.ptr;
            if (map->GetCellValue(x, y) != map->clean) {
                target->FillRect(static_cast<SQFloat>(x), static_cast<SQFloat>(y), 1.0f, 1.0f);
            }
        }
    }
    target->SetTransform(saved);
}

// RET 4. IRenderable vtable+0x10: while enabled and the Tile child is
// visible, the blocked cells of the footprint in red.
// FUNCTION: VC1 0x004b28a0
int KdBuildingBuilder::Render(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);
    KdPtr<IVisible> visible(tile.AsPtr());

    if (enabled && visible->IsVisible()) {
        Matrix3 saved;
        int x;

        KdMemcpy(&saved, &target->GetTransform(), sizeof(saved));
        {
            ICamera* const view = static_cast<ICamera*>(*target->GetCamera());
            const Matrix3& grid = gridMap->GetPosition();
            target->SetTransform(saved * view->GetWorldToScreen() * grid);
        }
        target->SetColor(kCellBlocked);
        for (x = bounds.x; x < bounds.x + bounds.width; ++x) {
            int y;
            for (y = bounds.y; y < bounds.y + bounds.height; ++y) {
                if (!IsCellFree(x, y)) {
                    target->FillRect(static_cast<SQFloat>(x), static_cast<SQFloat>(y), 1.0f, 1.0f);
                }
            }
        }
        target->SetTransform(saved);
    }
    return 0;
}

// ECX=this. Script "CheckPlacement": every footprint cell is free.
// FUNCTION: VC1 0x004b2af0
bool KdBuildingBuilder::CheckPlacement()
{
    int x;

    for (x = bounds.x; x < bounds.x + bounds.width; ++x) {
        int y;
        for (y = bounds.y; y < bounds.y + bounds.height; ++y) {
            if (!IsCellFree(x, y)) {
                return false;
            }
        }
    }
    return true;
}

// RET 4. Script "SetGridMap": one margin flag per cell, all clear.
// FUNCTION: VC1 0x004b2b80
void KdBuildingBuilder::SetGridMap(const KdPtr<IGridMap>& value)
{
    gridMap.Assign(value);
    if (!gridMap.ptr) {
        margins.items.size = 0;
    }
    else {
        IGridMap* const map = gridMap.ptr;
        const unsigned cells = map->GetHeight() * map->GetWidth();
        if (margins.items.capacity < cells) {
            GrowCountedBuffer(&margins.items, static_cast<int>(cells), 1);
        }
        margins.items.size = cells;
    }
    ResetMargins();
}

// RET 0xc. Script "SetBuildingData"(animation data, grid width, grid height):
// the Tile child shows the first frame of the "Draft" animation.
// FUNCTION: VC1 0x004b2c10
void KdBuildingBuilder::SetBuildingData(const KdPtr<ITileAnimationData>& data, int width, int height)
{
    if (data.ptr) {
        const TileAnimation* draft = data->Find(KdString(kDraftAnimation));
        if (!draft) {
            HostLogError(&g_kdNameBuildingBuilder, kCantFindAnimation);
            return;
        }
        tile->SetTile(draft->channels.Begin()[0][0]);
        bounds.width = width;
        bounds.height = height;
        ResetMargins();
    }
}

// KdComponentGroup vtable+0x0c.
// FUNCTION: VC1 0x004b2cf0
int KdBuildingBuilder::Shutdown()
{
    KdCompoundObject::Shutdown();
    ResetFrom(gridMap, g_kdBuildingBuilderNullObject.ptr, &g_iidGridMap);
    ResetFrom(camera, g_kdBuildingBuilderNullObject.ptr, &g_iidCamera);
    ResetFrom(tile, g_kdBuildingBuilderNullObject.ptr, &g_iidTileObject);
    return 0;
}

// FUNCTION: VC1 0x004b31c0
KdBuildingBuilder::~KdBuildingBuilder() throw()
{
}

// EDI=this. Creates the "Tile" child (drawn on the top layer, translucent,
// placed by this object's position) and registers it.
// FUNCTION: VC1 0x004b3290
KdBuildingBuilder::KdBuildingBuilder() throw()
{
    enabled = false;
    layerTop = SquirrelVM::GetRootTable().GetInt(kScriptLayerTop);
    {
        KdPtr<IUnknown> self = SelfObject(this);
        ResetFrom(positionProvider, self.ptr, &g_iidPositionable);
    }
    {
        void* unknown;
        GetKdFactory()->QueryNamed(&unknown, &g_kdNameTileObject);
        if (tile.ptr) {
            tile.ptr->Release();
        }
        if (unknown) {
            tile.ptr = static_cast<ITileObject*>(
                static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidTileObject));
            if (tile.ptr) {
                tile.ptr->AddRef();
            }
            static_cast<IUnknown*>(unknown)->Release();
        }
        else {
            tile.ptr = 0;
        }
    }
    tile->SetRenderLayer(layerTop);
    tile->SetColor(kDraftColor);
    {
        KdPtr<IPositionableUser> user(tile.AsPtr());
        if (user.ptr) {
            KdPtr<IPositionable> source(SelfObject(this));
            user->SetPositionable(source);
        }
    }
    {
        KdPtr<IAbstract> child(tile.AsPtr());
        RegisterObject(KdString(kTileChild), child);
    }
}

// FUNCTION: VC1 0x004b3650
void** CreateKdBuildingBuilder(void** out)
{
    KdBuildingBuilder* memory = static_cast<KdBuildingBuilder*>(malloc(sizeof(KdBuildingBuilder)));
    IUnknown* queried = 0;

    if (memory) {
        new (memory) KdBuildingBuilder();
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

// RET 4. IScriptLoadable vtable+0x0c.
// FUNCTION: VC1 0x004b36b0
bool KdBuildingBuilder::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this));
}

// Inferred name. EAX=this. A click: "OnBuild"(self, position) when the
// footprint is free, else "OnRefuseBuild"(self).
// FUNCTION: VC1 0x004b36d0
void KdBuildingBuilder::TryBuild()
{
    IScriptInstance* instance = &script;

    if (!CheckPlacement()) {
        SquirrelObject member;
        instance->GetMember(kScriptOnRefuseBuild, member);
        if (member.GetObjectHandle()._type != OT_NULL) {
            SquirrelObject self = ScriptSelf(this);
            SquirrelVM::BeginCall(member, self);
            SquirrelVM::EndCall();
        }
    }
    else {
        SquirrelObject member;
        instance->GetMember(kScriptOnBuild, member);
        if (member.GetObjectHandle()._type != OT_NULL) {
            SquirrelObject self = ScriptSelf(this);
            SquirrelVM::BeginCall(member, self);
            SquirrelVM::PushParam(SquirrelObject(reinterpret_cast<Vector2&>(position._31)));
            SquirrelVM::EndCall();
        }
    }
}

// Inferred name. EAX=this, stack screen x/y, RET 8. Snaps the footprint
// centre under the cursor to the grid (cell centres for odd sizes, cell
// corners for even ones); when the footprint moves, places the building and
// calls "OnBoundsChange"(self, bounds).
// FUNCTION: VC1 0x004b38f0
void KdBuildingBuilder::UpdateCursor(int x, int y)
{
    if (gridMap.ptr && camera.ptr) {
        const Matrix3& uiToWorld = camera->GetUIToWorld();
        const Matrix3 m = Matrix3::Inverse(gridMap->GetPosition()) * uiToWorld;
        const SQFloat fx = static_cast<SQFloat>(x);
        const SQFloat fy = static_cast<SQFloat>(y);
        SQFloat cx = fy * m._21 + fx * m._11 + m._31;
        SQFloat cy = fy * m._22 + fx * m._12 + m._32;
        int left;
        int top;

        if (bounds.width % 2 == 0) {
            cx = floorf(static_cast<SQFloat>(cx + 0.5));
        }
        else {
            cx = static_cast<SQFloat>(floorf(cx) + 0.5);
        }
        if (bounds.height % 2 == 0) {
            cy = floorf(static_cast<SQFloat>(cy + 0.5));
        }
        else {
            cy = static_cast<SQFloat>(floorf(cy) + 0.5);
        }
        left = static_cast<int>(cx - bounds.width / 2);
        top = static_cast<int>(cy - bounds.height / 2);
        if (left != bounds.x || top != bounds.y) {
            IScriptInstance* instance = &script;
            SquirrelObject member;

            bounds.x = left;
            bounds.y = top;
            {
                const Matrix3& grid = gridMap->GetPosition();
                const SQFloat px = cy * grid._21 + cx * grid._11 + grid._31;
                const SQFloat py = cy * grid._22 + cx * grid._12 + grid._32;
                position._31 = px;
                position._32 = py;
            }
            instance->GetMember(kScriptOnBoundsChange, member);
            if (member.GetObjectHandle()._type != OT_NULL) {
                SquirrelObject self((SelfObject(this)));
                SquirrelVM::BeginCall(member, self);
                SquirrelVM::PushParam(SquirrelObject(bounds));
                SquirrelVM::EndCall();
            }
        }
    }
}

// RET 4. IInputHandler vtable+0x10: while enabled, pointer moves (0x4001,
// 0x4002) follow the cursor and a release of button 1 (0x4003) builds.
// FUNCTION: VC1 0x004b3bc0
unsigned char KdBuildingBuilder::OnPointer(const unsigned* data)
{
    if (enabled) {
        const int kind = static_cast<int>(data[0]);
        if (kind >= 0x4001) {
            if (kind > 0x4002) {
                if (kind == 0x4003 && data[1] == 1) {
                    UpdateCursor(data[2], data[3]);
                    TryBuild();
                    return 1;
                }
            }
            else {
                UpdateCursor(data[2], data[3]);
                return 1;
            }
        }
    }
    return 0;
}

// FUNCTION: VC1 0x004b3c40
unsigned char KdBuildingBuilder::GetMember(const char* name, SquirrelObject* out)
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
        methods[kScriptSetGridMap] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::SetGridMap);
        methods[kScriptSetCamera] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::SetCamera);
        methods[kScriptSetEnabled] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::SetEnabled);
        methods[kScriptIsEnabled] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::IsEnabled);
        methods[kScriptShowBuilding] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::ShowBuilding);
        methods[kScriptSetBuildingData] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::SetBuildingData);
        methods[kScriptAddMarginRect] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::AddMarginRect);
        methods[kScriptCheckPlacement] = CreateScriptMethod<KdBuildingBuilder>(
            &KdBuildingBuilder::CheckPlacement);
    }
    return methods.Find(name, out);
}

}
