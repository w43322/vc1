#ifndef VC1_KD_COMPOUND_OBJECT_H
#define VC1_KD_COMPOUND_OBJECT_H

#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icomponent_faces.h"
#include "kd/component_group.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/iscript_object.h"
#include "kd/factory.h"
#include "kd/script_instance.h"
#include "bindings/rectf.h"
#include "bindings/squirrel_object.h"

namespace vc1 {

// Inferred name. Never assigned; released at exit (0x004ca710). The engine's
// "no object" value, compared by IAbstract identity (0x00415e00 / 0x00415e50).
extern KdRef<IUnknown> g_kdNullObject;

bool IsNullObject(IUnknown* const& object);
bool IsNotNullObject(IUnknown* const& object);

// Factory "CompoundObject" and the base of ScriptedObject: a named component
// group whose name and classificators propagate to its children, with an
// optional position provider and a script instance (LoadFromScript) whose
// members come before the "RegisterObject" ... "GetBoundingRect" methods.
// QI 0x00472f00 establishes the face offsets below. IComponentGroup also
// answers IShutdownable at +0x28 through its inheritance contract.
// SIZE 0x70
// VTABLE: VC1 0x004d4f54 vc1::IScriptObject
// VTABLE: VC1 0x004d4f6c vc1::IScriptLoadable
// VTABLE: VC1 0x004d4f7c vc1::IPositionable
// VTABLE: VC1 0x004d4f90 vc1::IIdentifiable
// VTABLE: VC1 0x004d4fa8 vc1::IClassifiable
// VTABLE: VC1 0x004d4fc4 vc1::KdComponentGroup
// VTABLE: VC1 0x004d4fec vc1::IAbstract
class KdCompoundObject : public IScriptObject,
                         public IScriptLoadable,
                         public IPositionable,
                         public IIdentifiable,
                         public IClassifiable,
                         public KdComponentGroup,
                         public IAbstract
{
public:
    KdScriptInstance script;               // 0x54
    KdRef<IPositionable> positionProvider; // 0x6c

    KdCompoundObject() throw();
    ~KdCompoundObject() throw();
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject* out);
    unsigned char SetMember(void* name, void* value);
    const char* GetTypeName();
    bool LoadFromScript(SquirrelObject& table);
    void SetPosition(const Matrix3& value);
    const Matrix3& GetPosition();
    void SetName(const KdString& value);
    void AddClassificator(unsigned mask);
    void RemoveClassificator(unsigned mask);
    int Shutdown();
    void RegisterObject(const KdString& name, const KdPtr<IAbstract>& object);
    KdPtr<IUnknown> UnregisterObject(const KdString& name);
    KdPtr<IUnknown> FindObject(const KdString& name);

    void SetPositionProvider(const KdPtr<IAbstract>& provider);
    RectF GetBoundingRect();
};

typedef char Vc1KdCompoundObjectSizeCheck[sizeof(KdCompoundObject) == 0x70 ? 1 : -1];

void** CreateKdCompoundObject(void** out);

}

// SYNTHETIC: VC1 0x00415ab0
// vc1::KdCompoundObject::`scalar deleting destructor'

#endif
