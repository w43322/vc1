#ifndef VC1_BINDINGS_MATRIX4_H
#define VC1_BINDINGS_MATRIX4_H
#include "squirrel_object.h"
#include "vector3.h"
#include "vector4.h"
#include "memory/fixed_pool.h"

namespace vc1 {

class KdString;

// Inferred name. Script class "Matrix4" (0x004d7598); SqPlus ClassType at
// 0x004ed85c. Pool pools[16] / 0x004ed498.
// 3D affine, script fields _11.._44: X=(_11,_12,_13), Y=(_21,_22,_23),
// Z=(_31,_32,_33), origin=(_41,_42,_43). m[] aliases the same sixteen floats.
struct Matrix4
{
    union {
        struct {
            SQFloat _11, _12, _13, _14;
            SQFloat _21, _22, _23, _24;
            SQFloat _31, _32, _33, _34;
            SQFloat _41, _42, _43, _44;
        };
        SQFloat m[16];
    };

    Matrix4();
    Matrix4(SQFloat a11, SQFloat a12, SQFloat a13, SQFloat a14,
        SQFloat a21, SQFloat a22, SQFloat a23, SQFloat a24,
        SQFloat a31, SQFloat a32, SQFloat a33, SQFloat a34,
        SQFloat a41, SQFloat a42, SQFloat a43, SQFloat a44)
        : _11(a11), _12(a12), _13(a13), _14(a14), _21(a21), _22(a22), _23(a23), _24(a24),
          _31(a31), _32(a32), _33(a33), _34(a34), _41(a41), _42(a42), _43(a43), _44(a44)
    {
    }
    Matrix4(const Vector3& x, const Vector3& origin);
    Matrix4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& origin);
    explicit Matrix4(const Vector3& origin);

    VC1_SIZE_CLASS_NEW(Matrix4)

    // Class constants "Identity", "Null".
    static Matrix4 Identity;
    static Matrix4 Null;

    Vector3& GetXVec();                            // "GetXVec"
    Vector3& GetOrigin();                          // "GetOrigin"
    Vector3& GetYVec();                            // "GetYVec"
    Vector3& GetZVec();                            // "GetZVec"
    void SetZVec(const Vector3& axis);             // "SetZVec"
    void SetYVec(const Vector3& axis);             // "SetYVec"
    void SetXVec(const Vector3& axis);             // "SetXVec"
    void MultVecs(const Vector3& scale);           // "MultVecs"
    Vector3 MultInvVrt(const Vector3& v) const;    // "MultInvVrt"
    Matrix4 GetNormalized() const;                 // "GetNormalized"
    void Normalize();                              // "Normalize"
    void SetOrigin(const Vector3& origin);         // "SetOrigin"
    KdString& ToString() const;                    // "_tostring"
    SQInteger Cmp(const Matrix4& other) const;     // "_cmp"
    __declspec(noinline) Matrix4 operator-() const;
    Matrix4 Unm() const;                           // "_unm"
    Matrix4 Div(SQFloat scalar) const;             // "_div"
    Matrix4 Mul(const Matrix4& other) const;       // "_mul"
    Matrix4 Sub(const Matrix4& other) const;       // "_sub"
    Matrix4 Add(const Matrix4& other) const;       // "_add"
    Vector4 Mult(const Vector4& v) const;          // "Mult"
    static Matrix4 Transpose(const Matrix4& src);  // "Transpose"
    static Matrix4 Inverse(const Matrix4& src);    // "Inverse"
    Vector3 MultVrt(const Vector3& v) const;       // "MultVrt"
    Vector3 MultInvVect(const Vector3& v) const;   // "MultInvVect"
    Vector3 MultVect(const Vector3& v) const;      // "MultVect"
    __declspec(noinline) Matrix4 operator/(SQFloat scalar) const;
    __declspec(noinline) Matrix4 operator-(const Matrix4& other) const;
    __declspec(noinline) Matrix4 operator+(const Matrix4& other) const;
    __declspec(noinline) Matrix4 operator*(const Matrix4& other) const;

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1Matrix4SizeCheck[sizeof(Matrix4) == 64 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::Matrix4, Matrix4)

// SqPlus instantiations for Matrix4. Binders and dispatchers that share a
// signature with a Vector3/Vector4 one are folded with it.

// TEMPLATE: VC1 0x0041dd40
// ?release@?$ConstructReleaseClass@UMatrix4@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041e0e0
// ??0?$SQClassDef@UMatrix4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@UMatrix4@vc1@@@SqPlus@@YAHPAUSQVM@@PAUMatrix4@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041e9c0
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@UMatrix4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041ea40
// ??$var@M@?$SQClassDefBase@UMatrix4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQMatrix4@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041eaa0
// ??$constant@UMatrix4@vc1@@@?$SQClassDefBase@UMatrix4@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@UMatrix4@vc1@@PBD@Z

// TEMPLATE: VC1 0x0041f6d0
// ??$RegisterClassType@UMatrix4@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x0041ff90
// ?construct@?$ConstructReleaseClass@UMatrix4@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00421f20
// ??$RegisterInstance@UMatrix4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BE?AU34@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00421fd0
// ??$RegisterInstance@UMatrix4@vc1@@P812@BE?AU12@M@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BE?AU34@M@ZPBD@Z

// TEMPLATE: VC1 0x00422080
// ??$RegisterInstance@UMatrix4@vc1@@P812@BE?AU12@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BE?AU34@XZPBD@Z

// TEMPLATE: VC1 0x00422130
// ??$RegisterInstance@UMatrix4@vc1@@P812@BEHABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BEHABU34@@ZPBD@Z

// TEMPLATE: VC1 0x004221e0
// ??$RegisterInstance@UMatrix4@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x00422290
// ??$RegisterInstance@UMatrix4@vc1@@P812@AEAAUVector3@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@AEAAUVector3@4@XZPBD@Z

// TEMPLATE: VC1 0x00422340
// ??$RegisterInstance@UMatrix4@vc1@@P812@AEXABUVector3@2@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@AEXABUVector3@4@@ZPBD@Z

// TEMPLATE: VC1 0x004223f0
// ??$RegisterInstance@UMatrix4@vc1@@P812@AEXXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@AEXXZPBD@Z

// TEMPLATE: VC1 0x004224a0
// ??$RegisterInstance@UMatrix4@vc1@@P812@BE?AUVector4@2@ABU32@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BE?AUVector4@4@ABU54@@ZPBD@Z

// TEMPLATE: VC1 0x00422550 FOLDED
// ??$RegisterInstance@UMatrix4@vc1@@P812@BE?AUVector3@2@ABU32@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAUMatrix4@vc1@@P834@BE?AUVector3@4@ABU54@@ZPBD@Z

// TEMPLATE: VC1 0x00422600
// ??$Register@P6A?AUMatrix4@vc1@@ABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@P6A?AUMatrix4@vc1@@ABU34@@ZPBD@Z

// TEMPLATE: VC1 0x004236e0
// ??$sq_typeof@UMatrix4@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423d70
// ??$Set@UMatrix4@vc1@@@SquirrelObject@@QAEAAV0@AAUMatrix4@vc1@@@Z

// TEMPLATE: VC1 0x00423f20 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BE?AUVector3@2@ABU32@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424060 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@AEXXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004241f0 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BE?AUVector4@2@ABU32@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004245d0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424620
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BE?AU12@M@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424670
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BEHABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x004246c0
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@AEAAUVector3@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424710
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@AEXABUVector3@2@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424910
// ?Dispatch@?$DirectCallInstanceMemberFunction@UMatrix4@vc1@@P812@BE?AU12@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@UMatrix4@vc1@@P812@BE?AU12@ABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x004253e0
// ?Dispatch@?$DirectCallFunction@P6A?AUMatrix4@vc1@@ABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00429020
// ??$GetInstance@UVector3@vc1@@$0A@@SqPlus@@YAPAUVector3@vc1@@PAUSQVM@@H@Z

#endif
