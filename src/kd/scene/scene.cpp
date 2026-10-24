#include "kd/scene/scene.h"

#include "kd/iface/ifunctional_group.h"
#include "kd/iface/iid.h"
#include "kd/group/ptr_set.h"
#include "kdcore/platform.h"
#include "bindings/component.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004e988c
KdString g_kdSceneEmptyName = KdEmptyText();

int MemCompare(const void* left, const void* right, unsigned size);

// STRING: VC1 0x004d4b80
static const char kScene[] = "Scene";
// GLOBAL: VC1 0x004d067c
const char* g_kdNameScene = kScene;

// GLOBAL: VC1 0x004ed6d8
KdRef<IUnknown> g_kdNullSceneObject;

// GLOBAL: VC1 0x004ed664
KdRef<IUnknown> g_kdNullSceneIdentifier;

// Inferred name. The identifier of a group without an IIdentifiable face.
// GLOBAL: VC1 0x004e9630
KdString g_kdEmptyIdentifier = KdEmptyText();

// STRING: VC1 0x004d5410
static const char kObjectLoaded[] = "Object %s loaded";
// STRING: VC1 0x004d5438
static const char kObjectDeleted[] = "Object %s deleted";
// Also GameLevel "SetDynamicLayer".
// GLOBAL: VC1 0x004d544c
extern const char kCantFindGroup[] = "can't find group: %s";
// STRING: VC1 0x004d53d0
static const char kScriptAppendGroup[] = "AppendGroup";
// STRING: VC1 0x004d53dc
static const char kScriptRemoveGroup[] = "RemoveGroup";
// STRING: VC1 0x004d53e8
static const char kScriptEnableGroup[] = "EnableGroup";

static inline bool IsSceneNull(IUnknown* object)
{
    return PtrEqual(object, g_kdNullSceneObject.ptr);
}

// Release slot, then the system component name's iid face (counted), or 0.
template <class T>
static inline void QueryGroup(KdRef<T>& slot, const char** name, const char** iid)
{
    void* unknown;

    GetKdFactory()->QueryNamed(&unknown, name);
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!unknown) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(static_cast<IUnknown*>(unknown)->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
    static_cast<IUnknown*>(unknown)->Release();
}

// Release slot, then the iid face of g_kdNullSceneObject (counted), or 0.
template <class T>
static inline void ResetGroup(KdRef<T>& slot, const char** iid)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!g_kdNullSceneObject.ptr) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<T*>(g_kdNullSceneObject.ptr->QueryInterface(iid));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

// The group's IFunctionalGroup face (counted) appended to list.
static inline void AppendFunctionalGroup(KdRefList* list, IUnknown* group)
{
    IUnknown* face = group
        ? static_cast<IUnknown*>(group->QueryInterface(&g_iidFunctionalGroup)) : 0;
    if (face) {
        face->AddRef();
    }
    PtrListAppend(list, &face);
    if (face) {
        face->Release();
    }
}

static inline IVisible* QueryVisible(IRenderable* group)
{
    IVisible* visible = group
        ? static_cast<IVisible*>(group->QueryInterface(&g_iidVisible)) : 0;
    if (visible) {
        visible->AddRef();
    }
    return visible;
}

// XOR EAX,EAX; RET. IRenderable vtable+0x14; ICF-folded return-0.
// FUNCTION: VC1 0x0041adb0 FOLDED
int KdScene::GetRenderOrder()
{
    return 0;
}

// MOV EAX, "Scene"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00417440
const char* KdScene::GetTypeName()
{
    return kScene;
}

// RET 4. IRenderable vtable+0x10: RenderMasked with the default mask.
// FUNCTION: VC1 0x0046e2b0
int KdScene::Render(void* graphics)
{
    return RenderMasked(graphics, &g_defaultRenderMask);
}

// OR EAX, -1; RET. IUpdateable vtable+0x0c.
// FUNCTION: VC1 0x0046e2d0
int KdScene::GetUpdateLayer()
{
    return -1;
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call).
// FUNCTION: VC1 0x0046e2e0
unsigned char KdScene::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// IVisible vtable+0x10: the render group's IVisible, or false.
// FUNCTION: VC1 0x0046e2f0
bool KdScene::IsVisible()
{
    IVisible* visible;
    bool result;

    if (!renderGroup.ptr) {
        return false;
    }
    visible = QueryVisible(renderGroup.ptr);
    result = visible->IsVisible();
    visible->Release();
    return result;
}

// RET 4. IVisible vtable+0x0c, forwarded to the render group's IVisible.
// FUNCTION: VC1 0x0046e340
void KdScene::SetVisible(bool value)
{
    IVisible* visible;

    if (!renderGroup.ptr) {
        return;
    }
    visible = QueryVisible(renderGroup.ptr);
    visible->SetVisible(value);
    visible->Release();
}

// RET 4. IInputHandler vtable+0x10, forwarded to the input group.
// FUNCTION: VC1 0x0046e390
unsigned char KdScene::OnPointer(const unsigned* data)
{
    if (!inputGroup.ptr) {
        return 0;
    }
    return inputGroup->OnPointer(data);
}

// RET 4. IInputHandler vtable+0x0c, forwarded to the input group.
// FUNCTION: VC1 0x0046e3b0
unsigned char KdScene::OnKey(const unsigned* data)
{
    if (!inputGroup.ptr) {
        return 0;
    }
    return inputGroup->OnKey(data);
}

// RET 8. IRenderable vtable+0x0c, forwarded to the render group.
// FUNCTION: VC1 0x0046e3d0
int KdScene::RenderMasked(void* graphics, void* mask)
{
    if (!renderGroup.ptr) {
        return 0;
    }
    return renderGroup->RenderMasked(graphics, mask);
}

// RET 8. IUpdateable vtable+0x14, forwarded to the update group.
// FUNCTION: VC1 0x0046e3f0
void KdScene::Update(unsigned sliceBit, int dt)
{
    if (updateGroup.ptr) {
        updateGroup->Update(sliceBit, dt);
    }
}

// RET 4. IUpdateable vtable+0x10, forwarded to the update group.
// FUNCTION: VC1 0x0046e410
int KdScene::SetLayer(unsigned layer)
{
    if (!updateGroup.ptr) {
        return 0;
    }
    return updateGroup->SetLayer(layer);
}

// FUNCTION: VC1 0x0046e4f0
unsigned KdScene::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x0046e510
unsigned KdScene::AddRef()
{
    return IAbstract::AddRef();
}

// RET 8 (stack scene, name). Index of the group whose IIdentifiable name is
// name (an unnamed group compares as g_kdEmptyIdentifier), or -1.
// FUNCTION: VC1 0x0046e520
int FindSceneGroup(KdScene* scene, const KdString& name)
{
    unsigned i;

    for (i = 0; i < scene->groups.items.size >> 2; ++i) {
        IUnknown* group = RefBegin(&scene->groups)[i];
        IIdentifiable* ident = group
            ? static_cast<IIdentifiable*>(group->QueryInterface(&g_iidIdentifiable)) : 0;
        const KdString* other;
        bool same;

        if (ident) {
            ident->AddRef();
        }
        if (PtrEqual(ident, g_kdNullSceneIdentifier.ptr)) {
            other = &g_kdEmptyIdentifier;
        }
        else {
            other = &ident->GetName();
        }
        same = name.size == other->size
            && (name.begin == other->begin
                || MemCompare(name.c_str(), other->c_str(), name.size) == 0);
        if (ident) {
            ident->Release();
        }
        if (same) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// RET 8. Script "EnableGroup": sets or clears the group's groupMask bit.
// FUNCTION: VC1 0x0046e620
void KdScene::EnableGroup(const KdString& name, bool enable)
{
    const int index = FindSceneGroup(this, name);

    if (index == -1) {
        HostLogError(&g_kdLogPrefix, kCantFindGroup, name.c_str());
        return;
    }
    if (enable) {
        groupMask |= 1u << index;
    }
    else {
        groupMask &= ~(1u << index);
    }
}

// RET 4. Primary vftable +0x1c: children first (through the object's
// component group), then the object leaves every enabled group.
// FUNCTION: VC1 0x0046e680
void KdScene::RemoveObject(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    IComponentGroup* group = source
        ? static_cast<IComponentGroup*>(source->QueryInterface(&g_iidComponentGroup)) : 0;
    unsigned i;

    if (group) {
        group->AddRef();
    }
    if (!IsSceneNull(group)) {
        KdFactory* records = static_cast<KdFactory*>(group->GetRecords());
        unsigned char* it;

        for (it = reinterpret_cast<unsigned char*>(records->begin);
            it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
            it += sizeof(NamedRecord)) {
            RemoveObject(reinterpret_cast<void**>(it + 0x10));
        }
    }
    for (i = 0; i < groups.items.size >> 2; ++i) {
        if (groupMask & (1u << i)) {
            static_cast<IFunctionalGroup*>(RefBegin(&groups)[i])->Remove(*reinterpret_cast<const KdPtr<IAbstract>*>(object));
        }
    }
    if (group) {
        group->Release();
    }
}

// RET 4. Primary vftable +0x18: children first (through the object's
// component group), then the object joins every enabled group.
// FUNCTION: VC1 0x0046e790
void KdScene::AddObject(void** object)
{
    IUnknown* const source = static_cast<IUnknown*>(*object);
    IComponentGroup* group = source
        ? static_cast<IComponentGroup*>(source->QueryInterface(&g_iidComponentGroup)) : 0;
    unsigned i;

    if (group) {
        group->AddRef();
    }
    if (!IsSceneNull(group)) {
        KdFactory* records = static_cast<KdFactory*>(group->GetRecords());
        unsigned char* it;

        for (it = reinterpret_cast<unsigned char*>(records->begin);
            it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
            it += sizeof(NamedRecord)) {
            AddObject(reinterpret_cast<void**>(it + 0x10));
        }
    }
    for (i = 0; i < groups.items.size >> 2; ++i) {
        if (groupMask & (1u << i)) {
            static_cast<IFunctionalGroup*>(RefBegin(&groups)[i])->Add(*reinterpret_cast<const KdPtr<IAbstract>*>(object));
        }
    }
    if (group) {
        group->Release();
    }
}

// RET 4. Script "AppendGroup": the group's IFunctionalGroup face.
// FUNCTION: VC1 0x0046e8a0
void KdScene::AppendGroup(const KdPtr<IAbstract>& group)
{
    AppendFunctionalGroup(&groups, group.ptr);
}

// RET 4. Script "RemoveGroup": drops every slot equal to group.
// FUNCTION: VC1 0x0046e900
void KdScene::RemoveGroup(const KdPtr<IAbstract>& group)
{
    IUnknown** const first = RefBegin(&groups);
    IUnknown** const last = RefEnd(&groups);
    IUnknown** const kept = PtrRemove(first, last,
        reinterpret_cast<IUnknown* const*>(&group.ptr));
    const unsigned removed = static_cast<unsigned>(last - kept) * 4;

    EraseCountedBytes(kept, kept + (last - kept), RefEnd(&groups));
    groups.items.size -= removed;
}

// KdComponentGroup vtable+0x0c. Detaches the three system groups, empties the
// group list, shuts the children down, then the script instance.
// FUNCTION: VC1 0x0046e960
int KdScene::Shutdown()
{
    IScriptInstanceReset* reset;

    ResetGroup(renderGroup, &g_iidRenderable);
    ResetGroup(updateGroup, &g_iidUpdateable);
    ResetGroup(inputGroup, &g_iidInputHandler);
    RefClear(&groups);
    KdComponentGroup::Shutdown();
    reset = &script;
    reset->Reset();
    return 0;
}

// FUNCTION: VC1 0x0046ea50
KdScene::~KdScene() throw()
{
    IScriptInstanceReset* reset = &script;
    reset->Reset();
}

// RET 8. KdComponentGroup::FindObject (0x00472a70).
// FUNCTION: VC1 0x0046ed40
KdPtr<IUnknown> KdScene::FindObject(const KdString& name)
{
    return KdComponentGroup::FindObject(name);
}

// RET 8. Removes key; on success the object also leaves the scene's groups.
// FUNCTION: VC1 0x0046ed60
KdPtr<IUnknown> KdScene::UnregisterObject(const KdString& name)
{
    KdPtr<IUnknown> found = FindObject(name);

    if (!found.ptr) {
        HostLogError(&g_kdNameScene, kSystemObjectNotFound, name.c_str());
    }
    else {
        RemoveObject(reinterpret_cast<void**>(&found.ptr));
        KdComponentGroup::UnregisterObject(name);
        ClearCollected();
        HostLog(&g_kdNameScene, kObjectDeleted, name.c_str());
    }
    return found;
}

// RET 8. Refuses an existing key; otherwise names an IIdentifiable object
// after key, inserts it and adds it to the scene's groups.
// FUNCTION: VC1 0x0046ee10
void KdScene::RegisterObject(const KdString& name, const KdPtr<IAbstract>& object)
{
    IIdentifiable* ident;
    bool exists;

    {
        KdPtr<IUnknown> existing = FindObject(name);
        exists = !IsSceneNull(existing.ptr);
    }
    if (exists) {
        HostLogError(&g_kdNameScene, kSystemObjectExists, name.c_str());
        return;
    }
    ident = object.ptr
        ? static_cast<IIdentifiable*>(object.ptr->QueryInterface(&g_iidIdentifiable)) : 0;
    if (ident) {
        ident->AddRef();
    }
    if (!IsSceneNull(ident)) {
        ident->SetName(name);
    }
    HostLog(&g_kdNameScene, kObjectLoaded, name.c_str());
    KdComponentGroup::RegisterObject(name, object);
    ClearCollected();
    AddObject(reinterpret_cast<void**>(const_cast<IAbstract**>(&object.ptr)));
    if (ident) {
        ident->Release();
    }
}

// ESI=this. Joins the system's update, render and input groups.
// FUNCTION: VC1 0x0046ef60
KdScene::KdScene() throw()
{
    QueryGroup(updateGroup, &g_kdNameUpdateGroup, &g_iidUpdateable);
    QueryGroup(renderGroup, &g_kdNameRenderGroup, &g_iidRenderable);
    QueryGroup(inputGroup, &g_kdNameInputGroup, &g_iidInputHandler);
    AppendFunctionalGroup(&groups, updateGroup.ptr);
    AppendFunctionalGroup(&groups, renderGroup.ptr);
    AppendFunctionalGroup(&groups, inputGroup.ptr);
    groupMask = 0xffffffff;
}

// FUNCTION: VC1 0x0046f270
void** CreateKdScene(void** out)
{
    KdScene* memory = static_cast<KdScene*>(malloc(sizeof(KdScene)));
    IUnknown* queried;

    if (memory) {
        new (memory) KdScene();
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

// RET 8. The script instance's members first, then the method table (table
// 0x004ee614, guard bit 0x004ee624).
// FUNCTION: VC1 0x0046f370
unsigned char KdScene::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptRegisterObject] = CreateScriptMethod<KdScene>(&KdScene::RegisterObject);
        methods[kScriptUnregisterObject] = CreateScriptMethod<KdScene>(&KdScene::UnregisterObject);
        methods[kScriptGetObject] = CreateScriptMethod<KdScene>(&KdScene::FindObject);
        methods[kScriptAppendGroup] = CreateScriptMethod<KdScene>(&KdScene::AppendGroup);
        methods[kScriptRemoveGroup] = CreateScriptMethod<KdScene>(&KdScene::RemoveGroup);
        methods[kScriptEnableGroup] = CreateScriptMethod<KdScene>(&KdScene::EnableGroup);
    }
    return methods.Find(name, out);
}

// RET 4. IScriptLoadable vtable+0x0c, owner = the IScriptObject face.
// FUNCTION: VC1 0x0046f5f0
bool KdScene::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this)) != 0;
}

}
