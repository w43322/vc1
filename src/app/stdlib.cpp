#include "host/runtime.h"

#include <squirrel.h>

namespace vc1 {

static const unsigned kSqRefCounted = 0x08000000;

// STRING: VC1 0x004d2df0
static const SQChar kMaskN[] = ".n";
// STRING: VC1 0x004d2db4
static const SQChar kMaskNN[] = ".nn";
// STRING: VC1 0x004d2e68
static const SQChar kMaskS[] = ".s";
// STRING: VC1 0x004d2e34
static const SQChar kMaskSS[] = ".ss";
// STRING: VC1 0x004d2f0c
static const SQChar kMaskX[] = "x";
// STRING: VC1 0x004d32e0
static const SQChar kMaskXS[] = "xs";
// STRING: VC1 0x004d32ec
static const SQChar kMaskXSN[] = "xsn";
// STRING: VC1 0x004d322c
static const SQChar kMaskXN[] = "xn";
// STRING: VC1 0x004d3208
static const SQChar kMaskXNN[] = "xnn";
// STRING: VC1 0x004d3f88
static const SQChar kMaskXX[] = "xx";
// STRING: VC1 0x004d2ef4
static const SQChar kMaskSB[] = ".sb";
// STRING: VC1 0x004d2ed4
static const SQChar kMaskSC[] = ".sc";
// STRING: VC1 0x004d4000
static const SQChar kStdStream[] = "std_stream";
// STRING: VC1 0x004d26a4
static const SQChar kIndexMissing[] = "the index doesn't exist";

static HSQOBJECT* StackObj(HSQUIRRELVM vm, SQInteger idx)
{
    const SQInteger top = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    char* const stack = *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x18);
    return reinterpret_cast<HSQOBJECT*>(stack + (top + idx) * 8);
}

static void CreateSlotMinus3(HSQUIRRELVM vm)
{
    const SQInteger top = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    const SQInteger base = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);
    HSQOBJECT* self;
    HSQOBJECT* key;
    HSQOBJECT* val;

    if (top - base < 3) {
        // STRING: VC1 0x004d24e8
        UnmappedRaiseFmt("not enough params in the stack");
        return;
    }
    self = StackObj(vm, -3);
    if (self->_type != OT_TABLE && self->_type != OT_CLASS) {
        return;
    }
    key = StackObj(vm, -2);
    if (key->_type == OT_NULL) {
        // STRING: VC1 0x004d2640
        sq_throwerror(vm, "null is not a valid key");
        return;
    }
    val = StackObj(vm, -1);
    UnmappedNewSlot(vm, self, key, val, SQFalse);
    sq_pop(vm, 2);
}

static void RegisterNativeFuncs(HSQUIRRELVM vm, SQRegFunction* funcs, int setName)
{
    SQRegFunction* r;

    for (r = funcs; r->name != 0; ++r) {
        sq_pushstring(vm, r->name, -1);
        sq_newclosure(vm, r->f, 0);
        sq_setparamscheck(vm, r->nparamscheck, r->typemask);
        if (setName) {
            SetNativeClosureName(vm, r->name);
        }
        CreateSlotMinus3(vm);
    }
}

static void PushRawInteger(HSQUIRRELVM vm, SQInteger value)
{
    HSQOBJECT* const slot = StackObj(vm, 0);
    const unsigned oldType = slot->_type;
    int* const oldValue = reinterpret_cast<int*>(slot->_unVal.pUserPointer);
    SQInteger* const pTop = reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);

    slot->_unVal.nInteger = value;
    slot->_type = OT_INTEGER;
    *pTop = *pTop + 1;
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
}

static void PushRawFloatBits(HSQUIRRELVM vm, unsigned bits)
{
    HSQOBJECT* const slot = StackObj(vm, 0);
    const unsigned oldType = slot->_type;
    int* const oldValue = reinterpret_cast<int*>(slot->_unVal.pUserPointer);
    SQInteger* const pTop = reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);

    slot->_unVal.nInteger = static_cast<SQInteger>(bits);
    slot->_type = OT_FLOAT;
    *pTop = *pTop + 1;
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
}

// regexp class methods. Inferred names from the original string table.
// GLOBAL: VC1 0x004e91a0
SQRegFunction g_rexobjFuncs[] = {
    // STRING: VC1 0x004d2f10
    { "constructor", reinterpret_cast<SQFUNCTION>(0x0048e600), 2, kMaskS },
    // STRING: VC1 0x004d32f0
    { "search", reinterpret_cast<SQFUNCTION>(0x0048e290), -2, kMaskXSN },
    // STRING: VC1 0x004d32e4
    { "match", reinterpret_cast<SQFUNCTION>(0x0048d780), 2, kMaskXS },
    // STRING: VC1 0x004d32d8
    { "capture", reinterpret_cast<SQFUNCTION>(0x0048e0f0), -2, kMaskXSN },
    // STRING: VC1 0x004d32cc
    { "subexpcount", reinterpret_cast<SQFUNCTION>(0x0048d4c0), 1, kMaskX },
    // STRING: VC1 0x004d2f04
    { "_typeof", reinterpret_cast<SQFUNCTION>(0x0048d4a0), 1, kMaskX },
    { 0, 0, 0, 0 }
};

// stringlib functions after the regexp class is slotted.
// GLOBAL: VC1 0x004e9210
SQRegFunction g_stringlibFuncs[] = {
    // STRING: VC1 0x004d32c4
    { "format", reinterpret_cast<SQFUNCTION>(0x0048e3c0), -2, kMaskS },
    // STRING: VC1 0x004d32bc
    { "strip", reinterpret_cast<SQFUNCTION>(0x0048dc50), 2, kMaskS },
    // STRING: VC1 0x004d32b4
    { "lstrip", reinterpret_cast<SQFUNCTION>(0x0048dbb0), 2, kMaskS },
    // STRING: VC1 0x004d32ac
    { "rstrip", reinterpret_cast<SQFUNCTION>(0x0048dab0), 2, kMaskS },
    // STRING: VC1 0x004d32a4
    { "split", reinterpret_cast<SQFUNCTION>(0x0048d8b0), 3, kMaskSS },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e93d0
SQRegFunction g_systemlibFuncs[] = {
    // STRING: VC1 0x004d2e6c
    { "getenv", reinterpret_cast<SQFUNCTION>(0x004bb2e0), 2, kMaskS },
    // STRING: VC1 0x004d2e60
    { "system", reinterpret_cast<SQFUNCTION>(0x004bb210), 2, kMaskS },
    // STRING: VC1 0x004d2e58
    { "clock", reinterpret_cast<SQFUNCTION>(0x004bafb0), 1, 0 },
    // STRING: VC1 0x004d2e50
    { "time", reinterpret_cast<SQFUNCTION>(0x004baf50), 1, 0 },
    // STRING: VC1 0x004d2e48
    { "date", reinterpret_cast<SQFUNCTION>(0x004bb370), -1, kMaskNN },
    // STRING: VC1 0x004d2e40
    { "remove", reinterpret_cast<SQFUNCTION>(0x004bb160), 2, kMaskS },
    // STRING: VC1 0x004d2e38
    { "rename", reinterpret_cast<SQFUNCTION>(0x004bb070), 3, kMaskSS },
    { 0, 0, 0, 0 }
};

// Host mathlib: stock 2.2.3 plus round and fmod.
// GLOBAL: VC1 0x004e9450
SQRegFunction g_mathlibFuncs[] = {
    // STRING: VC1 0x004d2df4
    { "sqrt", reinterpret_cast<SQFUNCTION>(0x004bbec0), 2, kMaskN },
    // STRING: VC1 0x004d2dec
    { "sin", reinterpret_cast<SQFUNCTION>(0x004bbe00), 2, kMaskN },
    // STRING: VC1 0x004d2de8
    { "cos", reinterpret_cast<SQFUNCTION>(0x004bbda0), 2, kMaskN },
    // STRING: VC1 0x004d2de0
    { "asin", reinterpret_cast<SQFUNCTION>(0x004bbd40), 2, kMaskN },
    // STRING: VC1 0x004d2dd8
    { "acos", reinterpret_cast<SQFUNCTION>(0x004bbce0), 2, kMaskN },
    // STRING: VC1 0x004d2dd4
    { "log", reinterpret_cast<SQFUNCTION>(0x004bbc80), 2, kMaskN },
    // STRING: VC1 0x004d2dcc
    { "log10", reinterpret_cast<SQFUNCTION>(0x004bbbf0), 2, kMaskN },
    // STRING: VC1 0x004d2dc8
    { "tan", reinterpret_cast<SQFUNCTION>(0x004bbb90), 2, kMaskN },
    // STRING: VC1 0x004d2dc0
    { "atan", reinterpret_cast<SQFUNCTION>(0x004bbb30), 2, kMaskN },
    // STRING: VC1 0x004d2db8
    { "atan2", reinterpret_cast<SQFUNCTION>(0x004bbaa0), 3, kMaskNN },
    // STRING: VC1 0x004d2db0
    { "pow", reinterpret_cast<SQFUNCTION>(0x004bba10), 3, kMaskNN },
    // STRING: VC1 0x004d2da8
    { "floor", reinterpret_cast<SQFUNCTION>(0x004bb9a0), 2, kMaskN },
    // STRING: VC1 0x004d2da0
    { "ceil", reinterpret_cast<SQFUNCTION>(0x004bb940), 2, kMaskN },
    // STRING: VC1 0x004d2d9c
    { "exp", reinterpret_cast<SQFUNCTION>(0x004bb8e0), 2, kMaskN },
    // STRING: VC1 0x004d2d94
    { "srand", reinterpret_cast<SQFUNCTION>(0x004bbfa0), 2, kMaskN },
    // STRING: VC1 0x004d2d8c
    { "rand", reinterpret_cast<SQFUNCTION>(0x004bb7b0), 1, 0 },
    // STRING: VC1 0x004d2d84
    { "fabs", reinterpret_cast<SQFUNCTION>(0x004bbe60), 2, kMaskN },
    // STRING: VC1 0x004d2d80
    { "abs", reinterpret_cast<SQFUNCTION>(0x004bbf20), 2, kMaskN },
    // STRING: VC1 0x004d2d78
    { "round", reinterpret_cast<SQFUNCTION>(0x004bb890), 2, kMaskN },
    // STRING: VC1 0x004d2d70
    { "fmod", reinterpret_cast<SQFUNCTION>(0x004bb800), 3, kMaskNN },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e8a00
SQRegFunction g_streamMethods[] = {
    // STRING: VC1 0x004d3fa0
    { "readblob", reinterpret_cast<SQFUNCTION>(0x004c22c0), 2, kMaskXN },
    // STRING: VC1 0x004d3f98
    { "readn", reinterpret_cast<SQFUNCTION>(0x004c1d00), 2, kMaskXN },
    // STRING: VC1 0x004d3f8c
    { "writeblob", reinterpret_cast<SQFUNCTION>(0x004c1f70), -2, kMaskXX },
    // STRING: VC1 0x004d3f80
    { "writen", reinterpret_cast<SQFUNCTION>(0x004c1ab0), 3, kMaskXNN },
    // STRING: VC1 0x004d28b0
    { "seek", reinterpret_cast<SQFUNCTION>(0x004c1920), -2, kMaskXNN },
    // STRING: VC1 0x004d3f78
    { "tell", reinterpret_cast<SQFUNCTION>(0x004c1850), 1, kMaskX },
    // STRING: VC1 0x004d3c88
    { "len", reinterpret_cast<SQFUNCTION>(0x004c1780), 1, kMaskX },
    // STRING: VC1 0x004d3f74
    { "eos", reinterpret_cast<SQFUNCTION>(0x004c1620), 1, kMaskX },
    // STRING: VC1 0x004d3f6c
    { "flush", reinterpret_cast<SQFUNCTION>(0x004c16d0), 1, kMaskX },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e9270
SQRegFunction g_blobMethods[] = {
    { "constructor", reinterpret_cast<SQFUNCTION>(0x0048ea10), -1, kMaskXN },
    // STRING: VC1 0x004d3224
    { "resize", reinterpret_cast<SQFUNCTION>(0x0048efc0), 2, kMaskXN },
    // STRING: VC1 0x004d321c
    { "swap2", reinterpret_cast<SQFUNCTION>(0x0048ee40), 1, kMaskX },
    // STRING: VC1 0x004d3214
    { "swap4", reinterpret_cast<SQFUNCTION>(0x0048ef00), 1, kMaskX },
    // STRING: VC1 0x004d320c
    { "_set", reinterpret_cast<SQFUNCTION>(0x0048ed30), 3, kMaskXNN },
    // STRING: VC1 0x004d3200
    { "_get", reinterpret_cast<SQFUNCTION>(0x0048ec60), 2, kMaskXN },
    { "_typeof", reinterpret_cast<SQFUNCTION>(0x0048eb20), 1, kMaskX },
    // STRING: VC1 0x004d31f8
    { "_nexti", reinterpret_cast<SQFUNCTION>(0x0048eb40), 2, kMaskX },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e9300
SQRegFunction g_blobGlobals[] = {
    // STRING: VC1 0x004d31f0
    { "casti2f", reinterpret_cast<SQFUNCTION>(0x0048e980), 2, kMaskN },
    // STRING: VC1 0x004d31e8
    { "castf2i", reinterpret_cast<SQFUNCTION>(0x0048e910), 2, kMaskN },
    { "swap2", reinterpret_cast<SQFUNCTION>(0x0048e890), 2, kMaskN },
    { "swap4", reinterpret_cast<SQFUNCTION>(0x0048e7f0), 2, kMaskN },
    // STRING: VC1 0x004d31dc
    { "swapfloat", reinterpret_cast<SQFUNCTION>(0x0048e780), 2, kMaskN },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e9360
SQRegFunction g_fileMethods[] = {
    { "constructor", reinterpret_cast<SQFUNCTION>(0x0048f620), 3, kMaskX },
    { "_typeof", reinterpret_cast<SQFUNCTION>(0x0048f600), 1, kMaskX },
    { 0, 0, 0, 0 }
};

// GLOBAL: VC1 0x004e9390
SQRegFunction g_fileGlobals[] = {
    // STRING: VC1 0x004d2ef8
    { "loadfile", IoLoadFile, -2, kMaskSB },
    // STRING: VC1 0x004d2eec
    { "dofile", reinterpret_cast<SQFUNCTION>(0x0048fd20), -2, kMaskSB },
    // STRING: VC1 0x004d2ed8
    { "writeclosuretofile", reinterpret_cast<SQFUNCTION>(0x0048f830), 3, kMaskSC },
    { 0, 0, 0, 0 }
};

// EAX=vm. sqstd_register_stringlib: regexp class then format/strip helpers.
// FUNCTION: VC1 0x0048e420
int RegisterStdString(HSQUIRRELVM vm)
{
    // STRING: VC1 0x004d342c
    sq_pushstring(vm, "regexp", -1);
    sq_newclass(vm, SQFalse);
    RegisterNativeFuncs(vm, g_rexobjFuncs, 1);
    sq_newslot(vm, -3, SQFalse);
    RegisterNativeFuncs(vm, g_stringlibFuncs, 1);
    return 1;
}

// EAX=idx, ESI=vm. Shift objects down and write g_sqNull at the old top.
// FUNCTION: VC1 0x0048ff90
void SqRemove(HSQUIRRELVM vm, SQInteger idx)
{
    char* const stack = *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x18);
    SQInteger* const pTop = reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    const SQInteger stackbase = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);
    SQInteger i;
    HSQOBJECT* slot;
    HSQOBJECT next;
    unsigned oldType;
    int* oldValue;

    if (idx >= 0) {
        i = idx + stackbase - 1;
    }
    else {
        i = idx + *pTop;
    }
    while (i < *pTop) {
        slot = reinterpret_cast<HSQOBJECT*>(stack + i * 8);
        oldType = slot->_type;
        oldValue = reinterpret_cast<int*>(slot->_unVal.pUserPointer);
        next = slot[1];
        *slot = next;
        if ((next._type & kSqRefCounted) != 0) {
            reinterpret_cast<int*>(next._unVal.pUserPointer)[1] += 1;
        }
        if ((oldType & kSqRefCounted) != 0) {
            oldValue[1] -= 1;
            if (oldValue[1] == 0) {
                UnmappedReleaseRefCounted(oldValue);
            }
        }
        i += 1;
    }
    slot = reinterpret_cast<HSQOBJECT*>(stack + *pTop * 8);
    oldType = slot->_type;
    oldValue = reinterpret_cast<int*>(slot->_unVal.pUserPointer);
    slot->_unVal = g_sqNull._unVal;
    slot->_type = g_sqNull._type;
    if ((slot->_type & kSqRefCounted) != 0) {
        reinterpret_cast<int*>(slot->_unVal.pUserPointer)[1] += 1;
    }
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
        *pTop -= 1;
        return;
    }
    *pTop -= 1;
}

// ESI=vm. Copy stack top onto vm+0x48 if closure/nativeclosure/null, then poptop.
// FUNCTION: VC1 0x00497430
void SetErrorHandler(HSQUIRRELVM vm)
{
    HSQOBJECT* const obj = StackObj(vm, -1);
    const unsigned type = obj->_type;
    void* const value = obj->_unVal.pUserPointer;
    unsigned oldType;
    int* oldValue;

    if (type != OT_CLOSURE && type != OT_NATIVECLOSURE && type != OT_NULL) {
        return;
    }
    oldType = *reinterpret_cast<unsigned*>(reinterpret_cast<char*>(vm) + 0x48);
    oldValue = *reinterpret_cast<int**>(reinterpret_cast<char*>(vm) + 0x4c);
    *reinterpret_cast<void**>(reinterpret_cast<char*>(vm) + 0x4c) = value;
    *reinterpret_cast<unsigned*>(reinterpret_cast<char*>(vm) + 0x48) = type;
    if ((type & kSqRefCounted) != 0) {
        reinterpret_cast<int*>(value)[1] += 1;
    }
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
    sq_poptop(vm);
}

// EAX=vm, EDX=name. Top must be OT_NATIVECLOSURE. Interns name at closure+0x40.
// FUNCTION: VC1 0x00497ea0
SQInteger SetNativeClosureName(HSQUIRRELVM vm, const SQChar* name)
{
    HSQOBJECT* const obj = StackObj(vm, -1);
    char* closure;
    void* ss;
    int* interned;
    unsigned oldType;
    int* oldValue;

    if (obj->_type != OT_NATIVECLOSURE) {
        // STRING: VC1 0x004d2528
        return sq_throwerror(vm, "the object is not a nativeclosure");
    }
    closure = static_cast<char*>(obj->_unVal.pUserPointer);
    ss = *reinterpret_cast<void**>(reinterpret_cast<char*>(vm) + 0x8c);
    interned = static_cast<int*>(UnmappedCreateString(
        *reinterpret_cast<void**>(static_cast<char*>(ss) + 0x14), name));
    reinterpret_cast<void**>(interned)[3] = ss;
    interned[1] += 1;
    oldType = *reinterpret_cast<unsigned*>(closure + 0x40);
    oldValue = *reinterpret_cast<int**>(closure + 0x44);
    *reinterpret_cast<int**>(closure + 0x44) = interned;
    *reinterpret_cast<unsigned*>(closure + 0x40) = OT_STRING;
    interned[1] += 1;
    if ((oldType & kSqRefCounted) != 0) {
        oldValue[1] -= 1;
        if (oldValue[1] == 0) {
            UnmappedReleaseRefCounted(oldValue);
        }
    }
    interned[1] -= 1;
    if (interned[1] == 0) {
        UnmappedReleaseRefCounted(interned);
    }
    return 0;
}

// cdecl vm. Printfunc at sharedstate+0xa4, then printcallstack.
// FUNCTION: VC1 0x004b8680
SQInteger AuxPrintError(HSQUIRRELVM vm)
{
    typedef void (*PrintFn)(HSQUIRRELVM v, const SQChar* fmt, ...);
    PrintFn printfn;
    const SQChar* err;
    const SQInteger top = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30);
    const SQInteger base = *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);

    printfn = *reinterpret_cast<PrintFn*>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa4);
    if (!printfn) {
        return 0;
    }
    if (top - base < 1) {
        return 0;
    }
    err = 0;
    if (UnmappedGetStringIdx(vm, 2, &err) >= 0) {
        // STRING: VC1 0x004d318c
        printfn(vm, "error: [%s]\n", err);
        UnmappedPrintCallstack(vm);
        return 0;
    }
    // STRING: VC1 0x004d319c
    printfn(vm, "error: [unknown]\n");
    UnmappedPrintCallstack(vm);
    return 0;
}

// EAX=vm. Compiler error at sharedstate+0xa0, then printerror closure + seterrorhandler.
// FUNCTION: VC1 0x004b8700
void RegisterStdAux(HSQUIRRELVM vm)
{
    *reinterpret_cast<void**>(
        *reinterpret_cast<char**>(reinterpret_cast<char*>(vm) + 0x8c) + 0xa0)
        = reinterpret_cast<void*>(0x004b82d0);
    sq_newclosure(vm, AuxPrintError, 0);
    SetErrorHandler(vm);
}

// EAX=vm. sqstd_register_systemlib analog.
// FUNCTION: VC1 0x004bb6c0
int RegisterStdSystem(HSQUIRRELVM vm)
{
    RegisterNativeFuncs(vm, g_systemlibFuncs, 1);
    return 1;
}

// EAX=vm. sqstd_register_mathlib plus DBPI/HALFPI/INVDBPI.
// FUNCTION: VC1 0x004bbfd0
int RegisterStdMath(HSQUIRRELVM vm)
{
    RegisterNativeFuncs(vm, g_mathlibFuncs, 1);
    // STRING: VC1 0x004d2e0c
    sq_pushstring(vm, "RAND_MAX", -1);
    PushRawInteger(vm, 0x7fff);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d2e18
    sq_pushstring(vm, "PI", -1);
    PushRawFloatBits(vm, 0x40490fdb);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d2e1c
    sq_pushstring(vm, "DBPI", -1);
    PushRawFloatBits(vm, 0x40c90fdb);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d2e24
    sq_pushstring(vm, "HALFPI", -1);
    PushRawFloatBits(vm, 0x3fc90fdb);
    sq_newslot(vm, -3, SQFalse);
    // STRING: VC1 0x004d2e2c
    sq_pushstring(vm, "INVDBPI", -1);
    PushRawFloatBits(vm, 0x3e22f983);
    sq_newslot(vm, -3, SQFalse);
    return 0;
}

// EAX=vm. Create registry std_stream / root stream if missing.
// FUNCTION: VC1 0x004c20a0
void InitStreamClass(HSQUIRRELVM vm)
{
    sq_pushregistrytable(vm);
    sq_pushstring(vm, kStdStream, -1);
    if (UnmappedGet(vm)) {
        sq_pop(vm, 1);
        sq_pop(vm, 1);
        return;
    }
    sq_pop(vm, 1);
    if (sq_throwerror(vm, kIndexMissing) >= 0) {
        sq_pop(vm, 1);
        sq_pop(vm, 1);
        return;
    }
    sq_pushstring(vm, kStdStream, -1);
    sq_newclass(vm, SQFalse);
    sq_settypetag(vm, -1, reinterpret_cast<SQUserPointer>(0x80000000));
    RegisterNativeFuncs(vm, g_streamMethods, 0);
    sq_newslot(vm, -3, SQFalse);
    sq_pushroottable(vm);
    // STRING: VC1 0x004d400c
    sq_pushstring(vm, "stream", -1);
    sq_pushstring(vm, kStdStream, -1);
    if (!UnmappedGet(vm)) {
        sq_pop(vm, 1);
        sq_throwerror(vm, kIndexMissing);
    }
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
    sq_pop(vm, 1);
}

// EAX=vm, cdecl name, typetag, reg_name, methods, globals. sqstd declare_stream.
// FUNCTION: VC1 0x004c2420
int DeclareStream(HSQUIRRELVM vm, const SQChar* name, SQUserPointer typetag,
    const SQChar* regName, SQRegFunction* methods, SQRegFunction* globals)
{
    const SQInteger savedTop =
        *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x30)
        - *reinterpret_cast<SQInteger*>(reinterpret_cast<char*>(vm) + 0x34);

    if (StackObj(vm, -1)->_type != OT_TABLE) {
        // STRING: VC1 0x004d4014
        return sq_throwerror(vm, "table expected");
    }
    InitStreamClass(vm);
    sq_pushregistrytable(vm);
    sq_pushstring(vm, regName, -1);
    sq_pushstring(vm, kStdStream, -1);
    if (!UnmappedGet(vm)) {
        sq_pop(vm, 1);
        if (sq_throwerror(vm, kIndexMissing) < 0) {
            sq_settop(vm, savedTop);
            return -1;
        }
    }
    sq_newclass(vm, SQTrue);
    sq_settypetag(vm, -1, typetag);
    RegisterNativeFuncs(vm, methods, 1);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 1);
    RegisterNativeFuncs(vm, globals, 1);
    sq_pushstring(vm, name, -1);
    sq_pushregistrytable(vm);
    sq_pushstring(vm, regName, -1);
    if (!UnmappedGet(vm)) {
        sq_pop(vm, 1);
        sq_throwerror(vm, kIndexMissing);
    }
    SqRemove(vm, -2);
    sq_newslot(vm, -3, SQFalse);
    sq_settop(vm, savedTop);
    return 0;
}

}
