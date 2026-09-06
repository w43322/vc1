#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

// Bound as "GetA" (0x004d6b30) from 0x004bf9af. this in ECX; returns [argb+3].
// FUNCTION: VC1 0x00418480
unsigned char Color::GetA()
{
    return static_cast<unsigned char>(argb >> 24);
}

// Bound as "GetR" (0x004d6b38) from 0x004bf9cb. this in ECX; returns [argb+2].
// FUNCTION: VC1 0x00418490
unsigned char Color::GetR()
{
    return static_cast<unsigned char>(argb >> 16);
}

// Bound as "GetG" (0x004d6b40) from 0x004bf9e7. this in ECX; MOV EAX,[this]; SHR 8.
// FUNCTION: VC1 0x004184a0
SQUnsignedInteger Color::GetG()
{
    return argb >> 8;
}

// Bound as "GetB" (0x004d6b48) from 0x004bfa03. this in ECX; returns [argb+0].
// FUNCTION: VC1 0x004184b0
unsigned char Color::GetB()
{
    return static_cast<unsigned char>(argb);
}

// Packs A,R,G,B bytes; string "MakeARGB" (0x004d6b78). cdecl, no this.
// FUNCTION: VC1 0x004184c0
SQUnsignedInteger Color::MakeARGB(unsigned char a, unsigned char r, unsigned char g,
    unsigned char b)
{
    return (static_cast<SQUnsignedInteger>(a) << 24)
        | (static_cast<SQUnsignedInteger>(r) << 16)
        | (static_cast<SQUnsignedInteger>(g) << 8)
        | b;
}

// Bound as "SetValue" (0x004d6b24) from 0x00420280's sibling 0x00420330. thiscall RET 4.
// FUNCTION: VC1 0x00418560
void Color::SetValue(SQUnsignedInteger value)
{
    argb = value;
}

// Bound as "GetValue" (0x004d6b18) from 0x00420280. this in ECX.
// FUNCTION: VC1 0x00418570
SQUnsignedInteger Color::GetValue()
{
    return argb;
}

// SQRELEASEHOOK; size unused. Puts 8 bytes back on pools[2].
// FUNCTION: VC1 0x0041dd00
SQInteger Vector2::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector2);
    }
    return 0;
}

// SQRELEASEHOOK; size unused. Puts 16 bytes back on pools[4] / 0x004ed468.
// FUNCTION: VC1 0x0041dd60
SQInteger Vector4::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector4);
    }
    return 0;
}

static NativeClassBinding* CreateValueBinding(NativeClassBinding* self, const SQChar* name,
    SquirrelObject* (*registerClass)(SquirrelObject*, HSQUIRRELVM, const SQChar*))
{
    SquirrelObject created;
    self->vm = g_vm;
    self->className = name;
    InitSquirrelObject(&self->obj);
    if (!self->className) {
        self->className = name;
    }
    registerClass(&created, self->vm, self->className);
    AssignSquirrelObject(&self->obj, &created);
    sq_release(g_vm, &created.obj);
    return self;
}

// this in ESI. Fills NativeClassBinding {g_vm, "Color", obj} then RegisterClass + Assign.
// FUNCTION: VC1 0x0041de10
NativeClassBinding* Color::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Color", Color::RegisterClass);
}

// this in ESI. Name 0x004d7588.
// FUNCTION: VC1 0x0041dea0
NativeClassBinding* Vector3::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector3", Vector3::RegisterClass);
}

// this in ESI. Name 0x004cfb44.
// FUNCTION: VC1 0x0041df30
NativeClassBinding* Vector2::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector2", Vector2::RegisterClass);
}

// this in ESI. Name 0x004d7590.
// FUNCTION: VC1 0x0041dfc0
NativeClassBinding* Vector4::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Vector4", Vector4::RegisterClass);
}

// FUNCTION: VC1 0x0041e3b0
SQInteger BindNativeInstance(HSQUIRRELVM vm, SQUserPointer nativeObject,
    SQRELEASEHOOK releaseHook)
{
    sq_setinstanceup(vm, 1, nativeObject);
    sq_setreleasehook(vm, 1, releaseHook);
    return 1;
}

static NativeClassBinding* BindStarConstructor(NativeClassBinding* self, SQFUNCTION fn)
{
    SquirrelObject slot;
    sq_pushobject(g_vm, self->obj.obj);
    RegisterNativeSlot(&slot, "constructor", fn, "*");
    sq_release(g_vm, &slot.obj);
    sq_pop(g_vm, 1);
    return self;
}

// this in EDI. Pushes binding->obj, RegisterNativeSlot("constructor", ConstructArgs, "*").
// FUNCTION: VC1 0x0041e440
NativeClassBinding* Color::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Color::ConstructArgs);
}

// FUNCTION: VC1 0x0041e4c0
NativeClassBinding* Vector3::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector3::ConstructArgs);
}

// FUNCTION: VC1 0x0041e620
NativeClassBinding* Vector2::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector2::ConstructArgs);
}

// FUNCTION: VC1 0x0041e780
NativeClassBinding* Vector4::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Vector4::ConstructArgs);
}

struct NativeTypeInfo
{
    const void* spec;
    void* unused1;
    const SQChar* typeName;
    void* unused2;
    int extra;
};

static NativeTypeInfo g_colorType;
static NativeTypeInfo g_vector2Type;
static NativeTypeInfo g_vector3Type;
static NativeTypeInfo g_vector4Type;
static unsigned g_colorTypeReady;
static unsigned g_vector2TypeReady;
static unsigned g_vector3TypeReady;
static unsigned g_vector4TypeReady;
static const int kColorTypeSpec = 0;
static const int kVector2TypeSpec = 0;
static const int kVector3TypeSpec = 0;
static const int kVector4TypeSpec = 0;

static void EnsureTypeInfo(NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName)
{
    if ((*ready & 1) == 0) {
        *ready |= 1;
        type->unused1 = 0;
        type->unused2 = 0;
        type->extra = -1;
        type->spec = spec;
        type->typeName = typeName;
    }
}

static void BindNamedMethod(SquirrelObject* classObject, const SQChar* name,
    SQFUNCTION fn)
{
    SquirrelObject slot;
    sq_pushobject(g_vm, classObject->obj);
    RegisterNativeSlot(&slot, name, fn, 0);
    sq_pop(g_vm, 1);
    sq_release(g_vm, &slot.obj);
}

static SquirrelObject* RegisterValueClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className, NativeTypeInfo* type, unsigned* ready,
    const void* spec, const SQChar* typeName, SQFUNCTION ctor, SQFUNCTION typeOf)
{
    const SQInteger savedTop = sq_gettop(vm);
    InitSquirrelObject(out);
    EnsureTypeInfo(type, ready, spec, typeName);
    if (CreateNativeClass(vm, out, type, className)) {
        BindNamedMethod(out, "constructor", ctor);
        BindNamedMethod(out, "_typeof", typeOf);
    }
    sq_settop(vm, savedTop);
    return out;
}

// Creates Color class, binds Constructor (null typemask) and TypeOf. Once-flag 0x004ee3ec,
// type info 0x004ee214, spec 0x004d74b8, name 0x004d7580. Helper 0x004c3240.
// FUNCTION: VC1 0x0041eff0
SquirrelObject* Color::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_colorType, &g_colorTypeReady,
        &kColorTypeSpec, "Color", Color::Constructor, Color::TypeOf);
}

// Once-flag 0x004ee3e0, type info 0x004ee1d8, spec 0x004d75b8, name 0x004d7588.
// FUNCTION: VC1 0x0041f150
SquirrelObject* Vector3::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_vector3Type, &g_vector3TypeReady,
        &kVector3TypeSpec, "Vector3", Vector3::Constructor, Vector3::TypeOf);
}

// Once-flag 0x004ea818, type info 0x004ea830, spec 0x004cfb9c, name 0x004cfb44.
// FUNCTION: VC1 0x0041f2b0
SquirrelObject* Vector2::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_vector2Type, &g_vector2TypeReady,
        &kVector2TypeSpec, "Vector2", Vector2::Constructor, Vector2::TypeOf);
}

// Once-flag 0x004ee3e8, type info 0x004ee200, spec 0x004cfba4, name 0x004d7590.
// FUNCTION: VC1 0x0041f410
SquirrelObject* Vector4::RegisterClass(SquirrelObject* out, HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterValueClass(out, vm, className, &g_vector4Type, &g_vector4TypeReady,
        &kVector4TypeSpec, "Vector4", Vector4::Constructor, Vector4::TypeOf);
}

// Squirrel class constructor registered at 0x0041eff0 as "constructor" (0x004d2f10)
// with null typemask. Does not read extra stack args. Arg-taking factory is 0x004bed80.
// FUNCTION: VC1 0x0041fe10
SQInteger Color::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Color);
    Color* const color = static_cast<Color*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Allocate());
    if (color) {
        color->argb = 0xff000000;
    }
    return BindNativeInstance(vm, color, Color::ReleaseHook);
}

// Registered from 0x0041f150 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyz. Arg-taking factory is 0x004beb40.
// FUNCTION: VC1 0x0041fe50
SQInteger Vector3::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector3);
    Vector3* const vector = static_cast<Vector3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector3::ReleaseHook);
}

// FUNCTION: VC1 0x0041fea0
SQInteger Vector2::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector2);
    Vector2* const vector = static_cast<Vector2*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector2::ReleaseHook);
}

// Registered from 0x0041f410 as "constructor" (0x004d2f10) with null typemask.
// Zeros xyzw. Release hook also used from 0x004bdd60 / 0x004bdf10 / 0x004be0e0 / 0x004be6c0.
// FUNCTION: VC1 0x0041fee0
SQInteger Vector4::Constructor(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector4);
    Vector4* const vector = static_cast<Vector4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Allocate());
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        vector->w = 0.0f;
    }
    return BindNativeInstance(vm, vector, Vector4::ReleaseHook);
}

// FUNCTION: VC1 0x004201f0
SQInteger Color::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Color);
    }
    return 0;
}

// SQRELEASEHOOK; size unused. Puts 12 bytes back on pools[3] / 0x004ed464.
// FUNCTION: VC1 0x00420210
SQInteger Vector3::ReleaseHook(SQUserPointer object, SQInteger size)
{
    (void)size;
    if (object) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Release(object);
        g_sizeClassHeap.releasedBytes += sizeof(Vector3);
    }
    return 0;
}

// FUNCTION: VC1 0x00423640
SQInteger Color::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Color", -1);
    return 1;
}

// FUNCTION: VC1 0x00423660
SQInteger Vector3::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector3", -1);
    return 1;
}

// FUNCTION: VC1 0x00423680
SQInteger Vector2::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector2", -1);
    return 1;
}

// FUNCTION: VC1 0x004236a0
SQInteger Vector4::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Vector4", -1);
    return 1;
}

static SQFloat NumericFloat(HSQUIRRELVM vm, SQInteger idx)
{
    SQFloat value = 0.0f;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getfloat(vm, idx, &value);
    }
    return value;
}

static SQInteger NumericInt(HSQUIRRELVM vm, SQInteger idx)
{
    SQInteger value = 0;
    const SQObjectType type = sq_gettype(vm, idx);
    if (type == OT_INTEGER || type == OT_FLOAT) {
        sq_getinteger(vm, idx, &value);
    }
    return value;
}

static SQUnsignedInteger ColorByte(SQInteger value)
{
    return static_cast<SQUnsignedInteger>(value) & 0xff;
}

// Registered from 0x0041e780 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004be9c0. Error string 0x004d6a38.
// FUNCTION: VC1 0x004be6c0
SQInteger Vector4::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector4);
    Vector4* const vector = static_cast<Vector4*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector4))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector4);
        return sq_throwerror(vm, "constructVector4: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        vector->w = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
            vector->z = scalar;
            vector->w = scalar;
        }
        else if (extra >= 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
            if (extra >= 3) {
                vector->z = NumericFloat(vm, 4);
            }
            if (extra >= 4) {
                vector->w = NumericFloat(vm, 5);
            }
        }
    }
    return BindNativeInstance(vm, vector, Vector4::ReleaseHook);
}

// Registered from 0x0041e620 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1. Error string 0x004d6a14.
// FUNCTION: VC1 0x004be9e0
SQInteger Vector2::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector2);
    Vector2* const vector = static_cast<Vector2*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 2) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector2))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector2);
        return sq_throwerror(vm, "constructVector2: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
        }
        else if (extra == 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
        }
    }
    return BindNativeInstance(vm, vector, Vector2::ReleaseHook);
}

// Registered from 0x0041e4c0 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004bed64. Error string 0x004d69f0.
// FUNCTION: VC1 0x004beb40
SQInteger Vector3::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Vector3);
    Vector3* const vector = static_cast<Vector3*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra > 3) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Vector3))]->Release(vector);
        g_sizeClassHeap.releasedBytes += sizeof(Vector3);
        return sq_throwerror(vm, "constructVector3: invalid arguments");
    }
    if (vector) {
        vector->x = 0.0f;
        vector->y = 0.0f;
        vector->z = 0.0f;
        if (extra == 1) {
            const SQFloat scalar = NumericFloat(vm, 2);
            vector->x = scalar;
            vector->y = scalar;
            vector->z = scalar;
        }
        else if (extra >= 2) {
            vector->x = NumericFloat(vm, 2);
            vector->y = NumericFloat(vm, 3);
            if (extra >= 3) {
                vector->z = NumericFloat(vm, 4);
            }
        }
    }
    return BindNativeInstance(vm, vector, Vector3::ReleaseHook);
}

// Registered from 0x0041e440 as "constructor" (0x004d2f10) with typemask "*" (0x004d6b14).
// extra = gettop-1; jump table 0x004bf058. extra==2 is invalid. Error string 0x004d69cc.
// FUNCTION: VC1 0x004bed80
SQInteger Color::ConstructArgs(HSQUIRRELVM vm)
{
    g_sizeClassHeap.allocatedBytes += sizeof(Color);
    Color* const color = static_cast<Color*>(
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Allocate());
    const unsigned extra = static_cast<unsigned>(sq_gettop(vm) - 1);
    if (extra == 2 || extra > 4) {
        g_sizeClassHeap.pools[SizeClassIndex(sizeof(Color))]->Release(color);
        g_sizeClassHeap.releasedBytes += sizeof(Color);
        return sq_throwerror(vm, "constructColor: invalid arguments");
    }
    if (color) {
        if (extra == 1) {
            color->argb = static_cast<SQUnsignedInteger>(NumericInt(vm, 2));
        }
        else if (extra == 3) {
            color->argb = 0xff000000
                | (ColorByte(NumericInt(vm, 2)) << 16)
                | (ColorByte(NumericInt(vm, 3)) << 8)
                | ColorByte(NumericInt(vm, 4));
        }
        else if (extra == 4) {
            color->argb = (ColorByte(NumericInt(vm, 2)) << 24)
                | (ColorByte(NumericInt(vm, 3)) << 16)
                | (ColorByte(NumericInt(vm, 4)) << 8)
                | ColorByte(NumericInt(vm, 5));
        }
        else {
            color->argb = 0xff000000;
        }
    }
    return BindNativeInstance(vm, color, Color::ReleaseHook);
}

}
