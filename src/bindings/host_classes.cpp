#include "host/runtime.h"
#include "bindings/sqplus.h"
#include "bindings/value_types.h"
#include "bindings/vector2.h"
#include "bindings/vector3.h"

namespace vc1 {

// Called from InitScriptEngine. Seven root-table natives, then one temporary
// SqPlus SQClassDef per value type: Color (with its named colour constants),
// Vector3, Vector2, Vector4, Matrix3, Matrix4, Quaternion, Rect, RectF and a
// last class built by 0x0041e320. LTCG pushes every by-value argument of a
// chain before its first call.
// Partial: only the Vector3 (0x004c04b2-0x004c06fa) and Vector2
// (0x004c06ff-0x004c093f) chains are written so far.
// STUB: VC1 0x004bf610
void BindHostClasses()
{
    SqPlus::SQClassDef<Vector3>("Vector3")
        .staticFuncVarArgs(&Vector3::ConstructArgs, "constructor")
        .var(&Vector3::x, "x")
        .var(&Vector3::y, "y")
        .var(&Vector3::z, "z")
        .func(&Vector3::Add, "_add")
        .func(&Vector3::Sub, "_sub")
        .func(&Vector3::Mul, "_mul")
        .func(&Vector3::Div, "_div")
        .func(&Vector3::Unm, "_unm")
        .func(&Vector3::Cmp, "_cmp")
        .func(&Vector3::ToString, "_tostring")
        .func(&Vector3::Normalize, "Normalize")
        .func(&Vector3::GetNormalized, "GetNormalized")
        .func(&Vector3::GetMagnitude, "GetMagnitude")
        .func(&Vector3::GetSquareMagnitude, "GetSquareMagnitude")
        .staticFunc(&Vector3::GetDotProduct, "GetDotProduct")
        .staticFunc(&Vector3::GetCrossProduct, "GetCrossProduct")
        .staticFunc(&Vector3::Lerp, "Lerp")
        .constant(Vector3::AxisX, "AxisX")
        .constant(Vector3::AxisY, "AxisY")
        .constant(Vector3::AxisZ, "AxisZ")
        .constant(Vector3::Null, "Null");
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
        .func(&Vector2::Equal, "Equal")
        .func(&Vector2::Normalize, "Normalize")
        .func(&Vector2::GetNormalized, "GetNormalized")
        .func(&Vector2::GetMagnitude, "GetMagnitude")
        .func(&Vector2::GetSquareMagnitude, "GetSquareMagnitude")
        .func(&Vector2::IsValid, "IsValid")
        .staticFunc(&Vector2::GetDotProduct, "GetDotProduct")
        .staticFunc(&Vector2::AngleBetween, "AngleBetween")
        .staticFunc(&Vector2::Lerp, "Lerp")
        .staticFunc(&Vector2::GetOrthogonal, "GetOrthogonal")
        .constant(Vector2::AxisX, "AxisX")
        .constant(Vector2::AxisY, "AxisY")
        .constant(Vector2::Null, "Null");
}

}
