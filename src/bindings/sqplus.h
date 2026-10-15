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
// snapshot). Value types (Vector3, Color, ...) are plain SQClassDef classes
// on the stock DirectCallInstanceMemberFunction. Kd objects instead keep
// their methods in a ScriptMethodTable handed out by IScriptObject::Script0c;
// those closures use ScriptMethodCall, whose instance user pointer is an
// IUnknown QI'd to IScriptObject (ScriptInstance).

// Script method names shared by several Script0c tables.
extern const char kScriptGetWidth[];
extern const char kScriptGetHeight[];
extern const char kScriptRegisterObject[];
extern const char kScriptUnregisterObject[];
extern const char kScriptGetObject[];

template<typename Callee>
Callee* ScriptInstance(SQUserPointer up)
{
    return static_cast<Callee*>(static_cast<IScriptObject*>(
        static_cast<IUnknown*>(up)->QueryInterface(&g_iidScriptObject)));
}

// Engine overload: a script array of the list's strings. Non-const so it wins
// over SqPlus's generic Push(HSQUIRRELVM, T&).
void Push(HSQUIRRELVM v, KdStringList& value);

// Engine overload for the value types' "_tostring" (inlined into the
// Dispatch, e.g. 0x00424470).
inline void Push(HSQUIRRELVM v, KdString& value)
{
    sq_pushstring(v, value.c_str(), -1);
}
// A const KdString& return (getIdentificator's Call, 0x0042b400).
inline void Push(HSQUIRRELVM v, const KdString& value)
{
    sq_pushstring(v, const_cast<KdString&>(value).c_str(), -1);
}

// Engine overloads found by argument-dependent lookup from SqPlus's Call.
// A const KdString& argument borrows the script string (0x00403990).
KdString Get(SqPlus::TypeWrapper<const KdString&>, HSQUIRRELVM v, int idx);
// A script array of the list's integers (0x00403860).
void Push(HSQUIRRELVM v, KdArray<unsigned>& value);

// Inferred name. Slot idx converted to T, kept in a function-local static
// (out of line for unsigned at 0x00403910, inlined for int into 0x00403520).
template<class T>
const T& GetArrayItem(HSQUIRRELVM v, int idx)
{
    static T value;
    SQInteger i;
    SQPLUS_CHECK_GET(sq_getinteger(v, idx, &i));
    value = static_cast<T>(i);
    return value;
}

// A const KdArray<T>& argument: the script array at idx, element by element
// (0x00403520 for int, 0x00403700 for unsigned). Missing slots stay as
// allocated.
template<class T>
KdArray<T> Get(SqPlus::TypeWrapper<const KdArray<T>&>, HSQUIRRELVM v, int idx)
{
    unsigned i;

    if (idx < 0) {
        idx = sq_gettop(v) + idx + 1;
    }
    KdArray<T> result(sq_getsize(v, idx));
    for (i = 0; i < result.Count(); ++i) {
        sq_pushinteger(v, i);
        if (SQ_SUCCEEDED(sq_get(v, idx))) {
            result[i] = GetArrayItem<T>(v, -1);
            sq_pop(v, 1);
        }
    }
    return result;
}

// Inferred name. SqPlus DirectCallInstanceMemberFunction without the picker
// and without the "Invalid Instance Type" check.
template<typename Callee, typename Func>
struct ScriptMethodCall
{
    static int Dispatch(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        Callee* instance = ScriptInstance<Callee>(sa.GetInstanceUp(1, 0));
        int paramCount = sa.GetParamCount();
        Func* func = static_cast<Func*>(sa.GetUserData(paramCount));
        return SqPlus::Call(*instance, *func, v, 2);
    }
};

// Inferred name. The closure for one method, as stored in a class's
// ScriptMethodTable (SqPlus sq_pushdirectinstanceclosure on ScriptMethodCall).
template<typename Callee, typename Func>
SquirrelObject CreateScriptMethod(Func func)
{
    SQUserPointer up = sq_newuserdata(SquirrelVM::_VM, sizeof(func));
    memcpy(up, &func, sizeof(func));
    sq_newclosure(SquirrelVM::_VM, ScriptMethodCall<Callee, Func>::Dispatch, 1);
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
// ?Dispatch@?$ScriptMethodCall@VKdTexture@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

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
// ?Dispatch@?$ScriptMethodCall@VKdFont@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004672b0
// ?Dispatch@?$ScriptMethodCall@VKdFont@vc1@@P812@AE_NI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00467350
// ?Dispatch@?$ScriptMethodCall@VKdFont@vc1@@P812@AE?AUKdStringList@2@PBDH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004673f0
// ?Dispatch@?$ScriptMethodCall@VKdFont@vc1@@P812@AEHPBD@Z@vc1@@SAHPAUSQVM@@@Z

// RegisterStdBindings (0x004bdc50) globals, in registration order. The first
// two RegisterGlobals inline Register; the rest call it out of line. Calls
// with arguments beyond the int/bool ones are out of line too.

// TEMPLATE: VC1 0x00402690
// ??$RegisterGlobal@P6AXXZ@SqPlus@@YAXPAUSQVM@@P6AXXZPBD@Z

// TEMPLATE: VC1 0x00402db0
// ?Dispatch@?$DirectCallFunction@P6AXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00402770
// ??$RegisterGlobal@P6AXH@Z@SqPlus@@YAXPAUSQVM@@P6AXH@ZPBD@Z

// TEMPLATE: VC1 0x00402e00
// ?Dispatch@?$DirectCallFunction@P6AXH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00402850
// ??$RegisterGlobal@P6AXH_N@Z@SqPlus@@YAXPAUSQVM@@P6AXH_N@ZPBD@Z

// TEMPLATE: VC1 0x00402b70
// ??$Register@P6AXH_N@Z@SqPlus@@YAXPAUSQVM@@P6AXH_N@ZPBD@Z

// TEMPLATE: VC1 0x00402eb0
// ?Dispatch@?$DirectCallFunction@P6AXH_N@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004028f0
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@HHABUVector2@2@_N@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@HHABUVector2@3@_N@ZPBD@Z

// TEMPLATE: VC1 0x00402bd0
// ??$Register@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@HHABUVector2@2@_N@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@HHABUVector2@3@_N@ZPBD@Z

// TEMPLATE: VC1 0x00402f70
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@HHABUVector2@2@_N@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00403120
// ??$Call@ABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@HHABUVector2@2@_N@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@HHABUVector2@3@_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00402990
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@ABU?$KdArray@H@2@H@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@ABU?$KdArray@H@3@H@ZPBD@Z

// TEMPLATE: VC1 0x00402c30
// ??$Register@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@ABU?$KdArray@H@2@H@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@ABU?$KdArray@H@3@H@ZPBD@Z

// TEMPLATE: VC1 0x00402fe0
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@ABU?$KdArray@H@2@H@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00403240
// ??$Call@ABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@2@ABU?$KdArray@H@2@H@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIGraphics@vc1@@@vc1@@ABURect@3@ABU?$KdArray@H@3@H@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00402a30
// ??$RegisterGlobal@P6A?AVKdString@vc1@@ABU?$KdArray@I@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVKdString@vc1@@ABU?$KdArray@I@3@@ZPBD@Z

// TEMPLATE: VC1 0x00402c90
// ??$Register@P6A?AVKdString@vc1@@ABU?$KdArray@I@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVKdString@vc1@@ABU?$KdArray@I@3@@ZPBD@Z

// TEMPLATE: VC1 0x00403050
// ?Dispatch@?$DirectCallFunction@P6A?AVKdString@vc1@@ABU?$KdArray@I@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00403380
// ??$Call@ABU?$KdArray@I@vc1@@@?$ReturnSpecialization@VKdString@vc1@@@SqPlus@@SAHP6A?AVKdString@vc1@@ABU?$KdArray@I@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00402ad0
// ??$RegisterGlobal@P6A?AU?$KdArray@I@vc1@@ABVKdString@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AU?$KdArray@I@vc1@@ABVKdString@3@@ZPBD@Z

// TEMPLATE: VC1 0x00402cf0
// ??$Register@P6A?AU?$KdArray@I@vc1@@ABVKdString@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AU?$KdArray@I@vc1@@ABVKdString@3@@ZPBD@Z

// TEMPLATE: VC1 0x004030c0
// ?Dispatch@?$DirectCallFunction@P6A?AU?$KdArray@I@vc1@@ABVKdString@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00403450
// ??$Call@ABVKdString@vc1@@@?$ReturnSpecialization@U?$KdArray@I@vc1@@@SqPlus@@SAHP6A?AU?$KdArray@I@vc1@@ABVKdString@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00403520
// ??$Get@H@vc1@@YA?AU?$KdArray@H@0@U?$TypeWrapper@ABU?$KdArray@H@vc1@@@SqPlus@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00403700
// ??$Get@I@vc1@@YA?AU?$KdArray@I@0@U?$TypeWrapper@ABU?$KdArray@I@vc1@@@SqPlus@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00403910
// ??$GetArrayItem@I@vc1@@YAABIPAUSQVM@@H@Z

}

// Declared in SquirrelObject.h; the stock SqPlus body lives in sqplus.h but is
// missing from the vendored copy. Inlined in getComponentIDs (0x004b6060):
// sq_pushobject, sq_pushstring, sq_arrayappend, then the inlined sq_pop.
template<typename T>
inline BOOL SquirrelObject::ArrayAppend(T item)
{
    sq_pushobject(SquirrelVM::_VM, GetObjectHandle());
    SqPlus::Push(SquirrelVM::_VM, item);
    BOOL res = sq_arrayappend(SquirrelVM::_VM, -2) == SQ_OK;
    sq_pop(SquirrelVM::_VM, 1);
    return res;
}

#endif
