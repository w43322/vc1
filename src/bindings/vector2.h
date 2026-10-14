#ifndef VC1_BINDINGS_VECTOR2_H
#define VC1_BINDINGS_VECTOR2_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Vector2" (0x004cfb44); SqPlus ClassType at
// 0x004ea830.
struct Vector2
{
    SQFloat x;
    SQFloat y;

    Vector2() : x(0.0f), y(0.0f) {}
    explicit Vector2(SQFloat s) : x(s), y(s) {}
    Vector2(SQFloat ax, SQFloat ay) : x(ax), y(ay) {}

    VC1_SIZE_CLASS_NEW(Vector2)

    // Class constants "AxisX", "AxisY", "Null".
    static Vector2 AxisX;
    static Vector2 AxisY;
    static Vector2 Null;

    SQInteger Cmp(const Vector2& other) const;  // "_cmp"
    KdString& ToString() const;                 // "_tostring"
    bool Equal(const Vector2& other) const;     // "Equal"
    bool IsValid() const;                       // "IsValid"
    Vector2 Unm() const;                        // "_unm"
    Vector2 Div(SQFloat scalar) const;          // "_div"
    Vector2 Mul(SQFloat scalar) const;          // "_mul"
    Vector2 Sub(const Vector2& other) const;    // "_sub"
    Vector2 Add(const Vector2& other) const;    // "_add"
    SQFloat GetSquareMagnitude() const;
    SQFloat GetMagnitude() const;
    Vector2 GetNormalized() const;
    void Normalize();
    static SQFloat GetDotProduct(const Vector2& a, const Vector2& b);
    static SQFloat AngleBetween(const Vector2& a, const Vector2& b);
    static Vector2 GetOrthogonal(const Vector2& v);
    static Vector2 Lerp(const Vector2& a, const Vector2& b, SQFloat t);

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1Vector2SizeCheck[sizeof(Vector2) == 8 ? 1 : -1];

// Inferred name. acos clamped to [-1, 1]: pi at or below -1, 0 at or above 1.
// Never inlined into its two callers in the original.
__declspec(noinline) SQFloat ClampedACos(SQFloat value);

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Vector2, Vector2)

// SqPlus instantiations for Vector2 (SQClassDef<Vector2> and friends). The
// member binders get the function and name folded in, except where two
// members share a signature.

// TEMPLATE: VC1 0x004015b0
// ??$Set@UVector2@vc1@@@SquirrelObject@@QAEAAV0@AAUVector2@vc1@@@Z

// TEMPLATE: VC1 0x0041dd00
// ?release@?$ConstructReleaseClass@UVector2@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041df30
// ??0?$SQClassDef@UVector2@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UVector2@vc1@@@SqPlus@@YAHPAUSQVM@@PAUVector2@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e620
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UVector2@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041e6a0
// ??$var@M@?$SQClassDefBase@UVector2@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQVector2@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041e700
// ??$constant@UVector2@vc1@@@?$SQClassDefBase@UVector2@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@UVector2@vc1@@PBD@Z

// TEMPLATE: VC1 0x0041f2b0
// ??$RegisterClassType@UVector2@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041fea0
// ?construct@?$ConstructReleaseClass@UVector2@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00420bc0
// ??$RegisterInstance@UVector2@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x00420c70
// ??$RegisterInstance@UVector2@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x00420d20
// ??$RegisterInstance@UVector2@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00420dd0
// ??$RegisterInstance@UVector2@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00420e80
// ??$RegisterInstance@UVector2@vc1@@P812@BE_NABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BE_NABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00420f30
// ??$RegisterInstance@UVector2@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00420fe0
// ??$RegisterInstance@UVector2@vc1@@P812@BE_NXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BE_NXZPBD@Z

// TEMPLATE: VC1 0x00421090
// ??$Register@P6AMABUVector2@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6AMABUVector2@vc1@@2@ZPBD@Z

// TEMPLATE: VC1 0x00421140
// ??$Register@P6A?AUVector2@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector2@vc1@@ABU34@2M@ZPBD@Z

// TEMPLATE: VC1 0x004211b0
// ??$Register@P6A?AUVector2@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector2@vc1@@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00421dc0 FOLDED
// ??$RegisterInstance@UVector2@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UVector2@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector2@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x00423680
// ??$sq_typeof@UVector2@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423af0
// ??$Register@P6A?AUVector2@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector2@vc1@@ABU23@1M@ZPBD@Z

// TEMPLATE: VC1 0x00423b50
// ??$Register@P6A?AUVector2@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector2@vc1@@ABU23@@ZPBD@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004240b0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424100
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424150
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004241a0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BE_NABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424580 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424a50
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BE_NXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector2@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UVector2@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00425090
// ?Dispatch@?$DirectCallFunction@P6AMABUVector2@vc1@@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00425120
// ?Dispatch@?$DirectCallFunction@P6A?AUVector2@vc1@@ABU12@0M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00425190
// ?Dispatch@?$DirectCallFunction@P6A?AUVector2@vc1@@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

#endif
