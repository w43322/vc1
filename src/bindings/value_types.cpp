#include "value_types.h"

#include "memory/fixed_pool.h"

#include <new>

namespace vc1 {

SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx)
{
    SQFloat value = 0.0f;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getfloat(vm, idx, &value);
    }
    return value;
}

SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx)
{
    SQInteger value = 0;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getinteger(vm, idx, &value);
    }
    return value;
}

// SqPlus PostConstruct<T> (no class inheritance); the original folds every
// instantiation here.
// FUNCTION: VC1 0x0041e3b0 FOLDED
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook)
{
    sq_setinstanceup(vm, 1, nativeObject);
    sq_setreleasehook(vm, 1, releaseHook);
    return 1;
}

}
