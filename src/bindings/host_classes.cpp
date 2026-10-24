#include "kdcore/runtime.h"
#include "bindings/sqplus.h"
#include "bindings/value_types.h"
#include "bindings/vector2.h"
#include "bindings/vector3.h"
#include "bindings/vector4.h"
#include "bindings/matrix3.h"
#include "bindings/matrix4.h"
#include "bindings/quaternion.h"
#include "bindings/rect.h"
#include "bindings/rectf.h"
#include "bindings/integral.h"
#include "bindings/component.h"

namespace vc1 {

// Called from InitScriptEngine. The "Component" class first: its newClass is
// copied out of the temporary SQClassDef and gets seven metamethods through
// SquirrelVM::CreateFunction (push, 0x0049a370, sq_pop). Then one temporary
// SqPlus SQClassDef per value type: Color (with its named colour constants),
// Vector3, Vector2, Vector4, Matrix3, Matrix4, Quaternion, Rect, RectF and
// IntegralF. LTCG pushes every by-value argument of a chain before its first
// call.
// FUNCTION: VC1 0x004bf610
void BindHostClasses()
{
    SquirrelObject component(SqPlus::SQClassDef<ComponentRef>("Component").newClass);
    SquirrelVM::CreateFunction(component, &ComponentCmp, "_cmp", "*");
    SquirrelVM::CreateFunction(component, &ComponentGet, "_get", "*");
    SquirrelVM::CreateFunction(component, &ComponentSet, "_set", "*");
    SquirrelVM::CreateFunction(component, &ComponentNextI, "_nexti", "*");
    SquirrelVM::CreateFunction(component, &ComponentTypeOf, "_typeof", "*");
    SquirrelVM::CreateFunction(component, &ComponentToString, "_tostring", "*");
    SquirrelVM::CreateFunction(component, &ComponentCloned, "_cloned", "*");
    SqPlus::SQClassDef<Color>("Color")
        .staticFuncVarArgs(&Color::ConstructArgs, "constructor")
        // STRING: VC1 0x004d6b18
        .func(&Color::GetValue, "GetValue")
        // STRING: VC1 0x004d6b24
        .func(&Color::SetValue, "SetValue")
        // STRING: VC1 0x004d6b30
        .func(&Color::GetA, "GetA")
        // STRING: VC1 0x004d6b38
        .func(&Color::GetR, "GetR")
        // STRING: VC1 0x004d6b40
        .func(&Color::GetG, "GetG")
        // STRING: VC1 0x004d6b48
        .func(&Color::GetB, "GetB")
        // STRING: VC1 0x004d6b50
        .func(&Color::GetAlpha, "GetAlpha")
        // STRING: VC1 0x004d6b5c
        .func(&Color::GetRed, "GetRed")
        // STRING: VC1 0x004d6b64
        .func(&Color::GetGreen, "GetGreen")
        // STRING: VC1 0x004d6b70
        .func(&Color::GetBlue, "GetBlue")
        // STRING: VC1 0x004d36e0
        .func(&Color::Cmp, "_cmp")
        // STRING: VC1 0x004d3710
        .func(&Color::ToString, "_tostring")
        // STRING: VC1 0x004d6b78
        .staticFunc(&Color::MakeARGB, "MakeARGB")
        // STRING: VC1 0x004d6b84
        .constant<int>(0x18, "AlphaShift")
        // STRING: VC1 0x004d6b90
        .constant<int>(0x10, "RedShift")
        // STRING: VC1 0x004d6b9c
        .constant<int>(8, "GreenShift")
        // STRING: VC1 0x004d6ba8
        .constant<int>(0, "BlueShift")
        // STRING: VC1 0x004d6bb4
        .constant<int>(0xff000000, "AlphaMask")
        // STRING: VC1 0x004d6bc0
        .constant<int>(0xff0000, "RedMask")
        // STRING: VC1 0x004d6bc8
        .constant<int>(0xff00, "GreenMask")
        // STRING: VC1 0x004d6bd4
        .constant<int>(0xff, "BlueMask")
        // STRING: VC1 0x004d6be0
        .constant<int>(0xfff0f8ff, "AliceBlue")
        // STRING: VC1 0x004d6bec
        .constant<int>(0xfffaebd7, "AntiqueWhite")
        // STRING: VC1 0x004d6bfc
        .constant<int>(0xff00ffff, "Aqua")
        // STRING: VC1 0x004d6c04
        .constant<int>(0xff7fffd4, "Aquamarine")
        // STRING: VC1 0x004d6c10
        .constant<int>(0xfff0ffff, "Azure")
        // STRING: VC1 0x004d6c18
        .constant<int>(0xfff5f5dc, "Beige")
        // STRING: VC1 0x004d6c20
        .constant<int>(0xffffe4c4, "Bisque")
        // STRING: VC1 0x004d6c28
        .constant<int>(0xff000000, "Black")
        // STRING: VC1 0x004d6c30
        .constant<int>(0xffffebcd, "BlanchedAlmond")
        .constant<int>(0xff0000ff, "Blue")
        // STRING: VC1 0x004d6c48
        .constant<int>(0xff8a2be2, "BlueViolet")
        .constant<int>(0xffa52a2a, "Brown")
        // STRING: VC1 0x004d6c5c
        .constant<int>(0xffdeb887, "BurlyWood")
        // STRING: VC1 0x004d6c68
        .constant<int>(0xff5f9ea0, "CadetBlue")
        // STRING: VC1 0x004d6c74
        .constant<int>(0xff7fff00, "Chartreuse")
        // STRING: VC1 0x004d6c80
        .constant<int>(0xffd2691e, "Chocolate")
        // STRING: VC1 0x004d6c8c
        .constant<int>(0xffff7f50, "Coral")
        // STRING: VC1 0x004d6c94
        .constant<int>(0xff6495ed, "CornflowerBlue")
        // STRING: VC1 0x004d6ca4
        .constant<int>(0xfffff8dc, "Cornsilk")
        // STRING: VC1 0x004d6cb0
        .constant<int>(0xffdc143c, "Crimson")
        .constant<int>(0xff00ffff, "Cyan")
        // STRING: VC1 0x004d6cc0
        .constant<int>(0xff00008b, "DarkBlue")
        // STRING: VC1 0x004d6ccc
        .constant<int>(0xff008b8b, "DarkCyan")
        // STRING: VC1 0x004d6cd8
        .constant<int>(0xffb8860b, "DarkGoldenrod")
        // STRING: VC1 0x004d6ce8
        .constant<int>(0xffa9a9a9, "DarkGray")
        // STRING: VC1 0x004d6cf4
        .constant<int>(0xff006400, "DarkGreen")
        // STRING: VC1 0x004d6d00
        .constant<int>(0xffbdb76b, "DarkKhaki")
        // STRING: VC1 0x004d6d0c
        .constant<int>(0xff8b008b, "DarkMagenta")
        // STRING: VC1 0x004d6d18
        .constant<int>(0xff556b2f, "DarkOliveGreen")
        // STRING: VC1 0x004d6d28
        .constant<int>(0xffff8c00, "DarkOrange")
        // STRING: VC1 0x004d6d34
        .constant<int>(0xff9932cc, "DarkOrchid")
        // STRING: VC1 0x004d6d40
        .constant<int>(0xff8b0000, "DarkRed")
        // STRING: VC1 0x004d6d48
        .constant<int>(0xffe9967a, "DarkSalmon")
        // STRING: VC1 0x004d6d54
        .constant<int>(0xff8fbc8b, "DarkSeaGreen")
        // STRING: VC1 0x004d6d64
        .constant<int>(0xff483d8b, "DarkSlateBlue")
        // STRING: VC1 0x004d6d74
        .constant<int>(0xff2f4f4f, "DarkSlateGray")
        // STRING: VC1 0x004d6d84
        .constant<int>(0xff00ced1, "DarkTurquoise")
        // STRING: VC1 0x004d6d94
        .constant<int>(0xff9400d3, "DarkViolet")
        // STRING: VC1 0x004d6da0
        .constant<int>(0xffff1493, "DeepPink")
        // STRING: VC1 0x004d6dac
        .constant<int>(0xff00bfff, "DeepSkyBlue")
        // STRING: VC1 0x004d6db8
        .constant<int>(0xff696969, "DimGray")
        // STRING: VC1 0x004d6dc0
        .constant<int>(0xff1e90ff, "DodgerBlue")
        // STRING: VC1 0x004d6dcc
        .constant<int>(0xffb22222, "Firebrick")
        // STRING: VC1 0x004d6dd8
        .constant<int>(0xfffffaf0, "FloralWhite")
        // STRING: VC1 0x004d6de4
        .constant<int>(0xff228b22, "ForestGreen")
        // STRING: VC1 0x004d6df0
        .constant<int>(0xffff00ff, "Fuchsia")
        // STRING: VC1 0x004d6df8
        .constant<int>(0xffdcdcdc, "Gainsboro")
        // STRING: VC1 0x004d6e04
        .constant<int>(0xfff8f8ff, "GhostWhite")
        // STRING: VC1 0x004d6e10
        .constant<int>(0xffffd700, "Gold")
        .constant<int>(0xffdaa520, "Goldenrod")
        .constant<int>(0xff808080, "Gray")
        .constant<int>(0xff008000, "Green")
        // STRING: VC1 0x004d6e34
        .constant<int>(0xffadff2f, "GreenYellow")
        // STRING: VC1 0x004d6e40
        .constant<int>(0xfff0fff0, "Honeydew")
        // STRING: VC1 0x004d6e4c
        .constant<int>(0xffff69b4, "HotPink")
        // STRING: VC1 0x004d6e54
        .constant<int>(0xffcd5c5c, "IndianRed")
        // STRING: VC1 0x004d6e60
        .constant<int>(0xff4b0082, "Indigo")
        // STRING: VC1 0x004d6e68
        .constant<int>(0xfffffff0, "Ivory")
        .constant<int>(0xfff0e68c, "Khaki")
        // STRING: VC1 0x004d6e78
        .constant<int>(0xffe6e6fa, "Lavender")
        // STRING: VC1 0x004d6e84
        .constant<int>(0xfffff0f5, "LavenderBlush")
        // STRING: VC1 0x004d6e94
        .constant<int>(0xff7cfc00, "LawnGreen")
        // STRING: VC1 0x004d6ea0
        .constant<int>(0xfffffacd, "LemonChiffon")
        // STRING: VC1 0x004d6eb0
        .constant<int>(0xffadd8e6, "LightBlue")
        // STRING: VC1 0x004d6ebc
        .constant<int>(0xfff08080, "LightCoral")
        // STRING: VC1 0x004d6ec8
        .constant<int>(0xffe0ffff, "LightCyan")
        // STRING: VC1 0x004d6ed4
        .constant<int>(0xfffafad2, "LightGoldenrodYellow")
        // STRING: VC1 0x004d6eec
        .constant<int>(0xffd3d3d3, "LightGray")
        // STRING: VC1 0x004d6ef8
        .constant<int>(0xff90ee90, "LightGreen")
        // STRING: VC1 0x004d6f04
        .constant<int>(0xffffb6c1, "LightPink")
        // STRING: VC1 0x004d6f10
        .constant<int>(0xffffa07a, "LightSalmon")
        // STRING: VC1 0x004d6f1c
        .constant<int>(0xff20b2aa, "LightSeaGreen")
        // STRING: VC1 0x004d6f2c
        .constant<int>(0xff87cefa, "LightSkyBlue")
        // STRING: VC1 0x004d6f3c
        .constant<int>(0xff778899, "LightSlateGray")
        // STRING: VC1 0x004d6f4c
        .constant<int>(0xffb0c4de, "LightSteelBlue")
        // STRING: VC1 0x004d6f5c
        .constant<int>(0xffffffe0, "LightYellow")
        // STRING: VC1 0x004d6f68
        .constant<int>(0xff00ff00, "Lime")
        // STRING: VC1 0x004d6f70
        .constant<int>(0xff32cd32, "LimeGreen")
        // STRING: VC1 0x004d6f7c
        .constant<int>(0xfffaf0e6, "Linen")
        .constant<int>(0xffff00ff, "Magenta")
        // STRING: VC1 0x004d6f8c
        .constant<int>(0xff800000, "Maroon")
        // STRING: VC1 0x004d6f94
        .constant<int>(0xff66cdaa, "MediumAquamarine")
        // STRING: VC1 0x004d6fa8
        .constant<int>(0xff0000cd, "MediumBlue")
        // STRING: VC1 0x004d6fb4
        .constant<int>(0xffba55d3, "MediumOrchid")
        // STRING: VC1 0x004d6fc4
        .constant<int>(0xff9370db, "MediumPurple")
        // STRING: VC1 0x004d6fd4
        .constant<int>(0xff3cb371, "MediumSeaGreen")
        // STRING: VC1 0x004d6fe4
        .constant<int>(0xff7b68ee, "MediumSlateBlue")
        // STRING: VC1 0x004d6ff4
        .constant<int>(0xff00fa9a, "MediumSpringGreen")
        // STRING: VC1 0x004d7008
        .constant<int>(0xff48d1cc, "MediumTurquoise")
        // STRING: VC1 0x004d7018
        .constant<int>(0xffc71585, "MediumVioletRed")
        // STRING: VC1 0x004d7028
        .constant<int>(0xff191970, "MidnightBlue")
        // STRING: VC1 0x004d7038
        .constant<int>(0xfff5fffa, "MintCream")
        // STRING: VC1 0x004d7044
        .constant<int>(0xffffe4e1, "MistyRose")
        // STRING: VC1 0x004d7050
        .constant<int>(0xffffe4b5, "Moccasin")
        // STRING: VC1 0x004d705c
        .constant<int>(0xffffdead, "NavajoWhite")
        // STRING: VC1 0x004d7068
        .constant<int>(0xff000080, "Navy")
        // STRING: VC1 0x004d7070
        .constant<int>(0xfffdf5e6, "OldLace")
        // STRING: VC1 0x004d7078
        .constant<int>(0xff808000, "Olive")
        // STRING: VC1 0x004d7080
        .constant<int>(0xff6b8e23, "OliveDrab")
        .constant<int>(0xffffa500, "Orange")
        // STRING: VC1 0x004d7094
        .constant<int>(0xffff4500, "OrangeRed")
        .constant<int>(0xffda70d6, "Orchid")
        // STRING: VC1 0x004d70a8
        .constant<int>(0xffeee8aa, "PaleGoldenrod")
        // STRING: VC1 0x004d70b8
        .constant<int>(0xff98fb98, "PaleGreen")
        // STRING: VC1 0x004d70c4
        .constant<int>(0xffafeeee, "PaleTurquoise")
        // STRING: VC1 0x004d70d4
        .constant<int>(0xffdb7093, "PaleVioletRed")
        // STRING: VC1 0x004d70e4
        .constant<int>(0xffffefd5, "PapayaWhip")
        // STRING: VC1 0x004d70f0
        .constant<int>(0xffffdab9, "PeachPuff")
        // STRING: VC1 0x004d70fc
        .constant<int>(0xffcd853f, "Peru")
        .constant<int>(0xffffc0cb, "Pink")
        // STRING: VC1 0x004d710c
        .constant<int>(0xffdda0dd, "Plum")
        // STRING: VC1 0x004d7114
        .constant<int>(0xffb0e0e6, "PowderBlue")
        // STRING: VC1 0x004d7120
        .constant<int>(0xff800080, "Purple")
        .constant<int>(0xffff0000, "Red")
        // STRING: VC1 0x004d712c
        .constant<int>(0xffbc8f8f, "RosyBrown")
        // STRING: VC1 0x004d7138
        .constant<int>(0xff4169e1, "RoyalBlue")
        // STRING: VC1 0x004d7144
        .constant<int>(0xff8b4513, "SaddleBrown")
        .constant<int>(0xfffa8072, "Salmon")
        // STRING: VC1 0x004d7158
        .constant<int>(0xfff4a460, "SandyBrown")
        .constant<int>(0xff2e8b57, "SeaGreen")
        // STRING: VC1 0x004d7170
        .constant<int>(0xfffff5ee, "SeaShell")
        // STRING: VC1 0x004d717c
        .constant<int>(0xffa0522d, "Sienna")
        // STRING: VC1 0x004d7184
        .constant<int>(0xffc0c0c0, "Silver")
        .constant<int>(0xff87ceeb, "SkyBlue")
        .constant<int>(0xff6a5acd, "SlateBlue")
        .constant<int>(0xff708090, "SlateGray")
        // STRING: VC1 0x004d71ac
        .constant<int>(0xfffffafa, "Snow")
        // STRING: VC1 0x004d71b4
        .constant<int>(0xff00ff7f, "SpringGreen")
        // STRING: VC1 0x004d71c0
        .constant<int>(0xff4682b4, "SteelBlue")
        // STRING: VC1 0x004d71cc
        .constant<int>(0xffd2b48c, "Tan")
        // STRING: VC1 0x004d71d0
        .constant<int>(0xff008080, "Teal")
        // STRING: VC1 0x004d71d8
        .constant<int>(0xffd8bfd8, "Thistle")
        // STRING: VC1 0x004d71e0
        .constant<int>(0xffff6347, "Tomato")
        // STRING: VC1 0x004d71e8
        .constant<int>(0xffffff, "Transparent")
        .constant<int>(0xff40e0d0, "Turquoise")
        .constant<int>(0xffee82ee, "Violet")
        // STRING: VC1 0x004d7208
        .constant<int>(0xfff5deb3, "Wheat")
        // STRING: VC1 0x004d7210
        .constant<int>(0xffffffff, "White")
        // STRING: VC1 0x004d7218
        .constant<int>(0xfff5f5f5, "WhiteSmoke")
        // STRING: VC1 0x004d7224
        .constant<int>(0xffffff00, "Yellow")
        // STRING: VC1 0x004d722c
        .constant<int>(0xff9acd32, "YellowGreen");
    // STRING: VC1 0x004d7588
    SqPlus::SQClassDef<Vector3>("Vector3")
        .staticFuncVarArgs(&Vector3::ConstructArgs, "constructor")
        .var(&Vector3::x, "x")
        .var(&Vector3::y, "y")
        .var(&Vector3::z, "z")
        // STRING: VC1 0x004d36b0
        .func(&Vector3::Add, "_add")
        // STRING: VC1 0x004d36b8
        .func(&Vector3::Sub, "_sub")
        // STRING: VC1 0x004d36c0
        .func(&Vector3::Mul, "_mul")
        // STRING: VC1 0x004d36c8
        .func(&Vector3::Div, "_div")
        // STRING: VC1 0x004d36d0
        .func(&Vector3::Unm, "_unm")
        .func(&Vector3::Cmp, "_cmp")
        .func(&Vector3::ToString, "_tostring")
        // STRING: VC1 0x004d723c
        .func(&Vector3::Normalize, "Normalize")
        // STRING: VC1 0x004d7248
        .func(&Vector3::GetNormalized, "GetNormalized")
        // STRING: VC1 0x004d7258
        .func(&Vector3::GetMagnitude, "GetMagnitude")
        // STRING: VC1 0x004d7268
        .func(&Vector3::GetSquareMagnitude, "GetSquareMagnitude")
        // STRING: VC1 0x004d727c
        .staticFunc(&Vector3::GetDotProduct, "GetDotProduct")
        // STRING: VC1 0x004d728c
        .staticFunc(&Vector3::GetCrossProduct, "GetCrossProduct")
        // STRING: VC1 0x004d729c
        .staticFunc(&Vector3::Lerp, "Lerp")
        // STRING: VC1 0x004d72a4
        .constant(Vector3::AxisX, "AxisX")
        // STRING: VC1 0x004d72ac
        .constant(Vector3::AxisY, "AxisY")
        // STRING: VC1 0x004d72b4
        .constant(Vector3::AxisZ, "AxisZ")
        // STRING: VC1 0x004d72bc
        .constant(Vector3::Null, "Null");
    // STRING: VC1 0x004cfb44
    SqPlus::SQClassDef<Vector2>("Vector2")
        .staticFuncVarArgs(&Vector2::ConstructArgs, "constructor")
        .var(&Vector2::x, "x")
        .var(&Vector2::y, "y")
        .func(&Vector2::Add, "_add")
        .func(&Vector2::Sub, "_sub")
        .func(&Vector2::Mul, "_mul")
        .func(&Vector2::Div, "_div")
        .func(&Vector2::Unm, "_unm")
        .func(&Vector2::Cmp, "_cmp")
        .func(&Vector2::ToString, "_tostring")
        // STRING: VC1 0x004d72c4
        .func(&Vector2::Equal, "Equal")
        .func(&Vector2::Normalize, "Normalize")
        .func(&Vector2::GetNormalized, "GetNormalized")
        .func(&Vector2::GetMagnitude, "GetMagnitude")
        .func(&Vector2::GetSquareMagnitude, "GetSquareMagnitude")
        // STRING: VC1 0x004d72cc
        .func(&Vector2::IsValid, "IsValid")
        .staticFunc(&Vector2::GetDotProduct, "GetDotProduct")
        // STRING: VC1 0x004d72d4
        .staticFunc(&Vector2::AngleBetween, "AngleBetween")
        .staticFunc(&Vector2::Lerp, "Lerp")
        // STRING: VC1 0x004d72e4
        .staticFunc(&Vector2::GetOrthogonal, "GetOrthogonal")
        .constant(Vector2::AxisX, "AxisX")
        .constant(Vector2::AxisY, "AxisY")
        .constant(Vector2::Null, "Null");
    // STRING: VC1 0x004d7590
    SqPlus::SQClassDef<Vector4>("Vector4")
        .staticFuncVarArgs(&Vector4::ConstructArgs, "constructor")
        .var(&Vector4::x, "x")
        .var(&Vector4::y, "y")
        .var(&Vector4::z, "z")
        .var(&Vector4::w, "w")
        .func(&Vector4::Add, "_add")
        .func(&Vector4::Sub, "_sub")
        .func(&Vector4::Mul, "_mul")
        .func(&Vector4::Div, "_div")
        .func(&Vector4::Unm, "_unm")
        .func(&Vector4::Cmp, "_cmp")
        .func(&Vector4::ToString, "_tostring")
        .func(&Vector4::Normalize, "Normalize")
        .func(&Vector4::GetNormalized, "GetNormalized")
        .func(&Vector4::GetMagnitude, "GetMagnitude")
        .func(&Vector4::GetSquareMagnitude, "GetSquareMagnitude")
        .staticFunc(&Vector4::GetDotProduct, "GetDotProduct")
        .staticFunc(&Vector4::GetCrossProduct, "GetCrossProduct")
        .staticFunc(&Vector4::Lerp, "Lerp");
    // STRING: VC1 0x004d1920
    SqPlus::SQClassDef<Matrix3>("Matrix3")
        .staticFuncVarArgs(&Matrix3::ConstructArgs, "constructor")
        // STRING: VC1 0x004d72f8
        .var(&Matrix3::_11, "_11")
        // STRING: VC1 0x004d72fc
        .var(&Matrix3::_12, "_12")
        // STRING: VC1 0x004d7300
        .var(&Matrix3::_13, "_13")
        // STRING: VC1 0x004d7304
        .var(&Matrix3::_21, "_21")
        // STRING: VC1 0x004d7308
        .var(&Matrix3::_22, "_22")
        // STRING: VC1 0x004d730c
        .var(&Matrix3::_23, "_23")
        // STRING: VC1 0x004d7310
        .var(&Matrix3::_31, "_31")
        // STRING: VC1 0x004d7314
        .var(&Matrix3::_32, "_32")
        // STRING: VC1 0x004d7318
        .var(&Matrix3::_33, "_33")
        .func(&Matrix3::Add, "_add")
        .func(&Matrix3::Sub, "_sub")
        .func(&Matrix3::Mul, "_mul")
        .func(&Matrix3::Div, "_div")
        .func(&Matrix3::Unm, "_unm")
        .func(&Matrix3::Cmp, "_cmp")
        .func(&Matrix3::ToString, "_tostring")
        // STRING: VC1 0x004d731c
        .func(&Matrix3::GetXVec, "GetXVec")
        // STRING: VC1 0x004d7324
        .func(&Matrix3::GetYVec, "GetYVec")
        // STRING: VC1 0x004d732c
        .func(&Matrix3::GetOrigin, "GetOrigin")
        // STRING: VC1 0x004d7338
        .func(&Matrix3::SetXVec, "SetXVec")
        // STRING: VC1 0x004d7340
        .func(&Matrix3::SetYVec, "SetYVec")
        // STRING: VC1 0x004d7348
        .func(&Matrix3::SetOrigin, "SetOrigin")
        .func(&Matrix3::Normalize, "Normalize")
        .func(&Matrix3::GetNormalized, "GetNormalized")
        // STRING: VC1 0x004d7354
        .func(&Matrix3::Mult, "Mult")
        // STRING: VC1 0x004d735c
        .func(&Matrix3::MultInvVect, "MultInvVect")
        // STRING: VC1 0x004d7368
        .func(&Matrix3::MultInvVrt, "MultInvVrt")
        // STRING: VC1 0x004d7374
        .func(&Matrix3::MultVect, "MultVect")
        // STRING: VC1 0x004d7380
        .func(&Matrix3::MultVecs, "MultVecs")
        // STRING: VC1 0x004d738c
        .func(&Matrix3::MultVrt, "MultVrt")
        // STRING: VC1 0x004d7394
        .staticFunc(&Matrix3::Inverse, "Inverse")
        // STRING: VC1 0x004d739c
        .staticFunc(&Matrix3::Transpose, "Transpose")
        // STRING: VC1 0x004d73a8
        .constant(Matrix3::Identity, "Identity")
        .constant(Matrix3::Null, "Null");
    // STRING: VC1 0x004d7598
    SqPlus::SQClassDef<Matrix4>("Matrix4")
        .staticFuncVarArgs(&Matrix4::ConstructArgs, "constructor")
        .var(&Matrix4::_11, "_11")
        .var(&Matrix4::_12, "_12")
        .var(&Matrix4::_13, "_13")
        // STRING: VC1 0x004d73b4
        .var(&Matrix4::_14, "_14")
        .var(&Matrix4::_21, "_21")
        .var(&Matrix4::_22, "_22")
        .var(&Matrix4::_23, "_23")
        // STRING: VC1 0x004d73b8
        .var(&Matrix4::_24, "_24")
        .var(&Matrix4::_31, "_31")
        .var(&Matrix4::_32, "_32")
        .var(&Matrix4::_33, "_33")
        // STRING: VC1 0x004d73bc
        .var(&Matrix4::_34, "_34")
        // STRING: VC1 0x004d73c0
        .var(&Matrix4::_41, "_41")
        // STRING: VC1 0x004d73c4
        .var(&Matrix4::_42, "_42")
        // STRING: VC1 0x004d73c8
        .var(&Matrix4::_43, "_43")
        // STRING: VC1 0x004d73cc
        .var(&Matrix4::_44, "_44")
        .func(&Matrix4::Add, "_add")
        .func(&Matrix4::Sub, "_sub")
        .func(&Matrix4::Mul, "_mul")
        .func(&Matrix4::Div, "_div")
        .func(&Matrix4::Unm, "_unm")
        .func(&Matrix4::Cmp, "_cmp")
        .func(&Matrix4::ToString, "_tostring")
        .func(&Matrix4::GetXVec, "GetXVec")
        .func(&Matrix4::GetYVec, "GetYVec")
        // STRING: VC1 0x004d73d0
        .func(&Matrix4::GetZVec, "GetZVec")
        .func(&Matrix4::GetOrigin, "GetOrigin")
        .func(&Matrix4::SetXVec, "SetXVec")
        .func(&Matrix4::SetYVec, "SetYVec")
        // STRING: VC1 0x004d73d8
        .func(&Matrix4::SetZVec, "SetZVec")
        .func(&Matrix4::SetOrigin, "SetOrigin")
        .func(&Matrix4::Normalize, "Normalize")
        .func(&Matrix4::GetNormalized, "GetNormalized")
        .func(&Matrix4::Mult, "Mult")
        .func(&Matrix4::MultInvVect, "MultInvVect")
        .func(&Matrix4::MultInvVrt, "MultInvVrt")
        .func(&Matrix4::MultVect, "MultVect")
        .func(&Matrix4::MultVecs, "MultVecs")
        .func(&Matrix4::MultVrt, "MultVrt")
        .staticFunc(&Matrix4::Inverse, "Inverse")
        .staticFunc(&Matrix4::Transpose, "Transpose")
        .constant(Matrix4::Identity, "Identity")
        .constant(Matrix4::Null, "Null");
    // STRING: VC1 0x004d75a0
    SqPlus::SQClassDef<Quaternion>("Quaternion")
        .staticFuncVarArgs(&Quaternion::ConstructArgs, "constructor")
        .var(&Quaternion::v, "v")
        .var(&Quaternion::w, "w")
        .func(&Quaternion::Add, "_add")
        .func(&Quaternion::Sub, "_sub")
        .func(&Quaternion::Mul, "_mul")
        .func(&Quaternion::Div, "_div")
        .func(&Quaternion::Unm, "_unm")
        .func(&Quaternion::Cmp, "_cmp")
        .func(&Quaternion::ToString, "_tostring")
        .func(&Quaternion::Normalize, "Normalize")
        .func(&Quaternion::GetNormalized, "GetNormalized")
        .func(&Quaternion::GetMagnitude, "GetMagnitude")
        .func(&Quaternion::GetSquareMagnitude, "GetSquareMagnitude")
        // STRING: VC1 0x004d73e0
        .func(&Quaternion::FromMatrix, "FromMatrix")
        // STRING: VC1 0x004d73ec
        .func(&Quaternion::ToMatrix, "ToMatrix")
        // STRING: VC1 0x004d73f8
        .func(&Quaternion::Slerp, "Slerp")
        .staticFunc(&Quaternion::Inverse, "Inverse");
    SqPlus::SQClassDef<Rect>("Rect")
        .staticFuncVarArgs(&Rect::ConstructArgs, "constructor")
        .var(&Rect::x, "X")
        .var(&Rect::y, "Y")
        // STRING: VC1 0x004d7408
        .var(&Rect::width, "Width")
        // STRING: VC1 0x004d7410
        .var(&Rect::height, "Height")
        .func(&Rect::ToString, "_tostring")
        .func(&Rect::IsEmpty, "IsEmpty")
        // STRING: VC1 0x004d7418
        .func(&Rect::Union, "Union")
        // STRING: VC1 0x004d7420
        .func(&Rect::Intersect, "Intersect")
        // STRING: VC1 0x004d742c
        .func(&Rect::IsPointIn, "IsPointIn");
    // STRING: VC1 0x004d4ffc
    SqPlus::SQClassDef<RectF>("RectF")
        .staticFuncVarArgs(&RectF::ConstructArgs, "constructor")
        .var(&RectF::x, "X")
        .var(&RectF::y, "Y")
        .var(&RectF::width, "Width")
        .var(&RectF::height, "Height")
        .func(&RectF::ToString, "_tostring")
        .func(&RectF::IsEmpty, "IsEmpty")
        .func(&RectF::Union, "Union")
        .func(&RectF::Intersect, "Intersect");
    // STRING: VC1 0x004d75ac
    SqPlus::SQClassDef<IntegralF>("IntegralF")
        // STRING: VC1 0x004d7438
        .func(&IntegralF::SetIntegralSize, "SetIntegralSize")
        // STRING: VC1 0x004d7448
        .func(&IntegralF::PushValue, "PushValue")
        // STRING: VC1 0x004d7454
        .func(&IntegralF::GetAverageValue, "GetAverageValue")
        // STRING: VC1 0x004d7464
        .func(&IntegralF::GetIntegralValue, "GetIntegralValue")
        .func(&IntegralF::Clear, kScriptClear);
}

}
