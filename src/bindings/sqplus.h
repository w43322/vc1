#ifndef VC1_BINDINGS_SQPLUS_H
#define VC1_BINDINGS_SQPLUS_H

#include "bindings/squirrel_object.h"
#include "host/runtime.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iscript_object.h"

#include <string.h>

namespace vc1 {

// Engine side of the SqPlus call layer (third_party/sqplus, the 20080713
// snapshot). The script instance's user pointer is an IUnknown that Dispatch
// QIs to IScriptObject (ScriptInstance), and each class keeps its methods in a
// ScriptMethodTable handed out by IScriptObject::Script0c instead of
// registering them on a SQClassDef.

// Script method names shared by several Script0c tables.
extern const char kScriptGetWidth[];
extern const char kScriptGetHeight[];

template<typename Callee>
Callee* ScriptInstance(SQUserPointer up)
{
    return static_cast<Callee*>(static_cast<IScriptObject*>(
        static_cast<IUnknown*>(up)->QueryInterface(&g_iidScriptObject)));
}

// Engine overload: a script array of the list's strings. Non-const so it wins
// over SqPlus's generic Push(HSQUIRRELVM, T&).
void Push(HSQUIRRELVM v, KdStringList& value);

// Inferred name. The closure for one method, as stored in a class's
// ScriptMethodTable.
template<typename Callee, typename Func>
SquirrelObject CreateScriptMethod(Func func)
{
    SqPlus::sq_pushdirectinstanceclosure(SquirrelVM::_VM, *static_cast<Callee*>(0), func, 0);
    SquirrelObject so;
    so.AttachToStackObject(-1);
    sq_pop(SquirrelVM::_VM, 1);
    return so;
}

// Inferred name. One name -> closure record.
// SIZE 0x10
struct ScriptMethod
{
    const char* name;      // 0x00
    SquirrelObject method; // 0x04

    ScriptMethod() : name(0) {}
    explicit ScriptMethod(const char* key) : name(key) {}
};

typedef char Vc1ScriptMethodSizeCheck[sizeof(ScriptMethod) == 0x10 ? 1 : -1];

void DestroyScriptMethods(ScriptMethod* first, ScriptMethod* last);
void ConstructScriptMethods(ScriptMethod* first, ScriptMethod* last);

// Inferred name. Name-sorted ScriptMethod array in a KdFactory buffer; each
// Script0c builds one as a function-local static on first use.
// SIZE 0x10
struct ScriptMethodTable
{
    KdFactory items; // 0x00

    ~ScriptMethodTable() throw()
    {
        if (items.capacity != 0) {
            DestroyScriptMethods(Begin(), Begin() + items.capacity / sizeof(ScriptMethod));
        }
    }

    ScriptMethod* Begin() const
    {
        return reinterpret_cast<ScriptMethod*>(items.begin);
    }
    ScriptMethod* End() const
    {
        return reinterpret_cast<ScriptMethod*>(
            reinterpret_cast<unsigned char*>(items.begin) + items.size);
    }
    bool Empty() const
    {
        return items.size == 0;
    }

    ScriptMethod* LowerBound(const char* const& name);
    ScriptMethod* InsertSlot(ScriptMethod* pos);
    SquirrelObject& operator[](const char* const& name);

    // Script0c body: copies the closure for name into *out.
    unsigned char Find(const char* name, SquirrelObject* out)
    {
        ScriptMethod* found = LowerBound(name);
        ScriptMethod* end = End();
        if (found < end && strcmp(name, found->name) < 0) {
            found = end;
        }
        if (found == end) {
            return 0;
        }
        *out = found->method;
        return 1;
    }
};

typedef char Vc1ScriptMethodTableSizeCheck[sizeof(ScriptMethodTable) == 0x10 ? 1 : -1];

// KdTexture / KdFont script methods (Script0c). The original inlines the
// GetHeight Call into its Dispatch; the other Calls are SqPlus
// ReturnSpecialization<RT>::Call with the free Call wrapper inlined.

// TEMPLATE: VC1 0x0041d340
// ??$CreateScriptMethod@VKdTexture@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdTexture@0@AEHXZ@Z

// TEMPLATE: VC1 0x0045c2f0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdTexture@vc1@@P812@AEHXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00419cb0
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AE_NI@Z@vc1@@YA?AVSquirrelObject@@P8KdFont@0@AE_NI@Z@Z

// TEMPLATE: VC1 0x00419d20
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AEHPBD@Z@vc1@@YA?AVSquirrelObject@@P8KdFont@0@AEHPBD@Z@Z

// TEMPLATE: VC1 0x00419d90
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdFont@0@AEHXZ@Z

// TEMPLATE: VC1 0x00419e00
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AE?AUKdStringList@2@PBDH@Z@vc1@@YA?AVSquirrelObject@@P8KdFont@0@AE?AUKdStringList@0@PBDH@Z@Z

// TEMPLATE: VC1 0x00419f80
// ??$Call@VKdFont@vc1@@I@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdFont@vc1@@P823@AE_NI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041a020
// ??$Call@VKdFont@vc1@@PBD@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdFont@vc1@@P823@AEHPBD@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041a0c0
// ??$Call@VKdFont@vc1@@PBDH@?$ReturnSpecialization@UKdStringList@vc1@@@SqPlus@@SAHAAVKdFont@vc1@@P823@AE?AUKdStringList@3@PBDH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004671e0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AEHXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004672b0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AE_NI@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00467350
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AE?AUKdStringList@2@PBDH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004673f0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AEHPBD@Z@SqPlus@@SAHPAUSQVM@@@Z

}

#endif
