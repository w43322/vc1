#include "kd/tile/tile_object.h"

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
// GLOBAL: VC1 0x004e96c0
KdString g_kdTileObjectEmptyName = KdEmptyText();

// STRING: VC1 0x004cfd84
static const char kTileObject[] = "TileObject";
// GLOBAL: VC1 0x004d0afc
const char* g_kdNameTileObject = kTileObject;

// STRING: VC1 0x004d2024
static const char kScriptGetTile[] = "GetTile";
// STRING: VC1 0x004d202c
static const char kScriptSetTile[] = "SetTile";
// STRING: VC1 0x004d2040
static const char kScriptSetHighlighted[] = "SetHighlighted";
// STRING: VC1 0x004d2050
static const char kScriptEnableFloatCoords[] = "EnableFloatCoords";
// STRING: VC1 0x004d2064
static const char kScriptGetJoint[] = "GetJoint";
extern const char kScriptSetColor[];
extern const char kScriptSetClip[];
extern const char kScriptHitTest[];

// Inferred name. This file's own never-assigned null object.
// GLOBAL: VC1 0x004ed6a4
KdRef<IUnknown> g_kdTileObjectNullObject;

// TileManager render flags set by the script toggles.
static const unsigned kTileHighlighted = 0x10000;
static const unsigned kTileFloatCoords = 0x20000;

static inline Vector2 TransformPoint(const Matrix3& m, const Vector2& p)
{
    return Vector2(m._11 * p.x + m._21 * p.y + m._31, m._12 * p.x + m._22 * p.y + m._32);
}

// EAX=this, stack &key, RET 4. First entry whose joint is not below key.
// FUNCTION: VC1 0x00403cf0 FOLDED
KdTileJoint* KdTileJointMap::LowerBound(const int& key)
{
    KdTileJoint* first = Begin();
    KdTileJoint* last = End();

    while (first != last) {
        KdTileJoint* const middle = first + (last - first) / 2;
        if (middle->joint < key) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// MOV EAX, "TileObject"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x0040a9b0
const char* KdTileObject::GetTypeName()
{
    return kTileObject;
}

// EAX=&entry, ESI=this, EDI=result. Inserts entry unless its joint is
// already present.
// FUNCTION: VC1 0x0040ab60
KdTileJointInsert KdTileJointMap::Insert(const KdTileJoint& entry)
{
    KdTileJointInsert result;
    KdTileJoint* const pos = LowerBound(entry.joint);

    if (pos != End() && !(entry.joint < pos->joint)) {
        result.slot = pos;
        result.inserted = false;
        return result;
    }
    result.slot = InsertAt(pos, entry);
    result.inserted = true;
    return result;
}

// EAX=this, EBX=&entry, stack pos, RET 4. Opens a slot at pos and assigns
// entry (counted).
// FUNCTION: VC1 0x0040ae50
KdTileJoint* KdTileJointMap::InsertAt(KdTileJoint* pos, const KdTileJoint& entry)
{
    KdTileJoint* const begin = Begin();
    KdTileJoint* slot;

    Reserve(Count() + 1);
    slot = static_cast<KdTileJoint*>(InsertCountedBytes(&items,
        reinterpret_cast<char*>(items.begin) + (reinterpret_cast<char*>(pos) -
            reinterpret_cast<char*>(begin)), sizeof(KdTileJoint)));
    slot->joint = entry.joint;
    if (entry.object) {
        entry.object->AddRef();
    }
    if (slot->object) {
        slot->object->Release();
    }
    slot->object = entry.object;
    return slot;
}

// ECX=first, EAX=last. Releases the followers of [first, last).
// FUNCTION: VC1 0x0040aeb0
void KdTileJointMap::DestroyRange(KdTileJoint* first, KdTileJoint* last)
{
    for (; first < last; ++first) {
        if (first->object) {
            first->object->Release();
        }
        first->object = 0;
    }
}

// ECX=this, EAX=count. Loose grow; the new entries are zeroed.
// FUNCTION: VC1 0x0040b080 FOLDED
void KdTileJointMap::Reserve(unsigned count)
{
    const unsigned old = items.capacity >> 3;
    KdTileJoint* entry;
    KdTileJoint* end;

    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(KdTileJoint)), 0);
    if (items.capacity >> 3 > old) {
        entry = Begin() + old;
        end = Begin() + (items.capacity >> 3);
        for (; entry < end; ++entry) {
            if (entry) {
                entry->joint = 0;
                entry->object = 0;
            }
        }
    }
}

// IPositionable+0x0c, RET 4. The position follows the owner.
// FUNCTION: VC1 0x004a0100
void KdTileJointObject::SetPosition(const Matrix3&)
{
}

// IShutdownable+0x0c (ECX = IShutdownable face).
// FUNCTION: VC1 0x004a0110
int KdTileJointObject::Shutdown()
{
    if (owner.ptr) {
        owner.ptr->Release();
        owner.ptr = 0;
    }
    return 0;
}

// RET 4. Script "SetClip".
// FUNCTION: VC1 0x004a0130
void KdTileObject::SetClip(const Rect& rect)
{
    clip = rect;
}

// IRenderable+0x14.
// FUNCTION: VC1 0x004a0150
int KdTileObject::GetRenderOrder()
{
    return layer;
}

// IVisible+0x10 (ECX = IVisible face).
// FUNCTION: VC1 0x004a0160
bool KdTileObject::IsVisible()
{
    return visible;
}

// IVisible+0x0c, RET 4.
// FUNCTION: VC1 0x004a0170
void KdTileObject::SetVisible(bool value)
{
    visible = value;
}

// ITileObject+0x2c, RET 4; script "EnableFloatCoords".
// FUNCTION: VC1 0x004a0180
void KdTileObject::EnableFloatCoords(bool value)
{
    if (value) {
        flags |= kTileFloatCoords;
    }
    else {
        flags &= ~kTileFloatCoords;
    }
}

// ITileObject+0x28, RET 4; script "SetHighlighted".
// FUNCTION: VC1 0x004a01a0
void KdTileObject::SetHighlighted(bool value)
{
    if (value) {
        flags |= kTileHighlighted;
    }
    else {
        flags &= ~kTileHighlighted;
    }
}

// ITileObject+0x24, RET 4; script "SetColor".
// FUNCTION: VC1 0x004a01c0
void KdTileObject::SetColor(int value)
{
    color = value;
}

// ITileObject+0x20, RET 4; script "SetRenderLayer".
// FUNCTION: VC1 0x004a01d0
void KdTileObject::SetRenderLayer(int value)
{
    layer = value;
}

// ITileObject+0x1c, RET 4; script "SetTile".
// FUNCTION: VC1 0x004a01e0
void KdTileObject::SetTile(int value)
{
    tile = value;
}

// ITileObject+0x18; script "GetTile".
// FUNCTION: VC1 0x004a01f0
int KdTileObject::GetTile()
{
    return tile;
}

// IScriptObject+0x10. RET 8.
// FUNCTION: VC1 0x004a0200
unsigned char KdTileObject::SetMember(void*, void*)
{
    return 0;
}

// ESI=this. Inlined into the scalar deleting destructor 0x0040a9e0 too.
// FUNCTION: VC1 0x004a0210
KdTileJointObject::~KdTileJointObject() throw()
{
}

// ADD ECX, 8; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004a0290
unsigned KdTileJointObject::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a02b0
unsigned KdTileJointObject::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// IPositionableUser+0x0c, RET 4 (ECX = IPositionableUser face).
// FUNCTION: VC1 0x004a02c0
void KdTileObject::SetPositionable(const KdPtr<IPositionable>& source)
{
    positionable.Assign(source);
}

// ADD ECX, 0x34; deleting destructor through IAbstract on zero.
// FUNCTION: VC1 0x004a03c0
unsigned KdTileObject::Release()
{
    IAbstract* const self = static_cast<IAbstract*>(this);
    unsigned value = --self->refs;
    if (!value) {
        delete self;
        return 0;
    }
    return value;
}

// FUNCTION: VC1 0x004a03e0
unsigned KdTileObject::AddRef()
{
    unsigned value = refs;
    if (value != 0) {
        ++value;
        refs = value;
    }
    return value;
}

// ESI=this, ECX=source, EAX=index. Identity position until the first
// GetPosition; holds a reference on the owner.
// FUNCTION: VC1 0x004a03f0
KdTileJointObject::KdTileJointObject(KdTileObject* source, int index) throw()
{
    owner.ptr = source;
    joint = index;
    if (source) {
        source->AddRef();
    }
}

// Inferred name. EDI=out; cdecl (owner, joint). Returns the new follower as
// IAbstract.
// FUNCTION: VC1 0x004a0450
static void** CreateKdTileJointObject(void** out, KdTileObject* owner, int joint)
{
    void* const memory = malloc(sizeof(KdTileJointObject));
    KdTileJointObject* const object = memory ? new (memory) KdTileJointObject(owner, joint) : 0;
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

// IRenderable+0x0c, RET 8. Renders when the own classificators pass the
// mask.
// FUNCTION: VC1 0x004a04b0
int KdTileObject::RenderMasked(void* graphics, void* mask)
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

// ITileObject+0x30, RET 4. Screen y of the position's translation.
// FUNCTION: VC1 0x004a0570
float KdTileObject::GetDepth(IUnknown** camera)
{
    if (!positionable.ptr) {
        return 0.0f;
    }
    ICamera* const view = static_cast<ICamera*>(*camera);
    const Matrix3& position = positionable.ptr->GetPosition();
    const Matrix3& m = view->GetWorldToScreen();
    const float depth = m._22 * position._32 + m._12 * position._31 + m._32;
    return depth;
}

// IShutdownable+0x0c (ECX = IShutdownable face). Drops the positionable for
// the null object's face and shuts the joint followers down.
// FUNCTION: VC1 0x004a05c0
int KdTileObject::Shutdown()
{
    KdTileJoint* entry;

    if (positionable.ptr) {
        positionable.ptr->Release();
    }
    if (g_kdTileObjectNullObject.ptr) {
        positionable.ptr = static_cast<IPositionable*>(
            g_kdTileObjectNullObject.ptr->QueryInterface(&g_iidPositionable));
        if (positionable.ptr) {
            positionable.ptr->AddRef();
        }
    }
    else {
        positionable.ptr = 0;
    }
    for (entry = joints.Begin(); entry != joints.End(); ++entry) {
        IShutdownable* follower = 0;
        if (entry->object) {
            follower = static_cast<IShutdownable*>(entry->object->QueryInterface(&g_iidShutdownable));
            if (follower) {
                follower->AddRef();
            }
        }
        follower->Shutdown();
        follower->Release();
    }
    joints.items.ResizeBytes(0);
    return 0;
}

// EAX=this.
// FUNCTION: VC1 0x004a0680
KdTileObject::~KdTileObject() throw()
{
    Shutdown();
}

// ESI=this. No tile, white, visible, unclipped.
// FUNCTION: VC1 0x004a0c10
KdTileObject::KdTileObject() throw()
    : tile(-1), layer(0), color(-1), flags(0), visible(true)
{
}

// RET 4. Returns the new tile object as IAbstract.
// FUNCTION: VC1 0x004a0d00
void** CreateKdTileObject(void** out)
{
    void* const memory = malloc(sizeof(KdTileObject));
    KdTileObject* const object = memory ? new (memory) KdTileObject() : 0;
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

// Inferred name. The lazily created "TileManager" component in this file's
// own static, returned by reference.
// FUNCTION: VC1 0x004a0d50
static KdPtr<ITileManager>& TileManagerRef()
{
    static KdPtr<ITileManager> manager(QueryKdComponent(&g_kdNameTileManager));
    return manager;
}

// RET 0xc. Script "HitTest": point in UI coordinates against the tile at
// the position's UI translation, per pixel or by the tile rect.
// FUNCTION: VC1 0x004a0de0
bool KdTileObject::HitTest(const Vector2& point, const KdPtr<ICamera>& camera, bool precise)
{
    if (visible && positionable.ptr) {
        ICamera* const view = camera.ptr;
        const Matrix3& position = positionable.ptr->GetPosition();
        const Matrix3& m = view->GetWorldToUI();
        const Vector2 at(m._21 * position._32 + m._11 * position._31 + m._31,
            m._22 * position._32 + m._12 * position._31 + m._32);
        ITileManager* const tiles = TileManagerRef().ptr;

        if (precise) {
            return tiles->HitTest(tile, static_cast<int>(point.x - at.x),
                static_cast<int>(point.y - at.y), 0);
        }
        return tiles->GetRect(tile, 0).IsPointIn(static_cast<int>(point.x - at.x),
            static_cast<int>(point.y - at.y));
    }
    return false;
}

// IRenderable+0x10, RET 4. The tile at the position's screen translation;
// a non-empty clip rect (mapped through the camera, slot +0x28) replaces the
// graphics clip for the draw.
// FUNCTION: VC1 0x004a0ee0
int KdTileObject::Render(void* graphics)
{
    const KdPtr<IGraphics>& target = *static_cast<const KdPtr<IGraphics>*>(graphics);

    if (visible && positionable.ptr && tile != -1) {
        static Rect savedClip;
        const bool clipped = !(clip.width <= 0 || clip.height <= 0);

        if (clipped) {
            savedClip = target.ptr->GetClipRect();
            const Matrix3& m = static_cast<ICamera*>(*target.ptr->GetCamera())->GetScreenToUI();
            const Vector2 origin = m.MultVrt(Vector2(static_cast<SQFloat>(clip.x),
                static_cast<SQFloat>(clip.y)));
            const SQFloat w = static_cast<SQFloat>(clip.width);
            const SQFloat h = static_cast<SQFloat>(clip.height);
            const Vector2 extent(m._21 * h + m._11 * w, m._22 * h + m._12 * w);
            const Rect rect(static_cast<SQInteger>(origin.x), static_cast<SQInteger>(origin.y),
                static_cast<SQInteger>(extent.x), static_cast<SQInteger>(extent.y));
            target.ptr->SetClipRect(rect);
        }
        const Matrix3& position = positionable.ptr->GetPosition();
        const Matrix3& m = static_cast<ICamera*>(*target.ptr->GetCamera())->GetWorldToScreen();
        const SQFloat x = m._21 * position._32 + m._11 * position._31 + m._31;
        const SQFloat y = m._22 * position._32 + m._12 * position._31 + m._32;
        target.ptr->SetColor(color);
        TileManagerRef().ptr->Render(target, tile, x, y, flags);
        if (clipped) {
            target.ptr->SetClipRect(savedClip);
        }
    }
    return 0;
}

// IBoundingInfo+0x0c, RET 4 (ECX = IBoundingInfo face). The tile rect's
// corners through the position, as a static rect.
// FUNCTION: VC1 0x004a10d0
const RectF& KdTileObject::GetBoundingRect(const RenderMask&)
{
    static RectF bounds;
    bounds = RectF(0.0f, 0.0f, 0.0f, 0.0f);
    const Rect rect = TileManagerRef().ptr->GetRect(tile, 0);
    const Matrix3& m = positionable.ptr ? positionable.ptr->GetPosition() : Matrix3::Identity;

    bounds.Include(TransformPoint(m, Vector2(static_cast<SQFloat>(rect.x),
        static_cast<SQFloat>(rect.y))));
    bounds.Include(TransformPoint(m, Vector2(static_cast<SQFloat>(rect.x + rect.width),
        static_cast<SQFloat>(rect.y))));
    bounds.Include(TransformPoint(m, Vector2(static_cast<SQFloat>(rect.x),
        static_cast<SQFloat>(rect.y + rect.height))));
    bounds.Include(TransformPoint(m, Vector2(static_cast<SQFloat>(rect.x + rect.width),
        static_cast<SQFloat>(rect.y + rect.height))));
    return bounds;
}

// ITileObject+0x34, RET 8 (hidden result); script "GetJoint". The follower
// of the tile's joint point (created on first use), or the null object
// when the tile has no such joint.
// FUNCTION: VC1 0x004a1310
KdPtr<IUnknown> KdTileObject::GetJoint(int joint)
{
    TilePoint point;
    KdTileJoint* entry;

    point.x = 0;
    point.y = 0;
    if (!TileManagerRef().ptr->GetJoint(tile, joint, &point, 0)) {
        return KdPtr<IUnknown>(g_kdTileObjectNullObject.ptr);
    }
    entry = joints.Find(joint);
    if (entry == joints.End()) {
        void* created;
        KdTileJoint record;

        CreateKdTileJointObject(&created, this, joint);
        record.joint = joint;
        record.object = static_cast<IUnknown*>(created);
        if (record.object) {
            record.object->AddRef();
        }
        entry = joints.Insert(record).slot;
        if (record.object) {
            record.object->Release();
        }
        record.object = 0;
        if (created) {
            static_cast<IUnknown*>(created)->Release();
        }
    }
    return KdPtr<IUnknown>(entry->object);
}

// RET 8. The method table (guard 0x004ee760); the ITileObject methods bind
// through vcall thunks.
// FUNCTION: VC1 0x004a1410
unsigned char KdTileObject::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;

    if (methods.Empty()) {
        methods[kScriptGetTile] = CreateScriptMethod<KdTileObject>(
            static_cast<int (KdTileObject::*)()>(&ITileObject::GetTile));
        methods[kScriptSetTile] = CreateScriptMethod<KdTileObject>(
            static_cast<void (KdTileObject::*)(int)>(&ITileObject::SetTile));
        methods[kScriptSetRenderLayer] = CreateScriptMethod<KdTileObject>(
            static_cast<void (KdTileObject::*)(int)>(&ITileObject::SetRenderLayer));
        methods[kScriptSetColor] = CreateScriptMethod<KdTileObject>(
            static_cast<void (KdTileObject::*)(int)>(&ITileObject::SetColor));
        methods[kScriptSetHighlighted] = CreateScriptMethod<KdTileObject>(
            static_cast<void (KdTileObject::*)(bool)>(&ITileObject::SetHighlighted));
        methods[kScriptEnableFloatCoords] = CreateScriptMethod<KdTileObject>(
            static_cast<void (KdTileObject::*)(bool)>(&ITileObject::EnableFloatCoords));
        methods[kScriptGetJoint] = CreateScriptMethod<KdTileObject>(
            static_cast<KdPtr<IUnknown> (KdTileObject::*)(int)>(&ITileObject::GetJoint));
        methods[kScriptSetClip] = CreateScriptMethod<KdTileObject>(&KdTileObject::SetClip);
        methods[kScriptHitTest] = CreateScriptMethod<KdTileObject>(&KdTileObject::HitTest);
    }
    return methods.Find(name, out);
}

// IPositionable+0x10. The owner's position moved to its tile's joint point.
// FUNCTION: VC1 0x004a1860
const Matrix3& KdTileJointObject::GetPosition()
{
    if (owner.ptr) {
        TilePoint point;

        if (owner.ptr->positionable.ptr) {
            position = owner.ptr->positionable.ptr->GetPosition();
        }
        point.x = 0;
        point.y = 0;
        if (TileManagerRef().ptr->GetJoint(owner.ptr->tile, joint, &point, 0)) {
            const Vector2 p(static_cast<SQFloat>(point.x), static_cast<SQFloat>(point.y));
            const Vector2 t(position._21 * p.y + position._11 * p.x + position._31,
                position._22 * p.y + position._12 * p.x + position._32);
            position._31 = t.x;
            position._32 = t.y;
        }
    }
    return position;
}

}
