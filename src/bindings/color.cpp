#include "color.h"
#include "value_types.h"
#include "bindings/sqplus.h"

namespace vc1 {

// Bound as "GetA" (0x004d6b30) through 0x004203e0. this in ECX; [argb+3].
// FUNCTION: VC1 0x00418480
unsigned char Color::GetA()
{
    return static_cast<unsigned char>(argb >> 24);
}

// Bound as "GetR" (0x004d6b38) through 0x004203e0. this in ECX; [argb+2].
// FUNCTION: VC1 0x00418490
unsigned char Color::GetR()
{
    return static_cast<unsigned char>(argb >> 16);
}

// Bound as "GetG" (0x004d6b40) through 0x004203e0. MOV EAX,[this]; SHR 8.
// FUNCTION: VC1 0x004184a0
unsigned char Color::GetG()
{
    return static_cast<unsigned char>(argb >> 8);
}

// Bound as "GetB" (0x004d6b48) through 0x004203e0. this in ECX; [argb+0].
// FUNCTION: VC1 0x004184b0
unsigned char Color::GetB()
{
    return static_cast<unsigned char>(argb);
}

// Static "MakeARGB" (0x004d6b78), registered through 0x004205f0/0x004238d0.
// FUNCTION: VC1 0x004184c0
SQUnsignedInteger Color::MakeARGB(unsigned char a, unsigned char r, unsigned char g,
    unsigned char b)
{
    return (static_cast<SQUnsignedInteger>(a) << 24)
        | (static_cast<SQUnsignedInteger>(r) << 16)
        | (static_cast<SQUnsignedInteger>(g) << 8)
        | b;
}

// Bound as "SetValue" (0x004d6b24) through 0x00420330. thiscall RET 4.
// FUNCTION: VC1 0x00418560
void Color::SetValue(SQUnsignedInteger value)
{
    argb = value;
}

// Bound as "GetValue" (0x004d6b18) through 0x00420280. this in ECX.
// FUNCTION: VC1 0x00418570
SQUnsignedInteger Color::GetValue()
{
    return argb;
}

// EBX=this, out and other on the stack (cdecl). Used by LayerSetColor::Execute
// (0x004188e0) to tint a recorded color by the target's.
// FUNCTION: VC1 0x00419730
Color Color::operator*(const Color& other) const
{
    return Color(
        static_cast<unsigned char>(static_cast<int>(argb >> 24) * static_cast<int>(other.argb >> 24) / 255),
        static_cast<unsigned char>(static_cast<int>((argb >> 16) & 0xff)
            * static_cast<int>((other.argb >> 16) & 0xff) / 255),
        static_cast<unsigned char>(static_cast<int>((argb >> 8) & 0xff)
            * static_cast<int>((other.argb >> 8) & 0xff) / 255),
        static_cast<unsigned char>(static_cast<int>(argb & 0xff) * static_cast<int>(other.argb & 0xff) / 255));
}

// Bound as "_tostring" (0x004d3710) through 0x00420540 (dispatcher
// 0x00424470). Function-local static KdString (0x004ee408, guard 0x004ee418)
// with 0x80 reserved characters; prints A, R, G, B.
// FUNCTION: VC1 0x00428b70
KdString& Color::ToString() const
{
    // GLOBAL: VC1 0x004ee408
    static KdString text(0x80, '\0');
    // STRING: VC1 0x004d74c0
    text.Format("Color(%d, %d, %d, %d)", argb >> 24, (argb >> 16) & 0xff,
        (argb >> 8) & 0xff, argb & 0xff);
    return text;
}

// Bound as "_cmp" (0x004d36e0) through 0x00420490. this in ECX; other on stack.
// FUNCTION: VC1 0x00428c50
SQInteger Color::Cmp(const Color& other) const
{
    return MemCompare(this, &other, sizeof(Color));
}

// Bound as "GetBlue" (0x004d6b70) via 0x00422b70. this in ECX.
// Scale qword 0x004d8a58 is a truncated 1/255 (0x3f70101020000000).
// FUNCTION: VC1 0x00428c70
SQFloat Color::GetBlue() const
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb) * (1.0 / 255.0));
}

// Bound as "GetGreen" (0x004d6b64). MOV EAX,[this]; SHR 8; MOVZX AL.
// FUNCTION: VC1 0x00428c90
SQFloat Color::GetGreen() const
{
    return static_cast<SQFloat>(((argb >> 8) & 0xff) * (1.0 / 255.0));
}

// Bound as "GetRed" (0x004d6b5c). this in ECX; [argb+2].
// FUNCTION: VC1 0x00428cb0
SQFloat Color::GetRed() const
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb >> 16) * (1.0 / 255.0));
}

// Bound as "GetAlpha" (0x004d6b50). this in ECX; [argb+3].
// FUNCTION: VC1 0x00428cd0
SQFloat Color::GetAlpha() const
{
    return static_cast<SQFloat>(static_cast<unsigned char>(argb >> 24) * (1.0 / 255.0));
}

// Registered from 0x0041e440 as "constructor" (0x004d2f10) with typemask "*"
// (0x004d6b14). Switch on gettop-1 (jump table 0x004bf058): none, one ARGB
// value, R/G/B (alpha 0xff) or A/R/G/B, each argument read right to left
// with sq_getinteger inlined. Two or more than four throw (0x004d69cc).
// FUNCTION: VC1 0x004bed80
SQInteger Color::ConstructArgs(HSQUIRRELVM vm)
{
    Color* const color = static_cast<Color*>(Color::operator new(sizeof(Color)));
    StackHandler sa(vm);
    switch (sa.GetParamCount() - 1) {
    case 0:
        ::new (color) Color();
        break;
    case 1:
        ::new (color) Color(static_cast<SQUnsignedInteger>(sa.GetInt(2)));
        break;
    case 3:
        ::new (color) Color(static_cast<unsigned char>(sa.GetInt(2)),
            static_cast<unsigned char>(sa.GetInt(3)),
            static_cast<unsigned char>(sa.GetInt(4)));
        break;
    case 4:
        ::new (color) Color(static_cast<unsigned char>(sa.GetInt(2)),
            static_cast<unsigned char>(sa.GetInt(3)),
            static_cast<unsigned char>(sa.GetInt(4)),
            static_cast<unsigned char>(sa.GetInt(5)));
        break;
    default:
        Color::operator delete(color);
        // STRING: VC1 0x004d69cc
        return sa.ThrowError("constructColor: invalid arguments");
    }
    return SqPlus::PostConstruct<Color>(vm, color, SqPlus::ConstructReleaseClass<Color>::release);
}

}
