#include "host/runtime.h"
#include "bindings/sqplus.h"
#include "bindings/value_types.h"
#include "bindings/vector2.h"
#include "bindings/vector3.h"
#include "bindings/vector4.h"
#include "bindings/matrix3.h"

namespace vc1 {

// Called from InitScriptEngine. Seven root-table natives, then one temporary
// SqPlus SQClassDef per value type: Color (with its named colour constants),
// Vector3, Vector2, Vector4, Matrix3, Matrix4, Quaternion, Rect, RectF and a
// last class built by 0x0041e320. LTCG pushes every by-value argument of a
// chain before its first call.
// Partial: only the Vector3 (0x004c04b2-0x004c06fa), Vector2
// (0x004c06ff-0x004c093f), Vector4 (0x004c093f-0x004c0af0) and Matrix3
// (0x004c0af0-0x004c0e06) chains are
// written so far.
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
    SqPlus::SQClassDef<Matrix3>("Matrix3")
        .staticFuncVarArgs(&Matrix3::ConstructArgs, "constructor")
        .var(&Matrix3::_11, "_11")
        .var(&Matrix3::_12, "_12")
        .var(&Matrix3::_13, "_13")
        .var(&Matrix3::_21, "_21")
        .var(&Matrix3::_22, "_22")
        .var(&Matrix3::_23, "_23")
        .var(&Matrix3::_31, "_31")
        .var(&Matrix3::_32, "_32")
        .var(&Matrix3::_33, "_33")
        .func(&Matrix3::Add, "_add")
        .func(&Matrix3::Sub, "_sub")
        .func(&Matrix3::Mul, "_mul")
        .func(&Matrix3::Div, "_div")
        .func(&Matrix3::Unm, "_unm")
        .func(&Matrix3::Cmp, "_cmp")
        .func(&Matrix3::ToString, "_tostring")
        .func(&Matrix3::GetXVec, "GetXVec")
        .func(&Matrix3::GetYVec, "GetYVec")
        .func(&Matrix3::GetOrigin, "GetOrigin")
        .func(&Matrix3::SetXVec, "SetXVec")
        .func(&Matrix3::SetYVec, "SetYVec")
        .func(&Matrix3::SetOrigin, "SetOrigin")
        .func(&Matrix3::Normalize, "Normalize")
        .func(&Matrix3::GetNormalized, "GetNormalized")
        .func(&Matrix3::Mult, "Mult")
        .func(&Matrix3::MultInvVect, "MultInvVect")
        .func(&Matrix3::MultInvVrt, "MultInvVrt")
        .func(&Matrix3::MultVect, "MultVect")
        .func(&Matrix3::MultVecs, "MultVecs")
        .func(&Matrix3::MultVrt, "MultVrt")
        .staticFunc(&Matrix3::Inverse, "Inverse")
        .staticFunc(&Matrix3::Transpose, "Transpose")
        .constant(Matrix3::Identity, "Identity")
        .constant(Matrix3::Null, "Null");}

}
