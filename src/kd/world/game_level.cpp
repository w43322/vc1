#include "kd/world/game_level.h"

#include "kd/iface/iclassifiable.h"
#include "kd/iface/icamera.h"
#include "kd/iface/idisplay.h"
#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iid.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/itile_object.h"
#include "kd/group/ptr_set.h"
#include "kdcore/runtime.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"
#include "bindings/vector2.h"

#include <limits>
#include <new>
#include <stdlib.h>
#include "kdcore/native.h"

namespace vc1 {

extern const char kCameraName[];

// STRING: VC1 0x004cfdbc
static const char kGameLevel[] = "GameLevel";
// GLOBAL: VC1 0x004d0aec
const char* g_kdNameGameLevel = kGameLevel;

// STRING: VC1 0x004d0da8
static const char kScriptSetDynamicLayer[] = "SetDynamicLayer";
// STRING: VC1 0x004d0db8
static const char kScriptFindObjectsByMask[] = "FindObjectsByMask";
// STRING: VC1 0x004d0dcc
static const char kScriptGetObjectAt[] = "GetObjectAt";
extern const char kScriptHitTest[];

// Inferred name. The objects GetObjectAt considers are at most this far
// (in the larger axis) from the picked point.
static const double kHitTestRange = 350.0;

// The group's IIdentifiable name, or g_kdEmptyIdentifier when it has none
// (compared by IAbstract identity with g_kdNullSceneIdentifier).
static inline const KdString& GroupName(IIdentifiable* ident)
{
    if (PtrEqual(ident, g_kdNullSceneIdentifier.ptr)) {
        return g_kdEmptyIdentifier;
    }
    return ident->GetName();
}

static inline IIdentifiable* QueryIdentifiable(IUnknown* group)
{
    IIdentifiable* ident = group
        ? static_cast<IIdentifiable*>(group->QueryInterface(&g_iidIdentifiable)) : 0;
    if (ident) {
        ident->AddRef();
    }
    return ident;
}

// Whether object's classificators contain every include bit and no exclude
// bit; an object without IClassifiable matches only an empty include mask.
static inline bool MatchesMask(IUnknown* object, unsigned include, unsigned exclude)
{
    IClassifiable* classifiable = object
        ? static_cast<IClassifiable*>(object->QueryInterface(&g_iidClassifiable)) : 0;
    bool match;

    if (classifiable) {
        unsigned value;

        classifiable->AddRef();
        value = classifiable->GetClassificators();
        match = (value & include) == include && !(value & exclude);
    }
    else {
        match = include == 0;
    }
    if (classifiable) {
        classifiable->Release();
    }
    return match;
}

// p through m as a 2D affine transform (rows 0/1 linear, row 2 translation).
static inline Vector2 TransformPoint(const Matrix3& m, float x, float y)
{
    return Vector2(y * m._21 + x * m._11 + m._31, y * m._22 + x * m._12 + m._32);
}

static inline float AbsFloat(float value)
{
    return value < 0.0f ? -value : value;
}

// MOV EAX, "GameLevel"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00403ad0
const char* KdGameLevel::GetTypeName()
{
    return kGameLevel;
}

// ESI=this, stack &key, RET 4. The entry's group index, inserted as
// {key, 0} before the lower bound when key is missing.
// FUNCTION: VC1 0x00403b00
int& KdLayerMap::operator[](const int& key)
{
    KdLayerEntry* const oldBegin = Begin();
    KdLayerEntry* it = LowerBound(key);
    KdLayerEntry* inserted;
    int value;

    if (it != End() && !(key < it->layer)) {
        return it->groupIndex;
    }
    value = key;
    Reserve((items.size >> 3) + 1);
    inserted = static_cast<KdLayerEntry*>(InsertCountedBytes(&items,
        reinterpret_cast<unsigned char*>(Begin())
            + (reinterpret_cast<unsigned char*>(it) - reinterpret_cast<unsigned char*>(oldBegin)),
        sizeof(KdLayerEntry)));
    inserted->layer = value;
    inserted->groupIndex = 0;
    return inserted->groupIndex;
}

// EAX=this, stack &key, RET 4. First entry whose layer is not below key.
// FUNCTION: VC1 0x00403cf0 FOLDED
KdLayerEntry* KdLayerMap::LowerBound(const int& key)
{
    KdLayerEntry* first = Begin();
    KdLayerEntry* last = End();

    while (first != last) {
        KdLayerEntry* const middle = first + (last - first) / 2;
        if (middle->layer < key) {
            first = middle + 1;
        }
        else {
            last = middle;
        }
    }
    return first;
}

// ECX=this, EAX=count. Loose grow; the new entries are zeroed.
// FUNCTION: VC1 0x0040b080 FOLDED
void KdLayerMap::Reserve(unsigned count)
{
    const unsigned old = items.capacity >> 3;
    KdLayerEntry* entry;
    KdLayerEntry* end;

    GrowCountedBuffer(&items, static_cast<int>(count * sizeof(KdLayerEntry)), 0);
    if ((items.capacity >> 3) > old) {
        entry = Begin() + old;
        end = Begin() + (items.capacity >> 3);
        for (; entry < end; ++entry) {
            if (entry) {
                entry->layer = 0;
                entry->groupIndex = 0;
            }
        }
    }
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x004b0c50
unsigned char KdGameLevel::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// RET 8. IUpdateable vtable+0x14, forwarded to the update group.
// FUNCTION: VC1 0x004b0c60
void KdGameLevel::Update(unsigned sliceBit, int dt)
{
    if (updateGroup.ptr) {
        updateGroup->Update(sliceBit, dt);
    }
}

// FUNCTION: VC1 0x004b0cb0
unsigned KdGameLevel::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x004b0cd0
unsigned KdGameLevel::AddRef()
{
    return IAbstract::AddRef();
}

// Hidden return, RET 0xc. Script "FindObjectsByMask": every registered
// object whose classificators match (see MatchesMask), in record order.
// FUNCTION: VC1 0x004b0ce0
KdRefList KdGameLevel::FindObjectsByMask(unsigned include, unsigned exclude)
{
    KdRefList result;
    NamedRecord* it;

    for (it = reinterpret_cast<NamedRecord*>(records.items.begin);
        it != reinterpret_cast<NamedRecord*>(
            reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size);
        ++it) {
        if (MatchesMask(it->object.ptr, include, exclude)) {
            const unsigned bytes = ((result.items.size >> 2) + 1) * 4;
            IUnknown** slot;

            if (result.items.capacity < bytes) {
                const unsigned old = result.items.capacity >> 2;
                IUnknown** end;

                GrowCountedBuffer(&result.items, static_cast<int>(bytes), 0);
                if ((result.items.capacity >> 2) > old) {
                    slot = RefBegin(&result) + old;
                    end = RefBegin(&result) + (result.items.capacity >> 2);
                    for (; slot < end; ++slot) {
                        if (slot) {
                            *slot = 0;
                        }
                    }
                }
            }
            result.items.size = bytes;
            slot = reinterpret_cast<IUnknown**>(
                reinterpret_cast<unsigned char*>(result.items.begin) + bytes - 4);
            if (it->object.ptr) {
                it->object->AddRef();
            }
            if (*slot) {
                (*slot)->Release();
            }
            *slot = it->object.ptr;
        }
    }
    return result;
}

// RET 4. Primary vftable +0x1c. A tile object whose layer has a
// SetDynamicLayer group leaves that group in place of the render group.
// FUNCTION: VC1 0x004b0e30
void KdGameLevel::RemoveObject(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    KdLayerEntry* found = layers.End();
    ITileObject* tile;

    if (source) {
        tile = static_cast<ITileObject*>(source->QueryInterface(&g_iidTileObject));
        if (tile) {
            int layer;
            KdLayerEntry* it;

            tile->AddRef();
            layer = tile->GetRenderOrder();
            it = layers.LowerBound(layer);
            found = it < layers.End() && !(layer < it->layer) ? it : layers.End();
        }
    }
    else {
        tile = 0;
    }
    if (found != layers.End()) {
        groupMask = (groupMask | (1u << found->groupIndex)) & ~(1u << renderGroupIndex);
    }
    if (tile) {
        tile->Release();
    }
    KdScene::RemoveObject(object);
    if (found != layers.End()) {
        groupMask = (groupMask & ~(1u << found->groupIndex)) | (1u << renderGroupIndex);
    }
}

// RET 4. Primary vftable +0x18. A tile object whose layer has a
// SetDynamicLayer group joins that group in place of the render group.
// FUNCTION: VC1 0x004b0f30
void KdGameLevel::AddObject(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    KdLayerEntry* found = layers.End();
    ITileObject* tile;

    if (source) {
        tile = static_cast<ITileObject*>(source->QueryInterface(&g_iidTileObject));
        if (tile) {
            int layer;
            KdLayerEntry* it;

            tile->AddRef();
            layer = tile->GetRenderOrder();
            it = layers.LowerBound(layer);
            found = it < layers.End() && !(layer < it->layer) ? it : layers.End();
        }
    }
    else {
        tile = 0;
    }
    if (found != layers.End()) {
        groupMask = (groupMask | (1u << found->groupIndex)) & ~(1u << renderGroupIndex);
    }
    if (tile) {
        tile->Release();
    }
    KdScene::AddObject(object);
    if (found != layers.End()) {
        groupMask = (groupMask & ~(1u << found->groupIndex)) | (1u << renderGroupIndex);
    }
}

// RET 8. Script "SetDynamicLayer": appends group, maps layer to its index,
// and keeps it disabled for ordinary objects (only tile objects of that
// layer join it, see AddObject).
// FUNCTION: VC1 0x004b1030
void KdGameLevel::SetDynamicLayer(int layer, const KdPtr<IFunctionalGroup>& group)
{
    IIdentifiable* ident;
    int index;

    AppendGroup(KdPtr<IAbstract>(group));
    ident = QueryIdentifiable(group.ptr);
    {
        const KdString& name = GroupName(ident);
        int& slot = layers[layer];
        slot = FindSceneGroup(this, name);
    }
    if (ident) {
        ident->Release();
    }
    ident = QueryIdentifiable(group.ptr);
    {
        const KdString& name = GroupName(ident);
        index = FindSceneGroup(this, name);
        if (index == -1) {
            HostLogError(&g_kdLogPrefix, kCantFindGroup, name.c_str());
        }
        else {
            groupMask &= ~(1u << index);
        }
    }
    if (ident) {
        ident->Release();
    }
}

// KdComponentGroup vtable+0x0c: empties the layer table, then KdScene's.
// FUNCTION: VC1 0x004b11e0
int KdGameLevel::Shutdown()
{
    if (layers.items.capacity < layers.items.Count()) {
        GrowCountedBuffer(&layers.items, 0, 1);
    }
    layers.items.size = 0;
    return KdScene::Shutdown();
}

// The layer table's KdFactory destructor, then ~KdScene.
// FUNCTION: VC1 0x004b1210
KdGameLevel::~KdGameLevel() throw()
{
}

// EAX=this. Finds the render group's index and reads the display's client
// size.
// FUNCTION: VC1 0x004b1470
KdGameLevel::KdGameLevel() throw()
{
    IIdentifiable* ident = QueryIdentifiable(renderGroup.ptr);
    void* unknown;

    renderGroupIndex = FindSceneGroup(this, GroupName(ident));
    if (ident) {
        ident->Release();
    }
    GetKdFactory()->QueryNamed(&unknown, &g_kdNameDisplay);
    {
        IDisplay* display = unknown
            ? static_cast<IDisplay*>(static_cast<IUnknown*>(unknown)->QueryInterface(&g_iidDisplay))
            : 0;
        RECT client;

        if (display) {
            display->AddRef();
        }
        if (GetClientRect(static_cast<HWND*>(display->GetMixer())[1], &client)) {
            clientWidth = client.right - client.left;
            clientHeight = client.bottom - client.top;
        }
        display->Release();
    }
    if (unknown) {
        static_cast<IUnknown*>(unknown)->Release();
    }
}

// FUNCTION: VC1 0x004b1600
void** CreateKdGameLevel(void** out)
{
    KdGameLevel* memory = static_cast<KdGameLevel*>(malloc(sizeof(KdGameLevel)));
    IUnknown* queried;

    if (memory) {
        new (memory) KdGameLevel();
    }
    queried = memory
        ? static_cast<IUnknown*>(static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract))
        : 0;
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

// RET 4. IScriptLoadable vtable+0x0c, owner = the IScriptObject face.
// FUNCTION: VC1 0x004b1660
bool KdGameLevel::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this)) != 0;
}

// Hidden return, RET 0x10. Script "GetObjectAt": the front-most (largest
// view y) matching object near point whose script "HitTest"(point) is true.
// Point goes through the camera's GetUIToScreen, object positions through
// its view matrix. Without a camera, or with no hit, the null identifier.
// FUNCTION: VC1 0x004b1680
KdPtr<IUnknown> KdGameLevel::GetObjectAt(const Vector2& point, unsigned include, unsigned exclude)
{
    KdPtr<ICamera> camera(FindObject(KdString(kCameraName)));
    IUnknown* best;
    float bestDepth;

    best = g_kdNullSceneIdentifier.ptr;
    if (best) {
        best->AddRef();
    }
    bestDepth = -std::numeric_limits<float>::infinity();
    if (!camera.ptr) {
        KdPtr<IUnknown> result(best);
        if (best) {
            best->Release();
        }
        return result;
    }
    {
        const Matrix3& view = camera->GetWorldToScreen();
        const Matrix3& screen = camera->GetUIToScreen();
        const Vector2 target = TransformPoint(screen, point.x, point.y);
        NamedRecord* it;

        for (it = reinterpret_cast<NamedRecord*>(records.items.begin);
            it != reinterpret_cast<NamedRecord*>(
                reinterpret_cast<unsigned char*>(records.items.begin) + records.items.size);
            ++it) {
            IPositionable* positionable;
            Vector2 at;
            float dx;
            float dy;
            float distance;
            bool hit;

            if (!MatchesMask(it->object.ptr, include, exclude)) {
                continue;
            }
            positionable = it->object.ptr
                ? static_cast<IPositionable*>(it->object->QueryInterface(&g_iidPositionable)) : 0;
            if (positionable) {
                positionable->AddRef();
            }
            {
                const Matrix3& position = positionable->GetPosition();
                at = TransformPoint(view, position._31, position._32);
            }
            if (!(bestDepth < at.y)) {
                positionable->Release();
                continue;
            }
            dy = AbsFloat(target.y - at.y);
            dx = AbsFloat(target.x - at.x);
            distance = dy < dx ? dx : dy;
            if (distance > kHitTestRange) {
                positionable->Release();
                continue;
            }
            {
                KdPtr<IScriptObject> object(it->object.ptr
                    ? static_cast<IScriptObject*>(it->object->QueryInterface(&g_iidScriptObject))
                    : 0);
                SquirrelObject member;

                object->GetMember(kScriptHitTest, &member);
                {
                    SquirrelObject self(object);
                    SquirrelVM::BeginCall(member, self);
                    {
                        SquirrelObject argument(point);
                        SquirrelVM::PushParam(argument);
                        hit = SquirrelVM::EndCall().ToBool();
                    }
                }
            }
            if (hit) {
                if (it->object.ptr) {
                    it->object->AddRef();
                }
                if (best) {
                    best->Release();
                }
                bestDepth = at.y;
                best = it->object.ptr;
            }
            positionable->Release();
        }
    }
    {
        KdPtr<IUnknown> result(best);
        if (best) {
            best->Release();
        }
        return result;
    }
}

// RET 8. The script instance's members, then KdScene's, then the method
// table (table 0x004ee6cc, guard bit 0x004ee6d8).
// FUNCTION: VC1 0x004b1ad0
unsigned char KdGameLevel::GetMember(const char* name, SquirrelObject* out)
{
    static ScriptMethodTable methods;
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    if (KdScene::GetMember(name, out)) {
        return 1;
    }
    if (methods.Empty()) {
        methods[kScriptSetDynamicLayer] =
            CreateScriptMethod<KdGameLevel>(&KdGameLevel::SetDynamicLayer);
        methods[kScriptFindObjectsByMask] =
            CreateScriptMethod<KdGameLevel>(&KdGameLevel::FindObjectsByMask);
        methods[kScriptGetObjectAt] = CreateScriptMethod<KdGameLevel>(&KdGameLevel::GetObjectAt);
    }
    return methods.Find(name, out);
}

}
