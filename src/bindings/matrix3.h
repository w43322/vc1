#ifndef VC1_BINDINGS_MATRIX3_H
#define VC1_BINDINGS_MATRIX3_H
#include "squirrel_object.h"
#include "vector2.h"
#include "vector3.h"
#include "memory/fixed_pool.h"
#include "kdcore/kd_memory.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Matrix3" (0x004d1920); SqPlus ClassType at
// 0x004ed824. Pool pools[9] / 0x004ed47c.
// 2D affine, script fields _11.._33: X=(_11,_12), Y=(_21,_22),
// origin=(_31,_32). m[] aliases the same nine floats.
struct Matrix3
{
    union {
        struct {
            SQFloat _11, _12, _13;
            SQFloat _21, _22, _23;
            SQFloat _31, _32, _33;
        };
        SQFloat m[9];
    };

    // Identity, written column by column like the vector constructors.
    Matrix3()
    {
        _11 = 1.0f;
        _21 = 0.0f;
        _31 = 0.0f;
        _12 = 0.0f;
        _22 = 1.0f;
        _32 = 0.0f;
        _13 = 0.0f;
        _23 = 0.0f;
        _33 = 1.0f;
    }
    Matrix3(SQFloat a11, SQFloat a12, SQFloat a13, SQFloat a21, SQFloat a22, SQFloat a23,
        SQFloat a31, SQFloat a32, SQFloat a33)
        : _11(a11), _12(a12), _13(a13), _21(a21), _22(a22), _23(a23),
          _31(a31), _32(a32), _33(a33)
    {
    }
    // Copies go through KdMemcpy (a CRT memcpy call in the original).
    Matrix3(const Matrix3& other)
    {
        KdMemcpy(this, &other, sizeof(Matrix3));
    }
    Matrix3(const Vector2& x, const Vector2& y, const Vector2& origin);
    explicit Matrix3(const Vector2& origin);
    Matrix3(const Vector2& x, const Vector2& origin);

    VC1_SIZE_CLASS_NEW(Matrix3)

    // Class constants "Identity", "Null".
    static Matrix3 Identity;
    static Matrix3 Null;

    Vector2& GetOrigin();                          // "GetOrigin"
    void SetOrigin(const Vector2& origin);         // "SetOrigin"
    Vector2& GetYVec();                            // "GetYVec"
    Vector2& GetXVec();                            // "GetXVec"
    void SetYVec(const Vector2& axis);             // "SetYVec"
    void SetXVec(const Vector2& axis);             // "SetXVec"
    void MultVecs(const Vector2& scale);           // "MultVecs"
    Vector2 MultInvVrt(const Vector2& v) const;    // "MultInvVrt"
    Matrix3 GetNormalized() const;                 // "GetNormalized"
    void Normalize();                              // "Normalize"
    KdString& ToString() const;                    // "_tostring"
    SQInteger Cmp(const Matrix3& other) const;     // "_cmp"
    Matrix3 Unm() const;                           // "_unm"
    Matrix3 Div(SQFloat scalar) const;             // "_div"
    Matrix3 Mul(const Matrix3& other) const;       // "_mul"
    Matrix3 Sub(const Matrix3& other) const;       // "_sub"
    Matrix3 Add(const Matrix3& other) const;       // "_add"
    Matrix3* LeftMultiply(const Matrix3* left);    // this = left * this, in place
    __declspec(noinline) Matrix3 operator/(SQFloat scalar) const;
    Matrix3 operator*(const Matrix3& other) const;
    Vector2 MultVrt(const Vector2& v) const;       // "MultVrt"
    Vector2 MultInvVect(const Vector2& v) const;   // "MultInvVect"
    Vector2 MultVect(const Vector2& v) const;      // "MultVect"
    Vector3 Mult(const Vector3& v) const;          // "Mult"
    static Matrix3 Transpose(const Matrix3& src);  // "Transpose"
    static Matrix3 Inverse(const Matrix3& src);    // "Inverse"

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1Matrix3SizeCheck[sizeof(Matrix3) == 36 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Matrix3, Matrix3)

// SqPlus instantiations for Matrix3. The member binders and dispatchers that
// share a signature with a Vector2/Vector3 one are folded with it.

// TEMPLATE: VC1 0x0041dd20
// ?release@?$ConstructReleaseClass@UMatrix3@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041e050
// ??0?$SQClassDef@UMatrix3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UMatrix3@vc1@@@SqPlus@@YAHPAUSQVM@@PAUMatrix3@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e860
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UMatrix3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041e8e0
// ??$var@M@?$SQClassDefBase@UMatrix3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQMatrix3@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041e940
// ??$constant@UMatrix3@vc1@@@?$SQClassDefBase@UMatrix3@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@UMatrix3@vc1@@PBD@Z

// TEMPLATE: VC1 0x0041f570
// ??$RegisterClassType@UMatrix3@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041ff30
// ?construct@?$ConstructReleaseClass@UMatrix3@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00421790
// ??$RegisterInstance@UMatrix3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00421840
// ??$RegisterInstance@UMatrix3@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x004218f0
// ??$RegisterInstance@UMatrix3@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x004219a0
// ??$RegisterInstance@UMatrix3@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00421a50
// ??$RegisterInstance@UMatrix3@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00421b00
// ??$RegisterInstance@UMatrix3@vc1@@P812@AEAAUVector2@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@AEAAUVector2@4@XZPBD@Z

// TEMPLATE: VC1 0x00421bb0
// ??$RegisterInstance@UMatrix3@vc1@@P812@AEXABUVector2@2@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@AEXABUVector2@4@@ZPBD@Z

// TEMPLATE: VC1 0x00421c60
// ??$RegisterInstance@UMatrix3@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x00421d10
// ??$RegisterInstance@UMatrix3@vc1@@P812@BE?AUVector3@2@ABU32@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BE?AUVector3@4@ABU54@@ZPBD@Z

// TEMPLATE: VC1 0x00421dc0 FOLDED
// ??$RegisterInstance@UMatrix3@vc1@@P812@BE?AUVector2@2@ABU32@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix3@vc1@@P834@BE?AUVector2@4@ABU54@@ZPBD@Z

// TEMPLATE: VC1 0x00421e70
// ??$Register@P6A?AUMatrix3@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUMatrix3@vc1@@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x004236c0
// ??$sq_typeof@UMatrix3@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423cd0
// ??$Set@UMatrix3@vc1@@@SquirrelObject@@QAEAAV0@AAUMatrix3@vc1@@@Z

// TEMPLATE: VC1 0x00423f20 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BE?AUVector3@2@ABU32@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424330
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424380
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004243d0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424420
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004244d0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@AEAAUVector2@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424520
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@AEXABUVector2@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424580 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix3@vc1@@P812@BE?AUVector2@2@ABU32@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UMatrix3@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00425370
// ?Dispatch@?$DirectCallFunction@P6A?AUMatrix3@vc1@@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00429080
// ??$GetInstance@UVector2@vc1@@$0A@@SqPlus@@YAPAUVector2@vc1@@PAUSQVM@@H@Z

#endif
