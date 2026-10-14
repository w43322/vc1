#ifndef VC1_BINDINGS_QUATERNION_H
#define VC1_BINDINGS_QUATERNION_H
#include "squirrel_object.h"
#include "vector3.h"
#include "matrix4.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Quaternion" (0x004d75a0); SqPlus ClassType at
// 0x004ee1ec. Script fields "v" (Vector3, 0x004d3b58) and "w" (0x004d72f4).
// Pool pools[4]; the release hook is folded with Vector4's (0x0041dd60).
struct Quaternion
{
    Vector3 v;
    SQFloat w;

    Quaternion();
    // v is default-constructed (zeroed) and then assigned in every constructor.
    Quaternion(const Vector3& av, SQFloat aw) : w(aw) { v = av; }
    Quaternion(SQFloat ax, SQFloat ay, SQFloat az, SQFloat aw);
    // Axis/angle; script order Quaternion(angle, axis) (0x004be190).
    Quaternion(SQFloat angle, const Vector3& axis);
    explicit Quaternion(const Matrix4& matrix) { FromMatrix(matrix); }
    Quaternion(const Quaternion& a, const Quaternion& b, SQFloat t) { Slerp(a, b, t); }

    VC1_SIZE_CLASS_NEW(Quaternion)

    static Quaternion Inverse(const Quaternion& src);   // "Inverse"
    Quaternion GetNormalized() const;                   // "GetNormalized"
    SQFloat GetSquareMagnitude() const;                 // "GetSquareMagnitude"
    SQFloat GetMagnitude() const;                       // "GetMagnitude"
    void Normalize();                                   // "Normalize"
    KdString& ToString() const;                         // "_tostring"
    Quaternion Unm() const;                             // "_unm" conjugates v, keeps w
    __declspec(noinline) Quaternion operator/(SQFloat scalar) const;
    Quaternion Div(SQFloat scalar) const;               // "_div"
    Quaternion Mul(SQFloat scalar) const;               // "_mul" (scalar)
    Quaternion Sub(const Quaternion& other) const;      // "_sub"
    Quaternion Add(const Quaternion& other) const;      // "_add"
    SQInteger Cmp(const Quaternion& other) const;       // "_cmp"
    void FromMatrix(const Matrix4& matrix);             // "FromMatrix"
    Matrix4 ToMatrix() const;                           // "ToMatrix"
    void Slerp(const Quaternion& a, const Quaternion& b, SQFloat t); // "Slerp"

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1QuaternionSizeCheck[sizeof(Quaternion) == 16 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Quaternion, Quaternion)

// SqPlus instantiations for Quaternion. Binders and dispatchers that share a
// signature with a Vector3/Vector4/Matrix4 one are folded with it.

// TEMPLATE: VC1 0x0041dd60 FOLDED
// ?release@?$ConstructReleaseClass@UQuaternion@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041e170
// ??0?$SQClassDef@UQuaternion@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UQuaternion@vc1@@@SqPlus@@YAHPAUSQVM@@PAUQuaternion@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041eb20
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UQuaternion@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041eba0
// ??$var@UVector3@vc1@@@?$SQClassDefBase@UQuaternion@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQQuaternion@vc1@@UVector3@3@PBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041ec00
// ??$var@M@?$SQClassDefBase@UQuaternion@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQQuaternion@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041f830
// ??$RegisterClassType@UQuaternion@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x00420000
// ?construct@?$ConstructReleaseClass@UQuaternion@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004226b0
// ??$RegisterInstanceVariable@UVector3@vc1@@@SqPlus@@YAXAAVSquirrelObject@@PAUClassTypeBase@0@PAUVector3@vc1@@PBDW4VarAccessType@0@@Z

// TEMPLATE: VC1 0x00422750
// ??$RegisterInstance@UQuaternion@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00422800
// ??$RegisterInstance@UQuaternion@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x004228b0
// ??$RegisterInstance@UQuaternion@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x00422960
// ??$RegisterInstance@UQuaternion@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00422a10
// ??$RegisterInstance@UQuaternion@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00422ac0
// ??$RegisterInstance@UQuaternion@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UQuaternion@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x00422c20
// ??$RegisterInstance@UQuaternion@vc1@@P812@AEXABUMatrix4@2@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@AEXABUMatrix4@4@@ZPBD@Z

// TEMPLATE: VC1 0x00422cd0
// ??$RegisterInstance@UQuaternion@vc1@@P812@BE?AUMatrix4@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@BE?AUMatrix4@4@XZPBD@Z

// TEMPLATE: VC1 0x00422d80
// ??$RegisterInstance@UQuaternion@vc1@@P812@AEXABU12@0M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUQuaternion@vc1@@P834@AEXABU34@3M@ZPBD@Z

// TEMPLATE: VC1 0x00422e30
// ??$Register@P6A?AUQuaternion@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUQuaternion@vc1@@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00423700
// ??$sq_typeof@UQuaternion@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423dd0
// ??$Register@P6A?AUQuaternion@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@P6A?AUQuaternion@vc1@@ABU23@@ZPBD@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424770
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004247c0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424810
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424860
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004248b0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@AEXABUMatrix4@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424910 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BE?AUMatrix4@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424960
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@AEXABU12@0M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UQuaternion@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UQuaternion@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00425450
// ?Dispatch@?$DirectCallFunction@P6A?AUQuaternion@vc1@@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00428e30
// ??$GetInstance@UQuaternion@vc1@@$0A@@SqPlus@@YAPAUQuaternion@vc1@@PAUSQVM@@H@Z

// TEMPLATE: VC1 0x00428f50
// ??$GetInstance@UMatrix4@vc1@@$0A@@SqPlus@@YAPAUMatrix4@vc1@@PAUSQVM@@H@Z

#endif
