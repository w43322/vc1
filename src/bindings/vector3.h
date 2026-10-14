#ifndef VC1_BINDINGS_VECTOR3_H
#define VC1_BINDINGS_VECTOR3_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Vector3" (0x004d7588); SqPlus ClassType at
// 0x004ee1d8 (guard 0x004ee3e0, vftable 0x004d75b8).
struct Vector3
{
    SQFloat x;
    SQFloat y;
    SQFloat z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    explicit Vector3(SQFloat s) : x(s), y(s), z(s) {}
    Vector3(SQFloat ax, SQFloat ay) : x(ax), y(ay), z(0.0f) {}
    Vector3(SQFloat ax, SQFloat ay, SQFloat az) : x(ax), y(ay), z(az) {}

    VC1_SIZE_CLASS_NEW(Vector3)

    // Class constants "AxisX", "AxisY", "AxisZ", "Null".
    static Vector3 AxisX;
    static Vector3 AxisY;
    static Vector3 AxisZ;
    static Vector3 Null;

    KdString& ToString() const;                 // "_tostring"
    SQInteger Cmp(const Vector3& other) const;  // "_cmp"
    Vector3 Unm() const;                        // "_unm"
    Vector3 Div(SQFloat scalar) const;          // "_div"
    Vector3 Mul(SQFloat scalar) const;          // "_mul"
    Vector3 Sub(const Vector3& other) const;    // "_sub"
    Vector3 Add(const Vector3& other) const;    // "_add"
    SQFloat GetSquareMagnitude() const;
    SQFloat GetMagnitude() const;
    Vector3 GetNormalized() const;
    void Normalize();
    static Vector3 GetCrossProduct(const Vector3& a, const Vector3& b);
    static SQFloat GetDotProduct(const Vector3& a, const Vector3& b);
    static Vector3 Lerp(const Vector3& a, const Vector3& b, SQFloat t);

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1Vector3SizeCheck[sizeof(Vector3) == 12 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Vector3, Vector3)

// SqPlus instantiations for Vector3 (SQClassDef<Vector3> and friends).

// TEMPLATE: VC1 0x0041dea0
// ??0?$SQClassDef@UVector3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UVector3@vc1@@@SqPlus@@YAHPAUSQVM@@PAUVector3@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e4c0
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UVector3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041e540
// ??$var@M@?$SQClassDefBase@UVector3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQVector3@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041e5a0
// ??$constant@UVector3@vc1@@@?$SQClassDefBase@UVector3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@UVector3@vc1@@PBD@Z

// TEMPLATE: VC1 0x0041f150
// ??$RegisterClassType@UVector3@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041fe50
// ?construct@?$ConstructReleaseClass@UVector3@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00420210
// ?release@?$ConstructReleaseClass@UVector3@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x00423660
// ??$sq_typeof@UVector3@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// SQClassDefBase::func inlines into BindHostClasses; RegisterInstance stays
// out of line (v=ECX, name=EAX; hclass, func on the stack). The ones other
// classes reuse are FOLDED.

// TEMPLATE: VC1 0x00420700
// ??$RegisterInstance@UVector3@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x004207b0
// ??$RegisterInstance@UVector3@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x00420860
// ??$RegisterInstance@UVector3@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00420910
// ??$RegisterInstance@UVector3@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x004209c0
// ??$RegisterInstance@UVector3@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00420a70
// ??$Register@P6AMABUVector3@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6AMABUVector3@vc1@@2@ZPBD@Z

// TEMPLATE: VC1 0x00420ae0
// ??$Register@P6A?AUVector3@vc1@@ABU12@0@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector3@vc1@@ABU34@2@ZPBD@Z

// TEMPLATE: VC1 0x00420b50
// ??$Register@P6A?AUVector3@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector3@vc1@@ABU34@2M@ZPBD@Z

// TEMPLATE: VC1 0x00422550 FOLDED
// ??$RegisterInstance@UVector3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UVector3@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector3@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x00423930
// ??$Register@P6AMABUVector3@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@P6AMABUVector3@vc1@@1@ZPBD@Z

// TEMPLATE: VC1 0x00423990
// ??$Register@P6A?AUVector3@vc1@@ABU12@0@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector3@vc1@@ABU23@1@ZPBD@Z

// TEMPLATE: VC1 0x004239f0
// ??$Register@P6A?AUVector3@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector3@vc1@@ABU23@1M@ZPBD@Z

// TEMPLATE: VC1 0x00423a50
// ??$Set@UVector3@vc1@@@SquirrelObject@@QAEAAV0@AAUVector3@vc1@@@Z

// TEMPLATE: VC1 0x00423f20 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423f70
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423fc0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424010
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector3@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z

// The picker constructor is the same code for every Callee/Func (EAX=v,
// EBX=this).
// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UVector3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00424f20
// ?Dispatch@?$DirectCallFunction@P6AMABUVector3@vc1@@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424fb0
// ?Dispatch@?$DirectCallFunction@P6A?AUVector3@vc1@@ABU12@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00425020
// ?Dispatch@?$DirectCallFunction@P6A?AUVector3@vc1@@ABU12@0M@Z@SqPlus@@SAHPAUSQVM@@@Z

#endif
