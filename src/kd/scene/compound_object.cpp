#include "kd/scene/compound_object.h"

#include "kd/iface/iunknown.h"
#include "kd/iface/iid.h"
#include "kd/factory.h"
#include "kdcore/platform.h"
#include "bindings/component.h"
#include "bindings/matrix3.h"
#include "bindings/sqplus.h"

#include <new>
#include <stdlib.h>
#include <string.h>

namespace vc1 {

// Per-file statics from the original's dynamic initializer table (inferred
// names and file; order follows the table): a null object, an empty name
// and, in the ScriptedObject files, two +Inf state defaults.
// GLOBAL: VC1 0x004ed6b8
KdRef<IUnknown> g_kdCompoundFaceNullObject;
// GLOBAL: VC1 0x004e985c
KdString g_kdCompoundEmptyName = KdEmptyText();


// STRING: VC1 0x004d4b90
static const char kCompoundObject[] = "CompoundObject";
// GLOBAL: VC1 0x004d0674
const char* g_kdNameCompoundObject = kCompoundObject;

// GLOBAL: VC1 0x004ed6bc
KdRef<IUnknown> g_kdNullObject;

// Release slot, then the IPositionable face of source (counted), or 0.
static inline void AssignPositionable(KdRef<IPositionable>& slot, IUnknown* source)
{
    if (slot.ptr) {
        slot.ptr->Release();
    }
    if (!source) {
        slot.ptr = 0;
        return;
    }
    slot.ptr = static_cast<IPositionable*>(source->QueryInterface(&g_iidPositionable));
    if (slot.ptr) {
        slot.ptr->AddRef();
    }
}

static inline KdFactory* ChildRecords(KdComponentGroup* group)
{
    return static_cast<KdFactory*>(group->GetRecords());
}

static inline IUnknown* RecordObject(unsigned char* record)
{
    return *reinterpret_cast<IUnknown**>(record + 0x10);
}

// MOV EAX, "CompoundObject"; RET. IScriptObject vtable+0x14.
// FUNCTION: VC1 0x00415aa0
const char* KdCompoundObject::GetTypeName()
{
    return kCompoundObject;
}

// EAX=&object. Whether object is g_kdNullObject: both null, or the same
// IAbstract face.
// FUNCTION: VC1 0x00415e00
bool IsNullObject(IUnknown* const& object)
{
    IUnknown* const nullObject = g_kdNullObject.ptr;

    if (object && nullObject) {
        return object->QueryInterface(&g_iidAbstract)
            == g_kdNullObject.ptr->QueryInterface(&g_iidAbstract);
    }
    return !object && !nullObject;
}

// EAX=&object. The negation of IsNullObject.
// FUNCTION: VC1 0x00415e50
bool IsNotNullObject(IUnknown* const& object)
{
    IUnknown* const nullObject = g_kdNullObject.ptr;

    if (object && nullObject) {
        return object->QueryInterface(&g_iidAbstract)
            != g_kdNullObject.ptr->QueryInterface(&g_iidAbstract);
    }
    return object || nullObject;
}

// IScriptObject vtable+0x10: the script instance's SetMember (tail call
// through its vftable).
// FUNCTION: VC1 0x00472e70
unsigned char KdCompoundObject::SetMember(void* name, void* value)
{
    IScriptInstance* instance = &script;
    return instance->SetMember(static_cast<const char*>(name),
        *static_cast<const SquirrelObject*>(value));
}

// IPositionable vtable+0x10. The provider's position, or identity.
// FUNCTION: VC1 0x00472e80
const Matrix3& KdCompoundObject::GetPosition()
{
    if (IsNullObject(reinterpret_cast<IUnknown* const&>(positionProvider.ptr))) {
        return Matrix3::Identity;
    }
    return positionProvider->GetPosition();
}

// RET 4. IPositionable vtable+0x0c. Forwarded to the provider, if any.
// FUNCTION: VC1 0x00472ea0
void KdCompoundObject::SetPosition(const Matrix3& value)
{
    if (IsNotNullObject(reinterpret_cast<IUnknown* const&>(positionProvider.ptr))) {
        positionProvider->SetPosition(value);
    }
}

// FUNCTION: VC1 0x00472f90
unsigned KdCompoundObject::Release()
{
    return IAbstract::Release();
}

// FUNCTION: VC1 0x00472fb0
unsigned KdCompoundObject::AddRef()
{
    return IAbstract::AddRef();
}

// RET 4. Script "SetPositionProvider": the provider's IPositionable face.
// FUNCTION: VC1 0x00472fc0
void KdCompoundObject::SetPositionProvider(const KdPtr<IAbstract>& provider)
{
    AssignPositionable(positionProvider, provider.ptr);
}

// RET 4 (hidden return). Script "GetBoundingRect": the union of the
// children's IBoundingInfo rectangles under g_defaultRenderMask.
// FUNCTION: VC1 0x00473010
RectF KdCompoundObject::GetBoundingRect()
{
    RectF result;
    KdFactory* records = ChildRecords(this);
    unsigned char* it;

    for (it = reinterpret_cast<unsigned char*>(records->begin);
        it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
        it += 0x14) {
        IUnknown* object = RecordObject(it);
        IBoundingInfo* info;

        if (!object) {
            continue;
        }
        info = static_cast<IBoundingInfo*>(object->QueryInterface(&g_iidBoundingInfo));
        if (!info) {
            continue;
        }
        info->AddRef();
        const RectF& rect = info->GetBoundingRect(g_defaultRenderMask);
        if (result.IsEmpty()) {
            result = rect;
        }
        else {
            result.Union(rect);
        }
        info->Release();
    }
    return result;
}

// RET 4. IClassifiable vtable+0x10. Every child loses mask too.
// FUNCTION: VC1 0x004730d0
void KdCompoundObject::RemoveClassificator(unsigned mask)
{
    KdFactory* records = ChildRecords(this);
    unsigned char* it;

    for (it = reinterpret_cast<unsigned char*>(records->begin);
        it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
        it += 0x14) {
        IUnknown* object = RecordObject(it);
        IClassifiable* child = object
            ? static_cast<IClassifiable*>(object->QueryInterface(&g_iidClassifiable))
            : 0;
        if (child) {
            child->AddRef();
        }
        IUnknown* face = child;
        if (IsNotNullObject(face)) {
            child->RemoveClassificator(mask);
        }
        if (child) {
            child->Release();
        }
    }
    value &= ~mask;
}

// RET 4. IClassifiable vtable+0x0c. Every child gains mask too.
// FUNCTION: VC1 0x00473190
void KdCompoundObject::AddClassificator(unsigned mask)
{
    KdFactory* records = ChildRecords(this);
    unsigned char* it;

    for (it = reinterpret_cast<unsigned char*>(records->begin);
        it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
        it += 0x14) {
        IUnknown* object = RecordObject(it);
        IClassifiable* child = object
            ? static_cast<IClassifiable*>(object->QueryInterface(&g_iidClassifiable))
            : 0;
        if (child) {
            child->AddRef();
        }
        IUnknown* face = child;
        if (IsNotNullObject(face)) {
            child->AddClassificator(mask);
        }
        if (child) {
            child->Release();
        }
    }
    value |= mask;
}

// KdComponentGroup vtable+0x0c. Drops the position provider, shuts the
// children down, then the script instance.
// FUNCTION: VC1 0x00473260
int KdCompoundObject::Shutdown()
{
    IScriptInstanceReset* reset;

    AssignPositionable(positionProvider, g_kdNullObject.ptr);
    KdComponentGroup::Shutdown();
    reset = &script;
    reset->Reset();
    return 0;
}

// FUNCTION: VC1 0x004732c0
KdCompoundObject::~KdCompoundObject() throw()
{
    IScriptInstanceReset* reset;

    AssignPositionable(positionProvider, g_kdNullObject.ptr);
    reset = &script;
    reset->Reset();
}

// RET 8. KdComponentGroup::FindObject (0x00472a70).
// FUNCTION: VC1 0x00473520
KdPtr<IUnknown> KdCompoundObject::FindObject(const KdString& name)
{
    return KdComponentGroup::FindObject(name);
}

// STRING: VC1 0x004d4f04
static const char kObjectNotFound[] = "Object with identifier \"%s\" not found";

// RET 8. Logs a missing name.
// FUNCTION: VC1 0x00473540
KdPtr<IUnknown> KdCompoundObject::UnregisterObject(const KdString& name)
{
    KdPtr<IUnknown> found = KdComponentGroup::UnregisterObject(name);
    if (!found.ptr) {
        HostLogError(&g_kdNameCompoundObject, kObjectNotFound, name.c_str());
    }
    return found;
}

// STRING: VC1 0x004d4ed8
static const char kObjectExists[] = "Object with identifier \"%s\" already exists";

// RET 8. Refuses a taken name; otherwise the child takes this object's name
// and classificators before it is inserted.
// FUNCTION: VC1 0x004735a0
void KdCompoundObject::RegisterObject(const KdString& name, const KdPtr<IAbstract>& object)
{
    IIdentifiable* ident;
    IClassifiable* classes;
    IUnknown* face;

    if (IsNotNullObject(FindObject(name).ptr)) {
        HostLogError(&g_kdNameCompoundObject, kObjectExists, name.c_str());
        return;
    }
    ident = object.ptr
        ? static_cast<IIdentifiable*>(object.ptr->QueryInterface(&g_iidIdentifiable))
        : 0;
    if (ident) {
        ident->AddRef();
    }
    face = ident;
    if (IsNotNullObject(face)) {
        ident->SetName(static_cast<IIdentifiable*>(this)->GetName());
    }
    classes = object.ptr
        ? static_cast<IClassifiable*>(object.ptr->QueryInterface(&g_iidClassifiable))
        : 0;
    if (classes) {
        classes->AddRef();
    }
    face = classes;
    if (IsNotNullObject(face)) {
        classes->AddClassificator(static_cast<IClassifiable*>(this)->GetClassificators());
    }
    KdComponentGroup::RegisterObject(name, object);
    if (classes) {
        classes->Release();
    }
    if (ident) {
        ident->Release();
    }
}

// RET 4. IIdentifiable vtable+0x10. Every child is renamed too.
// FUNCTION: VC1 0x00473730
void KdCompoundObject::SetName(const KdString& value)
{
    KdFactory* records;
    unsigned char* it;

    name.Assign(value);
    records = ChildRecords(this);
    for (it = reinterpret_cast<unsigned char*>(records->begin);
        it != reinterpret_cast<unsigned char*>(records->begin) + records->size;
        it += 0x14) {
        IUnknown* object = RecordObject(it);
        IIdentifiable* child = object
            ? static_cast<IIdentifiable*>(object->QueryInterface(&g_iidIdentifiable))
            : 0;
        if (child) {
            child->AddRef();
        }
        IUnknown* face = child;
        if (IsNotNullObject(face)) {
            child->SetName(value);
        }
        if (child) {
            child->Release();
        }
    }
}

// ESI=this.
// FUNCTION: VC1 0x00473800
KdCompoundObject::KdCompoundObject() throw()
{
}

// FUNCTION: VC1 0x004738e0
void** CreateKdCompoundObject(void** out)
{
    KdCompoundObject* memory = static_cast<KdCompoundObject*>(malloc(sizeof(KdCompoundObject)));
    IUnknown* queried;

    if (!memory) {
        *out = 0;
        return out;
    }
    new (memory) KdCompoundObject();
    queried = static_cast<IUnknown*>(
        static_cast<IAbstract*>(memory)->QueryInterface(&g_iidAbstract));
    *out = queried;
    if (queried) {
        queried->AddRef();
    }
    KdRelease(static_cast<IUnknown*>(*out));
    return out;
}

// RET 4. IScriptLoadable vtable+0x0c, owner = the IScriptObject face.
// FUNCTION: VC1 0x004739d0
bool KdCompoundObject::LoadFromScript(SquirrelObject& table)
{
    return script.Load(table, static_cast<IScriptObject*>(this)) != 0;
}

// STRING: VC1 0x004d4eb4
static const char kScriptSetPositionProvider[] = "SetPositionProvider";
// STRING: VC1 0x004d4ec8
static const char kScriptGetBoundingRect[] = "GetBoundingRect";

// RET 8. The script instance's members first, then the method table (table
// 0x004ee678, guard bit 0x004ee688).
// FUNCTION: VC1 0x00473a90
unsigned char KdCompoundObject::GetMember(const char* name, SquirrelObject* out)
{
    IScriptInstance* instance = &script;

    if (instance->GetMember(name, *out)) {
        return 1;
    }
    static ScriptMethodTable methods;
    if (methods.Empty()) {
        methods[kScriptRegisterObject] = CreateScriptMethod<KdCompoundObject>(&KdCompoundObject::RegisterObject);
        methods[kScriptUnregisterObject] = CreateScriptMethod<KdCompoundObject>(&KdCompoundObject::UnregisterObject);
        methods[kScriptGetObject] = CreateScriptMethod<KdCompoundObject>(&KdCompoundObject::FindObject);
        methods[kScriptSetPositionProvider] = CreateScriptMethod<KdCompoundObject>(&KdCompoundObject::SetPositionProvider);
        methods[kScriptGetBoundingRect] = CreateScriptMethod<KdCompoundObject>(&KdCompoundObject::GetBoundingRect);
    }
    return methods.Find(name, out);
}

}
