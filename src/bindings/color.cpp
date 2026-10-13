#include "color.h"
#include "value_types.h"
#include "memory/fixed_pool.h"

namespace vc1 {

static NativeTypeInfo g_colorType;
static unsigned g_colorTypeReady;
static const int kColorTypeSpec = 0;

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

// this in ESI. Fills NativeClassBinding {SquirrelVM::_VM, "Color", obj} then RegisterClass + Assign.
// FUNCTION: VC1 0x0041de10
NativeClassBinding* Color::CreateBinding(NativeClassBinding* self)
{
    return CreateValueBinding(self, "Color", Color::RegisterClass);
}

// this in EDI. Pushes binding->obj, RegisterNativeSlot("constructor", ConstructArgs, "*").
// FUNCTION: VC1 0x0041e440
NativeClassBinding* Color::BindArgConstructor(NativeClassBinding* self)
{
    return BindStarConstructor(self, Color::ConstructArgs);
}

// Creates Color class, binds Constructor (null typemask) and TypeOf. Once-flag 0x004ee3ec,
// type info 0x004ee214, spec 0x004d74b8, name 0x004d7580. Helper 0x004c3240.
// FUNCTION: VC1 0x0041eff0
SquirrelObject Color::RegisterClass(HSQUIRRELVM vm,
    const SQChar* className)
{
    return RegisterClassType<Color>(vm, className, &g_colorType, &g_colorTypeReady,
        &kColorTypeSpec, "Color", Color::Constructor, Color::TypeOf);
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

// FUNCTION: VC1 0x00423640
SQInteger Color::TypeOf(HSQUIRRELVM vm)
{
    sq_pushstring(vm, "Color", -1);
    return 1;
}

// Bound as "_cmp" (0x004d36e0) from 0x00420490. this in ECX; other on stack.
// FUNCTION: VC1 0x00428c50
SQInteger Color::Cmp(const Color* other)
{
    return MemCompare(this, other, sizeof(Color));
}

// Bound as "GetBlue" (0x004d6b70) via 0x00422b70. this in ECX.
// Scale qword 0x004d8a58 is a truncated 1/255 (0x3f70101020000000).
// FUNCTION: VC1 0x00428c70
SQFloat Color::GetBlue()
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb) * (1.0 / 255.0));
}

// Bound as "GetGreen" (0x004d6b64). MOV EAX,[this]; SHR 8; MOVZX AL.
// FUNCTION: VC1 0x00428c90
SQFloat Color::GetGreen()
{
    return static_cast<SQFloat>(((argb >> 8) & 0xff) * (1.0 / 255.0));
}

// Bound as "GetRed" (0x004d6b5c). this in ECX; [argb+2].
// FUNCTION: VC1 0x00428cb0
SQFloat Color::GetRed()
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb >> 16) * (1.0 / 255.0));
}

// Bound as "GetAlpha" (0x004d6b50) from 0x004bfa1f. this in ECX; [argb+3].
// FUNCTION: VC1 0x00428cd0
SQFloat Color::GetAlpha()
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb >> 24) * (1.0 / 255.0));
}

static SQUnsignedInteger ColorByte(SQInteger value)
{
    return static_cast<SQUnsignedInteger>(value) & 0xff;
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
