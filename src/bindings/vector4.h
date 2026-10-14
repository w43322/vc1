#ifndef VC1_BINDINGS_VECTOR4_H
#define VC1_BINDINGS_VECTOR4_H
#include "squirrel_object.h"
#include "vector3.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Vector4" (0x004d7590); SqPlus ClassType at
// 0x004ee200.
struct Vector4
{
    SQFloat x;
    SQFloat y;
    SQFloat z;
    SQFloat w;

    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    explicit Vector4(SQFloat s) : x(s), y(s), z(s), w(s) {}
    Vector4(SQFloat ax, SQFloat ay) : x(ax), y(ay), z(0.0f), w(0.0f) {}
    Vector4(SQFloat ax, SQFloat ay, SQFloat az) : x(ax), y(ay), z(az), w(0.0f) {}
    Vector4(SQFloat ax, SQFloat ay, SQFloat az, SQFloat aw) : x(ax), y(ay), z(az), w(aw) {}

    VC1_SIZE_CLASS_NEW(Vector4)

    KdString& ToString() const;                 // "_tostring"
    SQInteger Cmp(const Vector4& other) const;  // "_cmp"
    Vector4 Unm() const;                        // "_unm"
    Vector4 Div(SQFloat scalar) const;          // "_div"
    Vector4 Mul(SQFloat scalar) const;          // "_mul"
    Vector4 Sub(const Vector4& other) const;    // "_sub"
    Vector4 Add(const Vector4& other) const;    // "_add"
    SQFloat GetSquareMagnitude() const;
    SQFloat GetMagnitude() const;
    Vector4 GetNormalized() const;
    void Normalize();
    static Vector4 Lerp(const Vector4& a, const Vector4& b, SQFloat t);
    static Vector3 GetCrossProduct(const Vector4& a, const Vector4& b);
    static SQFloat GetDotProduct(const Vector4& a, const Vector4& b);

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1Vector4SizeCheck[sizeof(Vector4) == 16 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Vector4, Vector4)

// SqPlus instantiations for Vector4. The release hook is the same 16-byte pool
// release for Quaternion, Rect and RectF.

// TEMPLATE: VC1 0x0041dd60 FOLDED
// ?release@?$ConstructReleaseClass@UVector4@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041dfc0
// ??0?$SQClassDef@UVector4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UVector4@vc1@@@SqPlus@@YAHPAUSQVM@@PAUVector4@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e780
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UVector4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041e800
// ??$var@M@?$SQClassDefBase@UVector4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQVector4@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041f410
// ??$RegisterClassType@UVector4@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041fee0
// ?construct@?$ConstructReleaseClass@UVector4@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00421220
// ??$RegisterInstance@UVector4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x004212d0
// ??$RegisterInstance@UVector4@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x00421380
// ??$RegisterInstance@UVector4@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x00421430
// ??$RegisterInstance@UVector4@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x004214e0
// ??$RegisterInstance@UVector4@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00421590
// ??$RegisterInstance@UVector4@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00421640
// ??$Register@P6AMABUVector4@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6AMABUVector4@vc1@@2@ZPBD@Z

// TEMPLATE: VC1 0x004216b0
// ??$Register@P6A?AUVector3@vc1@@ABUVector4@2@0@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector3@vc1@@ABUVector4@4@2@ZPBD@Z

// TEMPLATE: VC1 0x00421720
// ??$Register@P6A?AUVector4@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUVector4@vc1@@ABU34@2M@ZPBD@Z

// TEMPLATE: VC1 0x00422b70 FOLDED
// ??$RegisterInstance@UVector4@vc1@@P812@BEMXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUVector4@vc1@@P834@BEMXZPBD@Z

// TEMPLATE: VC1 0x004236a0
// ??$sq_typeof@UVector4@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423bb0
// ??$Register@P6AMABUVector4@vc1@@0@Z@SqPlus@@YAXPAUSQVM@@P6AMABUVector4@vc1@@1@ZPBD@Z

// TEMPLATE: VC1 0x00423c10
// ??$Register@P6A?AUVector3@vc1@@ABUVector4@2@0@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector3@vc1@@ABUVector4@3@1@ZPBD@Z

// TEMPLATE: VC1 0x00423c70
// ??$Register@P6A?AUVector4@vc1@@ABU12@0M@Z@SqPlus@@YAXPAUSQVM@@P6A?AUVector4@vc1@@ABU23@1M@ZPBD@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004241f0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424240
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424290
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004242e0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424b90 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UVector4@vc1@@P812@BEMXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UVector4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00425200
// ?Dispatch@?$DirectCallFunction@P6AMABUVector4@vc1@@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00425290
// ?Dispatch@?$DirectCallFunction@P6A?AUVector3@vc1@@ABUVector4@2@0@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00425300
// ?Dispatch@?$DirectCallFunction@P6A?AUVector4@vc1@@ABU12@0M@Z@SqPlus@@SAHPAUSQVM@@@Z

#endif
