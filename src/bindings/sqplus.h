#ifndef VC1_BINDINGS_SQPLUS_H
#define VC1_BINDINGS_SQPLUS_H

#include "bindings/squirrel_object.h"
#include "kdcore/platform.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iscript_object.h"
#include "kd/name_map.h"

#include <string.h>

namespace vc1 {

// Engine side of the SqPlus call layer (third_party/sqplus, the 20080713
// snapshot). Value types (Vector3, Color, ...) are plain SQClassDef classes
// on the stock DirectCallInstanceMemberFunction. Kd objects instead keep
// their methods in a ScriptMethodTable handed out by IScriptObject::GetMember;
// those closures use ScriptMethodCall, whose instance user pointer is an
// IUnknown QI'd to IScriptObject (ScriptInstance).

// Script method names shared by several GetMember tables.
extern const char kScriptGetWidth[];
extern const char kScriptGetHeight[];
extern const char kScriptRegisterObject[];
extern const char kScriptUnregisterObject[];
extern const char kScriptSetRenderLayer[];
extern const char kScriptGetRenderLayer[];
extern const char kScriptGetObject[];
extern const char kScriptGetFont[];
extern const char kScriptGetCamera[];
extern const char kScriptClear[];

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
// The same for bytes (TransportMover's route indexes, 0x0040c8d0).
void Push(HSQUIRRELVM v, KdArray<unsigned char>& value);

struct Vector2;
struct Rect;
// Script arrays of Vector2 / Rect instances (0x00406430, 0x0040c750).
KdObjArray<Vector2> Get(SqPlus::TypeWrapper<const KdObjArray<Vector2>&>, HSQUIRRELVM v, int idx);
KdObjArray<Rect> Get(SqPlus::TypeWrapper<const KdObjArray<Rect>&>, HSQUIRRELVM v, int idx);

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

// A script table as a name-sorted map of its values (0x00409220).
KdNameMap<SquirrelObject> Get(SqPlus::TypeWrapper<const KdNameMap<SquirrelObject>&>,
    HSQUIRRELVM v, int idx);
// A script array of int arrays (0x004093e0).
KdObjArray<KdArray<int> > Get(SqPlus::TypeWrapper<const KdObjArray<KdArray<int> >&>,
    HSQUIRRELVM v, int idx);

// Inferred name. object converted to T: pushed, read with the matching Get,
// then the inlined sq_poptop.
template<class T>
T GetScriptValue(const SquirrelObject& object)
{
    sq_pushobject(SquirrelVM::_VM, object.GetObjectHandle());
    T result = Get(SqPlus::TypeWrapper<const T&>(), SquirrelVM::_VM, -1);
    sq_poptop(SquirrelVM::_VM);
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
    HSQUIRRELVM v = SquirrelVM::_VM;
    SQUserPointer up = sq_newuserdata(v, sizeof(func));
    *static_cast<Func*>(up) = func;
    sq_newclosure(v, ScriptMethodCall<Callee, Func>::Dispatch, 1);
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
// GetMember builds one as a function-local static on first use.
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

    // GetMember body: copies the closure for name into *out. No original
    // address: inlined into every GetMember, which calls LowerBound.
    __forceinline unsigned char Find(const char* name, SquirrelObject* out)
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

// KdTexture / KdFont script methods (GetMember). The original inlines the
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

// InitHostServices (0x004b61b0) natives: one RegisterGlobal per function
// type (shared by natives of the same type), the out-of-line Register of
// the single-use ones with name and function folded in, their
// DirectCallFunction Dispatch and the ReturnSpecialization Call. Paired
// with the recompiled chain in registration order.

// TEMPLATE: VC1 0x00429a00
// ??$RegisterGlobal@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@Z@SqPlus@@YAXPAUSQVM@@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@Z1@Z

// TEMPLATE: VC1 0x00429aa0
// ??$RegisterGlobal@P6A?AVSquirrelObject@@XZ@SqPlus@@YAXPAUSQVM@@P6A?AVSquirrelObject@@XZPBD@Z

// TEMPLATE: VC1 0x00429b40
// ??$RegisterGlobal@P6AXPAUSQVM@@@Z@SqPlus@@YAXPAUSQVM@@P6AX0@ZPBD@Z

// TEMPLATE: VC1 0x00429c20
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x00429d00
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBD@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBD@Z2@Z

// TEMPLATE: VC1 0x00429de0
// ??$RegisterGlobal@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x00429ec0
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@_N@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@_N@ZPBD@Z

// TEMPLATE: VC1 0x00429fa0
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042a080
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@1@ZPBD@Z

// TEMPLATE: VC1 0x0042a160
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@ZPBD@Z

// TEMPLATE: VC1 0x0042a200
// ??$RegisterGlobal@P6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042a2a0
// ??$RegisterGlobal@P6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042a340
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042a420
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@H@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@H@ZPBD@Z

// TEMPLATE: VC1 0x0042a500
// ??$RegisterGlobal@P6AHABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6AHABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042a5e0
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPBD@Z

// TEMPLATE: VC1 0x0042a680
// ??$RegisterGlobal@P6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042a720
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042a7c0
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042a860
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@M@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@3@M@ZPBD@Z

// TEMPLATE: VC1 0x0042a900
// ??$RegisterGlobal@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@1@ZPBD@Z

// TEMPLATE: VC1 0x0042a9e0
// ??$RegisterGlobal@P6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042aa80
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@Z2@Z

// TEMPLATE: VC1 0x0042ab20
// ??$RegisterGlobal@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPBD@Z

// TEMPLATE: VC1 0x0042abc0
// ??$RegisterGlobal@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@0@Z@SqPlus@@YAXPAUSQVM@@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@1@ZPBD@Z

// TEMPLATE: VC1 0x0042ac60
// ??$RegisterGlobal@P6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042ad00
// ??$RegisterGlobal@P6AIABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6AIABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042ada0
// ??$RegisterGlobal@P6AHXZ@SqPlus@@YAXPAUSQVM@@P6AHXZPBD@Z

// TEMPLATE: VC1 0x0042ae80
// ??$RegisterGlobal@P6AHHVSquirrelObject@@@Z@SqPlus@@YAXPAUSQVM@@P6AHHVSquirrelObject@@@ZPBD@Z

// TEMPLATE: VC1 0x0042af20
// ??$RegisterGlobal@P6A_NPBD@Z@SqPlus@@YAXPAUSQVM@@P6A_NPBD@Z1@Z

// TEMPLATE: VC1 0x0042afc0
// ??$RegisterGlobal@P6AHH@Z@SqPlus@@YAXPAUSQVM@@P6AHH@ZPBD@Z

// TEMPLATE: VC1 0x0042b060
// ??$RegisterGlobal@P6AMMM@Z@SqPlus@@YAXPAUSQVM@@P6AMMM@ZPBD@Z

// TEMPLATE: VC1 0x0042b140
// ??$RegisterGlobal@P6AMM@Z@SqPlus@@YAXPAUSQVM@@P6AMM@ZPBD@Z

// TEMPLATE: VC1 0x0042b1e0
// ??$RegisterGlobal@P6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b280
// ??$Register@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@Z@SqPlus@@YAXPAUSQVM@@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@Z1@Z

// TEMPLATE: VC1 0x0042b2e0
// ??$Register@P6A?AVSquirrelObject@@XZ@SqPlus@@YAXPAUSQVM@@P6A?AVSquirrelObject@@XZPBD@Z

// TEMPLATE: VC1 0x0042b340
// ??$Register@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@ZPBD@Z

// TEMPLATE: VC1 0x0042b3a0
// ??$Register@P6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042b400
// ??$Register@P6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b460
// ??$Register@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPBD@Z

// TEMPLATE: VC1 0x0042b4c0
// ??$Register@P6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b520
// ??$Register@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b580
// ??$Register@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b5e0
// ??$Register@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@M@Z@SqPlus@@YAXPAUSQVM@@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@3@M@ZPBD@Z

// TEMPLATE: VC1 0x0042b640
// ??$Register@P6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b6a0
// ??$Register@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@Z2@Z

// TEMPLATE: VC1 0x0042b700
// ??$Register@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPBD@Z

// TEMPLATE: VC1 0x0042b760
// ??$Register@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@0@Z@SqPlus@@YAXPAUSQVM@@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@1@ZPBD@Z

// TEMPLATE: VC1 0x0042b7c0
// ??$Register@P6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@YAXPAUSQVM@@P6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042b820
// ??$Register@P6AIABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@YAXPAUSQVM@@P6AIABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPBD@Z

// TEMPLATE: VC1 0x0042b880
// ??$Register@P6AHHVSquirrelObject@@@Z@SqPlus@@YAXPAUSQVM@@P6AHHVSquirrelObject@@@ZPBD@Z

// TEMPLATE: VC1 0x0042b8e0
// ??$Register@P6A_NPBD@Z@SqPlus@@YAXPAUSQVM@@P6A_NPBD@Z1@Z

// TEMPLATE: VC1 0x0042b940
// ??$Register@P6AHH@Z@SqPlus@@YAXPAUSQVM@@P6AHH@ZPBD@Z

// TEMPLATE: VC1 0x0042b9a0
// ??$Register@P6AMM@Z@SqPlus@@YAXPAUSQVM@@P6AMM@ZPBD@Z

// TEMPLATE: VC1 0x0042ba00
// ??$Register@P6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@2@@Z@SqPlus@@YAXPAUSQVM@@P6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@3@@ZPBD@Z

// TEMPLATE: VC1 0x0042ba60
// ?Dispatch@?$DirectCallFunction@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bad0
// ?Dispatch@?$DirectCallFunction@P6A?AVSquirrelObject@@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bb40
// ?Dispatch@?$DirectCallFunction@P6AXPAUSQVM@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bbb0
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bc20
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBD@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bc90
// ?Dispatch@?$DirectCallFunction@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bd00
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@_N@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bd70
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bde0
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042be50
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bec0
// ?Dispatch@?$DirectCallFunction@P6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bf30
// ?Dispatch@?$DirectCallFunction@P6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042bf90
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c000
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@H@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c070
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c0e0
// ?Dispatch@?$DirectCallFunction@P6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c140
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c1b0
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c210
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c270
// ?Dispatch@?$DirectCallFunction@P6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c2e0
// ?Dispatch@?$DirectCallFunction@P6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c350
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c3c0
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c430
// ?Dispatch@?$DirectCallFunction@P6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c4a0
// ?Dispatch@?$DirectCallFunction@P6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c510
// ?Dispatch@?$DirectCallFunction@P6AHABV?$KdPtr@UIAbstract@vc1@@@vc1@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c580
// ?Dispatch@?$DirectCallFunction@P6AHXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c600
// ?Dispatch@?$DirectCallFunction@P6AHHVSquirrelObject@@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c670
// ?Dispatch@?$DirectCallFunction@P6A_NPBD@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c6e0
// ?Dispatch@?$DirectCallFunction@P6AHH@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c750
// ?Dispatch@?$DirectCallFunction@P6AMMM@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c7c0
// ?Dispatch@?$DirectCallFunction@P6AMM@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c830
// ?Dispatch@?$DirectCallFunction@P6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0042c960
// ??$Call@PBD@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHP6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@PBD@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042ca10
// ??$Call@PAUSQVM@@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXPAUSQVM@@@Z0H@Z

// TEMPLATE: VC1 0x0042ca70
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042cad0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBD@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBD@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042cbc0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHP6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042cc70
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@_N@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042ccf0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042cd90
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV12@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042ce40
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@VSquirrelObject@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042cf70
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@VSquirrelObject@@@SqPlus@@SAHP6A?AVSquirrelObject@@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d0a0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@ABVKdString@vc1@@@SqPlus@@SAHP6AABVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d120
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d1d0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@H@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@H@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d240
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d320
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@ABUMatrix3@vc1@@@SqPlus@@SAHP6AABUMatrix3@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d3c0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@2@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABUMatrix3@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d430
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV?$KdPtr@UIPositionable@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d510
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@2@M@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABVKdString@3@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d630
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV12@@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NABV?$KdPtr@UIAbstract@vc1@@@vc1@@0@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d720
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@VKdString@vc1@@@SqPlus@@SAHP6A?AVKdString@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d800
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@PBDH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d8f0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UIAbstract@vc1@@@vc1@@HH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042d9b0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@ABV12@@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHP6A?AV?$KdPtr@UIUnknown@vc1@@@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@0@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042dab0
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@URectF@vc1@@@SqPlus@@SAHP6A?AURectF@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042db70
// ??$Call@ABV?$KdPtr@UIAbstract@vc1@@@vc1@@@?$ReturnSpecialization@H@SqPlus@@SAHP6AHABV?$KdPtr@UIAbstract@vc1@@@vc1@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042dc00
// ??$Call@HVSquirrelObject@@@?$ReturnSpecialization@H@SqPlus@@SAHP6AHHVSquirrelObject@@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042dcf0
// ??$Call@PBD@?$ReturnSpecialization@_N@SqPlus@@SAHP6A_NPBD@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042dd90
// ??$Call@H@?$ReturnSpecialization@H@SqPlus@@SAHP6AHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042de20
// ??$Call@MM@?$ReturnSpecialization@M@SqPlus@@SAHP6AMMM@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042deb0
// ??$Call@M@?$ReturnSpecialization@M@SqPlus@@SAHP6AMM@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0042df00
// ??$Call@ABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@2@@?$ReturnSpecialization@X@SqPlus@@SAHP6AXABV?$KdPtr@UITexture@vc1@@@vc1@@ABUMatrix4@3@@ZPAUSQVM@@H@Z


// Callees of the TEMPLATE instantiations named elsewhere (Dispatch -> Call,
// CreateScriptMethod -> Dispatch), paired with the recompiled callees.

// TEMPLATE: VC1 0x004048c0
// ??$Call@VKdBuildingBuilder@vc1@@ABV?$KdPtr@UIGridMap@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdBuildingBuilder@vc1@@P823@AEXABV?$KdPtr@UIGridMap@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00408080
// ??$Call@VKdTerrainBuilder@vc1@@ABU?$KdArray@F@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTerrainBuilder@vc1@@P823@AEXABU?$KdArray@F@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0040a790
// ??$Call@VKdRoadRenderer@vc1@@F@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdRoadRenderer@vc1@@P823@AEXF@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004169c0
// ??$Call@VKdCamera@vc1@@ABUVector2@2@@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAVKdCamera@vc1@@P823@AE?AUVector2@3@ABU43@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00416a40 FOLDED
// ??$Call@VKdCamera@vc1@@@?$ReturnSpecialization@ABUMatrix3@vc1@@@SqPlus@@SAHAAVKdCamera@vc1@@P823@AEABUMatrix3@3@XZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00416a40 FOLDED
// ??$Call@VKdLayer@vc1@@@?$ReturnSpecialization@AAUMatrix3@vc1@@@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEAAUMatrix3@3@XZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x00416a40 FOLDED
// ??$Call@VKdGraphics@vc1@@@?$ReturnSpecialization@AAUMatrix3@vc1@@@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEAAUMatrix3@3@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041b390
// ??$Call@VKdEffect@vc1@@@?$ReturnSpecialization@V?$KdPtr@UIEffectEmitter@vc1@@@vc1@@@SqPlus@@SAHAAVKdEffect@vc1@@P823@AE?AV?$KdPtr@UIEffectEmitter@vc1@@@3@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041c180
// ??$Call@VKdSound@vc1@@KHH@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdSound@vc1@@P823@AE_NKHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041c260
// ??$Call@VKdMusic@vc1@@HH@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdMusic@vc1@@P823@AE_NHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041c6a0
// ??$Call@VKdMusic@vc1@@_N@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdMusic@vc1@@P823@AEH_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041c710
// ??$Call@VKdMusic@vc1@@M@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdMusic@vc1@@P823@AE_NM@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041c7a0
// ??$Call@VKdMusic@vc1@@_N@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdMusic@vc1@@P823@AE_N_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041d040 FOLDED
// ??$Call@VKdLayer@vc1@@ABV?$KdPtr@UIFont@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEXABV?$KdPtr@UIFont@vc1@@@3@@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0041d040 FOLDED
// ??$Call@VKdGraphics@vc1@@ABV?$KdPtr@UIFont@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEXABV?$KdPtr@UIFont@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041d0e0 FOLDED
// ??$Call@VKdLayer@vc1@@@?$ReturnSpecialization@AAUVector2@vc1@@@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEAAUVector2@3@XZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0041d0e0 FOLDED
// ??$Call@VKdGraphics@vc1@@@?$ReturnSpecialization@AAUVector2@vc1@@@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEAAUVector2@3@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041d130 FOLDED
// ??$Call@VKdLayer@vc1@@PBDMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdLayer@vc1@@P823@AEXPBDMM@ZPAUSQVM@@H@Z
// TEMPLATE: VC1 0x0041d130 FOLDED
// ??$Call@VKdGraphics@vc1@@PBDMM@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGraphics@vc1@@P823@AEXPBDMM@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004262c0
// ??$Call@EEEE@?$ReturnSpecialization@I@SqPlus@@SAHP6AIEEEE@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426420
// ??$Call@ABUVector3@vc1@@ABU12@@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHP6A?AUVector3@vc1@@ABU23@0@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004264b0
// ??$Call@ABUVector3@vc1@@ABU12@M@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHP6A?AUVector3@vc1@@ABU23@0M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426560
// ??$Call@ABUVector2@vc1@@ABU12@M@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHP6A?AUVector2@vc1@@ABU23@0M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426610
// ??$Call@ABUVector2@vc1@@@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHP6A?AUVector2@vc1@@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426680
// ??$Call@ABUVector4@vc1@@ABU12@@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHP6A?AUVector3@vc1@@ABUVector4@3@0@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426710
// ??$Call@ABUVector4@vc1@@ABU12@M@?$ReturnSpecialization@UVector4@vc1@@@SqPlus@@SAHP6A?AUVector4@vc1@@ABU23@0M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004267d0
// ??$Call@ABUMatrix3@vc1@@@?$ReturnSpecialization@UMatrix3@vc1@@@SqPlus@@SAHP6A?AUMatrix3@vc1@@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426840
// ??$Call@ABUMatrix4@vc1@@@?$ReturnSpecialization@UMatrix4@vc1@@@SqPlus@@SAHP6A?AUMatrix4@vc1@@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004268c0
// ??$Call@ABUQuaternion@vc1@@@?$ReturnSpecialization@UQuaternion@vc1@@@SqPlus@@SAHP6A?AUQuaternion@vc1@@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0045ae00
// ?Dispatch@?$ScriptMethodCall@VKdEffectEmitter@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045aec0
// ?Dispatch@?$ScriptMethodCall@VKdEffectEmitter@vc1@@P812@AEX_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045af90
// ?Dispatch@?$ScriptMethodCall@VKdEffectEmitter@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045b0a0
// ?Dispatch@?$ScriptMethodCall@VKdEffectEmitter@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0045b8f0
// ?Dispatch@?$ScriptMethodCall@VKdEffect@vc1@@P812@AE?AV?$KdPtr@UIEffectEmitter@vc1@@@2@XZ@vc1@@SAHPAUSQVM@@@Z


// Callees of the TEMPLATE instantiations named elsewhere (Dispatch -> Call,
// CreateScriptMethod -> Dispatch), paired with the recompiled callees.

// TEMPLATE: VC1 0x00403b60
// ??$CreateScriptMethod@VKdGameLevel@vc1@@P812@AEXHABV?$KdPtr@UIFunctionalGroup@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGameLevel@0@AEXHABV?$KdPtr@UIFunctionalGroup@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00403bd0
// ??$CreateScriptMethod@VKdGameLevel@vc1@@P812@AE?AUKdRefList@2@II@Z@vc1@@YA?AVSquirrelObject@@P8KdGameLevel@0@AE?AUKdRefList@0@II@Z@Z

// TEMPLATE: VC1 0x00403c40
// ??$CreateScriptMethod@VKdGameLevel@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@II@Z@vc1@@YA?AVSquirrelObject@@P8KdGameLevel@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABUVector2@0@II@Z@Z

// TEMPLATE: VC1 0x00403e50
// ??$Call@VKdGameLevel@vc1@@HABV?$KdPtr@UIFunctionalGroup@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdGameLevel@vc1@@P823@AEXHABV?$KdPtr@UIFunctionalGroup@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00403f10
// ??$Call@VKdGameLevel@vc1@@II@?$ReturnSpecialization@UKdRefList@vc1@@@SqPlus@@SAHAAVKdGameLevel@vc1@@P823@AE?AUKdRefList@3@II@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00403fd0
// ??$Call@VKdGameLevel@vc1@@ABUVector2@2@II@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHAAVKdGameLevel@vc1@@P823@AE?AV?$KdPtr@UIUnknown@vc1@@@3@ABUVector2@3@II@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00404960
// ??$Get@F@vc1@@YA?AU?$KdArray@F@0@U?$TypeWrapper@ABU?$KdArray@F@vc1@@@SqPlus@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00404ad0
// ??$GetArrayItem@F@vc1@@YAABFPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00404c80
// ??$CreateScriptMethod@VKdGridRouter@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRouter@0@AEXABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00404cf0
// ??$CreateScriptMethod@VKdGridRouter@vc1@@P812@AEHABUVector2@2@0_N@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRouter@0@AEHABUVector2@0@0_N@Z@Z

// TEMPLATE: VC1 0x00404d60
// ??$CreateScriptMethod@VKdGridRouter@vc1@@P812@AEAAUVector2@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdGridRouter@0@AEAAUVector2@0@H@Z@Z

// TEMPLATE: VC1 0x00404e20
// ??$Call@VKdGridRouter@vc1@@ABUVector2@2@ABU32@_N@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdGridRouter@vc1@@P823@AEHABUVector2@3@1_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004054a0
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEXHH@Z@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEXHH@Z@Z

// TEMPLATE: VC1 0x00405510
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEXABV?$KdPtr@UIPositionable@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00405580
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UIRenderable@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEXABV?$KdPtr@UIRenderable@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x004055f0
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@ABUVector2@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEXABV?$KdPtr@UICamera@vc1@@@0@ABUVector2@0@@Z@Z

// TEMPLATE: VC1 0x00405660
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEX_N@Z@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEX_N@Z@Z

// TEMPLATE: VC1 0x004056d0
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AE?AV?$KdPtr@UITexture@vc1@@@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AE?AV?$KdPtr@UITexture@vc1@@@0@XZ@Z

// TEMPLATE: VC1 0x00405740
// ??$CreateScriptMethod@VKdObjectWatcher@vc1@@P812@AEABUVector2@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdObjectWatcher@0@AEABUVector2@0@XZ@Z

// TEMPLATE: VC1 0x004057b0
// ??$Call@VKdObjectWatcher@vc1@@ABV?$KdPtr@UIRenderable@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdObjectWatcher@vc1@@P823@AEXABV?$KdPtr@UIRenderable@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00405850
// ??$Call@VKdObjectWatcher@vc1@@ABV?$KdPtr@UICamera@vc1@@@2@ABUVector2@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdObjectWatcher@vc1@@P823@AEXABV?$KdPtr@UICamera@vc1@@@3@ABUVector2@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00405a40
// ??$CreateScriptMethod@VKdPassMap@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdPassMap@0@AEHXZ@Z

// TEMPLATE: VC1 0x00405ab0
// ??$CreateScriptMethod@VKdPassMap@vc1@@P812@AEHHH@Z@vc1@@YA?AVSquirrelObject@@P8KdPassMap@0@AEHHH@Z@Z

// TEMPLATE: VC1 0x00405b20
// ??$CreateScriptMethod@VKdPassMap@vc1@@P812@AEXHHH@Z@vc1@@YA?AVSquirrelObject@@P8KdPassMap@0@AEXHHH@Z@Z

// TEMPLATE: VC1 0x00405b90
// ??$CreateScriptMethod@VKdPassMap@vc1@@P812@AEXABV?$KdPtr@UIGridRenderer@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdPassMap@0@AEXABV?$KdPtr@UIGridRenderer@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00405d00
// ??$Call@VKdPassMap@vc1@@HH@?$ReturnSpecialization@H@SqPlus@@SAHAAVKdPassMap@vc1@@P823@AEHHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00405d90
// ??$Call@VKdPassMap@vc1@@HHH@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdPassMap@vc1@@P823@AEXHHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00405e00
// ??$Call@VKdPassMap@vc1@@ABV?$KdPtr@UIGridRenderer@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdPassMap@vc1@@P823@AEXABV?$KdPtr@UIGridRenderer@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00407340
// ??$GetInstance@UMatrix3@vc1@@$00@SqPlus@@YAPAUMatrix3@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x0040a110
// ??$Call@VKdSystemBase@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdSystemBase@vc1@@P823@AEXABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0040dee0
// ?resize@?$sqvector@USQObjectPtr@@@@QAEXIABUSQObjectPtr@@@Z

// TEMPLATE: VC1 0x0040dfa0
// ?push_back@?$sqvector@USQObjectPtr@@@@QAEAAUSQObjectPtr@@ABU2@@Z

// TEMPLATE: VC1 0x0040e000
// ?insert@?$sqvector@USQObjectPtr@@@@QAEXIABUSQObjectPtr@@@Z

// TEMPLATE: VC1 0x0040e120
// ?copy@?$sqvector@H@@QAEXABV1@@Z

// TEMPLATE: VC1 0x0040e1b0
// ?resize@?$sqvector@H@@QAEXIABH@Z

// TEMPLATE: VC1 0x0040ec20
// ?resize@?$sqvector@UCallInfo@SQVM@@@@QAEXIABUCallInfo@SQVM@@@Z

// TEMPLATE: VC1 0x0040f8c0
// ?push_back@?$sqvector@H@@QAEAAHABH@Z

// TEMPLATE: VC1 0x0040fb60
// ?copy@?$sqvector@USQClassMember@@@@QAEXABV1@@Z

// TEMPLATE: VC1 0x0040fcd0
// ?resize@?$sqvector@USQClassMember@@@@QAEXIABUSQClassMember@@@Z

// TEMPLATE: VC1 0x004152d0
// ?push_back@?$sqvector@USQInstruction@@@@QAEAAUSQInstruction@@ABU2@@Z

// TEMPLATE: VC1 0x004155f0
// ?push_back@?$sqvector@USQLocalVarInfo@@@@QAEAAUSQLocalVarInfo@@ABU2@@Z

// TEMPLATE: VC1 0x00415790
// ?push_back@?$sqvector@USQLineInfo@@@@QAEAAUSQLineInfo@@ABU2@@Z

// TEMPLATE: VC1 0x00415c40
// ??$CreateScriptMethod@VKdCompoundObject@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdCompoundObject@0@AEXABVKdString@0@ABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00415cb0
// ??$CreateScriptMethod@VKdCompoundObject@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdCompoundObject@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABVKdString@0@@Z@Z

// TEMPLATE: VC1 0x00415d20
// ??$CreateScriptMethod@VKdCompoundObject@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdCompoundObject@0@AEXABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00415d90
// ??$CreateScriptMethod@VKdCompoundObject@vc1@@P812@AE?AURectF@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdCompoundObject@0@AE?AURectF@0@XZ@Z

// TEMPLATE: VC1 0x00415ea0
// ??$Call@VKdCompoundObject@vc1@@@?$ReturnSpecialization@URectF@vc1@@@SqPlus@@SAHAAVKdCompoundObject@vc1@@P823@AE?AURectF@3@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00417470
// ??$CreateScriptMethod@VKdScene@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdScene@0@AEXABVKdString@0@ABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x004174e0
// ??$CreateScriptMethod@VKdScene@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdScene@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABVKdString@0@@Z@Z

// TEMPLATE: VC1 0x00417550
// ??$CreateScriptMethod@VKdScene@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdScene@0@AEXABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x004175c0
// ??$CreateScriptMethod@VKdScene@vc1@@P812@AEXABVKdString@2@_N@Z@vc1@@YA?AVSquirrelObject@@P8KdScene@0@AEXABVKdString@0@_N@Z@Z

// TEMPLATE: VC1 0x00417630
// ??$Call@VKdSystemBase@vc1@@ABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdSystemBase@vc1@@P823@AEXABVKdString@3@ABV?$KdPtr@UIAbstract@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004176e0
// ??$Call@VKdScene@vc1@@ABVKdString@2@_N@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdScene@vc1@@P823@AEXABVKdString@3@_N@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00417980
// ??$CreateScriptMethod@VKdResourceManager@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdResourceManager@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABVKdString@0@@Z@Z

// TEMPLATE: VC1 0x004179f0
// ??$CreateScriptMethod@VKdResourceManager@vc1@@P812@AEXABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdResourceManager@0@AEXABVKdString@0@@Z@Z

// TEMPLATE: VC1 0x00417a60
// ??$CreateScriptMethod@VKdResourceManager@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdResourceManager@0@AEXXZ@Z

// TEMPLATE: VC1 0x00417ad0
// ??$Call@VKdResourceManager@vc1@@ABVKdString@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdResourceManager@vc1@@P823@AEXABVKdString@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00417d70
// ??$CreateScriptMethod@VKdBlob@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdBlob@0@AEXH@Z@Z

// TEMPLATE: VC1 0x004185b0
// ??$CreateScriptMethod@VKdFile@vc1@@P812@AE_NPBD0@Z@vc1@@YA?AVSquirrelObject@@P8KdFile@0@AE_NPBD0@Z@Z

// TEMPLATE: VC1 0x0041af60
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEXABVKdString@0@ABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x0041afd0
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AE?AV?$KdPtr@UIUnknown@vc1@@@0@ABVKdString@0@@Z@Z

// TEMPLATE: VC1 0x0041b040
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AE?AV?$KdPtr@UIAbstract@vc1@@@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AE?AV?$KdPtr@UIAbstract@vc1@@@0@XZ@Z

// TEMPLATE: VC1 0x0041b0b0
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEXABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x0041b120
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEXXZ@Z

// TEMPLATE: VC1 0x0041b190
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEMXZ@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEMXZ@Z

// TEMPLATE: VC1 0x0041b200
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEXM@Z@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEXM@Z@Z

// TEMPLATE: VC1 0x0041b270
// ??$CreateScriptMethod@VKdSystemBase@vc1@@P812@AEIXZ@vc1@@YA?AVSquirrelObject@@P8KdSystemBase@0@AEIXZ@Z

// TEMPLATE: VC1 0x0041b2e0
// ??$Call@VKdSystemBase@vc1@@ABVKdString@2@@?$ReturnSpecialization@V?$KdPtr@UIUnknown@vc1@@@vc1@@@SqPlus@@SAHAAVKdSystemBase@vc1@@P823@AE?AV?$KdPtr@UIUnknown@vc1@@@3@ABVKdString@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041b520
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AE?AURect@2@HI@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AE?AURect@0@HI@Z@Z

// TEMPLATE: VC1 0x0041b590
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AEXABV?$KdPtr@UIGraphics@vc1@@@2@HMMI@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AEXABV?$KdPtr@UIGraphics@vc1@@@0@HMMI@Z@Z

// TEMPLATE: VC1 0x0041b600
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AE_NHHHI@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AE_NHHHI@Z@Z

// TEMPLATE: VC1 0x0041b670
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AEHH@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AEHH@Z@Z

// TEMPLATE: VC1 0x0041b6e0
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AEAAV?$KdPtr@UITexture@vc1@@@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AEAAV?$KdPtr@UITexture@vc1@@@0@H@Z@Z

// TEMPLATE: VC1 0x0041b750
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AEXHABV?$KdPtr@UITexture@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AEXHABV?$KdPtr@UITexture@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x0041b7c0
// ??$CreateScriptMethod@VKdTileManager@vc1@@P812@AE?AU?$KdArray@I@2@HI@Z@vc1@@YA?AVSquirrelObject@@P8KdTileManager@0@AE?AU?$KdArray@I@0@HI@Z@Z

// TEMPLATE: VC1 0x0041b930
// ??$Call@VKdTileManager@vc1@@HI@?$ReturnSpecialization@URect@vc1@@@SqPlus@@SAHAAVKdTileManager@vc1@@P823@AE?AURect@3@HI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041b9e0
// ??$Call@VKdTileManager@vc1@@ABV?$KdPtr@UIGraphics@vc1@@@2@HMMI@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTileManager@vc1@@P823@AEXABV?$KdPtr@UIGraphics@vc1@@@3@HMMI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041bb20
// ??$Call@VKdTileManager@vc1@@HHHI@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTileManager@vc1@@P823@AE_NHHHI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041bbe0
// ??$Call@VKdTileManager@vc1@@HABV?$KdPtr@UITexture@vc1@@@2@@?$ReturnSpecialization@X@SqPlus@@SAHAAVKdTileManager@vc1@@P823@AEXHABV?$KdPtr@UITexture@vc1@@@3@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041bca0
// ??$Call@VKdTileManager@vc1@@HI@?$ReturnSpecialization@U?$KdArray@I@vc1@@@SqPlus@@SAHAAVKdTileManager@vc1@@P823@AE?AU?$KdArray@I@3@HI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041d920
// ??$GetInstance@URectF@vc1@@$00@SqPlus@@YAPAURectF@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426a80
// ??$GetInstance@UMatrix4@vc1@@$00@SqPlus@@YAPAUMatrix4@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426bc0
// ??$GetInstance@UVector4@vc1@@$00@SqPlus@@YAPAUVector4@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426c40
// ??$GetInstance@UVector3@vc1@@$00@SqPlus@@YAPAUVector3@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00427040
// ??$Set@VSquirrelObject@@@SquirrelObject@@QAEAAV0@AAV0@@Z

// TEMPLATE: VC1 0x0042c890
// ?Call@?$ReturnSpecialization@VSquirrelObject@@@SqPlus@@SAHP6A?AVSquirrelObject@@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00461050
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AE?AU?$KdArray@I@2@HI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004610f0
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AEXHABV?$KdPtr@UITexture@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00461190
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AEHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00461230
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AE_NHHHI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004612d0
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AEXABV?$KdPtr@UIGraphics@vc1@@@2@HMMI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00461370
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AEAAV?$KdPtr@UITexture@vc1@@@2@H@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00461620
// ?Dispatch@?$ScriptMethodCall@VKdTileManager@vc1@@P812@AE?AURect@2@HI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462a10
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEIXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462ae0
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEXM@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462bd0
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEMXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462c90
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462d40
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00462de0
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00463330
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AE?AV?$KdPtr@UIAbstract@vc1@@@2@XZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004633d0
// ?Dispatch@?$ScriptMethodCall@VKdSystemBase@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046a970
// ?Dispatch@?$ScriptMethodCall@VKdFile@vc1@@P812@AE_NPBD0@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046ceb0
// ?Dispatch@?$ScriptMethodCall@VKdBlob@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046d850
// ?Dispatch@?$ScriptMethodCall@VKdResourceManager@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046d900
// ?Dispatch@?$ScriptMethodCall@VKdResourceManager@vc1@@P812@AEXABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046da60
// ?Dispatch@?$ScriptMethodCall@VKdResourceManager@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046eb60
// ?Dispatch@?$ScriptMethodCall@VKdScene@vc1@@P812@AEXABVKdString@2@_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046ec00
// ?Dispatch@?$ScriptMethodCall@VKdScene@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046eca0
// ?Dispatch@?$ScriptMethodCall@VKdScene@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0046f2d0
// ?Dispatch@?$ScriptMethodCall@VKdScene@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004733e0
// ?Dispatch@?$ScriptMethodCall@VKdCompoundObject@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00473480
// ?Dispatch@?$ScriptMethodCall@VKdCompoundObject@vc1@@P812@AEXABVKdString@2@ABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00473930
// ?Dispatch@?$ScriptMethodCall@VKdCompoundObject@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABVKdString@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004739f0
// ?Dispatch@?$ScriptMethodCall@VKdCompoundObject@vc1@@P812@AE?AURectF@2@XZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004acf50
// ?Dispatch@?$ScriptMethodCall@VKdPassMap@vc1@@P812@AEXABV?$KdPtr@UIGridRenderer@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004acff0
// ?Dispatch@?$ScriptMethodCall@VKdPassMap@vc1@@P812@AEXHHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ad090
// ?Dispatch@?$ScriptMethodCall@VKdPassMap@vc1@@P812@AEHHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ad130
// ?Dispatch@?$ScriptMethodCall@VKdPassMap@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004add60
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEX_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ade20
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UICamera@vc1@@@2@ABUVector2@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004adec0
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UIRenderable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004adf60
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEXABV?$KdPtr@UIPositionable@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ae000
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEXHH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ae100
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AE?AV?$KdPtr@UITexture@vc1@@@2@XZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004ae2a0
// ?Dispatch@?$ScriptMethodCall@VKdObjectWatcher@vc1@@P812@AEABUVector2@2@XZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004af950
// ?Dispatch@?$ScriptMethodCall@VKdGridRouter@vc1@@P812@AEHABUVector2@2@0_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004af9f0
// ?Dispatch@?$ScriptMethodCall@VKdGridRouter@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004afa90
// ?Dispatch@?$ScriptMethodCall@VKdGridRouter@vc1@@P812@AEAAUVector2@2@H@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004b1290
// ?Dispatch@?$ScriptMethodCall@VKdGameLevel@vc1@@P812@AEXHABV?$KdPtr@UIFunctionalGroup@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004b1330
// ?Dispatch@?$ScriptMethodCall@VKdGameLevel@vc1@@P812@AE?AV?$KdPtr@UIUnknown@vc1@@@2@ABUVector2@2@II@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004b13d0
// ?Dispatch@?$ScriptMethodCall@VKdGameLevel@vc1@@P812@AE?AUKdRefList@2@II@Z@vc1@@SAHPAUSQVM@@@Z

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

// Callees found by pairing the direct calls of mapped functions with the
// recompiled ones (same call count and order); asmcmp confirms each body.

// TEMPLATE: VC1 0x00401c60
// ??$CreateCopyInstance@UVector2@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUVector2@vc1@@@Z

// TEMPLATE: VC1 0x00401d20
// ??$CreateCopyInstance@URect@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABURect@vc1@@@Z

// TEMPLATE: VC1 0x00404eb0
// ??$Call@VKdGridRouter@vc1@@H@?$ReturnSpecialization@AAUVector2@vc1@@@SqPlus@@SAHAAVKdGridRouter@vc1@@P823@AEAAUVector2@3@H@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00409c40
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AE_NABV?$KdPtr@UIAbstract@vc1@@@2@H@Z@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AE_NABV?$KdPtr@UIAbstract@vc1@@@0@H@Z@Z

// TEMPLATE: VC1 0x00409cb0
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AEHXZ@Z

// TEMPLATE: VC1 0x00409d90
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AEEABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AEEABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00409e00
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AEXABV?$KdPtr@UIAbstract@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x00409e70
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AEXXZ@Z

// TEMPLATE: VC1 0x0040a5d0
// ??$CreateScriptMethod@VVcTileManager@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8VcTileManager@0@AEXH@Z@Z

// TEMPLATE: VC1 0x0040a640
// ??$CreateScriptMethod@VVcTileManager@vc1@@P812@AEXI@Z@vc1@@YA?AVSquirrelObject@@P8VcTileManager@0@AEXI@Z@Z

// TEMPLATE: VC1 0x0040a6b0
// ??$CreateScriptMethod@VVcTileManager@vc1@@P812@AE?AUVector2@2@HH@Z@vc1@@YA?AVSquirrelObject@@P8VcTileManager@0@AE?AUVector2@0@HH@Z@Z

// TEMPLATE: VC1 0x0040a720
// ??$CreateScriptMethod@VVcTileManager@vc1@@P812@AEXXZ@vc1@@YA?AVSquirrelObject@@P8VcTileManager@0@AEXXZ@Z

// TEMPLATE: VC1 0x004160f0
// ??$CreateCopyInstance@URectF@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABURectF@vc1@@@Z

// TEMPLATE: VC1 0x00416b30
// ??$CreateCopyInstance@UMatrix3@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUMatrix3@vc1@@@Z

// TEMPLATE: VC1 0x00418620
// ??$Call@VKdFile@vc1@@PBDPBD@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdFile@vc1@@P823@AE_NPBD1@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00418e40
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEHXZ@Z

// TEMPLATE: VC1 0x00418eb0
// ??$CreateScriptMethod@VKdLayer@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdLayer@0@AEXH@Z@Z

// TEMPLATE: VC1 0x0041a6d0
// ??$CreateScriptMethod@VKdDisplayBase@vc1@@P812@AE?AV?$KdPtr@UIRenderTarget@vc1@@@2@II@Z@vc1@@YA?AVSquirrelObject@@P8KdDisplayBase@0@AE?AV?$KdPtr@UIRenderTarget@vc1@@@0@II@Z@Z

// TEMPLATE: VC1 0x0041a740
// ??$CreateScriptMethod@VKdDisplayBase@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdDisplayBase@0@AEHXZ@Z

// TEMPLATE: VC1 0x0041a7b0
// ??$CreateScriptMethod@VKdDisplayBase@vc1@@P812@AE_NXZ@vc1@@YA?AVSquirrelObject@@P8KdDisplayBase@0@AE_NXZ@Z

// TEMPLATE: VC1 0x0041c940
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEAAURect@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEAAURect@0@XZ@Z

// TEMPLATE: VC1 0x0041c9b0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXABURect@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXABURect@0@@Z@Z

// TEMPLATE: VC1 0x0041ca20
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEIXZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEIXZ@Z

// TEMPLATE: VC1 0x0041ca90
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXI@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXI@Z@Z

// TEMPLATE: VC1 0x0041cb00
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEHXZ@Z

// TEMPLATE: VC1 0x0041cb70
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXH@Z@Z

// TEMPLATE: VC1 0x0041cbe0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEPAPAUIFont@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEPAPAUIFont@0@XZ@Z

// TEMPLATE: VC1 0x0041cc50
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXABV?$KdPtr@UIFont@vc1@@@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXABV?$KdPtr@UIFont@vc1@@@0@@Z@Z

// TEMPLATE: VC1 0x0041ccc0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEPAPAUIUnknown@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEPAPAUIUnknown@0@XZ@Z

// TEMPLATE: VC1 0x0041cd30
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEAAUMatrix3@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEAAUMatrix3@0@XZ@Z

// TEMPLATE: VC1 0x0041cda0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXABUMatrix3@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXABUMatrix3@0@@Z@Z

// TEMPLATE: VC1 0x0041ce10
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEAAUVector2@2@XZ@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEAAUVector2@0@XZ@Z

// TEMPLATE: VC1 0x0041ce80
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXABUVector2@2@@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXABUVector2@0@@Z@Z

// TEMPLATE: VC1 0x0041cef0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXMMMM@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXMMMM@Z@Z

// TEMPLATE: VC1 0x0041cf60
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXABV?$KdPtr@UITexture@vc1@@@2@MMMMMM@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXABV?$KdPtr@UITexture@vc1@@@0@MMMMMM@Z@Z

// TEMPLATE: VC1 0x0041cfd0
// ??$CreateScriptMethod@VKdGraphics@vc1@@P812@AEXPBDMM@Z@vc1@@YA?AVSquirrelObject@@P8KdGraphics@0@AEXPBDMM@Z@Z

// TEMPLATE: VC1 0x00424c90
// ??$Call@UColor@vc1@@I@SqPlus@@YAHAAUColor@vc1@@P812@AEIXZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424ce0
// ??$Call@UColor@vc1@@E@SqPlus@@YAHAAUColor@vc1@@P812@AEEXZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424d30
// ??$Call@UColor@vc1@@HABU12@@SqPlus@@YAHAAUColor@vc1@@P812@BEHABU12@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424d80
// ??$Call@UMatrix3@vc1@@AAUVector2@2@@SqPlus@@YAHAAUMatrix3@vc1@@P812@AEAAUVector2@2@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424dd0
// ??$Call@UMatrix4@vc1@@AAUVector3@2@@SqPlus@@YAHAAUMatrix4@vc1@@P812@AEAAUVector3@2@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424e20
// ??$Call@UVector2@vc1@@_N@SqPlus@@YAHAAUVector2@vc1@@P812@BE_NXZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00424e70
// ??$Call@UIntegralF@vc1@@XM@SqPlus@@YAHAAUIntegralF@vc1@@P812@AEXM@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004254c0
// ??$Call@UVector3@vc1@@ABU12@@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHAAUVector3@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425540
// ??$Call@UVector3@vc1@@M@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHAAUVector3@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004255e0
// ??$Call@UVector3@vc1@@@?$ReturnSpecialization@UVector3@vc1@@@SqPlus@@SAHAAUVector3@vc1@@P823@BE?AU23@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425650
// ??$Call@UVector3@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUVector3@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004256b0
// ??$Call@UVector2@vc1@@M@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAUVector2@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425750
// ??$Call@UVector2@vc1@@@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAUVector2@vc1@@P823@BE?AU23@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004257b0
// ??$Call@UVector2@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUVector2@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425810
// ??$Call@UVector2@vc1@@ABU12@@?$ReturnSpecialization@_N@SqPlus@@SAHAAUVector2@vc1@@P823@BE_NABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425870
// ??$Call@UVector4@vc1@@ABU12@@?$ReturnSpecialization@UVector4@vc1@@@SqPlus@@SAHAAUVector4@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004258f0
// ??$Call@UVector4@vc1@@M@?$ReturnSpecialization@UVector4@vc1@@@SqPlus@@SAHAAUVector4@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425990
// ??$Call@UVector4@vc1@@@?$ReturnSpecialization@UVector4@vc1@@@SqPlus@@SAHAAUVector4@vc1@@P823@BE?AU23@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425a00
// ??$Call@UVector4@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUVector4@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425a60
// ??$Call@UMatrix3@vc1@@ABU12@@?$ReturnSpecialization@UMatrix3@vc1@@@SqPlus@@SAHAAUMatrix3@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425ae0
// ??$Call@UMatrix3@vc1@@M@?$ReturnSpecialization@UMatrix3@vc1@@@SqPlus@@SAHAAUMatrix3@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425b80
// ??$Call@UMatrix3@vc1@@@?$ReturnSpecialization@UMatrix3@vc1@@@SqPlus@@SAHAAUMatrix3@vc1@@P823@BE?AU23@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425bf0
// ??$Call@UMatrix3@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUMatrix3@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425c50
// ??$Call@UVector2@vc1@@ABU12@@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAUVector2@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425cc0
// ??$Call@UMatrix4@vc1@@ABU12@@?$ReturnSpecialization@UMatrix4@vc1@@@SqPlus@@SAHAAUMatrix4@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425d40
// ??$Call@UMatrix4@vc1@@M@?$ReturnSpecialization@UMatrix4@vc1@@@SqPlus@@SAHAAUMatrix4@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425de0
// ??$Call@UMatrix4@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUMatrix4@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425e40
// ??$Call@UQuaternion@vc1@@ABU12@@?$ReturnSpecialization@UQuaternion@vc1@@@SqPlus@@SAHAAUQuaternion@vc1@@P823@BE?AU23@ABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425ec0
// ??$Call@UQuaternion@vc1@@M@?$ReturnSpecialization@UQuaternion@vc1@@@SqPlus@@SAHAAUQuaternion@vc1@@P823@BE?AU23@M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425f60
// ??$Call@UQuaternion@vc1@@@?$ReturnSpecialization@UQuaternion@vc1@@@SqPlus@@SAHAAUQuaternion@vc1@@P823@BE?AU23@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00425fd0
// ??$Call@UQuaternion@vc1@@ABU12@@?$ReturnSpecialization@H@SqPlus@@SAHAAUQuaternion@vc1@@P823@BEHABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426030
// ??$Call@UQuaternion@vc1@@@?$ReturnSpecialization@UMatrix4@vc1@@@SqPlus@@SAHAAUQuaternion@vc1@@P823@BE?AUMatrix4@3@XZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004260a0
// ??$Call@UQuaternion@vc1@@ABU12@ABU12@M@?$ReturnSpecialization@X@SqPlus@@SAHAAUQuaternion@vc1@@P823@AEXABU23@1M@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426100
// ??$Call@URect@vc1@@ABU12@@?$ReturnSpecialization@_N@SqPlus@@SAHAAURect@vc1@@P823@AE_NABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426160
// ??$Call@URect@vc1@@HH@?$ReturnSpecialization@_N@SqPlus@@SAHAAURect@vc1@@P823@BE_NHH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004261f0
// ??$Call@URectF@vc1@@ABU12@@?$ReturnSpecialization@_N@SqPlus@@SAHAAURectF@vc1@@P823@AE_NABU23@@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426940
// ??$CreateCopyInstance@UQuaternion@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUQuaternion@vc1@@@Z

// TEMPLATE: VC1 0x00426a00
// ??$GetInstance@UQuaternion@vc1@@$00@SqPlus@@YAPAUQuaternion@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426b00
// ??$CreateCopyInstance@UVector4@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUVector4@vc1@@@Z

// TEMPLATE: VC1 0x00426e60
// ??$CreateCopyInstance@UVector3@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUVector3@vc1@@@Z

// TEMPLATE: VC1 0x004a1b50
// ?Dispatch@?$ScriptMethodCall@VVcTileManager@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a1c00
// ?Dispatch@?$ScriptMethodCall@VVcTileManager@vc1@@P812@AEXI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a1ca0
// ?Dispatch@?$ScriptMethodCall@VVcTileManager@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a1db0
// ?Dispatch@?$ScriptMethodCall@VVcTileManager@vc1@@P812@AE?AUVector2@2@HH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a3370
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AE_NABV?$KdPtr@UIAbstract@vc1@@@2@H@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a32a0
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a30f0
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AEEABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a3050
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AEXABV?$KdPtr@UIAbstract@vc1@@@2@@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a2fa0
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AEXXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00466a60
// ?Dispatch@?$ScriptMethodCall@VKdDisplayBase@vc1@@P812@AE?AV?$KdPtr@UIRenderTarget@vc1@@@2@II@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00466990
// ?Dispatch@?$ScriptMethodCall@VKdDisplayBase@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004668f0
// ?Dispatch@?$ScriptMethodCall@VKdDisplayBase@vc1@@P812@AE_NXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004a3190
// ?Dispatch@?$ScriptMethodCall@VKdTileLayer@vc1@@P812@AEXH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00409d20
// ??$CreateScriptMethod@VKdTileLayer@vc1@@P812@AEXH@Z@vc1@@YA?AVSquirrelObject@@P8KdTileLayer@0@AEXH@Z@Z

// TEMPLATE: VC1 0x0041a820
// ??$CreateScriptMethod@VKdDisplayBase@vc1@@P812@AEX_N@Z@vc1@@YA?AVSquirrelObject@@P8KdDisplayBase@0@AEX_N@Z@Z

// TEMPLATE: VC1 0x00466830
// ?Dispatch@?$ScriptMethodCall@VKdDisplayBase@vc1@@P812@AEX_N@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x0040a050
// ??$Call@VKdTileLayer@vc1@@ABV?$KdPtr@UIAbstract@vc1@@@2@H@?$ReturnSpecialization@_N@SqPlus@@SAHAAVKdTileLayer@vc1@@P823@AE_NABV?$KdPtr@UIAbstract@vc1@@@3@H@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0040a810
// ??$Call@VVcTileManager@vc1@@HH@?$ReturnSpecialization@UVector2@vc1@@@SqPlus@@SAHAAVVcTileManager@vc1@@P823@AE?AUVector2@3@HH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x00426d50
// ??$CreateCopyInstance@UMatrix4@vc1@@@SqPlus@@YAHPAUSQVM@@PBDABUMatrix4@vc1@@@Z

// The profiler stack copy in ScriptProfilerReset (app/script.cpp).
// TEMPLATE: VC1 0x0040e2c0
// ??0?$KdArray@PAUProfilerBlock@vc1@@@vc1@@QAE@ABU01@@Z

// TEMPLATE: VC1 0x0041a890
// ??$Call@VKdDisplayBase@vc1@@II@?$ReturnSpecialization@V?$KdPtr@UIRenderTarget@vc1@@@vc1@@@SqPlus@@SAHAAVKdDisplayBase@vc1@@P823@AE?AV?$KdPtr@UIRenderTarget@vc1@@@3@II@ZPAUSQVM@@H@Z

#endif
