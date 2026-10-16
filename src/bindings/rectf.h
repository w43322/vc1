#ifndef VC1_BINDINGS_RECTF_H
#define VC1_BINDINGS_RECTF_H
#include "squirrel_object.h"
#include "memory/fixed_pool.h"
#include "rect.h"

namespace vc1 {

class KdString;
struct Vector2;

// Inferred name. Script class "RectF" (0x004d4ffc); SqPlus ClassType at
// 0x004ed844. Script fields X/Y/Width/Height (0x004d7400-0x004d7410).
// Pool pools[4]; the release hook is folded with Vector4's (0x0041dd60).
// The default constructor stores height, width, y, x in that order
// (ConstructArgs 0x004bdd60).
struct RectF
{
    SQFloat x;
    SQFloat y;
    SQFloat width;
    SQFloat height;

    RectF() { x = y = width = height = 0.0f; }
    RectF(SQFloat ax, SQFloat ay, SQFloat aw, SQFloat ah)
        : x(ax), y(ay), width(aw), height(ah) {}

    VC1_SIZE_CLASS_NEW(RectF)

    bool Intersect(const RectF& other);    // "Intersect"
    bool IsEmpty() const;                  // "IsEmpty"
    bool Union(const RectF& other);        // "Union"
    void Include(const Vector2& p);
    KdString& ToString() const;            // "_tostring"

    static SQInteger ConstructArgs(HSQUIRRELVM vm); // "constructor" with typemask "*"
};

typedef char Vc1RectFSizeCheck[sizeof(RectF) == 16 ? 1 : -1];

}

DECLARE_INSTANCE_TYPE_NAME(vc1::RectF, RectF)

// SqPlus instantiations for RectF.

// TEMPLATE: VC1 0x0041dd60 FOLDED
// ?release@?$ConstructReleaseClass@URectF@vc1@@@SqPlus@@SAHPAXH@Z

// TEMPLATE: VC1 0x0041e290
// ??0?$SQClassDef@URectF@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAE@PBD@Z

// TEMPLATE: VC1 0x0041e3b0 FOLDED
// ??$PostConstruct@URectF@vc1@@@SqPlus@@YAHPAUSQVM@@PAURectF@vc1@@P6AHPAXH@Z@Z

// TEMPLATE: VC1 0x0041ed40
// ??$staticFuncVarArgs@P6AHPAUSQVM@@@Z@?$SQClassDefBase@URectF@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@P6AHPAUSQVM@@@ZPBD2@Z

// TEMPLATE: VC1 0x0041edc0
// ??$var@M@?$SQClassDefBase@URectF@vc1@@USQNoBaseClass@SqPlus@@@SqPlus@@QAEAAU01@PQRectF@vc1@@MPBDW4VarAccessType@1@@Z

// TEMPLATE: VC1 0x0041faf0
// ??$RegisterClassType@URectF@vc1@@@SqPlus@@YA?AVSquirrelObject@@PAUSQVM@@PBD1@Z

// TEMPLATE: VC1 0x004200c0
// ?construct@?$ConstructReleaseClass@URectF@vc1@@@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00423200
// ??$RegisterInstance@URectF@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURectF@vc1@@P834@BEAAVKdString@4@XZPBD@Z

// TEMPLATE: VC1 0x004232b0
// ??$RegisterInstance@URectF@vc1@@P812@BE_NXZ@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURectF@vc1@@P834@BE_NXZPBD@Z

// TEMPLATE: VC1 0x00423360
// ??$RegisterInstance@URectF@vc1@@P812@AE_NABU12@@Z@SqPlus@@YAXPAUSQVM@@UtagSQObject@@AAURectF@vc1@@P834@AE_NABU34@@ZPBD@Z

// TEMPLATE: VC1 0x00423740
// ??$sq_typeof@URectF@vc1@@@SqPlus@@YAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424470 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@URectF@vc1@@P812@BEAAVKdString@2@XZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424a50 FOLDED
// ?Dispatch@?$DirectCallInstanceMemberFunction@URectF@vc1@@P812@BE_NXZ@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424aa0
// ?Dispatch@?$DirectCallInstanceMemberFunction@URectF@vc1@@P812@AE_NABU12@@Z@SqPlus@@SAHPAUSQVM@@@Z

// TEMPLATE: VC1 0x00424c00 FOLDED
// ??0?$DirectCallInstanceFuncPicker@URectF@vc1@@P812@AE_NABU12@@Z@SqPlus@@QAE@PAUSQVM@@@Z

// TEMPLATE: VC1 0x00428cf0
// ??$GetInstance@URectF@vc1@@$0A@@SqPlus@@YAPAURectF@vc1@@PAUSQVM@@H@Z

#endif
