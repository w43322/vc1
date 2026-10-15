#ifndef VC1_KD_SCRIPT_INSTANCE_H
#define VC1_KD_SCRIPT_INSTANCE_H

#include "kd/iface/iunknown.h"
#include "bindings/squirrel_object.h"

namespace vc1 {

// Inferred names. The script half of a scripted engine object: the Squirrel
// instance made from the class (or table) given to LoadFromScript. The
// owner's Script0c / Script10 consult its members before their own method
// table, so script code sees engine methods and script members on one object.
// Neither face answers an IID (QueryInterface 0x004059e0 returns 0) and the
// reference count is a no-op (0x0041adb0): the holder lives inside its owner.
// SIZE 0x04
struct IScriptInstance : IUnknown
{
    virtual unsigned char GetMember(const char* name, SquirrelObject& out) = 0;          // vtable+0x0c
    virtual unsigned char SetMember(const char* name, const SquirrelObject& value) = 0; // vtable+0x10
    virtual const char* GetTypeName() = 0;                                               // vtable+0x14

    SquirrelObject GetValue(const char* name);
};

// SIZE 0x04
struct IScriptInstanceReset : IUnknown
{
    virtual void Reset() = 0; // vtable+0x0c
};

// SIZE 0x18
// VTABLE: VC1 0x004d1470 vc1::IScriptInstance
// VTABLE: VC1 0x004d1488 vc1::IScriptInstanceReset
class KdScriptInstance : public IScriptInstance,
                         public IScriptInstanceReset
{
public:
    SquirrelObject instance; // 0x08
    IUnknown* owner;         // 0x14  set by Load, not by the constructor

    void* QueryInterface(const char** iid);
    unsigned AddRef();
    unsigned Release();
    unsigned char GetMember(const char* name, SquirrelObject& out);
    unsigned char SetMember(const char* name, const SquirrelObject& value);
    const char* GetTypeName();
    void Reset();
    unsigned char Load(const SquirrelObject& script, IUnknown* owner);
};

typedef char Vc1KdScriptInstanceSizeCheck[sizeof(KdScriptInstance) == 0x18 ? 1 : -1];

// SYNTHETIC: VC1 0x00405f40
// ?QueryInterface@KdScriptInstance@vc1@@W3AEPAXPAPBD@Z
// SYNTHETIC: VC1 0x00405f90
// ?AddRef@KdScriptInstance@vc1@@W3AEIXZ

}

#endif
