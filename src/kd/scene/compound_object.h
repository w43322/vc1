#ifndef VC1_KD_COMPOUND_OBJECT_H
#define VC1_KD_COMPOUND_OBJECT_H

#include "kd/iface/iabstract.h"
#include "kd/iface/iclassifiable.h"
#include "kd/iface/icloneable.h"
#include "kd/component_group.h"
#include "kd/iface/iidentifiable.h"
#include "kd/iface/ipositionable.h"
#include "kd/iface/iscript_object.h"
#include "kd/factory.h"
#include "bindings/squirrel_object.h"

namespace vc1 {

// Inferred member type at KdCompoundObject+0x54. Its secondary ICloneable
// vptr is initialized at +0x58, exactly as the original constructor shows.
struct CompoundTailFace : IUnknown
{
    virtual int Tail0c(void* arg) = 0;
    virtual int Tail10(void* arg) = 0;
    virtual int Tail14() = 0;
};

// SIZE 0x08
// VTABLE: VC1 0x004d1470 vc1::CompoundTailFace
// VTABLE: VC1 0x004d1488 vc1::ICloneable
class CompoundTailValue : public CompoundTailFace,
                          public ICloneable
{
public:
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    int Tail0c(void* arg);
    int Tail10(void* arg);
    int Tail14();
    void** Clone(void** out);
};

typedef char Vc1CompoundTailValueSizeCheck[sizeof(CompoundTailValue) == 0x08 ? 1 : -1];

// Polymorphic script value constructed at KdCompoundObject+0x5c. This is kept
// separate from the still-partial generic SquirrelObject wrapper.
// Name is inferred from its HSQOBJECT payload.
// SIZE 0x0c
// VTABLE: VC1 0x004cfb98
class CompoundScriptValue
{
public:
    HSQOBJECT obj;

    virtual ~CompoundScriptValue() {}
    virtual void ScriptValue04() {}
    virtual void ScriptValue08() {}
    virtual void ScriptValue0c() {}
    virtual void ScriptValue10() {}
};

typedef char Vc1CompoundScriptValueSizeCheck[sizeof(CompoundScriptValue) == 0x0c ? 1 : -1];

// QI 0x00472f00 establishes the six face offsets below. IComponentGroup also
// answers IShutdownable at +0x28 through its inheritance contract.
// SIZE 0x70
// VTABLE: VC1 0x004d4f54 vc1::IScriptObject
// VTABLE: VC1 0x004d4f6c vc1::ICloneable
// VTABLE: VC1 0x004d4f7c vc1::IPositionable
// VTABLE: VC1 0x004d4f90 vc1::IIdentifiable
// VTABLE: VC1 0x004d4fa8 vc1::IClassifiable
// VTABLE: VC1 0x004d4fc4 vc1::KdComponentGroup
// VTABLE: VC1 0x004d4fec vc1::IAbstract
class KdCompoundObject : public IScriptObject,
                         public ICloneable,
                         public IPositionable,
                         public IIdentifiable,
                         public IClassifiable,
                         public KdComponentGroup,
                         public IAbstract
{
public:
    CompoundTailValue tailValue; // 0x54
    CompoundScriptValue scriptValue; // 0x5c
    unsigned tail68;             // 0x68
    void* tail6c;                // 0x6c

    KdCompoundObject() throw();
    ~KdCompoundObject() {}
    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char Script0c(void* a, void* b);
    unsigned char Script10(void* a, void* b);
    const char* Script14();
    void** Clone(void** out);
    void* Position0c(void* out);
    void Position10(const void* value);
    const char* GetName();
    void SetName(const char* value);
    int Classify0c(void* arg);
    int Classify10(void* arg);
    int Classify14(void* arg);
    int Classify18();
    int Shutdown();
    int Unmapped10(void* key, void** out);
    void** Unmapped14(void** out, void* key);
    void** Unmapped18(void** out, void* key);
};

typedef char Vc1KdCompoundObjectSizeCheck[sizeof(KdCompoundObject) == 0x70 ? 1 : -1];

void** CreateKdCompoundObject(void** out);

}

#endif
