#ifndef VC1_BINDINGS_SQPLUS_H
#define VC1_BINDINGS_SQPLUS_H

#include "bindings/squirrel_object.h"
#include "host/runtime.h"
#include "kd/factory.h"
#include "kd/iface/iid.h"
#include "kd/iface/iscript_object.h"

#include <string.h>

namespace vc1 {

// SqPlus call layer (sqplus.h / SqPlusCallTemplates.h of the 2008 SqPlus
// snapshot, the variant without SQ_USE_CLASS_INHERITANCE). The engine's
// changes: SQPLUS_CHECK_GET logs instead of throwing, the script instance's
// user pointer is an IUnknown that Dispatch QIs to IScriptObject, and each
// class keeps its methods in a ScriptMethodTable handed out by
// IScriptObject::Script0c instead of registering them on a SQClassDef.

// "sq_get*() failed (type error)"
extern const char g_sqplusGetError[];
// Script method names shared by several Script0c tables.
extern const char kScriptGetWidth[];
extern const char kScriptGetHeight[];

template<typename T>
struct TypeWrapper
{
};

// SQPLUS_CHECK_GET: the failed value is returned uninitialized, as in the
// original.
inline unsigned Get(TypeWrapper<unsigned>, HSQUIRRELVM v, int idx)
{
    SQInteger i;
    if (SQ_FAILED(sq_getinteger(v, idx, &i))) {
        KdLog(g_sqplusErrorFormat, g_sqplusGetError);
    }
    return static_cast<unsigned>(i);
}

int Get(TypeWrapper<int>, HSQUIRRELVM v, int idx);

inline const SQChar* Get(TypeWrapper<const SQChar*>, HSQUIRRELVM v, int idx)
{
    const SQChar* s;
    if (SQ_FAILED(sq_getstring(v, idx, &s))) {
        KdLog(g_sqplusErrorFormat, g_sqplusGetError);
    }
    return s;
}

inline void Push(HSQUIRRELVM v, bool value)
{
    sq_pushbool(v, value);
}

inline void Push(HSQUIRRELVM v, int value)
{
    sq_pushinteger(v, value);
}

// Engine overload: a script array of the list's strings.
void Push(HSQUIRRELVM v, const KdStringList& value);

// SqPlus StackHandler (SquirrelObject.h), the members Dispatch uses.
struct StackHandler
{
    StackHandler(HSQUIRRELVM vm)
    {
        _top = sq_gettop(vm);
        this->v = vm;
    }
    SQUserPointer GetInstanceUp(int idx, SQUserPointer tag)
    {
        SQUserPointer self;
        if (SQ_FAILED(sq_getinstanceup(v, idx, &self, tag))) {
            return 0;
        }
        return self;
    }
    SQUserPointer GetUserData(int idx, SQUserPointer tag = 0)
    {
        SQUserPointer otag;
        SQUserPointer up;
        if (idx > 0 && idx <= _top) {
            if (SQ_SUCCEEDED(sq_getuserdata(v, idx, &up, &otag))) {
                if (tag == otag) {
                    return up;
                }
            }
        }
        return 0;
    }
    int GetParamCount()
    {
        return _top;
    }

    int _top;
    HSQUIRRELVM v;
};

template<typename Callee, typename RT>
int Call(Callee& callee, RT (Callee::*func)(), HSQUIRRELVM v, int index)
{
    (void)index;
    RT ret = (callee.*func)();
    Push(v, ret);
    return 1;
}

template<typename Callee, typename RT, typename P1>
int Call(Callee& callee, RT (Callee::*func)(P1), HSQUIRRELVM v, int index)
{
    RT ret = (callee.*func)(Get(TypeWrapper<P1>(), v, index + 0));
    Push(v, ret);
    return 1;
}

template<typename Callee, typename RT, typename P1, typename P2>
int Call(Callee& callee, RT (Callee::*func)(P1, P2), HSQUIRRELVM v, int index)
{
    RT ret = (callee.*func)(Get(TypeWrapper<P1>(), v, index + 0),
        Get(TypeWrapper<P2>(), v, index + 1));
    Push(v, ret);
    return 1;
}

// SqPlus DirectCallInstanceMemberFunction. The instance user pointer is the
// object's IUnknown; Callee is reached through its IScriptObject base.
template<typename Callee, typename Func>
class DirectCallInstanceMemberFunction
{
public:
    static int Dispatch(HSQUIRRELVM v)
    {
        StackHandler sa(v);
        IUnknown* up = static_cast<IUnknown*>(sa.GetInstanceUp(1, 0));
        Callee* instance = static_cast<Callee*>(
            static_cast<IScriptObject*>(up->QueryInterface(&g_iidScriptObject)));
        int paramCount = sa.GetParamCount();
        Func* func = static_cast<Func*>(sa.GetUserData(paramCount));
        return Call(*instance, *func, v, 2);
    }
};

template<typename Callee, typename Func>
inline void sq_pushdirectinstanceclosure(HSQUIRRELVM v, const Callee& callee, Func func,
    SQUnsignedInteger nupvalues)
{
    (void)callee;
    unsigned char* up = static_cast<unsigned char*>(sq_newuserdata(v, sizeof(func)));
    memcpy(up, &func, sizeof(func));
    sq_newclosure(v, DirectCallInstanceMemberFunction<Callee, Func>::Dispatch, nupvalues + 1);
}

// Inferred name. The closure for one method, as stored in a class's
// ScriptMethodTable.
template<typename Callee, typename Func>
SquirrelObject CreateScriptMethod(Func func)
{
    sq_pushdirectinstanceclosure(g_vm, *static_cast<Callee*>(0), func, 0);
    SquirrelObject so;
    so.AttachToStackObject(-1);
    sq_pop(g_vm, 1);
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

// KdFont script methods (KdFont::Script0c). The original inlines the
// GetHeight Call into its Dispatch.

// TEMPLATE: VC1 0x0041d340
// ??$CreateScriptMethod@VKdTexture@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@0@P8KdTexture@0@AEHXZ@Z

// TEMPLATE: VC1 0x0045c2f0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdTexture@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00419cb0
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AE_NI@Z@vc1@@YA?AVSquirrelObject@0@P8KdFont@0@AE_NI@Z@Z

// TEMPLATE: VC1 0x00419d20
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AEHPBD@Z@vc1@@YA?AVSquirrelObject@0@P8KdFont@0@AEHPBD@Z@Z

// TEMPLATE: VC1 0x00419d90
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AEHXZ@vc1@@YA?AVSquirrelObject@0@P8KdFont@0@AEHXZ@Z

// TEMPLATE: VC1 0x00419e00
// ??$CreateScriptMethod@VKdFont@vc1@@P812@AE?AUKdStringList@2@PBDH@Z@vc1@@YA?AVSquirrelObject@0@P8KdFont@0@AE?AUKdStringList@0@PBDH@Z@Z

// TEMPLATE: VC1 0x00419f80
// ??$Call@VKdFont@vc1@@_NI@vc1@@YAHAAVKdFont@0@P810@AE_NI@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041a020
// ??$Call@VKdFont@vc1@@HPBD@vc1@@YAHAAVKdFont@0@P810@AEHPBD@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x0041a0c0
// ??$Call@VKdFont@vc1@@UKdStringList@2@PBDH@vc1@@YAHAAVKdFont@0@P810@AE?AUKdStringList@0@PBDH@ZPAUSQVM@@H@Z

// TEMPLATE: VC1 0x004671e0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AEHXZ@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004672b0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AE_NI@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00467350
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AE?AUKdStringList@2@PBDH@Z@vc1@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004673f0
// ?Dispatch@?$DirectCallInstanceMemberFunction@VKdFont@vc1@@P812@AEHPBD@Z@vc1@@SAHPAUSQVM@@@Z

}

#endif
