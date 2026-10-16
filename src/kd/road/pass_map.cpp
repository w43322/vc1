#include "kd/road/pass_map.h"

#include "kd/group/ptr_set.h"
#include "kd/iface/iid.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"
#include "sqplus/SquirrelVM.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// STRING: VC1 0x004cfd7c
static const char kPassMap[] = "PassMap";
// GLOBAL: VC1 0x004d0b00
const char* g_kdNamePassMap = kPassMap;

// STRING: VC1 0x004d12e4
static const char kScriptGetCellValue[] = "GetCellValue";
// STRING: VC1 0x004d12f4
static const char kScriptSetCellValue[] = "SetCellValue";
// STRING: VC1 0x004d1304
static const char kScriptAddRenderer[] = "AddRenderer";
// STRING: VC1 0x004d1310
static const char kScriptRemoveRenderer[] = "RemoveRenderer";
// STRING: VC1 0x004d1320
static const char kLayerPassMap[] = "LAYER_PassMap";
// STRING: VC1 0x004d1330
static const char kCellClean[] = "CELL_VALUE_Clean";
// STRING: VC1 0x004d1344
static const char kCellOccupied[] = "CELL_VALUE_Occupied";
// STRING: VC1 0x004d1358
static const char kCellPlant[] = "CELL_VALUE_Plant";
// STRING: VC1 0x004d136c
static const char kCellRoadBroken[] = "CELL_VALUE_RoadBroken";
// STRING: VC1 0x004d1384
static const char kCellRoad[] = "CELL_VALUE_Road";
// STRING: VC1 0x004d1394
static const char kCellRoadIndestructible[] = "CELL_VALUE_RoadIndestructible";
// STRING: VC1 0x004d13b4
static const char kCellRoadConstruction[] = "CELL_VALUE_RoadConstruction";
// STRING: VC1 0x004d13d0
static const char kCellForestMin[] = "CELL_VALUE_ForestMin";
// STRING: VC1 0x004d13e8
static const char kCellForestMax[] = "CELL_VALUE_ForestMax";
// STRING: VC1 0x004d1400
static const char kCellForestBurnt[] = "CELL_VALUE_ForestBurnt";
// STRING: VC1 0x004d1418
static const char kCellForestCutted[] = "CELL_VALUE_ForestCutted";
// STRING: VC1 0x004d1430
static const char kCellWaste[] = "CELL_VALUE_Waste";
// STRING: VC1 0x004d1444
static const char kCellRoadInvisible[] = "CELL_VALUE_RoadInvisible";
// STRING: VC1 0x004d1460
static const char kScriptOnCellChange[] = "OnCellChange";

// The object as a script value: the IAbstract face QI'd twice (0x00401e90),
// pushed through SquirrelObject::Set<KdPtr<IUnknown> > (0x004014a0).
static __forceinline SquirrelObject ScriptSelf(IAbstract* face)
{
    IUnknown* object = static_cast<IUnknown*>(face->QueryInterface(&g_iidAbstract));
    return SquirrelObject((KdPtr<IUnknown>(object
        ? static_cast<IUnknown*>(object->QueryInterface(&g_iidAbstract))
        : 0)));
}

static inline Vector2 ScaleAxis(const Vector2& axis, int value)
{
    const SQFloat scale = static_cast<SQFloat>(value);
    const Vector2 scaled(axis.x * scale, axis.y * scale);
    return Vector2(static_cast<SQFloat>(scaled.x * (1.0 / 4096)),
        static_cast<SQFloat>(scaled.y * (1.0 / 4096)));
}

// MOV EAX, "PassMap"; RET. IScriptObject+0x14.
// FUNCTION: VC1 0x004059f0
const char* KdPassMap::GetTypeName()
{
    return kPassMap;
}

// Inferred name. EAX=&stream, cdecl value. Folded with the IOutputStream
// twin: both are the slot+0x0c call with 2 bytes.
// FUNCTION: VC1 0x00405c00 FOLDED
static bool ReadShort(const KdPtr<IInputStream>& stream, short* value)
{
    return stream.ptr->Read(value, 2) == 2;
}

// FUNCTION: VC1 0x00405c00 FOLDED
static bool WriteShort(const KdPtr<IOutputStream>& stream, const short* value)
{
    return stream.ptr->Write(value, 2) == 2;
}

// IVisible+0x10.
// FUNCTION: VC1 0x004ac850
bool KdPassMap::IsVisible()
{
    return visible;
}

// IVisible+0x0c.
// FUNCTION: VC1 0x004ac860
void KdPassMap::SetVisible(bool value)
{
    visible = value;
}

// IGridMap+0x18; script "GetHeight".
// FUNCTION: VC1 0x004ac870
int KdPassMap::GetHeight()
{
    return height;
}

// IGridMap+0x14; script "GetWidth".
// FUNCTION: VC1 0x004ac880
int KdPassMap::GetWidth()
{
    return width;
}

// IPositionable+0x10.
// FUNCTION: VC1 0x004ac890
const Matrix3& KdPassMap::GetPosition()
{
    return position;
}

// IPositionable+0x0c.
// FUNCTION: VC1 0x004ac8a0
void KdPassMap::SetPosition(const Matrix3& value)
{
    position = value;
}

// IRenderable+0x14.
// FUNCTION: VC1 0x004ac8c0
int KdPassMap::GetRenderOrder()
{
    return renderLayer;
}

// IScriptObject+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004ac8d0
unsigned char KdPassMap::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// ADD ECX, 0x68; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004ac9b0
unsigned KdPassMap::Release()
{
    unsigned value = --refs;
    if (value == 0) {
        delete static_cast<IAbstract*>(this);
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004ac9d0
unsigned KdPassMap::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IWriteable+0x0c. The axis scales in 1/4096, a version byte 1, the origin
// as 16-bit integers, width, height, then the cells.
// FUNCTION: VC1 0x004ac9e0
bool KdPassMap::WriteTo(const KdPtr<IOutputStream>& stream)
{
    int scale;
    unsigned value;

    scale = static_cast<int>(position._11 * 4096.0);
    if (stream.ptr->Write(&scale, 4) != 4) {
        return false;
    }
    scale = static_cast<int>(position._22 * 4096.0);
    if (stream.ptr->Write(&scale, 4) != 4) {
        return false;
    }
    *reinterpret_cast<unsigned char*>(&value) = 1;
    if (stream.ptr->Write(&value, 1) != 1) {
        return false;
    }
    value = static_cast<unsigned short>(static_cast<int>(position._31));
    if (stream.ptr->Write(&value, 2) != 2) {
        return false;
    }
    value = static_cast<unsigned short>(static_cast<int>(position._32));
    if (stream.ptr->Write(&value, 2) != 2) {
        return false;
    }
    if (!WriteShort(stream, &width)) {
        return false;
    }
    if (!WriteShort(stream, &height)) {
        return false;
    }
    const unsigned size = cells.items.size;
    return stream.ptr->Write(cells.items.begin, size) == size;
}

// RET 8. IGridMap+0x1c; script "GetCellValue". -1 outside the grid.
// FUNCTION: VC1 0x004acb10
int KdPassMap::GetCellValue(int x, int y)
{
    const int index = width * y + x;
    if (index < 0 || index >= static_cast<int>(cells.items.size)) {
        return -1;
    }
    return cells.Data()[index];
}

// IReadable+0x0c. The layout WriteTo writes; the version byte is skipped.
// FUNCTION: VC1 0x004acb40
bool KdPassMap::ReadFrom(const KdPtr<IInputStream>& stream)
{
    int scaleX;
    int scaleY;
    unsigned char version;
    short x;
    short y;

    if (stream.ptr->Read(&scaleX, 4) != 4) {
        return false;
    }
    if (stream.ptr->Read(&scaleY, 4) != 4) {
        return false;
    }
    position = Matrix3(ScaleAxis(Vector2::AxisX, scaleX), ScaleAxis(Vector2::AxisY, scaleY),
        Vector2::Null);
    if (stream.ptr->Read(&version, 1) != 1) {
        return false;
    }
    if (stream.ptr->Read(&x, 2) != 2) {
        return false;
    }
    if (stream.ptr->Read(&y, 2) != 2) {
        return false;
    }
    position._31 = static_cast<SQFloat>(x);
    position._32 = static_cast<SQFloat>(y);
    if (!ReadShort(stream, &width)) {
        return false;
    }
    if (!ReadShort(stream, &height)) {
        return false;
    }
    cells.Resize(width * height);
    const unsigned size = cells.items.size;
    return stream.ptr->Read(cells.items.begin, size) == size;
}

// IShutdownable+0x0c. Drops the renderers.
// FUNCTION: VC1 0x004acd50
int KdPassMap::Shutdown()
{
    RefClear(&renderers);
    return 0;
}

// RET 4. IRenderable+0x10: every renderer draws while the map is visible.
// FUNCTION: VC1 0x004acd80
int KdPassMap::Render(void* graphics)
{
    if (visible) {
        IUnknown** it;
        for (it = RefBegin(&renderers); it != RefEnd(&renderers); ++it) {
            static_cast<IGridRenderer*>(*it)->Render(graphics);
        }
    }
    return 0;
}

// RET 4. IGridMap+0x28; script "RemoveRenderer".
// FUNCTION: VC1 0x004acdd0
void KdPassMap::RemoveRenderer(const KdPtr<IGridRenderer>& renderer)
{
    IUnknown** const first = RefBegin(&renderers);
    IUnknown** const last = RefEnd(&renderers);
    IUnknown* const* const value = reinterpret_cast<IUnknown* const*>(&renderer.ptr);
    IUnknown** const kept = PtrRemove(first, last, value);
    const unsigned removed = static_cast<unsigned>(last - kept) * 4;
    EraseCountedBytes(kept, kept + (last - kept), RefEnd(&renderers));
    renderers.items.size -= removed;
}

// RET 4. IGridMap+0x24; script "AddRenderer". Null renderers are ignored.
// FUNCTION: VC1 0x004ace30
void KdPassMap::AddRenderer(const KdPtr<IGridRenderer>& renderer)
{
    if (renderer.ptr) {
        PtrListAppend(&renderers, reinterpret_cast<IUnknown* const*>(&renderer.ptr));
    }
}

// EAX=this.
// FUNCTION: VC1 0x004ace50
KdPassMap::~KdPassMap() throw()
{
}

// RET 8. The script instance's members first, then the method table (table
// 0x004ee54c, guard bit 0x004ee55c, atexit destructor 0x004c9d90). Every
// method binds an IGridMap slot (vcall thunks).
// FUNCTION: VC1 0x004ad200
unsigned char KdPassMap::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptGetWidth] = CreateScriptMethod<KdPassMap>(
            static_cast<int (KdPassMap::*)()>(&IGridMap::GetWidth));
        methods[kScriptGetHeight] = CreateScriptMethod<KdPassMap>(
            static_cast<int (KdPassMap::*)()>(&IGridMap::GetHeight));
        methods[kScriptGetCellValue] = CreateScriptMethod<KdPassMap>(
            static_cast<int (KdPassMap::*)(int, int)>(&IGridMap::GetCellValue));
        methods[kScriptSetCellValue] = CreateScriptMethod<KdPassMap>(
            static_cast<void (KdPassMap::*)(int, int, int)>(&IGridMap::SetCellValue));
        methods[kScriptAddRenderer] = CreateScriptMethod<KdPassMap>(
            static_cast<void (KdPassMap::*)(const KdPtr<IGridRenderer>&)>(&IGridMap::AddRenderer));
        methods[kScriptRemoveRenderer] = CreateScriptMethod<KdPassMap>(
            static_cast<void (KdPassMap::*)(const KdPtr<IGridRenderer>&)>(&IGridMap::RemoveRenderer));
    }
    return methods.Find(name, out);
}

// EDI=this. The render layer comes from the root table.
// FUNCTION: VC1 0x004ad480
KdPassMap::KdPassMap() throw()
    : width(0), height(0), visible(false)
{
    renderLayer = SquirrelVM::GetRootTable().GetInt(kLayerPassMap);
    visible = true;
}

// RET 4. Returns the new map as IAbstract.
// FUNCTION: VC1 0x004ad640
void** CreateKdPassMap(void** out)
{
    void* const memory = malloc(sizeof(KdPassMap));
    KdPassMap* const object = memory ? new (memory) KdPassMap() : 0;
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

// RET 0xc. IGridMap+0x20; script "SetCellValue". A changed cell calls the
// script member "OnCellChange"(self, x, y, value) when the script has one.
// FUNCTION: VC1 0x004ad6a0
void KdPassMap::SetCellValue(int x, int y, int value)
{
    const int index = width * y + x;
    if (index < 0 || index >= static_cast<int>(cells.items.size)) {
        return;
    }
    unsigned char& cell = cells.Data()[index];
    if (cell == value) {
        return;
    }
    cell = static_cast<unsigned char>(value);

    IScriptInstance* instance = &script;
    SquirrelObject member;
    instance->GetMember(kScriptOnCellChange, member);
    if (member.GetObjectHandle()._type != OT_NULL) {
        SquirrelObject self = ScriptSelf(this);
        SquirrelVM::BeginCall(member, self);
        SquirrelVM::PushParam(x);
        SquirrelVM::PushParam(y);
        SquirrelVM::PushParam(value);
        SquirrelVM::EndCall();
    }
}

// RET 4. IScriptLoadable+0x0c: loads the script instance (owner = the
// IScriptObject face), then the cell kinds from the table.
// FUNCTION: VC1 0x004ad790
bool KdPassMap::LoadFromScript(SquirrelObject& table)
{
    if (!script.Load(table, static_cast<IScriptObject*>(this))) {
        return false;
    }
    clean = table.GetInt(kCellClean);
    occupied = table.GetInt(kCellOccupied);
    plant = table.GetInt(kCellPlant);
    roadBroken = table.GetInt(kCellRoadBroken);
    road = table.GetInt(kCellRoad);
    roadIndestructible = table.GetInt(kCellRoadIndestructible);
    roadConstruction = table.GetInt(kCellRoadConstruction);
    forestMin = table.GetInt(kCellForestMin);
    forestMax = table.GetInt(kCellForestMax);
    forestBurnt = table.GetInt(kCellForestBurnt);
    forestCutted = table.GetInt(kCellForestCutted);
    waste = table.GetInt(kCellWaste);
    roadInvisible = table.GetInt(kCellRoadInvisible);
    return true;
}

}
